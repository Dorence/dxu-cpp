# Contributing

Thank you for contributing to `dxu-cpp`.

## Before You Start

- Open an issue or start a discussion for non-trivial changes.
- Keep changes focused and easy to review.
- Update tests or add focused coverage when behavior changes.

## Development

Build the project:

```bash
cmake -B build
cd build
make -j
```

Run tests:

```bash
cd build
make run_tests
```

## Code Style

- Follow the existing C++ style used in this repository.
- Prefer brace initialization when practical.
- Keep public APIs small, consistent, and well-named.
- Avoid unrelated refactoring in the same change.
- Check `.trae/rules` for additional rules, you may add them to your AI tools.

## Pull Requests

- Describe what changed and why.
- Mention any user-visible behavior changes.
- Include test coverage details or explain why tests are not needed.
- Make sure the project builds and relevant tests pass before requesting review.
