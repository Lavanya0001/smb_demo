
#include "PIO.h"
#include "EEPROM.h"
#include "Lib/Flash/Flash_Inc.h"

uint8_t time_regs[] = {REG_SECONDS};
uint8_t time_data[7];

char buf[64];
char adc_buf[70];
char spi_buf[30];
char Print_buf[100] = {0};   

extern uint8_t Flash_buf[FLASH_PAGE_SIZE];

// Peripheral - User LED's (GPIO's)
static int pico_led_init(uint gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_OUT);

    return PICO_OK;
}
void pico_led_app(uint gpio,bool state){
    pico_set_led(gpio, state);
    sleep_ms(LED_DELAY_MS);
    pico_set_led(gpio, state);
    sleep_ms(LED_DELAY_MS);
}


// Peripheral - General UART port (UART0)
int pico_uart0_init(void) {
    #ifdef PICO2
        gpio_set_function(UART0_RX_GN, GPIO_FUNC_UART_AUX);
        gpio_set_function(UART0_TX_GN, GPIO_FUNC_UART_AUX);
    #endif
    uart_init(uart0, 115200);
    return PICO_OK;
}

void uart_printf(const char *fmt, ...) {
    char buf[255];  
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    uart_puts(UART_GN, buf);
}



// Peripheral - RS485/Modbus (UART1)
int pico_RS485_init(void) {
    #ifdef PICO_2
    gpio_set_function(UART1_RX_485, GPIO_FUNC_UART_AUX);
    gpio_set_function(UART1_TX_485, GPIO_FUNC_UART_AUX);
    #endif
    gpio_init(RS484_CTRL);
    gpio_set_dir(RS484_CTRL, GPIO_OUT);
    gpio_put(RS484_CTRL, true);

    uart_init(UART_RS484, 115200);
    return PICO_OK;
}
void RS485_transmit(const char* s) {
    gpio_put(RS484_CTRL, true);
    uart_puts(UART_RS484, s);
    uart_tx_wait_blocking(UART_RS484);
    gpio_put(RS484_CTRL, false);
}
void RS485_receive() {
    char a[30];
    while (uart_is_readable(UART_RS484)) {
        uint8_t rx = uart_getc(UART_RS484);
        sprintf(a,"Data = %c\n",rx);
        uart_puts(UART_GN,a);
    }
    sleep_ms(1);
}
void pico_RS485_app(void){
    RS485_transmit("\nHello, uart interrupts\n");
}



// Peripheral - Real Time Clock (i2c1)
int pico_i2c_rtc_init(void) {
    gpio_set_function(I2C1_SDA_RTC, GPIO_FUNC_I2C);   // SDA
    gpio_set_function(I2C1_SCL_RTC, GPIO_FUNC_I2C);   // SCL
    gpio_pull_up(I2C1_SDA_RTC);
    gpio_pull_up(I2C1_SCL_RTC);

    return PICO_OK;
}
uint8_t bcd2dec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}
uint8_t bin2bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}
void set_pcf85263a_time(uint8_t time[8]) {
    uint8_t buf[8];
    buf[0] = 0x01;             // Start at seconds register
    buf[1] = bin2bcd(time[0]); // seconds#include "Flash_Inc.h"

    buf[2] = bin2bcd(time[1]); // minutes
    buf[3] = bin2bcd(time[2]); // hours
    buf[4] = bin2bcd(time[3]); // weekday
    buf[5] = bin2bcd(time[4]); // date (day of month)
    buf[6] = bin2bcd(time[5]); // month
    buf[7] = bin2bcd(time[6]); // year (two digits)

    i2c_write_blocking(I2C1_PORT_RTC, PCF85263A_ADDR, buf, 8, false);
}
void pico_i2c_rtc_app(void){

    static bool flag = true;
    if(flag){
        //Setting RTC Timer from Current Instance  in SS:MM:HH:DATE:DAYOFWEEK:MONTH:YEAR format          
        uint8_t current_time[8] = {36, 53, 11, 06, 4, 8,25};       
        set_pcf85263a_time(current_time);

        flag = false;
    }

    i2c_write_blocking(I2C1_PORT_RTC, PCF85263A_ADDR, time_regs, 1, true);
    i2c_read_blocking(I2C1_PORT_RTC, PCF85263A_ADDR, time_data, 7, false);

    uint8_t seconds = bcd2dec(time_data[0] & 0B01111111);
    uint8_t minutes = bcd2dec(time_data[1] & 0B01111111);
    uint8_t hours   = bcd2dec(time_data[2] & 0B00111111);
    uint8_t days    = bcd2dec(time_data[3] & 0B00111111);
    uint8_t dayofweek  = bcd2dec(time_data[4] & 0B00000111);
    uint8_t months   = bcd2dec(time_data[5] & 0B00011111);
    uint8_t years = bcd2dec(time_data[6]);

    sprintf(buf, "Time: %02d-%02d-20%02d %02d:%02d:%02d\n", days, months, years, hours, minutes, seconds);
    uart_puts(UART_GN, buf);
}



