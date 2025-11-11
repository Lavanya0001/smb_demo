#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "pico/multicore.h"

#define UART_ID uart0
#define BAUD_RATE 115200

#define UART_TX_PIN 34
#define UART_RX_PIN 35

#define FLAG_VALUE 123

void core1_entry() {
    multicore_fifo_push_blocking(FLAG_VALUE);
    uint32_t g = multicore_fifo_pop_blocking();

    if (g != FLAG_VALUE)
        uart_puts(UART_ID, "Hmm, that's not right on core 1!\n");
    else
        uart_puts(UART_ID, "Its all gone well on core 1!");

    while (1)
        tight_loop_contents();
}


int main()
{
    stdio_init_all();
    sleep_ms(1000);


    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART_AUX);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART_AUX);
    
    uart_puts(UART_ID, " Hello, Multicore!\n");

     multicore_launch_core1(core1_entry);
        sleep_ms(2000);

    uint32_t g = multicore_fifo_pop_blocking();
    if (g != FLAG_VALUE)
        uart_puts(UART_ID, "Hmm, that's not right on core 0!\n");
    else {
        multicore_fifo_push_blocking(FLAG_VALUE);
        uart_puts(UART_ID, "It's all gone well on core 0!");
    }
}
