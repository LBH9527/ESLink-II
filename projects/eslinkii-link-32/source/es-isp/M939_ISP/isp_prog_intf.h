#ifndef _ISP_PROG_INTF_H_
#define _ISP_PROG_INTF_H_

//一次编程支持的长度，根据RAM大小可以修改
#define ISP_PRG_MINI_SIZE  1024 
//isp编程接口
extern struct  es_prog_ops isp_prog_intf ;
#endif
