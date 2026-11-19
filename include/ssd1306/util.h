#ifndef SSD1306_UTIL_H_
#define SSD1306_UTIL_H_

#ifndef UART_BAUD
#define UART_BAUD 9600
#endif

void uart_init();
void uart_transmit(const char *message);

#endif // SSD1306_UTIL_H_
