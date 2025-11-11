#ifndef PIO_H
#define PIO_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include <string.h>
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hal/RPiPico/PicoHal.h"

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 500
#endif

// PWM pin
#define PWM_PIN     46

// GPIO Interrupt
#define GPIO_WATCH_PIN  21
#define GPIO_OUTPUT_PIN 23

// LED
#define LED_PIN1 1
#define LED_PIN2 2
#define LED_PIN3 3

// UART0
#define UART_GN     uart0
#define UART0_RX_GN 35
#define UART0_TX_GN 34

// UART1 - RS485
#define UART_RS484   uart1
#define UART1_RX_485 11
#define UART1_TX_485 10
#define RS484_CTRL   22

/* RTC IC PCF85263A defnitions begin */
// I2C1 - RTC
#define I2C1_PORT_RTC   i2c1
#define I2C1_SCL_RTC    27
#define I2C1_SDA_RTC    26
#define PCF85263A_ADDR  0x51

// PCF85263A Register addresses (see datasheet for details)
#define REG_SECONDS 0x01
#define REG_MINUTES 0x02
#define REG_HOURS   0x03
#define REG_DAYS    0x04
#define REG_MONTHS  0x06
#define REG_YEARS   0x07

extern uint8_t time_regs[];
extern uint8_t time_data[7];
extern char buf[64];
/* RTC defnitions end*/

// ADC
#define ADC2_PIN    42
#define ADC3_PIN    43
#define ADC4_PIN    44
#define ADC5_PIN    45
#define ADC2_CHANNEL  2
#define ADC3_CHANNEL  3
#define ADC4_CHANNEL  4
#define ADC5_CHANNEL  5

extern char adc_buf[70];
extern uint16_t adc2_val;
extern uint16_t adc3_val;
extern uint16_t adc4_val;
extern uint16_t adc5_val;


#define RP_SPI_PORT spi0
#define RP_SCK   38
#define RP_MOSI  39
#define RP_MISO  36

#define RP_CS    37 // Chip select
#define RP_BUSY  32 // Interrupt pin
#define RP_RST   33 // Reset pin
#define RP_DIO1  40

extern char spi_buf[30];

#define SPI1_PORT_DAC   spi1
#define SPI1_RX_DAC     28
#define SPI1_CS_DAC     29
#define SPI1_SCK_DAC    30
#define SPI1_TX_DAC     31

#define DAC_CHANNEL_A   0x00
#define DAC_CHANNEL_B   0x01


// Universal IO Selection PIO
#define S1A     4
#define S2A     5
#define S3A     6
#define S1B     7
#define S2B     8
#define S3B     9
#define E_A     12
#define E_B     13

#define FLAG_PIO_LED_ENABLED        (1 << 0)
#define FLAG_PIO_EEPROM_ENABLED     (1 << 1)
#define FLAG_PIO_MODBUS_ENABLED     (1 << 2)
#define FLAG_PIO_RTC_ENABLED        (1 << 3)

// Peripheral
int pico_uart0_init(void);
int pico_uart1_init(void);
void RS485_transmit(const char*);
void RS485_receive();
int pico_i2c_rtc_init(void);
uint8_t bcd2dec(uint8_t);
uint8_t bin2bcd(uint8_t);
void set_pcf85263a_time(uint8_t time[8]);
void pico_set_led(int, bool);
int adc_user_init(void);
void spi_rf_init(void);
int spi_dac_init(void);
void write_dac(uint8_t channel, uint8_t value);
void dac_send(uint8_t);
void uio_init(void);
void uart_printf(const char *fmt, ...) ;
void Write_EEPROM(uint16_t addr,uint16_t len,uint8_t *data);
void Read_EEPROM(uint16_t address,uint16_t length,uint8_t *p_data);

void init_peripherals_from_flash(void);

#endif