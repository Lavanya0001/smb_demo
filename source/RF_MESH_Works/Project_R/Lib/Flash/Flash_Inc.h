
#pragma once
#include <stdint.h>

#ifndef FLASH_INC_H
#define FLASH_INC_H

#define FLASH_PAGE_SIZE                 256

#define FLASH_FACTORY_SETTINGS_Addr     (0xC000)
#define FLASH_FACTORY_SETTINGS_SIZE     (128 * 1024)

#define FLASH_USER_Addr                 (0x93000)
#define FALSH_USER_SIZE                 (512 * 1024)

#define FLASH_PIO_FLAGS_Addr            (0x94000)
#define FLASH_PIO_SIZE                  (256)    //

void flash_erase(uint32_t offset);
void flash_write_page(uint32_t offset,uint8_t *Write_buf);
void flash_read_page(uint64_t offset);
uint8_t flash_read_byte(uint64_t offset);
    
#endif