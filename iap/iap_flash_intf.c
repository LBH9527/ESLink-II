
#include "es_common.h" 
#include "cortex_m.h"
#include "fsl_flash.h"
#include "iap_flash_intf.h"
#include "eslink_addr.h" 
#include "update.h"


flash_config_t iap_flash; //!< Storage for flash driver.

/*
 *  iap init
 *    Return Value:   0 - OK,  1 - Failed
 */
uint32_t iap_Init(void)
{
    status_t result;
    
    cortex_int_state_t state = cortex_int_get_and_disable();
#if defined (WDOG)
    /* Write 0xC520 to the unlock register */
    WDOG->UNLOCK = 0xC520;
    /* Followed by 0xD928 to complete the unlock */
    WDOG->UNLOCK = 0xD928;
    /* Clear the WDOGEN bit to disable the watchdog */
    WDOG->STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
#else
    SIM->COPC = 0x00u;
#endif
    
    memset(&iap_flash, 0, sizeof(flash_config_t));

    /* Setup flash driver structure for device and initialize variables. */
    result = FLASH_Init(&iap_flash);
    if (kStatus_FLASH_Success != result)
    {
        return FALSE;
    }
    cortex_int_restore(state);
    return TRUE;
} 

/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */
//static uint32_t EraseChip(void)
//{
////    cortex_int_state_t state = cortex_int_get_and_disable();
//    int status = FLASH_EraseAll(&iap_flash, kFLASH_ApiEraseKey);
//    if (status == kStatus_Success)
//    {
//        status = FLASH_VerifyEraseAll(&iap_flash, kFLASH_MarginValueNormal);
//    }
////    cortex_int_restore(state);
//    return status;
//}

/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */
uint32_t iap_erase_sector(uint32_t adr)
{
    cortex_int_state_t state = cortex_int_get_and_disable();
    int status = FLASH_Erase(&iap_flash, adr, iap_flash.PFlashSectorSize, kFLASH_ApiEraseKey);
    if (status == kStatus_Success)
    {
        status = FLASH_VerifyErase(&iap_flash, adr, iap_flash.PFlashSectorSize, kFLASH_MarginValueNormal);
    }
    cortex_int_restore(state);
    if( status != kStatus_Success)
        return 1;
    return TRUE;
}

/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */
uint32_t iap_flash_program(uint32_t adr, uint8_t *buf,  uint32_t sz)
{
    if (adr & 0x03)
        return FALSE;
    cortex_int_state_t state = cortex_int_get_and_disable();
    int status = FLASH_Program(&iap_flash, adr, (uint32_t*)buf, sz);
    if (status == kStatus_Success)
    {
        // Must use kFlashMargin_User, or kFlashMargin_Factory for verify program
        status = FLASH_VerifyProgram(&iap_flash, adr, sz,
                              (uint32_t*)buf, kFLASH_MarginValueUser,
                              NULL, NULL);
    }
    cortex_int_restore(state);
    if( status != kStatus_Success)
        return FALSE;
    return TRUE;
}
/*
 *   erase app chip
 *    Parameter:      updt_start:  Page Start Address
 *                    updt_size:   Page Size
 *    Return Value:   0 - OK,  1 - Failed
 */
uint32_t iap_erase_chip(uint32_t updt_start, uint32_t updt_size)
{
    uint32_t addr ;
    for (uint32_t size = 0; size < updt_size; size += ESLINK_SECTOR_SIZE) 
    {
        addr = updt_start + size;
        if( iap_erase_sector(addr))
            return 1;         
    }

    return 0;
}
uint32_t iap_flash_checksum( uint32_t addr, uint32_t size)
{
    uint32_t data = 0;
    uint32_t sum = 0;
     for (uint32_t i = 0; i < size; i+=4)
    {
        data = *(volatile uint32_t *)(addr + i );
        sum += (data & 0xFF) ;
        sum += (data >> 8) & 0xFF;
        sum += (data >> 16) & 0xFF;
        sum += (data >> 24) & 0xFF;
    }
    return sum;
    
}




#if 0
#define BUFFER_LEN 16
void flash_test(void )
{
    static flash_config_t s_flashDriver;
    status_t result;
    flash_security_state_t securityStatus = kFLASH_SecurityStateNotSecure; 
    uint32_t pflashSectorSize = 0;
    
    uint32_t data = 0;
    uint8_t buffer[BUFFER_LEN];
    uint8_t s_buffer_rbc[BUFFER_LEN];
    uint32_t destAdrss =  0x18000;
    for (uint8_t i = 0; i < BUFFER_LEN; i++)
    {
        buffer[i] = i;
    }
        
    iap_Init();
//    result = FLASH_Init(&s_flashDriver);
//    /* Check security status. */
//    result = FLASH_GetSecurityState(&s_flashDriver, &securityStatus);

//    if (kStatus_FLASH_Success != result)     
//        while(1);
//    
//    if (kFLASH_SecurityStateNotSecure != securityStatus)
//        while(1);
//     FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashSectorSize, &pflashSectorSize);
    iap_erase_sector( destAdrss);
//    result = FLASH_Erase(&s_flashDriver, destAdrss, pflashSectorSize, kFLASH_ApiEraseKey);
    for (uint32_t i = 0; i < BUFFER_LEN; i+= 4)
    {
    
        data = *(volatile uint32_t *)(destAdrss + i );
        s_buffer_rbc[i] = data & 0xff;
        s_buffer_rbc[i + 1] = (data >> 8) & 0xFF;
        s_buffer_rbc[i + 2] = (data >> 16) & 0xFF;
        s_buffer_rbc[i + 3] = (data >> 24) & 0xFF;  
        
        if (s_buffer_rbc[i] != 0xff)
        {
            while(1);
        }
    }

        
//    result = FLASH_Program(&iap_flash, destAdrss, buffer, sizeof(buffer));

//     result = FLASH_VerifyProgram(&iap_flash, destAdrss, sizeof(buffer), buffer, kFLASH_MarginValueUser,
//                                     NULL, NULL);
//    if (kStatus_FLASH_Success != result)
//    {
//        while(1);
//    }
    
    iap_flash_program(destAdrss, buffer,16 );
    for (uint32_t i = 0; i < BUFFER_LEN; i+= 4)
    {
    
        data = *(volatile uint32_t *)(destAdrss + i );
        s_buffer_rbc[i] = data & 0xff;
        s_buffer_rbc[i + 1] = (data >> 8) & 0xFF;
        s_buffer_rbc[i + 2] = (data >> 16) & 0xFF;
        s_buffer_rbc[i + 3] = (data >> 24) & 0xFF;  
        
//        if (s_buffer_rbc[i] != 0xff)
//        {
//            while(1);
//        }
    }
}
#endif

