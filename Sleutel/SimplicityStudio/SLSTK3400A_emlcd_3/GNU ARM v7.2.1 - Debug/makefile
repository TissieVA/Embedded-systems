################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include emlib/subdir.mk
-include Drivers/subdir.mk
-include CMSIS/EFM32HG/subdir.mk
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
all: SLSTK3400A_emlcd_3.axf

# Tool invocations
SLSTK3400A_emlcd_3.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m0plus -mthumb -T "SLSTK3400A_emlcd_3.ld" -Xlinker --gc-sections -Xlinker -Map="SLSTK3400A_emlcd_3.map" --specs=nano.specs -o SLSTK3400A_emlcd_3.axf "./CMSIS/EFM32HG/startup_efm32hg.o" "./CMSIS/EFM32HG/system_efm32hg.o" "./Drivers/display.o" "./Drivers/displayls013b7dh03.o" "./Drivers/displaypalemlib.o" "./Drivers/retargetio.o" "./Drivers/retargettextdisplay.o" "./Drivers/textdisplay.o" "./Drivers/udelay.o" "./emlib/em_assert.o" "./emlib/em_cmu.o" "./emlib/em_core.o" "./emlib/em_emu.o" "./emlib/em_gpio.o" "./emlib/em_pcnt.o" "./emlib/em_prs.o" "./emlib/em_rtc.o" "./emlib/em_usart.o" "./src/stopwatch.o" -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: SLSTK3400A_emlcd_3.hex'
	arm-none-eabi-objcopy -O ihex "SLSTK3400A_emlcd_3.axf" "SLSTK3400A_emlcd_3.hex"
	@echo ' '

	@echo 'Building bin file: SLSTK3400A_emlcd_3.bin'
	arm-none-eabi-objcopy -O binary "SLSTK3400A_emlcd_3.axf" "SLSTK3400A_emlcd_3.bin"
	@echo ' '

	@echo 'Building s37 file: SLSTK3400A_emlcd_3.s37'
	arm-none-eabi-objcopy -O srec "SLSTK3400A_emlcd_3.axf" "SLSTK3400A_emlcd_3.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "SLSTK3400A_emlcd_3.axf" -A
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) SLSTK3400A_emlcd_3.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
