# Makefile for DSO203 Pawn execution environment
# Petteri Aimonen <jpa@dso.mail.kapsi.fi> 2012

# Name of the target application
NAME = PAWN_APP

# Names of the object files (add all .c files you want to include)
OBJS = main.o tinyprintf.o ds203_io.o

# Linker script (choose which application position to use)
LFLAGS  = -L linker_scripts -T app4.lds

# Any libraries to include
LIBS = -lm

# Include directories for .h files
CFLAGS = -I stm32_headers -I DS203 -I amx

# DS203 generic stuff
OBJS += startup.o BIOS.o Interrupt.o

# Names of the toolchain programs
CC      = arm-none-eabi-gcc
CP      = arm-none-eabi-objcopy
OD      = arm-none-eabi-objdump

# Processor type
CFLAGS += -mcpu=cortex-m3 -mthumb -mno-thumb-interwork

# Optimization & debug settings
CFLAGS += -fno-common -O1 -g -std=gnu99

# Compiler warnings
CFLAGS += -Wall -Werror -Wno-unused

# Default linker arguments (disables GCC-provided startup.c, creates .map file)
LFLAGS += -nostartfiles -Wl,-Map=build/$(NAME).map -eReset_Handler

# Directory for .o files
VPATH = build
_OBJS = $(addprefix build/,$(OBJS))

all: $(NAME).HEX

clean:
	rm -f $(NAME).HEX build/*

$(NAME).HEX: build/$(NAME).elf
	$(CP) -O ihex $< $@

build/$(NAME).elf: ${_OBJS}
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ ${_OBJS} ${LIBS}

# Rebuild all objects if any header changes
$(_OBJS): DS203/*.h Makefile

build/amxexec.o: amx/amxexec_thumb2_gas.s
	$(CC) $(CFLAGS) -Wa,--defsym,THUMB2=1 -c -o $@ $<

build/%.o: %.c *.h
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.o: DS203/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.o: DS203/%.S
	$(CC) $(CFLAGS) -c -o $@ $<

deploy: $(NAME).HEX
	mount /mnt/dso
	cp $< /mnt/dso
	umount /mnt/dso


