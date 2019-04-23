# Sprola
Signal PROcessing LAnguage (Sprola)

Sprola is a Domain Specific Language (DSL) with the pupose of simplifying
the creation of audio plugins in the LV2 format. Other plugin targets and GUI's
are out of scope at this time.

This project requires the tools flex and bison.

Sprola is being developed on a workstation running Debian. Flex version 2.6.1
and bison version 3.04 are both installed from the Debian repository.

To build:
  $ make

To test:
  $ ./sprola test_3.spl
