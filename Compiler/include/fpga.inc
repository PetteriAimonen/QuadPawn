/// This module provides a low-level FPGA interface for use with custom FPGA
/// binaries. It exposes most of the available signals between STM32 and FPGA.

const fpgapin:
{
    PB0     = 0x0001, // FPGA pin 63, GBIN
    PB1     = 0x0002, // FPGA pin 62, GBIN
    PB2     = 0x0004, // FPGA pin 60
    PA2     = 0x0008, // FPGA pin 72
    PA3     = 0x0010, // FPGA pin 71
    PA5     = 0x0020, // FPGA pin 69
    PA6     = 0x0040, // FPGA pin 68
    PA7     = 0x0080, // FPGA pin 66
    PC4     = 0x0100, // FPGA pin 65
    PC5     = 0x0200, // FPGA pin 64
    
    NO_PINS =  0x0000,
    ALL_PINS = 0xFFFF
}

/// Load a .bin file to the FPGA. Returns true if configuration succeeds.
/// Pass empty filename to load the default FPGA image from ROM.
/// Sets all pins to inputs before loading, to avoid any conflicts. 
native bool: fpga_load(const filename{});

/// Set the direction of the STM32 pins that are connected to the FPGA.
/// By default, all pins are input, i.e. 'read from FPGA'. Pass this function
/// a bitmask of all the pins you want to set as outputs.
/// All other pins are set to inputs with pull-up/pull-down enabled.
native fpga_config_outputs(fpgapin: outputs);

/// Read input pins. You can optionally give a mask of the pins to read, in
/// which case the return value will have all other bits set to zero.
/// For pins configured as outputs, the value read is the level present at
/// the pin. Because we don't currently use open-drain outputs, this should
/// be the same as the output value.
native fpgapin: fpga_read_pins(fpgapin: mask = ALL_PINS);

/// Write output pins. You can optionally give a mask of the pins to set,
/// other pins will keep their old values.
native fpga_write_pins(fpgapin: states, fpgapin: mask = ALL_PINS);

/// Set a single pin
stock fpga_set_pin(fpgapin: pin)
{
    fpga_write_pins(pin, pin);
}

/// Clear a single pin
stock fpga_clear_pin(fpgapin: pin)
{
    fpga_write_pins(NO_PINS, pin);
}

/// Read data from FPGA using the parallel bus. The data is transferred as
/// 16 bit values.
native fpga_read(dest[], count = sizeof dest);

/// Write data to FPGA using the parallel bus.
native fpga_write(const data[], count = sizeof data);


