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
#ifndef _I2C_API_UTILS_H_
#define _I2C_API_UTILS_H_ defined
#include "project.h"
#if !i2c__DISABLED
#include <stdio.h>
#include <ctype.h>
#include <string.h>
    
/* Master status */
#define I2C_STATUS_CLEAR            ((uint16_t) 0x00u)   /* Clear (init) status value */

#define I2C_STATUS_RD_CMPLT         ((uint16_t) 0x01u)   /* Read complete               */
#define I2C_STATUS_WR_CMPLT         ((uint16_t) 0x02u)   /* Write complete              */
#define I2C_STATUS_XFER_INP         ((uint16_t) 0x04u)   /* Master transfer in progress */
#define I2C_STATUS_XFER_HALT        ((uint16_t) 0x08u)   /* Transfer is halted          */

#define I2C_STATUS_ERR_MASK         ((uint16_t) 0x3F0u) /* Mask for all errors                          */
#define I2C_STATUS_ERR_SHORT_XFER   ((uint16_t) 0x10u)  /* Master NAKed before end of packet            */
#define I2C_STATUS_ERR_ADDR_NAK     ((uint16_t) 0x20u)  /* Slave did not ACK                            */
#define I2C_STATUS_ERR_ARB_LOST     ((uint16_t) 0x40u)  /* Master lost arbitration during communication */
#define I2C_STATUS_ERR_ABORT_XFER   ((uint16_t) 0x80u)  /* The Slave was addressed before the Start gen */
#define I2C_STATUS_ERR_BUS_ERROR    ((uint16_t) 0x100u) /* The misplaced Start or Stop was occurred     */
#define I2C_STATUS_ERR_XFER         ((uint16_t) 0x200u) /* Error during transfer                        */

/* Master API returns */
#define I2C_RESULT_NO_ERROR          (0x00u)  /* Function complete without error                       */
#define I2C_RESULT_ERR_ARB_LOST      (0x01u)  /* Master lost arbitration: INTR_MASTER_I2C_ARB_LOST     */
#define I2C_RESULT_ERR_LB_NAK        (0x02u)  /* Last Byte Naked: INTR_MASTER_I2C_NACK                 */
#define I2C_RESULT_NOT_READY         (0x04u)  /* Master on the bus or Slave operation is in progress   */
#define I2C_RESULT_BUS_BUSY          (0x08u)  /* Bus is busy, process not started                      */
#define I2C_RESULT_ERR_ABORT_START   (0x10u)  /* Slave was addressed before master begin Start gen     */
#define I2C_RESULT_ERR_BUS_ERR       (0x100u) /* Bus error has: INTR_MASTER_I2C_BUS_ERROR              */
    
extern uint8_t  i2c_slave_address ;
extern int      i2c_delay ;
    
uint32_t i2c_status(void);
uint32_t i2c_ClearStatus(void) ;
void     i2c_set_slave_address(uint8_t newAddress) ;
uint8_t  i2c_get_slave_address(void) ;
uint32_t i2c_SendStart(uint8_t readwrite) ;
uint32_t i2c_SendRestart(uint32_t bitRnW) ;
uint32_t i2c_SendStop(void) ;
uint8_t  i2c_readByte(uint32_t acknak) ;
uint32_t i2c_writeByte(uint8_t data) ;
uint8_t  i2c_readReg(uint8_t addr) ;
void     i2c_writeReg(uint8_t addr, uint8_t data) ;
void     i2c_readRegs(uint8_t addr, uint8_t *data, int len) ;
void     i2c_writeRegs(uint8_t addr, uint8_t *data, int len) ;
int      i2c_test_address(uint8_t address) ;

#endif /* !i2c__DISABLED */
#endif /* _I2C_API_UTILS_H_ */
