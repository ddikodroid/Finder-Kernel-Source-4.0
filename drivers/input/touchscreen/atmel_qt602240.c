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
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/i2c.h>
#include <linux/qt602240_ts.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/gpio.h>


#include <linux/proc_fs.h>
#include <linux/firmware.h>
#include <linux/miscdevice.h>
#include <linux/earlysuspend.h>
#include <asm/uaccess.h>
#include <linux/leds-pmic8058.h>

#ifdef ATMEL_TS_AUTO_RELEASE_POINTER
#include <linux/hrtimer.h>
#endif


#if 0
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#endif


/* Version */
#define QT602240_VER_16			16
#define QT602240_VER_20			20
#define QT602240_VER_21			21
#define QT602240_VER_22			22

/* Slave addresses */
#define QT602240_APP_LOW		0x4a
#define QT602240_APP_HIGH		0x4b
#define QT602240_BOOT_LOW		0x24
#define QT602240_BOOT_HIGH		0x25

/* Firmware */
#define QT602240_FW_NAME		"qt602240.fw"

/* Registers */
#define QT602240_FAMILY_ID		0x00
#define QT602240_VARIANT_ID		0x01
#define QT602240_VERSION		0x02
#define QT602240_BUILD			0x03
#define QT602240_MATRIX_X_SIZE		0x04
#define QT602240_MATRIX_Y_SIZE		0x05
#define QT602240_OBJECT_NUM		0x06
#define QT602240_OBJECT_START		0x07

#define QT602240_OBJECT_SIZE		6

/* Object types */
#define QT602240_DEBUG_DIAGNOSTIC	37
#define QT602240_GEN_MESSAGE		5
#define QT602240_GEN_COMMAND		6
#define QT602240_GEN_POWER		    7
#define QT602240_GEN_ACQUIRE		8

#define QT602240_TOUCH_MULTI		9
#define QT602240_TOUCH_KEYARRAY		15
#define QT602240_TOUCH_PROXIMITY	23

#define QT602240_PROCI_GRIPSUPPRESSION  40
#define QT602240_PROCI_TOUCHSUPPRESSION 42 
#define QT602240_PROCI_STYLUS           47
#define QT602240_PROCG_NOISESUPPRESSION 48

#define QT602240_SPT_COMMSCONFIG	18	/* firmware ver 21 over */
#define QT602240_SPT_GPIOPWM		19
#define QT602240_SPT_SELFTEST		25
#define QT602240_SPT_USERDATA		38	/* firmware ver 21 over */
#define QT602240_SPT_MESSAGECOUNT		44
#define QT602240_SPT_CTECONFIG	    	46
/*The following objects do not exsist in mXT E version*/
/*#define QT602240_PROCI_GRIPFACE		20*/
//#define QT602240_PROCI_GRIPFACE		20
//#define QT602240_PROCG_NOISE			22
//#define QT602240_PROCI_ONETOUCH		24
//#define QT602240_PROCI_TWOTOUCH		27


/* QT602240_GEN_COMMAND field */
#define QT602240_COMMAND_RESET		0
#define QT602240_COMMAND_BACKUPNV	1
#define QT602240_COMMAND_CALIBRATE	2
#define QT602240_COMMAND_REPORTALL	3
#define QT602240_COMMAND_DIAGNOSTIC	5

/* QT602240_GEN_POWER field */
#define QT602240_POWER_IDLEACQINT	0
#define QT602240_POWER_ACTVACQINT	1
#define QT602240_POWER_ACTV2IDLETO	2

/* QT602240_GEN_ACQUIRE field */
#define QT602240_ACQUIRE_CHRGTIME	0
#define QT602240_ACQUIRE_TCHDRIFT	2
#define QT602240_ACQUIRE_DRIFTST	3
#define QT602240_ACQUIRE_TCHAUTOCAL	4
#define QT602240_ACQUIRE_SYNC		5
#define QT602240_ACQUIRE_ATCHCALST	6
#define QT602240_ACQUIRE_ATCHCALSTHR	7
#define	QT602240_ACQUIRE_ATCHFRCCALTHR  8
#define	QT602240_ACQUIRE_ATCHFRCCALRATIO 9

/* QT602240_TOUCH_MULTI field */
#define QT602240_TOUCH_MULTI_T9_OFFSET 21
#define QT602240_TOUCH_CTRL		0
#define QT602240_TOUCH_XORIGIN		1
#define QT602240_TOUCH_YORIGIN		2
#define QT602240_TOUCH_XSIZE		3
#define QT602240_TOUCH_YSIZE		4
#define QT602240_TOUCH_AKSCFG		5
#define QT602240_TOUCH_BLEN			6
#define QT602240_TOUCH_TCHTHR		7
#define QT602240_TOUCH_TCHDI		8
#define QT602240_TOUCH_ORIENT		9
#define QT602240_TOUCH_MGRTIMEOUT   10
#define QT602240_TOUCH_MOVHYSTI		11	
#define QT602240_TOUCH_MOVHYSTN		12
#define QT602240_TOUCH_MOVFILTER    13
#define QT602240_TOUCH_NUMTOUCH		14
#define QT602240_TOUCH_MRGHYST		15
#define QT602240_TOUCH_MRGTHR		16
#define QT602240_TOUCH_AMPHYST		17
#define QT602240_TOUCH_XRANGE_LSB	18
#define QT602240_TOUCH_XRANGE_MSB	19
#define QT602240_TOUCH_YRANGE_LSB	20
#define QT602240_TOUCH_YRANGE_MSB	21
#define QT602240_TOUCH_XLOCLIP		22
#define QT602240_TOUCH_XHICLIP		23
#define QT602240_TOUCH_YLOCLIP		24
#define QT602240_TOUCH_YHICLIP		25
#define QT602240_TOUCH_XEDGECTRL	26
#define QT602240_TOUCH_XEDGEDIST	27
#define QT602240_TOUCH_YEDGECTRL	28
#define QT602240_TOUCH_YEDGEDIST	29
#define QT602240_TOUCH_JUMPLIMIT	30	/* firmware ver 22 over */
/*The following regs are for mXT E version*/
#define	QT602240_TOUCH_TCHHYST		31
#define	QT602240_TOUCH_XPITCH		32
#define	QT602240_TOUCH_YPITCH		33
#define	QT602240_TOUCH_NEXTTCHDI	34

/* QT602240_TOUCH_KEYARRAY field */
#define QT602240_TOUCH_CTRL		0
#define QT602240_TOUCH_XORIGIN		1
#define QT602240_TOUCH_YORIGIN		2
#define QT602240_TOUCH_XSIZE		3
#define QT602240_TOUCH_YSIZE		4
#define QT602240_TOUCH_AKSCFG		5
#define QT602240_TOUCH_BLEN			6
#define QT602240_TOUCH_TCHTHR		7
#define QT602240_TOUCH_TCHDI		8

/* QT602240_PROCI_GRIP field */
#define QT602240_GRIP_CTRL		0
#define QT602240_GRIP_XLOGRIP	1
#define QT602240_GRIP_XHIGRIP	2
#define QT602240_GRIP_YLOGRIP	3
#define QT602240_GRIP_YHIGRIP	4
/* QT602240_PROCI_TOUCH SUPPRESSION field */
#define QT602240_TOUCH_CTRL		0
#define QT602240_TOUCH_APPRTHR	1
#define QT602240_TOUCH_MAXAPPRAREA 2
#define QT602240_TOUCH_MAXTCHAREA  3
#define QT602240_TOUCH_SUPSTRENGTH 4
#define QT602240_TOUCH_SUPEXTTO	   5
#define QT602240_TOUCH_MAXNUMTCHS  6
#define QT602240_TOUCH_SHAPESTRENGTH 7

/*QT602240_PROCI_STYLUS field*/
#define QT602240_PROC_STYLUS_CTRL 0
#define QT602240_PROC_STYLUS_CONTMIN 1
#define QT602240_PROC_STYLUS_CONTMAX 2
#define QT602240_PROC_STYLUS_STABILITY 3
#define QT602240_PROC_STYLUS_MAXTCHAREA 4
#define QT602240_PROC_STYLUS_AMPLTHR 5
#define QT602240_PROC_STYLUS_STYSHAPE 6
#define QT602240_PROC_STYLUS_HOVERSUP 7
#define QT602240_PROC_STYLUS_CONFTHR 8
#define QT602240_PROC_STYLUS_SYNCSPERX 9

/* QT602240_PROCI_NOISE field 
#define QT602240_NOISE_CTRL		0
#define QT602240_NOISE_OUTFLEN		1
#define QT602240_NOISE_GCAFUL_LSB	3
#define QT602240_NOISE_GCAFUL_MSB	4
#define QT602240_NOISE_GCAFLL_LSB	5
#define QT602240_NOISE_GCAFLL_MSB	6
#define QT602240_NOISE_ACTVGCAFVALID	7
#define QT602240_NOISE_NOISETHR		8
#define QT602240_NOISE_FREQHOPSCALE	10
#define QT602240_NOISE_FREQ0		11
#define QT602240_NOISE_FREQ1		12
#define QT602240_NOISE_FREQ2		13
#define QT602240_NOISE_FREQ3		14
#define QT602240_NOISE_FREQ4		15
#define QT602240_NOISE_IDLEGCAFVALID	16*/

