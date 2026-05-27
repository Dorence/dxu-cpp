#include "dxu/concurrency/work_queue.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "test_common.h"

namespace DXU_NAMESPACE {

struct Popper {
  WorkQueue<int>* queue;
  int* results;
  std::mutex* mutex;

  void operator()() {
    int result;
    while (queue->pop(result)) {
      std::lock_guard<std::mutex> lock(*mutex);
      results[result] = result;
    }
  }
};

TEST_CASE("WorkQueue::SingleThreaded") {
  WorkQueue<int> queue;
  int result;

  REQUIRE(queue.push(5));
  REQUIRE(queue.pop(result));
  REQUIRE(result == 5);

  REQUIRE(queue.push(1));
  REQUIRE(queue.push(2));
  REQUIRE(queue.pop(result));
  REQUIRE(result == 1);
  REQUIRE(queue.pop(result));
  REQUIRE(result == 2);

  REQUIRE(queue.push(1));
  REQUIRE(queue.push(2));
  queue.finish();
  REQUIRE(queue.pop(result));
  REQUIRE(result == 1);
  REQUIRE(queue.pop(result));
  REQUIRE(result == 2);
  REQUIRE_FALSE(queue.pop(result));

  queue.waitUntilFinished();
}

TEST_CASE("WorkQueue::SPSC") {
  WorkQueue<int> queue;
  constexpr int max = 100;
  std::vector<int> popped;
  popped.reserve(max);
  std::atomic<bool> pop_count_matches{true};

  for (int i = 0; i < 10; ++i) {
    REQUIRE(queue.push(i));
  }

  std::thread thread([&queue, &popped, &pop_count_matches] {
    int result;
    for (int i = 0;; ++i) {
      if (!queue.pop(result)) {
        if (i != max) {
          pop_count_matches.store(false);
        }
        break;
      }
      popped.push_back(result);
    }
  });

  std::this_thread::yield();
  for (int i = 10; i < max; ++i) {
    REQUIRE(queue.push(i));
  }
  queue.finish();

  thread.join();

  REQUIRE(pop_count_matches.load());
  REQUIRE(popped.size() == static_cast<std::size_t>(max));
  for (int i = 0; i < max; ++i) {
    REQUIRE(popped[i] == i);
  }
}

TEST_CASE("WorkQueue::SPMC") {
  WorkQueue<int> queue;
  std::vector<int> results(50, -1);
  std::mutex mutex;
  std::vector<std::thread> threads;
  for (int i = 0; i < 5; ++i) {
    threads.emplace_back(Popper{&queue, results.data(), &mutex});
  }

  for (int i = 0; i < 50; ++i) {
    REQUIRE(queue.push(i));
  }
  queue.finish();

  for (auto& thread : threads) {
    thread.join();
  }

  for (int i = 0; i < 50; ++i) {
    REQUIRE(results[i] == i);
  }
}

TEST_CASE("WorkQueue::MPMC") {
  WorkQueue<int> queue;
  std::vector<int> results(100, -1);
  std::mutex mutex;
  std::vector<std::thread> popperThreads;
  for (int i = 0; i < 4; ++i) {
    popperThreads.emplace_back(Popper{&queue, results.data(), &mutex});
  }

  std::vector<std::thread> pusherThreads;
  for (int i = 0; i < 2; ++i) {
    auto min = i * 50;
    auto max = (i + 1) * 50;
    pusherThreads.emplace_back([&queue, min, max] {
      for (int j = min; j < max; ++j) {
        queue.push(j);
      }
    });
  }

  for (auto& thread : pusherThreads) {
    thread.join();
  }
  queue.finish();

  for (auto& thread : popperThreads) {
    thread.join();
  }

  for (int i = 0; i < 100; ++i) {
    REQUIRE(results[i] == i);
  }
}

TEST_CASE("WorkQueue::BoundedSizeWorks") {
  WorkQueue<int> queue(1);
  int result;
  REQUIRE(queue.push(5));
  REQUIRE(queue.pop(result));
  REQUIRE(queue.push(5));
  REQUIRE(queue.pop(result));
  REQUIRE(queue.push(5));
  queue.finish();
  REQUIRE(queue.pop(result));
  REQUIRE(result == 5);
}

TEST_CASE("WorkQueue::BoundedSizePushAfterFinish") {
  WorkQueue<int> queue(1);
  int result;
  REQUIRE(queue.push(5));
  std::thread pusher([&queue] { queue.push(6); });
  // Dirtily try and make sure that pusher has run.
  std::this_thread::sleep_for(std::chrono::seconds(1));
  queue.finish();
  REQUIRE(queue.pop(result));
  REQUIRE(result == 5);
  REQUIRE_FALSE(queue.pop(result));

  pusher.join();
}

TEST_CASE("WorkQueue::SetMaxSize") {
  WorkQueue<int> queue(2);
  int result;
  REQUIRE(queue.push(5));
  REQUIRE(queue.push(6));
  queue.setMaxSize(1);
  std::thread pusher([&queue] { queue.push(7); });
  // Dirtily try and make sure that pusher has run.
  std::this_thread::sleep_for(std::chrono::seconds(1));
  queue.finish();
  REQUIRE(queue.pop(result));
  REQUIRE(result == 5);
  REQUIRE(queue.pop(result));
  REQUIRE(result == 6);
  REQUIRE_FALSE(queue.pop(result));

  pusher.join();
}

TEST_CASE("WorkQueue::BoundedSizeMPMC") {
  WorkQueue<int> queue(10);
  std::vector<int> results(200, -1);
  std::mutex mutex;
  std::cerr << "Creating popperThreads" << std::endl;
  std::vector<std::thread> popperThreads;
  for (int i = 0; i < 4; ++i) {
    popperThreads.emplace_back(Popper{&queue, results.data(), &mutex});
  }

  std::cerr << "Creating pusherThreads" << std::endl;
  std::vector<std::thread> pusherThreads;
  for (int i = 0; i < 2; ++i) {
    auto min = i * 100;
    auto max = (i + 1) * 100;
    pusherThreads.emplace_back([&queue, min, max] {
      for (int j = min; j < max; ++j) {
        queue.push(j);
      }
    });
  }

  std::cerr << "Joining pusherThreads" << std::endl;
  for (auto& thread : pusherThreads) {
    thread.join();
  }
  std::cerr << "Finishing queue" << std::endl;
  queue.finish();

  std::cerr << "Joining popperThreads" << std::endl;
  for (auto& thread : popperThreads) {
    thread.join();
  }

  std::cerr << "Inspecting results" << std::endl;
  for (int i = 0; i < 200; ++i) {
    REQUIRE(results[i] == i);
  }
}

TEST_CASE("WorkQueue::FailedPush") {
  WorkQueue<int> queue;
  REQUIRE(queue.push(1));
  queue.finish();
  REQUIRE_FALSE(queue.push(1));
}

TEST_CASE("WorkQueue::FailedPop") {
  WorkQueue<int> queue;
  int x = 5;
  REQUIRE(queue.push(x));
  queue.finish();
  x = 0;
  REQUIRE(queue.pop(x));
  REQUIRE(x == 5);
  REQUIRE_FALSE(queue.pop(x));
  REQUIRE(x == 5);
}

TEST_CASE("BatchWorkQueue::SizeCalculatedCorrectly") {
  using Buffer = std::vector<int>;
  {
    BatchWorkQueue<Buffer> queue;
    queue.finish();
    REQUIRE(queue.size() == 0);
  }
  {
    BatchWorkQueue<Buffer> queue;
    queue.push(Buffer(10, 123));
    queue.finish();
    REQUIRE(queue.size() == 10);
  }
  {
    BatchWorkQueue<Buffer> queue;
    queue.push(Buffer(10, 123));
    queue.push(Buffer(5, 321));
    queue.finish();
    REQUIRE(queue.size() == 15);
  }
  {
    BatchWorkQueue<Buffer> queue;
    queue.push(Buffer(10));
    queue.push(Buffer(5));
    queue.finish();
    Buffer buffer;
    REQUIRE(queue.pop(buffer));
    REQUIRE(queue.size() == 5);
    REQUIRE(queue.pop(buffer));
    REQUIRE(queue.size() == 0);
    REQUIRE_FALSE(queue.pop(buffer));
  }
}

}  // namespace DXU_NAMESPACE
