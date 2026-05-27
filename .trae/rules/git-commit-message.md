---
alwaysApply: false
description: Git Commit Message Rules
---
- Use English.
- Follow Conventional Commits spec.
- Follow historical commit style.

## Conventional Commits

```plain
<type>[scope][!]: <description>

[body]

[footer]
```

1. `type` is required and MUST be one of `feat`, `fix`, `ai`, `build`, `ci`, `chore`, `docs`, `refactor`, `perf`, `test`.
2. (Optional) `scope` SHOULD be a noun that describes a section of the project.
3. (NEVER use in this project) Optional `!` marks a breaking change.
4. `description` is required and SHOULD be a short summary of the change.
5. (Optional) `body` is for complex changes and starts after one blank line.
6. (Optional) `footer` MAY contain multiple git trailers and starts after one blank line.
  - Breaking change: `BREAKING CHANGE:`.
  - Committer: `Signed-off-by:`, `Acked-by:`, `Co-authored-by:`, `Reviewed-by:`, `Reported-by:`.
  - Link: `Refs:`, `See:`.

## Examples

```text
feat(ui): add dark mode toggle
fix(parser): handle empty input safely
refactor(core): simplify cache lifecycle
test: add coverage for retry logic
```
