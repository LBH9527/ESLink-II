#include "RTL.h"
#include "rl_usb.h"
#include "ES_ISP.h"
#include "string.h"

#include "fsl_common.h"         //for assert
static uint32_t usb_buffer[1024 / sizeof(uint32_t)];

uint8_t usb_isp_buf[1024] ;         //用做U盘的存储介质，数组长度位协议帧最大长度。
//static OS_MUT sync_mutex;
//static OS_TID sync_thread = 0;



//static void sync_init(void)
//{
//    sync_thread = os_tsk_self();
//    os_mut_init(&sync_mutex);
//}

//#define FREE_SEM_INIT_COUNT          (DAP_PACKET_COUNT)
#define FREE_SEM_INIT_COUNT          4
#define PROC_SEM_INIT_COUNT          0
#define SEND_SEM_INIT_COUNT          0

//static OS_SEM free_sem;             
//static OS_SEM proc_sem;
//static OS_SEM send_sem;

//static OS_MUT msc_mutex;



void usbd_msc_init(void)
{
//    sync_init();
    // Set mass storage parameters
//    USBD_MSC_MemorySize = g_flash.total_size;
//    USBD_MSC_BlockSize  = g_flash.sector_size;
    USBD_MSC_MemorySize = ISP_PACKET_SIZE;         
    USBD_MSC_BlockSize  = 512;
    USBD_MSC_BlockGroup = sizeof(usb_buffer) / USBD_MSC_BlockSize;;
    USBD_MSC_BlockCount = USBD_MSC_MemorySize / USBD_MSC_BlockSize;
    USBD_MSC_BlockBuf   = (uint8_t *)usb_buffer;
//    vfs_state = VFS_MNGR_STATE_DISCONNECTED;
//    vfs_state_next = VFS_MNGR_STATE_DISCONNECTED;
//    time_usb_idle = 0;
    USBD_MSC_MediaReady = __TRUE;
    
//    os_sem_init(&free_sem, FREE_SEM_INIT_COUNT);
//    os_sem_init(&proc_sem, PROC_SEM_INIT_COUNT);
//    os_sem_init(&send_sem, SEND_SEM_INIT_COUNT);
//    os_mut_init(&msc_mutex);
    
    eslink_state = ISP_STATE_DISCONNECTED;
    
}
void usbd_msc_read_sect(U32 block, U8 *buf, U32 num_of_blocks)
{
    uint32_t i;
    uint8_t *p;
//    sync_assert_usb_thread();
    // dont proceed if we're not ready
    if (!USBD_MSC_MediaReady) {
        return;
    }
    p = usb_isp_buf;
    if(eslink_state == ISP_STATE_DISCONNECTED){
        memset(usb_isp_buf, 0, USBD_MSC_BlockSize);
        usb_isp_buf[0] = 0xBA;
        usb_isp_buf[1] = 0xDC;
        usb_isp_buf[2] = 0xCD;
        usb_isp_buf[3] = 0xAB;
        usb_isp_buf[4] = 0x51;
        for(i=0; i<512; i++){         
            *buf++ = usb_isp_buf[i] ;
        }  
      
    }else{
//        for(i=0; i<1024; i++){         
//            *buf++ = usb_isp_buf[i] ;
        for(i=0;i<num_of_blocks;i++){
            memcpy(buf, p, USBD_MSC_BlockSize);
            p += USBD_MSC_BlockSize;
        }  
        
//        os_sem_send(&free_sem);
    }
       
    // indicate msc activity
//    main_blink_msc_led(MAIN_LED_OFF);
//    vfs_read(sector, buf, num_of_sectors);
//    p = block*USBD_MSC_BlockSize;
//    for(i=0; i<num_of_blocks; i++)
//    {
//        spi_flash_read(buf, p, USBD_MSC_BlockSize);
//        p += USBD_MSC_BlockSize;
//    }
    
    
}

// USB MSC Callback: when data is received from the host
void usbd_msc_write_sect(U32 block, U8 *buf, U32 num_of_blocks)
{
    uint32_t i;
    uint32_t copy_size;
//    uint8_t *p;
    
//    sync_assert_usb_thread();
    if (!USBD_MSC_MediaReady) {
        return;
    }
//    if (eslink_state == ISP_STATE_DISCONNECTED){
//        return;
//    }

    // Store data into request packet buffer
    // If there are no free buffers discard the data
//    if (os_sem_wait(&free_sem, 0) == OS_R_OK) {
//        memcpy(usb_isp_buf, buf, USBD_MSC_BlockSize);
//        os_sem_send(&proc_sem);
//    }else{
//        //busy
//    }

//    p = usb_isp_buf;
//    for(i=0; i<num_of_blocks; i++){
//        memcpy(p, buf, USBD_MSC_BlockSize); 
//        p += USBD_MSC_BlockSize;
//    }
//    if(num_of_blocks == 2)
//        assert(0);

    copy_size = num_of_blocks * USBD_MSC_BlockSize;
    memcpy(usb_isp_buf, buf, copy_size);
    isp_process_command(usb_isp_buf);        
    
//    memcpy(usb_isp_buf, buf, 512);
    // Restart the disconnect counter on every packet
    // so the device does not detach in the middle of a
    // transfer.
//    time_usb_idle = 0;

//    if (TRASNFER_FINISHED == file_transfer_state.transfer_state) {
//        return;
//    }

    // indicate msc activity
//    main_blink_msc_led(MAIN_LED_OFF);

//    if (TRASNFER_FINISHED == file_transfer_state.transfer_state) {
//        return;
//    }
//    file_data_handler(sector, buf, num_of_sectors);
    
//        spi_flash_write( buf, sector, num_of_sectors);
    
//     p = block*USBD_MSC_BlockSize;
//        for(i=0; i<num_of_blocks; i++)
//    {
//        sf_erase_sector(p);
//        spi_flash_write(buf, p, USBD_MSC_BlockSize);
//        p += USBD_MSC_BlockSize;
//    }


}


//// ES-ISP task
//__task void isp_process(void *argv)
//{
//    while (1) {
//        // Process ISP Command
//        os_sem_wait(&proc_sem, 0xFFFF);
//        isp_process_command(usb_isp_buf);        
////        os_sem_send(&send_sem);
//    }
//}
