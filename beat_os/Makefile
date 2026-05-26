# Makefile - Build system for beat_os

# Toolchain
CC      = x86_64-linux-gnu-gcc
LD      = x86_64-linux-gnu-ld
NASM    = nasm

# Compiler flags: freestanding, no stdlib, no red-zone (critical for kernel)
CFLAGS  = -ffreestanding -nostdlib -nostdinc -fno-stack-protector \
          -fno-pic -fno-pie -mno-red-zone -mno-sse -mno-sse2 \
          -mcmodel=kernel -Wall -Wextra -Iinclude

# Assembler flags
ASMFLAGS = -f elf64

# Linker flags
LDFLAGS = -T linker.ld -nostdlib -static

# Directories
BUILD_DIR = build
ISO_DIR   = $(BUILD_DIR)/iso

# Output
KERNEL_ELF        = $(BUILD_DIR)/beat_os.elf
ISO               = $(BUILD_DIR)/beat_os.iso
COMPILE_COMMANDS  = compile_commands.json

# Source files
ASM_SRC            = src/boot/boot.asm
BOOT_INFO_SRC      = src/boot/multiboot2.c
MEMORY_MAP_SRC     = src/memory/memory_map.c
INTERRUPT_ASM_SRC  = src/interrupt/interrupt_stubs.asm
KERNEL_SRC         = src/kernel/kernel.c
LOG_SRC            = src/drivers/log.c
STRING_SRC         = src/lib/string.c
IDT_SRC            = src/interrupt/idt.c
TEST_SRC           = test/kernel/test_main.c
TEST_STRING_SRC    = test/lib/test_string.c
TEST_INTERRUPT_SRC = test/interrupt/test_interrupt.c

# Object files
ASM_OBJ            = $(BUILD_DIR)/boot.o
BOOT_INFO_OBJ      = $(BUILD_DIR)/multiboot2.o
MEMORY_MAP_OBJ     = $(BUILD_DIR)/memory_map.o
INTERRUPT_ASM_OBJ  = $(BUILD_DIR)/interrupt_stubs.o
KERNEL_OBJ         = $(BUILD_DIR)/kernel.o
LOG_OBJ            = $(BUILD_DIR)/log.o
STRING_OBJ         = $(BUILD_DIR)/string.o
IDT_OBJ            = $(BUILD_DIR)/idt.o
TEST_OBJ           = $(BUILD_DIR)/test_main.o
TEST_STRING_OBJ    = $(BUILD_DIR)/test_string.o
TEST_INTERRUPT_OBJ = $(BUILD_DIR)/test_interrupt.o
C_OBJ              = $(BOOT_INFO_OBJ) $(MEMORY_MAP_OBJ) $(INTERRUPT_ASM_OBJ) $(KERNEL_OBJ) $(LOG_OBJ) $(STRING_OBJ) $(IDT_OBJ) $(TEST_OBJ) $(TEST_STRING_OBJ) $(TEST_INTERRUPT_OBJ)

.PHONY: all run debug clean iso compile_commands

all: $(COMPILE_COMMANDS) $(ISO)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(ASM_OBJ): $(ASM_SRC) | $(BUILD_DIR)
	$(NASM) $(ASMFLAGS) -o $@ $<

$(INTERRUPT_ASM_OBJ): $(INTERRUPT_ASM_SRC) | $(BUILD_DIR)
	$(NASM) $(ASMFLAGS) -o $@ $<

$(BOOT_INFO_OBJ): $(BOOT_INFO_SRC) include/boot/multiboot2.h include/drivers/log.h include/lib/string.h include/memory/memory_map.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(MEMORY_MAP_OBJ): $(MEMORY_MAP_SRC) include/drivers/log.h include/memory/memory_map.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KERNEL_OBJ): $(KERNEL_SRC) include/boot/multiboot2.h include/drivers/log.h include/interrupt/idt.h include/test/test.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(LOG_OBJ): $(LOG_SRC) include/drivers/log.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(STRING_OBJ): $(STRING_SRC) include/lib/string.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(IDT_OBJ): $(IDT_SRC) include/drivers/log.h include/interrupt/idt.h include/lib/string.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TEST_OBJ): $(TEST_SRC) include/drivers/log.h include/test/test.h include/test/test_interrupt.h include/test/test_string.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TEST_STRING_OBJ): $(TEST_STRING_SRC) include/lib/string.h include/test/test.h include/test/test_string.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TEST_INTERRUPT_OBJ): $(TEST_INTERRUPT_SRC) include/drivers/log.h include/interrupt/idt.h include/test/test.h include/test/test_interrupt.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KERNEL_ELF): $(ASM_OBJ) $(C_OBJ) linker.ld | $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $@ $(ASM_OBJ) $(C_OBJ)

$(ISO): $(KERNEL_ELF) grub.cfg | $(BUILD_DIR)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_ELF) $(ISO_DIR)/boot/beat_os.elf
	cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(ISO_DIR) 2>/dev/null

