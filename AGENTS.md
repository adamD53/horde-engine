# Caveman Mode Rules

1. Always strip out conversational filler words.
2. Maximize information density to save output tokens.
3. Maintain extreme brevity across all workspace prompts.

# Project Coding Guidelines and Instructions

This project targets modern C++ (C++20/C++23) using SFML 3. When writing, modifying, or refactoring code in this repository, strictly adhere to the following conventions:

---

## 1. Modern C++ Standards & Type Deduction (`auto`)

- **Standard**: Write idiomatic C++20/C++23 code leveraging modern language features (concepts, ranges, `std::span`, etc.).
- **Function Return Types**: Use `auto` for function return types whenever possible (relying on return type deduction or trailing return types as appropriate). The only exception is functions that return `void`, which should explicitly specify `void`.
- **Variable Declarations**: Practice Almost Always Auto (`auto`) for local variable declarations whenever possible (e.g., `auto x = ...`, `const auto& ref = ...`), ensuring types remain deduced while preserving readability and refactorability.

---

## 2. Code Structure & Organization

- **Separation of Declarations and Definitions**:
  - Keep declarations (class/struct declarations, interfaces, function prototypes) in `.hpp` header files (e.g., in `include/` or `src/`).
  - Keep implementation details and non-inline/non-template definitions in corresponding `.cpp` source files (in `src/`).
  - **Exception for Abstract Classes & Interfaces**: Abstract classes and interfaces may define trivial or empty virtual methods (e.g. default hooks like `{}`) directly in the `.hpp` header without creating a dedicated `.cpp` file.
- **Headers**: Use `#pragma once` as header guards and maintain clean include hierarchies (prefer including only what is needed).
- **SFML RULE**: Use the newest SFML 3 types.

---

## 3. Const Correctness

- **Member Functions**: Mark all member functions as `const` if they do not modify the object's observable state.
- **Variables & References**: Enforce immutability by default. Mark local variables, member variables (where suitable), and references as `const` unless mutation is required.
- **Function Parameters**: Pass non-trivial, non-movable types by `const` reference (`const T&`) unless ownership transfer or in-place mutation is explicitly intended.

---

## 4. RAII & Memory Management

- **RAII Principle**: Follow Resource Acquisition Is Initialization (RAII) for all resource lifetimes (textures, sounds, memory, buffers, file handles).
- **Smart Pointers**:
  - Never use owning raw pointers (`new` / `delete`).
  - Use `std::unique_ptr` for exclusive resource ownership and `std::shared_ptr` / `std::weak_ptr` only when shared ownership is genuinely required.
  - Construct smart pointers using `std::make_unique` and `std::make_shared`.
- **Non-Owning References**: Use references (`T&`, `const T&`), views (`std::string_view`, `std::span`), or non-owning raw pointers purely for observation/borrowing where lifetime guarantees are assured by callers.
