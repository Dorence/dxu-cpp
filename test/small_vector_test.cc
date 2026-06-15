#include "dxu/small_vector.h"

#include <array>
#include <list>

#include "test_common.h"

namespace DXU_NAMESPACE {

// A helper class that counts the total number of constructor and
// destructor calls.
class Constructable {
 public:
  static int numConstructorCalls;
  static int numMoveConstructorCalls;
  static int numCopyConstructorCalls;
  static int numDestructorCalls;
  static int numAssignmentCalls;
  static int numMoveAssignmentCalls;
  static int numCopyAssignmentCalls;

 private:
  bool constructed_;
  int value_;

 public:
  Constructable() : constructed_(true), value_(0) { ++numConstructorCalls; }

  explicit Constructable(int value) : constructed_(true), value_(value) {
    ++numConstructorCalls;
  }

  Constructable(const Constructable& other)
      : constructed_(true), value_(other.value_) {
    CHECK(other.constructed_);
    ++numConstructorCalls;
    ++numCopyConstructorCalls;
  }

  Constructable(Constructable&& other) noexcept
      : constructed_(true), value_(other.value_) {
    CHECK(other.constructed_);
    other.value_ = 0;
    ++numConstructorCalls;
    ++numMoveConstructorCalls;
  }

  ~Constructable() {
    CHECK(constructed_);
    ++numDestructorCalls;
    constructed_ = false;
  }

  Constructable& operator=(const Constructable& other) {
    CHECK(constructed_);
    value_ = other.value_;
    ++numAssignmentCalls;
    ++numCopyAssignmentCalls;
    return *this;
  }

  Constructable& operator=(Constructable&& other) noexcept {
    CHECK(constructed_);
    value_ = other.value_;
    other.value_ = 0;
    ++numAssignmentCalls;
    ++numMoveAssignmentCalls;
    return *this;
  }

  int getValue() const { return std::abs(value_); }

  static void reset() {
    numConstructorCalls = 0;
    numMoveConstructorCalls = 0;
    numCopyConstructorCalls = 0;
    numDestructorCalls = 0;
    numAssignmentCalls = 0;
    numMoveAssignmentCalls = 0;
    numCopyAssignmentCalls = 0;
  }

  friend bool operator==(const Constructable& lhs, const Constructable& rhs) {
    return lhs.getValue() == rhs.getValue();
  }
};

int Constructable::numConstructorCalls;
int Constructable::numCopyConstructorCalls;
int Constructable::numMoveConstructorCalls;
int Constructable::numDestructorCalls;
int Constructable::numAssignmentCalls;
int Constructable::numCopyAssignmentCalls;
int Constructable::numMoveAssignmentCalls;

struct NonCopyable {
  NonCopyable() = default;
  NonCopyable(NonCopyable&&) noexcept {}
  NonCopyable& operator=(NonCopyable&&) noexcept { return *this; }

 private:
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
};

struct CopyOnly {
  int value;

  CopyOnly() : value{0} {}
  explicit CopyOnly(int v) : value{v} {}
  CopyOnly(const CopyOnly&) = default;
  CopyOnly& operator=(const CopyOnly&) = default;

 private:
  CopyOnly(CopyOnly&&) = delete;
  CopyOnly& operator=(CopyOnly&&) = delete;
};

template <typename T = int>
class InputIterator {
 public:
  using iterator_category = std::input_iterator_tag;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer = const T*;    // const iterator
  using reference = const T&;  // const iterator

  explicit InputIterator(pointer ptr) : ptr_(ptr) {}

  reference operator*() const { return *ptr_; }

  InputIterator& operator++() {
    ++ptr_;
    return *this;
  }

  InputIterator operator++(int) {
    InputIterator copy{*this};
    ++(*this);
    return copy;
  }

  friend bool operator==(const InputIterator& lhs, const InputIterator& rhs) {
    return lhs.ptr_ == rhs.ptr_;
  }

  friend bool operator!=(const InputIterator& lhs, const InputIterator& rhs) {
    return !(lhs == rhs);
  }

 private:
  pointer ptr_;
};

enum class EmplaceableArgState {
  Defaulted,
  Arg,
  LValue,
  RValue,
  Failure,
};

template <int I>
struct EmplaceableArg {
  EmplaceableArgState state{EmplaceableArgState::Defaulted};

  EmplaceableArg() = default;
  explicit EmplaceableArg(bool) : state{EmplaceableArgState::Arg} {}

  EmplaceableArg(EmplaceableArg&& other) noexcept
      : state{other.state == EmplaceableArgState::Arg
                  ? EmplaceableArgState::RValue
                  : EmplaceableArgState::Failure} {}

  EmplaceableArg(EmplaceableArg& other)
      : state{other.state == EmplaceableArgState::Arg
                  ? EmplaceableArgState::LValue
                  : EmplaceableArgState::Failure} {}

  EmplaceableArg& operator=(EmplaceableArg&&) = delete;
  EmplaceableArg& operator=(const EmplaceableArg&) = delete;
};

enum class EmplaceableState {
  Emplaced,
  Moved,
};

struct Emplaceable {
  EmplaceableArg<0> a0;
  EmplaceableArg<1> a1;
  EmplaceableArg<2> a2;
  EmplaceableArg<3> a3;
  EmplaceableState state{EmplaceableState::Emplaced};

  Emplaceable() = default;

  template <typename A0>
  explicit Emplaceable(A0&& arg0) : a0{std::forward<A0>(arg0)} {}

  template <typename A0, typename A1>
  Emplaceable(A0&& arg0, A1&& arg1)
      : a0{std::forward<A0>(arg0)}, a1{std::forward<A1>(arg1)} {}

  template <typename A0, typename A1, typename A2, typename A3>
  Emplaceable(A0&& arg0, A1&& arg1, A2&& arg2, A3&& arg3)
      : a0{std::forward<A0>(arg0)},
        a1{std::forward<A1>(arg1)},
        a2{std::forward<A2>(arg2)},
        a3{std::forward<A3>(arg3)} {}

  Emplaceable(Emplaceable&&) noexcept : state{EmplaceableState::Moved} {}
  Emplaceable& operator=(Emplaceable&&) noexcept {
    state = EmplaceableState::Moved;
    return *this;
  }

