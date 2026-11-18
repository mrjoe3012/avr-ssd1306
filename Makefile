MCU		= atmega328p
F_CPU	= 16000000UL
BAUD	= 115200
PORT	= /dev/ttyUSB0
CC		= avr-gcc
OBJCOPY	= avr-objcopy
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -O0 -Wall -Wextra -std=c11 -Iinclude -I.
TARGET	= ssd1306
SER_BAUD= 9600

all: $(TARGET).hex

$(TARGET).elf: src/main.c
	$(CC) $(CFLAGS) -o $@ $<

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	avr-size --mcu=$(MCU) -C $<

flash: $(TARGET).hex
	avrdude -c arduino -p $(MCU) -P $(PORT) -b $(BAUD) -U flash:w:$<
	picocom -b $(SER_BAUD) $(PORT)

clean:
	rm -f $(TARGET).elf $(TARGET).hex

.PHONY: all flash fuses clean