/* QT602240_PROCI_NOISE field FOR E version*/
#define QT602240_NOISE_CTRL		0
#define QT602240_NOISE_CFG		1
#define QT602240_NOISE_CALCFG	2
#define QT602240_NOISE_BASEFREQ	3

#define QT602240_NOISE_MFFREQLSB		8
#define QT602240_NOISE_MFFREQMSB		9

#define QT602240_NOISE_GCACTVINVLDADCS		13
#define QT602240_NOISE_GCIDLEINVLDADCS		14

#define QT602240_NOISE_GCMAXADCSPERX        17
#define QT602240_NOISE_GCLIMITMIN			18
#define QT602240_NOISE_GCLIMITMAX			19
#define QT602240_NOISE_GCCOUNTMINTGTLSB		20
#define QT602240_NOISE_GCCOUNTMINTGTMSB		21
#define QT602240_NOISE_MFINVLDDIFFTHR		22
#define QT602240_NOISE_MFINCADCSPXTHRLSB	23
#define QT602240_NOISE_MFINCADCSPXTHRMSB	24
#define QT602240_NOISE_MFERRORTHRLSB		25
#define QT602240_NOISE_MFERRORTHRMSB		26
#define QT602240_NOISE_SELFREQMAX			27


/* QT602240_SPT_COMMSCONFIG */
#define QT602240_COMMS_CTRL		0
#define QT602240_COMMS_CMD		1

/* QT602240_SPT_CTECONFIG field *//* E Version */
#define QT602240_CTE_CTRL		0
#define QT602240_CTE_MODE		1
#define QT602240_CTE_IDLEGCAFDEPTH	2
#define QT602240_CTE_ACTVGCAFDEPTH	3
#define QT602240_CTE_ADCSPERSYNC    4
#define QT602240_CTE_VOLTAGE    4

#define QT602240_CTE_PULSESPERADC	5	
#define QT602240_CTE_XSLEW			6
#define QT602240_CTE_SYNCDELAYLSB	7
#define QT602240_CTE_SYNCDELAYMSB	8


#define QT602240_VOLTAGE_DEFAULT	2700000
#define QT602240_VOLTAGE_STEP		10000

/* Define for QT602240_GEN_COMMAND */
#define QT602240_BOOT_VALUE		0xa5
#define QT602240_BACKUP_VALUE		0x55
#define QT602240_BACKUP_TIME		25	/* msec */
#define QT602240_RESET_TIME		65	/* msec */

#define QT602240_FWRESET_TIME		175	/* msec */

/* Command to unlock bootloader */
#define QT602240_UNLOCK_CMD_MSB		0xaa
#define QT602240_UNLOCK_CMD_LSB		0xdc

/* Bootloader mode status */
#define QT602240_WAITING_BOOTLOAD_CMD	0xc0	/* valid 7 6 bit only */
#define QT602240_WAITING_FRAME_DATA	0x80	/* valid 7 6 bit only */
#define QT602240_FRAME_CRC_CHECK	0x02
#define QT602240_FRAME_CRC_FAIL		0x03
#define QT602240_FRAME_CRC_PASS		0x04
#define QT602240_APP_CRC_FAIL		0x40	/* valid 7 8 bit only */
#define QT602240_BOOT_STATUS_MASK	0x3f

/* Touch status */
#define QT602240_SUPPRESS		(1 << 1)
#define QT602240_AMP			(1 << 2)
#define QT602240_VECTOR			(1 << 3)
#define QT602240_MOVE			(1 << 4)
#define QT602240_RELEASE		(1 << 5)
#define QT602240_PRESS			(1 << 6)
#define QT602240_DETECT			(1 << 7)

#define MAX_FINGERS		10
#define TS_PEN_IRQ_GPIO 61

#define TS_ATMEL_224E  1
#define TS_SYNA_1734   2
#define TS_ATMEL_224  3

//#define ATMEL_224E_SLEEP
//#define RESOLUTION_12_BIT
#define ATMEL_TS_AUTO_RELEASE_POINTER

#ifdef ATMEL_TS_AUTO_RELEASE_POINTER
#define RELEASE_POINTER_MS 2000
#endif

#define MPP11_VALID_GPIO 223 

static int bChangeUpDn= 0;
//static int is_tp_suspended = 0;
static int logo_level  = 2;
static int SimulateUnlock = SimuUnlock_disable;
static int SimulateUnlock_origin_x = 0;
static int Atmel_Program_state = 0;
int Disable_key_during_touch = 0;


#ifdef CONFIG_HAS_EARLYSUSPEND
static void Atmel_early_suspend(struct early_suspend *h);
static void Atmel_late_resume(struct early_suspend *h);
#endif


extern int Check_Touchscreen_type(int type);

/*virtual key support */
static struct kobject *Atmel_properties_kobj;


#define print_ts(level, ...) \
	do { \
		if (logo_level  >= (level)) \
			printk(__VA_ARGS__); \
	} while (0) 


#if 1
static const u8 init_vals_ver_16[] = {
    /*[SPT_USERDATA_T38 INSTANCE 0] 264*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    
    /*[GEN_POWERCONFIG_T7 INSTANCE 0] 272*/
    0x40, //idle Acquisition interval
    0xff, //active Acquisition interval
    0x0a, //avtive to idle time out
    
    /*[GEN_ACQUISITIONCONFIG_T8 INSTANCE 0]275*/
    0x14, //charge time
    0x00, 0x14, 0x14, 
    0x00, //Touch auto Calibration
    0x00, 
    0x05, 0x37, 0x0a, 0xc8, //Antitouch Cal
    
    /*[TOUCH_MULTITOUCHSCREEN_T9 INSTANCE 0]285*/
    0x8f, 0x00, 0x00, 0x13, 0x0b, 0x00, 0x21, 0x2d, 0x03, 0x05, 
    #ifdef RESOLUTION_12_BIT
    0x05, 
    0x0a, //MOVE hyst init
    0x0a, //MOVE hyst next
    0x02, //Adap threahold    4位符号数
    0x04, 0x10, 0x10, 0x0a, 
    0xff, 0x0f, //X size
    0xff, 0x0f, //y size
    0xff, 0x00, 0x02, 0xff, // CLIP
    152, //X edge ctrl  SPAN/DISCLOK/correction gradient@6
    50, //X edge correction distance
    209, //Y edge ctrl  SPAN/DISCLOK/correction gradient@6
    95, //Y edge correction distance
    #else
    0x05, 
    0x08, //MOVE hyst init
    0x08, //MOVE hyst next
    0x00, //Adap threahold    4位符号数
    0x04, 0x10, 0x10, 0x0a, 
    0x47, 0x03, //X size
    0xdf, 0x01, //y size
    0xff, 0x00, 0x02, 0xff, // CLIP
    152, //X edge ctrl  SPAN/DISCLOK/correction gradient@6
    50, //X edge correction distance
    209, //Y edge ctrl  SPAN/DISCLOK/correction gradient@6
    95, //Y edge correction distance
    #endif
    0x32, 0x14, 0x00, 0x00, 0x00,
    
    /*[TOUCH_KEYARRAY_T15 INSTANCE 0]320*/
    0x00, 0x0f, 0x0c, 0x02, 0x01, 0x00, 0x10, 0x32, 0x04, 0x00, 
	0x00,
    /*[SPT_COMMSCONFIG_T18 INSTANCE 0]331*/
    0x00, 0x00,
    /*[SPT_GPIOPWM_T19 INSTANCE 0]333*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*[TOUCH_PROXIMITY_T23 INSTANCE 0]349*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00,
    /*[SPT_SELFTEST_T25 INSTANCE 0]364*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00,
    /*[PROCI_GRIPSUPPRESSION_T40 INSTANCE 0]378*/
    0x00, 0x00, 0x00, 0x00, 0x00,
    /*[PROCI_TOUCHSUPPRESSION_T42 INSTANCE 0]383*/
    0x00, 0x32, 0x66, 0x38, 0x00, 0x00, 0x09, 0x00,
    
    /*[SPT_CTECONFIG_T46 INSTANCE 0]391*/
    0x00, 
    0x03, 
    0x10,  // Idle sync groups per X
    0x24, // Avtive sync groups per X
    0x00, //ADC per Sync group
    0x00, //Pulses per Adc
    0x01, //x pulse slew rate
    0x00, 
    0x00,
    /*[PROCI_STYLUS_T47 INSTANCE 0]400*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    
    /*[PROCG_NOISESUPPRESSION_T48 INSTANCE 0]410*/
    0x01,  //CTRL  -/-/-/-                       /RPTAPX/RPTFREQ/RPTEN/ENABLE
    0x80, //CFG    Drift EN/-/-/-              /CHGIN/GCMODE/GCMODE/GCMODE
    0xc0, //CALCFG INCRST/INCBIAS/CHARGON/DUALXEN           /MEANEN/-/MFEN/-
    0x00, //Base freq
    0x00, 0x00, 0x00, 0x00, 
    0x0a, //freq 2
    0x14, //freq 3
    0x00, 0x00, 0x00, 0x0a, 0x0a, 0x00, 0x00, 0x50, 0x0a, 0x28,     
    0x1e, 0x00, 0x14, 0x04, 0x00, 0x22, 0x00, 0x14, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    //T9 settings in DualX
    0x00, 0x20, 0x03,   //gain, THR
    #ifdef RESOLUTION_12_BIT
    0x10, 0x10, 0x03,        
    #else
    0x10, 0x10, 0x00,        
    #endif
    0x0a, 0x0a, 0x0a, 
    
   #ifdef RESOLUTION_12_BIT
    0xff, 0x00, 0x02, 0xff, // CLIP
    152, //X edge ctrl  SPAN/DISCLOK/correction gradient@6
    50, //X edge correction distance
    209, //Y edge ctrl  SPAN/DISCLOK/correction gradient@6
    95, //Y edge correction distance
   #else
    0xff, 0x00, 0x02, 0xff, // CLIP
    152, //X edge ctrl  SPAN/DISCLOK/correction gradient@6
    50, //X edge correction distance
    209, //Y edge ctrl  SPAN/DISCLOK/correction gradient@6
    95, //Y edge correction distance
   #endif
    0x64, 0x0f, 0x03,
 };
