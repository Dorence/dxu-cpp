/**
 * @brief std::shared_mutex portability.
 */
#pragma once

#include <mutex>
#if __cplusplus >= 201402L
#include <shared_mutex>
#endif

#include "dxu/version.h"

namespace DXU_NAMESPACE {
#if __cplusplus > 201402L  // C++17 and later
using shared_mutex = std::shared_mutex;
using shared_lock_sm = std::shared_lock<std::shared_mutex>;
using unique_lock_sm = std::unique_lock<std::shared_mutex>;
#elif __cplusplus >= 201402L  // C++14
using shared_mutex = std::shared_timed_mutex;
using shared_lock_sm = std::shared_lock<std::shared_timed_mutex>;
using unique_lock_sm = std::unique_lock<std::shared_timed_mutex>;
#else                         // C++11 and earlier
using shared_mutex = std::mutex;
using shared_lock_sm = std::unique_lock<std::mutex>;
using unique_lock_sm = std::unique_lock<std::mutex>;
#endif

}  // namespace DXU_NAMESPACE
