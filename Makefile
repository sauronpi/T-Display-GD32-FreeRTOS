##### GD32V RISC-V MCU Makefile ######
######################################
# Target
######################################
TARGET = HelloWorld


######################################
# Source
######################################
# C sources
C_SOURCES =  \
$(wildcard GD32VF103_Firmware_Library_V1.1.5/Firmware/GD32VF103_standard_peripheral/Source/*.c) \
$(wildcard GD32VF103_Firmware_Library_V1.1.5/Firmware/GD32VF103_standard_peripheral/*.c) \
$(wildcard GD32VF103_Firmware_Library_V1.1.5/Firmware/RISCV/drivers/*.c) \
$(wildcard GD32VF103_Firmware_Library_V1.1.5/Firmware/RISCV/env_Eclipse/*.c) \
$(wildcard GD32VF103_Firmware_Library_V1.1.5/Firmware/RISCV/stubs/*.c) \
$(wildcard FreeRTOSv202212.01/FreeRTOS/Source/*.c) \
$(wildcard FreeRTOSv202212.01/FreeRTOS/Source/portable/MemMang/*.c) \
$(wildcard FreeRTOSv202212.01/FreeRTOS/Source/portable/GCC/RISC-V/*.c) \

# add your c sources here
C_SOURCES += \
$(wildcard ./*.c) \

# ASM sources
ASM_SOURCES =  \
GD32VF103_Firmware_Library_V1.1.5/Firmware/RISCV/env_Eclipse/start.S \
GD32VF103_Firmware_Library_V1.1.5/Firmware/RISCV/env_Eclipse/entry.S \
FreeRTOSv202212.01/FreeRTOS/Source/portable/GCC/RISC-V/portASM.S \


######################################
# Includes
######################################
# C includes
C_INCLUDES =  \
-I GD32VF103_Firmware_Library_V1.1.5/Firmware/GD32VF103_standard_peripheral/Include \
-I GD32VF103_Firmware_Library_V1.1.5/Firmware/GD32VF103_standard_peripheral \
-I GD32VF103_Firmware_Library_V1.1.5/Firmware/RISCV/drivers \
-I GD32VF103_Firmware_Library_V1.1.5/Firmware/RISCV/stubs \
-I FreeRTOSv202212.01/FreeRTOS/Source/include \
-I FreeRTOSv202212.01/FreeRTOS/Source/portable/GCC/RISC-V \

# add your includes here
C_INCLUDES += \
-I FreeRTOSv202212.01/FreeRTOS/Source/portable/GCC/RISC-V/chip_specific_extensions/RISCV_MTIME_CLINT_no_extensions\
-I . \

# AS includes
AS_INCLUDES = \
-I FreeRTOSv202212.01/FreeRTOS/Source/portable/GCC/RISC-V/chip_specific_extensions/RISCV_MTIME_CLINT_no_extensions\


######################################
# Building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og

# Build path
BUILD_DIR = build


######################################
# Defines
######################################
# macros for gcc
C_DEFS =  \
-D USE_STDPERIPH_DRIVER \
-D HXTAL_VALUE=8000000U \

# AS defines
AS_DEFS = 


#######################################
# Linker
#######################################
# link script
LDSCRIPT = GD32VF103_Firmware_Library_V1.1.5/Firmware/RISCV/env_Eclipse/GD32VF103xB.lds


#######################################
# Binaries
#######################################
PREFIX = riscv64-unknown-elf-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
AR = $(PREFIX)ar
SZ = $(PREFIX)size
OD = $(PREFIX)objdump
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 

#######################################
# Flags
#######################################
# architecture
ARCH = -march=rv32imac_zicsr -mabi=ilp32 -mcmodel=medlow
LINKARCH = -march=rv32imac -mabi=ilp32 -mcmodel=medlow

# compile gcc flags
CFLAGS = $(ARCH) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wl,-Bstatic #, -ffreestanding -nostdlib
ASFLAGS = $(ARCH) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wl,-Bstatic #, -ffreestanding -nostdlib

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"

# Generation a separate ELF section for each function and variable in the source file
# Cooperate -Wl,--gc-sections option to eliminating the unused code and data
# from the final executable
CFLAGS += -ffunction-sections -fdata-sections

# libraries
LIBS = -lc_nano -lm
LIBDIR = 
LDFLAGS = $(LINKARCH) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) $(PERIFLIB_SOURCES) -Wl,--no-relax -Wl,--gc-sections -Wl,-Map,$(BUILD_DIR)/$(TARGET).map -nostartfiles #-ffreestanding -nostdlib

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# Build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	@echo "CC $<"
	@$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	@echo "AS $<"
	@$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@echo "LD $@"
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@echo "OD $@"
	@$(OD) $(BUILD_DIR)/$(TARGET).elf -xS > $(BUILD_DIR)/$(TARGET).S $@
	@echo "SZ $@"
	@$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
  
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@  
  
$(BUILD_DIR):
	mkdir $@

#######################################
# Clean up
#######################################

clean:
	-rm -fR .dep $(BUILD_DIR)
  
 
#######################################
# Program
#######################################
flash: all
	$()openocd -c "adapter driver cmsis-dap; adapter speed 5000; transport select jtag" -f ./gd32vf103.cfg -c "program $(BUILD_DIR)/$(TARGET).elf" -c "reset; exit"

debug: all
	$()openocd -c "adapter driver cmsis-dap; adapter speed 5000; transport select jtag" -f ./gd32vf103.cfg 

dfu: all
	$()dfu-util -a 0 -s 0x08000000:leave -D $(BUILD_DIR)/$(TARGET).bin