#else   //firmware 2.0
static const u8 init_vals_ver_16[] = {
    /*[SPT_USERDATA_T38 INSTANCE 0] 264*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    
    /*[GEN_POWERCONFIG_T7 INSTANCE 0] 272*/
    0x40, //idle Acquisition interval
    0xff, //active Acquisition interval
    0x0a, //avtive to idle time out
    
    /*[GEN_ACQUISITIONCONFIG_T8 INSTANCE 0]275*/
    0x14, //charge time
    0x00, 0x14, 0x14, 
    0x00, //Touch auto Calibration
    0x00, 
    0x05, 0x37, 0x0a, 0xc8, //Antitouch Cal
    
    /*[TOUCH_MULTITOUCHSCREEN_T9 INSTANCE 0]285*/
    0x8f, 0x00, 0x00, 0x13, 0x0b, 0x00, 0x21, 0x2d, 0x03, 0x05, 
    #ifdef RESOLUTION_12_BIT
    0x05, 
    0x0a, //MOVE hyst init
    0x0a, //MOVE hyst next
    0x02, //Adap threahold    4位符号数
    0x04, 0x10, 0x10, 0x0a, 
    0xff, 0x0f, //X size
    0xff, 0x0f, //y size
    0xff, 0x00, 0x02, 0xff, // CLIP
    152, //X edge ctrl  SPAN/DISCLOK/correction gradient@6
    50, //X edge correction distance
    209, //Y edge ctrl  SPAN/DISCLOK/correction gradient@6
    95, //Y edge correction distance
    #else
    0x05, 
    0x08, //MOVE hyst init
    0x08, //MOVE hyst next
    0x00, //Adap threahold    4位符号数
    0x04, 0x10, 0x10, 0x0a, 
    0x47, 0x03, //X size
    0xdf, 0x01, //y size
    0xff, 0x00, 0x02, 0xff, // CLIP
    152, //X edge ctrl  SPAN/DISCLOK/correction gradient@6
    50, //X edge correction distance
    209, //Y edge ctrl  SPAN/DISCLOK/correction gradient@6
    95, //Y edge correction distance
    #endif
    0x32, 0x14, 0x00, 0x00, 0x00,
    
    /*[SPT_COMMSCONFIG_T18 INSTANCE 0]331*/
    0x00, 0x00,
    /*[SPT_GPIOPWM_T19 INSTANCE 0]333*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    /*[SPT_SELFTEST_T25 INSTANCE 0]364*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*[PROCI_TOUCHSUPPRESSION_T42 INSTANCE 0]383*/
    0x00, 0x32, 0x66, 0x38, 0x00, 0x00, 0x09, 0x00,
    
    /*[SPT_CTECONFIG_T46 INSTANCE 0]391*/
    0x00, 
    0x03, 
    0x10,  // Idle sync groups per X
    0x24, // Avtive sync groups per X
    0x00, //ADC per Sync group
    0x00, //Pulses per Adc
    0x01, //x pulse slew rate
    0x00, 
    0x00,
    /*[PROCI_STYLUS_T47 INSTANCE 0]400*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    
    /*[PROCG_NOISESUPPRESSION_T48 INSTANCE 0]410*/
    0x01,  //CTRL  -/-/-/-                       /RPTAPX/RPTFREQ/RPTEN/ENABLE
    0x80, //CFG    Drift EN/-/-/-              /CHGIN/GCMODE/GCMODE/GCMODE
    0xc0, //CALCFG INCRST/INCBIAS/CHARGON/DUALXEN           /MEANEN/-/MFEN/-
    0x00, //Base freq
    0x00, 0x00, 0x00, 0x00, 
    0x0a, //freq 2
    0x14, //freq 3
    0x00, 0x00, 0x00, 0x0a, 0x0a, 0x00, 0x00, 0x50, 0x0a, 0x28,    // noise line gain, noise line THR
    0x1e, 0x00, 0x14, 0x04, 0x00, 0x22, 0x00, 0x14, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    //T9 settings in DualX
    0x00, 0x24, 0x03,   //gain, THR
    #ifdef RESOLUTION_12_BIT
    0x10, 0x10, 0x03,        
    #else
    0x10, 0x10, 0x00,        
    #endif
    0x0a, 0x0a, 0x0a, 
    
   #ifdef RESOLUTION_12_BIT
    0xff, 0x00, 0x02, 0xff, // CLIP
    152, //X edge ctrl  SPAN/DISCLOK/correction gradient@6
    50, //X edge correction distance
    209, //Y edge ctrl  SPAN/DISCLOK/correction gradient@6
    95, //Y edge correction distance
   #else
    0xff, 0x00, 0x02, 0xff, // CLIP
    152, //X edge ctrl  SPAN/DISCLOK/correction gradient@6
    50, //X edge correction distance
    209, //Y edge ctrl  SPAN/DISCLOK/correction gradient@6
    95, //Y edge correction distance
   #endif
    0x64, 0x0f, 0x03,
    
     //T55
     0x00, 0x00, 0x00, 0x00, 
 };
#endif

static const u8 atmel_T8_Runtime_config[] = {
0x14, 0x00, 0x14, 0x14, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00,};

struct qt602240_info {
	u8 family_id;
	u8 variant_id;
	u8 version;
	u8 build;
	u8 matrix_xsize;
	u8 matrix_ysize;
	u8 object_num;
};

struct qt602240_object {
	u8 type;
	u16 start_address;
	u8 size;
	u8 instances;
	u8 num_report_ids;

	/* to map object and message */
	u8 max_reportid;
};

struct qt602240_message {
	u8 reportid;
	u8 message[7];
	u8 checksum;
};

struct qt602240_finger {
	int status;
	int x;
	int y;
	int area;
	int id;
};

/* Each client has this additional data */
struct qt602240_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	const struct qt602240_platform_data *pdata;
	struct qt602240_object *object_table;
	struct qt602240_info info;
	struct qt602240_finger finger[MAX_FINGERS];
	unsigned int irq;

	#ifdef ATMEL_TS_AUTO_RELEASE_POINTER
	struct 		hrtimer 	*Atmel_release_timer;
	#endif


	struct delayed_work delay_work;
	struct work_struct  work;
	struct early_suspend early_suspend;
	struct completion	atmel_update_stop;
	uint16_t data_ready;
	uint16_t drop_data;
	uint16_t cmd;
	int (*power)(int on);
	u8	Noise_supressed;
};

static struct qt602240_data   *atmel_mxt;
static int qt602240_make_highchg(struct qt602240_data *data);


#if 0
static bool qt602240_object_readable(unsigned int type)
{
	switch (type) {
	case QT602240_GEN_MESSAGE:
	case QT602240_GEN_COMMAND:
	case QT602240_GEN_POWER:
	case QT602240_GEN_ACQUIRE:
	case QT602240_TOUCH_MULTI:
	case QT602240_TOUCH_KEYARRAY:
	case QT602240_TOUCH_PROXIMITY:
	case QT602240_PROCI_GRIPSUPPRESSION:
	case QT602240_PROCI_TOUCHSUPPRESSION: 
	case QT602240_PROCI_STYLUS:
	case QT602240_PROCG_NOISESUPPRESSION:
//	case QT602240_PROCI_ONETOUCH:
//	case QT602240_PROCI_TWOTOUCH:
	case QT602240_SPT_COMMSCONFIG:
	case QT602240_SPT_GPIOPWM:
	case QT602240_SPT_SELFTEST:
	case QT602240_SPT_USERDATA:
	case QT602240_SPT_MESSAGECOUNT:
	case QT602240_SPT_CTECONFIG:
		return true;
	default:
		return false;
	}
}
#endif

