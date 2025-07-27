# riscv64-unknown-elf- or riscv64-linux-gnu-
# perhaps in /opt/riscv/bin
# TOOLPREFIX =

# Try to infer the correct TOOLPREFIX if not set
ifndef TOOLPREFIX
TOOLPREFIX := $(shell if riscv64-unknown-elf-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-unknown-elf-'; \
	elif riscv64-linux-gnu-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-linux-gnu-'; \
	elif riscv64-unknown-linux-gnu-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-unknown-linux-gnu-'; \
	else echo "***" 1>&2; \
	echo "*** Error: Couldn't find a riscv64 version of GCC/binutils." 1>&2; \
	echo "*** To turn off this error, run 'gmake TOOLPREFIX= ...'." 1>&2; \
	echo "***" 1>&2; exit 1; fi)
endif

# Compilers and flags
CC := $(TOOLPREFIX)gcc
CFLAGS := -static -ggdb

# QEMU
QEMU = qemu-riscv64

# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
QEMUGDB = -g $(GDBPORT)


SRC ?= output.s
MAIN := main.c
EXE := $(patsubst %.s, %.elf, $(SRC))

.PHONY: clean

elf: $(MAIN) $(SRC)
	@echo "[+] Building $(EXE) with $(CC)..."
	@$(CC) $(CFLAGS) $(MAIN) $(SRC) -o $(EXE)

qemu: $(EXE)
	@echo "[*] Running $(EXE) via QEMU..."
	-@$(QEMU) $(EXE)

qemu-gdb: $(EXE)
	@echo "[*] Starting QEMU with GDB on port $(GDBPORT)..."
	@$(QEMU) $(QEMUGDB) $(EXE)

clean:
	@echo "[*] Cleaning..."
	@rm -f *.elf
