################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Startup/subdir.mk
-include Src/subdir.mk
-include Drivers/STM32F1xx_HAL_Driver/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
eSkate-Remote-RX.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
eSkate-Remote-RX.list \


# All Target
all: eSkate-Remote-RX.elf secondary-outputs

# Tool invocations
eSkate-Remote-RX.elf: $(OBJS) $(USER_OBJS) C:\Users\OTRIUM\Dropbox\OTRIUM-Personal\Altium-Projects\eSkate-Remote\RX\FW\eSkate-Remote-RX\STM32F103C8TX_FLASH.ld
	arm-none-eabi-gcc -o "eSkate-Remote-RX.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m3 -T"C:\Users\OTRIUM\Dropbox\OTRIUM-Personal\Altium-Projects\eSkate-Remote\RX\FW\eSkate-Remote-RX\STM32F103C8TX_FLASH.ld" --specs=nosys.specs -Wl,-Map="eSkate-Remote-RX.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

eSkate-Remote-RX.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "eSkate-Remote-RX.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