static bool qt602240_object_writable(unsigned int type)
{
    switch (type) {
        case QT602240_SPT_USERDATA:
        case QT602240_GEN_POWER:
        case QT602240_GEN_ACQUIRE:
        case QT602240_TOUCH_MULTI:
        case QT602240_TOUCH_KEYARRAY:
        case QT602240_SPT_COMMSCONFIG:
        case QT602240_SPT_GPIOPWM:
        case QT602240_TOUCH_PROXIMITY:
        case QT602240_SPT_SELFTEST:
        case QT602240_PROCI_GRIPSUPPRESSION:
        case QT602240_PROCI_TOUCHSUPPRESSION:
        case QT602240_SPT_CTECONFIG:
        case QT602240_PROCI_STYLUS:
        case QT602240_PROCG_NOISESUPPRESSION:
		return true;
	default:
		return false;
	}
}

static void Process_satus_message(struct qt602240_message *message)
{
	 switch (message->reportid) {
        case 0x01: //T6
	        if(message->message[0] & 0x80)
	        	print_ts(DEBUG_TRACE, "Atmel T6 RESET\n");
	        if(message->message[0] & 0x40)
	        	print_ts(DEBUG_TRACE, "Atmel T6 OFL\n");
	        if(message->message[0] & 0x20)
	        	print_ts(DEBUG_TRACE, "Atmel T6 SIG Erro\n");
	        if(message->message[0] & 0x10)
	        	print_ts(DEBUG_TRACE, "Atmel T6 Calibrate\n");
	        if(message->message[0] & 0x08)
	        	print_ts(DEBUG_TRACE, "Atmel T6 Cfg Erro\n");
	        if(message->message[0] & 0x04)
	        	print_ts(DEBUG_TRACE, "Atmel T6 Communi Erro\n");
        	break;

        case 0x11: // T46
	        if(message->message[0] & 0x01)
	        	print_ts(DEBUG_TRACE, "Atmel T46 CHK Erro\n");
        	break;
        	
        case 0x12:  //T48
	        if(message->message[0] & 0x10)
	        	print_ts(DEBUG_TRACE, "Atmel T48 State Change\n");
	        if(message->message[0] & 0x04)
	        	print_ts(DEBUG_TRACE, "Atmel T48 Algo Erro\n");
	        if(message->message[0] & 0x02)
	        	print_ts(DEBUG_TRACE, "Atmel T48 ADC perX Change\n");
	        if(message->message[0] & 0x01)
	        	print_ts(DEBUG_TRACE, "Atmel T48 FREQ Change\n");

	        print_ts(DEBUG_TRACE, "Atmel ADC perX:%d\n", message->message[1]);
	        print_ts(DEBUG_TRACE, "Atmel Current Freq:%d\n", message->message[2]);
	        print_ts(DEBUG_TRACE, "Atmel State:%d\n", message->message[3]);
	        break;

	 default:
	 	 print_ts(DEBUG_TRACE, "Atmel reportid:%d\n", message->reportid);
	        print_ts(DEBUG_TRACE, "Atmel message0:%d\n", message->message[0]);
	        print_ts(DEBUG_TRACE, "Atmel message1:%d\n", message->message[1]);
	        print_ts(DEBUG_TRACE, "Atmel message2:%d\n", message->message[2]);
	        print_ts(DEBUG_TRACE, "Atmel message3:%d\n", message->message[3]);
		break;
	}
}

