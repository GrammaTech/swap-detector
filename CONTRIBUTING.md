Contributing
============


## Code of Conduct

Please read the [Swap Detector Code of Conduct](CODE_OF_CONDUCT.md).

## General Guidelines

- Text files may not have trailing whitespace.

- Text files must end with a trailing newline.

- All tests should be able to run and pass.
  This can be checked by running `make check` on your build directory
    after running `cmake`.

## C++ Code Requirements

- All code shall be formatted with [clang-format](https://clang.llvm.org/docs/ClangFormat.html).
  A `.clang-format` is provided in the root directory for the project.

  - Code should generally follow the C++ Core Guidelines recommendations.

  - Code should generally allow for thread safety.
    - No static variables.
      - No globals
        - Free functions should not maintain state.
          - Use caution when using iterators to guard against invalidation.

          - Maintain const-correctness.

          - Use UpperCamelCase for type names.

          - Use UpperCamelCase for enum members.

          - Use lowerCamelCase for variable and class members.

          - Use lowerCamelCase or lower_snake_case for function and method names.

          - Do not use `using namespace std`

          - Use `auto` when the deduced type is explicitly spelled out in the
            initialization or if the deduced type is an abstract type
              alias.  Always explicitly specify type qualifiers, pointers, and
                references.  E.g.,
                  ```cpp
                    const auto *Ptr = dynamic_cast<const Foo *>(SomePtr);
                      auto Val = static_cast<unsigned>(SomeValue);
                        for (auto Iter = SomeContainer.begin(), End = SomeContainer.end(); Iter != End; ++Iter) {}
                          ```

                          - Use `auto` to make code more readable, but prefer `auto &` or `const auto *`
                            to explicitly spell out qualifiers and pointers or references.

### Testing Development

- All code you care about should be tested.
- Any code you don't care about should be removed.
- C++ code should be tested on Linux using GCC and Clang.
- Code testing is done via Google Test and lit.

## Documentation

The Swap Detector documentation consists of compilation and example usage, but
does not currently require other kinds of documentation like Doxygen. However,
new code should have appropriate comments and be self-documenting.

