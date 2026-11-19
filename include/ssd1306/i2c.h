#ifndef SSD1306_I2C_H_
#define SSD1306_I2C_H_

#include <stdint.h>

#define I2C_WRITE   (0u)
#define I2C_READ    (1u)

void i2c_init();
void i2c_start();
void i2c_stop();
void i2c_send_byte(uint8_t byte);
void i2c_send_bytes(const uint8_t *data, uint32_t n);

#endif // SSD1306_I2C_H_
