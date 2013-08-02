/*
 * include/linux/synaptics_i2c_rmi.h - platform data structure for f75375s sensor
 *
 * Copyright (C) 2008 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LINUX_SYNAPTICS_I2C_RMI_H

#include <linux/ioctl.h>

#define _LINUX_SYNAPTICS_I2C_RMI_H

#define SYNA_IOCTL_MAGIC 's'

#define SYNA_GET_VERSION  				_IOR(SYNA_IOCTL_MAGIC, 0, unsigned)
#define SYNA_GET_FW_VERSION   			_IOR(SYNA_IOCTL_MAGIC, 1, unsigned)
#define SYNA_PROGRAM_START        		_IOW(SYNA_IOCTL_MAGIC, 2, unsigned)
#define SYNA_GET_PROGRAM_STATE  		_IOR(SYNA_IOCTL_MAGIC, 3, unsigned)
#define SYNAPTICS_I2C_RMI_NAME "syna_touch"

// original, stable, newest
#define SYNA_FW_0_NAME 	"PR1007723-tm2034-001.img"   	//Product  Version 100  max_x 1052, max_y 1744
#define SYNA_FW_1_NAME 	"PR1043284-tm2034-001.img"	//Product  Version 166  max_x 1052, max_y 1883
#define SYNA_FW_2_NAME	"PR1067310-tm2034-001.img"	//Product  Version 102  max_x 1052, max_y 1883
//testing firmware
#define SYNA_FW_4_NAME	"PR1042137-tm2034-001.img"	
#define SYNA_FW_5_NAME	"PR1050564-tm2034-001.img"	
#define SYNA_FW_6_NAME	"PR1050570-tm2034-001.img"	

#define TOUCH_SCREEN_VERION_0 	0
#define TOUCH_SCREEN_VERION_1		1
#define TOUCH_SCREEN_VERION_2		2

#define SYNA_TS_CMD_RESET  			1
#define SYNA_TS_CMD_INIT 				2
#define SYNA_TS_CMD_RESUME_200MS 	3

#define SYNA_PROGRAM_STATE_STARTED  1   
#define SYNA_PROGRAM_STATE_WRITING  2   
#define SYNA_PROGRAM_STATE_SUCCEED  3   
#define SYNA_PROGRAM_STATE_FAILED   4	

#define PAGE_SELECT				0xff

#define FINGER_Package		5

#define MAX_FINGERS		10

#define TS_PEN_IRQ_GPIO 61
enum {
	SYNAPTICS_FLIP_X = 1UL << 0,
	SYNAPTICS_FLIP_Y = 1UL << 1,
	SYNAPTICS_SWAP_XY = 1UL << 2,
	SYNAPTICS_SNAP_TO_INACTIVE_EDGE = 1UL << 3,
};
#define DEBUG_INFO_ERROR     1
#define DEBUG_INFO_TS     2
#define DEBUG_TRACE     3

struct synaptics_i2c_rmi_platform_data {
	uint32_t version;	/* Use this entry for panels with */
				/* (major << 8 | minor) version or above. */
				/* If non-zero another array entry follows */
	int (*power)(int on);	/* Only valid in first array entry */
	uint32_t flags;
	unsigned long irqflags;
	uint32_t inactive_left; /* 0x10000 = screen width */
	uint32_t inactive_right; /* 0x10000 = screen width */
	uint32_t inactive_top; /* 0x10000 = screen height */
	uint32_t inactive_bottom; /* 0x10000 = screen height */
	uint32_t snap_left_on; /* 0x10000 = screen width */
	uint32_t snap_left_off; /* 0x10000 = screen width */
	uint32_t snap_right_on; /* 0x10000 = screen width */
	uint32_t snap_right_off; /* 0x10000 = screen width */
	uint32_t snap_top_on; /* 0x10000 = screen height */
	uint32_t snap_top_off; /* 0x10000 = screen height */
	uint32_t snap_bottom_on; /* 0x10000 = screen height */
	uint32_t snap_bottom_off; /* 0x10000 = screen height */
	uint32_t fuzz_x; /* 0x10000 = screen width */
	uint32_t fuzz_y; /* 0x10000 = screen height */
	int fuzz_p;
	int fuzz_w;
	int8_t sensitivity_adjust;
};

#endif /* _LINUX_SYNAPTICS_I2C_RMI_H */
