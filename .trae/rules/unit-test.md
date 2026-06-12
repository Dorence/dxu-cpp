---
alwaysApply: false
description: unit test
---

- Build directory: `build-trae/`

```bash
cmake ..
make tests
# run test like
test/chrono_test
# run specific test (regex filter)
test/chrono_test '*Now*'
```

- Run test for both `Debug` and `RelWithDebInfo` build types.
- Follow `README.md` for more details.
