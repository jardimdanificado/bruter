
# BRUTER


*`UNDER HEAVY DEVELOPMENT`*


# Reserved

- `()` = expression

- `(@@ )` = string delimiter

- `""` = string delimiter

- `''` = string delimiter

- `;` = end of command separator

# Usage

    command;
    command ...;
    command (command (command ...));
    command variable_1 variable_2 ...;
    command name;

# Types
      
  BRUTER essentially has 2 types of data:
  - `any`, 4 or 8 bytes;
  - `alloc`, a pointer, usually to a string;

# Building instructions

  BRUTER include its own build script;

    # clean build for the current system;
    
    ./build.sh

    # debug build;
    
    ./build.sh --debug

    # cc option:
    # you can also define the path to a compiler;

    ./build.sh -cc path/to/compiler
