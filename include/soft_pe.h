#include <linux/printk.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/slab.h>

enum pe_2_0_pattern{
	PE_2_0_PATTERN_8V=0,
	PE_2_0_PATTERN_WDT,
	PE_2_0_PATTERN_SIZE
};
enum pe_state{
	PE_STATE_RESET,
	PE_STATE_BUSY,
	PE_STATE_PE_2_0,
	PE_STATE_PE_1_0,
	PE_STATE_NOT_PE
};

struct soft_pe;
struct soft_pe_driver{
	void (*set_input_current)(struct soft_pe *,long int);
	long int (*get_input_voltage)(struct soft_pe *);
	void (*pe_begin)(struct soft_pe *);
	void (*pe_end)(struct soft_pe *);
};

struct soft_pe{
	struct soft_pe_driver *driver;
	struct task_struct *thread;
	enum pe_state state;
};


struct soft_pe * soft_pe_register(struct soft_pe_driver *driver);
void soft_pe_unregister(struct soft_pe *s_pe);

void soft_pe_set_input_current(struct soft_pe *s_pe,long int current_ma);
long int soft_pe_get_input_voltage(struct soft_pe *s_pe);

/*pe 2.0*/
void soft_pe_2_0_send_pattern(struct soft_pe *s_pe,enum pe_2_0_pattern  pattern);


/*public interface*/
enum pe_state soft_pe_get_state(struct soft_pe *s_pe);
void soft_pe_start_pump_up(struct soft_pe *s_pe);
void soft_pe_start_pump_down(struct soft_pe *s_pe);//fall down but remember the pe state
void soft_pe_reset(struct soft_pe *s_pe);//reset all the state


