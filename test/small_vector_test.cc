#include "dxu/small_vector.h"

#include "test_common.h"

namespace DXU_NAMESPACE {

/** A helper class that counts the total number of constructor and destructor
 * calls. */
class Constructable {
 public:
  Constructable() : constructed_{true}, value_{0} { ++num_constructor_calls; }

  explicit Constructable(int value) : constructed_{true}, value_{value} {
    ++num_constructor_calls;
  }

  Constructable(const Constructable& other)
      : constructed_{true}, value_{other.value_} {
    CHECK(other.constructed_);
    ++num_constructor_calls;
    ++num_copy_constructor_calls;
  }

  Constructable(Constructable&& other) noexcept
      : constructed_{true}, value_{other.value_} {
    CHECK(other.constructed_);
    other.value_ = 0;
    ++num_constructor_calls;
    ++num_move_constructor_calls;
  }

  Constructable& operator=(const Constructable& other) {
    CHECK(constructed_);
    CHECK(other.constructed_);
    value_ = other.value_;
    ++num_assignment_calls;
    ++num_copy_assignment_calls;
    return *this;
  }

  Constructable& operator=(Constructable&& other) noexcept {
    CHECK(constructed_);
    CHECK(other.constructed_);
    value_ = other.value_;
    other.value_ = 0;
    ++num_assignment_calls;
    ++num_move_assignment_calls;
    return *this;
  }

  ~Constructable() {
    CHECK(constructed_);
    constructed_ = false;
    ++num_destructor_calls;
  }

  friend bool operator==(const Constructable& lhs, const Constructable& rhs) {
    return lhs.get_value() == rhs.get_value();
  }

  int get_value() const { return std::abs(value_); }

  static void reset() {
    num_constructor_calls = 0;
    num_copy_constructor_calls = 0;
    num_move_constructor_calls = 0;
    num_destructor_calls = 0;
    num_assignment_calls = 0;
    num_copy_assignment_calls = 0;
    num_move_assignment_calls = 0;
  }

  static int num_constructor_calls;
  static int num_copy_constructor_calls;
  static int num_move_constructor_calls;
  static int num_destructor_calls;
  static int num_assignment_calls;
  static int num_copy_assignment_calls;
  static int num_move_assignment_calls;

 private:
  bool constructed_;
  int value_;
};

int Constructable::num_constructor_calls;
int Constructable::num_copy_constructor_calls;
int Constructable::num_move_constructor_calls;
int Constructable::num_destructor_calls;
int Constructable::num_assignment_calls;
int Constructable::num_copy_assignment_calls;
int Constructable::num_move_assignment_calls;

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

struct NonCopyable {
  NonCopyable() = default;
  NonCopyable(NonCopyable&&) noexcept {}
  NonCopyable& operator=(NonCopyable&&) noexcept { return *this; }

 private:
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
};

class InputIterator {
 public:
  using iterator_category = std::input_iterator_tag;
  using value_type = int;
  using difference_type = std::ptrdiff_t;
  using pointer = const int*;
  using reference = const int&;

  explicit InputIterator(const int* ptr) : ptr_{ptr} {}

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
  const int* ptr_;
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

template <typename Vector>
void expect_empty(const Vector& vector) {
  REQUIRE((vector.size()) == (std::size_t{0}));
  REQUIRE(vector.empty());
}

template <typename Vector, typename... Values>
void expect_values(const Vector& vector, Values... values) {
  const int expected[]{values...};
  constexpr std::size_t expected_size{sizeof...(Values)};

  REQUIRE((vector.size()) == (expected_size));
  for (std::size_t i{0}; i < expected_size; ++i) {
    REQUIRE((vector[i].get_value()) == (expected[i]));
    REQUIRE(&vector[i] == vector.data() + i);
  }
}

template <typename Vector>
void make_sequence(Vector& vector, int first, int last) {
  for (int value{first}; value <= last; ++value) {
    vector.push_back(Constructable{value});
  }
}

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
  SmallVector<NonCopyable, N> vector;
  vector.resize(42);
  REQUIRE(vector.size() == std::size_t{42});
  REQUIRE(vector.data() != nullptr);
}

// Adapted from LLVM ConstructNonCopyableTest.
// dxu::SmallVector requires N > 0 and has no size constructor, so this uses
// N=1 plus resize().
TEST_CASE("SmallVectorTest::ConstructNonCopyableTest") {
  test_construct_non_copyable<1>();
}

template <std::size_t N>
void test_empty_vector() {
  Constructable::reset();
  SmallVector<Constructable, N> vector;
  expect_empty(vector);
  REQUIRE(vector.data() != nullptr);
  REQUIRE(Constructable::num_constructor_calls == 0);
  REQUIRE(Constructable::num_destructor_calls == 0);
}

// Adapted from LLVM EmptyVectorTest.
// Converted from typed gtest to explicit dxu capacities.
TEST_CASE("SmallVectorTest::EmptyVectorTest") {
  RUN_TEST_CAPACITY(test_empty_vector);
}

template <std::size_t N>
void test_push_back() {
  Constructable::reset();
  SmallVector<Constructable, N> vector;

  vector.push_back(Constructable{1});
  expect_values(vector, 1);
  REQUIRE_FALSE(vector.empty());

  vector.push_back(Constructable{2});
  expect_values(vector, 1, 2);

  vector.emplace_back(3);
  expect_values(vector, 1, 2, 3);
}

// Adapted from LLVM PushPopTest.
// dxu::SmallVector has no insert/pop APIs, so this covers push_back and
// emplace_back only.
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

