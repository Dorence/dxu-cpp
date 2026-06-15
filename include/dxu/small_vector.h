#pragma once

#include <cstring>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "dxu/lang/contiguous_storage_iterator_facade.h"
#include "dxu/port/nodiscard.h"

namespace DXU_NAMESPACE {
/**
 * @brief Vector with SBO (small-buffer optimization).
 *
 * Stores up to N elements inline and switches to heap when it grows. Elements
 * are placed contiguously, so can use {data(), size()} as C-array.
 *
 * Supported operations:
 * - operator[] / at / front / back
 * - data / size / capacity / empty
 * - [c][r]begin / [c][r]end
 * - reserve / resize / clear
 * - push_back / emplace_back
 * - assign / move&copy ctor&assignment: allow different inline capacities
 * - swap
 *
 * Limitations:
 * - Don't pass references to existing elements as insertion or fill arguments,
 *   because reallocation can invalidate those references. For example,
 *   `v.push_back(v.front())`, `v.resize(v.size() + 1, v.back())`.
 * - Use move/swap only when you understand the storage state and cost.
 *   * Moving/swapping inline elements requires element-wise moves. Swap
 *     requires 3 moves in the worst case.
 *   * Trivially relocatable elements can use memcpy fast path, but still
 *     expensive.
 *   * If cheap move/swap is important, reserve more than N elements or choose a
 *     smaller N to spill to heap earlier.
 * - No assertions are provided, but at() still throws std::out_of_range.
 *   Use operator[] / front() / back() with care.
 * - No special handling for std::bad_alloc or throwing destructors. May
 *   propagate or terminate directly.
 */
template <typename T, std::size_t N = 16>
class SmallVector
    : public ContiguousStorageIteratorFacade<SmallVector<T, N>, T> {
  static_assert(N > 0, "SmallVector requires N > 0");
  using base_ = ContiguousStorageIteratorFacade<SmallVector<T, N>, T>;

 public:
  using value_type = typename base_::value_type;
  using allocator_type = std::allocator<T>;
  using size_type = typename base_::size_type;
  using difference_type = typename base_::difference_type;
  using reference = typename base_::reference;
  using const_reference = typename base_::const_reference;
  using pointer = typename base_::pointer;
  using const_pointer = typename base_::const_pointer;
  using iterator = typename base_::iterator;
  using const_iterator = typename base_::const_iterator;
  using reverse_iterator = typename base_::reverse_iterator;
  using const_reverse_iterator = typename base_::const_reverse_iterator;

  static constexpr size_type inline_capacity = N;

 protected:
  using alloc_traits_ = std::allocator_traits<allocator_type>;

  static_assert(std::is_same<typename alloc_traits_::pointer, pointer>::value,
                "SmallVector requires allocator::pointer == T*");

  static_assert(
      std::is_same<typename alloc_traits_::size_type, size_type>::value,
      "SmallVector requires allocator::size_type == std::size_t");

  // for faster noexcept destroy
  static constexpr bool kNothrowDestructible =
      std::is_nothrow_destructible<T>::value;

  // for faster noexcept copy construction
  static constexpr bool kNothrowCopyConstructible =
      std::is_nothrow_copy_constructible<T>::value;
  static constexpr auto kNothrowCopyConstructibleTag =
      std::integral_constant<bool, kNothrowCopyConstructible>{};

  // for faster noexcept move (both source move and target clear are noexcept)
  static constexpr bool kNothrowMovable =
      std::is_nothrow_move_constructible<T>::value &&
      std::is_nothrow_destructible<T>::value;
  static constexpr auto kNothrowMovableTag =
      std::integral_constant<bool, kNothrowMovable>{};

  // for faster element destroy
  static constexpr auto kTriviallyDestructibleTag =
      std::integral_constant<bool, std::is_trivially_destructible<T>::value>{};

  // for faster element memcpy (relocation)
  static constexpr bool kTriviallyRelocatable =
      std::is_trivially_copy_constructible<T>::value &&
      std::is_trivially_move_constructible<T>::value &&
      std::is_trivially_destructible<T>::value;
  static constexpr auto kTriviallyRelocatableTag =
      std::integral_constant<bool, kTriviallyRelocatable>{};

 public:
  SmallVector() noexcept : begin_(inline_data()), size_(0), capacity_(N) {}

  explicit SmallVector(size_type count) : SmallVector() { resize(count); }

  SmallVector(size_type count, const_reference value) : SmallVector() {
    resize(count, value);
  }

  SmallVector(std::initializer_list<T> init) : SmallVector() {
    reserve(init.size());
    copy_into_uninitialized(init.begin(), init.end());
  }

  SmallVector(const SmallVector& other) : SmallVector() {
    reserve(other.size());
    copy_into_uninitialized(other.begin(), other.end());
  }

  template <std::size_t N2>
  SmallVector(const SmallVector<T, N2>& other) : SmallVector() {
    reserve(other.size());
    copy_into_uninitialized(other.begin(), other.end());
  }

  SmallVector(SmallVector&& other) noexcept(kNothrowMovable) : SmallVector() {
    move_from(std::move(other));
  }

  template <std::size_t N2>
  SmallVector(SmallVector<T, N2>&& other) noexcept(kNothrowMovable)
      : SmallVector() {
    move_from(std::move(other));
  }

  ~SmallVector() noexcept(kNothrowDestructible) { reset(); }

  SmallVector& operator=(std::initializer_list<T> init) {
    assign(init);
    return *this;
  }

  SmallVector& operator=(const SmallVector& other) {
    assign(other);
    return *this;
  }

  template <std::size_t N2>
  SmallVector& operator=(const SmallVector<T, N2>& other) {
    assign(other);
    return *this;
  }

  SmallVector& operator=(SmallVector&& other) noexcept(kNothrowMovable) {
    assign(std::move(other));
    return *this;
  }

  template <std::size_t N2>
  SmallVector& operator=(SmallVector<T, N2>&& other) noexcept(kNothrowMovable) {
    assign(std::move(other));
    return *this;
  }

  pointer data() noexcept { return begin_; }

  const_pointer data() const noexcept { return begin_; }

  size_type size() const noexcept { return size_; }

  size_type capacity() const noexcept { return capacity_; }

  NODISCARD bool empty() const noexcept { return size_ == 0; }

  reference operator[](size_type index) noexcept { return begin_[index]; }

  const_reference operator[](size_type index) const noexcept {
    return begin_[index];
  }

  reference at(size_type index) {
    if (index >= size_) {
      throw std::out_of_range{"SmallVector::at"};
    }
    return begin_[index];
  }

  const_reference at(size_type index) const {
    if (index >= size_) {
      throw std::out_of_range{"SmallVector::at"};
    }
    return begin_[index];
  }

  reference front() noexcept { return begin_[0]; }

  const_reference front() const noexcept { return begin_[0]; }

  reference back() noexcept { return begin_[size_ - 1]; }

  const_reference back() const noexcept { return begin_[size_ - 1]; }

  void push_back(const_reference value) { emplace_back(value); }

  void push_back(T&& value) { emplace_back(std::move(value)); }

  template <typename... Args>
  reference emplace_back(Args&&... args) {
    if (size_ == capacity_) {
      size_type next_capacity = capacity_ + capacity_ / 2 + 1;
      grow_to_capacity(next_capacity);
    }
    construct_at(begin_ + size_, std::forward<Args>(args)...);
    ++size_;
    return begin_[size_ - 1];
  }

  void clear() noexcept(kNothrowDestructible) {
    destroy_range(begin_, size_);
    size_ = 0;
  }

  void reserve(size_type new_capacity) {
    if (new_capacity > capacity_) {
      grow_to_capacity(new_capacity);
    }
  }

  void resize(size_type new_size) {
    if (new_size < size_) {
      destroy_range(begin_ + new_size, size_ - new_size);
      size_ = new_size;
    } else {
      reserve(new_size);
      for (; size_ < new_size; ++size_) {
        construct_at(begin_ + size_);
      }
    }
  }

  void resize(size_type new_size, const_reference value) {
    if (new_size < size_) {
      destroy_range(begin_ + new_size, size_ - new_size);
      size_ = new_size;
    } else {
      reserve(new_size);
      fill_into_uninitialized(new_size, value);
    }
  }

  template <std::size_t N2>
  void assign(const SmallVector<T, N2>& other) {
    if (!same_addr(this, &other)) {
      clear();
      reserve(other.size());
      copy_into_uninitialized(other.begin(), other.end());
    }
  }

  template <std::size_t N2>
  void assign(SmallVector<T, N2>&& other) noexcept(kNothrowMovable) {
    if (!same_addr(this, &other)) {
      reset();
      move_from(std::move(other));
    }
  }

  void assign(std::initializer_list<T> init) {
    clear();
    reserve(init.size());
    copy_into_uninitialized(init.begin(), init.end());
  }

  void assign(size_type count, const_reference value) {
    clear();
    reserve(count);
    fill_into_uninitialized(count, value);
  }

  template <
      typename InputIt,
      typename std::enable_if<!std::is_integral<InputIt>::value, int>::type = 0>
  void assign(InputIt first, InputIt last) {
    using IterCategory =
        typename std::iterator_traits<InputIt>::iterator_category;
    assign(first, last, IterCategory{});
  }

  void swap(SmallVector& other) noexcept(kNothrowMovable) {
    if (this != &other) {
      SmallVector tmp = std::move(other);
      other = std::move(*this);
      *this = std::move(tmp);
    }
  }

 private:
  NODISCARD static bool same_addr(const void* a, const void* b) noexcept {
    return a == b;
  }

  NODISCARD pointer inline_data() noexcept {
    return reinterpret_cast<pointer>(inline_storage_);
  }

  NODISCARD bool is_small() const noexcept {
    return same_addr(begin_, inline_storage_);
  }

  NODISCARD static allocator_type& allocator_ref() noexcept {
    static allocator_type allocator;
    return allocator;
  }

  NODISCARD pointer alloc(size_type capacity) {
    return alloc_traits_::allocate(allocator_ref(), capacity);
  }

  void dealloc(pointer ptr, size_type capacity) noexcept {
    alloc_traits_::deallocate(allocator_ref(), ptr, capacity);
  }

  template <typename... Args>
  static void construct_at(pointer ptr, Args&&... args) noexcept(
      std::is_nothrow_constructible<T, Args...>::value) {
    ::new (static_cast<void*>(ptr)) T(std::forward<Args>(args)...);
  }

  // delete [ptr, ptr + count) elements
  static void destroy_range(pointer ptr,
                            size_type count) noexcept(kNothrowDestructible) {
    destroy_range_impl(ptr, count, kTriviallyDestructibleTag);
  }

  // kTriviallyDestructibleTag: fast -> skip destroy
  static void destroy_range_impl(pointer, size_type, std::true_type) noexcept {}

  // kTriviallyDestructibleTag: slow -> destroy one by one
  static void destroy_range_impl(
      pointer ptr, size_type count,
      std::false_type) noexcept(kNothrowDestructible) {
    for (size_type i = 0; i < count; ++i) {
      ptr[i].~T();
    }
  }

  // clear, deallocate heap if used
  void reset() {
    clear();
    if (!is_small()) {
      dealloc(begin_, capacity_);
      begin_ = inline_data();
      capacity_ = N;
    }
  }

  // Called by move ctor and move assign. Restriction: (1) begin_ ==
  // inline_data(), (2) size_ == 0, (3) capacity_ == N, (4) this != &other.
  template <std::size_t N2>
  void move_from(SmallVector<T, N2>&& other) noexcept(kNothrowMovable) {
    if (other.is_small()) {
      reserve(other.size_);
      move_into_uninitialized(std::move(other), kNothrowMovableTag);
    } else {
      begin_ = other.begin_;
      size_ = other.size_;
      capacity_ = other.capacity_;
      other.begin_ = other.inline_data();
      other.size_ = 0;
      other.capacity_ = N2;
    }
  }

  template <std::size_t N2>
  void move_into_uninitialized(SmallVector<T, N2>&& other,
                               std::true_type) noexcept {
    for (; size_ < other.size_; ++size_) {
      construct_at(begin_ + size_, std::move(other.begin_[size_]));
    }
    other.clear();
  }

  template <std::size_t N2>
  void move_into_uninitialized(SmallVector<T, N2>&& other, std::false_type) {
    try {
      for (; size_ < other.size_; ++size_) {
        construct_at(begin_ + size_, std::move(other.begin_[size_]));
      }
    } catch (...) {
      reset();
      throw;
    }
    other.clear();
  }

  // point begin_ to data, deallocate if necessary
  void relocate_to_new_storage(pointer data, size_type capacity) noexcept {
    if (!is_small()) {
      dealloc(begin_, capacity_);
    }
    begin_ = data;
    capacity_ = capacity;
  }

  // size_ is unchanged
  void grow_to_capacity(size_type new_capacity) {
    if (size_ == 0) {
      pointer new_data = alloc(new_capacity);
      relocate_to_new_storage(new_data, new_capacity);
    } else {
      grow_and_copy(new_capacity, kTriviallyRelocatableTag);
    }
  }

  // size_ > 0, kTriviallyRelocatableTag: fast -> memcpy
  void grow_and_copy(size_type new_capacity, std::true_type) {
    pointer new_data = alloc(new_capacity);
    std::memcpy(new_data, begin_, size_ * sizeof(T));
    relocate_to_new_storage(new_data, new_capacity);
  }

  // size_ > 0, kTriviallyRelocatableTag: slow -> move one by one
  void grow_and_copy(size_type new_capacity, std::false_type) {
    grow_to_capacity_by_move(new_capacity, kNothrowMovableTag);
  }

  // size_ > 0, kNothrowMovableTag: fast -> noexcept move and clear obsolete
  void grow_to_capacity_by_move(size_type new_capacity,
                                std::true_type) noexcept(kNothrowDestructible) {
    pointer new_data = alloc(new_capacity);
    for (size_type i = 0; i < size_; ++i) {
      construct_at(new_data + i, std::move(begin_[i]));
    }
    destroy_range(begin_, size_);
    relocate_to_new_storage(new_data, new_capacity);
  }

  // size_ > 0, kNothrowMovableTag: slow -> restore on exception
  void grow_to_capacity_by_move(size_type new_capacity, std::false_type) {
    pointer new_data = alloc(new_capacity);
    size_type constructed = 0;
    try {
      for (; constructed < size_; ++constructed) {
        construct_at(new_data + constructed,
                     std::move_if_noexcept(begin_[constructed]));
      }
    } catch (...) {
      destroy_range(new_data, constructed);
      dealloc(new_data, new_capacity);
      throw;
    }
    destroy_range(begin_, size_);
    relocate_to_new_storage(new_data, new_capacity);
  }

  // fill [size_, count) with value
  void fill_into_uninitialized(
      size_type new_size,
      const_reference value) noexcept(kNothrowCopyConstructible) {
    fill_into_uninitialized_impl(new_size, value, kNothrowCopyConstructibleTag);
  }

  // kNothrowCopyConstructibleTag: fast -> noexcept copy
  void fill_into_uninitialized_impl(size_type new_size, const_reference value,
                                    std::true_type) noexcept {
    for (; size_ < new_size; ++size_) {
      construct_at(begin_ + size_, value);
    }
  }

  // kNothrowCopyConstructibleTag: slow -> reset on exception
  void fill_into_uninitialized_impl(size_type new_size, const_reference value,
                                    std::false_type) {
    try {
      for (; size_ < new_size; ++size_) {
        construct_at(begin_ + size_, value);
      }
    } catch (...) {
      reset();
      throw;
    }
  }

  template <typename InputIt>
  void assign(InputIt first, InputIt last, std::input_iterator_tag) {
    clear();
    for (; first != last; ++first) {
      emplace_back(*first);
    }
  }

  template <typename ForwardIt>
  void assign(ForwardIt first, ForwardIt last, std::forward_iterator_tag) {
    const auto count = static_cast<size_type>(std::distance(first, last));
    clear();
    reserve(count);
    copy_into_uninitialized(first, last);
  }

  // Called by copy/init_list ctor and assign(). Restriction: size_ == 0.
  template <typename InputIt>
  void copy_into_uninitialized(InputIt first, InputIt last) noexcept(
      kNothrowCopyConstructible) {
    copy_into_uninitialized_impl(first, last, kNothrowCopyConstructibleTag);
  }

  // kNothrowCopyConstructibleTag: fast -> noexcept copy
  template <typename InputIt>
  void copy_into_uninitialized_impl(InputIt first, InputIt last,
                                    std::true_type) noexcept {
    for (; first != last; ++first) {
      construct_at(begin_ + size_, *first);
      ++size_;
    }
  }

  // kNothrowCopyConstructibleTag: slow -> reset on exception
  template <typename InputIt>
  void copy_into_uninitialized_impl(InputIt first, InputIt last,
                                    std::false_type) {
    try {
      for (; first != last; ++first) {
        construct_at(begin_ + size_, *first);
        ++size_;
      }
    } catch (...) {
      reset();
      throw;
    }
  }

  template <typename, std::size_t>
  friend class SmallVector;  // allow private access to a different N

  pointer begin_;
  size_type size_;
  size_type capacity_;  // must >= N
  alignas(T) char inline_storage_[N * sizeof(T)];
};
}  // namespace DXU_NAMESPACE
