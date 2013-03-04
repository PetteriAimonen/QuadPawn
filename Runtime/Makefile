# Makefile for DSO203 Pawn execution environment
# Petteri Aimonen <jpa@dso.mail.kapsi.fi> 2012

# Name of the target application
NAME = PAWN_APP

# Names of the object files (add all .c files you want to include)
OBJS = main.o ds203_io.o libc_glue.o drawing.o menubar.o buttons.o \
	file_selector.o msgbox.o metadata.o debug.o \
	amx.o amxexec.o amxaux.o amxpool.o amx_debug.o \
	amx_draw.o amx_core.o amx_string.o amx_fixed.o amx_wavein.o \
	amx_waveout.o amx_menu.o amx_file.o amx_buttons.o amx_fourier.o \
	amx_time.o amx_device.o amx_fpga.o fpga.o\
	fix16.o fix16_sqrt.o fix16_trig.o fix16_exp.o \
	alterbios.o amx_overlays.o amx_usart.o

COMMITID := $(shell git describe --always || echo unknown)

# Linker script (choose which application position to use)
#cf i've used 3 to test and not overwrite official app
LFLAGS  = -L linker_scripts -T app3.lds 

# Any libraries to include
LIBS = -lm -lgcc baselibc/libc.a

# Include directories for .h files
CFLAGS = -I baselibc/include -I stm32_headers -I DS203 \
	-I amx -I libfixmath -DCOMMITID=\"$(COMMITID)\" \
	-I alterbios -I alterbios/fatfs

# DS203 generic stuff
OBJS += startup.o BIOS.o Interrupt.o

# Names of the toolchain programs
CC      = arm-none-eabi-gcc
CP      = arm-none-eabi-objcopy
OD      = arm-none-eabi-objdump

ifdef windir
RM = del
endif

# Processor type
CFLAGS += -mcpu=cortex-m3 -mthumb -mno-thumb-interwork

# Optimization & debug settings
CFLAGS += -fno-common -Os -g -std=gnu99 -DNDEBUG

# Link-time optimization
#CFLAGS += -flto

# Compiler warnings
CFLAGS += -Wall -Werror -Wno-unused

# Default linker arguments (disables GCC-provided startup.c, creates .map file)
LFLAGS += -nostartfiles -nostdlib -fwhole-program -Wl,-Map=build/$(NAME).map -eReset_Handler

# Directory for .o files
VPATH = build
_OBJS = $(addprefix build/,$(OBJS))

all: $(NAME).HEX

build_dir:
	mkdir -p build

clean:
	$(RM) $(NAME).HEX build/*

$(NAME).HEX: build/$(NAME).elf
	$(CP) -O ihex $< build/temp.hex
	head -n -1 build/temp.hex > $(NAME).HEX
	cat alterbios/ALTBIOS.HEX >> $(NAME).HEX

baselibc/libc.a: baselibc/Makefile
	make -C baselibc

build/$(NAME).elf: ${_OBJS} baselibc/libc.a
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ ${_OBJS} ${LIBS}

# Rebuild all objects if any header changes
$(_OBJS): DS203/*.h Makefile

build/cortex_stub.o: gdb_stub/cortex_stub.c gdb_stub/*.h
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.o: amx/%.c
	$(CC) $(CFLAGS) -Os -Wno-parentheses -DAMX_ASM=1 -DAMX_ANSIONLY=1 \
		-DAMX_NO_PACKED_OPC=1 -DAMX_NO_DYNALOAD=1 \
		-c -o $@ $<

build/amxexec.o: amx/amxexec_thumb2_gas.s
	$(CC) $(CFLAGS) -Wa,-mthumb -Wa,--defsym,THUMB2=1 \
		-Wa,--defsym,AMX_NO_PACKED_OPC=1\
		-c -o $@ $<

build/alterbios.o: alterbios/alterbios.s
	$(CC) $(CFLAGS) -c -o $@ $<

build/ff.o: fatfs/ff.c fatfs/*.h
	$(CC) $(CFLAGS) -Os -fno-strict-aliasing -c -o $@ $<

build/%.o: libfixmath/%.c
	$(CC) $(CFLAGS) -DFIXMATH_NO_CACHE -c -o $@ $<

build/%.o: %.c *.h | build_dir
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.o: DS203/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.o: DS203/%.S
	$(CC) $(CFLAGS) -c -o $@ $<

deploy: $(NAME).HEX
	mount /mnt/dso
	cp $< /mnt/dso
	sync
	umount /mnt/dso


