#include <avr/interrupt.h>
#include <avr/io.h>
#include <ssd1306/i2c.h>
#include <util/twi.h>

#define I2C_STATUS (TWSR & 0xf8)

#if I2C_USE_INTERRUPT

#include <avr/interrupt.h>
#include <avr/sleep.h>
ISR(TWI_vect) {}

#endif

static inline void block_i2c() {
#if I2C_USE_INTERRUPT
  uint8_t sreg = SREG;
  cli();
  set_sleep_mode(SLEEP_MODE_IDLE);
  while (!(TWCR & (1 << TWINT))) {
    sei();
    sleep_enable();
    sleep_cpu();
    sleep_disable();
    cli();
  }
  SREG = sreg;
#else
  while (!(TWCR & (1 << TWINT)))
    ;
#endif
}

void i2c_init() {
  // 100khz i2c clock (standard)
  TWSR |= (1 << TWPS0);
  TWBR = 18;
}

I2CRetCode i2c_send_byte(const uint8_t byte) {
  TWDR = byte;
  TWCR = (1 << TWINT) | (1 << TWEN) | (I2C_USE_INTERRUPT << TWIE);
  block_i2c();
  if (I2C_STATUS != TW_MT_DATA_ACK && I2C_STATUS != TW_MT_SLA_ACK) {
    return I2C_R_FAILED;
  }
  return I2C_R_OK;
}

I2CRetCode i2c_send_bytes(const uint8_t *const data, uint32_t n) {
  for (uint32_t i = 0; i < n; i++) {
    if (i2c_send_byte(data[i]) != I2C_R_OK) {
      return I2C_R_FAILED;
    }
  }
  return I2C_R_OK;
}

I2CRetCode i2c_start() {
  TWCR =
      (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (I2C_USE_INTERRUPT << TWIE);
  block_i2c();
  if (I2C_STATUS != TW_START) {
    return I2C_R_FAILED;
  }
  return I2C_R_OK;
}

void i2c_stop() {
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
  while ((TWCR & (1 << TWSTO)))
    ;
}