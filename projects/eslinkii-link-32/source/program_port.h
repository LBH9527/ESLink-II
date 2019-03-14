#ifndef _PROGRAM_PORT_H
#define _PROGRAM_PORT_H

void es_set_trget_power(trget_power_t power);

void serial_number_intercept_write(serial_number_t *sn, uint32_t addr, uint8_t *data, uint32_t size);
void eslink_set_target_hold_reset(uint8_t delay_ms);
void eslink_set_target_reset_run(uint8_t delay_ms);
void eslink_set_target_power_reset(uint8_t delay_ms); 

#endif
