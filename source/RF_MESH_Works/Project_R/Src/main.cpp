// Memory partitioning + FreeRTOS + PIO + Multicore all are included in this project
// Project_R v 0.1
// Keys - 1. Memory partition(.json) bricking the board for PICO 2(RP2350),but tested for RP2354 sucessfully - false(Assumption)
// 2. RF mesh connection request +  is suceeded (Only Parallel processing, not freeRTOS).
// 3. Config comes into picture here.



// C & PICO Lib
#include <stdio.h>
#include "pico/stdlib.h"

// Multicore Lib
#include "pico/multicore.h"

// Mutual Exclusion Lib
#include "pico/mutex.h"

// FreeRTOS Lib's
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"  
#include "pico/async_context_freertos.h"
// RF Lib
#include <RadioLib.h>

// User Flash Lib
#include "Lib/Flash/Flash_Inc.h"

// User Peripheral I/O lib
#include "PIO.h"

// User API Lib
#include "Application_Layer.hpp"

// #define RX
#define TX

PicoHal hal(spi0, RF_MISO, RF_MOSI, RF_SCK);
Module mod(&hal, RF_CS, RF_DIO1, RF_RST, RF_BUSY);
LLCC68 radio(&mod);

Mesh_api Mesh;

static uint counter_msg_Ready = 1;
volatile uint8_t shared_msg[MAX_DATA_RECIEVED];
volatile bool msg_ready = false; // doorbell for notify
uint8_t globalParseBuf[MAX_DATA_RECIEVED] = {0};
uint8_t globalTxBuf[MAX_DATA_RECIEVED] = {0};
uint8_t globaltempBuf[MAX_DATA_RECIEVED] = {0};
size_t actualLength = {0};
size_t globaltempBuf_len = {0};
//Initiating mutex from the pico/mutex
mutex_t spi_mutex;

struct repeating_timer timer;
bool repeating_timer_callback(struct repeating_timer *t);
volatile bool ping_triggered = false;
// flag to indicate that a packet was received
volatile bool receivedFlag = false;
volatile bool msg_readyFlag = false;

void static setFlagRx(void) {
  // we got a packet, set the flag
    receivedFlag = true;

}

void static core0_doorbell_irq(){

        // Clear the doorbell first
        msg_readyFlag = true;
        multicore_doorbell_clear_current_core(0);

}

void  core1_entry(){

        #ifdef TX
            // Enter Rx mode only after the initial Connection req sent to Node
            while(!Mesh.Tx_done_flag);
            printf("Reception started...\n");
        #endif

        // set the function that will be called
        // when new packet is received

        // Mutex - SPI Blocking
        mutex_enter_blocking(&spi_mutex);
        radio.setPacketReceivedAction(setFlagRx);
        int state = radio.startReceive();
        mutex_exit(&spi_mutex);
        // Mutex - SPI Unblocking

        if(state == RADIOLIB_ERR_NONE){
            printf("success!\n");
        }else{
            printf("Failed to start,Code %d\n",state);
            while(1){sleep_ms(100);}
        }

        if(state == RADIOLIB_ERR_NONE){
        }else{
            while(1){sleep_ms(100);}
        }
        uint static temp_c = 1;
        while(1){
            if(receivedFlag){
                // Clearing received flag from RF Interrupt
                receivedFlag = false;

                // Mutex - SPI Blocking
                mutex_enter_blocking(&spi_mutex);
                bool Isvalid = Mesh.Receive_data_bytes(radio);// process the packet
                mutex_exit(&spi_mutex);
                // Mutex - SPI Unblocking

                if(Isvalid){
                    // doorbell ringing for core0
                    multicore_doorbell_set_other_core(0);
                }else{

                    // Mutex - SPI Blocking
                    mutex_enter_blocking(&spi_mutex);
                    radio.startReceive();// re-enable receive
                    mutex_exit(&spi_mutex);
                    // Mutex - SPI Unblocking
    
                }


            }
        }
}

