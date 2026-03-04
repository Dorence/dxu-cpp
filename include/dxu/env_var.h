#ifndef DXU_ENV_VAR_H_INCLUDE
#define DXU_ENV_VAR_H_INCLUDE

#include "dxu/conversion.h"

namespace DXU_NAMESPACE {

class EnvVarManager {
 public:
  static std::string GetString(const char* name,
                               const std::string& default_value = "") {
    const char* value = std::getenv(name);
    return value ? std::string(value) : default_value;
  }

  /**
   * @param name Environment variable name
   * @param default_value Value if not set or parsed error
   * @param error_value When parseing error, set to std::getenv result
   *
   * Case insensitive.
   * Truthy values: 1, true, on, y, yes.
   * Falsy values: 0, false, off, n, no.
   */
  static bool GetBool(const char* name, const bool default_value = false,
                      std::string* error_value = nullptr) {
    const char* value = std::getenv(name);
    if (value == nullptr) return default_value;
    Slice s = Slice(value).trim();
    if (!s.empty()) {
      std::string str = s.to_lower_string();  // case insensitive
      if (str == "1" || str == "true" || str == "on" || str == "y" ||
          str == "yes") {
        return true;
      } else if (str == "0" || str == "false" || str == "off" || str == "n" ||
                 str == "no") {
        return false;
      }
    }
    if (error_value != nullptr) *error_value = value;
    return default_value;
  }

  static int32_t GetInt32(const char* name, const int32_t default_value = 0) {
    const char* value = std::getenv(name);
    if (value == nullptr) {
      return default_value;
    }
    int err = 0;
    int result = StrToInt(value, {&err});
    if (err) {
      fprintf(stderr, "[%s] parse error %d for \"%s\"\n", __func__, err, value);
      result = default_value;
    }
    return result;
  }

  static int64_t GetInt64(const char* name, const int64_t default_value = 0) {
    const char* value = std::getenv(name);
    if (value == nullptr) {
      return default_value;
    }
    int err;
    int64_t result = StrToInt64(value, {&err});
    if (err) {
      fprintf(stderr, "[%s] parse error %d for \"%s\"\n", __func__, err, value);
      result = default_value;
    }
    return result;
  }

  // not overwrite existing env var
  static int Set(const char* name, const char* value) {
    return ::setenv(name, value, 0);
  }
};

}  // namespace DXU_NAMESPACE

#endif  // DXU_ENV_VAR_H_INCLUDE
