#include <avr/pgmspace.h>
#include <ssd1306/i2c.h>
#include <ssd1306/ssd1306.h>
#include <ssd1306/util.h>
#include <stdio.h>

#define CONTROL_BYTE(co, dc) ((co << 7) | (dc << 6))

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
    0x20, // configure addressing mode (horizontal)
    0x00,
    0x21, // configure column start/end
    0x0,
    0x7f,
    0x22, // configure page start/end
    0x00,
    0x07,
    0x40, // set start line register
    0xa0, // configure remap
    0xA8, // mux ratio (128x64)
    0x3f,
    0xc0, // com0 scan dir
    0xd3, // display offset
    0x00,
    0xda, // com pin config (alt)
    0x12,
    0xd5, // clock ratio
    0x80,
    0xd9, // pre-charge
    0x22,
    0xdb, // vcom deselect
    0x20,
    0x2e, // deactivate scrolling
    0xaf, // display on
};

void ssd1306_init() {
  i2c_start();
  uart_transmit("SENDING INITIALISATION BYTES...\r\n");
  for (size_t i = 0; i < sizeof(ssd1306_reset) / sizeof(uint8_t); i++) {
    i2c_send_byte(pgm_read_byte(ssd1306_reset + i));
  }
  uart_transmit("FINISHED SENDING INITIALISATION BYTES...\r\n");
  i2c_stop();
  uart_transmit("SSD1306 INIT COMPLETE\r\n");
}

static inline void ssd1306_prepare_write(const uint8_t pageBegin,
                                         const uint8_t pageEnd,
                                         const uint8_t colBegin,
                                         const uint8_t colEnd) {
  i2c_send_byte((SSD1306_ADDRESS << 1) | I2C_WRITE);
  i2c_send_byte(CONTROL_BYTE(1, 0));
  i2c_send_byte(0x21); // col start/end
  i2c_send_byte(CONTROL_BYTE(1, 0));
  i2c_send_byte(colBegin);
  i2c_send_byte(CONTROL_BYTE(1, 0));
  i2c_send_byte(colEnd);
  i2c_send_byte(CONTROL_BYTE(1, 0));
  i2c_send_byte(0x22); // page start/end
  i2c_send_byte(CONTROL_BYTE(1, 0));
  i2c_send_byte(pageBegin);
  i2c_send_byte(CONTROL_BYTE(1, 0));
  i2c_send_byte(pageEnd);
  i2c_send_byte(CONTROL_BYTE(0, 1));
}

void ssd1306_write_gddram(const uint8_t pageStartIdx, const uint8_t pageEndIdx,
                          const uint8_t colStartIdx, const uint8_t colEndIdx,
                          const uint8_t *const data, const uint32_t n,
                          const uint8_t flags) {
  i2c_start();
  ssd1306_prepare_write(pageStartIdx, pageEndIdx - 1, colStartIdx, colEndIdx - 1);
  for (uint32_t i = 0; i < n; ++i) {
    if (flags & SSD1306_F_PGM) {
        i2c_send_byte(pgm_read_byte(data + i));
    } else {
        i2c_send_byte(data[i]);
    }
  }
  i2c_stop();
}

void ssd1306_fill_gddram(uint8_t pageStartIdx, uint8_t pageEndIdx,
                         uint8_t colStartIdx, uint8_t colEndIdx, uint8_t value) {
  i2c_start();
  ssd1306_prepare_write(pageStartIdx, pageEndIdx - 1, colStartIdx, colEndIdx - 1);
  const uint32_t n = (uint32_t)(colEndIdx - colStartIdx) * (pageEndIdx - pageStartIdx);
  for (uint32_t i = 0; i < n; ++i) {
    i2c_send_byte(value);
  }
  i2c_stop();
}
