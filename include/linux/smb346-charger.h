/*
 * Copyright (C) 2011-2012 OPPO, Inc.
 * Author: chendx <cdx@oppo.com>
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
#ifndef __SMB346_CHARGER_H__
#define __SMB346_CHARGER_H__
#include <linux/power_supply.h>
#include <mach/msm_hsusb.h>

//smb346-charger platform pdata
struct smb346_platform_pdata {
	int smb346_irq_gpio;//add for smb346 charge chip interrupt
	
	int gpio_DRDY;
	int (*power)(int on);
	bool (*get_ftm_mode)(void);
};

#define SMB346_CHARGER_NAME "smb346-charger"

#define FEATURE_TAPPER_ADJUST_CURRENT
//#define FEATURE_AUTO_RECHARGE
//#define FEATURE_SUSPEND_CHARGE
//#define FEATURE_OPPO_TEMP_CONTROL
#define FEATURE_OPPO_AICL_CONTROL

#define CHG_POLL_DELAY 1000000

#define ACCESSORY_CHG_COMPELETE_LEVEL 100
#define ACCESSORY_RECHG_HYST_LEVEL 20

#define ACCESSORY_RECHG_LEVEL ACCESSORY_CHG_COMPELETE_LEVEL - ACCESSORY_RECHG_HYST_LEVEL

/*Accessory smart charge mode*/
enum{
	DC_DC_OFF_MODE,
	DC_DC_ON_MODE
};

enum{
	SMART_CHARGE_MODE_INVALID,
	SMART_CHARGE_MODE
};

enum {
	CHG_TYPE_USB,
	CHG_TYPE_AC
};

enum smb346_hardware_charger_event {
	/* irq event */
	IRQ_TRIGGERED__STAT,

	/* timer expired event */
	TIMER_EXPIRED__HEART_BEAT,
	TIMER_EXPIRED__CC_CHG_DETECT,
	TIMER_EXPIRED__CC_DISCHG_DETECT,
	TIMER_EXPIRED__RESUME_CHECK,
	TIMER_EXPIRED__CHG_GET_ADC,
	TIMER_EXPIRED__MAX_CHG_TIME,
	TIMER_EXPIRED__END_CHG_DETECT_TIME,
	#ifndef FEATURE_AUTO_RECHARGE
	TIMER_EXPIRED__CHG_RESUME,
	#endif
	TIMER_EXPIRED__CALLING,
	/* system status changed event */
	SYSTEM_STATUS_CHANGED__AC_VALID,
	SYSTEM_STATUS_CHANGED__USB_VALID,
	SYSTEM_STATUS_CHANGED__NONSTANDARD_VALID,
	SYSTEM_STATUS_CHANGED__USB_INVALID,
	SYSTEM_STATUS_CHANGED__USB_SUSPEND_ON,
	SYSTEM_STATUS_CHANGED__USB_SUSPEND_OFF,
	SYSTEM_STATUS_CHANGED__FTM_SWITCH_ON_CHRGING,
	SYSTEM_STATUS_CHANGED__FTM_SWITCH_OFF_CHRGING,
	SYSTEM_STATUS_CHANGED__HDMI_VALID,
	SYSTEM_STATUS_CHANGED__ACCESSORY,
	/* irq mpp11 event */
	IRQ_TRIGGERED__MPP11,
};

typedef enum   
{
    /*! Battery is cold               */
    CV_BATTERY_TEMP_REGION__COLD,
    /*! Battery is little cold               */
    CV_BATTERY_TEMP_REGION_LITTLE__COLD,
    /*! Battery is cool               */
    CV_BATTERY_TEMP_REGION__COOL,
    /*! Battery is normal             */
    CV_BATTERY_TEMP_REGION__NORMAL,
    /*! Battery is warm               */
    CV_BATTERY_TEMP_REGION__WARM,
    /*! Battery is hot                */
    CV_BATTERY_TEMP_REGION__HOT,
    /*! Invalid battery temp region   */
    CV_BATTERY_TEMP_REGION__INVALID,
}chg_cv_battery_temp_region_type;

/*! \enum chg_charger_status_type
 *  \brief This enum contains defintions of the charger hardware status
 */
typedef enum
{
	/* The charger is good      */
	CHARGER_STATUS_GOOD,
	/* The charger is bad       */
	CHARGER_STATUS_BAD,
	/* The charger is weak      */
	CHARGER_STATUS_WEAK,
	/* Invalid charger status.  */
	CHARGER_STATUS_INVALID
}chg_charger_status_type;

/*! \enum chg_mhl_type
 *  \brief This enum contains defintions of the chg mhl type
 */
typedef enum
{
   /*chg mhl none*/
   CHG_MHL_NONE,
   /*chg mhl nonstanard,input current is 500mA*/
   CHG_MHL_NONSTANARD,
   /*chg mhl nonstanard,input current is 700mA*/
   CHG_MHL_STANARD,
}chg_mhl_type;

 
/*
 *  This enum contains defintions of the battery status
 */
