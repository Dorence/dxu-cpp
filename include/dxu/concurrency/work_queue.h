/**
 * Thread-safe work queue.
 *
 * Inspired by facebook's rocksdb/pzstd implementation.
 */
#pragma once

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <queue>

#include "dxu/port/cacheline.h"
#include "dxu/version.h"

namespace DXU_NAMESPACE {

/** Unbounded thread-safe work queue. */
template <typename T>
class WorkQueue {
  std::mutex mutex_;
  std::condition_variable reader_cv_;
  std::condition_variable writer_cv_;
  std::condition_variable finish_cv_;

  std::queue<T> queue_;
  bool done_;
  std::size_t max_size_;

  /** Not thread-safe */
  bool full() const { return max_size_ > 0 && queue_.size() >= max_size_; }

 public:
  /**
   * Constructs an empty work queue with an optional max size.
   * If `max_size == 0` the queue size is unbounded.
   */
  WorkQueue(std::size_t max_size = 0) : done_(false), max_size_(max_size) {}

  /**
   * Push an item onto the work queue. Notify a single thread that work is
   * available. If `finish()` has been called, do nothing and return false.
   * If `push()` returns false, then `item` has not been moved from.
   *
   * @param item  Item to push onto the queue.
   * @returns     True upon success, false if `finish()` has been called.
   *              An item was pushed iff `push()` returns true.
   */
  template <typename U>
  bool push(U&& item) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      while (full() && !done_) {
        writer_cv_.wait(lock);
      }
      if (done_) {
        return false;
      }
      queue_.push(std::forward<U>(item));
    }
    reader_cv_.notify_one();
    return true;
  }

  /**
   * Attempts to pop an item off the work queue. It will block until data is
   * available or `finish()` has been called.
   *
   * @param[out] item  If `pop` returns `true`, it contains the popped item.
   *                   If `pop` returns `false`, it is unmodified.
   * @returns          True upon success. False if the queue is empty and
   *                   `finish()` has been called.
   */
  bool pop(T& item) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      while (queue_.empty() && !done_) {
        reader_cv_.wait(lock);
      }
      if (queue_.empty()) {
        assert(done_);
        return false;
      }
      item = std::move(queue_.front());
      queue_.pop();
    }
    writer_cv_.notify_one();
    return true;
  }

  /** Sets the maximum queue size. 0 means unbounded */
  void setMaxSize(std::size_t max_size) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      max_size_ = max_size;
    }
    writer_cv_.notify_all();
  }

  /**
   * Promise that either the reader side or the writer side is done.
   * If the writer is done, `push()` won't be called again, so once the queue
   * is empty there will never be any more work. If the reader is done, `pop()`
   * won't be called again, so further items pushed will just be ignored.
   */
  void finish() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      assert(!done_);
      done_ = true;
    }
    reader_cv_.notify_all();
    writer_cv_.notify_all();
    finish_cv_.notify_all();
  }

  /** Blocks until `finish()` has been called (but queue may not be empty) */
  void waitUntilFinished() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (!done_) {
      finish_cv_.wait(lock);
    }
  }
};

/**
 * Work queue that pass array of elements, and count the number of elements in
 * the queue.
 */
template <typename Array>
class BatchWorkQueue {
  WorkQueue<Array> queue_;
  FIELD_CL_PAD(_pad_queue_, sizeof(WorkQueue<Array>));
  std::atomic<std::size_t> size_;

 public:
  BatchWorkQueue(std::size_t maxSize = 0) : queue_(maxSize), size_(0) {}

  template <typename U>
  bool push(U&& buffer) {
    std::size_t sz = buffer.size();
    bool ok = queue_.push(std::forward<U>(buffer));
    if (ok) {
      size_.fetch_add(sz);
    }
    return ok;
  }

  /**
   * Block wait until data is available or `finish()` has been called.
   * @returns true upon success. false if `finish()` has been called and all
   * data in the queue is consumed.
   */
  bool pop(Array& buffer) {
    bool result = queue_.pop(buffer);
    if (result) {
      size_.fetch_sub(buffer.size());
    }
    return result;
  }

  void finish() { queue_.finish(); }

  void setMaxSize(std::size_t maxSize) { queue_.setMaxSize(maxSize); }

  std::size_t size() { return size_.load(); }

  void waitUntilFinished() { queue_.waitUntilFinished(); }
};

}  // namespace DXU_NAMESPACE
