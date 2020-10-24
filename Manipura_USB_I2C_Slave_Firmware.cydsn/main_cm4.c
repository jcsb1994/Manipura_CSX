
#include "project.h"
#include <stdio.h> 
#include "message.h"


/************************** Macro Definitions *********************************/
    
#define BytesToU16LE(B) (       \
      (uint16_t)((B)[1]) << 8   \
    | (uint16_t)((B)[0])        \
)
    
#define BytesToU32LE(B) (       \
      (uint32_t)((B)[3]) << 24  \
    | (uint32_t)((B)[2]) << 16  \
    | (uint32_t)((B)[1]) << 8   \
    | (uint32_t)((B)[0])        \
)


/******************************************************************************/

uint8_t ezi2cBuffer[(1 + (2*49))]; 

static uint32_t print_counter_flag_bitmask;

enum bitmask_pos
{
    RAW_PRINT_BITMASK_POS = 0,
    PARASITIC_BITMASK_POS = 1,
};

void EZI2C_InterruptHandler(void)
{
    Cy_SCB_EZI2C_Interrupt(EZI2C_HW, &EZI2C_context);
}

void print_counter_int_handler()  // Custom function for handling interrupts from print_counter
{
    Cy_TCPWM_ClearInterrupt(print_counter_HW, print_counter_CNT_NUM, CY_TCPWM_INT_ON_TC);
    print_counter_flag_bitmask |= 0xFF; // all flags turned on
}

void printf_counter_setup() 
{
    Cy_SysInt_Init(&counterInt_cfg, print_counter_int_handler);
    NVIC_EnableIRQ(counterInt_cfg.intrSrc); /* Enable the core interrupt */

    (void)Cy_TCPWM_Counter_Init(print_counter_HW, print_counter_CNT_NUM, &print_counter_config); 
    Cy_TCPWM_Enable_Multiple(print_counter_HW, print_counter_CNT_MASK); /* Enable the counter instance */

    Cy_TCPWM_TriggerStart(print_counter_HW, print_counter_CNT_MASK);
}


void i2c_init()
{
        /* Hook interrupt service routine and enable interrupt */
    Cy_SysInt_Init(&EZI2C_SCB_IRQ_cfg, &EZI2C_InterruptHandler);
    NVIC_EnableIRQ(EZI2C_SCB_IRQ_cfg.intrSrc);

    /* Initialize SCB for EZI2C operation */
    (void) Cy_SCB_EZI2C_Init(EZI2C_HW, &EZI2C_config, &EZI2C_context);
    
    /* Configure buffer for communication with master */
    Cy_SCB_EZI2C_SetBuffer1(EZI2C_HW, ezi2cBuffer, BUFFER_SIZE, BUFFER_SIZE, &EZI2C_context);
    
    /* Enable SCB for the EZI2C operation */
    Cy_SCB_EZI2C_Enable(EZI2C_HW);
}


int main(void)
{
    uint32 ezi2cState;  // stores ezi2c status
    
    __enable_irq(); /* Enable global interrupts. */

    i2c_init();

    UART_Start();
    
    printf_counter_setup();
    
    uint32_t taxel_raw_values[TAXELS_NB];
    uint32_t cp_values[ROW_TAXELS_NB + COL_TAXELS_NB];
    
    for(;;)
    {     
        NVIC_DisableIRQ(EZI2C_SCB_IRQ_cfg.intrSrc);     // Disable the EZI2C interrupts so that ISR is not serviced while checking for EZI2C status 
        ezi2cState = Cy_SCB_EZI2C_GetActivity(EZI2C_HW, &EZI2C_context);    // check status
        
        /* Write complete without errors: parse packets, otherwise ignore */
        if((0u != (ezi2cState & CY_SCB_EZI2C_STATUS_WRITE1)) && (0u == (ezi2cState & CY_SCB_EZI2C_STATUS_ERR)))
        {
            
            if (ezi2cBuffer[0] == raw_count_id)
            {
                const uint8_t bytes_per_taxel = 2;
                uint8_t current_taxel_bytes[bytes_per_taxel];   
                uint8_t payload_size = bytes_per_taxel * TAXELS_NB;
                uint8_t current_taxel_index = 0;
                
                for(int i = 1; i <= payload_size; i++)
                {        
                    current_taxel_bytes[i % bytes_per_taxel] = ezi2cBuffer[i];
                    
                    if(!(i % bytes_per_taxel))
                    {
                        taxel_raw_values[current_taxel_index] = BytesToU16BE(current_taxel_bytes);
                        current_taxel_index++;
                     //   if(current_taxel_index >= TAXELS_NB)
                     //       current_taxel_index = 0;
                    }
                }   
                
                if(print_counter_flag_bitmask & (1 << RAW_PRINT_BITMASK_POS))
                {
                    printf("bytes %d %d %d %d %d\r\n", ezi2cBuffer[0], ezi2cBuffer[1], 
                    ezi2cBuffer[2], ezi2cBuffer[3], ezi2cBuffer[4]);
                    
                    for(int i = 1; i <= TAXELS_NB; i++)
                    {
                        printf("%d: %lu  ", i, (unsigned long)taxel_raw_values[i-1]);
                        if (!(i%7))
                            printf("\r\n");
                    }
                    printf("\r\n");
                    print_counter_flag_bitmask &= ~(1 << RAW_PRINT_BITMASK_POS); 
                }
            }
            
            else if (ezi2cBuffer[0] == parasitic_id)
            {
                
                const uint8_t bytes_per_cp_val = 4;
                uint8_t current_value_bytes[bytes_per_cp_val];   
                uint8_t payload_size = bytes_per_cp_val * (COL_TAXELS_NB * ROW_TAXELS_NB);
                uint8_t current_value_index = 0;
                
                for(int i = 1; i <= payload_size; i++)
                {        
                    current_value_bytes[i % bytes_per_cp_val] = ezi2cBuffer[i];
                    
                    if(!(i % bytes_per_cp_val))
                    {
                        cp_values[current_value_index] = BytesToU32LE(current_value_bytes);
                        current_value_index++;
                   //     if(current_value_index >= TAXELS_NB)
                   //         current_value_index = 0;
                    }
                }
                    
                if (print_counter_flag_bitmask & (1 << PARASITIC_BITMASK_POS))
                {
                    for(int i = 0; i < (ROW_TAXELS_NB + COL_TAXELS_NB); i++)
                    {
                        printf("Cp %d: %lu \r\n", i, (unsigned long)cp_values[i]); 
                    }
                    printf("\r\n");
                    print_counter_flag_bitmask &= ~(1 << PARASITIC_BITMASK_POS);
                }
            }
            //printf("Msg ID: %d", ezi2cBuffer[0]);
        }
    
    NVIC_EnableIRQ(EZI2C_SCB_IRQ_cfg.intrSrc); // enable interrupts to call the i2c isr
    }
}                    


/* [] END OF FILE */
