#ifndef _EEPROM_H
#define _EEPROM_H 

#define  FM24C04
#ifdef FM24C02
	#define EE_MODEL_NAME		"FM24C02"
	#define EE_DEV_ADDR			0xA0		/* 设备地址 */
//	#define EE_PAGE_SIZE		8			/* 页面大小(字节) */
	#define EE_SIZE				256			/* 总容量(字节) */
	#define EE_ADDR_BYTES		1			/* 地址字节个数 */
	#define EE_ADDR_A8			0			/* 9bit ADDRESS */
#endif

#ifdef FM24C04
	#define EE_MODEL_NAME		"FM24C04"
	#define EE_DEV_ADDR			0xA0		/* 设备地址 */
//	#define EE_PAGE_SIZE		16			/* 页面大小(字节) */
	#define EE_SIZE				512			/* 总容量(字节) */
	#define EE_ADDR_BYTES		1			/* 地址字节个数 */
	#define EE_ADDR_A8			1			/* 9bit ADDRESS */
#endif



#define EE_SERIAL_NUMBER_ADDR     0x0000    //序列号地址     

uint8_t fm24cxx_init(void);
uint8_t fm24cxx_write(uint16_t addr, uint8_t *str, uint16_t len);
uint8_t fm24cxx_read(uint16_t addr, uint8_t *str, uint16_t len);
#endif
