#include <avr/io.h>
#include <ssd1306/util.h>

#define UBRR0_VAL ((F_CPU / (16UL * UART_BAUD)))

void uart_init() {
  // initialise UART
  UBRR0L = UBRR0_VAL;
  UBRR0H = (UBRR0_VAL >> 8);
  UCSR0B = (1 << TXEN0);
  UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
}

void uart_transmit(const char *message) {
  while (*message) {
    while (!(UCSR0A & (1 << UDRE0))) {
    }
    UDR0 = *message++;
  }
}