static int __qt602240_read_reg(struct i2c_client *client,
			       u16 reg, u16 len, void *val)
{
	struct i2c_msg xfer[2];
	u8 buf[2];

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;

	/* Write register */
	xfer[0].addr = client->addr;
	xfer[0].flags = 0;
	xfer[0].len = 2;
	xfer[0].buf = buf;

	/* Read data */
	xfer[1].addr = client->addr;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = len;
	xfer[1].buf = val;

	if (i2c_transfer(client->adapter, xfer, 2) != 2) {
		dev_err(&client->dev, "%s: i2c transfer failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int qt602240_read_reg(struct i2c_client *client, u16 reg, u8 *val)
{
	return __qt602240_read_reg(client, reg, 1, val);
}

static int qt602240_write_reg(struct i2c_client *client, u16 reg, u8 val)
{
	u8 buf[3];

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;
	buf[2] = val;

	if (i2c_master_send(client, buf, 3) != 3) {
		dev_err(&client->dev, "%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int qt602240_read_object_table(struct i2c_client *client,
				      u16 reg, u8 *object_buf)
{
	return __qt602240_read_reg(client, reg, QT602240_OBJECT_SIZE,
				   object_buf);
}

static struct qt602240_object *qt602240_get_object(struct qt602240_data *data, u8 type)
{
	struct qt602240_object *object;
	int i;

	for (i = 0; i < data->info.object_num; i++) {
		object = data->object_table + i;
		if (object->type == type)
			return object;
	}

	dev_err(&data->client->dev, "Invalid object type, type = %d\n", type);
	return NULL;
}

static int qt602240_read_message(struct qt602240_data *data,
				 struct qt602240_message *message)
{
	struct qt602240_object *object;
	u16 reg;

	object = qt602240_get_object(data, QT602240_GEN_MESSAGE);
	if (!object)
		return -EINVAL;

	reg = object->start_address;
	return __qt602240_read_reg(data->client, reg,
			sizeof(struct qt602240_message), message);
}

static int qt602240_read_object(struct qt602240_data *data,
				u8 type, u8 offset, int size, u8 *val)
{
	struct qt602240_object *object;
	u16 reg;

	object = qt602240_get_object(data, type);
	if (!object)
		return -EINVAL;
	
	reg = object->start_address;
	return __qt602240_read_reg(data->client, reg + offset, size, val);
}

static int qt602240_write_object(struct qt602240_data *data,
				 u8 type, u8 offset, u8 val)
{
	struct qt602240_object *object;
	u16 reg;

	object = qt602240_get_object(data, type);
	if (!object)
		return -EINVAL;

	reg = object->start_address;
	return qt602240_write_reg(data->client, reg + offset, val);
}

static int atmel_write_T8_Runtime(struct qt602240_data *data) //解锁之后使用新的T8参数，屏蔽antitouch-cal
{
	int size  = sizeof(atmel_T8_Runtime_config);
	int i;

        for (i = 0; i < size; i++)
        {
            qt602240_write_object(data, QT602240_GEN_ACQUIRE, i,
                    atmel_T8_Runtime_config[i]);
        }

	return 0;
}

static int atmel_Cancle_AntiTouch_cfg(void)
{
	 return 0;
		//Atmel_ts_T9_Disable();
	     atmel_write_T8_Runtime(atmel_mxt);
            //qt602240_write_object(atmel_mxt, QT602240_PROCI_TOUCHSUPPRESSION, MXT_CFG_T42_CTRL, 1);
	     //Atmel_ts_T9_Enable();
}

#if 0
 static void Atmel_Check_AntiTouch_and_Calibrate(void)
{
	u8 data[82];
	u8 loop_i, loop_j, check_mask, tch_ch = 0, atch_ch = 0;
	u8 x_limit = 38;  // X size*2

	memset(data, 0xff, sizeof(data));

	qt602240_write_object(atmel_mxt, QT602240_GEN_COMMAND, QT602240_COMMAND_DIAGNOSTIC, 0xf3);

	for (loop_i = 0;
		!(data[0] == 0xf3 && data[1] == 0) && loop_i < 10; loop_i++) {
		msleep(5);
		qt602240_read_object(atmel_mxt, QT602240_DEBUG_DIAGNOSTIC, 0, 2, data);
	}

	if (loop_i == 10)
		print_ts(DEBUG_INFO_TS, "%s: Diag data not ready\n", __func__);
	else
		print_ts(DEBUG_INFO_TS, "%s: loops %d\n", __func__, loop_i);

	qt602240_read_object(atmel_mxt, QT602240_DEBUG_DIAGNOSTIC, 0, 82, data);

	if (data[0] == 0xf3 && data[1] == 0) {
		for (loop_i = 0; loop_i < x_limit; loop_i += 2) {
			for (loop_j = 0; loop_j < BITS_PER_BYTE; loop_j++) {
				check_mask = BIT_MASK(loop_j);
				if (data[2 + loop_i] & check_mask)
					tch_ch++;
				if (data[2 + loop_i + 1] & check_mask)
					tch_ch++;
				
				if (data[2 + 40 + loop_i] & check_mask)
					atch_ch++;
				if (data[2 + 40 + loop_i + 1] & check_mask)
					atch_ch++;
			}
		}
	}
	qt602240_write_object(atmel_mxt, QT602240_GEN_COMMAND, QT602240_COMMAND_DIAGNOSTIC, 0x01);  //PAGEUP

	print_ts(DEBUG_INFO_TS, "Atmel tch_ch = %d, atch_ch = %d\n", tch_ch, atch_ch);

	#if 0
	if ((tch_ch - 25) <= atch_ch && (tch_ch || atch_ch)) {
		qt602240_write_object(atmel_mxt, QT602240_GEN_COMMAND, QT602240_COMMAND_CALIBRATE, 0x55);
	}
	#endif
}
#endif

static void qt602240_input_report(struct qt602240_data *data, int single_id)
{
	struct qt602240_finger *finger = data->finger;
	struct input_dev *input_dev = data->input_dev;
	int i;
	static int nPrevID= -1;

	if(data->drop_data) {
		data->drop_data --;
		goto end;
	}


	     if( nPrevID >= single_id || bChangeUpDn )
            {

                for( i= 0; i<MAX_FINGERS; ++i )
                {
                    if(finger[i].area == -1 )
                    {
                        continue;
                    }

                    input_report_abs(input_dev, ABS_MT_TRACKING_ID, finger[i].id);
                    input_report_abs(input_dev, ABS_MT_POSITION_X, finger[i].x);
                    input_report_abs(input_dev, ABS_MT_POSITION_Y, finger[i].y);
                   	input_report_abs(input_dev, ABS_MT_PRESSURE, 0);

                    if(finger[i].area == 0)
                    {
	                    input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR, 0);
	                    input_report_abs(input_dev, ABS_MT_PRESSURE, 0);
	                    print_ts(DEBUG_TRACE,"atmel ts up id =%d, x=%d, y=%d\n", finger[i].id, finger[i].x, finger[i].y);
                    	}
                    else
                    {
	                    input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR, finger[i].area);
	                    input_report_abs(input_dev,ABS_MT_WIDTH_MAJOR, finger[i].area);
	                    input_report_abs(input_dev, ABS_MT_PRESSURE, finger[i].area);
	                    print_ts(DEBUG_TRACE,"atmel ts dn id =%d, x=%d, y=%d, size = %d\n", finger[i].id, finger[i].x, finger[i].y, finger[i].area);
                     }
                    input_mt_sync(input_dev);

                    if(finger[i].area == 0 )
                    {
                        finger[i].area= -1;
                    }
                }

                input_sync(input_dev);

            }

	end:
            nPrevID= single_id;
            bChangeUpDn = 0;
	    //input_report_key(input_dev, BTN_TOUCH, finger_num > 0);

}

static void Process_T9_touchevent(struct qt602240_data *data,
				      struct qt602240_message *message, int id)
{
	struct qt602240_finger *finger = data->finger;
	u8 status = message->message[0];
	int x;
	int y;
	int area;

    	#ifdef RESOLUTION_12_BIT
	x = (message->message[1] << 4) | ((message->message[3] & 0xf0) >> 4);
	y = (message->message[2] << 4) | ((message->message[3] & 0x0f) );
	#else
	x = (message->message[1] << 2) | ((message->message[3] & ~0x3f) >> 6);
	y = (message->message[2] << 2) | ((message->message[3] & ~0xf3) >> 2);
	#endif

	area = message->message[4] * 10;

	/* Check the touch is present on the screen */
	if (!(status & QT602240_DETECT)) {
		if( (status & QT602240_RELEASE) || (status & QT602240_SUPPRESS)) {
			bChangeUpDn= 1;
			finger[id].status = QT602240_RELEASE;
			finger[id].area= 0;

			//当解锁弹起时，取消antitouch的设置
			#ifdef RESOLUTION_12_BIT
			if((SimulateUnlock == SimuUnlock_pressed) && abs(x - SimulateUnlock_origin_x) > 400)
			#else
			if((SimulateUnlock == SimuUnlock_pressed) && abs(x - SimulateUnlock_origin_x) > 100)
			#endif
			{
				atmel_Cancle_AntiTouch_cfg();
				SimulateUnlock = SimuUnlock_disable;
			}
		}
	}
	else
	{
		if(SimulateUnlock == SimuUnlock_enable) 
		{
			SimulateUnlock = SimuUnlock_pressed;
			SimulateUnlock_origin_x = x;
		}

		if (status & QT602240_PRESS)
		{
			bChangeUpDn= 1;
			print_ts(DEBUG_TRACE, "Atmel pressed\n");
		}

		finger[id].status = status & QT602240_MOVE ?
					QT602240_MOVE : QT602240_PRESS;
		finger[id].x = x;
		finger[id].y = y;
		finger[id].area = area;
		finger[id].id = id;

		#ifdef ATMEL_TS_AUTO_RELEASE_POINTER
			/* Start the HR timer if one is not active */
			if (hrtimer_active(data->Atmel_release_timer))
				hrtimer_cancel(data->Atmel_release_timer);

			hrtimer_start(data->Atmel_release_timer,
				ktime_set((RELEASE_POINTER_MS /1000),
				(RELEASE_POINTER_MS % 1000) * 1000000), HRTIMER_MODE_REL);
		#endif

	}
}

static int release_all_pointers(void)
{	 
	struct	qt602240_data *data=atmel_mxt;
	struct qt602240_finger *fingers = data->finger;
	int i;

	print_ts(DEBUG_TRACE, "Atmel release_all_pointers\n");
	 for( i= 0; i<MAX_FINGERS; ++i )
	 {
		 if(fingers[i].area == -1 )
		{
			continue;
		 }
		 
	  	input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, fingers[i].id);
		 input_report_abs(data->input_dev, ABS_MT_POSITION_X, fingers[i].x);
		 input_report_abs(data->input_dev, ABS_MT_POSITION_Y, fingers[i].y);
		 input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);

		input_mt_sync(data->input_dev);

		fingers[i].area= -1;
	}
	input_sync(data->input_dev);
	return 1;
}

#ifdef ATMEL_TS_AUTO_RELEASE_POINTER
static enum hrtimer_restart Release_pointer_handle(struct hrtimer *timer)
{
	release_all_pointers();
	
	return HRTIMER_NORESTART;
}
#endif



static int Read_Gpio_TS_irq(void)
{
	int rc;
	rc = gpio_get_value(TS_PEN_IRQ_GPIO);
	return rc;
}

static void Atmel_irq_set_locked(struct i2c_client *client, int enable_disable, int in_isr)
{
	static int irq_status = 1;

	if (enable_disable == irq_status) {
		print_ts(DEBUG_TRACE, "%s: irq is configed, status=%d\n", __func__, enable_disable);
		return;
	}

	irq_status = enable_disable;
	
	if (enable_disable) {
		enable_irq(client->irq);
	} else {
		if (in_isr) 
			disable_irq_nosync(client->irq);
		else
			disable_irq(client->irq);
	}
}

/*
 * Processes messages when the interrupt line (CHG) is asserted. Keeps
 * reading messages until a message with report ID 0xFF is received,
 * which indicates that there is no more new messages.
 *
 */

static void ts_thread_read_msg(struct work_struct *work)
{
	struct qt602240_data *data = container_of(work, struct qt602240_data, work);
	struct qt602240_message message;
	struct qt602240_object *object;
	
	int id;
	u8 reportid;
	u8 max_reportid;
	u8 min_reportid;

	/*
	if (is_tp_suspended) {
			print_ts(DEBUG_INFO_TS, "%s: Atmel is suspended.  do nothing!!!\n\n", __func__);
			return;
	}
	*/
		
	do {
		if (qt602240_read_message(data, &message)) {
			print_ts(DEBUG_INFO_ERROR,"Atmel ts_thread not ready\n");
			goto end;
		}

		reportid = message.reportid;

		/* whether reportid is thing of QT602240_TOUCH_MULTI */
		object = qt602240_get_object(data, QT602240_TOUCH_MULTI);
		if (!object)
			goto end;

		max_reportid = object->max_reportid;
		min_reportid = max_reportid - object->num_report_ids + 1;
		id = reportid - min_reportid;

		if (reportid >= min_reportid && reportid <= max_reportid)
		{
			Process_T9_touchevent(data, &message, id);
			qt602240_input_report(data, id);
		}
		else
			Process_satus_message(&message);
	//} while (reportid != 0xff);
	} while (Read_Gpio_TS_irq() == 0);

	end:

	complete_all(&data->atmel_update_stop);
	Atmel_irq_set_locked(data->client, 1, 0);

}


static struct workqueue_struct *queue = NULL;

static irqreturn_t qt602240_interrupt_handle(int irq, void *dev_id)
{
	struct qt602240_data *data = dev_id;

	print_ts(DEBUG_TRACE, "Atmel qt602240_interrupt_handle\n");

	Atmel_irq_set_locked(data->client, 0, 1);

	queue_work(queue, &data->work);
	return IRQ_HANDLED;
}

static void atmel_delayed_work(struct work_struct *work)
{
	struct qt602240_data *data = \
		container_of(work, struct qt602240_data, delay_work.work);

	if(data->cmd == TS_CMD_RESET_6S) {
		
	} 
	else if( data->cmd == TS_CMD_INIT) {
		
	} 
	else if( data->cmd == TS_CMD_RESUME_80MS) {
		print_ts(DEBUG_TRACE, "Atmel clear drop_data 200ms after later resume\n");
		data->drop_data = 0;
		release_all_pointers();
	}
}

/*2011-11-08 van add virtual key support start*/
static ssize_t vk_Atmel_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
		return sprintf(buf,
	             __stringify(EV_KEY) ":" __stringify(KEY_MENU)  ":51:830:100:46"
	         ":" __stringify(EV_KEY) ":" __stringify(KEY_HOME)   ":240:830:100:46"
	         ":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":429:830:100:46"
	         "\n");

}

static struct kobj_attribute vk_Atmel_attr = {
	.attr = {
		.name = "virtualkeys.qt602240_ts",
		.mode = S_IRUGO,
	},
	.show = &vk_Atmel_show,
};

static struct attribute *Atmel_properties_attrs[] = {
	&vk_Atmel_attr.attr,
	NULL
};

