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
all: Ex2_2.axf

# Tool invocations
Ex2_2.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m0plus -mthumb -T "Ex2_2.ld" -Xlinker --gc-sections -Xlinker -Map="Ex2_2.map" --specs=nano.specs -o Ex2_2.axf "./BSP/bsp_stk_leds.o" "./CMSIS/EFM32HG/startup_efm32hg.o" "./CMSIS/EFM32HG/system_efm32hg.o" "./emlib/em_assert.o" "./emlib/em_cmu.o" "./emlib/em_core.o" "./emlib/em_emu.o" "./emlib/em_gpio.o" "./emlib/em_rtc.o" "./emlib/em_system.o" "./src/2_interrupt_timer.o" -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: Ex2_2.hex'
	arm-none-eabi-objcopy -O ihex "Ex2_2.axf" "Ex2_2.hex"
	@echo ' '

	@echo 'Building bin file: Ex2_2.bin'
	arm-none-eabi-objcopy -O binary "Ex2_2.axf" "Ex2_2.bin"
	@echo ' '

	@echo 'Building s37 file: Ex2_2.s37'
	arm-none-eabi-objcopy -O srec "Ex2_2.axf" "Ex2_2.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "Ex2_2.axf" -A
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) Ex2_2.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
