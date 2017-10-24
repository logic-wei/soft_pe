#include <soft_pe.h>

#define debug_printf(fmt,...) 					\
	do{							\
		printk("soft_pe:"fmt,##__VA_ARGS__);		\
	}while(0)

void soft_pe_set_input_current(struct soft_pe *s_pe,long int current_ma)
{
	s_pe->driver->set_input_current(s_pe,current_ma);
}
long int soft_pe_get_input_voltage(struct soft_pe *s_pe)
{
	return s_pe->driver->get_input_voltage(s_pe);
}

/*pe 2.0*/
void soft_pe_2_0_set_levels_for(struct soft_pe *s_pe,int level,int msec)
{
	if(level){
		soft_pe_set_input_current(s_pe,500);
		msleep(msec);
	}else{
		soft_pe_set_input_current(s_pe,100);
		msleep(msec);
	}
}
void soft_pe_2_0_send_pattern(struct soft_pe *s_pe,enum pe_2_0_pattern  pattern)
{
	int i;
	int levels_8v[][2]={
		{1,50},{0,100},//0
		{1,50},{0,100},//0
		{1,100},{0,50},//1
		{1,50},{0,100},//0
		{1,100},{0,50},//1
		{1,150},//end
		{0,50},{1,50}//no wdt
	};
	int levels_wdt[][2]={
		{0,250}//wdt
	};
	switch(pattern){
		case PE_2_0_PATTERN_8V:
			for(i=0;i<ARRAY_SIZE(levels_8v);i+=1){
				soft_pe_2_0_set_levels_for(s_pe,levels_8v[i][0],levels_8v[i][1]);
			}
		break;
		case PE_2_0_PATTERN_WDT:
			for(i=0;i<ARRAY_SIZE(levels_wdt);i+=1){
				soft_pe_2_0_set_levels_for(s_pe,levels_wdt[i][0],levels_wdt[i][1]);
			}
		break;
		default:
			debug_printf("soft_pe 2.0:don't support this pattern\n");
	}
}
EXPORT_SYMBOL_GPL(soft_pe_2_0_send_pattern);

int soft_pe_thread(void *data)
{
	while(1){
		debug_printf("test\n");
		msleep(2000);
	}
}

struct soft_pe * soft_pe_register(struct soft_pe_driver *driver)
{
	struct soft_pe *s_pe;
	debug_printf("register\n");
	//alloc memery
	s_pe = kzalloc(sizeof(struct soft_pe), GFP_KERNEL);
	if (!s_pe)
		return (struct soft_pe *)-ENOMEM;
	s_pe->driver=driver;
	//create thread
	s_pe->thread=kthread_run(soft_pe_thread,s_pe,"soft_pe_thread");
	if(!s_pe->thread)
		return (struct soft_pe *)-ENOMEM;
	return s_pe;
}
EXPORT_SYMBOL_GPL(soft_pe_register);

void soft_pe_unregister(struct soft_pe *s_pe)
{
	debug_printf("unregister\n");
	kfree(s_pe);
}
EXPORT_SYMBOL_GPL(soft_pe_unregister);

static int __init soft_pe_init(void)
{
	return 0;
}

static void __exit soft_pe_exit(void)
{
}

module_init(soft_pe_init);
module_exit(soft_pe_exit);

MODULE_DESCRIPTION("soft pe protocol");
MODULE_AUTHOR("weipeng <weipeng@meizu.com>");
MODULE_LICENSE("GPL");
