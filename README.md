# CD32 controller to USB adapter firmware

AVR microcontroller firmware to interface an Amiga CD32 controller and USB.

## Supported micro-controller(s)

Currently supported micro-controller(s):

* Atmega8
* Atmega168

Adding support for other micro-controllers should be easy, as long as the target has enough
IO pins, enough memory (flash and SRAM) and is supported by V-USB.

## Wiring

This project does not have a dedicated page so there is not CD32-specific schematic. However,
this firmware is meant to be used on a raphnet [multiuse PCB2 circuit](http://www.raphnet.net/electronique/multiuse_pcb2/index_en.php).

Follow the Multiuse PCB2 schematic and using the following table to wire the controller:

| IO  | DB9 pin | Color      | Description |
|-----|---------|------------|-------------|
| PC5 |  1      | White      | Up          |
| PC4 |  2      | Blue       | Down        |
| PC3 |  3      | Green      | Left        |
| PC2 |  4      | Brown      | Right       |
| PC1 |  6      | Orange     | Clock (to controller) |
| PC0 |  9      | Grey       | Serial Data (from controller) |
| PB5 |  5      | Yellow     | Shift/Load  (When high, LOAD) |
| VCC |  7      | Red        | Controller VCC |
| GND |  8      | Black      | Controller GND |

## Built with

* [avr-gcc](https://gcc.gnu.org/wiki/avr-gcc)
* [avr-libc](http://www.nongnu.org/avr-libc/)
* [gnu make](https://www.gnu.org/software/make/manual/make.html)

## License

This project is licensed under the terms of the GNU General Public License, version 2.

## Acknowledgments

* Thank you to Objective development, author of [V-USB](https://www.obdev.at/products/vusb/index.html) for a wonderful software-only USB device implementation.
