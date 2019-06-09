# Sprola

Signal PROcessing LAnguage (Sprola)

Sprola is a Domain Specific Language (DSL) with the purpose of simplifying
the creation of an audio plugin in the LV2 format. Other plugin targets and GUI creation is out of the project's scope.

## Required development tools

This project is being developed using the following development tools
* [Flex](https://github.com/westes/flex) for lexical analysis (2.6.4)
* [GNU Bison](https://www.gnu.org/software/bison/) for parsing (3.3.2)
* [LLVM](https://llvm.org/) for LLVM C API (8.0.1) and Clang compiler (7.0.1-8)

I'm developing using the Flex, Bison, and Clang versions from the [Debian](https://www.debian.org/) Buster respositories. The LLVM API, and llvm-config, is from the LLVM github repository 8.0 branch.

## Building and testing
To build:
  $ make clean
  $ make

The path to the LLVM binaries, for example '/usr/lib/llvm-4.0/bin' must be in your path statement.

To test:
  $ ./sprola amp.spl
  $ llvm-dis default_output.bc
  $ cat default_output.ll
