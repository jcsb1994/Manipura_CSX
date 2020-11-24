
#include "project.h"
#include <stdio.h> 
#include "message.h"


/************************** Macro Definitions *********************************/
    
#define BytesToU16BE(B) (       \
      (uint16_t)((B)[1]) << 8   \
    | (uint16_t)((B)[0])        \
)
    
#define BytesToU32BE(B) (       \
      (uint32_t)((B)[3]) << 24  \
    | (uint32_t)((B)[2]) << 16  \
    | (uint32_t)((B)[1]) << 8   \
    | (uint32_t)((B)[0])        \
)


/******************************************************************************/
// a travailler: revoir le buffer size, voir si capable de changer sans tout fucker, sinon 99 sera size pr tout
// lire des messages fixes avec bytestouint32 et voir si fctne
// cp lecture 

uint8_t ezi2cBuffer[(1 + (2*49))]; 

static uint32_t print_counter_flag_bitmask;

enum bitmask_pos
{
    RAW_PRINT_BITMASK_POS = 0,
    PARASITIC_BITMASK_POS = 1,
    EXTC_BITMASK_POS,
    VDDA_BITMASK_POS
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

uint32_t test_val[4] = {0B1, 0B10, 0B100, 0B1000};
uint8_t test_i2c_fake_buffer[1+ (2*49)];

void unpack_i2c_packet(uint8_t nb_values_in_packet, uint8_t bytes_per_value, uint32_t * values, uint8_t buffer_head_size)
{
    uint8_t current_value_bytes[bytes_per_value];   
    uint16_t payload_size = bytes_per_value * nb_values_in_packet;
    uint8_t current_value_index = 0;
    
    if(!nb_values_in_packet | !bytes_per_value)
        return;
    
    for(int i = 1; i <= payload_size; i++)
    {        
        current_value_bytes[(i % bytes_per_value)] = ezi2cBuffer[i];   //  test_i2c_fake_buffer[i+buffer_head_size]; //
        
        if(!(i % bytes_per_value))
        {
           // values[current_value_index] = BytesToU16LE(current_value_bytes);
            
            
            values[current_value_index] = 0;
            
            for(int bi = (bytes_per_value-1); bi >= 0; bi --)
            {
                values[current_value_index] |= ((uint32_t)(current_value_bytes[bi]) << (bi * 8));    
            }
            
            current_value_index++;
        }
    }
}

int main(void)
{
    for (uint16 i = 1; i < (1+(2*49)); i+=2 )
    {
        test_i2c_fake_buffer[i+1] = i>>8;       //big byte at end (BE)
        test_i2c_fake_buffer[i] = i & 0xFF;
    }
    
    
    uint32 ezi2cState;  // stores ezi2c status
    
    __enable_irq(); /* Enable global interrupts. */

    i2c_init();

    UART_Start();
    
    printf_counter_setup();
    
    uint32_t taxel_raw_values[TAXELS_NB];
    uint32_t cp_values[ROW_TAXELS_NB + COL_TAXELS_NB];
    uint32_t extc_values[2];   
    uint16_t vdda_value;
    
    for(;;)
    {     
        NVIC_DisableIRQ(EZI2C_SCB_IRQ_cfg.intrSrc);     // Disable the EZI2C interrupts so that ISR is not serviced while checking for EZI2C status 
        ezi2cState = Cy_SCB_EZI2C_GetActivity(EZI2C_HW, &EZI2C_context);    // check status
        
        /* Write complete without errors: parse packets, otherwise ignore */
        if((0u != (ezi2cState & CY_SCB_EZI2C_STATUS_WRITE1)) && (0u == (ezi2cState & CY_SCB_EZI2C_STATUS_ERR)))
        {
  
            if (ezi2cBuffer[0] == raw_count_id)
            {
            /*    
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
                    }
                }
             */   
                unpack_i2c_packet(TAXELS_NB, 2, taxel_raw_values, 1);
                
                if(print_counter_flag_bitmask & (1 << RAW_PRINT_BITMASK_POS))
                {                    
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
                
                for(int i = 0; i < payload_size; i++)
                {        
                    current_value_bytes[i % bytes_per_cp_val] = ezi2cBuffer[i+1];
                    
                    if((i % bytes_per_cp_val) == (bytes_per_cp_val - 1))
                    {
                        cp_values[current_value_index] = BytesToU32BE(current_value_bytes);
                        current_value_index++;
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
            /*
            else if (ezi2cBuffer[0] == ext_cap_id)
            {
                const uint8_t nb_extc_val = 2;
                const uint8_t bytes_per_extc_val = 4;
                
                uint8_t vdda_value_bytes[bytes_per_vdda_val];   
                //uint8_t current_value_index = 0;
                

                for(int i = 0; i < payload_size; i++)
                {        
                    current_value_bytes[i % bytes_per_extc_val] = ezi2cBuffer[i+1];
                    
                    if((i % bytes_per_extc_val) == (bytes_per_extc_val - 1))
                    {
                        extc_values[current_value_index] = BytesToU32LE(current_value_bytes);
                        current_value_index++;
                    }
                }
                
                if (print_counter_flag_bitmask & (1 << EXTC_BITMASK_POS))
                {
                    printf("CINTA: %u , CINTB: %u \r\n", extc_values[0], extc_values[1]);                 
                    printf("\r\n");
                    
                    print_counter_flag_bitmask &= ~(1 << EXTC_BITMASK_POS);
                }
            }
            
            else if (ezi2cBuffer[0] == vdda_id)
            {
                //uint16_t vdda_val = BytesToU16LE(current_taxel_bytes) 
                const uint8_t vdda_bytes = 2;
                uint8_t vdda_value_bytes;   
                
                uint8_t current_value_index = 0;
                
                for(int i = 0; i < bytes_per_vdda_val; i++)
                {   
                    vdda_value_bytes[i % bytes_per_vdda_val] = ezi2cBuffer[i+1];
                    
                    if((i % bytes_per_vdda_val) == (bytes_per_vdda_val - 1))
                    {
                        vdda_value = BytesToU32LE(vdda_value_bytes);
                    }
                }
                
                if (print_counter_flag_bitmask & (1 << EXTC_BITMASK_POS))
                {
                    printf("CINTA: %u , CINTB: %u \r\n", extc_values[0], extc_values[1]);                 
                    printf("\r\n");
                    
                    print_counter_flag_bitmask &= ~(1 << EXTC_BITMASK_POS);
                }
            }
            */
        }
    
    NVIC_EnableIRQ(EZI2C_SCB_IRQ_cfg.intrSrc); // enable interrupts to call the i2c isr
    }
}                    


/* [] END OF FILE */