  Emplaceable(const Emplaceable&) = delete;
  Emplaceable& operator=(const Emplaceable&) = delete;
};

template <typename VecT, typename... Values>
void EXPECT_VALUES(const VecT& vector, Values... values) {
  const int expected[]{values...};
  constexpr std::size_t expected_size{sizeof...(Values)};

  REQUIRE(vector.size() == expected_size);
  for (std::size_t i = 0; i < expected_size; ++i) {
    REQUIRE(vector[i].getValue() == expected[i]);
    REQUIRE(&vector[i] == vector.data() + i);
  }
}

template <typename VecT>
void EXPECT_EMPTY(const VecT& vector) {
  REQUIRE(vector.size() == 0UL);
  REQUIRE(vector.empty());
  REQUIRE(vector.begin() == vector.end());
}

// Generate a sequence of values to initialize the vector.
template <typename VecT>
void make_sequence(VecT& vector, int first, int last) {
  for (int value = first; value <= last; ++value) {
    vector.push_back(Constructable{value});
  }
}

// Mock gtest's TYPED_TEST_SUITE
#define RUN_TEST_CAPACITY(func) \
  do {                          \
    func<1>();                  \
    func<2>();                  \
    func<4>();                  \
    func<5>();                  \
    func<100>();                \
  } while (0)

template <std::size_t N>
void test_construct_non_copyable() {
  SmallVector<NonCopyable, N> vector(42);
  REQUIRE(vector.size() == 42UL);
  REQUIRE(vector.data() != nullptr);
}

// Adapted from LLVM ConstructNonCopyableTest.
// dxu::SmallVector requires N>0, use N=1.
TEST_CASE("SmallVectorTest::ConstructNonCopyableTest") {
  test_construct_non_copyable<1>();
}

template <std::size_t N>
void test_constructor_non_iter() {
  Constructable::reset();
  SmallVector<Constructable, N> V{};
  V = SmallVector<Constructable, 2>(2, Constructable{2});
  EXPECT_VALUES(V, 2, 2);
}

// Adapted from LLVM ConstructorNonIterTest.
TEST_CASE("SmallVectorTest::ConstructorNonIterTest") {
  RUN_TEST_CAPACITY(test_constructor_non_iter);
}

template <std::size_t N>
void test_empty_vector() {
  Constructable::reset();
  SmallVector<Constructable, N> V;
  EXPECT_EMPTY(V);
  REQUIRE(V.rbegin() == V.rend());
  REQUIRE(Constructable::numConstructorCalls == 0);
  REQUIRE(Constructable::numDestructorCalls == 0);
}

// Adapted from LLVM EmptyVectorTest.
TEST_CASE("SmallVectorTest::EmptyVectorTest") {
  RUN_TEST_CAPACITY(test_empty_vector);
}

template <std::size_t N>
void test_push_back() {
  Constructable::reset();
  SmallVector<Constructable, N> V;

  // Push an element
  V.push_back(Constructable{1});

  // Size tests
  EXPECT_VALUES(V, 1);
  REQUIRE_FALSE(V.begin() == V.end());
  REQUIRE_FALSE(V.empty());

  // Push another element
  V.push_back(Constructable(2));
  EXPECT_VALUES(V, 1, 2);

  // Each push_back constructs a temporary argument and move-constructs it into
  // storage (2 ctors, 1 dtor per call). The two live elements always equal
  // ctor - dtor.
  if (N < 2) {
    // The second push_back must grow, relocating the existing element.
    REQUIRE(Constructable::numConstructorCalls == 5);
    REQUIRE(Constructable::numDestructorCalls == 3);
  } else {
    REQUIRE(Constructable::numConstructorCalls == 4);
    REQUIRE(Constructable::numDestructorCalls == 2);
  }
}

// Adapted from LLVM PushPopTest.
// dxu::SmallVector has no insert/pop APIs, so this covers push_back only.
TEST_CASE("SmallVectorTest::PushBackTest") {
  RUN_TEST_CAPACITY(test_push_back);
}

template <std::size_t N>
void test_clear() {
  Constructable::reset();
  SmallVector<Constructable, N> vector;

  vector.reserve(2);
  make_sequence(vector, 1, 2);
  vector.clear();

  EXPECT_EMPTY(vector);
  REQUIRE(Constructable::numConstructorCalls == 4);
  REQUIRE(Constructable::numDestructorCalls == 4);
}

// Adapted from LLVM ClearTest.
TEST_CASE("SmallVectorTest::ClearTest") { RUN_TEST_CAPACITY(test_clear); }

template <std::size_t N>
void test_resize_shrink() {
  Constructable::reset();
  SmallVector<Constructable, N> vector;

  vector.reserve(3);
  make_sequence(vector, 1, 3);
  vector.resize(1);

  EXPECT_VALUES(vector, 1);
  REQUIRE(Constructable::numConstructorCalls == 6);
  REQUIRE(Constructable::numDestructorCalls == 5);
}

// Adapted from LLVM ResizeShrinkTest.
TEST_CASE("SmallVectorTest::ResizeShrinkTest") {
  RUN_TEST_CAPACITY(test_resize_shrink);
}

template <std::size_t N>
void test_resize_grow() {
  Constructable::reset();
  SmallVector<Constructable, N> vector;

  vector.resize(2);

  REQUIRE(Constructable::numConstructorCalls == 2);
  REQUIRE(Constructable::numDestructorCalls == 0);
  REQUIRE(vector.size() == std::size_t{2});
  REQUIRE(vector[0].getValue() == 0);
  REQUIRE(vector[1].getValue() == 0);
  REQUIRE(vector.data() != nullptr);
  REQUIRE(&vector[1] == vector.data() + 1);
}

// Adapted from LLVM ResizeGrowTest.
TEST_CASE("SmallVectorTest::ResizeGrowTest") {
  RUN_TEST_CAPACITY(test_resize_grow);
}

template <std::size_t N>
void test_resize_with_existing_elements() {
  Constructable::reset();
  SmallVector<Constructable, N> vector;

  vector.resize(2);
  Constructable::reset();
  vector.resize(4);

  // LLVM ResizeWithElementsTest tolerates either of the two SBO growth shapes.
  const std::size_t ctors = Constructable::numConstructorCalls;
  REQUIRE((ctors == 2 || ctors == 4));
  const std::size_t move_ctors = Constructable::numMoveConstructorCalls;
  REQUIRE((move_ctors == 0 || move_ctors == 2));
  const std::size_t dtors = Constructable::numDestructorCalls;
  REQUIRE((dtors == 0 || dtors == 2));

  REQUIRE(vector.size() == std::size_t{4});
  REQUIRE(vector[0].getValue() == 0);
  REQUIRE(vector[1].getValue() == 0);
  REQUIRE(vector[2].getValue() == 0);
  REQUIRE(vector[3].getValue() == 0);
  REQUIRE(&vector[3] == vector.data() + 3);
}

// Adapted from LLVM ResizeWithElementsTest.
// Mirrors the same tolerant ctor/move/dtor "or" assertions.
TEST_CASE("SmallVectorTest::ResizeWithElementsTest") {
  RUN_TEST_CAPACITY(test_resize_with_existing_elements);
}

template <std::size_t N>
void test_resize_fill() {
  Constructable::reset();
  SmallVector<Constructable, N> vector;

  vector.resize(3, Constructable{77});
  EXPECT_VALUES(vector, 77, 77, 77);
}

// Adapted from LLVM ResizeFillTest.
// Rewritten for Catch2.
TEST_CASE("SmallVectorTest::ResizeFillTest") {
  RUN_TEST_CAPACITY(test_resize_fill);
}

template <std::size_t N>
void test_overflow_inline_storage() {
  Constructable::reset();
  SmallVector<Constructable, N> vector;

  make_sequence(vector, 1, 10);

  REQUIRE(vector.size() == std::size_t{10});
  for (std::size_t i{0}; i < vector.size(); ++i) {
    REQUIRE(vector[i].getValue() == static_cast<int>(i + 1));
    REQUIRE(&vector[i] == vector.data() + i);
  }

  vector.resize(1);
  EXPECT_VALUES(vector, 1);
}

// Adapted from LLVM OverflowTest.
// Verifies contiguous storage explicitly for dxu::SmallVector.
TEST_CASE("SmallVectorTest::OverflowTest") {
  RUN_TEST_CAPACITY(test_overflow_inline_storage);
}

template <std::size_t N>
void test_front_back_accessors() {
  Constructable::reset();
  SmallVector<Constructable, N> vector;
  make_sequence(vector, 1, 2);

  REQUIRE(vector.front() == vector[0]);
  REQUIRE(vector.front().getValue() == 1);
  REQUIRE(vector.back() == vector[1]);
  REQUIRE(vector.back().getValue() == 2);

  const SmallVector<Constructable, N>& const_vector = vector;
  REQUIRE(const_vector.front() == const_vector[0]);
  REQUIRE(const_vector.front().getValue() == 1);
  REQUIRE(const_vector.back() == const_vector[1]);
  REQUIRE(const_vector.back().getValue() == 2);
}

template <std::size_t N>
void test_begin_end_iteration() {
  SmallVector<Constructable, N> vector;
  REQUIRE(vector.begin() == vector.end());

  make_sequence(vector, 1, 2);

  auto it = vector.begin();
  REQUIRE(*it == vector.front());
  REQUIRE(*it == vector[0]);
  REQUIRE(it->getValue() == 1);
  ++it;
  REQUIRE(*it == vector[1]);
  REQUIRE(*it == vector.back());
  REQUIRE(it->getValue() == 2);
  ++it;
  REQUIRE(it == vector.end());
  // Bidirectional iteration, mirroring LLVM IterationTest.
  --it;
  REQUIRE(*it == vector[1]);
  REQUIRE(it->getValue() == 2);
  --it;
  REQUIRE(*it == vector[0]);
  REQUIRE(it->getValue() == 1);

  auto rit = vector.rbegin();
  REQUIRE(*rit == vector[1]);
  REQUIRE(rit->getValue() == 2);
  ++rit;
  REQUIRE(*rit == vector[0]);
  REQUIRE(rit->getValue() == 1);
  ++rit;
  REQUIRE(rit == vector.rend());
  --rit;
  REQUIRE(*rit == vector[0]);
  REQUIRE(rit->getValue() == 1);
  --rit;
  REQUIRE(*rit == vector[1]);
  REQUIRE(rit->getValue() == 2);

  int expected_value = 10;
  for (auto& value : vector) {
    value = Constructable{expected_value++};
  }
  EXPECT_VALUES(vector, 10, 11);

  const SmallVector<Constructable, N>& const_vector = vector;
  auto const_it = const_vector.begin();
  REQUIRE(const_it == const_vector.cbegin());
  REQUIRE(*const_it == const_vector[0]);
  REQUIRE(const_it->getValue() == 10);
  ++const_it;
  REQUIRE(*const_it == const_vector[1]);
  REQUIRE(const_it->getValue() == 11);
  ++const_it;
  REQUIRE(const_it == const_vector.end());
  REQUIRE(const_it == const_vector.cend());

  auto const_rit = const_vector.rbegin();
  REQUIRE(const_rit == const_vector.crbegin());
  REQUIRE(*const_rit == const_vector[1]);
  REQUIRE(const_rit->getValue() == 11);
  ++const_rit;
  REQUIRE(*const_rit == const_vector[0]);
  REQUIRE(const_rit->getValue() == 10);
  ++const_rit;
  REQUIRE(const_rit == const_vector.rend());
  REQUIRE(const_rit == const_vector.crend());

  int sum = 0;
  for (const auto& value : const_vector) {
    sum += value.getValue();
  }
  REQUIRE(sum == 21);
}

// Adapted from LLVM IterationTest.
// Includes reverse iterator and range-for checks for dxu pointer iterators.
TEST_CASE("SmallVectorTest::IterationTest") {
  RUN_TEST_CAPACITY(test_front_back_accessors);
  RUN_TEST_CAPACITY(test_begin_end_iteration);
}

template <std::size_t N>
void test_swap_small_and_empty() {
  SmallVector<Constructable, N> vector;
  SmallVector<Constructable, N> other;
  make_sequence(vector, 1, 2);

  vector.swap(other);

  EXPECT_EMPTY(vector);
  EXPECT_VALUES(other, 1, 2);
}

// Adapted from LLVM SwapTest.
// dxu::SmallVector only exposes member swap.
TEST_CASE("SmallVectorTest::SwapTest") {
  RUN_TEST_CAPACITY(test_swap_small_and_empty);
}

template <std::size_t N>
void test_assign() {
  SmallVector<Constructable, N> vector;
  auto& V = vector;
  V.push_back(Constructable{1});
  V.assign(2, Constructable{77});
  EXPECT_VALUES(V, 77, 77);
}

// Adapted from LLVM AssignTest.
TEST_CASE("SmallVectorTest::AssignTest") { RUN_TEST_CAPACITY(test_assign); }

template <std::size_t N>
void test_assign_range() {
  SmallVector<Constructable, N> vector;
  auto& V = vector;
  V.push_back(Constructable{1});
  Constructable arr[]{Constructable{1}, Constructable{2}, Constructable{3}};
  V.assign(std::begin(arr), std::end(arr));
  EXPECT_VALUES(V, 1, 2, 3);
}

// Adapted from LLVM AssignRangeTest.
// dxu::Constructable(int) is explicit, so the source array is built explicitly.
TEST_CASE("SmallVectorTest::AssignRangeTest") {
  RUN_TEST_CAPACITY(test_assign_range);
}

template <std::size_t N>
void test_assign_non_iter() {
  SmallVector<Constructable, N> vector;
  auto& V = vector;
  V.push_back(Constructable{1});
  V.assign(2, Constructable{7});
  EXPECT_VALUES(V, 7, 7);
}

// Adapted from LLVM AssignNonIterTest.
// dxu::Constructable(int) is explicit, so the fill value is Constructable{7}.
TEST_CASE("SmallVectorTest::AssignNonIterTest") {
  RUN_TEST_CAPACITY(test_assign_non_iter);
}

template <std::size_t N>
void test_assign_small_vector() {
  SmallVector<Constructable, N> vector;
  auto& V = vector;
  SmallVector<Constructable, 3> otherVector = {Constructable{7},
                                               Constructable{7}};
  V.push_back(Constructable{1});
  V.assign(otherVector);
  EXPECT_VALUES(V, 7, 7);
}

// Adapted from LLVM AssignSmallVector.
TEST_CASE("SmallVectorTest::AssignSmallVector") {
  RUN_TEST_CAPACITY(test_assign_small_vector);
}

void test_assign_range_from_small_vector_different_n_reallocate() {
  SmallVector<CopyOnly, 4> source;
  for (int value : {11, 13, 17}) {
    CopyOnly item{value};
    source.push_back(item);
  }

  SmallVector<CopyOnly, 1> target;
  CopyOnly initial{99};
  target.push_back(initial);
  const CopyOnly* before = target.data();

  target.assign(source);

  REQUIRE(target.size() == std::size_t{3});
  REQUIRE(target[0].value == 11);
  REQUIRE(target[1].value == 13);
  REQUIRE(target[2].value == 17);
  REQUIRE(target.data() != before);
}

void test_assign_range_from_small_vector_different_n_reuses_existing_capacity() {
  SmallVector<CopyOnly, 1> source;
  for (int value : {21, 34}) {
    CopyOnly item{value};
    source.push_back(item);
  }

  SmallVector<CopyOnly, 4> target;
  target.reserve(8);
  CopyOnly initial{99};
  target.push_back(initial);
  const CopyOnly* before = target.data();
  const std::size_t before_capacity = target.capacity();

  target.assign(source);

  REQUIRE(target.data() == before);
  REQUIRE(target.capacity() == before_capacity);
  REQUIRE(target.size() == std::size_t{2});
  REQUIRE(target[0].value == 21);
  REQUIRE(target[1].value == 34);
}

void test_assign_count_value_grow_and_shrink() {
  SmallVector<int, 2> vector{1, 2, 3};
  vector.assign(4, 9);

  REQUIRE(vector.size() == std::size_t{4});
  REQUIRE(vector.capacity() >= std::size_t{4});
  for (const auto& value : vector) {
    REQUIRE(value == 9);
  }

  const int* before = vector.data();
  const std::size_t before_capacity = vector.capacity();
  vector.assign(1, 5);
  REQUIRE(vector.size() == std::size_t{1});
  REQUIRE(vector[0] == 5);
  // Shrinking must not reallocate.
  REQUIRE(vector.data() == before);
  REQUIRE(vector.capacity() == before_capacity);
}

void test_assign_range_preallocates_for_forward_iterators() {
  // std::list exposes bidirectional (non-random-access) iterators, so the size
  // must be computed via std::distance before reserving.
  std::list<int> source{1, 2, 3, 4};
  SmallVector<int, 2> target;
  target.push_back(99);

  target.assign(source.begin(), source.end());

  REQUIRE(target.size() == std::size_t{4});
  REQUIRE(target.capacity() == std::size_t{4});
  REQUIRE(target[0] == 1);
  REQUIRE(target[1] == 2);
  REQUIRE(target[2] == 3);
  REQUIRE(target[3] == 4);
}

void test_assign_range_from_input_iterators() {
  const int values[]{3, 5, 7};
  SmallVector<int, 2> target{99};

  target.assign(InputIterator{values}, InputIterator{values + 3});

  REQUIRE(target.size() == std::size_t{3});
  REQUIRE(target[0] == 3);
  REQUIRE(target[1] == 5);
  REQUIRE(target[2] == 7);
}

void test_assign_range_from_random_access_iterators() {
  std::array<int, 4> source{1, 2, 3, 4};
  SmallVector<int, 2> target{99};
  target.assign(source.begin(), source.end());
  REQUIRE(target.size() == std::size_t{4});
  REQUIRE(target[0] == 1);
  REQUIRE(target[1] == 2);
  REQUIRE(target[2] == 3);
  REQUIRE(target[3] == 4);
}

void test_assign_initializer_list() {
  SmallVector<int, 2> vector{1, 2, 3};
  vector.assign({4});
  REQUIRE(vector.size() == std::size_t{1});
  REQUIRE(vector[0] == 4);

  vector.assign({7, 8, 9});
  REQUIRE(vector.size() == std::size_t{3});
  REQUIRE(vector[0] == 7);
  REQUIRE(vector[1] == 8);
  REQUIRE(vector[2] == 9);
}

// Non-LLVM coverage for dxu cross-capacity and input-iterator assignment paths.
TEST_CASE("SmallVectorTest::AssignAdditionalCases") {
  test_assign_range_from_small_vector_different_n_reallocate();
  test_assign_range_from_small_vector_different_n_reuses_existing_capacity();
  test_assign_count_value_grow_and_shrink();
  test_assign_range_preallocates_for_forward_iterators();
  test_assign_range_from_random_access_iterators();
  test_assign_range_from_input_iterators();
  test_assign_initializer_list();
}

template <std::size_t N>
void test_move_assignment_matrix() {
  auto check_case = [](std::size_t source_size, std::size_t target_size,
                       bool expect_heap_steal) {
    SmallVector<Constructable, N> source;
    for (std::size_t i{0}; i < source_size; ++i) {
      source.push_back(Constructable{static_cast<int>(i + 1)});
    }
    const Constructable* source_data = source.data();
    const bool source_was_inline = source_size <= N;

    SmallVector<Constructable, N> target;
    for (std::size_t i{0}; i < target_size; ++i) {
      target.push_back(Constructable{static_cast<int>(100 + i)});
    }
    target = std::move(source);

    REQUIRE(target.size() == source_size);
    for (std::size_t i{0}; i < source_size; ++i) {
      REQUIRE(target[i].getValue() == static_cast<int>(i + 1));
    }
    REQUIRE((target.data() == source_data) == expect_heap_steal);
    REQUIRE(source.empty());
    REQUIRE((source.data() == source_data) == source_was_inline);
  };

  const std::size_t small_size = 1;
  const std::size_t big_source_size = N + 2;
  const std::size_t big_target_size = N + 3;

  check_case(small_size, small_size, false);
  check_case(small_size, big_target_size, false);
  check_case(big_source_size, small_size, true);
  check_case(big_source_size, big_target_size, true);
}

// Adapted from LLVM MoveAssignTest.
// Expanded into a matrix over source/target inline and heap states.
TEST_CASE("SmallVectorTest::MoveAssignTest") {
  RUN_TEST_CAPACITY(test_move_assignment_matrix);
}

void test_move_assignment_balances_ctor_dtor() {
  Constructable::reset();
  SmallVector<Constructable, 4> v;
  SmallVector<Constructable, 4> u;
  v.reserve(4);
  v.push_back(Constructable{1});
  u.push_back(Constructable{2});
  u.push_back(Constructable{3});

  v = std::move(u);
  EXPECT_VALUES(v, 2, 3);

  // Mirrors LLVM MoveAssignTest: after clearing the moved-from container, two
  // live elements remain in the destination.
  u.clear();
  REQUIRE(Constructable::numConstructorCalls - std::size_t{2} ==
          Constructable::numDestructorCalls);

  v.clear();
  REQUIRE(Constructable::numConstructorCalls ==
          Constructable::numDestructorCalls);
}

// Adapted from LLVM MoveAssignTest.
// Mirrors the constructor/destructor accounting after a move-assign cycle.
TEST_CASE("SmallVectorTest::MoveAssignBalancesCtorDtor") {
  test_move_assignment_balances_ctor_dtor();
}

template <std::size_t VN, std::size_t UN>
void test_dual_move_assignment() {
  Constructable::reset();
  SmallVector<Constructable, VN> vector;
  SmallVector<Constructable, UN> other_vector;
  auto& V = vector;
  auto& U = other_vector;
  for (unsigned I = 0; I < 4; ++I) {
    U.push_back(Constructable{static_cast<int>(I)});
  }

  const Constructable* OrigDataPtr = U.data();

  V = std::move(U);

  EXPECT_VALUES(V, 0, 1, 2, 3);

  U.clear();
  REQUIRE(Constructable::numConstructorCalls - std::size_t{4} ==
          Constructable::numDestructorCalls);

  // If the source vector was in small mode, the data pointer cannot be stolen.
  REQUIRE((UN >= 4 || V.data() == OrigDataPtr));

  V.clear();
  REQUIRE(Constructable::numConstructorCalls ==
          Constructable::numDestructorCalls);

  REQUIRE(Constructable::numCopyConstructorCalls == 0);
}

// Adapted from LLVM DualSmallVectorsTest::MoveAssignment.
// Mirrors the LLVM type-pair instantiations across distinct SBO capacities.
TEST_CASE("SmallVectorTest::MoveAssignAcrossN") {
  // Small mode -> Small mode.
  test_dual_move_assignment<4, 4>();
  // Small mode -> Big mode.
  test_dual_move_assignment<4, 2>();
  // Big mode -> Small mode.
  test_dual_move_assignment<2, 4>();
  // Big mode -> Big mode.
  test_dual_move_assignment<2, 2>();
}

template <std::size_t SrcN, std::size_t DstN>
void test_move_assignment_across_n_accounting() {
  Constructable::reset();
  SmallVector<Constructable, SrcN> u;
  for (int i{0}; i < 4; ++i) {
    u.push_back(Constructable{i});
  }

  SmallVector<Constructable, DstN> v;
  v = std::move(u);

  REQUIRE(v.size() == std::size_t{4});

  // Mirrors LLVM DualSmallVectorsTest::MoveAssignment: clearing the moved-from
  // container leaves the destination with the four elements alive.
  u.clear();
  REQUIRE(Constructable::numConstructorCalls - std::size_t{4} ==
          Constructable::numDestructorCalls);

  v.clear();
  REQUIRE(Constructable::numConstructorCalls ==
          Constructable::numDestructorCalls);
  // Move-assign across SBO capacities must never copy elements.
  REQUIRE(Constructable::numCopyConstructorCalls == 0);
}

// Adapted from LLVM DualSmallVectorsTest::MoveAssignment.
// Mirrors the ctor/dtor accounting and zero-copy invariant.
TEST_CASE("SmallVectorTest::MoveAssignAcrossNAccounting") {
  test_move_assignment_across_n_accounting<4, 4>();
  test_move_assignment_across_n_accounting<4, 2>();
  test_move_assignment_across_n_accounting<2, 4>();
  test_move_assignment_across_n_accounting<2, 2>();
}

template <std::size_t N>
void test_const_accessors() {
  Constructable::reset();
  SmallVector<Constructable, N> vector;

  // Mirrors LLVM ConstVectorTest: a default-constructed const vector reports
  // empty state and yields begin() == end().
  const SmallVector<Constructable, N> empty_const_vector;
  REQUIRE(empty_const_vector.size() == std::size_t{0});
  REQUIRE(empty_const_vector.empty());
  REQUIRE(empty_const_vector.begin() == empty_const_vector.end());

  make_sequence(vector, 1, 2);
  const SmallVector<Constructable, N>& const_vector = vector;

  REQUIRE(const_vector.size() == std::size_t{2});
  REQUIRE_FALSE(const_vector.empty());
  REQUIRE(const_vector.data() != nullptr);
  REQUIRE(const_vector[0].getValue() == 1);
  REQUIRE(const_vector[1].getValue() == 2);
  REQUIRE(&const_vector[0] == const_vector.data());
  REQUIRE(&const_vector[1] == const_vector.data() + 1);
}

// Adapted from LLVM ConstVectorTest.
// dxu adds const data/operator[] checks.
TEST_CASE("SmallVectorTest::ConstVectorTest") {
  RUN_TEST_CAPACITY(test_const_accessors);
}

template <std::size_t N>
void test_direct_vector_access() {
  Constructable::reset();
  SmallVector<Constructable, N> vector;

  REQUIRE(vector.size() == std::size_t{0});
  vector.reserve(4);
  REQUIRE(vector.capacity() >= std::size_t{4});
  REQUIRE(Constructable::numConstructorCalls == 0);

  make_sequence(vector, 1, 4);
  EXPECT_VALUES(vector, 1, 2, 3, 4);
  // LLVM DirectVectorTest: 4 temporaries + 4 in-place moves = 8 ctor calls.
  REQUIRE(Constructable::numConstructorCalls == 8);
}

// Adapted from LLVM DirectVectorTest.
TEST_CASE("SmallVectorTest::DirectVectorTest") {
  RUN_TEST_CAPACITY(test_direct_vector_access);
}

void test_emplace_back_forwarding() {
  EmplaceableArg<0> a0{true};
  EmplaceableArg<1> a1{true};
  EmplaceableArg<2> a2{true};
  EmplaceableArg<3> a3{true};

  {
    SmallVector<Emplaceable, 3> vector;
    Emplaceable& back = vector.emplace_back();
    REQUIRE(&back == &vector.back());
    REQUIRE(vector.size() == std::size_t{1});
    REQUIRE(back.state == EmplaceableState::Emplaced);
    REQUIRE(back.a0.state == EmplaceableArgState::Defaulted);
    REQUIRE(back.a1.state == EmplaceableArgState::Defaulted);
    REQUIRE(back.a2.state == EmplaceableArgState::Defaulted);
    REQUIRE(back.a3.state == EmplaceableArgState::Defaulted);
  }

  {
    SmallVector<Emplaceable, 3> vector;
    Emplaceable& back = vector.emplace_back(std::move(a0));
    REQUIRE(&back == &vector.back());
    REQUIRE(vector.size() == std::size_t{1});
    REQUIRE(back.state == EmplaceableState::Emplaced);
    REQUIRE(back.a0.state == EmplaceableArgState::RValue);
    REQUIRE(back.a1.state == EmplaceableArgState::Defaulted);
    REQUIRE(back.a2.state == EmplaceableArgState::Defaulted);
    REQUIRE(back.a3.state == EmplaceableArgState::Defaulted);
  }

  {
    SmallVector<Emplaceable, 3> vector;
    Emplaceable& back = vector.emplace_back(a0);
    REQUIRE(&back == &vector.back());
    REQUIRE(vector.size() == std::size_t{1});
    REQUIRE(back.state == EmplaceableState::Emplaced);
    REQUIRE(back.a0.state == EmplaceableArgState::LValue);
    REQUIRE(back.a1.state == EmplaceableArgState::Defaulted);
    REQUIRE(back.a2.state == EmplaceableArgState::Defaulted);
    REQUIRE(back.a3.state == EmplaceableArgState::Defaulted);
  }

  {
    SmallVector<Emplaceable, 3> vector;
    Emplaceable& back = vector.emplace_back(a0, a1);
    REQUIRE(&back == &vector.back());
    REQUIRE(vector.size() == std::size_t{1});
    REQUIRE(back.state == EmplaceableState::Emplaced);
    REQUIRE(back.a0.state == EmplaceableArgState::LValue);
    REQUIRE(back.a1.state == EmplaceableArgState::LValue);
    REQUIRE(back.a2.state == EmplaceableArgState::Defaulted);
    REQUIRE(back.a3.state == EmplaceableArgState::Defaulted);
  }

  {
    SmallVector<Emplaceable, 3> vector;
    Emplaceable& back = vector.emplace_back(std::move(a0), std::move(a1));
    REQUIRE(&back == &vector.back());
    REQUIRE(vector.size() == std::size_t{1});
    REQUIRE(back.state == EmplaceableState::Emplaced);
    REQUIRE(back.a0.state == EmplaceableArgState::RValue);
    REQUIRE(back.a1.state == EmplaceableArgState::RValue);
    REQUIRE(back.a2.state == EmplaceableArgState::Defaulted);
    REQUIRE(back.a3.state == EmplaceableArgState::Defaulted);
  }

  {
    SmallVector<Emplaceable, 3> vector;
    Emplaceable& back =
        vector.emplace_back(std::move(a0), a1, std::move(a2), a3);
    REQUIRE(&back == &vector.back());
    REQUIRE(vector.size() == std::size_t{1});
    REQUIRE(back.state == EmplaceableState::Emplaced);
    REQUIRE(back.a0.state == EmplaceableArgState::RValue);
    REQUIRE(back.a1.state == EmplaceableArgState::LValue);
    REQUIRE(back.a2.state == EmplaceableArgState::RValue);
    REQUIRE(back.a3.state == EmplaceableArgState::LValue);
  }
}

void test_emplace_back_int() {
  SmallVector<int, 1> vector;

  vector.emplace_back();
  vector.emplace_back(42);

  REQUIRE(vector.size() == std::size_t{2});
  REQUIRE(vector[0] == 0);
  REQUIRE(vector[1] == 42);
  REQUIRE(&vector[0] == vector.data());
  REQUIRE(&vector[1] == vector.data() + 1);
}

// Adapted from LLVM EmplaceBack.
// Rewritten for Catch2 and the reduced dxu::SmallVector API.
TEST_CASE("SmallVectorTest::EmplaceBack") {
  test_emplace_back_forwarding();
  test_emplace_back_int();
}

void test_default_inlined_elements() {
  SmallVector<int> vector;
  REQUIRE(vector.empty());
  REQUIRE(vector.capacity() == std::size_t{16});
  vector.push_back(7);
  REQUIRE(vector[0] == 7);

  SmallVector<SmallVector<SmallVector<int>>> nested_vector;
  nested_vector.emplace_back().emplace_back().emplace_back(42);
  REQUIRE(nested_vector[0][0][0] == 42);
}

// Adapted from LLVM DefaultInlinedElements.
// dxu has fixed default inline capacity 16, so the capacity is asserted
// explicitly.
TEST_CASE("SmallVectorTest::DefaultInlinedElements") {
  test_default_inlined_elements();
}

void test_initializer_list_constructor() {
  SmallVector<int, 2> empty{};
  REQUIRE(empty.empty());
  REQUIRE(empty.capacity() == std::size_t{2});

  SmallVector<int, 4> inline_vector{1, 2, 3};
  REQUIRE(inline_vector.size() == std::size_t{3});
  REQUIRE(inline_vector.capacity() == std::size_t{4});
  REQUIRE(inline_vector[0] == 1);
  REQUIRE(inline_vector[1] == 2);
  REQUIRE(inline_vector[2] == 3);

  SmallVector<int, 2> heap_vector{4, 5, 6, 7};
  REQUIRE(heap_vector.size() == std::size_t{4});
  REQUIRE(heap_vector.capacity() >= std::size_t{4});
  REQUIRE(heap_vector[0] == 4);
  REQUIRE(heap_vector[1] == 5);
  REQUIRE(heap_vector[2] == 6);
  REQUIRE(heap_vector[3] == 7);
}

// Covers the initializer-list constructor across empty/inline/heap capacities.
// Initializer-list assignment (operator=) is covered by the InitializerList
// port.
TEST_CASE("SmallVectorTest::InitializerListConstructor") {
  test_initializer_list_constructor();
}

void test_initializer_list() {
  SmallVector<int, 2> V1 = {};
  REQUIRE(V1.empty());
  V1 = {0, 0};
  REQUIRE(V1.size() == std::size_t{2});
  REQUIRE(V1[0] == 0);
  REQUIRE(V1[1] == 0);
  V1 = {-1, -1};
  REQUIRE(V1.size() == std::size_t{2});
  REQUIRE(V1[0] == -1);
  REQUIRE(V1[1] == -1);

  SmallVector<int, 2> V2 = {1, 2, 3, 4};
  REQUIRE(V2.size() == std::size_t{4});
  REQUIRE(V2[0] == 1);
  REQUIRE(V2[1] == 2);
  REQUIRE(V2[2] == 3);
  REQUIRE(V2[3] == 4);
  V2.assign({4});
  REQUIRE(V2.size() == std::size_t{1});
  REQUIRE(V2[0] == 4);
}

// Adapted from LLVM InitializerList.
// dxu::SmallVector has no append/insert, so those trailing steps are omitted.
TEST_CASE("SmallVectorTest::InitializerList") { test_initializer_list(); }

void test_count_value_constructor() {
  SmallVector<int, 2> empty(0, 7);
  REQUIRE(empty.empty());
  REQUIRE(empty.capacity() == std::size_t{2});

  SmallVector<int, 4> inline_vector(3, 8);
  REQUIRE(inline_vector.size() == std::size_t{3});
  REQUIRE(inline_vector.capacity() == std::size_t{4});
  REQUIRE(inline_vector[0] == 8);
  REQUIRE(inline_vector[1] == 8);
  REQUIRE(inline_vector[2] == 8);

  SmallVector<int, 2> heap_vector(4, 9);
  REQUIRE(heap_vector.size() == std::size_t{4});
  REQUIRE(heap_vector.capacity() >= std::size_t{4});
  for (const auto& value : heap_vector) {
    REQUIRE(value == 9);
  }
}

// Adapted from LLVM ConstructorNonIterTest; also covers the empty-count case.
TEST_CASE("SmallVectorTest::CountValueConstructor") {
  test_count_value_constructor();
}

template <std::size_t N>
void test_size_constructor() {
  SmallVector<Constructable, N> empty(0);
  REQUIRE(empty.empty());
  REQUIRE(empty.capacity() >= N);
  REQUIRE(empty.data() != nullptr);

  SmallVector<Constructable, N> inline_vector(N);
  REQUIRE(inline_vector.size() == N);
  REQUIRE(inline_vector.capacity() >= N);
  for (std::size_t i{0}; i < inline_vector.size(); ++i) {
    REQUIRE(inline_vector[i].getValue() == 0);
    REQUIRE(&inline_vector[i] == inline_vector.data() + i);
  }

  const std::size_t heap_size = N + 3;
  SmallVector<Constructable, N> heap_vector(heap_size);
  REQUIRE(heap_vector.size() == heap_size);
  REQUIRE(heap_vector.capacity() >= heap_size);
  for (std::size_t i{0}; i < heap_vector.size(); ++i) {
    REQUIRE(heap_vector[i].getValue() == 0);
    REQUIRE(&heap_vector[i] == heap_vector.data() + i);
  }
}

// Adapted from LLVM ConstructorNonIterTest with count-only form.
// Exercises the explicit SmallVector(size_type) constructor across capacities.
TEST_CASE("SmallVectorTest::SizeConstructor") {
  RUN_TEST_CAPACITY(test_size_constructor);
}

// Non-LLVM compatibility and coverage tests for dxu::SmallVector follow.

TEST_CASE("SmallVectorTest::TypeTraits") {
  STATIC_REQUIRE(std::is_same<SmallVector<int>::value_type, int>::value);
  STATIC_REQUIRE(std::is_same<SmallVector<int>::allocator_type,
                              std::allocator<int>>::value);
  STATIC_REQUIRE(std::is_same<SmallVector<int>::size_type, std::size_t>::value);
  STATIC_REQUIRE(
      std::is_same<SmallVector<int>::difference_type, std::ptrdiff_t>::value);
  STATIC_REQUIRE(std::is_same<SmallVector<int>::reference, int&>::value);
  STATIC_REQUIRE(
      std::is_same<SmallVector<int>::const_reference, const int&>::value);
  STATIC_REQUIRE(std::is_same<SmallVector<int>::pointer, int*>::value);
  STATIC_REQUIRE(
      std::is_same<SmallVector<int>::const_pointer, const int*>::value);
  STATIC_REQUIRE(std::is_same<SmallVector<int>::iterator, int*>::value);
  STATIC_REQUIRE(
      std::is_same<SmallVector<int>::const_iterator, const int*>::value);
  STATIC_REQUIRE(std::is_same<SmallVector<int>::reverse_iterator,
                              std::reverse_iterator<int*>>::value);
  STATIC_REQUIRE(std::is_same<SmallVector<int>::const_reverse_iterator,
                              std::reverse_iterator<const int*>>::value);
}

template <std::size_t N>
void test_at_accessors() {
  Constructable::reset();
  SmallVector<Constructable, N> vector;
  make_sequence(vector, 1, 2);

  REQUIRE(vector.at(0).getValue() == 1);
  REQUIRE(vector.at(1).getValue() == 2);
  REQUIRE(&vector.at(0) == &vector[0]);
  REQUIRE(&vector.at(1) == &vector[1]);

  const SmallVector<Constructable, N>& const_vector = vector;
  REQUIRE(const_vector.at(0).getValue() == 1);
  REQUIRE(const_vector.at(1).getValue() == 2);
  REQUIRE(&const_vector.at(0) == &const_vector[0]);
  REQUIRE(&const_vector.at(1) == &const_vector[1]);

  REQUIRE_THROWS_AS(vector.at(2), std::out_of_range);
  REQUIRE_THROWS_AS(const_vector.at(2), std::out_of_range);
}

TEST_CASE("SmallVectorTest::AtTest") { RUN_TEST_CAPACITY(test_at_accessors); }

template <std::size_t N>
void test_reserve_branches() {
  Constructable::reset();
  SmallVector<Constructable, N> vector;

  make_sequence(vector, 1, 2);
  const Constructable* before = vector.data();
  const std::size_t before_capacity = vector.capacity();

  vector.reserve(before_capacity);
  REQUIRE(vector.data() == before);
  REQUIRE(vector.capacity() == before_capacity);

  vector.reserve(before_capacity + 3);
  REQUIRE(vector.capacity() >= before_capacity + 3);
  REQUIRE(vector[0].getValue() == 1);
  REQUIRE(vector[1].getValue() == 2);

  const Constructable* after_grow = vector.data();
  const std::size_t after_grow_capacity = vector.capacity();
  vector.reserve(1);
  REQUIRE(vector.data() == after_grow);
  REQUIRE(vector.capacity() == after_grow_capacity);
}

template <std::size_t N>
void test_reserve_after_clear_on_heap_storage() {
  Constructable::reset();
  SmallVector<Constructable, N> vector;

  make_sequence(vector, 1, static_cast<int>(N) + 2);
  const Constructable* heap_data = vector.data();
  const std::size_t heap_capacity = vector.capacity();

  vector.clear();
  REQUIRE(vector.empty());
  REQUIRE(vector.data() == heap_data);
  REQUIRE(vector.capacity() == heap_capacity);

  vector.reserve(heap_capacity + 3);
  REQUIRE(vector.empty());
  REQUIRE(vector.data() != heap_data);
  REQUIRE(vector.capacity() >= heap_capacity + 3);

  vector.push_back(Constructable{42});
  EXPECT_VALUES(vector, 42);
}

TEST_CASE("SmallVectorTest::ReserveTest") {
  RUN_TEST_CAPACITY(test_reserve_branches);
  RUN_TEST_CAPACITY(test_reserve_after_clear_on_heap_storage);
}

void test_reserve_empty_heap_storage_relocates() {
  SmallVector<int, 1> vector;
  REQUIRE(vector.empty());

  vector.reserve(4);
  int* heap_data = vector.data();
  const std::size_t heap_capacity = vector.capacity();
  REQUIRE(vector.empty());
  REQUIRE(heap_capacity >= std::size_t{4});

  vector.reserve(heap_capacity + 3);
  REQUIRE(vector.empty());
  REQUIRE(vector.data() != heap_data);
  REQUIRE(vector.capacity() >= heap_capacity + 3);
}

TEST_CASE("SmallVectorTest::ReserveEmptyHeapStorageRelocates") {
  test_reserve_empty_heap_storage_relocates();
}

template <std::size_t N>
void test_resize_fill_shrink() {
  Constructable::reset();
  SmallVector<Constructable, N> vector;

  make_sequence(vector, 1, 3);
  vector.resize(1, Constructable{77});

  EXPECT_VALUES(vector, 1);
}

TEST_CASE("SmallVectorTest::ResizeFillShrinkTest") {
  RUN_TEST_CAPACITY(test_resize_fill_shrink);
}

void test_resize_fill_grows_non_empty_vector() {
  SmallVector<int, 2> vector{1, 2};

  vector.resize(4, 9);

  REQUIRE(vector.size() == std::size_t{4});
  REQUIRE(vector[0] == 1);
  REQUIRE(vector[1] == 2);
  REQUIRE(vector[2] == 9);
  REQUIRE(vector[3] == 9);
}

TEST_CASE("SmallVectorTest::ResizeFillGrowsNonEmptyVector") {
  test_resize_fill_grows_non_empty_vector();
}

template <std::size_t N>
void test_copy_constructor() {
  Constructable::reset();
  SmallVector<Constructable, N> inline_source;
  inline_source.push_back(Constructable{1});
  SmallVector<Constructable, N> inline_copy{inline_source};

  REQUIRE(inline_copy.size() == std::size_t{1});
  REQUIRE(inline_copy[0].getValue() == 1);
  REQUIRE(inline_copy.data() != inline_source.data());

  SmallVector<Constructable, N> heap_source;
  make_sequence(heap_source, 1, static_cast<int>(N) + 2);
  SmallVector<Constructable, N> heap_copy{heap_source};

  REQUIRE(heap_copy.size() == heap_source.size());
  REQUIRE(heap_copy.data() != heap_source.data());
  for (std::size_t i{0}; i < heap_copy.size(); ++i) {
    REQUIRE(heap_copy[i].getValue() == heap_source[i].getValue());
    REQUIRE(&heap_copy[i] == heap_copy.data() + i);
  }
}

TEST_CASE("SmallVectorTest::CopyConstructorTest") {
  RUN_TEST_CAPACITY(test_copy_constructor);
}

void test_copy_constructor_across_capacity() {
  SmallVector<Constructable, 1> source;
  make_sequence(source, 1, 3);
  const Constructable* source_data = source.data();

  SmallVector<Constructable, 4> copy{source};

  REQUIRE(copy.size() == source.size());
  REQUIRE(copy.data() != source_data);
  REQUIRE(source.data() == source_data);
  EXPECT_VALUES(copy, 1, 2, 3);
  EXPECT_VALUES(source, 1, 2, 3);
}

TEST_CASE("SmallVectorTest::CopyConstructorAcrossCapacity") {
  test_copy_constructor_across_capacity();
}

template <std::size_t N>
void test_assign_reallocate_for_copy_only_type() {
  SmallVector<CopyOnly, N> source;
  for (int value : {11, 13, 17}) {
    CopyOnly item{value};
    source.push_back(item);
  }

  SmallVector<CopyOnly, N> target;
  CopyOnly initial{99};
  target.push_back(initial);
  const CopyOnly* before = target.data();

  target.assign(source.begin(), source.end());

  REQUIRE(target.size() == std::size_t{3});
  REQUIRE(target[0].value == 11);
  REQUIRE(target[1].value == 13);
  REQUIRE(target[2].value == 17);
  if (N < 3) {
    REQUIRE(target.data() != before);
  }
}

template <std::size_t N>
void test_assign_reuses_existing_capacity() {
  SmallVector<CopyOnly, N> source;
  for (int value : {21, 34}) {
    CopyOnly item{value};
    source.push_back(item);
  }

  SmallVector<CopyOnly, N> target;
  target.reserve(N + 4);
  CopyOnly existing_first{1};
  CopyOnly existing_second{2};
  target.push_back(existing_first);
  target.push_back(existing_second);
  const CopyOnly* before = target.data();
  const std::size_t before_capacity = target.capacity();

  target.assign(source.begin(), source.end());

  REQUIRE(target.data() == before);
  REQUIRE(target.capacity() == before_capacity);
  REQUIRE(target.size() == std::size_t{2});
  REQUIRE(target[0].value == 21);
  REQUIRE(target[1].value == 34);
}

template <std::size_t N>
void test_assign_empty_source() {
  SmallVector<Constructable, N> source;
  SmallVector<Constructable, N> target;
  make_sequence(target, 1, 3);
  const Constructable* before = target.data();
  const std::size_t before_capacity = target.capacity();

  target.assign(source.begin(), source.end());

  EXPECT_EMPTY(target);
  REQUIRE(target.data() == before);
  REQUIRE(target.capacity() == before_capacity);
}

template <std::size_t N>
void test_assign_for_capacity() {
  test_assign_reallocate_for_copy_only_type<N>();
  test_assign_reuses_existing_capacity<N>();
  test_assign_empty_source<N>();
}

TEST_CASE("SmallVectorTest::AssignForMultipleN") {
  RUN_TEST_CAPACITY(test_assign_for_capacity);
}

void test_assign_self() {
  SmallVector<Constructable, 1> vector;
  make_sequence(vector, 1, 3);
  const Constructable* before = vector.data();
  const std::size_t before_capacity = vector.capacity();

  vector.assign(vector);
  REQUIRE(vector.data() == before);
  REQUIRE(vector.capacity() == before_capacity);
  EXPECT_VALUES(vector, 1, 2, 3);

  vector.operator=(vector);
  REQUIRE(vector.data() == before);
  REQUIRE(vector.capacity() == before_capacity);
  EXPECT_VALUES(vector, 1, 2, 3);
}

TEST_CASE("SmallVectorTest::AssignSelf") { test_assign_self(); }

void test_copy_assignment_same_capacity() {
  SmallVector<int, 2> source{1, 2, 3};
  const int* source_data = source.data();

  SmallVector<int, 2> target;
  target = source;

  REQUIRE(target.size() == std::size_t{3});
  REQUIRE(target[0] == 1);
  REQUIRE(target[1] == 2);
  REQUIRE(target[2] == 3);
  REQUIRE(target.data() != source_data);
  REQUIRE(source.size() == std::size_t{3});
  REQUIRE(source.data() == source_data);
}

void test_move_assignment_same_capacity() {
  SmallVector<Constructable, 2> source;
  make_sequence(source, 1, 3);
  const Constructable* source_data = source.data();

  SmallVector<Constructable, 2> target;
  target = std::move(source);

  REQUIRE(target.size() == std::size_t{3});
  REQUIRE(target.data() == source_data);
  EXPECT_VALUES(target, 1, 2, 3);
  REQUIRE(source.empty());
  REQUIRE(source.data() != source_data);
  REQUIRE(source.capacity() == std::size_t{2});
}

TEST_CASE("SmallVectorTest::SameCapacityAssignmentOperators") {
  test_copy_assignment_same_capacity();
  test_move_assignment_same_capacity();
}

void test_assign_move_from_inline_storage_across_capacity() {
  SmallVector<Constructable, 1> source;
  source.push_back(Constructable{1});
  const Constructable* source_data = source.data();

  SmallVector<Constructable, 4> target;
  make_sequence(target, 7, 8);
  target.assign(std::move(source));

  REQUIRE(target.size() == std::size_t{1});
  REQUIRE(target[0].getValue() == 1);
  REQUIRE(target.data() != source_data);
  REQUIRE(source.empty());
  REQUIRE(source.data() == source_data);
  REQUIRE(source.capacity() == std::size_t{1});
}

void test_assign_move_steals_heap_storage_across_capacity() {
  SmallVector<Constructable, 1> source;
  make_sequence(source, 1, 4);
  const Constructable* source_data = source.data();

  SmallVector<Constructable, 4> target;
  make_sequence(target, 7, 12);
  target.assign(std::move(source));

  REQUIRE(target.size() == std::size_t{4});
  for (std::size_t i{0}; i < target.size(); ++i) {
    REQUIRE(target[i].getValue() == static_cast<int>(i + 1));
  }
  REQUIRE(target.data() == source_data);
  REQUIRE(source.empty());
  REQUIRE(source.data() != source_data);
  REQUIRE(source.capacity() == std::size_t{1});
}

void test_assign_move_self() {
  SmallVector<Constructable, 1> vector;
  make_sequence(vector, 1, 3);
  const Constructable* before = vector.data();
  const std::size_t before_capacity = vector.capacity();

  vector.assign(std::move(vector));

  REQUIRE(vector.data() == before);
  REQUIRE(vector.capacity() == before_capacity);
  EXPECT_VALUES(vector, 1, 2, 3);
}

TEST_CASE("SmallVectorTest::AssignMove") {
  test_assign_move_from_inline_storage_across_capacity();
  test_assign_move_steals_heap_storage_across_capacity();
  test_assign_move_self();
}

template <std::size_t N>
void test_move_constructor_from_inline_storage() {
  SmallVector<Constructable, N> source;
  const std::size_t inline_size = N < 2 ? 1 : 2;
  for (std::size_t i{0}; i < inline_size; ++i) {
    source.push_back(Constructable{static_cast<int>(i + 1)});
  }
  const Constructable* source_data = source.data();

  SmallVector<Constructable, N> moved{std::move(source)};

  REQUIRE(moved.size() == inline_size);
  for (std::size_t i{0}; i < inline_size; ++i) {
    REQUIRE(moved[i].getValue() == static_cast<int>(i + 1));
  }
  REQUIRE(moved.data() != source_data);
  REQUIRE(source.empty());
  REQUIRE(source.data() == source_data);
}

template <std::size_t N>
void test_move_constructor_steals_heap_storage() {
  SmallVector<Constructable, N> source;
  for (std::size_t i{0}; i < N + 3; ++i) {
    source.push_back(Constructable{static_cast<int>(i + 1)});
  }
  const Constructable* source_data = source.data();

  SmallVector<Constructable, N> moved{std::move(source)};

  REQUIRE(moved.data() == source_data);
  REQUIRE(moved.size() == N + 3);
  for (std::size_t i{0}; i < moved.size(); ++i) {
    REQUIRE(moved[i].getValue() == static_cast<int>(i + 1));
  }
  REQUIRE(source.empty());
  REQUIRE(source.data() != source_data);
}

TEST_CASE("SmallVectorTest::MoveConstructorTest") {
  RUN_TEST_CAPACITY(test_move_constructor_from_inline_storage);
  RUN_TEST_CAPACITY(test_move_constructor_steals_heap_storage);
}

void test_move_constructor_from_inline_storage_across_capacity() {
  SmallVector<Constructable, 1> source;
  source.push_back(Constructable{1});
  const Constructable* source_data = source.data();

  SmallVector<Constructable, 4> moved{std::move(source)};

  REQUIRE(moved.size() == std::size_t{1});
  REQUIRE(moved[0].getValue() == 1);
  REQUIRE(moved.data() != source_data);
  REQUIRE(source.empty());
  REQUIRE(source.data() == source_data);
  REQUIRE(source.capacity() == std::size_t{1});
}

void test_move_constructor_steals_heap_storage_across_capacity() {
  SmallVector<Constructable, 1> source;
  make_sequence(source, 1, 4);
  const Constructable* source_data = source.data();

  SmallVector<Constructable, 4> moved{std::move(source)};

  REQUIRE(moved.size() == std::size_t{4});
  for (std::size_t i{0}; i < moved.size(); ++i) {
    REQUIRE(moved[i].getValue() == static_cast<int>(i + 1));
  }
  REQUIRE(moved.data() == source_data);
  REQUIRE(source.empty());
  REQUIRE(source.data() != source_data);
  REQUIRE(source.capacity() == std::size_t{1});
}

TEST_CASE("SmallVectorTest::MoveConstructorAcrossCapacity") {
  test_move_constructor_from_inline_storage_across_capacity();
  test_move_constructor_steals_heap_storage_across_capacity();
}

void test_move_assignment_from_inline_storage_across_capacity() {
  SmallVector<Constructable, 1> source;
  source.push_back(Constructable{1});
  const Constructable* source_data = source.data();

  SmallVector<Constructable, 4> target;
  make_sequence(target, 7, 8);
  target = std::move(source);

  REQUIRE(target.size() == std::size_t{1});
  REQUIRE(target[0].getValue() == 1);
  REQUIRE(target.data() != source_data);
  REQUIRE(source.empty());
  REQUIRE(source.data() == source_data);
  REQUIRE(source.capacity() == std::size_t{1});
}

void test_move_assignment_steals_heap_storage_across_capacity() {
  SmallVector<Constructable, 1> source;
  make_sequence(source, 1, 4);
  const Constructable* source_data = source.data();

  SmallVector<Constructable, 4> target;
  make_sequence(target, 7, 12);
  target = std::move(source);

  REQUIRE(target.size() == std::size_t{4});
  for (std::size_t i{0}; i < target.size(); ++i) {
    REQUIRE(target[i].getValue() == static_cast<int>(i + 1));
  }
  REQUIRE(target.data() == source_data);
  REQUIRE(source.empty());
  REQUIRE(source.data() != source_data);
  REQUIRE(source.capacity() == std::size_t{1});
}

TEST_CASE("SmallVectorTest::MoveAssignmentAcrossCapacity") {
  test_move_assignment_from_inline_storage_across_capacity();
  test_move_assignment_steals_heap_storage_across_capacity();
}

template <std::size_t N>
void test_swap_small_and_big() {
  SmallVector<Constructable, N> small;
  make_sequence(small, 1, 2);

  SmallVector<Constructable, N> big;
  for (std::size_t i{0}; i < N + 3; ++i) {
    big.push_back(Constructable{static_cast<int>(7 + i)});
  }
  const Constructable* big_data = big.data();

  small.swap(big);

  REQUIRE(small.size() == N + 3);
  for (std::size_t i{0}; i < small.size(); ++i) {
    REQUIRE(small[i].getValue() == static_cast<int>(7 + i));
  }
  REQUIRE(small.data() == big_data);

  EXPECT_VALUES(big, 1, 2);
}

void test_swap_self() {
  SmallVector<Constructable, 1> vector;
  make_sequence(vector, 1, 3);
  const Constructable* before = vector.data();

  vector.swap(vector);

  REQUIRE(vector.data() == before);
  EXPECT_VALUES(vector, 1, 2, 3);
}

TEST_CASE("SmallVectorTest::SwapAdditionalCases") {
  test_swap_small_and_big<2>();
  test_swap_small_and_big<100>();
  test_swap_self();
}

void test_large_inline_capacity_overflow() {
  SmallVector<int, 100> vector;
  const auto inline_begin = reinterpret_cast<std::uintptr_t>(vector.data());
  const auto object_begin = reinterpret_cast<std::uintptr_t>(&vector);
  const auto object_end = object_begin + sizeof(vector);

  REQUIRE(inline_begin >= object_begin);
  REQUIRE(inline_begin < object_end);

  for (int i{0}; i < 100; ++i) {
    vector.push_back(i);
  }
  REQUIRE(reinterpret_cast<std::uintptr_t>(vector.data()) == inline_begin);

  vector.push_back(100);
  REQUIRE(reinterpret_cast<std::uintptr_t>(vector.data()) != inline_begin);
  REQUIRE_FALSE(
      (reinterpret_cast<std::uintptr_t>(vector.data()) >= object_begin &&
       reinterpret_cast<std::uintptr_t>(vector.data()) < object_end));
  REQUIRE(vector.size() == std::size_t{101});
  REQUIRE(vector[0] == 0);
  REQUIRE(vector[100] == 100);
}

TEST_CASE("SmallVectorTest::LargeInlineCapacityOverflow") {
  test_large_inline_capacity_overflow();
}

void test_object_layout() {
  SmallVector<int, 4> vector;
  const auto object_begin{reinterpret_cast<std::uintptr_t>(&vector)};
  const auto object_end{object_begin + sizeof(vector)};
  const auto inline_begin{reinterpret_cast<std::uintptr_t>(vector.data())};

  REQUIRE(inline_begin >= object_begin);
  REQUIRE(inline_begin < object_end);
  REQUIRE(inline_begin >=
          object_begin + sizeof(void*) + 2 * sizeof(std::size_t));
  REQUIRE(vector.capacity() == std::size_t{4});

  vector.resize(4, 7);
  REQUIRE(reinterpret_cast<std::uintptr_t>(vector.data()) == inline_begin);

  vector.push_back(8);
  const auto heap_begin{reinterpret_cast<std::uintptr_t>(vector.data())};
  REQUIRE_FALSE((heap_begin >= object_begin && heap_begin < object_end));
  REQUIRE(vector.size() == std::size_t{5});
  REQUIRE(vector[0] == 7);
  REQUIRE(vector[4] == 8);
}

TEST_CASE("SmallVectorTest::ObjectLayout") { test_object_layout(); }

}  // namespace DXU_NAMESPACE
