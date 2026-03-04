#include "dxu/env_var.h"

#include "test_common.h"

namespace DXU_NAMESPACE {

const char* kTestEnvName = "__ENV_FOR_TEST__";

void TestGetBool(const char* val, bool expect, bool def = false,
                 const std::string& err_val = "") {
  ::setenv(kTestEnvName, val, 1 /* replace */);
  std::string err;
  bool ret = EnvVarManager::GetBool(kTestEnvName, def, &err);
  REQUIRE(ret == expect);
  REQUIRE(err_val == err);
}

TEST_CASE("EnvVar::GetBool") {
  // good cases: truthy
  TestGetBool("1", true);
  TestGetBool("true", true);
  TestGetBool("on", true);
  TestGetBool("y", true);
  TestGetBool("yes", true);
  TestGetBool(" TRUE ", true);

  // good cases: falsy
  TestGetBool("0", false, true);
  TestGetBool("false", false, true);
  TestGetBool("off", false, true);
  TestGetBool("n", false, true);
  TestGetBool("no", false, true);
  TestGetBool("\t\tFALSE", false, true);

  // bad cases
  TestGetBool("\t\t\t", true, true, "\t\t\t");
  TestGetBool("123", false, false, "123");
  TestGetBool("turn", false, false, "turn");
};

}  // namespace DXU_NAMESPACE
