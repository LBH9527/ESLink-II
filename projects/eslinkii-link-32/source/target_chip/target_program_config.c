#include "eslink.h"
//#include "stdbool.h" 
#include "target_program_config.h"  

void config_set_auto_rst(bool on)
{
    // Do nothing
}

void config_set_automation_allowed(bool on)
{
    // Do nothing
}

void config_set_overflow_detect(bool on)
{
    // Do nothing
}

bool config_get_auto_rst()
{
    return false;
}

bool config_get_automation_allowed()
{
    return true;
}

bool config_get_overflow_detect()
{
    return false;
}

//是否编程校验
bool config_isp_verify_program()
{
    return false;
}


