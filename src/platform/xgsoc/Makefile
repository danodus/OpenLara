# Ref.: https://makefiletutorial.com/#makefile-cookbook

BUILD_DIR := ./build
SRC_DIRS := ./src

PYTHON = python3

RISCV_TOOLCHAIN_PATH =
RISCV_TOOLCHAIN_PREFIX = riscv-none-elf-

AS = ${RISCV_TOOLCHAIN_PATH}${RISCV_TOOLCHAIN_PREFIX}as
OBJCOPY = ${RISCV_TOOLCHAIN_PATH}${RISCV_TOOLCHAIN_PREFIX}objcopy
OBJDUMP = ${RISCV_TOOLCHAIN_PATH}${RISCV_TOOLCHAIN_PREFIX}objdump
CC = ${RISCV_TOOLCHAIN_PATH}${RISCV_TOOLCHAIN_PREFIX}gcc
CXX = ${RISCV_TOOLCHAIN_PATH}${RISCV_TOOLCHAIN_PREFIX}g++

RISCV_CC_OPT = -march=rv32imaf_zicsr -mabi=ilp32f

SRCS = start.S $(shell find -L $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.S')
OBJS := $(SRCS:%=./$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find -L $(SRC_DIRS) -type d) ../../fixed
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

DEFINES := -D__XGSOC__ -DFATFS_DEF_UINT32_AS_INT

SERIAL ?= /dev/tty.usbserial-D00039

ifeq ($(XGSOC_DIR),)
$(error "Please set XGSOC_DIR")
endif

all: $(BUILD_DIR)/program.hex

run: $(BUILD_DIR)/program.hex
	$(PYTHON) $(XGSOC_DIR)/utils/sendhex.py $(SERIAL) $(BUILD_DIR)/program.hex

program: program.hex

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/program.lst: $(BUILD_DIR)/program.elf
	mkdir -p $(dir $@)
	${OBJDUMP} --disassemble $< > $@

$(BUILD_DIR)/program.hex: $(BUILD_DIR)/program.bin
	mkdir -p $(dir $@)
	$(PYTHON) $(XGSOC_DIR)/utils/makehex.py $< > $@

$(BUILD_DIR)/program.bin: $(BUILD_DIR)/program.elf $(BUILD_DIR)/program.lst
	mkdir -p $(dir $@)
	${OBJCOPY} -O binary $(BUILD_DIR)/program.elf $(BUILD_DIR)/program.bin

$(BUILD_DIR)/%.S.o: %.S
	mkdir -p $(dir $@)
	${CC} ${RISCV_CC_OPT} -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	${CC} ${RISCV_CC_OPT} -MMD -MP -O3 $(INC_FLAGS) $(DEFINES) -fpermissive -c $< -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	${CXX} ${RISCV_CC_OPT} -std=c++17 -MMD -MP -O3 $(INC_FLAGS) $(DEFINES) -fpermissive -c $< -o $@

$(BUILD_DIR)/program.elf: $(OBJS)
	mkdir -p $(dir $@)
	${CC} ${RISCV_CC_OPT} -O3 -nostartfiles -T program.ld $(OBJS) -o $@ -lm -lstdc++

-include $(DEPS)

.PHONY: all clean run program
