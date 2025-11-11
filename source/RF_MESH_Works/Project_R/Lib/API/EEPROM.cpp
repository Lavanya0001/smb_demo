#include "EEPROM.h"
#include "PIO.h"

// EEPROM Initialization on PICO
static int pico_i2c_EEPROM_init(void){
    i2c_init(I2C0_PORT_EEPROM,300 * 1000);
    gpio_set_function(I2C0_SDA_EEPROM,GPIO_FUNC_I2C);   //SDA
    gpio_set_function(I2C0_SCL_EEPROM,GPIO_FUNC_I2C);   //SCL
    gpio_pull_up(I2C0_SCL_EEPROM);
    gpio_pull_up(I2C0_SDA_EEPROM);

    return PICO_OK;
}


//i2c_write() function for EEPROM
static int i2c_write_EEPROM(const uint8_t *Buf,size_t len,bool nonstop){
        int Res = i2c_write_blocking(I2C0_PORT_EEPROM, M24C32_Address, Buf, len, nonstop);
        return Res;// end with stop

}

//i2c_read() functoion for EEPROM
static int i2c_read_EEPROM( uint8_t *p_data,size_t length,bool nonstop){
      int res = i2c_read_blocking(I2C0_PORT_EEPROM, M24C32_Address, p_data, length, nonstop); 
      return res;

}

//Initialization of the EEPROM
void Init_EEPROM(){
 int Res =  pico_i2c_EEPROM_init();
 if(Res != PICO_OK){
    uart_printf("EEPROM Initialization Failed\n");
 }
}

//Write Buffer into the EEPROM
static void writeBuffer(uint16_t address, uint8_t length, uint8_t* p_data) {

    uint8_t buffer[2 + length];

    buffer[0] = (address >> 8) & 0xFF;  // MSB
    buffer[1] = address & 0xFF;         // LSB

    // Copy data into buffer after the address
    for (int i = 0; i < length; i++) {
        buffer[2 + i] = p_data[i];
    }

    // Now send over I2C
    int Res = i2c_write_EEPROM(buffer,2 + length,false);
        if(Res == PICO_ERROR_GENERIC){
            uart_printf("I2C0 Failed with %d\n",Res);
        }else{
            uart_printf("Written Done to the Buffer\n");
        }
    
    // === Store the "next write address" at EEPROM[0x0000] ===
    uint16_t next_address = address + length;
    uart_printf("Next adress this time is = %d\n",next_address);
    if (next_address >= 4096) { // wrap if end reached
        next_address = 0;
    }
    uint8_t addr_buf1[4] = {0};
    addr_buf1[0] = 0x00;                         // High byte of pointer address in EEPROM (MSB)
    addr_buf1[1] = 0x00;                         // Low byte of pointer address in EEPROM (LSB)
    addr_buf1[2] = (next_address >> 8) & 0xFF;   // High byte of value
    addr_buf1[3] = next_address & 0xFF;          // Low byte of value

    i2c_write_EEPROM(addr_buf1, 4, false); // Write pointer (2 bytes) to EEPROM[0x0000]    
}

//Read Buffer from the EEPROM
static void readBuffer(uint16_t address, uint8_t length, uint8_t* p_data) {
    if (length > 32) //EEPROM page size
    {
        uart_printf("ERROR: read too long\n");
        return;
    }

    // Step 1: Send the 2-byte memory address (write mode)
    uint8_t addr_buf[2];
    addr_buf[0] = (address >> 8) & 0xFF;  // MSB
    addr_buf[1] = address & 0xFF;         // LSB

    int res = i2c_write_EEPROM(addr_buf, 2, true);// keep bus active (repeated start)
    if (res == PICO_ERROR_GENERIC) {
        uart_printf("EEPROM Address Write Failed\n");
        return;
    }

    // Step 2: Read the data from EEPROM
    res = i2c_read_EEPROM(p_data,length,false);//end with stop
    if (res == PICO_ERROR_GENERIC) {
        uart_printf("EEPROM Read Failed\n");
        return;
    }

    uart_printf("Read Done\n");
}

//Write pages i.e., Partisioning into 32 Byrtes and writing into the EEPROM
static void write_page_EEPROM(uint16_t address,uint8_t length,uint8_t *data){
        // Write complete buffers.
    uint8_t bufferCount = length / EEPROM__WR_BUFFER_SIZE;
    for (uint8_t i = 0; i < bufferCount; i++)
    {
        uint8_t offset = i * EEPROM__WR_BUFFER_SIZE;
        writeBuffer(address + offset, EEPROM__WR_BUFFER_SIZE, data + offset);
    }

    // Write remaining uint8_ts.
    uint8_t remainingBytes = length % EEPROM__WR_BUFFER_SIZE;
    uint8_t offset = length - remainingBytes;
    writeBuffer(address + offset, remainingBytes, data + offset);

}

//Write into EEPROM with a proper dividance of the Buffer
void Write_EEPROM(uint16_t addr,uint16_t len,uint8_t *data){
    if(len == 1){
        writeBuffer(addr,len,data);
        return;
    }
 // Write first page if not aligned.
    uint8_t notAlignedLength = 0;
    uint8_t pageOffset = addr % 32;
    if (pageOffset > 0)
    {
        notAlignedLength = 32 - pageOffset;
        if (len < notAlignedLength)
        {
            notAlignedLength = len;
        }
        write_page_EEPROM(addr, notAlignedLength, data);
        len -= notAlignedLength;
    }

    if (len > 0)
    {
        addr += notAlignedLength;
        data += notAlignedLength;

        // Write complete and aligned pages.
        uint8_t pageCount = len / 32;
        for (uint8_t i = 0; i < pageCount; i++)
        {
            write_page_EEPROM(addr, 32, data);
            addr += 32;
            data += 32;
            len -= 32;
        }

        if (len > 0)
        {
            // Write remaining uncomplete page.
            write_page_EEPROM(addr, len, data);
        }
    }
}

//Reading and storing in the Buffer
void Read_EEPROM(uint16_t address,uint16_t length,uint8_t *p_data){
    if(length == 1){
        readBuffer(address,length,p_data);
    }

    uint8_t bufferCount = length / EEPROM__RD_BUFFER_SIZE;

    for (uint8_t i = 0; i < bufferCount; i++)
    {
        uint16_t offset = i * EEPROM__RD_BUFFER_SIZE;
        readBuffer(address + offset, EEPROM__RD_BUFFER_SIZE, p_data + offset);
    }

    uint8_t remainingBytes = length % EEPROM__RD_BUFFER_SIZE;
    uint16_t offset = length - remainingBytes;
    readBuffer(address + offset, remainingBytes, p_data + offset);
}