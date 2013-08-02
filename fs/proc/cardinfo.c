/*OPPO 2012-04-26 add by wjp */
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/kernel_stat.h>


extern int get_sim_detect_gpio_value(void);
static int cardinfo_proc_show(struct seq_file *m, void *v)
{
	int value;
	value = get_sim_detect_gpio_value();
	seq_printf(m, "cardinfo:%d\n",value);
	return 0;
}


static int cardinfo_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cardinfo_proc_show, NULL);
}

static const struct file_operations cardinfo_proc_fops = {
	.open		= cardinfo_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_cardinfo_init(void)
{
	proc_create("cardinfo", 0, NULL, &cardinfo_proc_fops);
	return 0;
}
module_init(proc_cardinfo_init);
/*OPPO 2012-04-26 end by wjp */