  expect_empty(vector);
  REQUIRE(Constructable::num_constructor_calls == 4);
  REQUIRE(Constructable::num_destructor_calls == 4);
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

  expect_values(vector, 1);
  REQUIRE(Constructable::num_constructor_calls == 6);
  REQUIRE(Constructable::num_destructor_calls == 5);
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

  REQUIRE(Constructable::num_constructor_calls == 2);
  REQUIRE(Constructable::num_destructor_calls == 0);
  REQUIRE(vector.size() == std::size_t{2});
  REQUIRE(vector[0].get_value() == 0);
  REQUIRE(vector[1].get_value() == 0);
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
  const std::size_t ctors = Constructable::num_constructor_calls;
  REQUIRE((ctors == 2 || ctors == 4));
  const std::size_t move_ctors = Constructable::num_move_constructor_calls;
  REQUIRE((move_ctors == 0 || move_ctors == 2));
  const std::size_t dtors = Constructable::num_destructor_calls;
  REQUIRE((dtors == 0 || dtors == 2));

  REQUIRE(vector.size() == std::size_t{4});
  REQUIRE(vector[0].get_value() == 0);
  REQUIRE(vector[1].get_value() == 0);
  REQUIRE(vector[2].get_value() == 0);
  REQUIRE(vector[3].get_value() == 0);
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
  expect_values(vector, 77, 77, 77);
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
    REQUIRE(vector[i].get_value() == static_cast<int>(i + 1));
    REQUIRE(&vector[i] == vector.data() + i);
  }

  vector.resize(1);
  expect_values(vector, 1);
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
  REQUIRE(vector.front().get_value() == 1);
  REQUIRE(vector.back() == vector[1]);
  REQUIRE(vector.back().get_value() == 2);

  const SmallVector<Constructable, N>& const_vector = vector;
  REQUIRE(const_vector.front() == const_vector[0]);
  REQUIRE(const_vector.front().get_value() == 1);
  REQUIRE(const_vector.back() == const_vector[1]);
  REQUIRE(const_vector.back().get_value() == 2);
}

template <std::size_t N>
void test_begin_end_iteration() {
  SmallVector<Constructable, N> vector;
  REQUIRE(vector.begin() == vector.end());

  make_sequence(vector, 1, 2);

  auto it = vector.begin();
  REQUIRE(*it == vector.front());
  REQUIRE(*it == vector[0]);
  REQUIRE(it->get_value() == 1);
  ++it;
  REQUIRE(*it == vector[1]);
  REQUIRE(*it == vector.back());
  REQUIRE(it->get_value() == 2);
  ++it;
  REQUIRE(it == vector.end());
  // Bidirectional iteration, mirroring LLVM IterationTest.
  --it;
  REQUIRE(*it == vector[1]);
  REQUIRE(it->get_value() == 2);
  --it;
  REQUIRE(*it == vector[0]);
  REQUIRE(it->get_value() == 1);

  int expected_value = 10;
  for (auto& value : vector) {
    value = Constructable{expected_value++};
  }
  expect_values(vector, 10, 11);

  const SmallVector<Constructable, N>& const_vector = vector;
  auto const_it = const_vector.begin();
  REQUIRE(const_it == const_vector.cbegin());
  REQUIRE(*const_it == const_vector[0]);
  REQUIRE(const_it->get_value() == 10);
  ++const_it;
  REQUIRE(*const_it == const_vector[1]);
  REQUIRE(const_it->get_value() == 11);
  ++const_it;
  REQUIRE(const_it == const_vector.end());
  REQUIRE(const_it == const_vector.cend());

  int sum = 0;
  for (const auto& value : const_vector) {
    sum += value.get_value();
  }
  REQUIRE(sum == 21);
}

// Adapted from LLVM IterationTest.
// Reverse iterators are not provided, and range-for checks were added for dxu
// pointer iterators.
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

  expect_empty(vector);
  expect_values(other, 1, 2);
}

