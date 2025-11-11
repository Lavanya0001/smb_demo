#pragma once

#include <stdio.h>
#include <pico/stdlib.h>
#include <RadioLib.h>
#include <iostream>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/timer.h"
#include "hal/RPiPico/PicoHal.h"
#include "hardware/flash.h"
#include "pico/multicore.h"

#ifndef APPLICATION_LAYER_HPP
#define APPLICATION_LAYER_HPP

#define GATEWAY
// #define NODE
// Defining SPI pins
#ifdef GATEWAY
    #define RF_SCK      2    // SPI SCK
    #define RF_MISO     4    // SPI MISO
    #define RF_MOSI     3    // SPI MOSI
    #define RF_CS       5    // Chip select (NSS)
    #define RF_RST      6    // Reset pin
    #define RF_DIO1     7    // DIO1 IRQ
    #define RF_BUSY     8    // BUSY pin
    #define LORA_ANTSW  17    // Antenna switch (optional)

    #define USER_LED_ON_PICO2 25
#endif

#ifdef NODE
    #define RF_SCK      14    // SPI SCK
    #define RF_MISO     24    // SPI MISO
    #define RF_MOSI     15    // SPI MOSI
    #define RF_CS       13    // Chip select (NSS)
    #define RF_RST      23    // Reset pin
    #define RF_DIO1     16    // DIO1 IRQ
    #define RF_BUSY     18    // BUSY pin
    #define LORA_ANTSW  17    // Antenna switch (optional)

    #define USER_LED_ON_PICO2 25
#endif

//Related to Flash Write memory
#define PAGE_SIZE 256
#define NVS_SIZE  4096
#define FLASH_WRITE_START  (PICO_FLASH_SIZE_BYTES - NVS_SIZE)
#define FLASH_READ_START   (FLASH_WRITE_START + XIP_BASE)

/* Constant Numericals */
#define MAX_DATA_RECIEVED       256

/* Opcodes related to RF communication */
#define HEADER_BYTE_1           0x55
#define HEADER_BYTE_2           0xAA

#define CONNECT_REQUEST         0x01
#define CONNECT_ACK             0x81
#define PING                    0x02
#define SET_CONFIG              0x03

class Mesh_api {
private:

public:
    bool volatile Tx_done_flag;
    bool volatile Connect_done_flag = false;
    Mesh_api();

    void RF_init();
    uint8_t Get_opcode(const uint8_t* msg, size_t len);
    int16_t Transmit_data_bytes(LLCC68 &radio,uint8_t opcode ,uint8_t* payload ,size_t payload_len, uint8_t* txBuf);
    bool Receive_data_bytes(LLCC68 &radio);
    void Handle_connect_req(LLCC68 &radio,uint8_t* OutBuffer,size_t len);
    const uint8_t* Get_payload(const uint8_t* msg, size_t len, size_t& outPayloadLen);

};

#endif
