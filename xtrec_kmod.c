#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <asm/desc.h>
#include <cpuid.h>

#include "xtrec_dev.h"
#include "asm.h"
#include "idt.h"

#define SUCCESS 0
#define BUF_LEN 80

#define IA32_DEBUGCTL_MSR 0x01D9

const uint32_t CPUID_FLAG_MSR = 1 << 5;

void cpuGetMSR(uint32_t msr, uint32_t *a)
{
	asm volatile ("rdmsr" : "=A" (*a) : "c" (msr)); 
}
void cpuSetMSR(uint32_t msr, uint32_t lo, uint32_t hi)
{
	asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}


/* --------------------------------------------------------------------------*/
static int Device_Open = 0;
static char Message[BUF_LEN];
static char *Message_ptr;

static int
device_open(struct inode *inode, struct file *file)
{
	printk (KERN_EMERG "device_open() called\n");
	if (Device_Open)
		return -EBUSY;
	Device_Open++;
	Message_ptr = Message;
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

static int
device_release(struct inode *inode, struct file *file)
{
	printk (KERN_EMERG "device_release() called\n");
	Device_Open--;
	module_put(THIS_MODULE);
	return SUCCESS;
}

static int
device_ioctl (struct file *file, unsigned int ioctl_num,
								  unsigned long long ioctl_param)
{
	printk (KERN_EMERG "device_ioctl() called\n");
	return 0;
}
struct file_operations Fops = {
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release,
};

/* --------------------------------------------------------------------------*/

void blockStep_handler(int p, void *q, struct pt_regs *regs)
{
	printk (KERN_EMERG "Got a block Step interrupt!\n");
}

static int __init init_mod(void)
{
	uint32_t val, a, b, c, d = 0;
	int ret;
	struct task_struct *task = current;
	printk(KERN_EMERG "Initializing XTREC module...\n");

	__get_cpuid(1, &a, &b, &c, &d);
	printk(KERN_EMERG "MISC_ENABLE? %u\n", d);
	cpuGetMSR(IA32_DEBUGCTL_MSR, &val);
	printk(KERN_EMERG "MSR Value: %u\n", val);
	cpuSetMSR(IA32_DEBUGCTL_MSR, 2, 0);
	cpuGetMSR(IA32_DEBUGCTL_MSR, &val);
	printk(KERN_EMERG "MSR Value: %u\n", val);
	printk(KERN_EMERG "MSR Value: %u\n", val); if ((ret = register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops)) < 0) {
		printk (KERN_EMERG "Could not register device\n");
		return ret;
	}

	/* Install IDT entry */
	void *idt_table = idt_base();
	printk(KERN_EMERG "IDT base: %p\n", idt_table);
	printk(KERN_EMERG "IDT addr: %x\n", *(unsigned int*)idt_table);
	install_interrupt_gate(1,blockStep_handler,IDT_KERN_DPL);
	
	
	
	printk(KERN_EMERG "Current process: %d\n", task->pid);
	printk(KERN_EMERG "Done initialinzing\n");
	return 0;
}

static void __exit cleanup_mod(void)
{
	printk(KERN_EMERG "Cleaning up module...\n");

	free_irq(1, NULL);
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
	printk(KERN_EMERG "Module Exited\n");
}

module_init(init_mod);
module_exit(cleanup_mod);