typedef enum
{
	/* The battery is good        */
	BATTERY_STATUS_GOOD,
	/* The battery is cold/hot    */
	BATTERY_STATUS_BAD_TEMP,
	/* The battery is bad         */
	BATTERY_STATUS_BAD,
	/* The battery is removed     */
	BATTERY_STATUS_REMOVED,		/* on v2.2 only */
	BATTERY_STATUS_INVALID_v1 = BATTERY_STATUS_REMOVED,
	/* Invalid battery status.    */
	BATTERY_STATUS_INVALID
}chg_battery_status_type;

/*
 * This enum contains defintions of the charger hardware status
 */
typedef enum{
	/* The charger is removed                 */
	CHARGER_TYPE_NONE,
	/* The charger is a regular wall charger   */
	CHARGER_TYPE_WALL,
	/* The charger is a PC USB                 */
	CHARGER_TYPE_USB_PC,
	/* The charger is a wall USB charger       */
	CHARGER_TYPE_USB_WALL,
	/* The charger is a USB carkit             */
	CHARGER_TYPE_USB_CARKIT,
	/* The charger is a non standard             */
	CHARGER_TYPE_NON_STANDARD,
	/*The charger is HMDI*/
	CHARGER_TYPE_USB_HDMI,
	/* Invalid charger hardware status.        */
	CHARGER_TYPE_INVALID,
	/* The charger is a accessory             */
	CHARGER_TYPE_ACCESSORY
}chg_charger_hardware_type;

/*
 *This enum contains defintions of the battery voltage level
 */
typedef enum{
	/* The battery voltage is dead/very low (less than 3.2V) */
	BATTERY_LEVEL_DEAD,
	/* The battery voltage is weak/low (between 3.2V and 3.4V) */
	BATTERY_LEVEL_WEAK,
	/* The battery voltage is good/normal(between 3.4V and 4.2V) */
	BATTERY_LEVEL_GOOD,
	/* The battery voltage is up to full (close to 4.2V) */
	BATTERY_LEVEL_FULL,
	/* Invalid battery voltage level. */
	BATTERY_LEVEL_INVALID
}chg_battery_level_type;

struct smb346_charger_info_type {
	u32 batt_status;
	u32 batt_health;
	//u32 batt_valid;
	bool batt_valid;
	chg_charger_status_type charger_status;
	chg_charger_hardware_type charger_type;
	chg_battery_status_type battery_status;
	chg_battery_level_type battery_level;
    int aicl_result;
	bool is_adapt_completed;
};
extern struct smb346_charger_info_type smb346_charger_info;

struct smb346_battery_adc_params_type{
    int current_batt_adc_voltage ; 
    int  batt_temperature ;
    int  vchg_mv;
    int  ichg_mv;
	int  batt_capacity;
};

struct smb346_battery_gauge {
	void (*qury_adc_params) (struct smb346_battery_adc_params_type* smb346_battery_adc_params);
	void (*update_psy_status)(struct smb346_charger_info_type *smb346_charger_info, struct smb346_battery_adc_params_type *smb346_battery_adc_params);
};

struct oppo_battery_fuelgauge {
	int (*get_battery_mvolts) (void);
	int (*get_battery_temperature) (void);
	int (*get_vchg_mvolts) (void);
	int (*get_ichg_current) (void);
	int (*get_battery_capacity) (void);
	int (*set_full_capacity) (void);
};

#define AC_CHG     0x00000001
#define USB_CHG    0x00000002
struct smb346_battery_platform_data 
{
	#ifdef CONFIG_OPPO_MODIFY
	//add by tanglc 2011-11-19 for current down
	void (*ldo_power) (int on);
	void (*ldo_init) (void);
	void (*hsusb_enable) (int on);
	/*uart switch feature*/
	void (*hsuart_enable) (int on);
	/*uart switch feature*/
	u32 voltage_max_design;
	u32 voltage_min_design;
	u32 avail_chg_sources;
	u32 batt_technology;
	u32 (*calculate_capacity)(u32 voltage);
	#endif
};
bool is_show_batt_log(void);
bool is_show_gague_log(void);
void smb346_battery_gauge_register(struct smb346_battery_gauge *batt_gauge);
void smb346_battery_gauge_unregister(struct smb346_battery_gauge *batt_gauge);
void msm_charger_vbus_draw(unsigned int mA);
void smb346_chg_connected(enum chg_type chgtype);
void smb346_battery_fuelgauge_register(struct oppo_battery_fuelgauge *fuelgauge);
void smb346_battery_fuelgauge_unregister(struct oppo_battery_fuelgauge *fuelgauge);
int get_prop_vchg_movlts(void);
int smb346_mpp11_irq_register(void);
int smb346_vchg_mvolts_update(void);
int smb346_hsusb_enable(int on);
int smb346_mhl_stanard_charge(void);
int updata_batt_params_fulegague_resume(void);

/*end by chendx 2011-09-22*/

//end by chendx
#endif /* __SMB346-CHARGER_H__ */
