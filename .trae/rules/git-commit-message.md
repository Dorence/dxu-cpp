---
alwaysApply: false
description: Git commit message
scene: git_message
---
- Use English.
- Follow Conventional Commits.
- Follow historical commit style when available.
- Prefer single-line message for simple changes.

## Conventional Commits

```
<type>[scope]: <description>

[body]

[footer]
```

1. (Required) `type` MUST be one of `feat`, `fix`, `ai`, `build`, `ci`, `chore`, `docs`, `refactor`, `perf`, `style`, `test`.
2. (Optional) `scope` SHOULD be a noun that describes a section of the project.
3. (Required) `description` SHOULD be a short but concise summary of the change.
4. (Optional) `body` is ONLY for explaining complex changes, starts after one blank line. 
5. (Optional) `footer` MAY have multiple git trailers, starts after one blank line. Can be one of `BREAKING CHANGE:`, `Signed-off-by:`, `Co-authored-by:`, `Refs:`, `See:`, etc.

## Examples

```
feat(ui): add dark mode toggle
fix(parser): handle empty input safely
refactor(core): simplify cache lifecycle
test: add coverage for retry logic
```