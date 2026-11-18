MCU		= atmega328p
F_CPU	= 16000000UL
BAUD	= 115200
PORT	= /dev/ttyUSB0
CC		= avr-gcc
OBJCOPY	= avr-objcopy
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -O0 -Wall -Wextra -std=c11 -Iinclude
TARGET	= ssd1306
SER_BAUD= 9600

all: $(TARGET).hex

$(TARGET).elf: main.c
	$(CC) $(CFLAGS) -o $@ $<

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	avr-size --mcu=$(MCU) -C $<

flash: $(TARGET).hex
	avrdude -c arduino -p $(MCU) -P $(PORT) -b $(BAUD) -U flash:w:$<
	picocom -b $(SER_BAUD) $(PORT)

fuses:
	avrdude -c arduino -p $(MCU) -P $(PORT) -b $(BAUD) -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h

clean:
	rm -f $(TARGET).elf $(TARGET).hex

.PHONY: all flash fuses clean
