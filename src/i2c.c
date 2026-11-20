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

I2CRetCode i2c_init(const uint32_t clockSpeed) {
  if (clockSpeed == 0) {
    return I2C_R_CLOCK_SPEED_IMPOSSIBLE;
  }
  // iterate through possible prescaler values to find a suitable
  // configuration for the desired clock speed.
  // this may not be possible depending on the desired clock speed
  // and the mcu clock speed (F_CPU)
  uint8_t i, prescaler, twbrVal;
  for (i = 0, prescaler = 1; i < 4; i++, prescaler <<= 2) {
    // equation from atmega328p datasheet 21.5.2
    uint64_t A = 16ull * clockSpeed;
    if (A > F_CPU) {
      continue;
    }
    uint64_t numerator = F_CPU - A;
    uint64_t denom = 2ull * prescaler * clockSpeed;
    if (numerator >= denom && numerator % denom == 0) {
      uint64_t div = numerator / denom;
      if (div <= 255) {
        twbrVal = div;
        break;
      }
    }
  }
  if (i >= 4) {
    return I2C_R_CLOCK_SPEED_IMPOSSIBLE;
  }

  TWSR &= ~((1 << TWPS0) | (1 << TWPS1));
  TWSR |= ((i & 1) << TWPS0) | (((i & 2) >> 1) << TWPS1);
  TWBR = twbrVal;

  return I2C_R_OK;
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