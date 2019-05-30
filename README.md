# Sprola

Signal PROcessing LAnguage (Sprola)

Sprola is a Domain Specific Language (DSL) with the purpose of simplifying
the creation of an audio plugin in the LV2 format. Other plugin targets and GUI creation is out of the project's scope.

## Required development tools

This project requires the following development tools
*[Flex](https://github.com/westes/flex) for lexical analysis (2.6.1)
*[GNU Bison](https://www.gnu.org/software/bison/) for parsing (3.0.4)
*[LLVM](https://llvm.org/) for LLVM C API (4.0) and Clang compiler (3.8.1)

I'm developing using the version from the [Debian](https://www.debian.org/) respositories

## Building and testing
To build:
  $ make clean
  $ make

The path to the LLVM binaries, for example '/usr/lib/llvm-4.0/bin' must be in your path statement.

To test:
  $ ./sprola amp.spl
  $ llvm-dis default_output.bc
  $ cat default_output.ll
