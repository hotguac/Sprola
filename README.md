# Sprola
Signal PROcessing LAnguage (Sprola)

Sprola is a Domain Specific Language (DSL) with the purpose of simplifying
the creation of an audio plugin in the LV2 format. Other plugin targets and GUI creation is out of the project's scope.

This project requires the tools GNU flex and GNU bison.

Sprola is being developed on a workstation running Debian. Flex version 2.6.1
and bison version 3.04 are both installed from the Debian repository.

To build:
  $ make clean
  $ make

To test:
  $ ./sprola test_3.spl