int main()
{   
    stdio_init_all();
    sleep_ms(3000);
    Mesh.RF_init();// RF pin Init's
    int state = radio.begin(918.0,500.0,5,5,0x12,22,8,0.0,false); // 918 MHz Fx
    if(state == RADIOLIB_ERR_NONE) {
            printf("LoRa init SUCCESS!\n");
    } else {
            printf("LoRa init failed: %d\n", state);
    }

    mutex_init(&spi_mutex);// Global SPI mutex Init's

    // Creating RTOS Tasks
    // BaseType_t task1 = xTaskCreate();

    #ifdef RX
        // Node side application
        printf("Node Application start's :\n");

        // Setup doorbell interrupt for Core 0, irq num = 0!
        uint32_t irq_num = multicore_doorbell_irq_num(0);// doorbell for core0
        irq_set_exclusive_handler(irq_num, core0_doorbell_irq);// doorbell Handler function call
        irq_set_enabled(irq_num, true);// Enabling IRQ

        multicore_launch_core1(core1_entry);// Launching core1
        uint static temp_counter = 1;
        while(1){
            if(msg_readyFlag){

                printf("\nCore0 calling =  %d\n",temp_counter++);
                msg_readyFlag = false;// Clearing flag
                printf("Message parsing started\n");

                memcpy(globaltempBuf,globalParseBuf,actualLength);// Copying to mid-temp buffer's
                globaltempBuf_len = actualLength;
                

                // Handling Received Messages
                uint8_t opcode = Mesh.Get_opcode(globaltempBuf,globaltempBuf_len);
                switch(opcode){
                    case CONNECT_REQUEST:{
                        printf("Opcode is = 0x%02x (Connection Request)\n",opcode);

                        // Mutex - SPI Blocking
                        mutex_enter_blocking(&spi_mutex);
                        Mesh.Handle_connect_req(radio,globaltempBuf,globaltempBuf_len);
                        mutex_exit(&spi_mutex);
                        // Mutex - SPI Unblocking

                        bool ok = add_repeating_timer_ms(10000, repeating_timer_callback, NULL, &timer);
                        printf("Timer added: %d\n", ok);

                        // Mutex - SPI Blocking
                        mutex_enter_blocking(&spi_mutex);
                        radio.startReceive();// re-enable receive
                        mutex_exit(&spi_mutex);
                        // Mutex - SPI Unblocking


                        break;
                    }
                    default:
                        printf("Opcode is = 0x%02x (Undefined)\n",opcode);
                        // Unknow case, leaving for now
                        
                        // Mutex - SPI Blocking
                        mutex_enter_blocking(&spi_mutex);
                        radio.startReceive();// re-enable receive
                        mutex_exit(&spi_mutex);
                        // Mutex - SPI Unblocking
 
                }
                    
            }
            if(ping_triggered){
                ping_triggered = false;

                uint8_t static ping_count = 1;
                uint8_t PingBuf[MAX_DATA_RECIEVED] = {0};
                uint8_t ping_Data[] = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF};
                // Mutex - SPI Blocking
                mutex_enter_blocking(&spi_mutex);
                printf("Ping - %d, Transmission is completed\n",ping_count++);
                Mesh.Transmit_data_bytes(radio,PING,ping_Data,sizeof(ping_Data),PingBuf);
                mutex_exit(&spi_mutex);
                // Mutex - SPI Unblocking
            }
        }
    #endif

    #ifdef TX
        printf("Gateway Application start's :\n");

        // Setup doorbell interrupt for Core 0, irq num = 0!
        uint32_t irq_num = multicore_doorbell_irq_num(0);// doorbell for core0
        irq_set_exclusive_handler(irq_num, core0_doorbell_irq);// doorbell Handler function call
        irq_set_enabled(irq_num, true);// Enabling IRQ

        multicore_launch_core1(core1_entry);// Launch core1

        uint8_t DataBuf[] = {0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
        sleep_ms(1000);
        Mesh.Tx_done_flag = false;

        while(!Mesh.Tx_done_flag){

            // Mutex - SPI Blocking
            mutex_enter_blocking(&spi_mutex);
            uint16_t stat = Mesh.Transmit_data_bytes(radio,CONNECT_REQUEST,DataBuf,sizeof(DataBuf),globalTxBuf);
            mutex_exit(&spi_mutex);
            // Mutex - SPI Unblocking

            if(!stat){
                Mesh.Tx_done_flag = true;
                printf("Request sent!!!\nStarted Listening\n");
                break;
            }

        }

        uint temp_counter = 1;
        uint ping_counter = 1;
        while(1){
            if(msg_readyFlag){
                msg_readyFlag = false;// Clearing flag

                memcpy(globaltempBuf,globalParseBuf,actualLength);// Copying to mid-temp buffer's
                globaltempBuf_len = actualLength;

                uint8_t op = Mesh.Get_opcode(globaltempBuf,globaltempBuf_len);
                switch (op){
                    case CONNECT_ACK:
                        printf("Node joining Sucessful!\n");
                        Mesh.Connect_done_flag = true;

                        // Mutex - SPI Blocking
                        mutex_enter_blocking(&spi_mutex);
                        printf("Everything will stops here\nBefore delay\n");
                        sleep_ms(500);
                        printf("After the Delay\n");
                        radio.startReceive();// re-enable receive
                        mutex_exit(&spi_mutex);
                        // Mutex - SPI Unblocking

                        break;
                    case PING:
                            printf("PING received,no - %d\n",ping_counter++);

                            // Mutex - SPI Blocking
                            mutex_enter_blocking(&spi_mutex);
                            printf("Everything will stops here\nBefore delay\n");
                            sleep_ms(1000);// Wait for 50ms
                            printf("After the Delay\n");
                            radio.startReceive();// re-enable receive
                            mutex_exit(&spi_mutex);
                            // Mutex - SPI Unblocking

                        break;
                    default:
                        printf("ERROR: Undefined opcode\n");

                        // Mutex - SPI Blocking
                        mutex_enter_blocking(&spi_mutex);
                        radio.startReceive();// re-enable receive
                        mutex_exit(&spi_mutex);
                        // Mutex - SPI Unblocking

                }

            }
        }

        
    #endif

}

bool repeating_timer_callback(struct repeating_timer *t) {
    // Setting ping_enable flag
    ping_triggered = true;
    return true;
}
