/*Created a JSON file pt.json and trying to use that in practical programming..*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "boot/picobin.h"

#define UART0_TX    34
#define UART0_RX    35
#define UART0_ID    uart0


char arr[255];
void u_printf(const char *fmt, ...) {
    char buf[255];  
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    uart_puts(UART0_ID, buf);
}

int main() {
    stdio_init_all();

    gpio_set_function(UART0_TX, GPIO_FUNC_UART_AUX);
    gpio_set_function(UART0_RX, GPIO_FUNC_UART_AUX);

    uart_init(uart0, 115200);
    while(1){
        u_printf("Hi..\n");
    }
   
}