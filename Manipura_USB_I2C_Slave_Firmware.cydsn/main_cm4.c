
#include "project.h"
#include <stdio.h> 
#include "message.h"

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

/*
// BELOW MACROS NOT NEEDED ANYMORE

#define BYTES_PER_TAXEL_RAW_COUNT (2)
#define RAW_PACKET_SIZE (uint8_t) (TAXELS_NB*BYTES_PER_TAXEL_RAW_COUNT) 

#define BYTES_PER_ELEM_CP (4)
#define CP_PACKET_SIZE (uint8_t) ( (ROW_TAXELS_NB + COL_TAXELS_NB) * BYTES_PER_ELEM_CP ) 

// Temporary, packets have no ID and cannot be decyphered by this device. Strictly receiving and printing
#define RECEIVE_RAW_COUNTS_CONFIG 0 
#define RECEIVE_BIST_CP_CONFIG 0
#define TOTAL_PACKET_SIZE CP_PACKET_SIZE
*/

/******************************************************************************/


uint8_t ezi2cBuffer[BUFFER_SIZE]; // I2C buffer for communication with master 

static bool print_counter_flag; // Global marker for when counter rdy to send
bool full_buffer_flag; // Global marker for when counter rdy to send

void EZI2C_InterruptHandler(void)
{
    Cy_SCB_EZI2C_Interrupt(EZI2C_HW, &EZI2C_context);
}

void print_counter_int_handler()  // Custom function for handling interrupts from print_counter
{
    Cy_TCPWM_ClearInterrupt(print_counter_HW, print_counter_CNT_NUM, CY_TCPWM_INT_ON_TC);
    print_counter_flag++;
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
            full_buffer_flag = true;
            
            if (ezi2cBuffer[0] == raw_count_id)
            {
                
                const uint8_t bytes_per_taxel = 2;
                uint8_t current_taxel_bytes[bytes_per_taxel];   
                uint8_t payload_size = bytes_per_taxel * TAXELS_NB;
                uint8_t current_taxel_index = 0;
                
                for(int i = 1; i <= payload_size; i++)
                {        
                    current_taxel_bytes[i % bytes_per_taxel] = ezi2cBuffer[i];
                    
                    if(i % bytes_per_taxel == (bytes_per_taxel-1))
                    {
                        taxel_raw_values[current_taxel_index] = BytesToU16BE(current_taxel_bytes);
                        current_taxel_index++;
                     //   if(current_taxel_index >= TAXELS_NB)
                     //       current_taxel_index = 0;
                    }
                }    
            }
            
            else if (ezi2cBuffer[0] == parasitic_id)
            {
                
                const uint8_t bytes_per_cp_val = 4;
                uint8_t current_taxel_bytes[bytes_per_cp_val];   
                uint8_t payload_size = bytes_per_cp_val * (COL_TAXELS_NB * ROW_TAXELS_NB);
                uint8_t current_taxel_index = 0;
            }

        }
            
        if (print_counter_flag && full_buffer_flag)
        {
            printf("Msg ID: %d", ezi2cBuffer[0]);
            switch(ezi2cBuffer[0])
            {
                case raw_count_id:
                    for(int i = 1; i <= TAXELS_NB; i++)
                    {
                        printf("%d: %lu  ", i, (unsigned long)taxel_raw_values[i-1]);
                        if (!(i%7))
                            printf("\r\n");
                    }
                    printf("\r\n");
                    print_counter_flag = 0;
                    break;
                case parasitic_id:
                    for(int i = 0; i < (ROW_TAXELS_NB + COL_TAXELS_NB); i++)
                    {
                        printf("Cp %d: %lu \r\n", i, (unsigned long)cp_values[i]); 
                    }
                    break;
                default:
                    break;
            }
        full_buffer_flag = false;
        print_counter_flag = false;
        }

    NVIC_EnableIRQ(EZI2C_SCB_IRQ_cfg.intrSrc); // enable interrupts to call the i2c isr
    }
}                    
#if RECEIVE_RAW_COUNTS_CONFIG
            uint8 bytes[BYTES_PER_TAXEL_RAW_COUNT];
            int element_i = 0;

            for(int i = 0; i < TOTAL_PACKET_SIZE; i++)
            {        
                bytes[i % BYTES_PER_TAXEL_RAW_COUNT] = ezi2cBuffer[i];
                ezi2cBuffer[i] = 0;
                if(i % BYTES_PER_TAXEL_RAW_COUNT)
                {
                    raw_values[element_i] = BytesToU16BE(bytes);
                    element_i++;
                    if(element_i >= TAXELS_NB)
                        element_i = 0;
                }
            }           
#endif // RECEIVE_RAW_COUNTS_CONFIG

#if RECEIVE_BIST_CP_CONFIG

            uint8 bytes[BYTES_PER_ELEM_CP];
            int element_i = 0;
           
            for(int i = 0; i < TOTAL_PACKET_SIZE; i++)
            {        
                bytes[i % BYTES_PER_ELEM_CP] = ezi2cBuffer[i];
                
                if(i % BYTES_PER_ELEM_CP == (BYTES_PER_ELEM_CP-1) )
                {
                    cp_values[element_i] = BytesToU32LE(bytes);
                    element_i++;
                    if(element_i >= TAXELS_NB)
                        element_i = 0;
                }
            }           

#endif // RECEIVE_BIST_CP_CONFIG


#if RECEIVE_BIST_CP_CONFIG
            for(int i = 0; i < (ROW_TAXELS_NB + COL_TAXELS_NB); i++)
                {
                    printf("Cp %d: %lu \r\n", i, (unsigned long)cp_values[i]); 
                }
#endif // RECEIVE_BIST_CP_CONFIG
                         
#if RECEIVE_RAW_COUNTS_CONFIG
                for(int i = 1; i <= TAXELS_NB; i++)
                {
                    printf("%d: %lu  ", i, (unsigned long)raw_values[i-1]);
                    if (!(i%7))
                        printf("\r\n");
                }
                printf("\r\n\n");
                print_counter_flag = 0;
#endif



/* [] END OF FILE */
