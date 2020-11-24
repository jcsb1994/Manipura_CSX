#include <project.h>
#include "i2c_api_utils.h"
#include "message.h"

/*
#define BYTES_PER_TAXEL_RAW_COUNT (2)
#define TOTAL_RAW_COUNTS_PACKET_SIZE (uint8_t) (TAXELS_NB*BYTES_PER_TAXEL_RAW_COUNT) 

#define BYTES_PER_TAXEL_CP_BIST (4)
#define TOTAL_CP_BIST_PACKET_SIZE (uint8_t) ( (ROW_TAXELS_NB + COL_TAXELS_NB) * BYTES_PER_TAXEL_CP_BIST ) 
*/

#define SEND_RAW_COUNTS_CONFIG 1
#define SEND_BIST_CP_CONFIG 1
#define SEND_BIST_EXTC_CONFIG 1
#define SEND_BIST_VDDA_CONFIG 1

void u16tobytes(uint8_t *out, uint16_t in)  //be
{
        out[1] = (in >> 8); 
        out[0] = (in);      
}

void u32tobytes(uint8_t *out, uint32_t in)
{
        out[3] = (in >> 24); 
        out[2] = (in >> 16); 
        out[1] = (in >> 8); 
        out[0] = (in);      
}
    
void init_hardware(void)
{
    CyGlobalIntEnable;
    
    I2C_Start() ;
    i2c_set_slave_address(0x08) ;
    
    CapSense_Start();
    CapSense_InitializeAllBaselines();
    CapSense_ScanAllWidgets();
    
    for(int i = 0; i < 5; i++)
    {
        ledPin2_Write(1);
        CyDelay(100);
        ledPin2_Write(0);
        CyDelay(100);
    }
}


int main()
{
    init_hardware() ;   
    
        for(int i = 0; i < 5; i++)
    {
        ledPin_Write(1);
        CyDelay(100);
        ledPin_Write(0);
        CyDelay(100);
    }

    for(;;)
    {
        if (CapSense_IsBusy() == CapSense_NOT_BUSY) {
        
        CapSense_RunTuner();

#if SEND_RAW_COUNTS_CONFIG
        
            const uint8_t values_nb = TAXELS_NB;
            uint taxel_raw_values[values_nb];
            const uint8_t bytes_per_value = 2;
            const uint8_t packet_size = (bytes_per_value * values_nb) + 1;
            uint8_t raw_counts_message_bytes[packet_size]; // + 1 for ID
            
            raw_counts_message_bytes[0] = raw_count_id;
            for(int i = 0; i < values_nb; i++)
            {
                taxel_raw_values[i] = CapSense_dsRam.snsList.touchpad0[i].raw[0];
                u16tobytes(&raw_counts_message_bytes[(i*bytes_per_value) + 1], taxel_raw_values[i]);
            }       
            
            i2c_writeRegs(0, raw_counts_message_bytes, packet_size);
        
#endif // SEND_RAW_COUNTS_CONFIG
        
        
#if SEND_BIST_CP_CONFIG
        {
            CapSense_TST_MEASUREMENT_STATUS_ENUM state; //can be accessed with another API fct
           
            cystatus status;

            const uint8_t values_nb = (ROW_TAXELS_NB + COL_TAXELS_NB);
            uint32_t capsense_cp_values[values_nb];
            const uint8_t bytes_per_value = 4;
            const uint8_t packet_size = (bytes_per_value * values_nb) + 1; 
            uint8_t capsense_cp_packet_bytes[packet_size];

            capsense_cp_packet_bytes[0] = parasitic_id;
            for (int i = 0; i < values_nb; i ++)
            {
                capsense_cp_values[i] = CapSense_GetSensorCapacitance(CapSense_TOUCHPAD0_WDGT_ID, i, &state);
                u32tobytes(&capsense_cp_packet_bytes[(i*bytes_per_value) + 1], capsense_cp_values[i]);
            }
            
            i2c_writeRegs(0, capsense_cp_packet_bytes, packet_size);

            ledPin_Write(0);
        }    
#endif // SEND_BIST_CP_CONFIG


#if SEND_BIST_EXTC_CONFIG

        {
            uint32_t cinta_val = CapSense_GetExtCapCapacitance(CapSense_TST_CINTA_ID);  //pF
            uint32_t cintb_val = CapSense_GetExtCapCapacitance(CapSense_TST_CINTB_ID);  //pF
            const uint8_t nb_values = 2;
            const uint8_t bytes_per_value = 4;
            const uint8_t packet_size = bytes_per_value*nb_values + 1; 
            uint8_t capsense_extc_packet_bytes[packet_size];
            capsense_extc_packet_bytes[0] = ext_cap_id;
            
            u16tobytes(&capsense_extc_packet_bytes[1], cinta_val);
            u16tobytes(&capsense_extc_packet_bytes[(1 + bytes_per_value)], cintb_val);
            i2c_writeRegs(0, capsense_extc_packet_bytes, packet_size);         
        }
    
#endif // SEND_BIST_EXTC_CONFIG


#if SEND_BIST_VDDA_CONFIG
    
        {
            uint16_t vdda_val = CapSense_GetVdda();
            const uint8_t bytes_per_value = 2;
            const uint8_t packet_size = bytes_per_value + 1; 
            uint8_t capsense_vdda_packet_bytes[packet_size];
            capsense_vdda_packet_bytes[0] = vdda_id;
            
            u16tobytes(&capsense_vdda_packet_bytes[1], vdda_val);
            i2c_writeRegs(0, capsense_vdda_packet_bytes, packet_size);           
        }
    
#endif // #define SEND_BIST_VDDA_CONFIG

        CapSense_UpdateAllBaselines();
        CapSense_ScanAllWidgets();
    }
    }
}


/* [] END OF FILE */
