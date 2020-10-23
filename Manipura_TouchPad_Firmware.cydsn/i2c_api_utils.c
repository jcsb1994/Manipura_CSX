/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>
#if !i2c__DISABLED
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "i2c_api_utils.h"

extern int i2c_delay ;
#define TIMEOUT_MSEC    300u
uint8_t i2c_slave_address = 0x00 ;

uint32_t i2c_status(void) 
{ 
    return(I2C_I2CMasterStatus()); 
}

uint32_t i2c_ClearStatus(void) 
{ 
    return(I2C_I2CMasterClearStatus()); 
}

void   i2c_set_slave_address(uint8_t newAddress) 
{ 
    i2c_slave_address = newAddress ; 
}

uint8_t  i2c_get_slave_address(void) 
{
    return( i2c_slave_address );
}

uint32_t i2c_SendStart(uint8_t rw) 
{
    return(I2C_I2CMasterSendStart(i2c_slave_address, (uint32_t)rw, TIMEOUT_MSEC));
}

uint32_t i2c_SendRestart(uint32_t bitRnW) 
{
    return(I2C_I2CMasterSendRestart(i2c_slave_address, bitRnW, TIMEOUT_MSEC));
}

uint32_t i2c_SendStop(void) 
{ 
    return(I2C_I2CMasterSendStop(TIMEOUT_MSEC)); 
}

uint8_t  i2c_readByte(uint32_t acknak) /* 6-Mar-2018 acknak added */
{
    uint8_t data ;
    (void)I2C_I2CMasterReadByte(acknak, &data, TIMEOUT_MSEC) ;  /* ignore error */
    return( data ) ;
}

uint8_t  i2c_readReg(uint8_t addr)
{
    uint8_t data ;
    i2c_SendStart(0u) ; /* for write */
    i2c_writeByte(addr) ; 
    i2c_SendRestart(0x1u) ; /* for read */
    CyDelayUs(50) ;
    data = i2c_readByte(I2C_I2C_NAK_DATA) ;
    i2c_SendStop() ;
    return( data ) ;
}

void i2c_readRegs(uint8_t addr, uint8_t *data, int len) 
{
    int i ;
    
    i2c_SendStart(0u) ; /* for write */
    i2c_writeByte(addr) ;
    i2c_SendRestart(0x1u) ; /* for read */
    for (i = 0 ; i < len-1 ; i++ ) {
        CyDelayUs(i2c_delay) ;        
        data[i] = i2c_readByte(I2C_I2C_ACK_DATA) ;
    }
        CyDelayUs(100) ; 
    data[i] = i2c_readByte(I2C_I2C_NAK_DATA) ;
    i2c_SendStop() ;
}

uint32_t i2c_writeByte(uint8_t data) 
{
    uint32_t theByte, result ;
    
    theByte = data ;
    result = I2C_I2CMasterWriteByte(theByte, TIMEOUT_MSEC) ;

    return(result) ;
}

void i2c_writeReg(uint8_t addr, uint8_t data) 
{
    i2c_SendStart(0u) ; /* for write */
    i2c_writeByte(addr) ; /* write register address */
    CyDelayUs(1) ; 
    i2c_writeByte(data) ;
    i2c_SendStop() ;
}

void i2c_writeRegs(uint8_t addr, uint8_t *data, int len) 
{
    int i ;
    i2c_SendStart(0u) ; /* for write */
    i2c_writeByte(addr) ;
    for (i = 0 ; i < len ; i++ ) {
        CyDelayUs(1) ; 
        i2c_writeByte(data[i]) ;
    }
    CyDelayUs(1) ; 
    i2c_SendStop() ;
}

int i2c_test_address(uint8_t address) 
{
    uint32 Status ;
    uint8 data[2] ; /* dummy buffer to read */
    uint16_t timeout = 10 ; /* 10 ms */
    
    Status = I2C_I2CMasterSendStart(address, (uint32_t)0u, timeout) ; /* for write */
#if 0
    Status = I2C_I2CMasterWriteByte(address, timeout) ;
    Status = I2C_I2CMasterSendRestart(address, (uint32_t)1u, timeout) ; /* for read */
    CyDelay(1) ;   
    I2C_I2CMasterReadByte(I2C_I2C_ACK_DATA, &data[0], timeout) ;
    I2C_I2CMasterReadByte(I2C_I2C_NAK_DATA, &data[1], timeout) ;
#endif
    I2C_I2CMasterSendStop(timeout) ;
    
    if (Status == I2C_I2C_MSTR_NO_ERROR) {
        return 1 ;
    } else {
        return 0 ;
    }   
}
#endif /* i2c__DISABLED */
