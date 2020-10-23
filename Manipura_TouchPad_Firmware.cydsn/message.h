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

#define ROW_TAXELS_NB (7)
#define COL_TAXELS_NB (7)
#define TAXELS_NB (ROW_TAXELS_NB * COL_TAXELS_NB)

#define BUFFER_SIZE (1 + (4 * TAXELS_NB) )


enum message_id
{
    raw_count_id = 'r',
    parasitic_id = 'p'
};

/* [] END OF FILE */
