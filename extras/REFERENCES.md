# Technical references

The implementation in this repository is independently written.

The following projects were consulted to understand public protocol behavior, compatibility issues and API design trade-offs:

- Bill Porter / PS2X family — historical Arduino PS2 protocol behavior, command sequences and controller modes.
- SukkoPera/PsxNewLib — hardware SPI vs bit-bang architecture, SPI mode/bit order and compatibility discussion.
- lqtx96/PS2Controller_AVR — AVR hardware-SPI timing and recovery ideas.
- pixelwaster/PS2X_library — analog-mode recovery behavior.

No source code from those projects is incorporated into this repository. Their respective licenses remain applicable to their own code.
