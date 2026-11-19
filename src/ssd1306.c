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

bool ssd1306_init() {
  uint8_t code;
  code = i2c_start();
  if (code != I2C_R_OK) {
    return false;
  }
  for (size_t i = 0; i < sizeof(ssd1306_reset) / sizeof(uint8_t); i++) {
    code = i2c_send_byte(pgm_read_byte(ssd1306_reset + i));
    if (code != I2C_R_OK) {
      return false;
    }
  }
  i2c_stop();
  return true;
}

static inline bool ssd1306_prepare_write(const uint8_t pageBegin,
                                         const uint8_t pageEnd,
                                         const uint8_t colBegin,
                                         const uint8_t colEnd) {
  const uint8_t dataToTransfer[] = {(SSD1306_ADDRESS << 1) | I2C_WRITE,
                                    CONTROL_BYTE(1, 0),
                                    0x21, // col start/end
                                    CONTROL_BYTE(1, 0),
                                    colBegin,
                                    CONTROL_BYTE(1, 0),
                                    colEnd,
                                    CONTROL_BYTE(1, 0),
                                    0x22, // page start/end,
                                    CONTROL_BYTE(1, 0),
                                    pageBegin,
                                    CONTROL_BYTE(1, 0),
                                    pageEnd,
                                    CONTROL_BYTE(0, 1)};
  for (uint32_t i = 0; i < sizeof(dataToTransfer); i++) {
    if (i2c_send_byte(dataToTransfer[i]) != I2C_R_OK) {
      return false;
    }
  }
  return true;
}

bool ssd1306_write_gddram(const uint8_t pageStartIdx, const uint8_t pageEndIdx,
                          const uint8_t colStartIdx, const uint8_t colEndIdx,
                          const uint8_t *const data, const uint32_t n,
                          const uint8_t flags) {
  if (i2c_start() != I2C_R_OK ||
      !ssd1306_prepare_write(pageStartIdx, pageEndIdx - 1, colStartIdx,
                             colEndIdx - 1)) {
    return false;
  }
  for (uint32_t i = 0; i < n; ++i) {
    if (flags & SSD1306_F_PGM) {
      if (i2c_send_byte(pgm_read_byte(data + i)) != I2C_R_OK) {
        return false;
      }
    } else {
      if (i2c_send_byte(data[i]) != I2C_R_OK) {
        return false;
      }
    }
  }
  i2c_stop();
  return true;
}

bool ssd1306_fill_gddram(uint8_t pageStartIdx, uint8_t pageEndIdx,
                         uint8_t colStartIdx, uint8_t colEndIdx,
                         uint8_t value) {
  if (i2c_start() != I2C_R_OK ||
      !ssd1306_prepare_write(pageStartIdx, pageEndIdx - 1, colStartIdx,
                             colEndIdx - 1)) {
    return false;
  }
  const uint32_t n =
      (uint32_t)(colEndIdx - colStartIdx) * (pageEndIdx - pageStartIdx);
  for (uint32_t i = 0; i < n; ++i) {
    if (i2c_send_byte(value) != I2C_R_OK) {
      return false;
    }
  }
  i2c_stop();
  return true;
}