static struct attribute_group Atmel_properties_attr_group = {
	.attrs = Atmel_properties_attrs,
};
/*2011-11-08 van add virtual key support end*/

/*2011-12-15 van add ioctrl operation start*/
static int Atmel_dev_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int Atmel_dev_release(struct inode *inode, struct file *file)
{
	return 0;
}

static long Atmel_dev_ioctl(struct file *file,
		     unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int version;
	char *fn;
	

	switch (cmd) {
	case ATMEL_GET_VERSION:
		if (copy_to_user((void *)arg, &Atmel_Program_state,
				 sizeof(Atmel_Program_state))) {
			ret = -EFAULT;
			print_ts(DEBUG_INFO_TS, "Atmel GET_VERSION copy_to_user error\n");
		}
		break;
		
	case ATMEL_GET_FW_VERSION:
		break;
		
	case ATMEL_PROGRAM_START:
		if (copy_from_user(&version, (void *)arg, sizeof(version))) {
			ret = -EFAULT;
			print_ts(DEBUG_INFO_TS, "Atmel PROGRAM_START copy_from_user error\n");
			break;
		}
		
		switch (version) {
		case TOUCH_SCREEN_VERION_0:
			fn = ATMEL_FW_0_NAME;
			break;
		case TOUCH_SCREEN_VERION_1:
			fn = ATMEL_FW_1_NAME;
			break;
		case TOUCH_SCREEN_VERION_2:
			fn = ATMEL_FW_2_NAME;
			break;
		default:
			fn = ATMEL_FW_1_NAME;
			break;
		}

		break;
		
	case ATMEL_GET_PROGRAM_STATE:
		if (copy_to_user((void *)arg, &Atmel_Program_state,
				 sizeof(Atmel_Program_state))) {
			ret = -EFAULT;
			print_ts(DEBUG_INFO_TS, "Atmel GET_PROGRAM_STATE copy_to_user error\n");
		}
		break;

	default:
		return -EIO;
	}

	return ret;
}
/*2011-12-15 van add ioctrl operation end*/


/*2011-12-15 van add proc debug entry start*/
static ssize_t Atmel_proc_write( struct file *filp, const char __user *buff,
                        unsigned long len, void *data )
{
	char proc_data[32] = {0};  

		if (copy_from_user( &proc_data, buff, len )) {
		print_ts(DEBUG_INFO_ERROR, "Atmel_proc_write read data error.\n");
		return -EFAULT;
	}

	if(proc_data[0] == 'L')
	{
		u8 val;
		val = (proc_data[1]-0x30)*100 + (proc_data[2]-0x30)*10 + (proc_data[3]-0x30); 

		pm8058_set_led_current(3, val);
		print_ts(DEBUG_INFO_TS, "pm8058_set_led_current = %d mA\n", val);
	}

	if(proc_data[0] == 'T')
	{
		u8 type, offset, val;

		type = (proc_data[1]-0x30)*10 + (proc_data[2]-0x30);
		offset = (proc_data[3]-0x30)*10 + (proc_data[4]-0x30);
		val = (proc_data[5]-0x30)*100 + (proc_data[6]-0x30)*10 + (proc_data[7]-0x30); 

		qt602240_write_object(atmel_mxt, type, offset, val);
		print_ts(DEBUG_INFO_TS, "Write Atmel Reg T%d at offset %d, value = %d\n", type, offset, val);
	}

	if(proc_data[0] == 'R')
	{
		u8 type, offset, val;

		type = (proc_data[1]-0x30)*10 + (proc_data[2]-0x30);
		offset = (proc_data[3]-0x30)*10 + (proc_data[4]-0x30);
		
		qt602240_read_object(atmel_mxt, type, offset, 1, &val);
		print_ts(DEBUG_INFO_TS, "Read Atmel Reg T%d at offset %d, value = %d\n", type, offset, val);
	}
	
	//polling the irq line status
	if(proc_data[0] == 'I')
	{
		atmel_mxt->cmd = 8;
		schedule_delayed_work(&atmel_mxt->delay_work, msecs_to_jiffies(1000));
	}

	//software reset
	if (proc_data[0] == '1') {
		print_ts(DEBUG_INFO_TS, "atmel 224E software reset\n");
		/* Soft reset */
		qt602240_write_object(data, QT602240_GEN_COMMAND,
				QT602240_COMMAND_RESET, 1);
		msleep(QT602240_RESET_TIME);
	} 

	// check anti touch
	else if(proc_data[0] == '2') 
	{
		//Atmel_Check_AntiTouch_and_Calibrate();
	} 

	//check logo level
	else if(proc_data[0] == '3') 
	{
		if(logo_level != 3) 
		{
			print_ts(DEBUG_INFO_TS, "atmel change logo_level to 3!\n");
			logo_level = 3;
		}
		else
		{
			print_ts(DEBUG_INFO_TS, "atmel change logo_level to 2!\n");
			logo_level = 2;
		}
	} 

	//update
	else if(proc_data[0] == '4') 
	{
	}
	else if(proc_data[0] == '5') 
	{
	}
	else if(proc_data[0] == '6') 
	{
	}
	
	return len;
}


static struct proc_dir_entry *proc_entry;
static int init_Atmel_proc(struct qt602240_data *data)
{
	int ret=0;
	
	proc_entry = create_proc_entry( "Atmel_proc_write", 0666, NULL );
	proc_entry->data = data;
	
	if (proc_entry == NULL) {
		ret = -ENOMEM;
	  	print_ts(DEBUG_INFO_ERROR, "Atmel Couldn't create proc entry\n");
	} else {
		proc_entry->write_proc = Atmel_proc_write;
		//proc_entry->owner = THIS_MODULE;
	}
  
	return ret;
}



/*2011-12-15 van add proc debug entry end*/

static int qt602240_check_reg_init(struct qt602240_data *data)
{
	struct qt602240_object *object;
	int index = 0;
	int i, j;
	//u8 version = data->info.version;
	u8 *init_vals;

	#if 0
	switch (version) {
	case QT602240_VER_16:
		init_vals = (u8 *)init_vals_ver_16;
		break;
	case QT602240_VER_20:
		init_vals = (u8 *)init_vals_ver_20;
		break;
	case QT602240_VER_21:
		init_vals = (u8 *)init_vals_ver_21;
		break;
	case QT602240_VER_22:
		init_vals = (u8 *)init_vals_ver_22;
		break;
	default:
		 print_ts(DEBUG_INFO_ERROR, "Firmware version %d doesn't support\n", version);
		return -EINVAL;
	}
	#else
	init_vals = (u8 *)init_vals_ver_16;
	#endif

	for (i = 0; i < data->info.object_num; i++) {
		object = data->object_table + i;

		if (!qt602240_object_writable(object->type))
				continue;

	        for (j = 0; j < object->size + 1; j++)
	            qt602240_write_object(data, object->type, j,
	                    init_vals[index + j]);

		index += object->size + 1;
	}

	return 0;
}

static int qt602240_check_matrix_size(struct qt602240_data *data)
{
	const struct qt602240_platform_data *pdata = data->pdata;
	struct device *dev = &data->client->dev;
	int mode = -1;
	int error;
	u8 val;

	switch (pdata->x_line) {
	case 0 ... 15:
		if (pdata->y_line <= 14)
			mode = 0;
		break;
	case 16:
		if (pdata->y_line <= 12)
			mode = 1;
		if (pdata->y_line == 13 || pdata->y_line == 14)
			mode = 0;
		break;
	case 17:
		if (pdata->y_line <= 11)
			mode = 2;
		if (pdata->y_line == 12 || pdata->y_line == 13)
			mode = 1;
		break;
	case 18:
		if (pdata->y_line <= 10)
			mode = 3;
		if (pdata->y_line == 11 || pdata->y_line == 12)
			mode = 2;
		break;
	case 19:
		if (pdata->y_line <= 9)
			mode = 4;
		if (pdata->y_line == 10 || pdata->y_line == 11)
			mode = 3;
		break;
	case 20:
		mode = 4;
	}

	if (mode < 0) {
		dev_err(dev, "Invalid X/Y lines\n");
		return -EINVAL;
	}

	error = qt602240_read_object(data, QT602240_SPT_CTECONFIG,
				QT602240_CTE_MODE, 1, &val);
	if (error)
		return error;

	if (mode == val)
		return 0;

	/* Change the CTE configuration */
	qt602240_write_object(data, QT602240_SPT_CTECONFIG,
			QT602240_CTE_CTRL, 1);
	qt602240_write_object(data, QT602240_SPT_CTECONFIG,
			QT602240_CTE_MODE, mode);
	qt602240_write_object(data, QT602240_SPT_CTECONFIG,
			QT602240_CTE_CTRL, 0);

	return 0;
}

static int qt602240_make_highchg(struct qt602240_data *data)
{
	int count = 0;
	int error = 0;
	u8 val = 0;

	/* Read dummy message to make high CHG pin */
	do {
		count++;
		error = qt602240_read_object(data, QT602240_GEN_MESSAGE, 0, 1, &val);
		if (error)
		{
			print_ts(DEBUG_INFO_ERROR,"atmel make_highchg not ready\n");
			return error;
		}
		print_ts(DEBUG_TRACE,"atmel qt602240_make_highchg %d times\n", count);
	} while ((val != 0xff) && (count < 100));

	return 0;
}

