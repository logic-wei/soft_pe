#include <soft_pe.h>

#define debug_printf(fmt,...) 					\
	do{							\
		printk("soft_pe:"fmt,##__VA_ARGS__);		\
	}while(0)

void soft_pe_set_input_current(struct soft_pe *s_pe,int current_ma)
{
	s_pe->driver->set_input_current(s_pe,current_ma);
}
int soft_pe_get_input_voltage(struct soft_pe *s_pe)
{
	return s_pe->driver->get_input_voltage(s_pe);
}
void soft_pe_begin(struct soft_pe *s_pe)
{
	debug_printf("pe_begin\n");
	s_pe->driver->pe_begin(s_pe);
}
void soft_pe_end(struct soft_pe *s_pe)
{
	debug_printf("pe_end\n");
	s_pe->driver->pe_end(s_pe);
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
		{0,100},//start
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
	soft_pe_begin(s_pe);
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
	soft_pe_end(s_pe);
}
EXPORT_SYMBOL_GPL(soft_pe_2_0_send_pattern);
void soft_pe_1_0_send_pattern(struct soft_pe *s_pe,enum pe_1_0_pattern pattern)
{
	int i;
	int levels_increase[][2]={
		{0,100},//start
		{1,100},{0,100},
		{1,100},{0,100},
		{1,300},{0,100},
		{1,300},{0,100},
		{1,300},{0,100},
		{1,500},
		{0,100},{1,100}//stop
	};
	int levels_decrease[][2]={
		{0,100},//start
		{1,300},{0,100},
		{1,300},{0,100},
		{1,300},{0,100},
		{1,100},{0,100},
		{1,100},{0,100},
		{1,500},
		{0,100},{1,100}//stop
	};
	int levels_wdt[][2]={
		{0,250}
	};
	int (*levels_selected)[2]=NULL;
	int levels_length=0;
	switch(pattern){
		case PE_1_0_PATTERN_INCREASE:
			levels_selected=levels_increase;
			levels_length=ARRAY_SIZE(levels_increase);
		break;
		case PE_1_0_PATTERN_DECREASE:
			levels_selected=levels_decrease;
			levels_length=ARRAY_SIZE(levels_decrease);
		break;
		case PE_1_0_PATTERN_WDT:
			levels_selected=levels_wdt;
			levels_length=ARRAY_SIZE(levels_wdt);
		break;
		default:
			levels_selected=NULL;
			levels_length=0;
	}
	if(levels_selected==NULL)
		debug_printf("soft_pe 1.0:don't support this pattern\n");
	else{
		soft_pe_begin(s_pe);
		for(i=0;i<levels_length;i+=1)
				soft_pe_2_0_set_levels_for(s_pe,levels_selected[i][0],levels_selected[i][1]);
		soft_pe_end(s_pe);
	}
}
EXPORT_SYMBOL_GPL(soft_pe_1_0_send_pattern);

enum pe_state soft_pe_get_state(struct soft_pe *s_pe)
{
	debug_printf("%s entry\n",__func__);
	return s_pe->state;
}
EXPORT_SYMBOL_GPL(soft_pe_get_state);

void soft_pe_start_pump_up(struct soft_pe *s_pe)
{
	debug_printf("%s entry\n",__func__);
	s_pe->event=PE_EVENT_START_PUMP_UP;
	wake_up(&s_pe->wait_queue);
}
EXPORT_SYMBOL_GPL(soft_pe_start_pump_up);

void soft_pe_start_pump_down(struct soft_pe *s_pe)
{
	debug_printf("%s entry\n",__func__);
	s_pe->event=PE_EVENT_START_PUMP_DOWN;
	wake_up(&s_pe->wait_queue);
}
EXPORT_SYMBOL_GPL(soft_pe_start_pump_down);

void soft_pe_reset(struct soft_pe *s_pe)
{
	debug_printf("%s entry\n",__func__);
	s_pe->state=PE_STATE_RESET;
}
EXPORT_SYMBOL_GPL(soft_pe_reset);

void soft_pe_set_private(struct soft_pe *s_pe,void *private)
{
	debug_printf("%s entry\n",__func__);
	s_pe->private=private;
}
EXPORT_SYMBOL_GPL(soft_pe_set_private);

void * soft_pe_get_private(struct soft_pe *s_pe)
{
	debug_printf("%s entry\n",__func__);
	return s_pe->private;
}
EXPORT_SYMBOL_GPL(soft_pe_get_private);

int soft_pe_2_0_try_8v(struct soft_pe *s_pe,int times)
{
	int i,result=0;
	for(i=0;i<times;i+=1){
		int voltage;
		msleep(500);//wait for adapter
		voltage=soft_pe_get_input_voltage(s_pe);
		if(voltage>7000&&voltage<9000){
			result=1;
			break;
		}
		soft_pe_2_0_send_pattern(s_pe,PE_2_0_PATTERN_8V);
	}
	return result;
}
int soft_pe_2_0_try_5v(struct soft_pe *s_pe,int times)
{
	int i,result=0;
	for(i=0;i<times;i+=1){
		int voltage;
		msleep(500);//wait for adapter
		voltage=soft_pe_get_input_voltage(s_pe);
		if(voltage>4000&&voltage<6000){
			result=1;
			break;
		}
		soft_pe_2_0_send_pattern(s_pe,PE_2_0_PATTERN_WDT);
	}
	return result;	
}
int soft_pe_1_0_try_9v(struct soft_pe *s_pe,int times)
{
	int i,result=0;
	for(i=0;i<times;i+=1){
		int voltage;
		msleep(500);//wait for adapter
		voltage=soft_pe_get_input_voltage(s_pe);
		if(voltage<=8000)
			soft_pe_1_0_send_pattern(s_pe,PE_1_0_PATTERN_INCREASE);
		else if(voltage>=10000)
			soft_pe_1_0_send_pattern(s_pe,PE_1_0_PATTERN_DECREASE);
		else{
			result=1;
			break;
		}				
	}
	return result;
}
int soft_pe_1_0_try_5v(struct soft_pe *s_pe,int times)
{
	int i,result=0;
	for(i=0;i<times;i+=1){
		int voltage;
		msleep(500);//wait for adapter
		voltage=soft_pe_get_input_voltage(s_pe);
		if(voltage>4000&&voltage<6000){
			result=1;
			break;
		}
		soft_pe_1_0_send_pattern(s_pe,PE_1_0_PATTERN_WDT);
	}
	return result;	
}

void soft_pe_handle_event_when_reset(struct soft_pe *s_pe,enum pe_event event)
{
	s_pe->state=PE_STATE_BUSY;
	if(event==PE_EVENT_START_PUMP_UP){
		/*try pe 2.0 first*/
		if(soft_pe_2_0_try_8v(s_pe,5))
			s_pe->state=PE_STATE_PE_2_0;
		else{
			/*try pe 1.0 then*/
			if(soft_pe_1_0_try_9v(s_pe,6))
				s_pe->state=PE_STATE_PE_1_0;
			else
				s_pe->state=PE_STATE_NOT_PE;
		}
	}
}
void soft_pe_handle_event_when_busy(struct soft_pe *s_pe,enum pe_event event)
{
	//never run to here
}
void soft_pe_handle_event_when_pe_2_0(struct soft_pe *s_pe,enum pe_event event)
{
	switch(event){
		case PE_EVENT_START_PUMP_UP:
			s_pe->state=PE_STATE_BUSY;
			soft_pe_2_0_try_8v(s_pe,5);
			s_pe->state=PE_STATE_PE_2_0;
		break;
		case PE_EVENT_START_PUMP_DOWN:
			s_pe->state=PE_STATE_BUSY;
			soft_pe_2_0_try_5v(s_pe,3);
			s_pe->state=PE_STATE_PE_2_0;
		break;
		default:
		break;
	}
}
void soft_pe_handle_event_when_pe_1_0(struct soft_pe *s_pe,enum pe_event event)
{
	switch(event){
		case PE_EVENT_START_PUMP_UP:
			s_pe->state=PE_STATE_BUSY;
			soft_pe_1_0_try_9v(s_pe,6);
			s_pe->state=PE_STATE_PE_1_0;
		break;
		case PE_EVENT_START_PUMP_DOWN:
			s_pe->state=PE_STATE_BUSY;
			soft_pe_1_0_try_5v(s_pe,3);
			s_pe->state=PE_STATE_PE_1_0;
		break;
		default:
		break;
	}
}
void soft_pe_handle_event_when_not_pe(struct soft_pe *s_pe,enum pe_event event)
{
	//don't care any event
}
int soft_pe_thread(void *data)
{
	struct soft_pe *s_pe=data;
	while(1){
		enum pe_event event;
		debug_printf("wait for event...\n");
		wait_event(s_pe->wait_queue,s_pe->event!=PE_EVENT_NONE);
		event=s_pe->event;
		s_pe->event=PE_EVENT_NONE;
		debug_printf("event received:%d !\n",event);
		/*handle event*/
		debug_printf("state:%d\n",s_pe->state);
		switch(s_pe->state){
			case PE_STATE_RESET:
				soft_pe_handle_event_when_reset(s_pe,event);
			break;
			case PE_STATE_BUSY:
				soft_pe_handle_event_when_busy(s_pe,event);
			break;
			case PE_STATE_PE_2_0:
				soft_pe_handle_event_when_pe_2_0(s_pe,event);
			break;
			case PE_STATE_PE_1_0:
				soft_pe_handle_event_when_pe_1_0(s_pe,event);
			break;
			case PE_STATE_NOT_PE:
				soft_pe_handle_event_when_not_pe(s_pe,event);
			break;
		}
	}
#if 0//for test
	while(1){
		soft_pe_begin(s_pe);
		debug_printf("vbus=%d", soft_pe_get_input_voltage(s_pe));
		soft_pe_1_0_send_pattern(s_pe,PE_1_0_PATTERN_INCREASE);//soft_pe_2_0_send_pattern(s_pe,PE_2_0_PATTERN_8V);
		soft_pe_end(s_pe);
		debug_printf("vbus=%d", soft_pe_get_input_voltage(s_pe));
		msleep(3000);
	}
#endif
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
	//init waitqueue
	init_waitqueue_head(&s_pe->wait_queue);
	//init other members
	s_pe->state=PE_STATE_RESET;
	s_pe->event=PE_EVENT_NONE;
	//create thread(must be created behind)
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