// Adapted from LLVM SwapTest.
// dxu::SmallVector only exposes member swap.
TEST_CASE("SmallVectorTest::SwapTest") {
  RUN_TEST_CAPACITY(test_swap_small_and_empty);
}

template <std::size_t N>
void test_assign_range_from_small_vector_same_n() {
  SmallVector<Constructable, N> other_vector;
  other_vector.push_back(Constructable{7});
  other_vector.push_back(Constructable{7});

  SmallVector<Constructable, N> vector;
  vector.push_back(Constructable{1});
  vector.assign(other_vector);

  expect_values(vector, 7, 7);
  REQUIRE(vector.capacity() >= N);
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

void test_assign_count_value() {
  SmallVector<int, 2> vector{1, 2, 3};
  vector.assign(4, 9);

  REQUIRE(vector.size() == std::size_t{4});
  REQUIRE(vector.capacity() >= std::size_t{4});
  for (const auto& value : vector) {
    REQUIRE(value == 9);
  }

  vector.assign(1, 5);
  REQUIRE(vector.size() == std::size_t{1});
  REQUIRE(vector[0] == 5);
}

void test_assign_range_preallocates_for_forward_iterators() {
  SmallVector<int, 3> source{1, 2, 3, 4};
  SmallVector<int, 2> target;
  target.push_back(99);

  target.assign(source.begin(), source.end());

  REQUIRE(target.size() == source.size());
  REQUIRE(target.capacity() == source.size());
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

// Adapted from LLVM AssignTest, AssignRangeTest, AssignNonIterTest, and
// AssignSmallVector. Extra cases cover dxu cross-capacity assignment paths.
TEST_CASE("SmallVectorTest::AssignTest") {
  RUN_TEST_CAPACITY(test_assign_range_from_small_vector_same_n);
  test_assign_range_from_small_vector_different_n_reallocate();
  test_assign_range_from_small_vector_different_n_reuses_existing_capacity();
  test_assign_count_value();
  test_assign_range_preallocates_for_forward_iterators();
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
      REQUIRE(target[i].get_value() == static_cast<int>(i + 1));
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
  expect_values(v, 2, 3);

  // Mirrors LLVM MoveAssignTest: after clearing the moved-from container, two
  // live elements remain in the destination.
  u.clear();
  REQUIRE(Constructable::num_constructor_calls - std::size_t{2} ==
          Constructable::num_destructor_calls);

  v.clear();
  REQUIRE(Constructable::num_constructor_calls ==
          Constructable::num_destructor_calls);
}

// Adapted from LLVM MoveAssignTest.
// Mirrors the constructor/destructor accounting after a move-assign cycle.
TEST_CASE("SmallVectorTest::MoveAssignBalancesCtorDtor") {
  test_move_assignment_balances_ctor_dtor();
}

template <std::size_t SrcN, std::size_t DstN>
void test_move_assignment_across_n_case(std::size_t source_size,
                                        std::size_t target_size) {
  SmallVector<Constructable, SrcN> source;
  for (std::size_t i{0}; i < source_size; ++i) {
    source.push_back(Constructable{static_cast<int>(i + 1)});
  }
  const Constructable* source_data = source.data();
  const bool source_was_inline = source_size <= SrcN;

  SmallVector<Constructable, DstN> target;
  for (std::size_t i{0}; i < target_size; ++i) {
    target.push_back(Constructable{static_cast<int>(100 + i)});
  }
  target = std::move(source);

  REQUIRE(target.size() == source_size);
  for (std::size_t i{0}; i < source_size; ++i) {
    REQUIRE(target[i].get_value() == static_cast<int>(i + 1));
  }
  // Heap is stolen iff the source held a heap buffer.
  REQUIRE((target.data() == source_data) == !source_was_inline);
  REQUIRE(source.empty());
  REQUIRE((source.data() == source_data) == source_was_inline);
}

// Adapted from LLVM DualSmallVectorsTest::MoveAssignment.
// Covers the four small/big combinations across two different SBO capacities.
TEST_CASE("SmallVectorTest::MoveAssignAcrossN") {
  // Small mode -> Small mode.
  test_move_assignment_across_n_case<4, 4>(1, 1);
  // Small mode -> Big mode.
  test_move_assignment_across_n_case<4, 2>(1, 4);
  // Big mode -> Small mode.
  test_move_assignment_across_n_case<2, 4>(5, 1);
  // Big mode -> Big mode.
  test_move_assignment_across_n_case<2, 2>(5, 6);
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
  REQUIRE(Constructable::num_constructor_calls - std::size_t{4} ==
          Constructable::num_destructor_calls);

  v.clear();
  REQUIRE(Constructable::num_constructor_calls ==
          Constructable::num_destructor_calls);
  // Move-assign across SBO capacities must never copy elements.
  REQUIRE(Constructable::num_copy_constructor_calls == 0);
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
  REQUIRE(const_vector[0].get_value() == 1);
  REQUIRE(const_vector[1].get_value() == 2);
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
  REQUIRE(Constructable::num_constructor_calls == 0);

  make_sequence(vector, 1, 4);
  expect_values(vector, 1, 2, 3, 4);
  // LLVM DirectVectorTest: 4 temporaries + 4 in-place moves = 8 ctor calls.
  REQUIRE(Constructable::num_constructor_calls == 8);
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

  // Mirrors LLVM InitializerList: re-binding via `= { ... }` materializes a
  // temporary and move-assigns it.
  SmallVector<int, 2> rebind = {};
  REQUIRE(rebind.empty());
  rebind = {0, 0};
  REQUIRE(rebind.size() == std::size_t{2});
  REQUIRE(rebind[0] == 0);
  REQUIRE(rebind[1] == 0);
  rebind = {-1, -1};
  REQUIRE(rebind.size() == std::size_t{2});
  REQUIRE(rebind[0] == -1);
  REQUIRE(rebind[1] == -1);
}

// Adapted from LLVM InitializerList.
// dxu currently covers construction only, while assignment is covered in
// AssignTest.
TEST_CASE("SmallVectorTest::InitializerListConstructor") {
  test_initializer_list_constructor();
}

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
    REQUIRE(inline_vector[i].get_value() == 0);
    REQUIRE(&inline_vector[i] == inline_vector.data() + i);
  }

  const std::size_t heap_size = N + 3;
  SmallVector<Constructable, N> heap_vector(heap_size);
  REQUIRE(heap_vector.size() == heap_size);
  REQUIRE(heap_vector.capacity() >= heap_size);
  for (std::size_t i{0}; i < heap_vector.size(); ++i) {
    REQUIRE(heap_vector[i].get_value() == 0);
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

  REQUIRE(vector.at(0).get_value() == 1);
  REQUIRE(vector.at(1).get_value() == 2);
  REQUIRE(&vector.at(0) == &vector[0]);
  REQUIRE(&vector.at(1) == &vector[1]);

  const SmallVector<Constructable, N>& const_vector = vector;
  REQUIRE(const_vector.at(0).get_value() == 1);
  REQUIRE(const_vector.at(1).get_value() == 2);
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
  REQUIRE(vector[0].get_value() == 1);
  REQUIRE(vector[1].get_value() == 2);

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
  expect_values(vector, 42);
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

  expect_values(vector, 1);
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
  REQUIRE(inline_copy[0].get_value() == 1);
  REQUIRE(inline_copy.data() != inline_source.data());

  SmallVector<Constructable, N> heap_source;
  make_sequence(heap_source, 1, static_cast<int>(N) + 2);
  SmallVector<Constructable, N> heap_copy{heap_source};

  REQUIRE(heap_copy.size() == heap_source.size());
  REQUIRE(heap_copy.data() != heap_source.data());
  for (std::size_t i{0}; i < heap_copy.size(); ++i) {
    REQUIRE(heap_copy[i].get_value() == heap_source[i].get_value());
    REQUIRE(&heap_copy[i] == heap_copy.data() + i);
  }
}

TEST_CASE("SmallVectorTest::CopyConstructorTest") {
  RUN_TEST_CAPACITY(test_copy_constructor);
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

  expect_empty(target);
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
  expect_values(vector, 1, 2, 3);

  vector.operator=(vector);
  REQUIRE(vector.data() == before);
  REQUIRE(vector.capacity() == before_capacity);
  expect_values(vector, 1, 2, 3);
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
  expect_values(target, 1, 2, 3);
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
  REQUIRE(target[0].get_value() == 1);
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
    REQUIRE(target[i].get_value() == static_cast<int>(i + 1));
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
  expect_values(vector, 1, 2, 3);
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
    REQUIRE(moved[i].get_value() == static_cast<int>(i + 1));
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
    REQUIRE(moved[i].get_value() == static_cast<int>(i + 1));
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
  REQUIRE(moved[0].get_value() == 1);
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
    REQUIRE(moved[i].get_value() == static_cast<int>(i + 1));
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
  REQUIRE(target[0].get_value() == 1);
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
    REQUIRE(target[i].get_value() == static_cast<int>(i + 1));
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
    REQUIRE(small[i].get_value() == static_cast<int>(7 + i));
  }
  REQUIRE(small.data() == big_data);

  expect_values(big, 1, 2);
}

void test_swap_self() {
  SmallVector<Constructable, 1> vector;
  make_sequence(vector, 1, 3);
  const Constructable* before = vector.data();

  vector.swap(vector);

  REQUIRE(vector.data() == before);
  expect_values(vector, 1, 2, 3);
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