static void qt602240_handle_pdata(struct qt602240_data *data)
{
	const struct qt602240_platform_data *pdata = data->pdata;
	u8 voltage;

	return;

	/* Set touchscreen lines */
	qt602240_write_object(data, QT602240_TOUCH_MULTI, QT602240_TOUCH_XSIZE,
			pdata->x_line);
	qt602240_write_object(data, QT602240_TOUCH_MULTI, QT602240_TOUCH_YSIZE,
			pdata->y_line);

	/* Set touchscreen orient */
	qt602240_write_object(data, QT602240_TOUCH_MULTI, QT602240_TOUCH_ORIENT,
			pdata->orient);

	/* Set touchscreen burst length */
	qt602240_write_object(data, QT602240_TOUCH_MULTI,
			QT602240_TOUCH_BLEN, pdata->blen);

	/* Set touchscreen threshold */
	qt602240_write_object(data, QT602240_TOUCH_MULTI,
			QT602240_TOUCH_TCHTHR, pdata->threshold);

	/* Set touchscreen resolution */
	qt602240_write_object(data, QT602240_TOUCH_MULTI,
			QT602240_TOUCH_XRANGE_LSB, (pdata->x_size - 1) & 0xff);
	qt602240_write_object(data, QT602240_TOUCH_MULTI,
			QT602240_TOUCH_XRANGE_MSB, (pdata->x_size - 1) >> 8);
	qt602240_write_object(data, QT602240_TOUCH_MULTI,
			QT602240_TOUCH_YRANGE_LSB, (pdata->y_size - 1) & 0xff);
	qt602240_write_object(data, QT602240_TOUCH_MULTI,
			QT602240_TOUCH_YRANGE_MSB, (pdata->y_size - 1) >> 8);

	/* Set touchscreen voltage */
	if (data->info.version >= QT602240_VER_21 && pdata->voltage) {
		if (pdata->voltage < QT602240_VOLTAGE_DEFAULT) {
			voltage = (QT602240_VOLTAGE_DEFAULT - pdata->voltage) /
				QT602240_VOLTAGE_STEP;
			voltage = 0xff - voltage + 1;
		} else
			voltage = (pdata->voltage - QT602240_VOLTAGE_DEFAULT) /
				QT602240_VOLTAGE_STEP;

		qt602240_write_object(data, QT602240_SPT_CTECONFIG,
				QT602240_CTE_VOLTAGE, voltage);
	}
}

static int qt602240_get_info(struct qt602240_data *data)
{
	struct i2c_client *client = data->client;
	struct qt602240_info *info = &data->info;
	int error;
	u8 val;

	error = qt602240_read_reg(client, QT602240_FAMILY_ID, &val);
	if (error)
		return error;
	info->family_id = val;

	error = qt602240_read_reg(client, QT602240_VARIANT_ID, &val);
	if (error)
		return error;
	info->variant_id = val;

	error = qt602240_read_reg(client, QT602240_VERSION, &val);
	if (error)
		return error;
	info->version = val;

	error = qt602240_read_reg(client, QT602240_BUILD, &val);
	if (error)
		return error;
	info->build = val;

	error = qt602240_read_reg(client, QT602240_OBJECT_NUM, &val);
	if (error)
		return error;
	info->object_num = val;

	return 0;
}

static int qt602240_get_object_table(struct qt602240_data *data)
{
	int error;
	int i;
	u16 reg;
	u8 reportid = 0;
	u8 buf[QT602240_OBJECT_SIZE];

	for (i = 0; i < data->info.object_num; i++) {
		struct qt602240_object *object = data->object_table + i;

		reg = QT602240_OBJECT_START + QT602240_OBJECT_SIZE * i;
		error = qt602240_read_object_table(data->client, reg, buf);
		if (error)
			return error;

		object->type = buf[0];
		object->start_address = (buf[2] << 8) | buf[1];
		object->size = buf[3];
		object->instances = buf[4];
		object->num_report_ids = buf[5];

		if (object->num_report_ids) {
			reportid += object->num_report_ids *
					(object->instances + 1);
			object->max_reportid = reportid;
		}
	}

	return 0;
}

static int qt602240_initialize(struct qt602240_data *data)
{
	struct i2c_client *client = data->client;
	struct qt602240_info *info = &data->info;
	int error;
	u8 val;

	error = qt602240_get_info(data);
	if (error)
		return error;

	data->object_table = kcalloc(info->object_num,
				     sizeof(struct qt602240_data),
				     GFP_KERNEL);
	if (!data->object_table) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		return -ENOMEM;
	}

	/* Get object table information */
	error = qt602240_get_object_table(data);
	if (error)
		return error;

	if(0)
	{
	/* Check register init values */
	error = qt602240_check_reg_init(data);
	if (error)
		return error;

	/* Check X/Y matrix size */
	error = qt602240_check_matrix_size(data);
	if (error)
		return error;

	qt602240_handle_pdata(data);
	}

	/* Backup to memory */
	qt602240_write_object(data, QT602240_GEN_COMMAND,
			QT602240_COMMAND_BACKUPNV,
			QT602240_BACKUP_VALUE);
	msleep(QT602240_BACKUP_TIME);

	/* Soft reset */
	qt602240_write_object(data, QT602240_GEN_COMMAND,
			QT602240_COMMAND_RESET, 1);
	msleep(QT602240_RESET_TIME);

	error = qt602240_make_highchg(data);
	if (error)
		return error;

	/* Update matrix size at info struct */
	error = qt602240_read_reg(client, QT602240_MATRIX_X_SIZE, &val);
	if (error)
		return error;
	info->matrix_xsize = val;

	error = qt602240_read_reg(client, QT602240_MATRIX_Y_SIZE, &val);
	if (error)
		return error;
	info->matrix_ysize = val;

	dev_info(&client->dev,
			"Family ID: %d Variant ID: %d Version: %d Build: %d\n",
			info->family_id, info->variant_id, info->version,
			info->build);

	dev_info(&client->dev,
			"Matrix X Size: %d Matrix Y Size: %d Object Num: %d\n",
			info->matrix_xsize, info->matrix_ysize,
			info->object_num);

	return 0;
}


#if 0
static ssize_t qt602240_object_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct qt602240_data *data = dev_get_drvdata(dev);
	struct qt602240_object *object;
	int count = 0;
	int i, j;
	int error;
	u8 val;

	for (i = 0; i < data->info.object_num; i++) {
		object = data->object_table + i;

		count += sprintf(buf + count,
				"Object Table Element %d(Type %d)\n",
				i + 1, object->type);

		if (!qt602240_object_readable(object->type)) {
			count += sprintf(buf + count, "\n");
			continue;
		}

		for (j = 0; j < object->size + 1; j++) {
			error = qt602240_read_object(data,
						object->type, j, 1, &val);
			if (error)
				return error;

			count += sprintf(buf + count,
					"  Byte %d: 0x%x (%d)\n", j, val, val);
		}

		count += sprintf(buf + count, "\n");
	}

	return count;
}
#endif

static void qt602240_start(struct qt602240_data *data)
{
	/* Touch enable */
	qt602240_write_object(data,
			QT602240_TOUCH_MULTI, QT602240_TOUCH_CTRL, 0x83);
}

static void qt602240_stop(struct qt602240_data *data)
{
	/* Touch disable */
	qt602240_write_object(data,
			QT602240_TOUCH_MULTI, QT602240_TOUCH_CTRL, 0);
}

static int qt602240_input_open(struct input_dev *dev)
{
	struct qt602240_data *data = input_get_drvdata(dev);

	qt602240_start(data);

	return 0;
}

static void qt602240_input_close(struct input_dev *dev)
{
	struct qt602240_data *data = input_get_drvdata(dev);

	qt602240_stop(data);
}

