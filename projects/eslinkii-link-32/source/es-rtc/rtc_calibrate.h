#ifndef __RTC_CALIBRATE_H
#define __RTC_CALIBRATE_H

void rtc_Init(void);
void rtc_pwm_out(uint8_t mode);

error_t rtc_calibration_handler( void);
error_t rtc_calibration_verify(uint8_t *data);
#endif
