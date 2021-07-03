# Pawn for DSO Quad

![Header image](http://kapsi.fi/~jpa/stuff/pix/pawn_header.jpg)

Pawn scripting language for the DSO Quad pocket oscilloscope.
This is intended to make programming for DSO Quad easier and to allow tens of programs to reside on the flash drive.

It offers a comprehensive set of [libraries](https://github.com/PetteriAimonen/QuadPawn/tree/master/Compiler/include),
intuitive program selector and basic debugging facilities.

Read the [Getting started](https://github.com/PetteriAimonen/QuadPawn/wiki/Getting-started) guide, or just download
[the runtime](http://koti.kapsi.fi/~jpa/dsoquad/#QuadPawn) and [some apps](https://github.com/PetteriAimonen/QuadPawn/wiki).

Update 03/07/2021
tested on 2.72 8Mb only.
Modified linker script to use the new Alterbios placed just after SYS (ALT_F164.HEX/BIN).
Since the latest Alterbios is patched to work on 8Mb devices, no more fat corruption occur.
Now the flash in the range 0x08040000-0x0804C000 (52Kb!!) is free for other app.

Just for fun I added 2 new pawn functions: read_flash(address) and read_SN()
Wrote getlicense.pawn to test them, this app simply shows the serial number and the valid license to insert in case of DFU wipe.
