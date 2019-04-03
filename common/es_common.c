#include "es_common.h"

uint32_t check_sum(uint32_t number,uint8_t *pData)
{
  uint32_t n,Data = 0;
    
  for(n=0; n<number; n++){
    Data += *(pData+n);
  }
    
  return Data;
}



