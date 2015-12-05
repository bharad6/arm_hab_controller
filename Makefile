# This file was automagically generated by mbed.org. For more information, 
# see http://mbed.org/handbook/Exporting-to-GCC-ARM-Embedded

GCC_BIN = 
PROJECT = arm_hab_controller
OBJECTS = ./main.o ./SDFileSystem/SDFileSystem.o ./SDFileSystem/SDCRC.o ./SDFileSystem/FATFileSystem/FATFileSystem.o ./SDFileSystem/FATFileSystem/FATFileHandle.o ./SDFileSystem/FATFileSystem/FATDirHandle.o ./SDFileSystem/FATFileSystem/ChaN/diskio.o ./SDFileSystem/FATFileSystem/ChaN/ccsbcs.o ./SDFileSystem/FATFileSystem/ChaN/ff.o ./TinyGPS/TinyGPS.o ./Pressure/MS5803.o ./Temperature/TMP102.o ./PID/PID.o ./Watchdog/Watchdog.o ./RingBuffer/RingBuffer.o ./TaskManager/TaskManager.o ./ScheduleEvent/ScheduleEvent.o ./InterruptEvent/InterruptEvent.o ./IridiumSBD/IridiumSBD2.o ./millis/millis.o
SYS_OBJECTS = ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_flash_ramfunc.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/board.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/cmsis_nvic.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/hal_tick.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/mbed_overrides.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/retarget.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/startup_stm32l152xe.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_adc.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_adc_ex.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_comp.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_cortex.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_crc.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_cryp.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_cryp_ex.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_dac.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_dac_ex.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_dma.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_flash.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_flash_ex.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_smartcard.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_gpio.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_i2c.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_i2s.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_irda.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_iwdg.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_lcd.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_nor.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_opamp.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_opamp_ex.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_pcd.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_pcd_ex.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_pwr.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_pwr_ex.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_rcc.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_rcc_ex.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_rtc.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_rtc_ex.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_sd.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_spi.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_spi_ex.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_sram.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_tim.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_tim_ex.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_uart.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_usart.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_hal_wwdg.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_ll_fsmc.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/stm32l1xx_ll_sdmmc.o ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/system_stm32l1xx.o 
INCLUDE_PATHS = -I. -I./millis -I./IridiumSBD -I./InterruptEvent -I./TaskManager -I./ScheduleEvent -I./RingBuffer -I./TaskContext -I./PID -I./Watchdog -I./Pressure -I./Temperature -I./SDFileSystem -I./TinyGPS -I./mbed -I./SDFileSystem/FATFileSystem -I./SDFileSystem/FATFileSystem/ChaN -I./mbed/TARGET_NUCLEO_L152RE -I./mbed/TARGET_NUCLEO_L152RE/TARGET_STM -I./mbed/TARGET_NUCLEO_L152RE/TARGET_STM/TARGET_STM32L1 -I./mbed/TARGET_NUCLEO_L152RE/TARGET_STM/TARGET_STM32L1/TARGET_NUCLEO_L152RE -I./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM 
LIBRARY_PATHS = -L./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM 
LIBRARIES = -lmbed 
LINKER_SCRIPT = ./mbed/TARGET_NUCLEO_L152RE/TOOLCHAIN_GCC_ARM/STM32L152XE.ld

############################################################################### 
AS      = $(GCC_BIN)arm-none-eabi-as
CC      = $(GCC_BIN)arm-none-eabi-gcc
CPP     = $(GCC_BIN)arm-none-eabi-g++
LD      = $(GCC_BIN)arm-none-eabi-gcc
OBJCOPY = $(GCC_BIN)arm-none-eabi-objcopy
OBJDUMP = $(GCC_BIN)arm-none-eabi-objdump
SIZE    = $(GCC_BIN)arm-none-eabi-size 


CPU = -mcpu=cortex-m3 -mthumb 
CC_FLAGS = $(CPU) -c -g -fno-common -fmessage-length=0 -Wall -Wextra -fno-exceptions -ffunction-sections -fdata-sections -fomit-frame-pointer -MMD -MP
CC_SYMBOLS = -DTARGET_FF_ARDUINO -DTOOLCHAIN_GCC_ARM -DTOOLCHAIN_GCC -DMBED_BUILD_TIMESTAMP=1446416315.25 -DTARGET_FF_MORPHO -DTARGET_CORTEX_M -DARM_MATH_CM3 -DTARGET_STM32L1 -DTARGET_STM -D__CORTEX_M3 -DTARGET_NUCLEO_L152RE -DTARGET_M3 -DTARGET_STM32L152RE -D__MBED__=1 

LD_FLAGS = $(CPU) -Wl,--gc-sections --specs=nano.specs -u _printf_float -u _scanf_float -Wl,--wrap,main -Wl,-Map=$(PROJECT).map,--cref
LD_SYS_LIBS = -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys


ifeq ($(DEBUG), 1)
  CC_FLAGS += -DDEBUG -O0
else
  CC_FLAGS += -DNDEBUG -Os
endif

.PHONY: all clean lst size

all: $(PROJECT).bin $(PROJECT).hex size


clean:
	rm -f $(PROJECT).bin $(PROJECT).elf $(PROJECT).hex $(PROJECT).map $(PROJECT).lst $(OBJECTS) $(DEPS)


.asm.o:
	$(CC) $(CPU) -c -x assembler-with-cpp -o $@ $<
.s.o:
	$(CC) $(CPU) -c -x assembler-with-cpp -o $@ $<
.S.o:
	$(CC) $(CPU) -c -x assembler-with-cpp -o $@ $<

.c.o:
	$(CC)  $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu99   $(INCLUDE_PATHS) -o $@ $<

.cpp.o:
	$(CPP) $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu++98 -fno-rtti $(INCLUDE_PATHS) -o $@ $<



$(PROJECT).elf: $(OBJECTS) $(SYS_OBJECTS)
	$(LD) $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LIBRARY_PATHS) -o $@ $^ $(LIBRARIES) $(LD_SYS_LIBS) $(LIBRARIES) $(LD_SYS_LIBS)


$(PROJECT).bin: $(PROJECT).elf
	$(OBJCOPY) -O binary $< $@

$(PROJECT).hex: $(PROJECT).elf
	@$(OBJCOPY) -O ihex $< $@

$(PROJECT).lst: $(PROJECT).elf
	@$(OBJDUMP) -Sdh $< > $@

lst: $(PROJECT).lst

size: $(PROJECT).elf
	$(SIZE) $(PROJECT).elf

DEPS = $(OBJECTS:.o=.d) $(SYS_OBJECTS:.o=.d)
-include $(DEPS)


