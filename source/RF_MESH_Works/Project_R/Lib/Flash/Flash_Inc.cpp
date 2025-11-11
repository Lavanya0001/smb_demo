#include "Flash_Inc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/flash.h"
#include "hardware/flash.h"
 
#include "PIO.h"
uint8_t Flash_buf[FLASH_PAGE_SIZE] = {0};

static void __not_in_flash_func(flash_erase_call)(void *param){
    uint32_t offset = (uint32_t)param;
    hard_assert((offset % FLASH_SECTOR_SIZE) == 0);
    flash_range_erase(offset, FLASH_SECTOR_SIZE);
}
void flash_erase(uint32_t offset){
    uint32_t primask = save_and_disable_interrupts(); // or __disable_irq() on ARM
    flash_erase_call((void*)(uintptr_t)offset);
    restore_interrupts(primask); // or __enable_irq()
}

static void __not_in_flash_func(flash_program_call)(void *param){
    uint32_t offset = ((uintptr_t*)param)[0];
    const uint8_t *data = (const uint8_t *)((uintptr_t*)param)[1];
    flash_range_program(offset, data, FLASH_PAGE_SIZE);
}
void flash_write_page(uint32_t offset, uint8_t *Write_buf){
    uintptr_t params[] = { (uintptr_t)offset, (uintptr_t)Write_buf };
    uint32_t primask = save_and_disable_interrupts();
    flash_program_call((void*)params);
    restore_interrupts(primask);
}
void flash_read_page(uint64_t offset){
    hard_assert((offset % FLASH_PAGE_SIZE) == 0);

    // Flash memory is memory-mapped starting at XIP_BASE
    const uint8_t *flash_ptr = (const uint8_t *)(XIP_BASE + offset);

    // Copy one page into RAM buffer
    memcpy(Flash_buf, flash_ptr, FLASH_PAGE_SIZE);
    for(int i = 0; i < FLASH_PAGE_SIZE; i++){
        uart_printf("%d\t",Flash_buf[i]);
    }
    uart_printf("\n");
}

uint8_t flash_read_byte(uint64_t offset){
    uint8_t byte;
    hard_assert((offset % FLASH_PAGE_SIZE) == 0);

    const uint8_t *flash_ptr = (const uint8_t *)(XIP_BASE + offset);
    memcpy(Flash_buf, flash_ptr, FLASH_PAGE_SIZE);
    byte = Flash_buf[0];
    uart_printf("Flag Byte = 0b%08b\n",byte);

    return byte;
}