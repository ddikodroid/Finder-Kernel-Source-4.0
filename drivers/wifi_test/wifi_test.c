/*
 * DHD Protocol Module for CDC and BDC.
 *
 * Copyright (C) 1999-2011, Broadcom Corporation
 * 
 *         Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 * 
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 * 
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 * $Id: dhd_cdc.c,v 1.51.6.28.4.1 2011/02/01 19:36:23 Exp $
 *
 * BDC is like CDC, except it includes a header for data packets to convey
 * packet priority over the bus, and flags (e.g. to indicate checksum status
 * for dongle offload.)
 */
#include <linux/proc_fs.h>
#include <linux/gpio.h>
#include <linux/seq_file.h>
#include <linux/delay.h>

#define WIFI_FTMTEST_PIN0    98
#define WIFI_FTMTEST_PIN1    99

#ifdef CONFIG_OPPO_MODIFY 
 static int get_wftm_gpio(void)
 {
	 int gpio98_value = 0 ;
	 int gpio99_value = 0 ;
	 
	 gpio_tlmm_config(GPIO_CFG(WIFI_FTMTEST_PIN0, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_12MA), 0);
	 msleep(10);
	 gpio_tlmm_config(GPIO_CFG(WIFI_FTMTEST_PIN1, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_12MA), 0);
	 msleep(10);
	 
	 gpio98_value = gpio_get_value(WIFI_FTMTEST_PIN0);
	 msleep(10);
	 
	 gpio99_value = gpio_get_value(WIFI_FTMTEST_PIN1);
	 
	 if ((gpio98_value == 0)&&(gpio99_value == 1))//into ftm 
	 	return 1 ;
	 else
	 	return 0 ;
	 
 }
 static int wftm_proc_show(struct seq_file *m, void *v)
 {
	 seq_printf(m, "%d\n",get_wftm_gpio());
	 
	 return 0;
 }
 
 static int wftm_proc_open(struct inode *inode, struct file *file)
 {
 
	 return single_open(file, wftm_proc_show, NULL);
 }
 
 static const struct file_operations wftm_proc_fops = {
	 .open		 = wftm_proc_open,
	 .read		 = seq_read,
	 .llseek	 = seq_lseek,
	 .release	 = single_release,
 };
 
 static int __init proc_wftm_init(void)
 {
	 printk(KERN_INFO"liuhd-proc_wftm_init--\n");
 
	 proc_create("WftmEnable", 0, NULL, &wftm_proc_fops);
	 return 0;
 }
 module_init(proc_wftm_init);
#endif //[End add by liuhd]

