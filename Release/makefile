################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Debug/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
Schlafzimmer.lss \

FLASH_IMAGE += \
Schlafzimmer.hex \

EEPROM_IMAGE += \
Schlafzimmer.eep \

SIZEDUMMY += \
sizedummy \

AVRDUDEDUMMY += \
avrdudedummy \


# All Target
all: Schlafzimmer.elf secondary-outputs

# Tool invocations
Schlafzimmer.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,Schlafzimmer.map -mmcu=atmega8 -o "Schlafzimmer.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

Schlafzimmer.lss: Schlafzimmer.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S Schlafzimmer.elf  >"Schlafzimmer.lss"
	@echo 'Finished building: $@'
	@echo ' '

Schlafzimmer.hex: Schlafzimmer.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -O ihex Schlafzimmer.elf  "Schlafzimmer.hex"
	@echo 'Finished building: $@'
	@echo ' '

Schlafzimmer.eep: Schlafzimmer.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex Schlafzimmer.elf  "Schlafzimmer.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: Schlafzimmer.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega8 Schlafzimmer.elf
	@echo 'Finished building: $@'
	@echo ' '

avrdudedummy: Schlafzimmer.elf
	@echo 'Invoking: AVRDude'
	C:\WinAVR-20100110\bin\avrdude -pm8 -cusbasp -u -Uflash:w:Schlafzimmer.hex:a -Ulfuse:w:0x3f:m -Uhfuse:w:0xd1:m
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(ASM_DEPS)$(EEPROM_IMAGE)$(FLASH_IMAGE)$(ELFS)$(LSS)$(AVRDUDEDUMMY)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS) Schlafzimmer.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY) $(AVRDUDEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
