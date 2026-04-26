# Contributing to cVisa

## Project Standards

### Language Requirements

**All project content must be in English**, including:
- Documentation (README, comments, guides)
- Code comments and documentation strings
- Commit messages
- Issue descriptions and pull requests
- Variable names, function names, and identifiers

### Code Standards

- **Language**: C++23
- **Build System**: CMake (minimum version 3.20)
- **Generator**: Ninja
- **Dependencies**: NI-VISA library (visa64.lib)

### Coding Guidelines

- Use modern C++23 features
- Follow type-safe practices with compile-time verification
- Use designated initializers for configuration structs
- Prefer `std::variant` for zero-cost abstraction over traditional polymorphism
- Keep error messages descriptive and informative

### Documentation

- Update README.md for any user-facing changes
- Document all public APIs with clear descriptions
- Include code examples for new features
- Maintain consistency in formatting and style

### Commit Messages

Write clear, concise commit messages in English:
- Use imperative mood ("Add feature" not "Added feature")
- Keep first line under 50 characters
- Provide detailed description if needed

## Questions?

For questions or discussions, please open an issue on the project repository.