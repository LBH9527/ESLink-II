//脱机方案分区管理

#include "RTL.h"
#include "es_common.h"
#include "error.h"
#include "offline.h"
#include "offline_def.h"


int prj_partition_init(void)
{
    /* load partition table from the end address PRJ_PART_TABLE_END_OFFSET, error return 0 */
    long part_table_offset = PRJ_PART_TABLE_END_OFFSET;
    size_t table_num = 0, table_item_size = 0;
    uint8_t part_table_find_ok = 0;
    uint32_t read_magic_word;
    prj_partition_t new_part = NULL;
    
}

error_t ofl_prj_init( struct ofl_prj *prj)
{
    
    
}
//offline prj create
int ofl_prj_open( struct ofl_prj *prj)
{
    
    
}

int ofl_prj_write(const struct ofl_prj *prj, uint32_t addr, const uint8_t *buf, size_t size)
{
    
}
int ofl_prj_read(const struct prj_partition *part, uint32_t addr, uint8_t *buf, size_t size)
{
    
}

uint8_t prj_manager(offline_state_t state,uint32_t ops,  uint32_t size, uint8_t *buf)
{
    FILE *fout;   
    uint8_t result;
  
/*    switch(offline_wr_state)
    {
        case STATE_WR_INFO:
            result = prj_open(fout, size, buf);
            prj_module.part[OFFLINE_INFIO].type = OFFLINE_INFIO;
            prj_module.part[OFFLINE_INFIO].offset = 0;
            prj_module.part[OFFLINE_INFIO].size = size;
            break;
        case STATE_WR_TIMING_START:
            result = offline_prj_write(fout, size, buf) ;
            prj_module.part[OFFLINE_TIMING].type = OFFLINE_TIMING;
            prj_module.part[OFFLINE_TIMING].offset = 0;
            prj_module.part[OFFLINE_INFIO].size += size;
            break;
        case STATE_WR_TIMING:
            result = offline_prj_write(fout, size, buf) ;
            prj_module.part[OFFLINE_INFIO].size += size;
            
            break;
        case STATE_WR_TIMING_END:
            //返回校验
//            offline_prj_read(fout,prj_module.part[OFFLINE_INFIO].size, );
            break;
        case STATE_WR_CONFIG:
            
            break;
        case STATE_WR_IMAGE:
            break;
        
        
        
        
        
    }
 	
*/	
}
//计算方案个数

const struct prj_partition *prj_partition_find(const char *name)
{
    
}

const struct prj_partition *get_partition_table(size_t *len)
{

//    *len = partition_table_len;

//    return partition_table;
}


//方案添加
uint8_t prj_add(void)
{
    
    
}

//方案删除
uint8_t prj_del(void)
{
    
    
}



int olf_prj_erase(const struct olf_prj *prj, uint32_t addr, size_t size)
{


}    

int olf_prj_erase_all(const struct olf_prj *part)
{
    return fal_partition_erase(part, 0, part->len);
}



//static struct prj_item_list prj_item_head;

//int offline_prj_item_init(void) 
//{

//	memset(&prj_item_head, 0x0, sizeof(struct prj_item_list_node));
//	list_init(&prj_item_head.list);
//	prj_item_head.obj.name = "NO INFO";
//	return 0;
//}

//prj_item_list_t offline_prj_item_create(prj_item_t item)
//{
////    list_t *p_list;
//    struct prj_item_list *new_node;
////	struct prj_item_list_node *info_node;
////	struct prj_item_list_node *_info_node;
////	p_list = &prj_item_head.list;
//    
//    new_node = (struct prj_item_list *) malloc(
//			sizeof(struct prj_item_list));
//    
//    new_node->obj = item;
//	
//	list_insert_after(&prj_item_head.list, &new_node->list);
//	return new_node;
//}
//    

//int offline_prj_item_delete(prj_item_list_t item)
//{
//    list_remove(&(item->list));

//}	