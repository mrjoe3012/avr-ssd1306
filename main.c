#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <util/twi.h>
#include "output.h"

#define UART_BAUD 9600
#define UBRR0_VAL ((F_CPU / (16UL * UART_BAUD)))

#define SSD1306_ADDRESS 0b0111100
#define I2C_WRITE 0
#define I2C_READ 1

#define BLOCK_I2C() while(!(TWCR & (1 << TWINT))) {}
#define I2C_STATUS (TWSR & 0xf8)

#define CONTROL_BYTE(co, dc) ((co << 7) | (dc << 6))

void uart_transmit(const char* message) {
    while (*message) {
        while (!(UCSR0A & (1 << UDRE0))) {}
        UDR0 = *message++;
    }
}

void uart_init() {
    // initialise UART
    UBRR0L = UBRR0_VAL;
    UBRR0H = (UBRR0_VAL >> 8);
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
}

char buffer[128];

void i2c_init() {
    // 100khz i2c clock (standard)
    TWSR |= (1 << TWPS0);
    TWBR = 18;
}

void i2c_error() {
    snprintf(buffer, sizeof(buffer), "I2C ERROR. STATUS=0x%02x\r\n", TWSR & 0xf8);
    uart_transmit(buffer);
    while (1);
}

void i2c_send(uint8_t byte) {
    TWDR = byte;
    TWCR = (1 << TWINT) | (1 << TWEN);
    BLOCK_I2C();
    if (I2C_STATUS != TW_MT_DATA_ACK && I2C_STATUS != TW_MT_SLA_ACK) {
        i2c_error();
    }
}

void i2c_start() {
    uart_transmit("STARTING I2C TRANSACTION\r\n");
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    BLOCK_I2C();
    if (I2C_STATUS != TW_START) {
        i2c_error();
    }
    uart_transmit("STARTED I2C TRANSACTION\r\n");
}

void i2c_stop() {
    uart_transmit("STOPPING I2C TRANSACTION\r\n");
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    while ((TWCR & (1 << TWSTO)));
    uart_transmit("STOPPED I2C TRANSACTION\r\n");
}

static const uint8_t ssd1306_reset[] PROGMEM = {
    ((SSD1306_ADDRESS << 1) | I2C_WRITE),
    CONTROL_BYTE(0, 0), // control byte
    0xae,               // display off
    0x8d,               // charge pump configure
    0x14,               // charge pump on
    0xa4,               // display ram
    0xa6,               // normal mode
    0xa3,               // configure vertical scroll area
    0x00,
    0x40,
    0x20,               // configure addressing mode (horizontal)
    0x00,
    0x21,               // configure column start/end
    0x0,
    0x7f,
    0x22,               // configure page start/end
    0x00,
    0x07,
    0x40,               // set start line register
    0xa0,               // configure remap
    0xA8,               // mux ratio (128x64)
    0x3f,
    0xc0,               // com0 scan dir
    0xd3,               // display offset
    0x00,
    0xda,               // com pin config (alt)
    0x12, 
    0xd5,               // clock ratio
    0x80,
    0xd9,               // pre-charge
    0x22,
    0xdb,               // vcom deselect
    0x20,
    0x2e,               // deactivate scrolling
    0xaf,               // display on
};

void ssd1306_init() {
    i2c_start();
    uart_transmit("SENDING INITIALISATION BYTES...\r\n");
    for (size_t i = 0; i < sizeof(ssd1306_reset) / sizeof(uint8_t); i++) {
        i2c_send(pgm_read_byte(ssd1306_reset + i));
    }
    uart_transmit("FINISHED SENDING INITIALISATION BYTES...\r\n");
    i2c_stop();
    uart_transmit("SSD1306 INIT COMPLETE\r\n");
}

// split into _P
// split into flags
void ssd1306_write_gddram(const uint8_t pageIdx, const uint8_t colIdx, const uint8_t * const data, const size_t n, const uint8_t pgm, const uint8_t fill) {
    i2c_start();
    i2c_send((SSD1306_ADDRESS << 1) | I2C_WRITE);
    i2c_send(CONTROL_BYTE(1, 0));
    i2c_send(0x21); // col start/end
    i2c_send(CONTROL_BYTE(1, 0));
    i2c_send(colIdx);
    i2c_send(CONTROL_BYTE(1, 0));
    i2c_send(0x7f);
    i2c_send(CONTROL_BYTE(1, 0));
    i2c_send(0x22); // page start/end
    i2c_send(CONTROL_BYTE(1, 0));
    i2c_send(pageIdx);
    i2c_send(CONTROL_BYTE(1, 0));
    i2c_send(0x07);
    i2c_send(CONTROL_BYTE(0, 1));
    for (size_t i = 0; i < n; ++i) {
        if (fill) {
            if (pgm) {
                i2c_send(pgm_read_byte(data));
            } else {
                i2c_send(*data);
            }
        } else {
            if (pgm) {
                i2c_send(pgm_read_byte(data + i));
            } else {
                i2c_send(data[i]);
            }
        }
    }
    i2c_stop();
}

uint8_t checker[128];

int main(void) {
    uart_init();
    uart_transmit("STARTING...\r\n");

    i2c_init();
    ssd1306_init();
    uint8_t zero = 0;
    ssd1306_write_gddram(0, 0, &zero, 128ul*8, 0, 1);


    // debug led
    DDRB |= (1 << DDB1);
    uint8_t state = 0;

    uint8_t i = 0;

    uint8_t sx = 0;
    uint8_t sy = 0;
    uint8_t c = '1';

    while(1) {
        if (state) {
            PORTB &= ~(1 << PB1);
        } else {
            PORTB |= (1 << PB1);
        }
        state = !state;
        i++;

        if (i >= 5) {
            uint32_t idx = FONT_FONTNAME_CIDX(c);
            uint32_t off = pgm_read_dword(FONT_FONTNAME_OFFSETS + idx);
            uint8_t w = pgm_read_byte(FONT_FONTNAME_WIDTHS + idx);
            const uint8_t *fontDataP = FONT_FONTNAME_GBUF + off;
            if (sx + w > 128) {
                sy += FONT_FONTNAME_NUM_PAGES;
                sx = 0;
            }
            if (sy >= 8) {
                ssd1306_write_gddram(
                    0, 0, &zero, 128*8, 0, 1
                );
                sy = 0;
                sx = 0;
            }
            for (uint8_t page = 0; page < FONT_FONTNAME_NUM_PAGES; page++) {
                // clear
                // ssd1306_write_gddram(
                //     sy + page, sx, &zero, w, 0, 1
                // );
                // _delay_ms(100);
                // char
                ssd1306_write_gddram(
                    sy + page, sx, fontDataP + w * page, w, 1, 0
                );
            }
            sx += w;
            c++;
            if (c > 126) {
                c = 33;
            }
        }

        _delay_ms(500);
    }
}
