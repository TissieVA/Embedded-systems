################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include emlib/subdir.mk
-include CMSIS/EFM32HG/subdir.mk
-include BSP/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Oef1.axf

# Tool invocations
Oef1.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m0plus -mthumb -T "Oef1.ld" -Xlinker --gc-sections -Xlinker -Map="Oef1.map" --specs=nano.specs -o Oef1.axf "./BSP/bsp_stk_leds.o" "./CMSIS/EFM32HG/startup_efm32hg.o" "./CMSIS/EFM32HG/system_efm32hg.o" "./emlib/em_system.o" "./src/1_registers.o" "./src/1_registers_solution.o" -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: Oef1.hex'
	arm-none-eabi-objcopy -O ihex "Oef1.axf" "Oef1.hex"
	@echo ' '

	@echo 'Building bin file: Oef1.bin'
	arm-none-eabi-objcopy -O binary "Oef1.axf" "Oef1.bin"
	@echo ' '

	@echo 'Building s37 file: Oef1.s37'
	arm-none-eabi-objcopy -O srec "Oef1.axf" "Oef1.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "Oef1.axf" -A
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) Oef1.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
