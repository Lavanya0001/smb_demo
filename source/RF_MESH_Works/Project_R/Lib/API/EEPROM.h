#pragma once

#include<stdio.h>
#include "hardware/i2c.h"
#include "hal/RPiPico/PicoHal.h"
#include "pico/stdlib.h"

#ifndef EEPROM_H
#define EEPROM_H

    #define I2C0_PORT_EEPROM    i2c0
    #define I2C0_SCL_EEPROM     17
    #define I2C0_SDA_EEPROM     16
    #define M24C32_Address      0x50 //(0b1010 0000) 1 - Read, 0 - Write

    #define EEPROM__WR_BUFFER_SIZE  30      //This size is equal to BUFFER_LENGTH - 2 bytes reserved for address.
    #define EEPROM__RD_BUFFER_SIZE  32
    #define EEPROM__Storgae_addr    0x00    //For storing the next adress in between booting's.
    #define EEPROM__Starting_addr   0x02    // We are taking 0x01 as starting addr of the mem,as 0x00 is reserved as storage

    void Init_EEPROM(void);
    void Write_EEPROM(uint16_t addr,uint16_t len,uint8_t *data);
    void Read_EEPROM(uint16_t address,uint16_t length,uint8_t *p_data);

#endif