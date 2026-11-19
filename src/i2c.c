#include <avr/io.h>
#include <ssd1306/i2c.h>
#include <ssd1306/util.h>
#include <stdio.h>
#include <util/twi.h>

#define I2C_STATUS (TWSR & 0xf8)
#define BLOCK_I2C()                                                            \
  while (!(TWCR & (1 << TWINT))) {                                             \
  }

static void i2c_error() {
  static char buffer[64];
  snprintf(buffer, sizeof(buffer), "I2C ERROR. STATUS=0x%02x\r\n", TWSR & 0xf8);
  uart_transmit(buffer);
  while (1)
    ;
}

void i2c_init() {
  // 100khz i2c clock (standard)
  TWSR |= (1 << TWPS0);
  TWBR = 18;
}

void i2c_send_byte(const uint8_t byte) {
  TWDR = byte;
  TWCR = (1 << TWINT) | (1 << TWEN);
  BLOCK_I2C();
  if (I2C_STATUS != TW_MT_DATA_ACK && I2C_STATUS != TW_MT_SLA_ACK) {
    i2c_error();
  }
}

void i2c_send_bytes(const uint8_t *const data, uint32_t n) {
    for (uint32_t i = 0; i < n; i++) {
        i2c_send_byte(data[i]);
    }
}

void i2c_start() {
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
  BLOCK_I2C();
  if (I2C_STATUS != TW_START) {
    i2c_error();
  }
}

void i2c_stop() {
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
  while ((TWCR & (1 << TWSTO)))
    ;
}