MCU		?= atmega328p
F_CPU	?= 16000000UL
CC		= avr-gcc
AR		= avr-ar
OBJCOPY	= avr-objcopy
EXTRA_CFLAGS ?= 
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -O3 -Wall -Wextra -std=c11 -Iinclude $(DEFINES)
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin
LIB_DIR = $(BUILD_DIR)/lib
INCLUDE_DIR = $(BUILD_DIR)/include
SOURCE_DIR = src
SOURCES = $(SOURCE_DIR)/i2c.c $(SOURCE_DIR)/ssd1306.c $(SOURCE_DIR)/util.c
OBJECTS = $(SOURCES:$(SOURCE_DIR)/%.c=$(BUILD_DIR)/%.o)
LIB_TARGET = ssd1306.a
EXAMPLES_DIR = $(SOURCE_DIR)/examples
EXAMPLE_TARGETS = block text

all: ssd1306 examples

ssd1306: $(LIB_DIR)/$(LIB_TARGET) includes

prep:
	mkdir -p $(BUILD_DIR) $(LIB_DIR) $(INCLUDE_DIR) $(BIN_DIR)

includes: | prep
	cp -r include/ssd1306 $(INCLUDE_DIR)/

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c | prep
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_DIR)/$(LIB_TARGET): $(OBJECTS) | prep
	$(AR) rcs $@ $^

$(BIN_DIR)/%.elf: $(EXAMPLES_DIR)/%.c $(LIB_DIR)/$(LIB_TARGET) | prep
	$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/%.hex: $(BIN_DIR)/%.elf | prep
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	avr-size --mcu=$(MCU) -C $< || true

examples: prep $(addprefix $(BIN_DIR)/,$(EXAMPLE_TARGETS:%=%.hex))

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean examples prep includes ssd1306
.INTERMEDIATE: $(OBJECTS)