static int __devinit qt602240_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct qt602240_data *data;
	struct input_dev *input_dev;
	int error;
	int i;
	uint16_t max_x, max_y;
	
	#ifdef ATMEL_TS_AUTO_RELEASE_POINTER
	struct 	hrtimer 	*release_timer;
	#endif

	if (!client->dev.platform_data)
		return -EINVAL;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
	{
		print_ts(DEBUG_INFO_ERROR, "atmel %s I2C_FUNC_I2C error\n", __func__);
	}

	#ifdef ATMEL_TS_AUTO_RELEASE_POINTER
	release_timer= kzalloc(sizeof(*release_timer), GFP_KERNEL);
	if (release_timer== NULL) {
		print_ts(DEBUG_INFO_ERROR,"atmel TS i2c_probe Unable to allocate memory\n");
		error = -ENOMEM;
		goto err_timer_alloc;
	}
	else{
		hrtimer_init(release_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		release_timer->function =Release_pointer_handle;
	}
	#endif

	
	data = kzalloc(sizeof(struct qt602240_data), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!data || !input_dev) {
		dev_err(&client->dev, "atmel Failed to allocate memory\n");
		error = -ENOMEM;
		goto err_free_mem;
	}

	init_completion(&data->atmel_update_stop);
	INIT_DELAYED_WORK(&data->delay_work, atmel_delayed_work);

	queue = create_singlethread_workqueue("mxc_ts_handler"); /*创建一个单线程的工作队列*/
	INIT_WORK(&data->work, ts_thread_read_msg);

	print_ts(DEBUG_INFO_TS, "atmel qt602240_probe\n");

	data->pdata = client->dev.platform_data;
	data->client = client;
	data->input_dev = input_dev;
	data->irq = client->irq;
	#ifdef ATMEL_TS_AUTO_RELEASE_POINTER
	data->Atmel_release_timer = release_timer;
	#endif


	if (data->pdata)
	data->power = data->pdata->power;
	else
		print_ts(DEBUG_INFO_ERROR, "atmel qt602240_probe power null\n");

	#if 0
	if (data->power) 
	{
		error = data->power(1);
		if (error < 0) 
		{
			print_ts(DEBUG_INFO_ERROR, "atmel_ts_probe power on failed\n");
		}
	}
	msleep(20);
	#endif

	atmel_mxt = data;

	input_dev->name = ATMEL_QT602240E_NAME;
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;
	input_dev->open = qt602240_input_open;
	input_dev->close = qt602240_input_close;

	// x and y has been exchanged in ts Driver 224E 
	max_x = init_vals_ver_16[QT602240_TOUCH_MULTI_T9_OFFSET + QT602240_TOUCH_YRANGE_MSB] <<8 | init_vals_ver_16[QT602240_TOUCH_MULTI_T9_OFFSET + QT602240_TOUCH_YRANGE_LSB];
	max_y = init_vals_ver_16[QT602240_TOUCH_MULTI_T9_OFFSET + QT602240_TOUCH_XRANGE_MSB] <<8 | init_vals_ver_16[QT602240_TOUCH_MULTI_T9_OFFSET + QT602240_TOUCH_XRANGE_LSB];

	print_ts(DEBUG_INFO_TS,"atmel max_x =%d, max_y =%d ", max_x, max_y);

	// 框架层EventHub::getAbsoluteAxisInfo 逻辑有问题，把max当成maxsize，实际 maxsize = max+1
	/* For single touch */
	input_set_abs_params(input_dev, ABS_X, 0, max_x +1, 0, 0);
	#ifdef RESOLUTION_12_BIT
	input_set_abs_params(input_dev, ABS_Y, 0, max_y- 228, 0, 0);
	#else
	input_set_abs_params(input_dev, ABS_Y, 0, max_y- 56, 0, 0);
	#endif
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, 255, 0, 0);
	input_set_abs_params(input_dev, ABS_TOOL_WIDTH, 0, 255, 0, 0);

	/* For multi touch */
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, max_x +1, 0, 0);
	#ifdef RESOLUTION_12_BIT
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, max_y - 228, 0, 0);
	#else
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, max_y - 56, 0, 0);
	#endif
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, 255,0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, MAX_FINGERS, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, 255,0, 0);

	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(EV_SYN, input_dev->evbit);
	__set_bit(EV_MSC, input_dev->evbit);
	//__set_bit(BTN_TOUCH, input_dev->keybit);

	__set_bit(KEY_SEARCH, input_dev->keybit);
	__set_bit(KEY_HOME, input_dev->keybit);
	__set_bit(KEY_MENU, input_dev->keybit);
	__set_bit(KEY_BACK, input_dev->keybit);
	input_dev->mscbit[0] = BIT_MASK(MSC_GESTURE);

	input_set_drvdata(input_dev, data);

	i2c_set_clientdata(client, data);

	error = qt602240_initialize(data);
	if (error)
		goto err_free_object;

	error = request_irq(client->irq, qt602240_interrupt_handle,
			IRQF_TRIGGER_LOW, client->dev.driver->name, data);
    
	if (error) {
		dev_err(&client->dev, "Failed to register interrupt\n");
		goto err_free_object;
	}

	/* virtual keys */
	Atmel_properties_kobj = kobject_create_and_add("board_properties",
				NULL);
	if (Atmel_properties_kobj)
		error = sysfs_create_group(Atmel_properties_kobj,
			&Atmel_properties_attr_group);
	
	if (!Atmel_properties_kobj || error)
		print_ts(DEBUG_INFO_ERROR, "atmel %s: failed to create board_properties\n", __func__);
	/* virtual keys */

	error = input_register_device(input_dev);
	if (error)
		goto err_free_irq;

	#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 10;
	data->early_suspend.suspend = Atmel_early_suspend;
	data->early_suspend.resume = Atmel_late_resume;
	register_early_suspend(&data->early_suspend);
	#endif


	error = init_Atmel_proc(data);

	SimulateUnlock = SimuUnlock_enable;

	 for( i= 0; i<MAX_FINGERS; i++)  //初始压力值设置为-1，没有按下的点将不会 report
	{
		data->finger[i].area = -1 ;
	 }
	return 0;
	
err_free_irq:
	free_irq(client->irq, data);
err_free_object:
	kfree(data->object_table);
err_free_mem:
	input_free_device(input_dev);
	kfree(data);
#ifdef ATMEL_TS_AUTO_RELEASE_POINTER
	kfree(release_timer);
err_timer_alloc:
#endif
	return error;
}

static int __devexit qt602240_remove(struct i2c_client *client)
{
	struct qt602240_data *data = i2c_get_clientdata(client);
	atmel_mxt->Noise_supressed = USB_QUIT_NOISE_SUPPRESS;
	
	wait_for_completion(&data->atmel_update_stop);
	free_irq(data->irq, data);
	input_unregister_device(data->input_dev);
	unregister_early_suspend(&data->early_suspend);
	kfree(data->object_table);
	kfree(data);

	return 0;
}

static int qt602240_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct qt602240_data *data = i2c_get_clientdata(client);

	//is_tp_suspended = 1;
	Atmel_irq_set_locked(client, 0, 0);
	cancel_delayed_work_sync(&data->delay_work);

	#ifdef ATMEL_224E_SLEEP
	qt602240_stop(data);
	#else
	if (data->power) 
	{
		if (data->power(0) < 0)
			print_ts(DEBUG_INFO_ERROR, "atmel qt602240_suspend power off failed\n");
	}
	#endif
	atmel_mxt->Noise_supressed = USB_QUIT_NOISE_SUPPRESS;

	return 0;
}

static int qt602240_resume(struct i2c_client *client)
{
	struct qt602240_data *data = i2c_get_clientdata(client);

	#ifdef ATMEL_224E_SLEEP
	/* Soft reset */
	qt602240_write_object(data, QT602240_GEN_COMMAND,
			QT602240_COMMAND_RESET, 1);
	msleep(QT602240_RESET_TIME);

	qt602240_start(data);
	#else
	if (data->power) 
	{
		if (data->power(1) < 0)
			print_ts(DEBUG_INFO_ERROR, "atmel qt602240_resume power on failed\n");
	}
	#endif

	data->drop_data = 10;
	//is_tp_suspended = 0;
	Atmel_irq_set_locked(client, 1, 0);
	SimulateUnlock = SimuUnlock_enable;

	data->cmd = TS_CMD_RESUME_80MS;
	schedule_delayed_work(&data->delay_work, msecs_to_jiffies(200));

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void Atmel_early_suspend(struct early_suspend *h)
{
	struct qt602240_data *ts;
	ts = container_of(h, struct qt602240_data, early_suspend);
	qt602240_suspend(ts->client, PMSG_SUSPEND);
}

static void Atmel_late_resume(struct early_suspend *h)
{
	struct qt602240_data *ts;
	ts = container_of(h, struct qt602240_data, early_suspend);
	qt602240_resume(ts->client);
}
#endif


static const struct i2c_device_id qt602240_id[] = {
	{ ATMEL_QT602240E_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, qt602240_id);

static struct i2c_driver qt602240_driver = {
	.driver = {
		.name	= ATMEL_QT602240E_NAME,
		.owner	= THIS_MODULE,
	},
	.probe		= qt602240_probe,
	.remove		= __devexit_p(qt602240_remove),
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= qt602240_suspend,
	.resume		= qt602240_resume,
#endif
	.id_table	= qt602240_id,
};

static struct file_operations Atmel_dev_fops = {
	.owner		= THIS_MODULE,
	.open		= Atmel_dev_open,
	.unlocked_ioctl		= Atmel_dev_ioctl,
	.release	= Atmel_dev_release,
};

static struct miscdevice Atmel_control_device = {
	.minor	= 255,
	.name	= "syna_touch",  //保持和新思一样的操作节点
	.fops	= &Atmel_dev_fops,
};

static int __init qt602240_init(void)
{
	if(!Check_Touchscreen_type(TS_ATMEL_224))
		return -EINVAL;
	
	misc_register(&Atmel_control_device);
	return i2c_add_driver(&qt602240_driver);
}

static void __exit qt602240_exit(void)
{
	i2c_del_driver(&qt602240_driver);
}

module_init(qt602240_init);
module_exit(qt602240_exit);

/* Module information */
MODULE_AUTHOR("Joonyoung Shim <jy0922.shim@samsung.com>");
MODULE_DESCRIPTION("AT42QT602240/ATMXT224 Touchscreen driver");
MODULE_LICENSE("GPL");
