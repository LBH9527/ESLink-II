#ifndef __ESLINK_DEF_H__
#define __ESLINK_DEF_H__   


typedef struct {  
    uint32_t version;        /*时序版本*/
} offline_info_t;

//时序信息
typedef struct {
    uint32_t id;             /*时序号*/   
    uint32_t version;        /*时序版本*/
    uint32_t checksum;       //时序校验和
} timing_info_t;  

//32位机目标芯片信息
typedef struct  {       
    uint32_t code_start;            //主程序区起始地址
    uint32_t code_size;             //主程序区容量
    uint32_t config_word_start;     //配置字信息区起始地址
    uint32_t config_word_size;      //配置字信息区容量
    uint32_t encrypt_addr;           //加密字地址
    uint32_t encrypt_value;          //加密字值
    uint32_t chipid_addr;           //芯片ID地址
    uint32_t chipid_value;          //芯片ID值
    
} es_target_cfg;  
//8位机芯片信息
typedef struct{
    uint16_t    version;        //版本号
    uint16_t    checksum;       //校验和
    uint8_t     reserved1;      //保留
    uint16_t    timing_id;      /*时序号*/
    uint8_t     reset_voltage;  //复位电平
    uint32_t    chipid;         //芯片ID
    uint32_t    reserved2;
    uint16_t    prog_id;        //编程时序号
    uint16_t    code_start;     //主程序区起始地址
    uint16_t    code_size;      //主程序区容量
}hr_target_cfg;   

/*
 *  序列号代码
 */
typedef struct {     
    uint32_t addr;           	//序列号起始地址
    uint32_t size;           	//序列号数据长度
    uint8_t data[16];       		//序列号代码值
}serial_number_t;

//编程协议帧接口
typedef struct{
    uint32_t  frame_head;
    uint8_t   device_type;
	uint8_t   fun_code ;
	uint16_t  data_length;
    uint8_t   state;
    uint16_t  checksum;
    uint8_t *wrbuf;
    uint8_t *rdbuf;
}prog_comm_frame_t;    


//调试协议帧接口
typedef struct{
    uint32_t  frame_head;           //帧头
	uint16_t  data_length;          //数据长度（接收、发送）
    uint8_t   device_addr;          //设备地址
	uint8_t   fun_code ;            //功能码                           
    
    uint8_t   device_state;         //设备状态
    uint16_t  checksum;             //校验和 
    uint8_t   *wrbuf;            //指向接收数组
    uint8_t   *rdbuf;            //指向发送数组
    
}debug_comm_frame_t;  


//编程操作接口
struct es_prog_ops {
    void    (*init) (es_target_cfg *target) ;
    error_t (*prog_init) (void);
    error_t (*prog_uninit) (void);
    error_t (*erase_chip) ( uint8_t para);
    error_t (*check_empty) (uint32_t *addr, uint32_t *data);  
    error_t (*read_chipid) (uint32_t *data);  
//    error_t (*chipid_check) (void);
    error_t (*read_chip_chksum) (uint32_t *data);
    error_t (*encrypt_chip) (void);    
    error_t (*program_info) (uint32_t adr, uint8_t *buf, uint32_t size,uint32_t *failed_addr);
    error_t (*read_info) (uint32_t adr, uint8_t *buf, uint32_t size); 
    error_t (*verify_info) (uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data); 
    error_t (*program_flash) (uint32_t adr, uint8_t *buf, uint32_t size,uint32_t *failed_addr);
    error_t (*read_flash) (uint32_t adr, uint8_t *buf, uint32_t size);   
    error_t (*verify_flash) (uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);    
    error_t (*program_info_all)   (uint32_t *failed_addr);
    error_t (*program_all) (uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr); 
    error_t (*verify_all)  (uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr, uint32_t *failed_data);
};



#endif
