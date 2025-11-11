#include "Application_Layer.hpp"
#include <stdio.h>

//External Variables
extern uint8_t globalParseBuf[MAX_DATA_RECIEVED];
extern size_t actualLength;
//API Functions

static uint16_t CRC_calaculation(const uint8_t* data, size_t length) {
    uint16_t sum = 0;
    for (size_t i = 0; i < length; ++i) {
        sum += data[i];
    }
    return sum;
}

static bool Rx_Message_validation(uint8_t *Msg,size_t Len){
        uint16_t Rx_CRC = ((uint16_t)(Msg[Len - 1]) << 8) | Msg[Len - 2];
        // printf("Rx CRC = 0x%x,",Rx_CRC);
        uint16_t Sum_CRC = CRC_calaculation(Msg,Len - 2);
        // printf("Sum CRC = 0x%x,",Sum_CRC);

        if((Len > 0) && (Msg[0] == 0x55 && Msg[1] == 0xAA) && (Rx_CRC == Sum_CRC)){

            return true;
        }
        return false;
}

static void Write_to_flash(const uint8_t *data, size_t size) {
    if (size > 256) {
        printf("Data too large. Max 256 bytes allowed per page.\n");
        return;
    }

    // Create a 256-byte aligned buffer and clear it
    uint8_t page_buf[256] = {0};
    memcpy(page_buf, data, size);

    uint32_t ints = save_and_disable_interrupts();

    // Erase minimum sector (4KB)
    flash_range_erase(FLASH_WRITE_START, NVS_SIZE);

    // Program only one 256-byte page
    flash_range_program(FLASH_WRITE_START, page_buf, 256);
    restore_interrupts(ints);
}

static void read_from_flash(uint32_t flash_offset, uint8_t *dest_buf, size_t size) {
    const uint8_t *flash_ptr = (const uint8_t *)(XIP_BASE + flash_offset);
    memcpy(dest_buf, flash_ptr, size);
}

Mesh_api::Mesh_api() {
    printf("Mesh application is started\n");
}

void Mesh_api::RF_init(){
    gpio_init(USER_LED_ON_PICO2);         // put back into RX for next packet

    gpio_set_dir(USER_LED_ON_PICO2,GPIO_OUT);
    gpio_put(USER_LED_ON_PICO2,1 );

    spi_init(spi1, 1000 * 1000);
    gpio_set_function(RF_MISO, GPIO_FUNC_SPI);
    gpio_set_function(RF_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(RF_SCK, GPIO_FUNC_SPI);

    gpio_init(RF_CS); gpio_set_dir(RF_CS, GPIO_OUT); gpio_put(RF_CS, 1);
    gpio_init(RF_RST); gpio_set_dir(RF_RST, GPIO_OUT);

    gpio_init(RF_DIO1); gpio_set_dir(RF_DIO1, GPIO_IN);
    gpio_init(RF_BUSY); gpio_set_dir(RF_BUSY, GPIO_IN);

    // Do a manual reset
    gpio_put(RF_RST, 0);
    sleep_ms(100);
    gpio_put(RF_RST, 1);
    sleep_ms(100);
}


uint8_t Mesh_api::Get_opcode(const uint8_t* msg, size_t len) {
    if (len < 4) return 0xFF;  
    return msg[3];  
}


const uint8_t* Mesh_api::Get_payload(const uint8_t* msg, size_t len, size_t& outPayloadLen) {
    if (len < 4) return nullptr;

    outPayloadLen = msg[2]; // Byte 1 has the payload length
    return &msg[4];         // Payload starts at byte 4
}

void Mesh_api::Handle_connect_req(LLCC68 &radio,uint8_t* OutBuffer,size_t len){
        size_t payload_len;
        const uint8_t* payload = Get_payload(OutBuffer,len,payload_len);
        printf("Rx Payload len = %d\n",payload_len);
        printf("Rx Payload = ");
        for (size_t i = 0; i < payload_len; ++i) {
        printf("0x%02X ", payload[i]);
        }
        printf("\n");
        printf("Sending Ack to the Gateway\n");
        uint8_t Ack_Data[] = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
        Transmit_data_bytes(radio,CONNECT_ACK,Ack_Data,sizeof(Ack_Data),OutBuffer);
}
int16_t Mesh_api::Transmit_data_bytes(LLCC68 &radio,uint8_t opcode ,uint8_t* payload ,size_t payload_len, uint8_t* txBuf)
{
    if(!payload || (payload_len + 4) > MAX_DATA_RECIEVED){
        return 0;
    }

    size_t i = 0;
    txBuf[i++] = HEADER_BYTE_1;
    txBuf[i++] = HEADER_BYTE_2;
    txBuf[i++] = static_cast<uint8_t>(payload_len);
    txBuf[i++] = opcode;

    for (size_t j = 0; j < payload_len; ++j) {
        txBuf[i++] = payload[j];
    }
    size_t txbuf_Len = i;
    // Calculate checksum over original data
    uint16_t CRC = CRC_calaculation(txBuf,txbuf_Len);
    // printf("Tx CRC = 0x%x,",CRC);

    // Append checksum (LSB first)
    txBuf[txbuf_Len++] = CRC & 0xFF;
    txBuf[txbuf_Len++] = (CRC >> 8) & 0xFF;
    // printf("Tx CRC Appended in LSB First manner.");
    int16_t state = radio.transmit(txBuf, txbuf_Len);
    if (state == RADIOLIB_ERR_NONE) {
        //     printf("TxBuf Length = %zu\n",txbuf_Len);
        //     printf("Tx Data = ");
        // for(int i = 0; i < txbuf_Len; i ++){
        //     printf("0x%x,",txBuf[i]);
        // }
        // printf("\n");
        printf("Transmission DONE!\n");
        return state;
    } else {
        // printf("\nTx Data = ");
        // for(int i = 0; i < txbuf_Len; i ++){
        //     printf("0x%x,",txBuf[i]);
        // }
        printf("Transmission failed: %d\n", state);
        return state;
    }
}
bool Mesh_api::Receive_data_bytes(LLCC68 &radio) {
    uint8_t localBuf[MAX_DATA_RECIEVED];  
    int16_t state = radio.readData(localBuf, sizeof(localBuf));

    if (state == RADIOLIB_ERR_NONE) {
        actualLength = radio.getPacketLength(); 
        // printf("Length of the Packet = %zu\n", actualLength);
        // printf("Rx Data = ");
        // for (size_t i = 0; i < actualLength; i++) {
        //     printf("0x%x,", localBuf[i]);
        // }
        // printf("\n");
        // printf("RSSI = %.2f dBm\n", radio.getRSSI());
        // printf("SNR = %.2f dB\n", radio.getSNR());

        if (Rx_Message_validation(localBuf, actualLength)) {
            // printf("Rx Message Validated\n");
            memset(globalParseBuf, 0, 256);
            memcpy(globalParseBuf, localBuf, actualLength);
        } else {
            printf("Message validation failed\n");
            return false;
        }
    } else {
        printf("Read failed: %d\n", state);
        return false;
    }

    return true;
}
