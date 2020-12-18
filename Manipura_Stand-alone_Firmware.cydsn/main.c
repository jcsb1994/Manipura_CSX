/* ========================================
 *
 * Copyright Coro, ETS, Montreal, 2020
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * ========================================
*/
#include "project.h"

int main(void)
{
    CyGlobalIntEnable; // Enable global interrupts
    
    EZI2C_Start();  // Tune capsense sensors with I2C

    EZI2C_EzI2CSetBuffer1(
                      sizeof(CapSense_dsRam),
                      sizeof(CapSense_dsRam),
                      (uint8 *) &CapSense_dsRam 
                     );
    
    CapSense_Start();
    CapSense_ScanAllWidgets();

    for(;;)
    {
        if(CapSense_IsBusy() == CapSense_NOT_BUSY)
        {
            CapSense_RunTuner();
            CapSense_ScanAllWidgets();
        }
    }
}

/* [] END OF FILE */
