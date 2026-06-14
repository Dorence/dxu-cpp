#pragma once

// IWYU pragma: begin_exports
#include <cstddef>
#include <iterator>

#include "dxu/version.h"
// IWYU pragma: end_exports

namespace DXU_NAMESPACE {

/**
 * CRTP facade adding pointer iterators to a contiguous storage `Impl<T>`.
 * Should expose `data()` and `size()` that describe the contiguous storage.
 */
template <typename Impl, typename T>
class ContiguousStorageIteratorFacade {
 public:
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using size_type = std::size_t;

  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;

  using iterator = T*;
  using const_iterator = const T*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  constexpr iterator begin() noexcept { return vdata(); }
  constexpr iterator end() noexcept { return vdata() + vsize(); }

  constexpr const_iterator begin() const noexcept { return vdata(); }
  constexpr const_iterator end() const noexcept { return vdata() + vsize(); }

  constexpr const_iterator cbegin() const noexcept { return begin(); }
  constexpr const_iterator cend() const noexcept { return end(); }

  constexpr reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }
  constexpr reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }

  constexpr const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }
  constexpr const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  constexpr const_reverse_iterator crend() const noexcept { return rend(); }

 private:
  constexpr pointer vdata() noexcept {
    return static_cast<Impl*>(this)->data();
  }
  constexpr const_pointer vdata() const noexcept {
    return static_cast<const Impl*>(this)->data();
  }

  constexpr size_type vsize() const noexcept {
    return static_cast<const Impl*>(this)->size();
  }
};

}  // namespace DXU_NAMESPACE
