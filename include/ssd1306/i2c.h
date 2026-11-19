#ifndef SSD1306_I2C_H_
#define SSD1306_I2C_H_

#include <stdint.h>

#define I2C_WRITE   (0u)
#define I2C_READ    (1u)

typedef enum {
  I2C_R_OK = 0,
  I2C_R_FAILED
} I2CRetCode;

// if set to 0, busy waits, otherwise sleeps the cpu and uses interrupt
// if set to 1, you must enable interrupts!
#ifndef I2C_USE_INTERRUPT
#define I2C_USE_INTERRUPT 0
#endif

void i2c_init();
I2CRetCode i2c_start();
void i2c_stop();
I2CRetCode i2c_send_byte(uint8_t byte);
I2CRetCode i2c_send_bytes(const uint8_t *data, uint32_t n);

#endif // SSD1306_I2C_H_
