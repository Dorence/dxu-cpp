---
alwaysApply: false
description: How to create a single-file library file
---
Single-file library is a style that combines declaration and implmentation in a sigle file. 

- General file structure:

```cpp
#ifndef DXU_???_H_INCLUDE
#define DXU_???_H_INCLUDE

/* === declaration part === */

#endif  // DXU_???_H_INCLUDE
#ifdef DXU_???_IMPLEMENTATION

/* === implementation part === */

#endif  // DXU_???_IMPLEMENTATION
```

- `???` is the module name, e.g., `CHRONO`, `FORMAT`.
- For compatible with IDE, replace `#ifdef DXU_???_IMPLEMENTATION` with `#if defined(DXU_???_IMPLEMENTATION) || defined(__INTELLISENSE__)`.
- Example: `include/dxu/chrono.h`