$(COMPILE_COMMANDS): Makefile include/boot/multiboot2.h include/drivers/log.h include/interrupt/idt.h include/lib/string.h include/memory/memory_map.h include/test/test.h include/test/test_interrupt.h include/test/test_string.h $(BOOT_INFO_SRC) $(MEMORY_MAP_SRC) $(KERNEL_SRC) $(LOG_SRC) $(STRING_SRC) $(IDT_SRC) $(TEST_SRC) $(TEST_STRING_SRC) $(TEST_INTERRUPT_SRC)
	printf '[\n' > $@
	printf '  {\n' >> $@
	printf '    "directory": "%s",\n' '$(CURDIR)' >> $@
	printf '    "command": "%s -ffreestanding -nostdlib -nostdinc -fno-stack-protector -fno-pic -fno-pie -mno-red-zone -mno-sse -mno-sse2 -mcmodel=kernel -Wall -Wextra -Iinclude -c -o %s %s",\n' '$(CC)' '$(BOOT_INFO_OBJ)' '$(BOOT_INFO_SRC)' >> $@
	printf '    "file": "%s"\n' '$(BOOT_INFO_SRC)' >> $@
	printf '  },\n' >> $@
	printf '  {\n' >> $@
	printf '    "directory": "%s",\n' '$(CURDIR)' >> $@
	printf '    "command": "%s -ffreestanding -nostdlib -nostdinc -fno-stack-protector -fno-pic -fno-pie -mno-red-zone -mno-sse -mno-sse2 -mcmodel=kernel -Wall -Wextra -Iinclude -c -o %s %s",\n' '$(CC)' '$(MEMORY_MAP_OBJ)' '$(MEMORY_MAP_SRC)' >> $@
	printf '    "file": "%s"\n' '$(MEMORY_MAP_SRC)' >> $@
	printf '  },\n' >> $@
	printf '  {\n' >> $@
	printf '    "directory": "%s",\n' '$(CURDIR)' >> $@
	printf '    "command": "%s -ffreestanding -nostdlib -nostdinc -fno-stack-protector -fno-pic -fno-pie -mno-red-zone -mno-sse -mno-sse2 -mcmodel=kernel -Wall -Wextra -Iinclude -c -o %s %s",\n' '$(CC)' '$(KERNEL_OBJ)' '$(KERNEL_SRC)' >> $@
	printf '    "file": "%s"\n' '$(KERNEL_SRC)' >> $@
	printf '  },\n' >> $@
	printf '  {\n' >> $@
	printf '    "directory": "%s",\n' '$(CURDIR)' >> $@
	printf '    "command": "%s -ffreestanding -nostdlib -nostdinc -fno-stack-protector -fno-pic -fno-pie -mno-red-zone -mno-sse -mno-sse2 -mcmodel=kernel -Wall -Wextra -Iinclude -c -o %s %s",\n' '$(CC)' '$(LOG_OBJ)' '$(LOG_SRC)' >> $@
	printf '    "file": "%s"\n' '$(LOG_SRC)' >> $@
	printf '  },\n' >> $@
	printf '  {\n' >> $@
	printf '    "directory": "%s",\n' '$(CURDIR)' >> $@
	printf '    "command": "%s -ffreestanding -nostdlib -nostdinc -fno-stack-protector -fno-pic -fno-pie -mno-red-zone -mno-sse -mno-sse2 -mcmodel=kernel -Wall -Wextra -Iinclude -c -o %s %s",\n' '$(CC)' '$(STRING_OBJ)' '$(STRING_SRC)' >> $@
	printf '    "file": "%s"\n' '$(STRING_SRC)' >> $@
	printf '  },\n' >> $@
	printf '  {\n' >> $@
	printf '    "directory": "%s",\n' '$(CURDIR)' >> $@
	printf '    "command": "%s -ffreestanding -nostdlib -nostdinc -fno-stack-protector -fno-pic -fno-pie -mno-red-zone -mno-sse -mno-sse2 -mcmodel=kernel -Wall -Wextra -Iinclude -c -o %s %s",\n' '$(CC)' '$(IDT_OBJ)' '$(IDT_SRC)' >> $@
	printf '    "file": "%s"\n' '$(IDT_SRC)' >> $@
	printf '  },\n' >> $@
	printf '  {\n' >> $@
	printf '    "directory": "%s",\n' '$(CURDIR)' >> $@
	printf '    "command": "%s -ffreestanding -nostdlib -nostdinc -fno-stack-protector -fno-pic -fno-pie -mno-red-zone -mno-sse -mno-sse2 -mcmodel=kernel -Wall -Wextra -Iinclude -c -o %s %s",\n' '$(CC)' '$(TEST_OBJ)' '$(TEST_SRC)' >> $@
	printf '    "file": "%s"\n' '$(TEST_SRC)' >> $@
	printf '  },\n' >> $@
	printf '  {\n' >> $@
	printf '    "directory": "%s",\n' '$(CURDIR)' >> $@
	printf '    "command": "%s -ffreestanding -nostdlib -nostdinc -fno-stack-protector -fno-pic -fno-pie -mno-red-zone -mno-sse -mno-sse2 -mcmodel=kernel -Wall -Wextra -Iinclude -c -o %s %s",\n' '$(CC)' '$(TEST_STRING_OBJ)' '$(TEST_STRING_SRC)' >> $@
	printf '    "file": "%s"\n' '$(TEST_STRING_SRC)' >> $@
	printf '  },\n' >> $@
	printf '  {\n' >> $@
	printf '    "directory": "%s",\n' '$(CURDIR)' >> $@
	printf '    "command": "%s -ffreestanding -nostdlib -nostdinc -fno-stack-protector -fno-pic -fno-pie -mno-red-zone -mno-sse -mno-sse2 -mcmodel=kernel -Wall -Wextra -Iinclude -c -o %s %s",\n' '$(CC)' '$(TEST_INTERRUPT_OBJ)' '$(TEST_INTERRUPT_SRC)' >> $@
	printf '    "file": "%s"\n' '$(TEST_INTERRUPT_SRC)' >> $@
	printf '  }\n' >> $@
	printf ']\n' >> $@

compile_commands: $(COMPILE_COMMANDS)

run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 128M -nographic

debug: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 128M -nographic -s -S &
	@echo "QEMU waiting for GDB on :1234"
	@echo "Run: gdb $(KERNEL_ELF) -ex 'target remote :1234'"

clean:
	rm -rf $(BUILD_DIR) $(COMPILE_COMMANDS)
