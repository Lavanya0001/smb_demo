#include "pico/stdlib.h"
#include "stdio.h"
#include "hardware/structs/scb.h"

#define BOOTLOADER_LED_PIN 25  // Onboard LED for most RP2040 boards
#define APP_ADDRESS 0x1000C800

typedef void (*entry_point_t)(void);

void jump_to_application(void) {
    uint32_t app_sp = *(uint32_t *)(APP_ADDRESS + 0);      // Initial stack pointer
    uint32_t app_entry_addr = *(uint32_t *)(APP_ADDRESS + 4); // Reset handler
    entry_point_t app_entry = (entry_point_t)app_entry_addr;

        // Set vector table offset register to application's vector table;
    scb_hw->vtor = (uintptr_t)(APP_ADDRESS);
    __asm volatile ("msr msp, %0" :: "r" (app_sp));  // Set stack pointer

    app_entry();  // Jump to app
}

int main() {
    stdio_init_all();
    sleep_ms(1000);  // Optional delay for debugging

    gpio_init(BOOTLOADER_LED_PIN);
    gpio_set_dir(BOOTLOADER_LED_PIN, GPIO_OUT);

    // Blink LED 3 times to indicate bootloader is active
    for (int i = 0; i < 3; ++i) {
        gpio_put(BOOTLOADER_LED_PIN, 1);
        sleep_ms(200);
        gpio_put(BOOTLOADER_LED_PIN, 0);
        sleep_ms(200);
    }
    printf("Bootloader: Jumping to application at 0x10020000\n");

    sleep_ms(500);  // Short delay to allow message to flush
    jump_to_application();
    while (1); // Fallback trap
}
