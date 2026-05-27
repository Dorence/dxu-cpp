---
alwaysApply: false
description: Rules for Git commit message
---
# Git Commit Message Rules

- Use English
- Use Conventional Commits 1.0.0.
- Should refer to historical commit messages.

## Conventional Commits

```plain
<type>[scope][!]: <description>

[body]

[footer: message]
```

1. `type` is required and must be one of `feat`, `fix`, `ai`, `build`, `ci`, `chore`, `docs`, `refactor`, `perf`, `test`.
2. `scope` is optional and should be a noun that describes a section of the project.
3. (Don't use in this project) `!` is optional and marks a breaking change.
4. `description` is required and should be a short summary of the change.
5. `body` is optional and starts after one blank line.
6. `footer` is optional, starts after one blank line, and may contain multiple git trailer-like entries.
  - Breaking change: `BREAKING CHANGE:`.
  - People: `Signed-off-by:`, `Acked-by:`, `Co-authored-by:`, `Reviewed-by:`.
  - Links: `Refs:`, `See:`, `See-also:`.

## Examples

```text
feat(ui): add dark mode toggle
fix(parser): handle empty input safely
refactor(core): simplify cache lifecycle
test: add coverage for retry logic
```
