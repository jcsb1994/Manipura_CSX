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
#include "project.h"
#include <stdio.h> 


/************************** Macro Definitions *********************************/
 
#define BytesToU16BE(B) (       \
      (uint16_t)((B)[0]) << 8   \
    | (uint16_t)((B)[1])        \
)
    
#define BytesToU32LE(B) (       \
      (uint32_t)((B)[3]) << 24  \
    | (uint32_t)((B)[2]) << 16  \
    | (uint32_t)((B)[1]) << 8   \
    | (uint32_t)((B)[0])        \
)
    
int main(void)
{
    __enable_irq(); /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    for(;;)
    {
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
