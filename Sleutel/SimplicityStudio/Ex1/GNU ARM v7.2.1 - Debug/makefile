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
all: Ex1.axf

# Tool invocations
Ex1.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m0plus -mthumb -T "Ex1.ld" -Xlinker --gc-sections -Xlinker -Map="Ex1.map" --specs=nano.specs -o Ex1.axf "./BSP/bsp_stk_leds.o" "./CMSIS/EFM32HG/startup_efm32hg.o" "./CMSIS/EFM32HG/system_efm32hg.o" "./emlib/em_assert.o" "./emlib/em_cmu.o" "./emlib/em_core.o" "./emlib/em_emu.o" "./emlib/em_gpio.o" "./src/Ex1.o" -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: Ex1.hex'
	arm-none-eabi-objcopy -O ihex "Ex1.axf" "Ex1.hex"
	@echo ' '

	@echo 'Building bin file: Ex1.bin'
	arm-none-eabi-objcopy -O binary "Ex1.axf" "Ex1.bin"
	@echo ' '

	@echo 'Building s37 file: Ex1.s37'
	arm-none-eabi-objcopy -O srec "Ex1.axf" "Ex1.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "Ex1.axf" -A
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) Ex1.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
