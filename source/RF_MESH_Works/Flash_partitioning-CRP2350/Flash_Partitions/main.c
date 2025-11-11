//Here we created partitions and writing in flash and reading them. Need to make function calls,permissions are there & assigned to it.
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "pico/bootrom.h"
#include "boot/picobin.h"

#include "pico/flash.h"
#include "hardware/flash.h"

#define UART_ID         uart0
#define BAUD_RATE       115200

#define UART_TX_PIN     34
#define UART_RX_PIN     35

#define FLASH_FACTORY_SETTINGS        (0xC000)
#define FLASH_FACTORY_SETTINGS_SIZE   (128 * 1024)

#define FLASH_USER                    (0x9300)
#define FALSH_USER_SIZE               (512 * 1024)

char arr[255];
void u_printf(const char *fmt, ...) {
    char buf[255];  
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    uart_puts(UART_ID, buf);
}

int main()
{
    stdio_init_all();

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART_AUX);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART_AUX);


    static uint8_t flash_buffer[256] = {0x01,0x02,0x03,0x04,0x05};

    //Erasing and porgramming using normal flash commands
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_FACTORY_SETTINGS,FLASH_FACTORY_SETTINGS_SIZE);
    flash_range_program(FLASH_FACTORY_SETTINGS,flash_buffer,sizeof(flash_buffer));
    restore_interrupts(ints);

    static uint8_t flash_Rx_buffer[5] = {};
    int ret;
    cflash_flags_t flags;

    flags.flags = (CFLASH_OP_VALUE_PROGRAM << CFLASH_OP_LSB) | (CFLASH_SECLEVEL_VALUE_SECURE << CFLASH_SECLEVEL_LSB);
    ret = rom_add_flash_runtime_partition(FLASH_FACTORY_SETTINGS,FLASH_FACTORY_SETTINGS_SIZE,(PICOBIN_PARTITION_PERMISSION_S_R_BITS ));
    //Only secure read is allowing so this factory setting is ony now Read only
    if(ret != BOOTROM_OK){
        u_printf("For partition creation,Flash op failed = %d\n",ret);
    }

    ret = rom_flash_op(flags,(XIP_BASE + FLASH_FACTORY_SETTINGS),sizeof(flash_buffer),flash_buffer);
    if(ret == BOOTROM_OK){
        u_printf("Write Sucessful\n");
        sleep_ms(1000);
    }else{
        u_printf("For write,flash op failed = %d\n",ret);
        sleep_ms(1000);
    }

    flags.flags = (CFLASH_OP_VALUE_READ << CFLASH_OP_LSB) | (CFLASH_SECLEVEL_VALUE_SECURE << CFLASH_SECLEVEL_LSB);
    ret = rom_flash_op(flags,(XIP_BASE + FLASH_FACTORY_SETTINGS),sizeof(flash_Rx_buffer),flash_Rx_buffer);
    if(ret == BOOTROM_OK){
        u_printf("Read Sucessful\n");
        for(int i = 0; i < 5;i++){
            u_printf("0x%0x,",flash_Rx_buffer[i]);
        }
        sleep_ms(1000);
    }else{
        u_printf("Flash read,Flash op failed = %d\n",ret);
        sleep_ms(1000);
    }

}
