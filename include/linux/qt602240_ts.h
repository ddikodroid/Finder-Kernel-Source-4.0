/*
 * AT42QT602240/ATMXT224 Touchscreen driver
 *
 * Copyright (C) 2010 Samsung Electronics Co.Ltd
 * Author: Joonyoung Shim <jy0922.shim@samsung.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#ifndef __LINUX_QT602240_TS_H
#define __LINUX_QT602240_TS_H

/* Orient */
#define QT602240_NORMAL			0x0
#define QT602240_DIAGONAL		0x1
#define QT602240_HORIZONTAL_FLIP	0x2
#define QT602240_ROTATED_90_COUNTER	0x3
#define QT602240_VERTICAL_FLIP		0x4
#define QT602240_ROTATED_90		0x5
#define QT602240_ROTATED_180		0x6
#define QT602240_DIAGONAL_COUNTER	0x7

#define TS_CMD_RESET_6S  			1
#define TS_CMD_INIT 				2
#define TS_CMD_RESUME_80MS 	3

#define MXT_CFG_T42_CTRL  0x00

#define ATMEL_IOCTL_MAGIC 'a'

#define ATMEL_GET_VERSION  				_IOR(ATMEL_IOCTL_MAGIC, 0, unsigned)
#define ATMEL_GET_FW_VERSION   			_IOR(ATMEL_IOCTL_MAGIC, 1, unsigned)
#define ATMEL_PROGRAM_START        		_IOW(ATMEL_IOCTL_MAGIC, 2, unsigned)
#define ATMEL_GET_PROGRAM_STATE  		_IOR(ATMEL_IOCTL_MAGIC, 3, unsigned)

#define SYNA_IOCTL_MAGIC 's'
#define SYNA_TEST_TP_REFERENCE  		_IOR(SYNA_IOCTL_MAGIC, 4, unsigned)


// original, stable, newest
#define ATMEL_FW_0_NAME 	"PR1007723-tm2034-001.img"   	//Product  Version 100  max_x 1052, max_y 1744
#define ATMEL_FW_1_NAME 	"PR1043284-tm2034-001.img"	//Product  Version 166  max_x 1052, max_y 1883
#define ATMEL_FW_2_NAME	"PR1067310-tm2034-001.img"	//Product  Version 102  max_x 1052, max_y 1883

#define TOUCH_SCREEN_VERION_0 	0
#define TOUCH_SCREEN_VERION_1		1
#define TOUCH_SCREEN_VERION_2		2


/* Debug levels */
#define DEBUG_INFO_ERROR     1
#define DEBUG_INFO_TS     2
#define DEBUG_INFO_TEST     2
#define DEBUG_TRACE     3

#define USB_QUIT_NOISE_SUPPRESS 0
#define USB_NOISE_SUPPRESS 1
#define	WIRELESS_NOISE_SUPPRESS 4
#define	DUALCHG_NOISE_SUPPRESS 5

#define HDMI_QUIT_NOISE_SUPPRESS 2
#define HDMI_NOISE_SUPPRESS 3

enum{
	SimuUnlock_disable = 0,
	SimuUnlock_enable,
	SimuUnlock_pressed,
};


#define ATMEL_QT602240E_NAME "qt602240_ts"

/* The platform data for the AT42QT602240/ATMXT224 touchscreen driver */
struct qt602240_platform_data {
	unsigned int x_line;  //17
	unsigned int y_line;  //12
	unsigned int x_size;  //x resolution
	unsigned int y_size;  //y resolution
	unsigned int blen;    //48 or 0x30
	unsigned int threshold; //80 0r 0x50
	unsigned int voltage;   //30000000
	unsigned char orient;	//0
	int (*power)(int on);	/* Only valid in first array entry */
};

#ifdef CONFIG_TOUCHSCREEN_ATMEL_QT602240E
 int atmel_noise_supress_cfg(int Noise_supress);
#else
/*
 int atmel_noise_supress_cfg(int Noise_supress)
{
	return 0;
}
*/
#endif


#endif /* __LINUX_QT602240_TS_H */