// Peripheral - EEPROM (i2c0)
void EEPROM_app(void){

    uint8_t Wdata[3] = {0,1,2};
        uint8_t Rdata[3]; 
        Write_EEPROM(EEPROM__Starting_addr,sizeof(Wdata),Wdata);
        sleep_ms(5);            // Mandatory delay after every write or Read from EEPROM
        Read_EEPROM(EEPROM__Starting_addr,18,Rdata);
        sleep_ms(5);
        uart_puts(UART_GN,"Data Read = ");
        for(uint8_t i = 0; i < 3;i++){
                sprintf(Print_buf,"%d,",Rdata[i]);
                uart_puts(UART_GN,Print_buf);

        }
        uint8_t Rx[2];
        Read_EEPROM(EEPROM__Storgae_addr,2,Rx);
        sleep_ms(5);
        uint16_t Next_addr = (Rx[0] << 8) | Rx[1];
        uart_printf("next adress = %d\n",Next_addr);

}



// Peripheral - ADC (GPIO's)
uint16_t adc2_val = 0;
uint16_t adc3_val = 0;
uint16_t adc4_val = 0;
uint16_t adc5_val = 0;

int adc_user_init(void) {
    adc_init();
    adc_gpio_init(ADC2_PIN);
    adc_gpio_init(ADC3_PIN);
    adc_gpio_init(ADC4_PIN);
    adc_gpio_init(ADC5_PIN);

    return PICO_OK;
}

void adc_app(void){
    adc_select_input(ADC2_CHANNEL);
    adc2_val = adc_read();
    uart_printf("ADC2 Value = %d\n",adc2_val);

    adc_select_input(ADC3_CHANNEL);
    adc3_val = adc_read();
    uart_printf("ADC3 Value = %d\n",adc3_val);
}



// Peripheral - Inbuilt RF (SPI0)
void spi_rf_init(void) {
    
    spi_init(RP_SPI_PORT, 1000*1000);
    gpio_set_function(RP_MISO, GPIO_FUNC_SPI);
    gpio_set_function(RP_CS,   GPIO_FUNC_SIO);
    gpio_set_function(RP_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(RP_MOSI, GPIO_FUNC_SPI);

    gpio_set_function(RP_BUSY, GPIO_FUNC_SIO);  
    gpio_set_function(RP_RST, GPIO_FUNC_SIO);
    gpio_set_function(RP_DIO1, GPIO_FUNC_SIO);

    
   
    gpio_set_dir(RP_CS, GPIO_OUT);
    gpio_put(RP_CS, 1);
    gpio_set_dir(RP_RST, GPIO_OUT);
    gpio_put(RP_RST,1);
    gpio_set_dir(RP_BUSY, GPIO_IN);
    gpio_set_dir(RP_DIO1,GPIO_IN);
}



// Peripheral - DAC (SPI1)
int spi_dac_init(void) {
    spi_init(SPI1_PORT_DAC, 1000 * 1000);
    spi_set_format(SPI1_PORT_DAC,16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(SPI1_RX_DAC, GPIO_FUNC_SPI);
    gpio_set_function(SPI1_SCK_DAC, GPIO_FUNC_SPI);
    gpio_set_function(SPI1_TX_DAC, GPIO_FUNC_SPI);
    gpio_set_function(SPI1_CS_DAC, GPIO_FUNC_SIO);

    gpio_set_dir(SPI1_CS_DAC, GPIO_OUT);
    gpio_put(SPI1_CS_DAC, true);

    return PICO_OK;
}

void write_dac(uint8_t channel, uint8_t value) {
    uint16_t DAC = 0;
    DAC = ((0b0000)|(value << 4)|(0b10 << 2) | (channel));
    gpio_put(SPI1_CS_DAC,0);
    spi_write16_blocking(SPI1_PORT_DAC, &DAC, 1);
    gpio_put(SPI1_CS_DAC,1);
    sleep_ms(100);
}



// Peripheral - Universal IO (GPIO's)
void uio_init(void) {
    gpio_init(S1A);
    gpio_set_dir(S1A, GPIO_OUT);
    gpio_init(S2A);
    gpio_set_dir(S2A, GPIO_OUT);
    gpio_init(S3A);
    gpio_set_dir(S3A, GPIO_OUT);
    gpio_init(S1B);
    gpio_set_dir(S1B, GPIO_OUT);
    gpio_init(S2B);
    gpio_set_dir(S2B, GPIO_OUT);
    gpio_init(S3B);
    gpio_set_dir(S3B, GPIO_OUT);
    gpio_init(E_A);
    gpio_set_dir(E_A, GPIO_OUT);
    gpio_init(E_B);
    gpio_set_dir(E_B, GPIO_OUT);
}


// PIO - Initilization related functions
void init_peripherals_from_flash(void) {
    uint8_t flags = flash_read_byte(FLASH_PIO_FLAGS_Addr);

    if (flags & FLAG_PIO_LED_ENABLED){
        int rc = pico_led_init(LED_PIN1);
        hard_assert(rc == PICO_OK);
        uart_printf("User LED's Enabled & Initilized\n");
    }
    if (flags & FLAG_PIO_EEPROM_ENABLED){
        Init_EEPROM();  
        uart_printf("EEPROM Enabled & Initilized\n");

    }
    if (flags & FLAG_PIO_MODBUS_ENABLED){
        int modbus = pico_RS485_init();
        hard_assert(modbus == PICO_OK);
        uart_printf("MODBUS Enabled & Initilized\n");

    }
    if (flags & FLAG_PIO_RTC_ENABLED){
        int rtc = pico_i2c_rtc_init();
        hard_assert(rtc == PICO_OK);
        uart_printf("RTC Enabled & Initilized\n");

    }
}