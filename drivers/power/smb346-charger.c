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
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/mfd/pmic8058.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/workqueue.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/msm_adc.h>
#include <linux/notifier.h>
#ifdef CONFIG_TOUCHSCREEN_ATMEL_QT602240E
#include <linux/qt602240_ts.h>
#endif

#include <mach/msm_xo.h>
#include <mach/msm_hsusb.h>

#include <linux/gpio.h>
#include <mach/mpp.h>
#include <linux/pmic8058-xoadc.h>
#include <linux/power_supply.h>
#include <linux/input.h>
#include <linux/smb346-charger.h>
#include <linux/smb346_reg.h>
#include <linux/reboot.h>
#include <linux/types.h>
#include <linux/mfd/pm8xxx/core.h>
#include <linux/mfd/pmic8058.h>
#include <linux/mfd/pm8xxx/batt-alarm.h>


#include <linux/oppo_i2c/oppo_gpio_i2c.h>
#ifndef CONFIG_GPIOLIB
#include "gpio_chip.h"
#endif
#include <mach/board-msm8660.h>

#include <linux/pmic8058-othc.h>


//use to check DC or USB charger plugin
#define MPP11_IRQ_ENABLE_FEATURE

//open charge dump log feature
#define DUMP_CHG_LOG_FEATURE

//open  dcin charging feature
#define DCIN_CHARGE_FEATURE


/*  this is for log debug iuse */
static bool show_chg_log = false;
static bool show_batt_log = false;
static bool show_gague_log = false;

#define DEBUG_CHG(fmt, ...) \
	({ if (show_chg_log) printk(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__); 0; })
	
#define ERR_CHG(fmt, ...) printk(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__)
#define chg_info(fmt, ...) printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)


//ad5246 i2c 7bits address 0x2e 
#define SMB346_I2C_ADDRESS 0x4d
/*add by chendx 2011-11-24 for battery low suspend shutdown */
#define VOL_LOW_RESUME_MV 3400 //voltage low resume vol
#define VOL_HIGH_RESUME_MV 4300 

struct input_dev  *smb346_input;

struct completion stanard_mhl_wait;

struct smb346_charger {
	//struct smb346_platform_pdata *pdata;
	struct pm8058_chip *pm_chip;
	struct device *dev;
	/*SMB346 abnormity irq*/
	int smb346_irq;
	#ifdef MPP11_IRQ_ENABLE_FEATURE
	int mpp11_irq;
	#endif
	struct mutex smb346_lock;
};

//#define INTERVAL_CHARGING_FEATURE

static struct smb346_charger smb346_chg;
 /*end by chendx 2011-11-24 for battery low suspend shutdown */

/* bellow this is concern with temperature iuse */
#define BATTREMOVETEMP                  -400 //-40C

//battery temperate get from gague precison 0.1C
#define BATTERY_TEN 10 
#define AUTO_CHARGING_BATT_TEMP_T0                           -100 //-10C
#define AUTO_CHARGING_BATT_TEMP_T1                            0     //0C
#define AUTO_CHARGING_BATT_TEMP_T2                           100  //10C
#define AUTO_CHARGING_BATT_TEMP_T3                            450  //45C
#define AUTO_CHARGING_BATT_TEMP_T4                            550  //55C

#define AUTO_CHARGING_BATTERY_TEMP_HYST_FROM_HOT_TO_WARM      30
#define AUTO_CHARGING_BATTERY_TEMP_HYST_FROM_WARM_TO_NORMAL   10
#define AUTO_CHARGING_BATTERY_TEMP_HYST_FROM_COOL_TO_NORMAL   10
#define AUTO_CHARGING_BATTERY_TEMP_HYST_FROM_COLD_TO_COOL     30

#define MAX_CHARGER_CHECK_COUNT			3

/* bellow this is concern with charging timer iuse */
#define MSM_CHG_MAX_EVENTS 16
#define CHARGE_ENABLE_DELAY 20000000 //20ms
#define CHARGING_TEOC_S 36000				//10 hours
#define UPDATE_TIME_S 5				//5s
#define CC_DISCHG_DETECT_TIME_NS 500000000	//0.5s
#define END_CHG_DETECT_S 2//10				//10s
#define RESUME_CHECK_PERIOD_S 60			//60s
#define INTERVAL_CHG_TIMER_MAX_COUNT 5		//total 20s
#define CALLING_TIMER_CHECK_PERIOID 1// 1S

/*ftm charge suspend mode timer */
#define FTM_CHARGER_MODE_TIMER 20 //20s

/* software control charger OVP voltage */  
#define CHARGER_SOFT_OVP_VOLTAGE		5700//5800
/* software control charger UVP voltage */  
#define CHARGER_SOFT_UVP_VOLTAGE		4300 //4500
/* the voltage to lower charger current to prevent OVP event. */ 
#define CHARGER_SOFT_HVP_VOLTAGE		5600
/* battery OVP voltage */  
#define BATTERY_SOFT_OVP_VOLTAGE		4500  
/* bad charger or battery count */
#define BAD_STATE_COUNT		3

/* software control battery voltage */
#define SOFT_CONTROL_THREHOLD 3000
/* software control synchronize battery voltage */
#define SOFT_CONTROL_THREHOLD_HYST 50

/* fast charging interval battery voltage */
#define MIISS_BATTTERY_VOLTAGE 3400
/* suspend charging meet battery voltage */
#define SUSPEND_MEET_THREHOLD 3500
/* tapper charging interval battery voltage */
#define TAPPER_INTERVAL_CHG_VOLT 4175
/* cool warm tapper charging interval battery voltage */
#define TAPPER_INTERVAL_CHG_WARM_VOLT 4075
/* little cold tapper charging interval battery voltage */
#define TAPPER_INTERVAL_CHG_LITTLE_COLD_VOLT 3975

#ifndef FEATURE_AUTO_RECHARGE
#define DEFAULT_BATT_MAX_V	4200
#define DEFAULT_BATT_MIN_V	3200
#define DEFAULT_BATT_RESUME_V	4100

#define MSM_CHARGER_RESUME_COUNT 5

#define AUTO_CHARGING_RESUME_CHARG_VOLT	4100
#define AUTO_CHARGING_RESUME_CHARG_VOLT_WARM	3900
#define AUTO_CHARGING_RESUME_CHARG_VOLT_LITTLECOLD 3800
#define AUTO_CHARGING_RESUME_CHARG_VOLT_COOL	3900
#endif

#define MSM_CHARGER_GAUGE_MISSING_VOLTS 3800
#define MSM_CHARGER_GAUGE_MISSING_TEMP  35
#define MSM_CHARGER_DEFAULT_VCHG 5000
#define MSM_CHARGER_DEFAULT_ICHG 0

/* this is for set charging current iuse */
#define SMB346_CC_SETP_MAX 8
#define FEATURE_SMB_COOL_TEMP_CONTROL 1
#define FEATURE_SMB_HOT_TEMP_CONTROL   0

typedef enum{
	FAST_CHG_350MA,
	FAST_CHG_450MA,
	FAST_CHG_600MA,
	FAST_CHG_750MA,
	FAST_CHG_900MA,
	FAST_CHG_1000MA,
	FAST_CHG_1100MA,
	FAST_CHG_1250MA,
}smb346_cc_step_type;

typedef enum{
	SMB_STOP_CHG_REASON__NONE,
	SMB_STOP_CHG_REASON__BATTERY_OVP,
	SMB_STOP_CHG_REASON__BATTERY_TEMP_COLD,
	SMB_STOP_CHG_REASON__BATTERY_TEMP_HOT,
	SMB_STOP_CHG_REASON__BATTERY_REMOVED,
	SMB_STOP_CHG_REASON__CHARGER_OVP,
	SMB_STOP_CHG_REASON__CHARGER_UVP,
	SMB_STOP_CHG_REASON__CHARGER_AICL
}smb346_stop_chg_reason_type;

typedef enum{
	SMB_CHG_MODE__NOT_CHARGING,
	SMB_CHG_MODE__PRE_CHARGING,
	SMB_CHG_MODE__FAST_CHARGING,
	SMB_CHG_MODE__TAPPER_CHARGING
}smb346_charging_mode_type;

typedef enum{
	SMB_CHG_STATUS__INVALID,
	SMB_CHG_STATUS__NOT_CHARGING,
	SMB_CHG_STATUS__CHARGING,
	SMB_CHG_STATUS__CHARGING_COMPLETED
}smb346_charging_status_type;

//huyu
typedef enum{
	ACCESSORY__CHG_STATUS__INVALID,	
	ACCESSORY__CHG_STATUS__CHARGING,
	ACCESSORY__CHG_STATUS__DISCHARGING,
	ACCESSORY__CHG_STATUS__NOT_CHARGING,
	ACCESSORY__CHG_STATUS__CHARGING_COMPLETED
}charging_status_type;

static char regval_cc_step[8] = {
	FAST_CHARGE_CURRENT_350MA,
	FAST_CHARGE_CURRENT_450MA,
	FAST_CHARGE_CURRENT_600MA,
	FAST_CHARGE_CURRENT_750MA,
	FAST_CHARGE_CURRENT_900MA,
	FAST_CHARGE_CURRENT_1000MA,
	FAST_CHARGE_CURRENT_1100MA,
	FAST_CHARGE_CURRENT_1250MA
};

/* charge thread event type */
struct msm_charger_event {
	enum smb346_hardware_charger_event event;
};

/* charge value */
struct smb346_charger_mux {
	struct smb346_charger  *smb346_chg;
	/* charge relative timer */
	struct hrtimer charge_enable_delay_timer;
	struct hrtimer update_heartbeat_timer;
	#ifdef INTERVAL_CHARGING_FEATURE
	struct hrtimer interval_charge_timer;
	int interval_chg_timer_count;
	#endif
	struct hrtimer max_charge_time_timer;
	struct hrtimer end_charge_detect_timer;
	struct hrtimer charge_resume_timer;
	struct hrtimer ftm_charge_suspend_timer;
	struct hrtimer calling_timer;
	/* charge hardware relative value */
	chg_charger_hardware_type charger_type;

	/* charge setting relative value */
	smb346_cc_step_type cc_step_curr;
	uint8_t aicl_set_val;
	uint8_t flt_set_val;
	smb346_charging_status_type charging_status;
	/* charge phase relative value */
	//bool mIsChargingComplete;
	bool mIsSuspendOn;

	/* battery status relative value */
	bool battery_valid;

	int smb346_i2c_address;

	/* software control relative value */
	bool is_interval_chg;
	unsigned int soft_control_threhold;

	/* battery temp relative value */
	chg_cv_battery_temp_region_type mBatteryTempRegion;
	
	short mBatteryTempBoundT0;
	short mBatteryTempBoundT1;
	short mBatteryTempBoundT2;
	short mBatteryTempBoundT3;
	short mBatteryTempBoundT4;

	/* bad state relative value */
	smb346_stop_chg_reason_type stop_charging_reason;
	bool StopChgDueToBadChagerState;
	bool StopChgDueToBadBatteryState;
	bool StopChgDueToBadTempState;
	int mBadChargerCounter;
	int mChargerCheckCounter;
	int mBadBatteryCounter;
	int mAICLCounter;

	/* counter for adjust current when enter tapper charging mode */
	int mTapperCounter;

	/* charge complete detection relative value */
	bool charging_completed_detect_enabled;
	int mEndCVTopoffCounter;

	/* teoc work relative value */
	struct delayed_work teoc_work;
	
	#ifndef FEATURE_AUTO_RECHARGE
	unsigned int max_voltage;
	unsigned int min_voltage;
	unsigned int resume_voltage;	
	/* resume work relative value */
	bool stop_resume_check;
	int resume_count;
	#endif

	#ifdef CONFIG_HAS_WAKELOCK
	struct wake_lock smb346_wake_lock;
	#endif
	
	/* msm charger work queue thread relative value */
	struct msm_charger_event *queue;
	int tail;
	int head;
	spinlock_t queue_lock;
	int queue_count;
	struct work_struct queue_work;
	struct workqueue_struct *event_wq_thread;
};

/*usb msm charger,it will init in probe function*/
static struct smb346_charger_mux *usb_msm_chg = NULL;

/* global value */
static bool chg_usb_OnOff_nv_item_val = true;
static bool chg_usb_suspend_mode_on = false;

/*MHL chg type*/
static chg_mhl_type mhl_chg_type = CHG_MHL_NONE;


static struct smb346_battery_gauge *msm_batt_gauge = NULL;
struct smb346_charger_info_type smb346_charger_info;
static int smb346_stop__charging(struct smb346_charger_mux *msm_chg);

#ifdef CONFIG_ACCESSORY_BATTERY
extern int usb_chg_charging_complete(void);
#endif

extern int usb_charger_connect(int on);
extern int accessory_is_wirelees_present(void);
extern int set_origin_main_capacity(void);
//fuelgague function
extern bool fuelgague_battery_is_present(void);

//huyu
extern charging_status_type accessory_charging_status_get(void);
extern int get_accessory_present_status(void);

#define BATTERY_POWER_ON_VOLTAGE_CALIBARATION_COUNT 5
#define TAPPER_CHG_MODE_SOFT_DETECT_COUNT 3//6
#define dump_log_timer 24;//120s




//FTM charge control 
#define FTM_CHARGE_MODE_FEATURE
#ifdef FTM_CHARGE_MODE_FEATURE
#define FTM_LOW_CAPACITY_LEVEL 60
#define FTM_HIGH_CAPACITY_LEVEL 80

#define BATTERY_DEFAULT_MVOLTS 4500
#define USBIN_VALID_MVOLTS_MAX 6200
#define USBIN_VALID_MVOLTS_MIN 3000 
#define MAIN_BATTERY_LEVEL 50
#define CHARGE_TERMINATION_CURR 100
#define CHARGE_END_MAX_COUNT 5
#define CHARGE_END_COUNT 3



static bool low_charge_mode = false; //60% stop charge
static bool high_charge_mode = false; //80% stop charge
#endif
//FTM charge control 

static struct smb346_battery_adc_params_type smb346_battery_adc_params;

/*add by chendx 2011-11-24 for battery low suspend shutdown */
static int smb346_battery_gague_alarm_notify(struct notifier_block *nb,
					  unsigned long status, void *unused);

static struct notifier_block alarm_notifier = {
	.notifier_call = smb346_battery_gague_alarm_notify,
};

#define SYSTEM_WAKE_UP_WARNING_CAPACITY_LEVEL 1
#define SYSTEM_WAKE_UP_SHUTDOWN_CAPACITY_LEVEL 0
static int smb346_battery_gague_alarm_notify(struct notifier_block *nb,
		unsigned long status, void *unused)			
{

    int rc;
	static int battery_capacity_old=0;
	
	pr_info("Notify system resume when voltage low than 3400mv, status: %lu,%d%%\n", status,smb346_battery_adc_params.batt_capacity);

	switch (status) {
	case 0:
		dev_err(smb346_chg.dev,
			"%s: spurious interrupt\n", __func__);
		break;
	/* expected case - trip of low threshold */
	case 1:
		rc = pm8xxx_batt_alarm_disable(
				PM8XXX_BATT_ALARM_UPPER_COMPARATOR);
		if (!rc)
			rc = pm8xxx_batt_alarm_disable(
				PM8XXX_BATT_ALARM_LOWER_COMPARATOR);
		if (rc)
			dev_err(smb346_chg.dev,
				"%s: unable to set alarm state\n", __func__);

		if(smb346_battery_adc_params.batt_capacity == SYSTEM_WAKE_UP_WARNING_CAPACITY_LEVEL){
			//allow wake up system when battery capacity low than 1%
			if(battery_capacity_old != smb346_battery_adc_params.batt_capacity){
				pr_info("%s:resume capacity old:%d%%,%d%%\n",
					__func__,battery_capacity_old,smb346_battery_adc_params.batt_capacity);
				battery_capacity_old = smb346_battery_adc_params.batt_capacity;
				
				input_report_key(smb346_input,KEY_POWER,1);
				input_sync(smb346_input);	 

				/**
				*  release Simulate KEY_POWER
				*/
				input_report_key(smb346_input,KEY_POWER,0);
				input_sync(smb346_input);
			}
		}else if(smb346_battery_adc_params.batt_capacity == SYSTEM_WAKE_UP_SHUTDOWN_CAPACITY_LEVEL){
				input_report_key(smb346_input,KEY_POWER,1);
				input_sync(smb346_input);	 

				/**
				*  release Simulate KEY_POWER
				*/
				input_report_key(smb346_input,KEY_POWER,0);
				input_sync(smb346_input);
		}
		break;
	case 2:
		dev_err(smb346_chg.dev,
			"%s: trip of high threshold\n", __func__);
		break;
	default:
		dev_err(smb346_chg.dev,
			"%s: error received\n", __func__);
	};

	return 0;
}
 /*end by chendx 2011-11-24 for battery low suspend shutdown */
 
#define STOP_CHG_BATT_VOL_WHILE_CALLING_MV 4050//MV
static smb346_charging_status_type chg_status_bak_for_calling = SMB_CHG_STATUS__NOT_CHARGING;

#ifdef MPP11_IRQ_ENABLE_FEATURE
#define BOOTING_CON	4 // 20s
static u32 boot_time = BOOTING_CON;

#define EXT_BQ27541_CHG_VALID_MPP 10
#define EXT_BQ27541_CHG_VALID_MPP_2 11
#define MPP11_VALID_GPIO PM8058_MPP_PM_TO_SYS(EXT_BQ27541_CHG_VALID_MPP)

 static struct pm8xxx_mpp_config_data smb346_mpp_10 = 
{
    .type = PM_MPP_TYPE_D_INPUT,
	.level = PM8058_MPP_DIG_LEVEL_S3,
	.control = PM_MPP_DIN_TO_INT,
};
static struct pm8xxx_mpp_config_data smb346_mpp_11 = 
{
    .type = PM_MPP_TYPE_D_BI_DIR,
	.level = PM8058_MPP_DIG_LEVEL_S3,
	.control = PM_MPP_BI_PULLUP_10KOHM,
};
static int smb346_detection_setup(void)
 {
	 int ret = 0;
	 ret = pm8xxx_mpp_config(PM8058_MPP_PM_TO_SYS(EXT_BQ27541_CHG_VALID_MPP),&smb346_mpp_10);
	 if (ret) {
		 pr_err("%s config EXT_smb346_CHG_VALID_MPP failed ret=%d\n",__func__, ret);
		 return ret;
	 }
		 
	 ret = pm8xxx_mpp_config(PM8058_MPP_PM_TO_SYS(EXT_BQ27541_CHG_VALID_MPP_2),&smb346_mpp_11);
	 if (ret) {
		 pr_err("%s config EXT_smb346_CHG_VALID_MPP_2  failed ret=%d\n",__func__, ret);
		 return ret;
	 }
 
	 return 0;
 }
 
 static int is_chg_plugged_in(void)
 {
	 int value;
	 /*DC or USB cable plugin MPP11 level LOW*/
	 value = gpio_get_value_cansleep(MPP11_VALID_GPIO);  
	 if(value < 0){
		  dev_err(smb346_chg.dev,"%s get value error value =%d\n",__func__,value);
		  return 0;
	 }
	 value =!value;
	 return value;
 }

#endif
//this section place some helpfull funcs
static bool smb346_aicl_adapt_completed_get(struct smb346_charger_mux *msm_chg)
{
	return	smb346_charger_info.is_adapt_completed;
}
static void smb346_aicl_adapt_completed_set(struct smb346_charger_mux *msm_chg, bool is_aicl_adapt_completed)
{
	smb346_charger_info.is_adapt_completed = is_aicl_adapt_completed;
}
static bool smb346_battery_valid_get(struct smb346_charger_mux *msm_chg)
{
	return	smb346_charger_info.batt_valid;//or return msm_chg->battery_valid;
}
static void smb346_battery_valid_set(struct smb346_charger_mux *msm_chg, bool is_valid)
{
	smb346_charger_info.batt_valid = is_valid;
	msm_chg->battery_valid = is_valid;
}
#if 0//not used yet
static chg_battery_level_type smb346_battery_level_get(struct smb346_charger_mux *msm_chg)
{
	return	smb346_charger_info.battery_level;
}
#endif
static void smb346_battery_level_set(struct smb346_charger_mux *msm_chg, chg_battery_level_type battery_level)
{
	smb346_charger_info.battery_level = battery_level;
}
static chg_battery_status_type smb346_battery_status_get(struct smb346_charger_mux *msm_chg)
{
	return smb346_charger_info.battery_status;
}
static void smb346_battery_status_set(struct smb346_charger_mux *msm_chg, chg_battery_status_type battery_status)
{
	smb346_charger_info.battery_status = battery_status;
}
#if 0//not used yet
static int smb346_battery_health_get(struct smb346_charger_mux *msm_chg)
{
	return	smb346_charger_info.batt_health;
}
#endif
static void smb346_battery_health_set(struct smb346_charger_mux *msm_chg, int batt_health)
{
	smb346_charger_info.batt_health = batt_health;
}
static chg_cv_battery_temp_region_type smb346_battery_temp_region_get(struct smb346_charger_mux *msm_chg)
{
	return msm_chg->mBatteryTempRegion;
}
static void smb346_battery_temp_region_set(struct smb346_charger_mux *msm_chg, chg_cv_battery_temp_region_type batt_temp_region)
{
	msm_chg->mBatteryTempRegion = batt_temp_region;
}
static smb346_charging_status_type smb346_charging_status_get(struct smb346_charger_mux *msm_chg)
{
	return msm_chg->charging_status;
}
static void smb346_charging_status_set(struct smb346_charger_mux *msm_chg, smb346_charging_status_type charging_status)
{
	msm_chg->charging_status = charging_status;
}
//for other files use.
bool get_charging_status(void)
{
	if (usb_msm_chg == NULL){
		pr_err("%s: usb_msm_chg is NULL\n",__func__);
		return false;
	}
	
	if (smb346_charging_status_get(usb_msm_chg) == SMB_CHG_STATUS__CHARGING || 
		smb346_charging_status_get(usb_msm_chg) == SMB_CHG_STATUS__CHARGING_COMPLETED)
		return true;
	else
		return false;
}

static int smb346_charger_relative_battery_status_get(struct smb346_charger_mux *msm_chg)
{
	return	smb346_charger_info.batt_status;
}


static void smb346_charger_relative_battery_status_set(struct smb346_charger_mux *msm_chg, int batt_status)
{
	smb346_charger_info.batt_status = batt_status;
}

static chg_charger_status_type smb346_charger_status_get(struct smb346_charger_mux *msm_chg)
{
	return	smb346_charger_info.charger_status;
}

static void smb346_charger_status_set(struct smb346_charger_mux *msm_chg, chg_charger_status_type charger_status)
{
	smb346_charger_info.charger_status = charger_status;
}

static chg_charger_hardware_type smb346_charger_type_get(struct smb346_charger_mux *msm_chg)
{
	return	smb346_charger_info.charger_type;//or return msm_chg->charger_type;
}

static void smb346_charger_type_set(struct smb346_charger_mux *msm_chg, chg_charger_hardware_type charger_type)
{
	smb346_charger_info.charger_type = charger_type;
	msm_chg->charger_type = charger_type;
}

chg_charger_hardware_type get_charger_type(void)
{
	return smb346_charger_type_get(usb_msm_chg);
}


//for other files use.
/**
*  USB charger status
*/
bool get_usb_chg_status(void)
{
    chg_charger_hardware_type charger_type = CHARGER_TYPE_INVALID ;
	
	charger_type = get_charger_type();
	//pr_info("%s: charger_type =%d\n",__func__,charger_type);
	if(charger_type == CHARGER_TYPE_USB_WALL ||
		charger_type == CHARGER_TYPE_USB_PC ||
		charger_type == CHARGER_TYPE_NON_STANDARD ||
		charger_type == CHARGER_TYPE_USB_HDMI)
		return true;
	else
		return false;
}
EXPORT_SYMBOL(get_usb_chg_status);

int get_main_battery_capacity(void)
{
	return smb346_battery_adc_params.batt_capacity;

}
EXPORT_SYMBOL(get_main_battery_capacity);

int get_main_battery_temperature(void)
{
	return smb346_battery_adc_params.batt_temperature;

}
EXPORT_SYMBOL(get_main_battery_temperature);


bool get_mains_battery_charge_status(void)
{
	smb346_charging_status_type mains_battery_charge_status = SMB_CHG_STATUS__NOT_CHARGING;

	mains_battery_charge_status = smb346_charging_status_get(usb_msm_chg);
	pr_info("%s %d\n", __func__, mains_battery_charge_status);

	//wangjc
	if(mains_battery_charge_status == SMB_CHG_STATUS__CHARGING_COMPLETED)
	//if(smb346_battery_adc_params.batt_capacity == ACCESSORY_CHG_COMPELETE_LEVEL)
		return true;
	else
	    return false;
}
EXPORT_SYMBOL(get_mains_battery_charge_status);
#if 0// wil be used in the future
static bool smb346_charging_stopped_for_charger_bad_state_get(struct smb346_charger_mux *msm_chg)
{
	return msm_chg->StopChgDueToBadChagerState;
}
static bool smb346_charging_stopped_for_battery_bad_temp_get(struct smb346_charger_mux *msm_chg)
{
	return msm_chg->StopChgDueToBadTempState;
}
static bool smb346_charging_stopped_for_battery_bad_state_get(struct smb346_charger_mux *msm_chg)
{
	return msm_chg->StopChgDueToBadBatteryState;
}
#endif

static void smb346_charging_stopped_for_charger_bad_state_set(struct smb346_charger_mux *msm_chg, bool value)
{
	msm_chg->StopChgDueToBadChagerState = value;
}

static void smb346_charging_stopped_for_battery_bad_temp_set(struct smb346_charger_mux *msm_chg, bool value)
{
	msm_chg->StopChgDueToBadTempState = value;
}

static void smb346_charging_stopped_for_battery_bad_state_set(struct smb346_charger_mux *msm_chg, bool value)
{
	msm_chg->StopChgDueToBadBatteryState = value;
}

#if 0//this part will be used in the future
static bool smb346_is_charging_stopped_for_battery_temp_cold(struct smb346_charger_mux *msm_chg)
{
	return msm_chg->stop_charging_reason & SMB_STOP_CHG_REASON__BATTERY_TEMP_COLD ? true: false;
}
static bool smb346_is_charging_stopped_for_battery_temp_hot(struct smb346_charger_mux *msm_chg)
{
	return msm_chg->stop_charging_reason & SMB_STOP_CHG_REASON__BATTERY_TEMP_HOT ? true: false;
}
static bool smb346_is_charging_stopped_for_battery_ovp(struct smb346_charger_mux *msm_chg)
{
	return msm_chg->stop_charging_reason & SMB_STOP_CHG_REASON__BATTERY_OVP ? true: false;
}
static bool smb346_is_charging_stopped_for_charger_ovp(struct smb346_charger_mux *msm_chg)
{
	return msm_chg->stop_charging_reason & SMB_STOP_CHG_REASON__CHARGER_OVP ? true: false;
}
static bool smb346_is_charging_stopped_for_charger_uvp(struct smb346_charger_mux *msm_chg)
{
	return msm_chg->stop_charging_reason & SMB_STOP_CHG_REASON__CHARGER_UVP ? true: false;
}
static bool smb346_is_charging_stopped_for_aicl(struct smb346_charger_mux *msm_chg)
{
	return msm_chg->stop_charging_reason & SMB_STOP_CHG_REASON__CHARGER_AICL ? true: false;
}

#endif

#if 0
static void smb346_charger_set_onoff_nv(struct smb346_charger_mux *msm_chg, bool is_suspend)
{
	chg_usb_OnOff_nv_item_val = !is_suspend;
}
#endif

static bool smb346_charger_suspend_mode_get(struct smb346_charger_mux *msm_chg)
{
	return chg_usb_suspend_mode_on;
}

static void smb346_charger_suspend_mode_set(struct smb346_charger_mux *msm_chg, bool is_suspend)
{
	chg_usb_suspend_mode_on = is_suspend;
}

static bool smb346_charging_completed_detect_enabled_get(struct smb346_charger_mux *msm_chg)
{
	return msm_chg->charging_completed_detect_enabled;
}

static void smb346_charging_completed_detect_enabled_set(struct smb346_charger_mux *msm_chg, bool is_enabled)
{
	msm_chg->charging_completed_detect_enabled = is_enabled;
}

/**
*   Reset charger info to init status
*/
static void smb346_charger_info_init(struct smb346_charger_mux *msm_chg)
{
	smb346_charger_relative_battery_status_set(msm_chg, POWER_SUPPLY_STATUS_NOT_CHARGING);
	smb346_battery_health_set(msm_chg, POWER_SUPPLY_HEALTH_GOOD);
	smb346_battery_valid_set(msm_chg, true);
	smb346_charger_status_set(msm_chg, CHARGER_STATUS_INVALID);
	//smb346_charger_type_set(msm_chg, CHARGER_TYPE_INVALID);
	smb346_battery_status_set(msm_chg, BATTERY_STATUS_GOOD);
	smb346_battery_level_set(msm_chg, BATTERY_LEVEL_GOOD);
	smb346_aicl_adapt_completed_set(msm_chg, false);
}

static void smb346_battery_adc_params_init(struct smb346_charger_mux *msm_chg)
{
	smb346_battery_adc_params.current_batt_adc_voltage = MSM_CHARGER_GAUGE_MISSING_VOLTS;
	smb346_battery_adc_params.batt_temperature = MSM_CHARGER_GAUGE_MISSING_TEMP;
	smb346_battery_adc_params.vchg_mv = MSM_CHARGER_DEFAULT_VCHG;
	smb346_battery_adc_params.ichg_mv = MSM_CHARGER_DEFAULT_ICHG;
}

//===========================

static int msm_charger_notify_event(struct smb346_charger_mux *msm_chg, enum smb346_hardware_charger_event event);
static void smb346_ftm_charge_suspend_mode(bool on);

static int smb346_read(struct smb346_charger_mux *msm_chg, BYTE reg, BYTE *val)
{
      int i=0;
      bool ret=true;

      for(i=0;i<3;i++){	  
      		ret = gpio_i2c0_readData(msm_chg->smb346_i2c_address,reg,0x01,val); 
		if(ret)//read data successfully
			return 0;
      }
     
	pr_err("%s: i2c0 read reg failed!\n",__func__);
	return -1;
}

static int smb346_write(struct smb346_charger_mux *msm_chg, BYTE reg, BYTE val)
{
      int i=0;
      bool ret=true;

      for(i=0;i<3;i++){	  
      		ret = gpio_i2c0_writeData(msm_chg->smb346_i2c_address,reg,0x01,&val); 
		if(ret)//read data successfully
			return 0;
      }
	pr_err("%s: i2c0 write reg failed!\n",__func__);
	return 0;
}

static int smb346_reads(struct smb346_charger_mux *msm_chg, BYTE reg, int len, BYTE *val)
{
	int i=0;
	bool ret=true;

	for(i=0;i<3;i++){	  
		ret = gpio_i2c0_readData(msm_chg->smb346_i2c_address,reg,len,val); 
		if(ret)//read data successfully
			return 0;
	}

	pr_err("%s: i2c0 reads regs failed!\n",__func__);
	return -1;
}

/*static int smb346_writes(struct smb346_charger_mux *msm_chg, int reg, int len, uint8_t *val)
{
	return __smb346_writes(msm_chg->smb346_i2c_address, reg, len, val);
}*/

//smb346 charging register value update
/*reg :register value
   val:  value
   mask: update val bits
*/
static int smb346_update(struct smb346_charger_mux *msm_chg, UINT16 reg, BYTE val, uint8_t mask)
{
	BYTE reg_val=0;
	int ret = 0;

	ret = smb346_read(msm_chg, reg, &reg_val);
	if (ret){
		pr_err("%s: smb346 read failed!\n",__func__);
		return ret;
	}

	DEBUG_CHG("%s before reg = 0x%x reg_val = 0x%x\n", __func__, reg, reg_val);

	if ((reg_val & mask) != val) {
		reg_val = (reg_val & ~mask) | val;
		ret = smb346_write(msm_chg, reg, reg_val);
		if (ret){
			pr_err("%s: smb346 write failed!\n",__func__);
			return ret;
		}
	}

	/* this is for debug read */
	if (show_chg_log) {
		ret = smb346_read(msm_chg, reg, &reg_val);
		DEBUG_CHG("%s after reg = 0x%x reg_val = 0x%x\n", __func__, reg, reg_val);
	}

	return ret;
}

static int smb346_reg_init_config(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	DEBUG_CHG("%s\n", __func__);
	
	/* allow volatile write to config register */
	rc = smb346_update(msm_chg, COMMAND_REGISTER_A, ALLOW_VOLATILE_WRITES_TO_CONFIG_REGISTERS, 
		ALLOW_VOLATILE_WRITES_TO_CONFIG_REGISTERS);
	if (rc){
		pr_err("%s:smb346 update COMMAND_REGISTER_A register failed!\n",__func__);
		return rc;
	}

	/* i2c control-"0" in command register disable charger */
	rc = smb346_update(msm_chg, PIN_AND_ENABLE_CONTROL_REGISTER, I2C_CONTROL_0_TO_DISABLE_CHARGER, 
		PIN_CONTROL_ACTIVE_LOW);
	if (rc){
		pr_err("%s:smb346 update PIN_AND_ENABLE_CONTROL_REGISTER register failed!\n",__func__);
		return rc;
	}

	#ifdef FEATURE_SUSPEND_CHARGE
	#error suspend charge
	/* active suspend mode */
	rc = smb346_update(msm_chg, STATUS_REGISTER_B, USB_SUSPEND_MODE__ACTIVE, USB_SUSPEND_MODE__ACTIVE);	
	if (rc)
		return rc;
	#endif

	#ifndef FEATURE_AUTO_RECHARGE
	/* disable auto recharge Fuction */
	rc = smb346_update(msm_chg, CHARGE_CONTROL_REGISTER, AUTOMATIC_RECHARGE_DISABLED, 
		AUTOMATIC_RECHARGE_ENABLED);
	if (rc){
		pr_err("%s:smb346 update CHARGE_CONTROL_REGISTER register failed!\n",__func__);
		return rc;
	}
	#endif
#if 0//the same as hardware defaulf config
	/* enable VCHG Fuction */
	rc = smb346_update(msm_chg, VARIOUS_FUNCTION_REGISTER, VCHG_FUNCTION_ENABLED, 
		VCHG_FUNCTION_ENABLED);
	if (rc)
		return rc;
#endif
	#ifdef FEATURE_OPPO_AICL_CONTROL
	/* disable automatic input current limit*/
	rc = smb346_update(msm_chg, VARIOUS_FUNCTION_REGISTER, AUTOMATIC_INPUT_CURRENT_LIMIT_DISABLED, 
		AUTOMATIC_INPUT_CURRENT_LIMIT_ENABLED);
	if (rc){
		pr_err("%s: disable automatic input current limit failed!\n",__func__);
		return rc;
	}
	if(smb346_charger_type_get(msm_chg)== CHARGER_TYPE_USB_HDMI){
		/* set AICL detection threhold */
		rc = smb346_update(msm_chg, VARIOUS_FUNCTION_REGISTER, AUTOMATIC_INPUT_CURRENT_LIMIT_DETECTION_THRESHOLD_4P50V, 
			AUTOMATIC_INPUT_CURRENT_LIMIT_DETECTION_THRESHOLD);
		if (rc){
			pr_err("%s: HDMI smb346 update VARIOUS_FUNCTION_REGISTER register failed!\n",__func__);
			return rc;
		}
	}else{
		/* set AICL detection threhold */
		rc = smb346_update(msm_chg, VARIOUS_FUNCTION_REGISTER, AUTOMATIC_INPUT_CURRENT_LIMIT_DETECTION_THRESHOLD_4P25V, 
			AUTOMATIC_INPUT_CURRENT_LIMIT_DETECTION_THRESHOLD_4P25V);
		if (rc){
			pr_err("%s:DEFAULT smb346 update VARIOUS_FUNCTION_REGISTER register failed!\n",__func__);
			return rc;
		}
	}
	/* enable VCHG Fuction */
	rc = smb346_update(msm_chg, VARIOUS_FUNCTION_REGISTER, AUTOMATIC_INPUT_CURRENT_LIMIT_DISABLED, 
		AUTOMATIC_INPUT_CURRENT_LIMIT_ENABLED);
	if (rc){
		pr_err("%s: enable automatic input current limit failed!\n",__func__);
		return rc;
	}
	#endif

	return 0;
}

/**
*   init smb346 charger info when 1.charger connect ,2.charger remove
*/
static void smb346_info_val_init(struct smb346_charger_mux *msm_chg)
{
	DEBUG_CHG("%s\n", __func__);
	
	//msm_chg->mBatteryTempRegion = CV_BATTERY_TEMP_REGION__NORMAL;
	smb346_battery_temp_region_set(msm_chg, CV_BATTERY_TEMP_REGION__NORMAL);
	msm_chg->mBatteryTempBoundT0 = AUTO_CHARGING_BATT_TEMP_T0;
	msm_chg->mBatteryTempBoundT1 = AUTO_CHARGING_BATT_TEMP_T1;	
	msm_chg->mBatteryTempBoundT2 = AUTO_CHARGING_BATT_TEMP_T2;
	msm_chg->mBatteryTempBoundT3 = AUTO_CHARGING_BATT_TEMP_T3;
	msm_chg->mBatteryTempBoundT4 = AUTO_CHARGING_BATT_TEMP_T4;
	msm_chg->StopChgDueToBadChagerState = false;
	msm_chg->StopChgDueToBadBatteryState = false;
	msm_chg->StopChgDueToBadTempState = false;
	msm_chg->mBadChargerCounter = 0;
	msm_chg->mChargerCheckCounter = MAX_CHARGER_CHECK_COUNT;
	msm_chg->mBadBatteryCounter = 0;
	#ifdef INTERVAL_CHARGING_FEATURE
	msm_chg->interval_chg_timer_count = 0;
	#endif
	msm_chg->mAICLCounter = 0;
	msm_chg->mTapperCounter = 0;
	msm_chg->mEndCVTopoffCounter = 0;
	msm_chg->charging_status  = SMB_CHG_STATUS__INVALID;
	msm_chg->charging_completed_detect_enabled = false;
	msm_chg->mIsSuspendOn = false;
	msm_chg->charger_type = CHARGER_TYPE_INVALID;
	msm_chg->is_interval_chg = false; //true;
	msm_chg->soft_control_threhold = SOFT_CONTROL_THREHOLD;
	msm_chg->cc_step_curr = FAST_CHG_450MA;
	msm_chg->aicl_set_val = USBIN_INPUT_CURRENT_LIMIT_500MA;
	msm_chg->flt_set_val = FLOAT_VOTAGE_4220MV;
	msm_chg->stop_resume_check = true;
	msm_chg->resume_count = 0;

	/*reset charger info to init status*/
	smb346_charger_info_init(msm_chg);
	smb346_battery_adc_params_init(msm_chg);
	
}

static int smb346_rx_irq_regs(struct smb346_charger_mux *msm_chg, BYTE* reg_irq)
{	
	int rc = -1;
	int i =0;

	rc = smb346_reads(msm_chg, INTERRUPT_REGISTER_A, 6, reg_irq);
	if (rc)
		return rc;
	
	for (i = 0; i < 6; i++){
		DEBUG_CHG("reg(0x%x) = 0x%x\n", INTERRUPT_REGISTER_A + i, reg_irq[i]);
	}
	
	return 0;
}

static void __dump_regs_all(struct smb346_charger_mux *msm_chg)
{
	uint8_t buf[15];
	uint8_t temp;
	int i =0;
	DEBUG_CHG("%s:======regs_all_info======\n", __func__);
	smb346_reads(msm_chg, CHARGE_CURRENT_REGISTER, 15, buf);
	for (i = 0; i < 15; i++)
	{
		ERR_CHG("reg(0x%x) = 0x%x\n", CHARGE_CURRENT_REGISTER + i, buf[i]);
	}

	smb346_read(msm_chg, COMMAND_REGISTER_A, &temp);
	ERR_CHG("reg(0x%x) = 0x%x\n", COMMAND_REGISTER_A, temp);
	smb346_read(msm_chg, COMMAND_REGISTER_B, &temp);
	ERR_CHG("reg(0x%x) = 0x%x\n", COMMAND_REGISTER_B, temp);
	smb346_read(msm_chg, COMMAND_REGISTER_C, &temp);
	ERR_CHG("reg(0x%x) = 0x%x\n", COMMAND_REGISTER_C, temp);	

	smb346_reads(msm_chg, INTERRUPT_REGISTER_A, 11, buf);
	for (i = 0; i < 11; i++)
	{
		DEBUG_CHG("reg(0x%x) = 0x%x\n", INTERRUPT_REGISTER_A + i, buf[i]);
	}	
}

static void __dump_batt_info(struct smb346_charger_mux *msm_chg)
{
	char *batt_status_string[] = {"unknow", "charing", "discharing", "not charing", "full"};
	char *batt_health_string[] = {"unknow", "GOOD", "over heat", "dead", "over-voltage", "unspec-failure", "cold"};
	char *batt_valid[] = {"no", "yes"};
	char *batt_temp_region_string[] = {"cold", "little cold", "cool", "normal", "warm", "hot", "invalid"};
	char *charger_status_string[] = {"Good", "Bad", "Weak", "Invalid"};
	char *charger_type_string[] = {"None", "Wall", "Usb_PC(usb charger)", "Usb_Wall(standard charger)", 
					"Usb_Carkit", "Non_Standard", "Invalid"};
	char *battery_status_string[] = {"Good", "Bad_temp", "Bad", "Removed", "Invalid"};
	char *battery_level_string[] = {"Dead", "Weak", "Good", "Full", "Invalid"};
	char *charger_adapt_string[] = {"False", "True"};
	
	DEBUG_CHG("========batt_info=====\n");
	DEBUG_CHG("batt_status = %s\n", batt_status_string[smb346_charger_info.batt_status]);
	DEBUG_CHG("batt_health = %s\n", batt_health_string[smb346_charger_info.batt_health]);
	DEBUG_CHG("batt_valid = %s\n", batt_valid[smb346_charger_info.batt_valid]);
	DEBUG_CHG("charger_status = %s\n", charger_status_string[smb346_charger_info.charger_status]);
	DEBUG_CHG("charger_type = %s\n", charger_type_string[smb346_charger_info.charger_type]);
	DEBUG_CHG("battery_status = %s\n", battery_status_string[smb346_charger_info.battery_status]);
	DEBUG_CHG("battery_level = %s\n", battery_level_string[smb346_charger_info.battery_level]);
	DEBUG_CHG("battery_temp_region=%s\n", batt_temp_region_string[msm_chg->mBatteryTempRegion]);
	DEBUG_CHG("is_adapt_completed = %s\n", charger_adapt_string[smb346_charger_info.is_adapt_completed]);
}

static void __dump_adc_info(struct smb346_charger_mux *msm_chg)
{
	DEBUG_CHG("========adc_info=====\n");
	DEBUG_CHG("current_batt_adc_voltage =%d\n", smb346_battery_adc_params.current_batt_adc_voltage);
	DEBUG_CHG("batt_temperature =%d\n", smb346_battery_adc_params.batt_temperature);
	DEBUG_CHG("vchg_mv =%d\n", smb346_battery_adc_params.vchg_mv);
	DEBUG_CHG("ichg_mv =%d\n", smb346_battery_adc_params.ichg_mv);
}

static void __dump_charger_info(struct smb346_charger_mux *msm_chg)
{
	char *charging_status[] = {"INVALID", "NOT CHARGING", "CHARGING", "CHARGING_COMPLETED"};

	DEBUG_CHG("========charger_info=====\n");
	DEBUG_CHG("chg_usb_OnOff_nv_item_val = %d\n", chg_usb_OnOff_nv_item_val);
	DEBUG_CHG("battery_valid = %d\n", msm_chg->battery_valid);
	DEBUG_CHG("is_interval_chg = %d\n", msm_chg->is_interval_chg);
	DEBUG_CHG("soft_control_threhold = %d\n", msm_chg->soft_control_threhold);
	DEBUG_CHG("charging_status = %s\n", charging_status[msm_chg->charging_status]);
	DEBUG_CHG("charging_completed_detect_enabled = %d\n", msm_chg->charging_completed_detect_enabled);
	DEBUG_CHG("StopChgDueToBadChagerState = %d\n", msm_chg->StopChgDueToBadChagerState);
	DEBUG_CHG("StopChgDueToBadBatteryState = %d\n", msm_chg->StopChgDueToBadBatteryState);
	DEBUG_CHG("StopChgDueToBadTempState = %d\n", msm_chg->StopChgDueToBadTempState);
	DEBUG_CHG("mIsSuspendOn = %d\n", msm_chg->mIsSuspendOn);
}

bool is_show_batt_log(void)
{
	return show_batt_log;
}

bool is_show_gague_log(void)
{
	return show_gague_log;
}

static bool smb346_is_battery_present(void)
{
	bool battery_is_present=false;

	battery_is_present =  fuelgague_battery_is_present(); 
	return battery_is_present;
}

static int smb346_get_battery_adc_realtime(struct smb346_charger_mux *msm_chg)
{
	if (msm_batt_gauge && msm_batt_gauge->qury_adc_params){
		msm_batt_gauge->qury_adc_params(&smb346_battery_adc_params);
		DEBUG_CHG("charge currrent read =%d\n", smb346_battery_adc_params.ichg_mv);
		__dump_adc_info(msm_chg);
		return 0;
	}
	else {
		ERR_CHG("msm-charger no batt gauge assuming 35 deg G\n");
		return -1;
	}
}

static int get_batt_adc(struct smb346_charger_mux *msm_chg)
{
	int charge_currrent = 0;
	DEBUG_CHG("%s\n", __func__);

	if (msm_batt_gauge && msm_batt_gauge->qury_adc_params){
		charge_currrent= smb346_battery_adc_params.ichg_mv;
		msm_batt_gauge->qury_adc_params(&smb346_battery_adc_params);
		DEBUG_CHG("charge currrent read =%d\n", smb346_battery_adc_params.ichg_mv);
		smb346_battery_adc_params.ichg_mv = charge_currrent;
		__dump_adc_info(msm_chg);
		return 0;
	}
	else {
		ERR_CHG("msm-charger no batt gauge assuming 35 deg G\n");
		return -1;
	}
}

static int get_batt_adc_chg(struct smb346_charger_mux *msm_chg)
{
	int battery_voltage = 0;
	int charger_voltage = 0;	
	DEBUG_CHG("%s\n", __func__);
	
	if (msm_batt_gauge && msm_batt_gauge->qury_adc_params){
		battery_voltage = smb346_battery_adc_params.current_batt_adc_voltage;
		charger_voltage = smb346_battery_adc_params.vchg_mv;
		msm_batt_gauge->qury_adc_params(&smb346_battery_adc_params);
		DEBUG_CHG("batt voltage read =%d\n", smb346_battery_adc_params.current_batt_adc_voltage);
		DEBUG_CHG("charger voltage read =%d\n", smb346_battery_adc_params.vchg_mv);
		smb346_battery_adc_params.current_batt_adc_voltage = battery_voltage;
		smb346_battery_adc_params.vchg_mv = charger_voltage;
		__dump_adc_info(msm_chg);
		return 0;
	}
	else {
		ERR_CHG("msm-charger no batt gauge assuming 35 deg G\n");
		return -1;
	}
}

static int update_psy_status(struct smb346_charger_mux *msm_chg)
{
	DEBUG_CHG("%s\n", __func__);
	if (msm_batt_gauge && msm_batt_gauge->update_psy_status) {
		__dump_batt_info(msm_chg);
		msm_batt_gauge->update_psy_status(&smb346_charger_info, &smb346_battery_adc_params);
		return 0;
	}
	else {
		ERR_CHG("msm-charger no batt gauge update psy status.\n");
		return -1;
	}
}

static bool get_aicl_status(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	uint8_t temp;
	int aicl_result_i;
	int aicl_result[16] = {
		300,
		500,
		700,
		900,
		1200,
		1500,
		1800,
		2000,
		2200,
		2500,
		2500,
		2500,
		2500,
		2500,
		2500,	
		2500
	};
	
	rc = smb346_read(msm_chg, STATUS_REGISTER_E, &temp);
	if (rc)
		return false;
	
	DEBUG_CHG("reg(0x%x) = 0x%x\n", STATUS_REGISTER_E, temp);

	if (temp & AICL_STATUS__COMPLETED){
		aicl_result_i = aicl_result[temp & AICL_RESULTS__MASK];
		pr_info("AICL complete! AICL result = %d mA!\n", aicl_result_i);
		return true;
	}else{
		DEBUG_CHG("AICL not complete!\n\n");
		return false;
	}
}
static smb346_charging_mode_type smb346_charging_mode_get(struct smb346_charger_mux *msm_chg)
{
	uint8_t temp;
	smb346_charging_mode_type charging_mode = SMB_CHG_MODE__NOT_CHARGING;
	smb346_read(msm_chg, STATUS_REGISTER_C, &temp);
	DEBUG_CHG("reg(0x%x) = 0x%x\n", STATUS_REGISTER_C, temp);	

	switch ((temp & CHARGE_STATUS__TAPER_CHARGING) >> 1)
	{
		case 0:
		{
			DEBUG_CHG("charing status: not charing!\n");
			charging_mode = SMB_CHG_MODE__NOT_CHARGING;
			break;
		}
		case 1:
		{
			DEBUG_CHG("charing status: pre charing!\n");
			charging_mode = SMB_CHG_MODE__PRE_CHARGING;
			break;
		}
		case 2:
		{
			DEBUG_CHG("charing status: fast charing!\n");
			charging_mode = SMB_CHG_MODE__FAST_CHARGING;
			break;
		}
		case 3:
		{
			DEBUG_CHG("charing status: tapper charing!\n");
			charging_mode = SMB_CHG_MODE__TAPPER_CHARGING;
			break;
		}
	}
	return charging_mode;
}

static enum hrtimer_restart smb346_update_heartbeat_timer__func(struct hrtimer *timer)
{
	struct smb346_charger_mux *msm_chg = container_of(timer, struct smb346_charger_mux, update_heartbeat_timer);

	pr_debug("%s\n", __func__);

	hrtimer_start(&msm_chg->update_heartbeat_timer, ktime_set(UPDATE_TIME_S, 0), HRTIMER_MODE_REL);
	msm_charger_notify_event(msm_chg, TIMER_EXPIRED__HEART_BEAT);
	
	return HRTIMER_NORESTART;
}
#ifdef INTERVAL_CHARGING_FEATURE
static enum hrtimer_restart smb346_interval_charge_timer__func(struct hrtimer *timer)
{
	struct smb346_charger_mux *msm_chg = container_of(timer, struct smb346_charger_mux, interval_charge_timer);
	
	pr_info("%s:smb346_interval_charge_timer alive", __func__);
	msm_chg->interval_chg_timer_count = (msm_chg->interval_chg_timer_count + 1)%INTERVAL_CHG_TIMER_MAX_COUNT;
	DEBUG_CHG("interval_chg_timer_count = %d\n", msm_chg->interval_chg_timer_count);
	switch (msm_chg->interval_chg_timer_count) 
	{
		/* pause charging when timer expired (0.5s) */
		case 0:
		{
			hrtimer_start(&msm_chg->interval_charge_timer, ktime_set(UPDATE_TIME_S, 0), HRTIMER_MODE_REL);
			msm_charger_notify_event(msm_chg, TIMER_EXPIRED__CC_DISCHG_DETECT);			
			break;
		}
		/* get battery adc when timer expired (5s) */
		case 1:
		case 2:
		case 3:
		{
			hrtimer_start(&msm_chg->interval_charge_timer, ktime_set(UPDATE_TIME_S, 0), HRTIMER_MODE_REL);
			msm_charger_notify_event(msm_chg, TIMER_EXPIRED__CHG_GET_ADC);			
			break;
		}
		/* resume charging when timer expired (5s) */
		case 4:
		{
			hrtimer_start(&msm_chg->interval_charge_timer, ktime_set(0, CC_DISCHG_DETECT_TIME_NS), HRTIMER_MODE_REL);
			msm_charger_notify_event(msm_chg, TIMER_EXPIRED__CC_CHG_DETECT);			
			break;
		}
		default:
		{
			ERR_CHG("invalid timer_func_id\n");
			break;
		}
	}
	
	return HRTIMER_NORESTART;
}
#endif


static enum hrtimer_restart ftm_charge_suspnd_mode__func(struct hrtimer *timer)
{
	pr_info("%s:\n",__func__);
	smb346_ftm_charge_suspend_mode(true);
	return HRTIMER_NORESTART;	
}
static enum hrtimer_restart smb346_calling_timer__func(struct hrtimer *timer)
{
	struct smb346_charger_mux *msm_chg = container_of(timer, struct smb346_charger_mux, calling_timer);

	hrtimer_start(&msm_chg->calling_timer, ktime_set(CALLING_TIMER_CHECK_PERIOID, 0), HRTIMER_MODE_REL);
	msm_charger_notify_event(msm_chg, TIMER_EXPIRED__CALLING);
	return HRTIMER_NORESTART;
}
static enum hrtimer_restart smb346_delay_enable_charge__func(struct hrtimer *timer)
{
	struct smb346_charger_mux *msm_chg = container_of(timer, struct smb346_charger_mux, charge_enable_delay_timer);

	chg_charger_hardware_type charger__type;
	mutex_lock(&smb346_chg.smb346_lock);
	charger__type = smb346_charger_type_get(msm_chg);
	mutex_unlock(&smb346_chg.smb346_lock);
	
	pr_info("%s,begin delay charging enable charger type =%d\n", __func__,charger__type);
	switch (charger__type){
		case CHARGER_TYPE_USB_PC:
				msm_charger_notify_event(msm_chg, SYSTEM_STATUS_CHANGED__USB_VALID);
				break;
		case CHARGER_TYPE_USB_WALL:
				msm_charger_notify_event(msm_chg, SYSTEM_STATUS_CHANGED__AC_VALID);
				break;
		case CHARGER_TYPE_USB_HDMI:
				msm_charger_notify_event(msm_chg, SYSTEM_STATUS_CHANGED__HDMI_VALID);
				break;
		case CHARGER_TYPE_NON_STANDARD:
				msm_charger_notify_event(msm_chg, SYSTEM_STATUS_CHANGED__NONSTANDARD_VALID);
				break;
		case CHARGER_TYPE_ACCESSORY:
				msm_charger_notify_event(msm_chg, SYSTEM_STATUS_CHANGED__ACCESSORY);
				break;
		default:
			break;
	}
	
	return HRTIMER_NORESTART;	
}

static enum hrtimer_restart smb346_max_charge_time_timer__func(struct hrtimer *timer)
{
	struct smb346_charger_mux *msm_chg = container_of(timer, struct smb346_charger_mux, max_charge_time_timer);

	pr_info("%s safe timer expired\n", __func__);

	msm_charger_notify_event(msm_chg, TIMER_EXPIRED__MAX_CHG_TIME);
	
	return HRTIMER_NORESTART;	
}

static enum hrtimer_restart smb346_charge_complete_detect_timer__func(struct hrtimer *timer)
{
	struct smb346_charger_mux *msm_chg = container_of(timer, struct smb346_charger_mux, end_charge_detect_timer);

	DEBUG_CHG("%s: smb346_charge_complete_detect_timer alive\n", __func__);

	hrtimer_start(&msm_chg->end_charge_detect_timer, ktime_set(END_CHG_DETECT_S, 0), HRTIMER_MODE_REL);
	msm_charger_notify_event(msm_chg, TIMER_EXPIRED__END_CHG_DETECT_TIME);
	
	return HRTIMER_NORESTART;	
}

#ifndef FEATURE_AUTO_RECHARGE
static enum hrtimer_restart smb346_charge_resume_timer__func(struct hrtimer *timer)
{
	struct smb346_charger_mux *msm_chg = container_of(timer, struct smb346_charger_mux, charge_resume_timer);

	pr_info("%s\n", __func__);

	hrtimer_start(&msm_chg->charge_resume_timer, ktime_set(RESUME_CHECK_PERIOD_S, 0), HRTIMER_MODE_REL);
	msm_charger_notify_event(msm_chg, TIMER_EXPIRED__CHG_RESUME);
	
	return HRTIMER_NORESTART;	
}
#endif

static inline int smb346_hrtimer_is_queued(struct hrtimer *timer)
{
	return timer->state & HRTIMER_STATE_ENQUEUED;
}

static void charge_task_timer_init(struct smb346_charger_mux *msm_chg)
{
	DEBUG_CHG("%s\n", __func__);

	/* charge enable delay timer init */
	hrtimer_init(&msm_chg->charge_enable_delay_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	msm_chg->charge_enable_delay_timer.function = smb346_delay_enable_charge__func;
	
	/* update heartbeat timer init */
	hrtimer_init(&msm_chg->update_heartbeat_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	msm_chg->update_heartbeat_timer.function = smb346_update_heartbeat_timer__func;
       #ifdef INTERVAL_CHARGING_FEATURE
	/* interval charge timer init */
	hrtimer_init(&msm_chg->interval_charge_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	msm_chg->interval_charge_timer.function = smb346_interval_charge_timer__func;	
	#endif

	/* max charge time timer init */
	hrtimer_init(&msm_chg->max_charge_time_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	msm_chg->max_charge_time_timer.function = smb346_max_charge_time_timer__func;	

	/* charge complete detect timer init */
	hrtimer_init(&msm_chg->end_charge_detect_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	msm_chg->end_charge_detect_timer.function = smb346_charge_complete_detect_timer__func;	

#ifndef FEATURE_AUTO_RECHARGE
	/* charge resume timer init */
	hrtimer_init(&msm_chg->charge_resume_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	msm_chg->charge_resume_timer.function = smb346_charge_resume_timer__func;
	#endif

    /* charge ftm suspend mode timer*/
	hrtimer_init(&msm_chg->ftm_charge_suspend_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	msm_chg->ftm_charge_suspend_timer.function = ftm_charge_suspnd_mode__func;
	/*timer for calling*/
	hrtimer_init(&msm_chg->calling_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	msm_chg->calling_timer.function = smb346_calling_timer__func;
}

/*charge_enable_delay_timer*/
static void add_listener_for_charge_enable_delay(struct smb346_charger_mux *msm_chg)
{
	DEBUG_CHG("%s\n", __func__);
	hrtimer_start(&msm_chg->charge_enable_delay_timer, ktime_set(0, CHARGE_ENABLE_DELAY), HRTIMER_MODE_REL);	
}

static void remove_listener_for_charge_enable_delay(struct smb346_charger_mux *msm_chg)
{
	DEBUG_CHG("%s\n", __func__);
	hrtimer_cancel(&msm_chg->charge_enable_delay_timer);
}
/*charge_enable_delay_timer*/
#ifdef INTERVAL_CHARGING_FEATURE
//delete interval charging feature on 12001 project by chendx 2012-03-18 
/*interval_charge_timer*/
static void add_listener_for_interval_charging(struct smb346_charger_mux *msm_chg)
{
	pr_info("%s\n", __func__);
	if (msm_chg->is_interval_chg == false){
		hrtimer_start(&msm_chg->interval_charge_timer, ktime_set(UPDATE_TIME_S, 0), HRTIMER_MODE_REL);
		msm_chg->is_interval_chg = true;
	}
	else
		DEBUG_CHG("listener for interval charging is already started\n");
}
static void remove_listener_for_interval_charging(struct smb346_charger_mux *msm_chg)
{
	DEBUG_CHG("%s\n", __func__);
	if (msm_chg->is_interval_chg == true){
		hrtimer_cancel(&msm_chg->interval_charge_timer);
		msm_chg->is_interval_chg = false;
	}
	else
		DEBUG_CHG("listener for interval charging is alreadly stopped or not started\n");
}
/*interval_charge_timer*/
#endif

/*update_heartbeat_timer*/
static void add_listener_for_update_heartbeat(struct smb346_charger_mux *msm_chg)
{
	DEBUG_CHG("%s\n", __func__);
	hrtimer_start(&msm_chg->update_heartbeat_timer, ktime_set(UPDATE_TIME_S, 500000000), HRTIMER_MODE_REL);
}

static void remove_listener_for_update_heartbeat(struct smb346_charger_mux *msm_chg)
{
	DEBUG_CHG("%s\n", __func__);
	hrtimer_cancel(&msm_chg->update_heartbeat_timer);
}
/*update_heartbeat_timer*/

static void add_listener_for_max_charge_time(struct smb346_charger_mux *msm_chg)
{
	DEBUG_CHG("%s\n", __func__);
	hrtimer_start(&msm_chg->max_charge_time_timer, ktime_set(CHARGING_TEOC_S, 0), HRTIMER_MODE_REL);
}

static void remove_listener_for_max_charge_time(struct smb346_charger_mux *msm_chg)
{
	DEBUG_CHG("%s\n", __func__);
	hrtimer_cancel(&msm_chg->max_charge_time_timer);
}

static void add_listener_for_charge_complete_detect(struct smb346_charger_mux *msm_chg)
{
	DEBUG_CHG("%s:charging_competed_detect_enable=%d\n", __func__, msm_chg->charging_completed_detect_enabled);
	if (smb346_charging_completed_detect_enabled_get(msm_chg)== false){
		/*start charging complete detect timer(2s) to check charging complete*/
		ERR_CHG("%s\n", __func__);
		hrtimer_start(&msm_chg->end_charge_detect_timer, ktime_set(END_CHG_DETECT_S, 0), HRTIMER_MODE_REL);		
		msm_chg->mEndCVTopoffCounter = 0;
		smb346_charging_completed_detect_enabled_set(msm_chg, true);
	}
	else
		DEBUG_CHG("%s: is alreadly started\n", __func__);
}

static void remove_listener_for_charge_complete_detect(struct smb346_charger_mux *msm_chg)
{
	if (smb346_charging_completed_detect_enabled_get(msm_chg) == true) {
		DEBUG_CHG("%s\n", __func__);
		hrtimer_cancel(&msm_chg->end_charge_detect_timer);
		//msm_chg->mEndCVTopoffCounter = 0;
		smb346_charging_completed_detect_enabled_set(msm_chg, false);
	}
	else
		DEBUG_CHG("%s: is alreadly stopped\n", __func__);
}

#ifndef FEATURE_AUTO_RECHARGE
static void add_listener_for_charge_resume(struct smb346_charger_mux *msm_chg)
{
	DEBUG_CHG("%s\n", __func__);
	if (msm_chg->stop_resume_check == true){
		hrtimer_start(&msm_chg->charge_resume_timer, ktime_set(RESUME_CHECK_PERIOD_S, 0), HRTIMER_MODE_REL);
		msm_chg->stop_resume_check = false;
		msm_chg->resume_count = 0;
	}
	else
		DEBUG_CHG("%s:add_listener_for_charge_resume is alreadly started\n", __func__);
}

static void remove_listener_for_charge_resume(struct smb346_charger_mux *msm_chg)
{
	DEBUG_CHG("%s\n", __func__);
	if (msm_chg->stop_resume_check == false){
		hrtimer_cancel(&msm_chg->charge_resume_timer);
		msm_chg->stop_resume_check = true;
		msm_chg->resume_count = 0;
	}
	else
		DEBUG_CHG("%s:remove_listener_for_charge_resume is alreadly stopped or not started\n", __func__);
}
#endif
static void add_listener_for_calling_timer(struct smb346_charger_mux *msm_chg)
{
	ERR_CHG("%s:\n", __func__);
	hrtimer_start(&msm_chg->calling_timer, ktime_set(CALLING_TIMER_CHECK_PERIOID, 0), HRTIMER_MODE_REL);
}
static void remove_listener_for_calling_timer(struct smb346_charger_mux *msm_chg)
{
	ERR_CHG("%s:\n", __func__);
	hrtimer_cancel(&msm_chg->calling_timer);
}
static int  smb346_set_cccv(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	DEBUG_CHG("%s\n", __func__);
	
#if (FEATURE_SMB_COOL_TEMP_CONTROL || FEATURE_SMB_HOT_TEMP_CONTROL)
	DEBUG_CHG("%s:===for therm control\n", __func__);
	rc = smb346_update(msm_chg, THERM_AND_SYSTEM_CONTROL_REGISTER, THERMISTER_MONITOR_DISABLED,
	THERMISTER_MONITOR_DISABLED);
	if (rc)
		return rc;	
	/*set therm and system control register usbin supply*/
	rc = smb346_update(msm_chg, THERM_AND_SYSTEM_CONTROL_REGISTER,THERM_MONITOR_SELECTION__USBIN_SUPPLY,THERM_MONITOR_SELECTION__VDDCAP_SUPPLY);
	if (rc)
		return rc;

#endif

	/* set current for fast charing. */
	rc = smb346_update(msm_chg, CHARGE_CURRENT_REGISTER, regval_cc_step[msm_chg->cc_step_curr], FAST_CHARGE_CURRENT_1250MA);
	if (rc)
		return rc;

	/* set termination charing current to 50 mA, default 100mA */
	#if 0
	/* set termination charing current. */
	rc = smb346_update(msm_chg, CHARGE_CURRENT_REGISTER, TERMINATION_CURRENT_50MA, TERMINATION_CURRENT_600MA);
	if (rc)
		return rc;
	#endif
	
	/* set Input current limit. */
	rc = smb346_update(msm_chg, INPUT_CURRENT_LIMIT_REGISTER, msm_chg->aicl_set_val, USBIN_DCIN_INPUT_CURRENT_LIMIT_MASK);
	if (rc)
		return rc;

	/* set Float voltage. */
	rc= smb346_update(msm_chg, FLOAT_VOTAGE_REGISTER, msm_chg->flt_set_val, FLOAT_VOTAGE_MASK);	
	if (rc)
		return rc;	
	return 0;
}
/*OPPO 20120-2-8 Jiangsm modify begin for handle cool and warm seperately*/
#if 1
static int smb346_set_charge_cool(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;	
	pr_info("%s\n", __func__);

	#if (FEATURE_SMB_COOL_TEMP_CONTROL || FEATURE_SMB_HOT_TEMP_CONTROL)
	DEBUG_CHG("%s:===for therm control\n", __func__);
	rc = smb346_update(msm_chg, THERM_AND_SYSTEM_CONTROL_REGISTER, THERMISTER_MONITOR_DISABLED,
	THERMISTER_MONITOR_DISABLED);
	if (rc)
		return rc;	
	/*set therm and system control register usbin supply*/
	rc = smb346_update(msm_chg, THERM_AND_SYSTEM_CONTROL_REGISTER,THERM_MONITOR_SELECTION__USBIN_SUPPLY,THERM_MONITOR_SELECTION__VDDCAP_SUPPLY);
	if (rc)
		return rc;

      #endif

	/* set current for fast charing. */
	rc = smb346_update(msm_chg, CHARGE_CURRENT_REGISTER, FAST_CHG_350MA, FAST_CHARGE_CURRENT_1250MA);
	if (rc)
		return rc;

	/* set Float voltage. */
	rc= smb346_update(msm_chg, FLOAT_VOTAGE_REGISTER, FLOAT_VOTAGE_4100MV, FLOAT_VOTAGE_MASK);	
	if (rc)
		return rc;	
	
	return 0;
}

static int smb346_set_charge_little_cold(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;	
	pr_info("%s\n", __func__);

	/* this feature is for match standard: set charging current to 125mA in condition battery temperature cold or warm. */
	rc = smb346_update(msm_chg, THERM_AND_SYSTEM_CONTROL_REGISTER, THERMISTER_MONITOR_ENABLED,
	THERMISTER_MONITOR_DISABLED);
	if (rc)
		return rc;
	
	rc = smb346_update(msm_chg, THERM_AND_SYSTEM_CONTROL_REGISTER,THERM_MONITOR_SELECTION__VDDCAP_SUPPLY,THERM_MONITOR_SELECTION__VDDCAP_SUPPLY);
	if (rc)
		return rc;

	
	/* set Float voltage. */
	rc = smb346_update(msm_chg, FLOAT_VOTAGE_REGISTER, FLOAT_VOTAGE_4000MV, FLOAT_VOTAGE_MASK);	
	if (rc)
		return rc;	
	
	return 0;
}

static int smb346_set_charge_warm(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;	
	pr_info("%s\n", __func__);
	
#if  FEATURE_SMB_HOT_TEMP_CONTROL
	/* this feature is for match standard: set charging current to 125mA in condition battery temperature cold or warm. */
	rc = smb346_update(msm_chg, THERM_AND_SYSTEM_CONTROL_REGISTER, THERMISTER_MONITOR_ENABLED,
	THERMISTER_MONITOR_DISABLED);
	if (rc)
		return rc;
	rc = smb346_update(msm_chg, THERM_AND_SYSTEM_CONTROL_REGISTER,THERM_MONITOR_SELECTION__VDDCAP_SUPPLY,THERM_MONITOR_SELECTION__VDDCAP_SUPPLY);
	if (rc)
		return rc;

#else
	rc = smb346_update(msm_chg, THERM_AND_SYSTEM_CONTROL_REGISTER, THERMISTER_MONITOR_DISABLED,
	THERMISTER_MONITOR_DISABLED);
	if (rc)
		return rc;	
	/*set therm and system control register usbin supply*/
	rc = smb346_update(msm_chg, THERM_AND_SYSTEM_CONTROL_REGISTER,THERM_MONITOR_SELECTION__USBIN_SUPPLY,THERM_MONITOR_SELECTION__VDDCAP_SUPPLY);
	if (rc)
		return rc;
		/* set current for fast charing. */
	rc = smb346_update(msm_chg, CHARGE_CURRENT_REGISTER, FAST_CHG_350MA, FAST_CHARGE_CURRENT_1250MA);
	if (rc)
		return rc;	

#endif

	/* set Float voltage. */
	rc = smb346_update(msm_chg, FLOAT_VOTAGE_REGISTER, FLOAT_VOTAGE_4100MV, FLOAT_VOTAGE_MASK);	
	if (rc)
		return rc;	
	return 0;
}

#else
static int smb346_set_charge_cool_warm(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;	
	DEBUG_CHG("%s\n", __func__);

#ifdef FEATURE_OPPO_TEMP_CONTROL
	/* this feature is for match standard: set charging current to 150mA in condition battery temperature cold or warm. */
	rc = smb346_update(msm_chg, THERM_AND_SYSTEM_CONTROL_REGISTER, THERMISTER_MONITOR_ENABLED,
	THERMISTER_MONITOR_DISABLED);
	if (rc)
		return rc;	
#else
	/* set current for fast charing. */
	rc = smb346_update(msm_chg, CHARGE_CURRENT_REGISTER, FAST_CHG_350MA, FAST_CHARGE_CURRENT_1250MA);
	if (rc)
		return rc;	
#endif
	
	/* set Float voltage. */
	rc = smb346_update(msm_chg, FLOAT_VOTAGE_REGISTER, FLOAT_VOTAGE_4100MV, FLOAT_VOTAGE_MASK);	
	if (rc)
		return rc;	
	
	return 0;
}
#endif
/*OPPO 2012-2-8 JIangsm modify end*/
static int smb346_set_charge_hvp(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	ERR_CHG("%s\n", __func__);
	
	msm_chg->cc_step_curr = FAST_CHG_350MA;
	msm_chg->aicl_set_val = USBIN_INPUT_CURRENT_LIMIT_300MA;
	msm_chg->flt_set_val = FLOAT_VOTAGE_4220MV;
	
	/* set current for fast charing. */
	rc = smb346_set_cccv(msm_chg);
	if (rc){
		ERR_CHG("%s:smb346_set_set_charge_hvp failed\n", __func__);
		return rc;	
	}
	return 0;
}

static int smb346_set_charge_ac(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	pr_info("%s\n", __func__);
	
	msm_chg->cc_step_curr = FAST_CHG_1000MA;
	msm_chg->aicl_set_val = USBIN_DCIN_INPUT_CURRENT_900MA;
	msm_chg->flt_set_val = FLOAT_VOTAGE_4220MV;
	
	/* set current for fast charing. */
	rc = smb346_set_cccv(msm_chg);
	if (rc){
		ERR_CHG("%s:smb346_set_set_charge_ac failed\n", __func__);
		return rc;	
	}
	return 0;
}

static int smb346_set_charge_usb(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;	
	pr_info("%s\n", __func__);
	
	msm_chg->cc_step_curr = FAST_CHG_1000MA;
	msm_chg->aicl_set_val = USBIN_DCIN_INPUT_CURRENT_500MA;
	msm_chg->flt_set_val = FLOAT_VOTAGE_4220MV;
	
	/* set current for fast charing. */
	rc = smb346_set_cccv(msm_chg);
	if (rc){
		ERR_CHG("%s:smb346_set_set_charge_usb failed\n", __func__);
		return rc;	
	}	
	return 0;	
}

static int smb346_set_charge_nonstanard(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;	
	pr_info("%s\n", __func__);
	
	msm_chg->cc_step_curr = FAST_CHG_1000MA;
	msm_chg->aicl_set_val = USBIN_DCIN_INPUT_CURRENT_500MA;
	msm_chg->flt_set_val = FLOAT_VOTAGE_4220MV;
	
	/* set current for fast charing. */
	rc = smb346_set_cccv(msm_chg);
	if (rc){
		ERR_CHG("%s:smb346_set_set_charge_nonstandard failed\n", __func__);		
		return rc;	
	}	
	return 0;	
}

static int smb346_set_charge_accessory(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;	
	pr_info("%s\n", __func__);
	
	msm_chg->cc_step_curr = FAST_CHG_1000MA;
	msm_chg->aicl_set_val = USBIN_DCIN_INPUT_CURRENT_500MA;
	msm_chg->flt_set_val = FLOAT_VOTAGE_4220MV;
	
	/* set current for fast charing. */
	rc = smb346_set_cccv(msm_chg);
	if (rc){
		ERR_CHG("%s:smb346_set_set_charge_accessory failed\n", __func__);		
		return rc;	
	}	
	
	return 0;	
}

static int smb346_set_charge_mhl__stanard(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;	
	pr_info("%s\n", __func__);
	
	msm_chg->cc_step_curr = FAST_CHG_1000MA;
	msm_chg->aicl_set_val = USBIN_DCIN_INPUT_CURRENT_700MA;
	msm_chg->flt_set_val = FLOAT_VOTAGE_4220MV;
	
	/* set current for fast charing. */
	rc = smb346_set_cccv(msm_chg);
	if (rc){
		ERR_CHG("%s:smb346_set_set_charge_nonstandard failed\n", __func__);		
		return rc;	
	}	
	return 0;	
}

static int smb346_set_charge_mhl__nonstanard(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;	
	pr_info("%s\n", __func__);
	
	msm_chg->cc_step_curr = FAST_CHG_1000MA;
	msm_chg->aicl_set_val = USBIN_DCIN_INPUT_CURRENT_500MA;
	msm_chg->flt_set_val = FLOAT_VOTAGE_4220MV;
	
	/* set current for fast charing. */
	rc = smb346_set_cccv(msm_chg);
	if (rc){
		ERR_CHG("%s:smb346_set_set_charge_nonstandard failed\n", __func__);		
		return rc;	
	}	
	
	return 0;	
}

static int smb346_set_charge_mhl(struct smb346_charger_mux *msm_chg)
{
	int rc = 0;	
	pr_info("%s\n", __func__);
	
	/*wait 1500ms for stanard mhl device */
    if (!wait_for_completion_timeout(&stanard_mhl_wait,
		 msecs_to_jiffies(1500))){
		  if (is_chg_plugged_in()){  
			 rc = smb346_set_charge_mhl__nonstanard(msm_chg);
		     if(rc)
				   pr_err("%s: nonstanard set failed!\n",__func__);
		  }
           pr_info("Timed out waiting for stanard mhl wait\n");
      }else{
		   if (is_chg_plugged_in()){  
			  rc = smb346_set_charge_mhl__stanard(msm_chg);
			  if(rc)
					pr_err("%s: stanard set failed!\n",__func__);
		   }
		   
           pr_info("stanard mhl device\n");
     }

	 return rc;
}

int smb346_mhl_stanard_charge(void)
{
	complete(&stanard_mhl_wait);
    pr_info("%s: stanard mhl charge set!\n",__func__);
	return 0;
}

EXPORT_SYMBOL(smb346_mhl_stanard_charge);

/*
*  this function use when FTM mode only
*/
static int smb346_suspend_mode_on(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	pr_info("%s,ftm mode\n", __func__);

    //12001 PVT hw can not suspend charge,so that set input limit current to 300mA
	/* set Input current limit. */
	rc = smb346_update(msm_chg, INPUT_CURRENT_LIMIT_REGISTER, USBIN_DCIN_INPUT_CURRENT_300MA, USBIN_DCIN_INPUT_CURRENT_LIMIT_MASK);
	if (rc){
		pr_err("%s: INPUT_CURRENT_LIMIT_REGISTER set failed!\n",__func__);
		return rc;
	}

	#ifndef FEATURE_SUSPEND_CHARGE
	/* active suspend mode */
	rc = smb346_update(msm_chg, STATUS_REGISTER_B, USB_SUSPEND_MODE__ACTIVE, USB_SUSPEND_MODE__ACTIVE);	
	if (rc){
		pr_err("%s: STATUS_REGISTER_B set failed!\n",__func__);
		return rc;
	}

	#endif

	/* write register to enable suspend mode, shutdown sys power. */
	rc = smb346_update(msm_chg, COMMAND_REGISTER_A, SUSPEND_MODE_ENABLED, SUSPEND_MODE_ENABLED);
	if (rc){
		pr_err("%s: COMMAND_REGISTER_A set failed!\n",__func__);
		return rc;
	}

	msm_chg->mIsSuspendOn = true;
	smb346_charger_suspend_mode_set(msm_chg, true);
	return 0;
}

static int smb346_suspend_mode_off(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	pr_info("%s\n", __func__);

	/* write register to enable suspend mode, shutdown sys power. */
	rc = smb346_update(msm_chg, COMMAND_REGISTER_A, SUSPEND_MODE_DISABLED, SUSPEND_MODE_ENABLED);
	if (rc)
		return rc;

	#ifndef FEATURE_SUSPEND_CHARGE
	/* disable suspend mode */
	rc = smb346_update(msm_chg, STATUS_REGISTER_B, USB_SUSPEND_MODE__NOT_ACTIVE, 
		USB_SUSPEND_MODE__ACTIVE);	
	if (rc)
		return rc;
	#endif

	msm_chg->mIsSuspendOn = false;	
	smb346_charger_suspend_mode_set(msm_chg, false);
	smb346_charging_status_set(msm_chg, SMB_CHG_STATUS__NOT_CHARGING);
	smb346_charger_relative_battery_status_set(msm_chg, POWER_SUPPLY_STATUS_NOT_CHARGING);
	return 0;
}

static void smb346_usb_suspend_mode(struct smb346_charger_mux *msm_chg, bool on)
{
      #if 1
	  //delete sys suspend charge
	  //TODO
	  return;
      #else
	pr_info("%s FTM MODE ### change suspend status =%d\n", __func__,on);

	if (on){
		pr_info("%s: usb charging suspend mode on\n",__func__);
		msm_charger_notify_event(msm_chg, SYSTEM_STATUS_CHANGED__USB_SUSPEND_ON);
	}else{
	    pr_info("%s: usb charging suspend mode off\n",__func__);
		msm_charger_notify_event(msm_chg, SYSTEM_STATUS_CHANGED__USB_SUSPEND_OFF);
	}
	#endif
}

static void smb346_ftm_charge_suspend_mode(bool on)
{
	pr_info("%s FTM MODE , change charge status =%d\n", __func__,on);

	if (on){
		pr_info("%s: ftm charging suspend mode on\n",__func__);
		if(usb_msm_chg){
		msm_charger_notify_event(usb_msm_chg, SYSTEM_STATUS_CHANGED__USB_SUSPEND_ON);
		}else{
              	pr_err("%s: usb_msm_chg is NULL\n",__func__);
		}
	}else{
	       pr_info("%s: ftm charging suspend mode off\n",__func__);
		if(usb_msm_chg){
			msm_charger_notify_event(usb_msm_chg, SYSTEM_STATUS_CHANGED__USB_SUSPEND_OFF);
		}else{
			pr_err("%s: usb_msm_chg is NULL\n",__func__);
		}
	}
}

static int smb346_pause_charging(struct smb346_charger_mux *msm_chg)
{
	int rc  = -1;
	DEBUG_CHG("%s\n", __func__);
	
	/* write EN register to stop charging. */
	rc = smb346_update(msm_chg, COMMAND_REGISTER_A, CHARGING_DISABLED, CHARGEING_ENABLED);
	if (rc)
		return rc;
	return 0;	
}

static int smb346_recov_charging(struct smb346_charger_mux *msm_chg)
{
	int rc  = -1;
	DEBUG_CHG("%s\n", __func__);
	
	/* write EN register to start charging. */
	rc = smb346_update(msm_chg, COMMAND_REGISTER_A, CHARGEING_ENABLED, CHARGEING_ENABLED);
	if (rc)
		return rc;
	return 0;
}
static bool no_charging_while_calling = false;
static bool restore_charging_when_calling_ends = false;
static bool calling_ends = false;
static int smb346_start__charging(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;	
	ERR_CHG("%s:\n", __func__);

	if (no_charging_while_calling){
		ERR_CHG("%s:no charging when calling with battery voltage high enough\n", __func__);
		restore_charging_when_calling_ends = true;
		return rc;
	}
	/* init register value. */
	rc = smb346_reg_init_config(msm_chg);
	if (rc){
		pr_err("%s: init reg config fail!\n",__func__);
		goto out;
	}

	pr_info("%s: Start charging>>> \n",__func__);
	
	if (smb346_battery_adc_params.vchg_mv >= CHARGER_SOFT_HVP_VOLTAGE) {
		rc = smb346_set_charge_hvp(msm_chg);
		if (rc)
			goto out;
	}else {
		switch (smb346_charger_type_get(msm_chg))
		{
			case CHARGER_TYPE_USB_WALL:
			{
				rc = smb346_set_charge_ac(msm_chg);
				break;
			}
			case CHARGER_TYPE_USB_PC:
			{
				rc = smb346_set_charge_usb(msm_chg);
				break;
			}
			case CHARGER_TYPE_NON_STANDARD:
			{
				rc = smb346_set_charge_nonstanard(msm_chg);
				break;
			}
			case CHARGER_TYPE_USB_HDMI:
			{
				rc = smb346_set_charge_mhl(msm_chg);
				break;
			}
			case CHARGER_TYPE_ACCESSORY:
			{
				rc = smb346_set_charge_accessory(msm_chg);
				break;
			}
			default:
			{
				rc = -1;
				ERR_CHG("invalid charger type!\n");
				break;
			}
		}
		
		if (rc)
			goto out;
	}
	
	if (smb346_charger_suspend_mode_get(msm_chg) == true){
		rc = smb346_suspend_mode_off(msm_chg);
		if (rc)
			goto out;
	}

	/*start charging en gpio start charging*/
	rc = smb346_recov_charging(msm_chg);
	if (rc)
		goto out;
	else{
		/* start toec timer. */
		add_listener_for_max_charge_time(msm_chg);	
              #ifdef INTERVAL_CHARGING_FEATURE
		/* charging for cc_chg_detect_time */
		add_listener_for_interval_charging(msm_chg);
		#endif
		smb346_charger_relative_battery_status_set(msm_chg, POWER_SUPPLY_STATUS_CHARGING);
		smb346_charging_status_set(msm_chg, SMB_CHG_STATUS__CHARGING);
		
		/* enbale irq when charger attached and start charging. */
		//enable_irq(smb346_chg.smb346_irq );
	}
out:
	//enable_irq(msm_chg->client->irq);
	return rc;
}
static int smb346_start_charging_for_restore_from_bad_temp(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;	
	pr_info("%s\n", __func__);
	if (smb346_charger_type_get(msm_chg) == CHARGER_TYPE_INVALID){
		ERR_CHG("%s:charger is invalid \n", __func__);
		return 0;
	}
	/* init register value. */
	rc = smb346_reg_init_config(msm_chg);
	if (rc){
		pr_err("%s: smb346 reg init config failed!\n",__func__);
		goto out;
	}

	if (smb346_battery_adc_params.vchg_mv >= CHARGER_SOFT_HVP_VOLTAGE) {
		rc = smb346_set_charge_hvp(msm_chg);
		if (rc)
			goto out;
	}
	else {
		switch (smb346_battery_temp_region_get(msm_chg))
		{
		    case CV_BATTERY_TEMP_REGION_LITTLE__COLD:
			{
				rc = smb346_set_charge_little_cold(msm_chg);
				break;
			}
			case CV_BATTERY_TEMP_REGION__COOL:
			{
				rc = smb346_set_charge_cool(msm_chg);
				break;
			}
			case CV_BATTERY_TEMP_REGION__WARM:
			{
				rc = smb346_set_charge_warm(msm_chg);
				break;
			}
			case CV_BATTERY_TEMP_REGION__NORMAL:
			{
				rc = smb346_set_cccv(msm_chg);
				break;
			}
			default:
			{
				rc = -1;
				ERR_CHG("invalid charger type!\n");
				break;
			}
		}
		
		if (rc)
			goto out;
	}
	
	if (smb346_charger_suspend_mode_get(msm_chg) == true){
		rc = smb346_suspend_mode_off(msm_chg);
		if (rc)
			goto out;
	}
	
	rc = smb346_recov_charging(msm_chg);
	if (rc)
		goto out;
	else{

		/* start toec timer. */
		add_listener_for_max_charge_time(msm_chg);	
  
		/* charging for cc_chg_detect_time */
		#ifdef INTERVAL_CHARGING_FEATURE
		add_listener_for_interval_charging(msm_chg);
		#endif
		smb346_charger_relative_battery_status_set(msm_chg, POWER_SUPPLY_STATUS_CHARGING);
		smb346_charging_status_set(msm_chg, SMB_CHG_STATUS__CHARGING);

		/* enbale irq when charger attached and start charging. */
		//enable_irq(smb346_chg.smb346_irq );
	}
out:
	//enable_irq(smb346_chg.smb346_irq );
	return rc;
}
static int smb346_start_resume_charging(struct smb346_charger_mux *msm_chg)
{
	return smb346_start_charging_for_restore_from_bad_temp(msm_chg);
}
static int smb346_stop__charging(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	ERR_CHG("%s:\n", __func__);
	pr_info("%s: stop charging>> \n",__func__);
	#ifdef INTERVAL_CHARGING_FEATURE
	remove_listener_for_interval_charging(msm_chg);
	#endif
	remove_listener_for_max_charge_time(msm_chg);
	remove_listener_for_charge_complete_detect(msm_chg);
	   
	#ifndef FEATURE_AUTO_RECHARGE
	#if 0//this status is only updated in add/remove_listener_for_charge_resume func
	msm_chg->stop_resume_check = 1;
	#endif
	
	remove_listener_for_charge_resume(msm_chg);
	#endif

	rc = smb346_pause_charging(msm_chg);
	if (rc){
		ERR_CHG("%s:smb346_pause_charging failed\n", __func__);
		return rc;	
	}else{
		//disable_irq(smb346_chg.smb346_irq);
		smb346_charging_status_set(msm_chg, SMB_CHG_STATUS__NOT_CHARGING);
		smb346_charger_relative_battery_status_set(msm_chg, POWER_SUPPLY_STATUS_NOT_CHARGING);
		//smb346_battery_adc_params.ichg_mv = 0;
	}
	
	return 0;
	
}

static bool is_suspend_threhold(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	DEBUG_CHG("%s\n", __func__);
	
	rc = get_batt_adc(msm_chg);
	if (rc)
		return false;
	
	if (smb346_battery_adc_params.current_batt_adc_voltage > SUSPEND_MEET_THREHOLD) {
		DEBUG_CHG("suspend threhold meet!\n");
		return true;
	}
	else {
		DEBUG_CHG("suspend threhold does not meet!\n");
		return false;
	}
}

 /*Tbatt <-10C*/
static int handle_batt_temp_cold(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	// msm_chg->mBatteryTempRegion
	if (smb346_battery_temp_region_get(msm_chg) != CV_BATTERY_TEMP_REGION__COLD)
	{
		ERR_CHG("%s\n", __func__);
		
		rc = smb346_stop__charging(msm_chg);
		if (rc)
			return rc;
		else
			smb346_charging_stopped_for_battery_bad_temp_set(msm_chg, true);
		
		/* Update the temperature boundaries */
		msm_chg->mBatteryTempBoundT0 = AUTO_CHARGING_BATT_TEMP_T0 + AUTO_CHARGING_BATTERY_TEMP_HYST_FROM_COLD_TO_COOL;
		msm_chg->mBatteryTempBoundT1 = AUTO_CHARGING_BATT_TEMP_T1 + AUTO_CHARGING_BATTERY_TEMP_HYST_FROM_COLD_TO_COOL;
		msm_chg->mBatteryTempBoundT2 = AUTO_CHARGING_BATT_TEMP_T2 + AUTO_CHARGING_BATTERY_TEMP_HYST_FROM_COOL_TO_NORMAL;
		msm_chg->mBatteryTempBoundT3 = AUTO_CHARGING_BATT_TEMP_T3;
		msm_chg->mBatteryTempBoundT4 = AUTO_CHARGING_BATT_TEMP_T4;

		/* Update battery temp region */
		smb346_battery_temp_region_set(msm_chg, CV_BATTERY_TEMP_REGION__COLD);
		smb346_battery_status_set(msm_chg, BATTERY_STATUS_BAD_TEMP);
		smb346_battery_level_set(msm_chg, BATTERY_LEVEL_DEAD);
		smb346_battery_health_set(msm_chg, POWER_SUPPLY_HEALTH_COLD);
	}

	return 0;
}

 /* -10 C <=Tbatt <= 0C*/
static int handle_batt_temp_little_cold(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	chg_cv_battery_temp_region_type batt_temp_region_pre;
	// msm_chg->mBatteryTempRegion
	if (smb346_battery_temp_region_get(msm_chg) != CV_BATTERY_TEMP_REGION_LITTLE__COLD){
		pr_info("%s\n", __func__);
		
		batt_temp_region_pre = smb346_battery_temp_region_get(msm_chg);
		
		/* Update battery temp region */
		smb346_battery_temp_region_set(msm_chg, CV_BATTERY_TEMP_REGION_LITTLE__COLD);

		//if temp from cold or hot to cool then start charging again
		if(batt_temp_region_pre == CV_BATTERY_TEMP_REGION__COLD || batt_temp_region_pre == CV_BATTERY_TEMP_REGION__HOT){
			rc = smb346_start_charging_for_restore_from_bad_temp(msm_chg);
			if (rc)
				goto failed;
			else
				smb346_charging_stopped_for_battery_bad_temp_set(msm_chg, false);
		}
		else{
			rc = smb346_set_charge_little_cold(msm_chg);
			if (rc){
				ERR_CHG("%s:smb346_set_charg_cool_warm failed\n", __func__);
				goto failed;
			}
		}
		/* Update the temperature boundaries */
		msm_chg->mBatteryTempBoundT0 = AUTO_CHARGING_BATT_TEMP_T0;
		msm_chg->mBatteryTempBoundT1 = AUTO_CHARGING_BATT_TEMP_T1 + AUTO_CHARGING_BATTERY_TEMP_HYST_FROM_COOL_TO_NORMAL;
		msm_chg->mBatteryTempBoundT2 = AUTO_CHARGING_BATT_TEMP_T2 + AUTO_CHARGING_BATTERY_TEMP_HYST_FROM_COOL_TO_NORMAL;
		msm_chg->mBatteryTempBoundT3 = AUTO_CHARGING_BATT_TEMP_T3;
		msm_chg->mBatteryTempBoundT4 = AUTO_CHARGING_BATT_TEMP_T4;

		smb346_battery_status_set(msm_chg, BATTERY_STATUS_GOOD);
		smb346_battery_level_set(msm_chg, BATTERY_LEVEL_GOOD);
		smb346_battery_health_set(msm_chg, POWER_SUPPLY_HEALTH_GOOD);

	}
	return 0;
failed:
	smb346_battery_temp_region_set(msm_chg, batt_temp_region_pre);
	return rc;
}
 
 /* 0 C <Tbatt <= 10C*/
static int handle_batt_temp_cool(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	chg_cv_battery_temp_region_type batt_temp_region_pre;
	// msm_chg->mBatteryTempRegion
	if (smb346_battery_temp_region_get(msm_chg) != CV_BATTERY_TEMP_REGION__COOL){
		pr_info("%s\n", __func__);
		
		batt_temp_region_pre = smb346_battery_temp_region_get(msm_chg);
		
		/* Update battery temp region */
		smb346_battery_temp_region_set(msm_chg, CV_BATTERY_TEMP_REGION__COOL);

		//if temp from cold or hot or little cold to cool then start charging again
		if(batt_temp_region_pre == CV_BATTERY_TEMP_REGION__COLD || batt_temp_region_pre == CV_BATTERY_TEMP_REGION__HOT){
			rc = smb346_start_charging_for_restore_from_bad_temp(msm_chg);
			if (rc)
				goto failed;
			else
				smb346_charging_stopped_for_battery_bad_temp_set(msm_chg, false);
		}
		else{
			rc = smb346_set_charge_cool(msm_chg);
			if (rc){
				ERR_CHG("%s:smb346_set_charg_cool_warm failed\n", __func__);
				goto failed;
			}
		}
		/* Update the temperature boundaries */
		msm_chg->mBatteryTempBoundT0 = AUTO_CHARGING_BATT_TEMP_T0;
		msm_chg->mBatteryTempBoundT1 = AUTO_CHARGING_BATT_TEMP_T1;
		msm_chg->mBatteryTempBoundT2 = AUTO_CHARGING_BATT_TEMP_T2 + AUTO_CHARGING_BATTERY_TEMP_HYST_FROM_COOL_TO_NORMAL;
		msm_chg->mBatteryTempBoundT3 = AUTO_CHARGING_BATT_TEMP_T3;
		msm_chg->mBatteryTempBoundT4 = AUTO_CHARGING_BATT_TEMP_T4;

		smb346_battery_status_set(msm_chg, BATTERY_STATUS_GOOD);
		smb346_battery_level_set(msm_chg, BATTERY_LEVEL_GOOD);
		smb346_battery_health_set(msm_chg, POWER_SUPPLY_HEALTH_GOOD);

	}
	return 0;
failed:
	smb346_battery_temp_region_set(msm_chg, batt_temp_region_pre);
	return rc;
}
 
 /* 10 C <Tbatt <45C*/
static int handle_batt_temp_normal(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	chg_cv_battery_temp_region_type batt_temp_region_pre;
	// msm_chg->mBatteryTempRegion
	if (smb346_battery_temp_region_get(msm_chg) != CV_BATTERY_TEMP_REGION__NORMAL)
	{
              
	       pr_info("%s\n", __func__);	
		batt_temp_region_pre = smb346_battery_temp_region_get(msm_chg);
		/* Update battery temp region */
		smb346_battery_temp_region_set(msm_chg, CV_BATTERY_TEMP_REGION__NORMAL);

		if (batt_temp_region_pre == CV_BATTERY_TEMP_REGION__COLD ||batt_temp_region_pre == CV_BATTERY_TEMP_REGION__HOT){
			rc = smb346_start_charging_for_restore_from_bad_temp(msm_chg);
			if (rc)
				goto failed;	
			else
				smb346_charging_stopped_for_battery_bad_temp_set(msm_chg, false);
		}
		else{
			rc = smb346_set_cccv(msm_chg);
			if (rc){
				ERR_CHG("%s:smb346_set_cccv failed\n", __func__);
				goto failed;
			}
		}
		/* Update the temperature boundaries */
		msm_chg->mBatteryTempBoundT0 = AUTO_CHARGING_BATT_TEMP_T0;
		msm_chg->mBatteryTempBoundT1 = AUTO_CHARGING_BATT_TEMP_T1;
		msm_chg->mBatteryTempBoundT2 = AUTO_CHARGING_BATT_TEMP_T2;
		msm_chg->mBatteryTempBoundT3 = AUTO_CHARGING_BATT_TEMP_T3;
		msm_chg->mBatteryTempBoundT4 = AUTO_CHARGING_BATT_TEMP_T4;

		smb346_battery_status_set(msm_chg, BATTERY_STATUS_GOOD);
		smb346_battery_level_set(msm_chg, BATTERY_LEVEL_GOOD);
		smb346_battery_health_set(msm_chg, POWER_SUPPLY_HEALTH_GOOD);

	}
	return 0;
failed:
	smb346_battery_temp_region_set(msm_chg, batt_temp_region_pre);
	return rc;
}
 
 /* 45C <=Tbatt <=55C*/
static int handle_batt_temp_warm(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	chg_cv_battery_temp_region_type batt_temp_region_pre;
	//msm_chg->mBatteryTempRegion
	if(smb346_battery_temp_region_get(msm_chg) != CV_BATTERY_TEMP_REGION__WARM)
	{
		pr_info("%s\n", __func__);
		
		batt_temp_region_pre = smb346_battery_temp_region_get(msm_chg);
		/* Update battery temp region */
		smb346_battery_temp_region_set(msm_chg, CV_BATTERY_TEMP_REGION__WARM);

		if (batt_temp_region_pre == CV_BATTERY_TEMP_REGION__COLD ||batt_temp_region_pre == CV_BATTERY_TEMP_REGION__HOT){
			rc = smb346_start_charging_for_restore_from_bad_temp(msm_chg);
			if (rc)
				goto failed;	
			else
				smb346_charging_stopped_for_battery_bad_temp_set(msm_chg, false);

		}
		else{
			rc = smb346_set_charge_warm(msm_chg);
			if (rc){
				ERR_CHG("%s:smb346_set_charge_warm failed\n", __func__);
				goto failed;
			}
		}
		/* Update the temperature boundaries */
		msm_chg->mBatteryTempBoundT0 = AUTO_CHARGING_BATT_TEMP_T0;
		msm_chg->mBatteryTempBoundT1 = AUTO_CHARGING_BATT_TEMP_T1;
		msm_chg->mBatteryTempBoundT2 = AUTO_CHARGING_BATT_TEMP_T2;
		msm_chg->mBatteryTempBoundT3 = AUTO_CHARGING_BATT_TEMP_T3 - AUTO_CHARGING_BATTERY_TEMP_HYST_FROM_WARM_TO_NORMAL;
		msm_chg->mBatteryTempBoundT4 = AUTO_CHARGING_BATT_TEMP_T4;

		smb346_battery_status_set(msm_chg, BATTERY_STATUS_GOOD);
		smb346_battery_level_set(msm_chg, BATTERY_LEVEL_GOOD);
		smb346_battery_health_set(msm_chg, POWER_SUPPLY_HEALTH_GOOD);

	}
	return 0;	
failed:
	smb346_battery_temp_region_set(msm_chg, batt_temp_region_pre);
	return rc;
}
 
 /* 55C <Tbatt*/
static int handle_batt_temp_hot(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	//msm_chg->mBatteryTempRegion
	if(smb346_battery_temp_region_get(msm_chg) != CV_BATTERY_TEMP_REGION__HOT)
	{
		ERR_CHG("%s\n", __func__);
		//rc = smb346_pause_charging(msm_chg);
		rc = smb346_stop__charging(msm_chg);
		if (rc)
			return rc;
		else
			smb346_charging_stopped_for_battery_bad_temp_set(msm_chg, true);
		/* Update the temperature boundaries */
		msm_chg->mBatteryTempBoundT0 = AUTO_CHARGING_BATT_TEMP_T0;
		msm_chg->mBatteryTempBoundT1 = AUTO_CHARGING_BATT_TEMP_T1;
		msm_chg->mBatteryTempBoundT2 = AUTO_CHARGING_BATT_TEMP_T2;
		msm_chg->mBatteryTempBoundT3 = AUTO_CHARGING_BATT_TEMP_T3 - AUTO_CHARGING_BATTERY_TEMP_HYST_FROM_WARM_TO_NORMAL;
		msm_chg->mBatteryTempBoundT4 = AUTO_CHARGING_BATT_TEMP_T4 - AUTO_CHARGING_BATTERY_TEMP_HYST_FROM_HOT_TO_WARM;

		/* Update battery temp region */
		//msm_chg->mBatteryTempRegion = CV_BATTERY_TEMP_REGION__HOT;
		smb346_battery_temp_region_set(msm_chg, CV_BATTERY_TEMP_REGION__HOT);

		smb346_battery_status_set(msm_chg, BATTERY_STATUS_BAD_TEMP);
		smb346_battery_level_set(msm_chg, BATTERY_LEVEL_DEAD);
		smb346_battery_health_set(msm_chg, POWER_SUPPLY_HEALTH_OVERHEAT);
	}

	return 0;
}

static int handle_battery_removed(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;

       /* the battery was just removed */
	pr_info("%s,error battery is remove\n", __func__);
	/* if a battery has been removed, stop charging */
	rc = smb346_stop__charging(msm_chg);
	if (rc){
		pr_err("%s: error stop charging failed!!!\n",__func__);
		return rc;
	}
	//smb346_battery_valid_set(msm_chg, false);
	smb346_battery_status_set(msm_chg, BATTERY_STATUS_REMOVED);
	smb346_battery_level_set(msm_chg, BATTERY_LEVEL_DEAD);
	smb346_battery_health_set(msm_chg, POWER_SUPPLY_HEALTH_UNKNOWN);
	smb346_charging_stopped_for_battery_bad_state_set(msm_chg, true);

	return 0;
}

static int handle_battery_inserted(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;	

       /* The battery was just attached back */
 	pr_info("%s,battery is reinserted\n", __func__);
	/* if a battery has been inserted, start charging */
	rc = smb346_start__charging(msm_chg);
	if (rc){
		pr_info("%s: error,start charging failed!\n",__func__);
		return rc;
	}
	//smb346_battery_valid_set(msm_chg, true);
	smb346_charging_stopped_for_battery_bad_state_set(msm_chg, false);
	smb346_battery_status_set(msm_chg, BATTERY_STATUS_GOOD);
	smb346_battery_level_set(msm_chg, BATTERY_LEVEL_GOOD);
	smb346_battery_health_set(msm_chg, POWER_SUPPLY_HEALTH_GOOD);
#ifndef FEATURE_AUTO_RECHARGE
#if 0//this status is only updated in add/remove_listener_for_charg_resume func
		msm_chg->stop_resume_check = 0;
#endif
#endif		
	 return 0;
}

static int smb346_handle_charging_done(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	chg_info("%s,charge done handle@@\n", __func__);

#ifndef FEATURE_AUTO_RECHARGE
	pr_info("current_vbatt_voltage =%d\n", smb346_battery_adc_params.current_batt_adc_voltage);
	#ifdef FEATURE_SUSPEND_CHARGE
	/* fix can not auto resume charging fuction */
	#endif
	rc = smb346_stop__charging(msm_chg);
	if (rc){
		pr_err("%s: stop charging failed!\n",__func__);
		return rc;
	}else{
		chg_info("%s: charging stopped\n", __func__);
		/* schedule resume check */
		add_listener_for_charge_resume(msm_chg);
		msm_chg->mEndCVTopoffCounter = 0;
		smb346_charging_status_set(msm_chg, SMB_CHG_STATUS__CHARGING_COMPLETED);
		smb346_battery_level_set(msm_chg, BATTERY_LEVEL_FULL);
		smb346_charger_relative_battery_status_set(msm_chg, POWER_SUPPLY_STATUS_FULL);
		usb_chg_charging_complete();
	}
#else
	/* fix can not auto resume charging fuction */
	rc = smb346_recov_charging(msm_chg);
	if (rc)
		return rc;	
#endif

	return 0;
}

#ifdef FEATURE_AUTO_RECHARGE
static void handle_charging_resume(struct smb346_charger_mux *msm_chg)
{
	DEBUG_CHG("%s\n", __func__);

	/* start toec timer. */
	add_listener_for_max_charge_time(msm_chg);
	
	smb346_battery_level_set(msm_chg, BATTERY_LEVEL_GOOD);
	smb346_charger_relative_battery_status_set(msm_chg, POWER_SUPPLY_STATUS_CHARGING);
}
#endif

static void smb346_check_battery_ovp(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	int count = BAD_STATE_COUNT;
	int max_retry_count = 3;
	DEBUG_CHG("%s\n", __func__);

	while (count--)
	{
		if (smb346_battery_adc_params.current_batt_adc_voltage >= BATTERY_SOFT_OVP_VOLTAGE) { 
			msm_chg->mBadBatteryCounter++;
		}
		else {
			msm_chg->mBadBatteryCounter = 0;
		}
		
		if ( !msm_chg->mBadBatteryCounter){
			DEBUG_CHG("%s:battery voltage status is ok\n", __func__);
			smb346_battery_status_set(msm_chg, BATTERY_STATUS_GOOD);
			return;
		}
		
		rc = smb346_get_battery_adc_realtime(msm_chg);
		if (rc){
			count = BAD_STATE_COUNT;
			if (max_retry_count-- < 0)
				break;
		}
		msleep(10);
	}
	
	if (msm_chg->mBadBatteryCounter == BAD_STATE_COUNT){
		ERR_CHG("%s:battery is ovp\n", __func__);
		smb346_battery_status_set(msm_chg, BATTERY_STATUS_BAD);
		msm_chg->mBadBatteryCounter = 0;
	}
	return;
}

static void smb346_check_battery_uovp(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	int count = BAD_STATE_COUNT;
	int max_retry_count = 3;
	DEBUG_CHG("%s\n", __func__);

	while (count--)
	{
		if (smb346_battery_adc_params.current_batt_adc_voltage > BATTERY_SOFT_OVP_VOLTAGE) { 
			pr_info("%s:warning, battery is high more than 4500mv,=%d\n",__func__,msm_chg->mBadBatteryCounter);
			msm_chg->mBadBatteryCounter++;
		}
		else {
			msm_chg->mBadBatteryCounter = 0;
		}

		if ( !msm_chg->mBadBatteryCounter) {
			DEBUG_CHG("%s:battery voltage status is ok\n", __func__);
			smb346_battery_status_set(msm_chg, BATTERY_STATUS_GOOD);
			return;
		}
		
		rc = smb346_get_battery_adc_realtime(msm_chg);
		if (rc){
			count = BAD_STATE_COUNT;
			if (max_retry_count-- < 0)
				break;
		}
		msleep(200);
	}
	
	if (msm_chg->mBadBatteryCounter == BAD_STATE_COUNT){
		ERR_CHG("%s:battery mvolts is ovp\n", __func__);
		smb346_battery_status_set(msm_chg, BATTERY_STATUS_BAD);
		msm_chg->mBadBatteryCounter = 0;
	}
	else{
		smb346_battery_status_set(msm_chg, BATTERY_STATUS_GOOD);
	}
	return;
}
#if 0//this part will be used in the future
static int smb346_handle_charger_uvp(struct smb346_charger_mux *msm_chg)
{
}
static int smb346_handle_charger_ovp(struct smb346_charger_mux *msm_chg)
{

}
#endif

static int smb346_handle_battery_ovp(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	DEBUG_CHG("%s\n", __func__);
	/* if battery OVP, we need to disable battery-to-sys fet, power on charger */
	if (smb346_battery_status_get(msm_chg) == BATTERY_STATUS_BAD){
		if (SMB_CHG_STATUS__CHARGING == smb346_charging_status_get(msm_chg)){
			rc = smb346_stop__charging(msm_chg);
			if (rc)
				return rc;
			smb346_battery_health_set(msm_chg, POWER_SUPPLY_HEALTH_OVERVOLTAGE);
			smb346_battery_level_set(msm_chg, BATTERY_LEVEL_DEAD);
			smb346_charging_stopped_for_battery_bad_state_set(msm_chg, true);
		}
	}
	else
		smb346_charging_stopped_for_battery_bad_state_set(msm_chg, false);
	return 0;
}

static int smb346_handle_battery_restore_from_uovp(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	DEBUG_CHG("%s\n", __func__);
	if (smb346_battery_status_get(msm_chg) == BATTERY_STATUS_GOOD){
		rc = smb346_start__charging(msm_chg);
		if (rc)
			return rc;
		smb346_battery_health_set(msm_chg, POWER_SUPPLY_HEALTH_GOOD);
		smb346_battery_level_set(msm_chg, BATTERY_LEVEL_GOOD);
		smb346_charging_stopped_for_battery_bad_state_set(msm_chg, false);
	}
	
	return rc;
}
static int smb346_handle_battery_uovp(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	DEBUG_CHG("%s\n", __func__);

/* in charger err , stop charging first */
	rc = smb346_stop__charging(msm_chg);
	if (rc)
		return rc;
	
/* if battery OVP, we need to disable battery-to-sys fet, power on charger */
	if (smb346_battery_status_get(msm_chg) == BATTERY_STATUS_BAD){
		smb346_battery_health_set(msm_chg, POWER_SUPPLY_HEALTH_OVERVOLTAGE);
		smb346_battery_level_set(msm_chg, BATTERY_LEVEL_DEAD);
		smb346_charging_stopped_for_battery_bad_state_set(msm_chg, true);
	}
	else
		smb346_charging_stopped_for_battery_bad_state_set(msm_chg, false);
	
	return 0;
}

static int smb346_handle_charger_uovp(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	DEBUG_CHG("%s\n", __func__);

/* in charger err , stop charging first */
	rc = smb346_stop__charging(msm_chg);
	if (rc)
		return rc;
	
/* if charger neither OVP/UVP or can not support charing current, we need to shutdown charger */
	if (smb346_charger_status_get(msm_chg) == CHARGER_STATUS_BAD){
		smb346_charging_stopped_for_charger_bad_state_set(msm_chg, true);
		smb346_charger_relative_battery_status_set(msm_chg, POWER_SUPPLY_STATUS_DISCHARGING);
	}
	else
		smb346_charging_stopped_for_charger_bad_state_set(msm_chg, false);

	
	return 0;
}


#ifdef FEATURE_TAPPER_ADJUST_CURRENT
static int handle_tapper_mode(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;	
	DEBUG_CHG("%s\n, cc_step_curr = %d", __func__, msm_chg->cc_step_curr);
	
	if(msm_chg->cc_step_curr <= 0){
		#ifdef INTERVAL_CHARGING_FEATURE
		remove_listener_for_interval_charging(msm_chg);
		#endif
		return 0;
	}//msm_chg->mBatteryTempRegion
	if ((msm_chg->cc_step_curr > 0) && 
		/* fix bug: adjust current in condition cool and warm temp */
		(smb346_battery_temp_region_get(msm_chg) == CV_BATTERY_TEMP_REGION__NORMAL)) {
		if (msm_chg->mTapperCounter < 2)
			msm_chg->mTapperCounter++;
		else {
			ERR_CHG("adjust current for tapper charging mode!\n");
			msm_chg->cc_step_curr--;
			ERR_CHG("%s:cc_step_curr = %d\n", __func__, msm_chg->cc_step_curr);
			rc = smb346_set_cccv(msm_chg);
			if (rc)
				return rc;
			msm_chg->mTapperCounter = 0;
		}
	}

	return 0;
}
#endif

/*check for smb346 37h bit1 and bit 0 to make sure charge done*/
static bool smb346_termination_charging_current_hit_irq(struct smb346_charger_mux *msm_chg)
{
    BYTE temp=0;
	
	smb346_read(msm_chg, INTERRUPT_REGISTER_C, &temp);
	
	pr_info("%s:reg(0x%x) = 0x%x\n",__func__, INTERRUPT_REGISTER_C, temp);
	if((temp & TERMINATION_CHARGING_CURRENT_HIT_IRQ) && (temp & TERMINATION_CHARGING_CURRENT_HIT_STATUS) ){
		 return true;
	}

	return false;
}

static int charge_complete_detect(struct smb346_charger_mux *msm_chg) 
{

	int ret = -1;
	int charge_current = 0;
	static int charge_end_counter = 0;
	DEBUG_CHG("%s\n", __func__);
	
	charge_current = smb346_battery_adc_params.ichg_mv;
	
	charge_end_counter++;
	if (charge_current < CHARGE_TERMINATION_CURR){
		msm_chg->mEndCVTopoffCounter++;
	}
	
	pr_info("%s: mEndCVTopoffCounter %d times in %d total(current(%dmA) < 100ma)\n",
		__func__, msm_chg->mEndCVTopoffCounter, charge_end_counter,charge_current);
	
	if (msm_chg->mEndCVTopoffCounter >= CHARGE_END_COUNT){
		    if(smb346_battery_temp_region_get(msm_chg) == CV_BATTERY_TEMP_REGION__NORMAL)
			{
				/*check Termination charging current hit irq when NORMAL temperate mode 2012-05-14*/
				if(smb346_termination_charging_current_hit_irq(msm_chg)){
					ret = smb346_handle_charging_done(msm_chg);
					if (ret)
					return ret;
					
			    }else{
			    	/*reset counter*/
					charge_end_counter = 0;
					msm_chg->mEndCVTopoffCounter = 0;
			    }
		    }else{
		    	ret = smb346_handle_charging_done(msm_chg);
				if (ret)
				return ret;
		    }
	}
	
	if (charge_end_counter >= CHARGE_END_MAX_COUNT){
		/*reset counter*/
		charge_end_counter = 0;
		msm_chg->mEndCVTopoffCounter = 0;
	}
	return 0;

}

#ifdef DCIN_CHARGE_FEATURE
void smb346_chg_connected(enum chg_type chgtype);

/*start handle accessory battery charge*/
static int accessory_charge_handle(struct smb346_charger_mux *msm_chg,bool connect)
{
    int vchg_mv;
	int rc = -1;
	uint8_t temp;
	chg_charger_hardware_type charger__type;
	bool usb_status = false;

	pr_info("%s connect:%d\n", __func__, connect);

	rc = smb346_read(msm_chg, STATUS_REGISTER_E, &temp);
	if (rc){
		pr_err("%s:read STATUS_REGISTER_E failed!!! \n",__func__);
		return -EAGAIN;
	}
		
	pr_info("%s:reg(0x%x) = 0x%x\n",__func__,STATUS_REGISTER_E, temp);
	
	//DCIN is exist ,check vchg mvolts to make sure is DCIN input source or USBIN input source
	vchg_mv = get_prop_vchg_movlts();
	if(USBIN_VALID_MVOLTS_MIN <= vchg_mv  && vchg_mv <= USBIN_VALID_MVOLTS_MAX)
	{  
	    pr_info("%s: input source #USBIN (vchg_mv =%dmv)\n",__func__,vchg_mv);
	    return 0;
	}else{
	    charger__type = get_charger_type();
	    pr_info("%s: input source #DCIN (vchg_mv =%dmv),%d\n",__func__,vchg_mv,charger__type);
		if(connect) {
			if(charger__type == CHARGER_TYPE_INVALID || charger__type == CHARGER_TYPE_NONE) {
				usb_status = get_usb_chg_status();

				if(!usb_status && 
					smb346_charger_info.battery_level != BATTERY_LEVEL_FULL){
					set_origin_main_capacity();
				    pr_info("Start Accessory charging!!\n");
					smb346_chg_connected(USB_CHG_TYPE__ACCESSORY);
		        }
			}
		}
		else {
			if(charger__type == CHARGER_TYPE_ACCESSORY){
				pr_info("Stop Accessory charging !!\n");
				smb346_chg_connected(USB_CHG_TYPE__INVALID);
	        }
		}
	}
	
	return 0;
}

static void handle_event__mpp11_irq_triggered(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	BYTE reg_interrupt_status_0x39;
	BYTE reg_interrupt_status_0x3a;

	pr_info("%s:smb346 mpp11 plugin interrupt>>>\n",__func__);
	
	rc = smb346_read(msm_chg, INTERRUPT_REGISTER_E,&reg_interrupt_status_0x39);
	if (rc){
		pr_err("%s: read INTERRUPT_REGISTER_E failed\n",__func__);
		return;
	}
	
    rc = smb346_read(msm_chg, INTERRUPT_REGISTER_F,&reg_interrupt_status_0x3a);
	if (rc){
		pr_err("%s: read INTERRUPT_REGISTER_F failed\n",__func__);
		return;
	}

	/* DCIN charging check */
	#ifdef DCIN_CHARGE_FEATURE
	pr_info("%s:Accessory charging: power ok irq! 0x3a = 0x%x ,0x39 = 0x%x\n",
			  __func__,reg_interrupt_status_0x3a,reg_interrupt_status_0x39);
	if (reg_interrupt_status_0x3a & POWER_OK_STATUS)
	{
		/*input source is DCIN, start with accessory battery charge.
		   DCIN plugin @USBIN_UNDER_VOLTAGE_STATUS 1 and DCIN_UNDER_VOLTAGE_STATUS 0
		   USBIN plugin @USBIN_UNDER_VOLTAGE_STATUS 0 and DCIN_UNDER_VOLTAGE_STATUS 0
		   DCIN or USBIN remove @USBIN_UNDER_VOLTAGE_STATUS 1 and DCIN_UNDER_VOLTAGE_STATUS 1*/
				if ((reg_interrupt_status_0x39 & USBIN_UNDER_VOLTAGE_STATUS) &&
				    (reg_interrupt_status_0x39 & DCIN_UNDER_VOLTAGE_STATUS)){
						rc = accessory_charge_handle(msm_chg,false);
						if (rc)
							return;
				}else if((reg_interrupt_status_0x39 & USBIN_UNDER_VOLTAGE_STATUS)&&
				     (~reg_interrupt_status_0x39 & DCIN_UNDER_VOLTAGE_STATUS)){
					rc = accessory_charge_handle(msm_chg,true);
					if (rc)
						return;
				}else if((~reg_interrupt_status_0x39 & USBIN_UNDER_VOLTAGE_STATUS)&&
				     (reg_interrupt_status_0x39 & DCIN_UNDER_VOLTAGE_STATUS)){
					rc = accessory_charge_handle(msm_chg,true);
					if (rc)
						return;
				}
	}
	else {
		if(smb346_charger_type_get(msm_chg) == CHARGER_TYPE_ACCESSORY ) {
			rc = accessory_charge_handle(msm_chg,false);
			if (rc)
				return;
		}
	}
	#endif

	return;
}

#endif
static void handle_event__irq_triggered(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	BYTE reg_irq[6];
	BYTE reg_interrupt_status_0x35;
	BYTE reg_interrupt_status_0x36;
	BYTE reg_interrupt_status_0x37;
	BYTE reg_interrupt_status_0x38;
	BYTE reg_interrupt_status_0x39;
	BYTE reg_interrupt_status_0x3a;

	pr_info("%s:smb346 abnormity interrupt>>>\n",__func__);
	
	rc = smb346_rx_irq_regs(msm_chg, reg_irq);
	if (rc)
		goto out;
	
	/* interrupt status register A */
	reg_interrupt_status_0x35 = reg_irq[0];
	/* not in use */
	if (reg_interrupt_status_0x35 & HOT_TEMPERATURE_HARD_LIMIT_IRQ){
		DEBUG_CHG("hot temperature hard limit irq!\n");
	}
	/* not in use */
	if (reg_interrupt_status_0x35 & COLD_TEMPERATURE_HARD_LIMIT_IRQ){
		DEBUG_CHG("cold temperature hard limit irq!\n");
	}
	/* not in use */
	if (reg_interrupt_status_0x35 & HOT_TEMPERATURE_SOFT_LIMIT_IRQ){
		DEBUG_CHG("hot temperature soft limit irq!\n");
	}	
	/* not in use */
	if (reg_interrupt_status_0x35 & COLD_TEMPERATURE_SOFT_LIMIT_IRQ){
		DEBUG_CHG("cold temperature soft limit irq!\n");
	}

	/* interrupt status register B */
	reg_interrupt_status_0x36 = reg_irq[1];
	if (reg_interrupt_status_0x36 & BATTERY_OVER_VOLTAGE_CONDITION_IRQ){
		pr_err("%s:battery over-voltage irq!\n",__func__);
		smb346_check_battery_ovp(msm_chg);
		rc = smb346_handle_battery_ovp(msm_chg);
		if (rc)
			goto out;
	}
	
	if (reg_interrupt_status_0x36 & MISSING_BATTERY_IRQ){
		pr_err("%s:missing battery irq!\n",__func__);
		rc = get_batt_adc(msm_chg);
		if (rc)
			goto out;
		
		/* fix bug: sometimes reports wrong missing battery irq */
		if (!smb346_is_battery_present()){
			smb346_battery_valid_set(msm_chg, false);
			//kernel_power_off();
		}
		if (smb346_battery_adc_params.current_batt_adc_voltage < MIISS_BATTTERY_VOLTAGE){
			ERR_CHG("%s:calling kernel_power_off \n", __func__);
			//kernel_power_off();
		}
	}
	
	/* not in use */
	if (reg_interrupt_status_0x36 & LOW_BATTERY_VOLTAGE_IRQ){
		DEBUG_CHG("low battery voltage irq!\n");
	}	
	/* not in use */
	if (reg_interrupt_status_0x36 & PRE_TO_FAST_CHARGE_BATTERY_VOLTAGE_IRQ){
		DEBUG_CHG("pre_to_fast charge battery voltage irq!\n");
	}

	/* interrupt status register C */
	reg_interrupt_status_0x37 = reg_irq[2];
	/* not in use */
	if (reg_interrupt_status_0x37 & INTERNAL_TEMPERATURE_LIMIT_IRQ){
		DEBUG_CHG("internal temperature limit irq!\n");
	}
	if (reg_interrupt_status_0x37 & RE_CHARGE_BATTERY_THRESHOLD_IRQ){
		ERR_CHG("recharge battery threshold irq!\n");
		#ifdef FEATURE_AUTO_RECHARGE
		handle_charging_resume(msm_chg);
		#endif
	}	
	if (reg_interrupt_status_0x37 & TAPER_CHARGE_MODE_IRQ){
		DEBUG_CHG("tapper charging mode irq!\n");
		#ifdef FEATURE_TAPPER_ADJUST_CURRENT
		rc = handle_tapper_mode(msm_chg);
		if (rc)
			goto out;
		#endif
	}	

	#if 0
	if (reg_interrupt_status_0x37 & TERMINATION_CHARGING_CURRENT_HIT_IRQ){
		ERR_CHG("termination charging current hit irq!vbatt=%dmv\n",smb346_battery_adc_params.current_batt_adc_voltage);
#if 0
		rc = get_batt_adc(msm_chg);
		if (rc)
			goto out;
		
		/* fix bug: sometimes reports wrong termination charging irq */
		if (smb346_is_battery_present() &&
			smb346_battery_adc_params.current_batt_adc_voltage >= TAPPER_INTERVAL_CHG_VOLT){
			rc = smb346_handle_charging_done(msm_chg);
			if (rc)
				goto out;
		}	
#else
		rc = smb346_handle_charging_done(msm_chg);
		if (rc)
			goto out;
#endif
}
	#endif
	/* interrupt status register D */
	reg_interrupt_status_0x38 = reg_irq[3];
	/* not in use */
	if (reg_interrupt_status_0x38 & APSD_COMPLETED_IRQ){
		DEBUG_CHG("APSD complete irq!\n");
	}
	if (reg_interrupt_status_0x38 & AICL_COMPLETED_IRQ){
		DEBUG_CHG("AICL complete irq!\n");
		//smb346_charger_info.is_adapt_completed = true;
		//smb346_charger_info.is_adapt_completed = get_aicl_status(msm_chg);
	}	
	
	if (reg_interrupt_status_0x38 & COMPLETE_CHARGE_TIMEOUT_IRQ){
		ERR_CHG("complete charing timeout irq!\n");
		hrtimer_start(&msm_chg->max_charge_time_timer, ktime_set(0, CHG_POLL_DELAY), HRTIMER_MODE_REL);
	}	
	if (reg_interrupt_status_0x38 & PRE_CHARGE_TIMEOUT_IRQ){
		ERR_CHG("pre_to_fast charing timeout irq!\n");
		hrtimer_start(&msm_chg->max_charge_time_timer, ktime_set(0, CHG_POLL_DELAY), HRTIMER_MODE_REL);
	}

	/* interrupt status register E */
	reg_interrupt_status_0x39 = reg_irq[4];
	/* not in use */
	if (reg_interrupt_status_0x39 & DCIN_OVER_VOLTAGE_IRQ){
		DEBUG_CHG("DCIN over-voltage irq!\n");
	}	
	/* not in use */
	if (reg_interrupt_status_0x39 & DCIN_UNDER_VOLTAGE_IRQ){
		DEBUG_CHG("DCIN under-voltage irq!\n");
	}			
	if (reg_interrupt_status_0x39 & USBIN_OVER_VOLTAGE_IRQ){
		ERR_CHG("USBIN over-voltage irq!\n");
	}	
	if (reg_interrupt_status_0x39 & USBIN_UNDER_VOLTAGE_IRQ){
		ERR_CHG("USBIN under-voltage irq!\n");
	}

	/* interrupt status register F */
	reg_interrupt_status_0x3a = reg_irq[5];
	/* not in use */
	if (reg_interrupt_status_0x3a & OTG_OVER_CURRENT_LIMIT_IRQ){
		DEBUG_CHG("otg over current limit irq!\n");
	}	
	/* not in use */
	if (reg_interrupt_status_0x3a & OTG_BATTERY_UNDER_VOLTAGE_IRQ){
		DEBUG_CHG("otg battery under voltage irq!\n");
	}	
	/* not in use */
	if (reg_interrupt_status_0x3a & OTG_DETECTION_IRQ){
		DEBUG_CHG("otg detection irq!\n");
	}
	
out:
	//enable_irq(smb346_chg.smb346_irq );
	return;
}
static void check_aicl_complete(struct smb346_charger_mux *msm_chg)
{
	if (!get_aicl_status(msm_chg)) {
		if (msm_chg->mAICLCounter < BAD_STATE_COUNT) {
			msm_chg->mAICLCounter++;
		}
	}
	else {
		smb346_aicl_adapt_completed_set(msm_chg, true);
		msm_chg->mAICLCounter = 0;
	}

}
static int smb346_battery_temp_handle(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	int temperature = smb346_battery_adc_params.batt_temperature;
	DEBUG_CHG("%s: temperate =%dC,t0=%dC,t1=%dC,t2=%dC", 
		__func__,temperature,msm_chg->mBatteryTempBoundT0,msm_chg->mBatteryTempBoundT1,msm_chg->mBatteryTempBoundT2);
	DEBUG_CHG(" t3=%dC,t4=%dC,region =%d\n",msm_chg->mBatteryTempBoundT3,msm_chg->mBatteryTempBoundT4,msm_chg->mBatteryTempRegion);
        if(temperature < msm_chg->mBatteryTempBoundT0) /* battery is cold */
        {
                rc = handle_batt_temp_cold(msm_chg);
        } 
	 else if( (temperature >=  msm_chg->mBatteryTempBoundT0) && 
                 (temperature <= msm_chg->mBatteryTempBoundT1) ) /* battery is more cool */
        {
                rc = handle_batt_temp_little_cold(msm_chg);
        }
        else if( (temperature >=  msm_chg->mBatteryTempBoundT1) && 
                 (temperature <= msm_chg->mBatteryTempBoundT2) ) /* battery is cool */
        {
                rc = handle_batt_temp_cool(msm_chg);
        }
        else if( (temperature > msm_chg->mBatteryTempBoundT2) && 
                 (temperature < msm_chg->mBatteryTempBoundT3) ) /* battery is normal */
        {
                rc = handle_batt_temp_normal(msm_chg);
        }
        else if( (temperature >= msm_chg->mBatteryTempBoundT3) && 
                 (temperature <=  msm_chg->mBatteryTempBoundT4) ) /* battery is warm */
        {
                rc = handle_batt_temp_warm(msm_chg);
        }
        else /* battery is hot */
        {
                rc = handle_batt_temp_hot(msm_chg);
        }
		
	return rc;
}

static void smb346_check_charger_uovp(struct smb346_charger_mux *msm_chg)
{
	int count = BAD_STATE_COUNT;
	int vchg_mv=5000;

	if(msm_chg->mChargerCheckCounter == 0)
		return;

	if(smb346_charger_type_get(msm_chg) == CHARGER_TYPE_ACCESSORY )
	{
	     pr_info("%s:accessor charger not uovp check\n",__func__);
	     return;
	}

	msm_chg->mChargerCheckCounter--;
	msm_chg->mBadChargerCounter = 0;
       vchg_mv = smb346_battery_adc_params.vchg_mv;
	   
	while (count--)
	{
	    /*get vchg mvolts*/
		vchg_mv = get_prop_vchg_movlts(); 
		pr_info("%s vchg_mv =%dmv\n", __func__,vchg_mv);
		if ((vchg_mv > CHARGER_SOFT_OVP_VOLTAGE ||vchg_mv < CHARGER_SOFT_UVP_VOLTAGE)) { 
			msm_chg->mBadChargerCounter++;
		}
		else {
			msm_chg->mBadChargerCounter = 0;
			break;
		}
		
		msleep(200);
	}
	
	smb346_battery_adc_params.vchg_mv = vchg_mv;
	if (msm_chg->mBadChargerCounter >= BAD_STATE_COUNT){
		ERR_CHG("%s:charger is uvp or ovp\n",__func__);
		if(msm_chg->mChargerCheckCounter == 0)
			smb346_charger_status_set(msm_chg, CHARGER_STATUS_BAD);
		else {
			smb346_charger_status_set(msm_chg, CHARGER_STATUS_GOOD);
			if(smb346_battery_adc_params.vchg_mv <= CHARGER_SOFT_UVP_VOLTAGE )
				smb346_battery_adc_params.vchg_mv = 4310;
			else if(smb346_battery_adc_params.vchg_mv >= CHARGER_SOFT_OVP_VOLTAGE )
				smb346_battery_adc_params.vchg_mv = 5690;
		}
		
		msm_chg->mBadChargerCounter = 0;
	}
	else {
		smb346_charger_status_set(msm_chg, CHARGER_STATUS_GOOD);
		msm_chg->mChargerCheckCounter = 0;
	}
	return;
}


static void smb346_heart_beat__handle_charger_battery_uovp_status(struct smb346_charger_mux *msm_chg)
{
	chg_charger_status_type charger_status_pre;
	chg_battery_status_type battery_status_pre;

	battery_status_pre = smb346_battery_status_get(msm_chg);
	smb346_check_battery_uovp(msm_chg);
	
	if (battery_status_pre == BATTERY_STATUS_GOOD){
		if (smb346_battery_status_get(msm_chg) == BATTERY_STATUS_BAD){
			ERR_CHG("%s:stop charing for battery uovp\n", __func__);
			smb346_handle_battery_uovp(msm_chg);
		}	
	}
	else if (battery_status_pre == BATTERY_STATUS_BAD){
			if (smb346_battery_status_get(msm_chg) == BATTERY_STATUS_GOOD){
			ERR_CHG("%s:recove charing from battery uovp\n", __func__);
			smb346_handle_battery_restore_from_uovp(msm_chg);
			}
	}

	   	//not need check charger uovp when charing 
		charger_status_pre = smb346_charger_status_get(msm_chg);
		smb346_check_charger_uovp(msm_chg);

		if (charger_status_pre == CHARGER_STATUS_GOOD){
			if (smb346_charger_status_get(msm_chg) == CHARGER_STATUS_BAD){
				ERR_CHG("%s:stop charing for charger uovp\n", __func__);
				smb346_handle_charger_uovp(msm_chg);
			}	
		}
    

    /*only check charger status when chg plugin ,not check charger status when charging*/
    return;
}




/**
* smb346_tapper_charging_mode_soft_detect
* check current battery voltage to start charging complete detect
*/
static bool smb346_tapper_charging_mode_soft_detect(struct smb346_charger_mux *msm_chg)
{
	static int tapper_mode_soft_detect_count = 0;
	chg_cv_battery_temp_region_type batt_temp_region = CV_BATTERY_TEMP_REGION__NORMAL ;
	int battery_voltage = smb346_battery_adc_params.current_batt_adc_voltage;

	batt_temp_region = smb346_battery_temp_region_get(msm_chg);
	DEBUG_CHG("%s:======battery * voltage =%d\n", __func__, battery_voltage);

	 if(smb346_battery_temp_region_get(msm_chg) == CV_BATTERY_TEMP_REGION__COOL ||
	 		smb346_battery_temp_region_get(msm_chg) == CV_BATTERY_TEMP_REGION__WARM){
		if(battery_voltage >= TAPPER_INTERVAL_CHG_WARM_VOLT)
			tapper_mode_soft_detect_count++;
		else
			tapper_mode_soft_detect_count = 0;
	} else if(smb346_battery_temp_region_get(msm_chg) == CV_BATTERY_TEMP_REGION_LITTLE__COLD){
		if(battery_voltage >= TAPPER_INTERVAL_CHG_LITTLE_COLD_VOLT)
			tapper_mode_soft_detect_count++;
		else
			tapper_mode_soft_detect_count = 0;
	}else{
		if(battery_voltage >= TAPPER_INTERVAL_CHG_VOLT)
			tapper_mode_soft_detect_count++;
		else
			tapper_mode_soft_detect_count = 0;
	}
	
	if (tapper_mode_soft_detect_count >= TAPPER_CHG_MODE_SOFT_DETECT_COUNT){
		tapper_mode_soft_detect_count = 0;
		return true;
	}
	else
		return false;
}

/**
*  @ SMB_CHG_STATUS__CHARGING
*  @ SMB_CHG_STATUS__CHARGING_COMPLETED
*  @ SMB_CHG_STATUS__NOT_CHARGING
*  Update charging status 5seconds every time
*/
static void smb346_heart_beat__handle_charging_status_update(struct smb346_charger_mux *msm_chg)
{
	int battery_voltage = smb346_battery_adc_params.current_batt_adc_voltage;
	
	if (SMB_CHG_STATUS__CHARGING == smb346_charging_status_get(msm_chg)) {
		/* tricle charging and pre charing do not need software control */
		if (battery_voltage < msm_chg->soft_control_threhold) {
			msm_chg->soft_control_threhold = SOFT_CONTROL_THREHOLD + 
				SOFT_CONTROL_THREHOLD_HYST;
			DEBUG_CHG("not software control mode!\n");
			return;						
		}
		/* charing phase need software control */
		else {		
			msm_chg->soft_control_threhold = SOFT_CONTROL_THREHOLD - 
				SOFT_CONTROL_THREHOLD_HYST;
			
			if (smb346_tapper_charging_mode_soft_detect(msm_chg) == true){
				pr_info("%s:tapper no interval charge mode!\n", __func__);
				#ifdef INTERVAL_CHARGING_FEATURE
				remove_listener_for_interval_charging(msm_chg);
				#endif
				
				add_listener_for_charge_complete_detect(msm_chg);
			}
		}			
	}
	else if (SMB_CHG_STATUS__CHARGING_COMPLETED == smb346_charging_status_get(msm_chg)){
		DEBUG_CHG("%s:charging completed\n", __func__);
		//smb346_battery_adc_params.ichg_mv = 0;
		smb346_charger_relative_battery_status_set(msm_chg, POWER_SUPPLY_STATUS_FULL);
	}else if (SMB_CHG_STATUS__NOT_CHARGING == smb346_charging_status_get(msm_chg)){
		//smb346_battery_adc_params.ichg_mv = 0;
		smb346_charger_relative_battery_status_set(msm_chg,POWER_SUPPLY_STATUS_NOT_CHARGING);
	}
}

#ifdef DUMP_CHG_LOG_FEATURE
//dump charge log 

void __dump_charge_log(struct smb346_charger_mux *msm_chg)
{
    char *charging_status[] = {"INVALID", "NOT CHARGING", "CHARGING", "CHARGING_COMPLETED"};

	pr_info("========__dump_charge_log=====\n");
	get_aicl_status(msm_chg);//get aicl current
	pr_info("battery_valid = %d\n", msm_chg->battery_valid);
	pr_info("charging_status = %s\n", charging_status[msm_chg->charging_status]);
	pr_info("%s:vbat %dmv\n",__func__,smb346_battery_adc_params.current_batt_adc_voltage);
	pr_info("%s:ichg %dmA\n",__func__,smb346_battery_adc_params.ichg_mv);
	pr_info("%s:capacity %d%%\n",__func__,smb346_battery_adc_params.batt_capacity);
}
#endif

#ifdef FTM_CHARGE_MODE_FEATURE
//FTM charge control
static int ftm_charge__pause_charging(void)
{
	int rc=0;
		
	if(usb_msm_chg){
		rc = smb346_stop__charging(usb_msm_chg);
		if (rc){
			pr_err("%s: stop charging failed!\n",__func__);
			return rc;
		}
		
		if(smb346_charging_status_get(usb_msm_chg) != SMB_CHG_STATUS__NOT_CHARGING)
			smb346_charging_status_set(usb_msm_chg, SMB_CHG_STATUS__NOT_CHARGING);
		if(smb346_charger_relative_battery_status_get(usb_msm_chg) != POWER_SUPPLY_STATUS_NOT_CHARGING)
			smb346_charger_relative_battery_status_set(usb_msm_chg, POWER_SUPPLY_STATUS_NOT_CHARGING);
		if(smb346_charger_type_get(usb_msm_chg)!= CHARGER_TYPE_INVALID)
			smb346_charger_type_set(usb_msm_chg, CHARGER_TYPE_INVALID);
		
	}else{
		rc =-1;
		pr_err("%s: ftm charge pause failed! usb_msm_chg is NULL\n",__func__);
	}
	return rc;
}
#endif

int is_usb_plugged_in(void)
{
	chg_charger_hardware_type charger__type;
	charger__type = get_charger_type();
	if((charger__type == CHARGER_TYPE_WALL || 
		charger__type == CHARGER_TYPE_USB_PC || 
		charger__type == CHARGER_TYPE_USB_WALL ||
		charger__type == CHARGER_TYPE_USB_CARKIT || 
		charger__type == CHARGER_TYPE_NON_STANDARD ||
		charger__type == CHARGER_TYPE_USB_HDMI))
	{
		return 1;
	}
	else if(charger__type == CHARGER_TYPE_ACCESSORY)
		return 2;
	else
		return 0;

}

int is_accessory_charging(void)
{
	charging_status_type type;
	type = accessory_charging_status_get();
#if 0
	if(get_accessory_present_status())
		return 1;
	else
		return 0;
#endif
	if(!get_accessory_present_status())
		return 0;
	if(/*type == ACCESSORY__CHG_STATUS__CHARGING_COMPLETED || */type == ACCESSORY__CHG_STATUS__CHARGING)
		return 1;
	else
		return 0;
}

extern int ateml_get_noise(void);

int is_status_changed(void)
{
	static int pre_chg_type = 0;
	int current_chg_type;
	static int pre_wireless_flag = 0;
	int current_wireless_flag;
	static int pre_smb346_flag = 0;
	int current_smb346_flag;

	current_chg_type = accessory_charging_status_get();
	current_wireless_flag = accessory_is_wirelees_present();
	current_smb346_flag = get_usb_chg_status();
	if(current_chg_type != pre_chg_type)
	{
		pre_chg_type = current_chg_type;
		return 1;
	}
	
	
	if(current_wireless_flag != pre_wireless_flag)
	{
		pre_wireless_flag = current_wireless_flag;
		return 1;
	}
	
	if(current_smb346_flag != pre_smb346_flag)
	{
		pre_smb346_flag = current_smb346_flag;
		return 1;
	}
	return 0;

}

static void handle_event_timer_expired__heart_beat_update(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	static int dump_log_counter=0;
#if 0	
	static int pre_chg_type = 0;
	int current_chg_type;
	//static int pre_accessory_status = 0;
#endif
	bool battery_valid_pre = smb346_battery_valid_get(msm_chg);
	
	if (show_chg_log){
		smb346_charging_mode_get(msm_chg);
#if 1//this part is used to check aicl completed or not
		/* charger auto AICL fuction relatively treation */
		check_aicl_complete(msm_chg);
		if (smb346_aicl_adapt_completed_get(msm_chg) == false)
			ERR_CHG("%s:aicl adapt failed\n", __func__);
#endif
	}
	//system boottime
	#ifdef MPP11_IRQ_ENABLE_FEATURE
	if(boot_time){
		pr_info("%s:boot_time =%d\n",__func__,boot_time);	
		boot_time--;
	}
	#endif
	if (msm_chg->is_interval_chg ==  true){
		DEBUG_CHG("%s:======in interval chg mode\n", __func__);
		rc = get_batt_adc_chg(msm_chg);
		if (rc){
			pr_err("%s: get_batt_adc_chg failed!\n",__func__);
			return;
		}

	}
	else{
		DEBUG_CHG("%s:======realtime read(not fast charging mode) \n", __func__);
		rc = smb346_get_battery_adc_realtime(msm_chg);
		if (rc){
			pr_err("%s: get adc realtime failed!\n",__func__);
			return;
		}
	}
	
    #ifdef FTM_CHARGE_MODE_FEATURE
		if(low_charge_mode == true && smb346_battery_adc_params.batt_capacity >= FTM_LOW_CAPACITY_LEVEL)
		{
			pr_info("%s: charge on FTM mode, charge to 60 stop charge,=%d\n",__func__,low_charge_mode);
			ftm_charge__pause_charging();
			update_psy_status(msm_chg);
			return; 
				
		}else if(high_charge_mode == true && smb346_battery_adc_params.batt_capacity >= FTM_HIGH_CAPACITY_LEVEL){
		    pr_info("%s: charge on FTM mode, charge to 80 stop charge,=%d\n",__func__,high_charge_mode);
			ftm_charge__pause_charging();
			update_psy_status(msm_chg);
			return;
		}
	#endif
#if 1	
///huyu,dual charging
	//current_chg_type = smb346_charger_type_get(msm_chg);
	//current_chg_type = accessory_charging_status_get();

	//pr_info("%s: the current noise type is  %d ----\n",__func__,ateml_get_noise());
	//pr_info("%s: current_chg_type = %d  pre_chg_type = %d ----\n",__func__,current_chg_type,pre_chg_type);
	//pr_info("%s: accessory_charging_status_get() = %d  ----\n",__func__,accessory_charging_status_get());
	//pr_info("%s: smb346_charger_type_get() = %d  ----\n",__func__,smb346_charger_type_get(msm_chg));
	//pr_info("%s: get_accessory_present_status() = %d  ----\n",__func__,get_accessory_present_status());
	if(is_status_changed())
	{
		//pr_info("%s: huyu --------charging Anti-interference----\n",__func__);
		//pre_chg_type = current_chg_type;	
		if(is_usb_plugged_in() == 1 && (is_accessory_charging() || accessory_is_wirelees_present()))
		{
			//pr_info("%s: huyu --------dual charging----\n",__func__);
			atmel_noise_supress_cfg(5);
		}
		else if(is_usb_plugged_in()	== 1 /*&& !get_accessory_present_status()*/)
		{
			//pr_info("%s: huyu --------pc charging----\n",__func__);
			atmel_noise_supress_cfg(1);
		}
		else if(is_usb_plugged_in()	== 2)
		{
			//pr_info("%s: huyu --------accessory charging----\n",__func__);
			atmel_noise_supress_cfg(4);
		}
	}

#if 0
	if(current_chg_type != pre_chg_type || pre_accessory_status != get_accessory_present_status())
	{
		pr_info("%s: huyu --------charging Anti-interference----\n",__func__);
		pre_chg_type = current_chg_type;		
		pre_accessory_status = get_accessory_present_status();
		if(is_usb_plugged_in() && is_accessory_charging())
		{
		    pr_info("%s: huyu --------dual charging----\n",__func__);
			atmel_noise_supress_cfg(5);
		}
		else if(is_accessory_charging())
		{
		    pr_info("%s: huyu --------accessory charging----\n",__func__);
			atmel_noise_supress_cfg(4);
		}
		else if( is_usb_plugged_in()) 
		{
			pr_info("%s: huyu --------pc charging----\n",__func__);
			atmel_noise_supress_cfg(1);
		}
		else
		{
			pr_info("%s: huyu --------no charging----\n",__func__);
			atmel_noise_supress_cfg(0);
		}
	}
#endif
#endif
//boot steg not need check charger or battery uovp
if(!boot_time){	
#if 1//this part is for charger battery status handle
	if (smb346_charger_type_get(msm_chg) != CHARGER_TYPE_INVALID)
		smb346_heart_beat__handle_charger_battery_uovp_status(msm_chg);
#endif
}

#if 1//this part used to handle that battery is removed or inserted
	if (smb346_is_battery_present() == false)
		smb346_battery_valid_set(msm_chg, false);
	else
		smb346_battery_valid_set(msm_chg, true);
	
	if (battery_valid_pre == false){
		if (smb346_battery_valid_get(msm_chg) == true){
			rc = handle_battery_inserted(msm_chg);
			if (rc)
				return;
		}
	}	
	if (smb346_battery_valid_get(msm_chg) == false){
		/* if no battery voltage,we shold power down */
		if (smb346_battery_adc_params.current_batt_adc_voltage < MIISS_BATTTERY_VOLTAGE&&
			smb346_charger_type_get(msm_chg) == CHARGER_TYPE_INVALID ){
			ERR_CHG("%s:battery removed ,shutdown\n", __func__);
			kernel_power_off();
		}
		rc = handle_battery_removed(msm_chg);
		if(rc)
			return;
		else
			goto update;
	}
#endif

       /*battery temperate health update*/
	if (smb346_charger_type_get(msm_chg) != CHARGER_TYPE_INVALID) {
		if (smb346_battery_valid_get(msm_chg) == true){
			rc = smb346_battery_temp_handle(msm_chg);
			if (rc){
				pr_err("%s: smb346_battery_temp_handle failed!!\n",__func__);
				return;
			}
		}
	}
	
	//charging phase, charging complete detect ,update charging status
	smb346_heart_beat__handle_charging_status_update(msm_chg);

#if 0
	if (smb346_charging_status_get(msm_chg) == SMB_CHG_STATUS__NOT_CHARGING){
		smb346_battery_adc_params.ichg_mv = 0;
		//smb346_charger_info.batt_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		smb346_charger_relative_battery_status_set(msm_chg, POWER_SUPPLY_STATUS_NOT_CHARGING);
	}
#endif
	if (smb346_charger_type_get(msm_chg) == CHARGER_TYPE_INVALID ||
		smb346_charger_suspend_mode_get(msm_chg) == true){
		//smb346_battery_adc_params.ichg_mv = 0;
		//smb346_charger_info.batt_status = POWER_SUPPLY_STATUS_DISCHARGING;
		smb346_charger_relative_battery_status_set(msm_chg, POWER_SUPPLY_STATUS_DISCHARGING);
	}
#if 0
	if (smb346_charging_status_get(msm_chg) == SMB_CHG_STATUS__CHARGING_COMPLETED){
		smb346_battery_adc_params.ichg_mv = 0;
		smb346_charger_relative_battery_status_set(msm_chg, POWER_SUPPLY_STATUS_FULL);
	}
#endif	

update:
	__dump_charger_info(msm_chg);	
    #ifdef DUMP_CHG_LOG_FEATURE
	dump_log_counter ++;
    /*dump charger printf log 60s every time*/
	dump_log_counter = dump_log_counter%dump_log_timer;
	if(dump_log_counter == 0){
	    __dump_charge_log(msm_chg);
	}
	#endif
	
	if (msm_chg->is_interval_chg == true)
		DEBUG_CHG("%s:battery status will be updated in interval timer pereiod\n", __func__);
	else
		update_psy_status(msm_chg);		
}
static void handle_event_timer_expired__chg_get_adc(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	rc = get_batt_adc_chg(msm_chg);
	if (rc)
		return;

	update_psy_status(msm_chg);
}
static void handle_event_timer_expired__cc_chg_detect(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
#if 0
	int current_bak = 0;
#endif
	rc = get_batt_adc_chg(msm_chg);
	if (rc)
		return;
#ifdef FEATURE_SUSPEND_CHARGE
	/* suspend charger to stop charging */
       #error ,not suspend charge
	rc = smb346_suspend_mode_on(msm_chg);
	if (rc)
		break;
#else		                        
	rc = smb346_pause_charging(msm_chg);
	if (rc)
		return;	
#endif
#if 0//deliever a not true status just for app display
	current_bak = smb346_battery_adc_params.ichg_mv;
	smb346_battery_adc_params.ichg_mv = 0;
	//smb346_charger_relative_battery_status_set(msm_chg, POWER_SUPPLY_STATUS_NOT_CHARGING);
	update_psy_status(msm_chg);
	smb346_battery_adc_params.ichg_mv = current_bak;
#else
	smb346_charger_relative_battery_status_set(msm_chg, POWER_SUPPLY_STATUS_CHARGING);
	update_psy_status(msm_chg);
#endif
	return;
}
static void handle_event_timer_expired__cc_dischg_detect(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;

	pr_info("%s:\n",__func__);
	rc = get_batt_adc(msm_chg);
	if (rc)
		return;

#ifdef FEATURE_SUSPEND_CHARGE
               #error not suspend charge
		/* suspend charger to stop charging */
		rc = smb346_suspend_mode_off(msm_chg);
		if (rc)
			break;
#else	                        
		rc = smb346_recov_charging(msm_chg);
		if (rc)
			return;
		smb346_charger_relative_battery_status_set(msm_chg, POWER_SUPPLY_STATUS_CHARGING);
#endif
		update_psy_status(msm_chg);

	return;
}

static void handle_event_timer_expired__end_chg_detect_time(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	DEBUG_CHG("%s:\n",__func__);
	//do not use get_batt_adc for we need realtime current 
	rc = get_batt_adc_chg(msm_chg);
	if(rc)
		return;
	
	rc = charge_complete_detect(msm_chg);
	if (rc)
		return;
	return;
}

static void handle_event_timer_expired__chg_resume(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	int battery_voltage;
	int battery_voltage_resume;
	
	pr_info("%s resume_count = %d\n", __func__, msm_chg->resume_count);
	rc = get_batt_adc(msm_chg);
	if (rc){
		pr_err("%s: get adc failed!!\n",__func__);
		return;
	}
	
	if (msm_chg->stop_resume_check) {
		//msm_chg->stop_resume_check = 0;
		DEBUG_CHG("%s stopping resume", __func__);
		return;
	}
	
	battery_voltage = smb346_battery_adc_params.current_batt_adc_voltage;
	DEBUG_CHG("%s:=====battery voltage = %d\n", __func__, battery_voltage);
	if (smb346_battery_temp_region_get(msm_chg) == CV_BATTERY_TEMP_REGION__NORMAL) {
		battery_voltage_resume = AUTO_CHARGING_RESUME_CHARG_VOLT;
	}
	else if (smb346_battery_temp_region_get(msm_chg) == CV_BATTERY_TEMP_REGION__WARM) {
		battery_voltage_resume = AUTO_CHARGING_RESUME_CHARG_VOLT_WARM;
	}
	else if (smb346_battery_temp_region_get(msm_chg) == CV_BATTERY_TEMP_REGION__COOL) {
		battery_voltage_resume = AUTO_CHARGING_RESUME_CHARG_VOLT_COOL;
	}
	else if (smb346_battery_temp_region_get(msm_chg) == CV_BATTERY_TEMP_REGION_LITTLE__COLD) {
		battery_voltage_resume = AUTO_CHARGING_RESUME_CHARG_VOLT_LITTLECOLD;
	}
	
	/*resume charging check*/
	if(battery_voltage < battery_voltage_resume) {
		
	      pr_info("%s:=====current battery voltage = %d,battery_voltage_resume = %d",
		         __func__, battery_voltage, battery_voltage_resume);
		/* Increase the counter*/
		msm_chg->resume_count++;
	}
	else {
		/* Reset the counter */
		msm_chg->resume_count = 0;
	}

	/*
	 * if we are within 500mV of min voltage range forget the count
	 * force start battery charging by increasing resume count
	 */
	if (battery_voltage < msm_chg->min_voltage + 500) {
		DEBUG_CHG("%s: batt lost voltage rapidly -force resume charging\n",
					__func__);
		msm_chg->resume_count += MSM_CHARGER_RESUME_COUNT + 1;
	}

	if (msm_chg->resume_count >= MSM_CHARGER_RESUME_COUNT) {
		/* the battery has dropped below 4.1V for 5 mins
		 * straight- resume charging */
		/* act as if the battery was just plugged in */

		if (smb346_start_resume_charging(msm_chg) == 0){
			//smb346_charger_info.battery_level = BATTERY_LEVEL_GOOD;
			smb346_battery_level_set(msm_chg, BATTERY_LEVEL_GOOD); 
			remove_listener_for_charge_resume(msm_chg);
		}

	} else {
		/* reschedule resume check */
		DEBUG_CHG( "%s: rescheduling resume timer work", __func__);
	}
	
	return;	
}

static void handle_event_timer_expired__max_chg_time(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	/* we have been charging too long - stop charging */
	pr_info("%s: safety timer work expired\n", __func__);
	/* we have been charging too long - stop charging */
	rc = smb346_stop__charging(msm_chg);
	if(rc)
		return;
	smb346_charging_status_set(msm_chg, SMB_CHG_STATUS__CHARGING_COMPLETED);

	return;
}
static void handle_event_timer_expired__calling(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	DEBUG_CHG("%s: calling_ends=%d, no_charging_while_calling=%d,restore_charging_when_calling_ends=%d\n",
		__func__, calling_ends, no_charging_while_calling, restore_charging_when_calling_ends);
//stop charging anytime if battery voltage is higher than 4050  when battery is charging
	if (calling_ends == false){
		if (smb346_battery_adc_params.current_batt_adc_voltage >= STOP_CHG_BATT_VOL_WHILE_CALLING_MV){
			no_charging_while_calling = true;
			if (SMB_CHG_STATUS__CHARGING == smb346_charging_status_get(msm_chg)){
				chg_status_bak_for_calling = SMB_CHG_STATUS__CHARGING;
				rc = smb346_stop__charging(msm_chg);
				if (rc)
					ERR_CHG("%s:stop charging failed when calling\n", __func__);
			}
		}
	}else{
//restore charging if possible when calling ends
		if (restore_charging_when_calling_ends || chg_status_bak_for_calling == SMB_CHG_STATUS__CHARGING){
			if (CHARGER_TYPE_INVALID != smb346_charger_type_get(msm_chg) &&
				SMB_CHG_STATUS__CHARGING != smb346_charging_status_get(msm_chg) &&
				SMB_CHG_STATUS__CHARGING_COMPLETED != smb346_charging_status_get(msm_chg)	){
					rc = smb346_start__charging(msm_chg);
					if (rc)
						ERR_CHG("%s: restore charging failed when calling ends\n", __func__);
					else
						restore_charging_when_calling_ends = false;
			}
		}
		remove_listener_for_calling_timer(msm_chg);
	}
}


static void handle_event_system_status_changed__charger_valid(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;

	msm_chg->mChargerCheckCounter = MAX_CHARGER_CHECK_COUNT;
	
	rc = smb346_get_battery_adc_realtime(msm_chg);
	if (rc)
		return;
	
       DEBUG_CHG("%s: Start charging with valid charger\n",__func__);

	if(msm_chg->charger_type == CHARGER_TYPE_ACCESSORY)
	{
		smb346_battery_adc_params.vchg_mv =5000;
		smb346_charger_info.charger_status = CHARGER_STATUS_GOOD;
	}
	else
	{
		/* check is charger OVP and UVP */
		smb346_check_charger_uovp(msm_chg);
	}
	
	if (smb346_charger_status_get(msm_chg) == CHARGER_STATUS_BAD ||
		smb346_battery_status_get(msm_chg) == BATTERY_STATUS_BAD){
		ERR_CHG("%s:bad charger state or battery state,will not start charging\n", __func__);	
		
	}else{
	
		/* disable suspend mode default when charger reconnect*/
		rc = smb346_update(msm_chg, STATUS_REGISTER_B, USB_SUSPEND_MODE__NOT_ACTIVE, 
			USB_SUSPEND_MODE__ACTIVE);	
		if (rc){
			pr_err("%s:smb346 update STATUS_REGISTER_B register failed!\n",__func__);
			return;
		}
		
		rc = smb346_start__charging(msm_chg);
		if (rc){
			pr_err("%s: error smb346_start__charging failed!!!\n ",__func__);
			return;
		}
	}

	/* smb346 start charging sucessfully, update psy status*/
	rc = update_psy_status(msm_chg);
	if (rc){
		pr_err("%s: Error psy status update failed!\n",__func__);
		return;
	}
	
	return;
			
}

static void handle_event_system_status_changed__usb_invalid(struct smb346_charger_mux * msm_chg)
{
	int rc = -1;
	uint8_t reg_irq[6];

	rc = smb346_stop__charging(msm_chg);
	if (rc){
		pr_err("%s: stop charging failed!!!\n",__func__);
		return;
	}
	
	/* init of battery info value. */
	smb346_info_val_init(msm_chg);

	rc =smb346_rx_irq_regs(msm_chg, reg_irq);
	if (rc)
		return;
	
	rc = get_batt_adc(msm_chg);
	if (rc)
		return;

	/*update charger status*/
	smb346_charger_status_set(msm_chg, CHARGER_STATUS_INVALID);
	
	rc = update_psy_status(msm_chg);
	if (rc)
		return;
	
	return;
}
static void handle_event_system_status_changed__usb_suspend_on(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;

	pr_info("%s: ftm charge suspend mode on\n",__func__);
	
	rc = smb346_stop__charging(msm_chg);
	if (rc)
		return;
	
	rc = smb346_suspend_mode_on(msm_chg);
	if (rc)
		return;		
	
	//smb346_charger_set_onoff_nv(msm_chg, true);
	
	rc = smb346_get_battery_adc_realtime(msm_chg);
	if (rc)
		return;
	
	rc = update_psy_status(msm_chg);
	if (rc)
		return;
	return;
}

static void handle_event_system_status_changed__usb_suspend_off(struct smb346_charger_mux * msm_chg)
{
	int rc = -1;
	
	rc = smb346_get_battery_adc_realtime(msm_chg);
	if (rc)
		return;

	rc = smb346_suspend_mode_off(msm_chg);
	if (rc)
		return;
	
	//smb346_charger_set_onoff_nv(msm_chg, false);
	
	rc = smb346_start__charging(msm_chg);
	if (rc)
		return;
	
	rc = update_psy_status(msm_chg);
	if (rc)
		return;
	return;
}

static void handle_event(struct smb346_charger_mux *msm_chg, int event)
{

	DEBUG_CHG("%s event = %d\n", __func__, event);
       
	switch (event) {
		case IRQ_TRIGGERED__STAT:
		{
			handle_event__irq_triggered(msm_chg);
			break;
		}
		
		case TIMER_EXPIRED__HEART_BEAT:
		{
			handle_event_timer_expired__heart_beat_update(msm_chg);
			break;
		}

		case TIMER_EXPIRED__CHG_GET_ADC:
		{
			handle_event_timer_expired__chg_get_adc(msm_chg);
			break;
		}
		
		case TIMER_EXPIRED__CC_CHG_DETECT:
		{
			handle_event_timer_expired__cc_chg_detect(msm_chg);
			break;
		}
		
		case TIMER_EXPIRED__CC_DISCHG_DETECT:
		{
			handle_event_timer_expired__cc_dischg_detect(msm_chg);
			break;
		}

		case TIMER_EXPIRED__END_CHG_DETECT_TIME:
		{
			handle_event_timer_expired__end_chg_detect_time(msm_chg);
			break;
		}

		#ifndef FEATURE_AUTO_RECHARGE
		case TIMER_EXPIRED__CHG_RESUME:
		{
			handle_event_timer_expired__chg_resume(msm_chg);
			break;
		}	
		#endif

		case TIMER_EXPIRED__MAX_CHG_TIME:
		{
			handle_event_timer_expired__max_chg_time(msm_chg);
			break;
		}	
		case TIMER_EXPIRED__CALLING:
		{
			handle_event_timer_expired__calling(msm_chg);
			break;
		}
		case SYSTEM_STATUS_CHANGED__AC_VALID:
		case SYSTEM_STATUS_CHANGED__USB_VALID:
		case SYSTEM_STATUS_CHANGED__NONSTANDARD_VALID:
		case SYSTEM_STATUS_CHANGED__HDMI_VALID:
		case SYSTEM_STATUS_CHANGED__ACCESSORY:
		{	/*start charging with valid charger*/
			handle_event_system_status_changed__charger_valid(msm_chg);
			break;
		}
		
		case SYSTEM_STATUS_CHANGED__USB_INVALID:
		{
			handle_event_system_status_changed__usb_invalid(msm_chg);
			break;
		}

		case SYSTEM_STATUS_CHANGED__USB_SUSPEND_ON:
		{
			handle_event_system_status_changed__usb_suspend_on(msm_chg);
			break;
		}

		case SYSTEM_STATUS_CHANGED__USB_SUSPEND_OFF:
		{
			handle_event_system_status_changed__usb_suspend_off(msm_chg);
			break;
		}	
		
		case SYSTEM_STATUS_CHANGED__FTM_SWITCH_OFF_CHRGING:
		{
			
			break;
		}
		
		case SYSTEM_STATUS_CHANGED__FTM_SWITCH_ON_CHRGING:
		{

			break;
		}
		case IRQ_TRIGGERED__MPP11:
		{
			handle_event__mpp11_irq_triggered(msm_chg);
			break;
		}
	}
	
}

static int smb346_chg_dequeue_event(struct smb346_charger_mux *msm_chg, struct msm_charger_event **event)
{
	unsigned long flags;

	spin_lock_irqsave(&msm_chg->queue_lock, flags);
	if (msm_chg->queue_count == 0) {
		spin_unlock_irqrestore(&msm_chg->queue_lock, flags);
		return -EINVAL;
	}
	//pr_info("%s dequeueing %d,queue count =%d\n", __func__, (*event)->event,msm_chg->queue_count);
	*event = &msm_chg->queue[msm_chg->head];
	msm_chg->head = (msm_chg->head + 1) % MSM_CHG_MAX_EVENTS;

	msm_chg->queue_count--;
	spin_unlock_irqrestore(&msm_chg->queue_lock, flags);
	return 0;
}

static int msm_chg_enqueue_event(struct smb346_charger_mux *msm_chg, enum smb346_hardware_charger_event event)
{
	unsigned long flags;

	spin_lock_irqsave(&msm_chg->queue_lock, flags);
	if (msm_chg->queue_count == MSM_CHG_MAX_EVENTS) {
		DEBUG_CHG("%s fatal err: drop event (%d) = 0!\n", __func__, event);
		spin_unlock_irqrestore(&msm_chg->queue_lock, flags);
		return -EAGAIN;
	}
	
       //pr_info("%s queueing %d,queue count =%d\n", __func__, event,msm_chg->queue_count);
	   
	msm_chg->queue[msm_chg->tail].event = event;
	msm_chg->tail = (msm_chg->tail + 1)%MSM_CHG_MAX_EVENTS;
	msm_chg->queue_count++;
	spin_unlock_irqrestore(&msm_chg->queue_lock, flags);
	return 0;
}

static void process_events(struct work_struct *work)
{
	struct smb346_charger_mux *msm_chg = container_of(work, struct smb346_charger_mux, queue_work);	
	struct msm_charger_event *event;
	int rc;

	do {
		rc = smb346_chg_dequeue_event(msm_chg, &event);
		if (!rc)
			handle_event(msm_chg, event->event);
	} while (!rc);
}

static int msm_charger_notify_event(struct smb346_charger_mux *msm_chg, enum smb346_hardware_charger_event event)
{
	msm_chg_enqueue_event(msm_chg, event);
	queue_work(msm_chg->event_wq_thread, &msm_chg->queue_work);
	return 0;
}


void smb346_chg_connected(enum chg_type chgtype)
{
	char *chg_types[] = {"STD DOWNSTREAM PORT(USB Charger)",
			"CARKIT",
			"DEDICATED CHARGER(STANDARD Charger)",
			"NON-STANDARD Charger",
			"HDMI Charger",
			"INVALID",
			"ACCESSORY Charger"};

	pr_info("%s  Charger Type: #### :%s\n", __func__, chg_types[chgtype]);
       	
	if (!usb_msm_chg){
		ERR_CHG("%s, error usb_msm_chg is null\n", __func__);
		return;
	}
	
	#ifdef FTM_CHARGE_MODE_FEATURE
	//clean ftm charge mode to default
	low_charge_mode = false;
	high_charge_mode = false;
	#endif
	
	switch (chgtype)
	{
		case USB_CHG_TYPE__SDP:
		{
			/* set the wake lock, do not let sytem sleep. */
			#ifdef CONFIG_HAS_WAKELOCK
			wake_lock(&usb_msm_chg->smb346_wake_lock);
			#endif

			#ifdef CONFIG_TOUCHSCREEN_ATMEL_QT602240E
			/*Add Touchpanel reduce Noise when USB plugin 2011-12-15*/
			atmel_noise_supress_cfg(1);
			/*End Touchpanel reduce Noise when USB plugin 2011-12-15*/	
			#endif
			
			/* init of charger params value. */
			smb346_info_val_init(usb_msm_chg);	
			smb346_charger_type_set(usb_msm_chg, CHARGER_TYPE_USB_PC);
			add_listener_for_charge_enable_delay(usb_msm_chg);
			usb_charger_connect(1);
			break;
		}
		case USB_CHG_TYPE__WALLCHARGER:
		{
			/* set the wake lock, do not let sytem sleep. */
			#ifdef CONFIG_HAS_WAKELOCK
			wake_lock(&usb_msm_chg->smb346_wake_lock);
			#endif
			#ifdef CONFIG_TOUCHSCREEN_ATMEL_QT602240E
			/*Add Touchpanel reduce Noise when USB plugin 2011-12-15*/
			atmel_noise_supress_cfg(1);
			/*End Touchpanel reduce Noise when USB plugin 2011-12-15*/	
			#endif
			/* init of battery info value. */
			smb346_info_val_init(usb_msm_chg);	
			smb346_charger_type_set(usb_msm_chg, CHARGER_TYPE_USB_WALL);
			add_listener_for_charge_enable_delay(usb_msm_chg);
			usb_charger_connect(1);
			
			break;
		}
		case USB_CHG_TYPE__NON_STANDARD:
		{
			/* set the wake lock, do not let sytem sleep. */
			#ifdef CONFIG_HAS_WAKELOCK
			wake_lock(&usb_msm_chg->smb346_wake_lock);
			#endif

			#ifdef CONFIG_TOUCHSCREEN_ATMEL_QT602240E
			/*Add Touchpanel reduce Noise when USB plugin 2011-12-15*/
			atmel_noise_supress_cfg(1);
			/*End Touchpanel reduce Noise when USB plugin 2011-12-15*/	
			#endif
			/* init of battery info value. */
			smb346_info_val_init(usb_msm_chg);	
			smb346_charger_type_set(usb_msm_chg, CHARGER_TYPE_NON_STANDARD);
			add_listener_for_charge_enable_delay(usb_msm_chg);
			usb_charger_connect(1);

			break;
		}
		case USB_CHG_TYPE__HDMI:
		{
			/* set the wake lock, do not let sytem sleep. */
			#ifdef CONFIG_HAS_WAKELOCK
			wake_lock(&usb_msm_chg->smb346_wake_lock);
			#endif

			#ifdef CONFIG_TOUCHSCREEN_ATMEL_QT602240E
			/*Add Touchpanel reduce Noise when USB plugin 2011-12-15*/
			atmel_noise_supress_cfg(1);
			/*End Touchpanel reduce Noise when USB plugin 2011-12-15*/	
			#endif
			/* init of battery info value. */
			smb346_info_val_init(usb_msm_chg);	
			smb346_charger_type_set(usb_msm_chg, CHARGER_TYPE_USB_HDMI);
			add_listener_for_charge_enable_delay(usb_msm_chg);
			usb_charger_connect(1);

			break;
		}
		case USB_CHG_TYPE__INVALID:
		{
			chg_charger_hardware_type charger_temp = get_charger_type();
			pr_info("%s charger_temp:%d\n", __func__, charger_temp);
			/* release wake lock. */
			#ifdef CONFIG_HAS_WAKELOCK
			//wake_unlock(&usb_msm_chg->smb346_wake_lock);
			wake_lock_timeout(&usb_msm_chg->smb346_wake_lock, HZ);
			#endif

			#ifdef CONFIG_TOUCHSCREEN_ATMEL_QT602240E
			/*Add Touchpanel reduce Noise when USB plugin 2011-12-15*/
			atmel_noise_supress_cfg(0);
			/*End Touchpanel reduce Noise when USB plugin 2011-12-15*/		 
			#endif

            /*clear mhl chg type*/
			mhl_chg_type = CHG_MHL_NONE;
			
			smb346_charger_type_set(usb_msm_chg, CHARGER_TYPE_INVALID);
			if (smb346_hrtimer_is_queued(&usb_msm_chg->charge_enable_delay_timer))
				remove_listener_for_charge_enable_delay(usb_msm_chg);
			else {
				//remove_listener_for_charge_enable_delay(usb_msm_chg);
				msm_charger_notify_event(usb_msm_chg, SYSTEM_STATUS_CHANGED__USB_INVALID);
			}
            usb_charger_connect(0);

			//pr_info("%s accessory_is_wirelees_present = :%d\n", __func__, accessory_is_wirelees_present());
			if(accessory_is_wirelees_present()) {
				if(charger_temp != CHARGER_TYPE_ACCESSORY && 
					charger_temp != CHARGER_TYPE_NONE && 
					charger_temp != CHARGER_TYPE_INVALID) {
					/* set the wake lock, do not let sytem sleep. */
					#ifdef CONFIG_HAS_WAKELOCK
					wake_lock(&usb_msm_chg->smb346_wake_lock);
					#endif

					#ifdef CONFIG_TOUCHSCREEN_ATMEL_QT602240E
					/*Add Touchpanel reduce Noise when USB plugin 2011-12-15*/
					//atmel_noise_supress_cfg(1);
					/*End Touchpanel reduce Noise when USB plugin 2011-12-15*/	
					#endif
			
					/* init of battery info value. */
					smb346_info_val_init(usb_msm_chg);	
					smb346_charger_type_set(usb_msm_chg, CHARGER_TYPE_ACCESSORY);
					add_listener_for_charge_enable_delay(usb_msm_chg);
					usb_charger_connect(1);
				}
			}

			break;
		}
		case USB_CHG_TYPE__ACCESSORY:
		{
			/* set the wake lock, do not let sytem sleep. */
			#ifdef CONFIG_HAS_WAKELOCK
			wake_lock(&usb_msm_chg->smb346_wake_lock);
			#endif

			#ifdef CONFIG_TOUCHSCREEN_ATMEL_QT602240E
			/*Add Touchpanel reduce Noise when USB plugin 2011-12-15*/
			atmel_noise_supress_cfg(4);
			/*End Touchpanel reduce Noise when USB plugin 2011-12-15*/	
			#endif
			
			/* init of battery info value. */
			smb346_info_val_init(usb_msm_chg);	
			smb346_charger_type_set(usb_msm_chg, CHARGER_TYPE_ACCESSORY);
			add_listener_for_charge_enable_delay(usb_msm_chg);
			usb_charger_connect(1);
			break;
		}
		default:
		{
			smb346_charger_status_set(usb_msm_chg, CHARGER_STATUS_INVALID);
			smb346_charger_type_set(usb_msm_chg, CHARGER_TYPE_INVALID);
			ERR_CHG("%s error invaild chager type!\n", __func__);
			break;
		}
	}
	//mutex_unlock(&smb346_chg.smb346_lock);
}

/**
*    smb346_irq
*    smb346 chip irq when abnormity happen*/
static irqreturn_t smb346_irq( int irq, void *dev_id )
{

	if(usb_msm_chg != NULL){
	       DEBUG_CHG("%s, SMB346 Charging chip IRQ coming\n", __func__);
		/* disable irq */
		//disable_irq(smb346_chg.smb346_irq);
		
		msm_charger_notify_event(usb_msm_chg, IRQ_TRIGGERED__STAT);
	}else{
	       pr_err("%s: usb_msm_chg is NULL\n",__func__);
	}
	
	return IRQ_HANDLED;
}

#ifdef MPP11_IRQ_ENABLE_FEATURE
//add by chendx 2011-09-01 

static irqreturn_t _smb346_valid_handler(int irq, void *dev_id)
{
	int rc;

	rc = get_head_set_status();

	if(!rc) {
		if (is_chg_plugged_in()){
		  smb346_hsusb_enable(1);
		}
		else
		{  
		  smb346_hsusb_enable(0);
		}
	}

	// if(get_accessory_ftm_mode()){
	if(usb_msm_chg != NULL)
	msm_charger_notify_event(usb_msm_chg, IRQ_TRIGGERED__MPP11);
	//}

	return IRQ_HANDLED;
}
#endif

static ssize_t smb346_log_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size) {
	unsigned long val = simple_strtoul(buf, NULL, 10);
	struct smb346_charger_mux * msm_chg = dev_get_drvdata(dev);
	
	if(val == 0) {
		show_batt_log = false;
		show_chg_log = false;
		show_gague_log = false;
	}
	else if (val == 1) {
		show_chg_log = true;
	}
	else if (val == 2){
		show_batt_log = true;
	}
	else if (val == 3) {
		if (msm_chg == NULL){
				ERR_CHG("%s: msm_chg is null\n", __func__);
				return size;
		}
		__dump_regs_all(msm_chg);
		__dump_batt_info(msm_chg);
		__dump_adc_info(msm_chg);
		__dump_charger_info(msm_chg);		
	} else if (val == 5){
		show_gague_log = true;
	}else if (val == 6){
		show_batt_log = true;
		show_chg_log = true;
		show_gague_log = true;
	}
	
	return size;
}
static DEVICE_ATTR(smb346_log, 0666, NULL, smb346_log_store);
#ifdef FTM_CHARGE_MODE_FEATURE
static ssize_t smb346_chg_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size) {
	unsigned long val = simple_strtoul(buf, NULL, 10);
	//struct smb346_charger_mux * msm_chg = dev_get_drvdata(dev);
    //set FTM charge mode,1. 60% stop charge,2.80% stop charge 3.normal charge
	if(val == 60)
	{
	   //60
	   pr_info("smb346_chg_mode_store: charge to capacity 60 stop mode =%ld\n",val);
	   low_charge_mode = true;
	}else if(val == 80){
	   //80
	   pr_info("smb346_chg_mode_store: charge to capacity 80 stop mode =%ld\n",val); 
	   high_charge_mode = true;
	}else{
	   //100
	   pr_info("smb346_chg_mode_store: charge to normal 100 stop mode =%ld\n",val); 
	   low_charge_mode = false;
	   high_charge_mode = false;
	}
	
	return size;
}
static DEVICE_ATTR(smb346_chg, 0666, NULL, smb346_chg_store);
#endif

static ssize_t smb346_suspend_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size) {
	unsigned long val = simple_strtoul(buf, NULL, 10);
	struct smb346_charger_mux * msm_chg = dev_get_drvdata(dev);
	
	ERR_CHG("warning!!charger will suspend, process(pid:%d, pid_name:%s) writes %ld to smb346_suspend_mode!!!!!!!!!!\n", current->pid, current->comm, val);
	if (msm_chg == NULL){
		ERR_CHG("%s: msm_chg is null\n", __func__);
		return size;
	}	
	if(val == 1) {
		ERR_CHG("start usb suspend mode!\n");
		smb346_usb_suspend_mode(msm_chg, true);
	}
	else if (val == 0) {
		ERR_CHG("stop usb suspend mode!\n");
		smb346_usb_suspend_mode(msm_chg, false);
	}
	
	return size;
}
static DEVICE_ATTR(smb346_suspend_mode, 0666, NULL, smb346_suspend_mode_store);
static ssize_t smb346_start_stop_charging_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size) {
	unsigned long val = simple_strtoul(buf, NULL, 10);
	struct smb346_charger_mux * msm_chg = dev_get_drvdata(dev);
	if (msm_chg == NULL){
		ERR_CHG("%s: msm_chg is null\n", __func__);
		return size;
	}
	if(val == 1) {
		ERR_CHG("recovery  charging!\n");
		smb346_start__charging(msm_chg);
	}
	else if (val == 0) {
		ERR_CHG("stop charging!\n");
		smb346_stop__charging(msm_chg);
	}
	
	return size;
}
static DEVICE_ATTR(smb346_start_stop_charging, 0666, NULL, smb346_start_stop_charging_store);
static ssize_t smb346_start_stop_charging_conditional_while_calling_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size) {
	unsigned long val = simple_strtoul(buf, NULL, 10);
	struct smb346_charger_mux  *msm_chg = dev_get_drvdata(dev);
	if (val == 0){
		calling_ends = false;
		add_listener_for_calling_timer(msm_chg);
	}else{
		no_charging_while_calling =false;
		calling_ends = true;
	}
	return size;
}
static DEVICE_ATTR(smb346_start_stop_charging_conditional_while_calling, 0666, NULL, smb346_start_stop_charging_conditional_while_calling_store);

static int battery_power_on_voltage = 0;
int get_battery_power_on_voltage(void)
{
	return battery_power_on_voltage;
}
static bool battery_power_on_voltage_is_real = true;
bool get_battery_power_on_voltage_is_real(void)
{
	return battery_power_on_voltage_is_real;
}

static int smb346_battery_power_on_voltaget_set(struct smb346_charger_mux *msm_chg)
{
	int rc = -1;
	int i = 0;
	int count = 0;
	int sum = 0;
	int battery_voltage = 0;
	DEBUG_CHG("%s\n", __func__);
	for (i = 0; i < BATTERY_POWER_ON_VOLTAGE_CALIBARATION_COUNT; i++)
	{
		usleep(50000);
		rc = get_batt_adc(msm_chg);
		if (rc)
			continue;
		
		battery_voltage = smb346_battery_adc_params.current_batt_adc_voltage;
		sum += battery_voltage;
		count++;
	}
	battery_power_on_voltage = sum  / count;
	ERR_CHG("boot battery voltage calibration result = %d\n", battery_power_on_voltage);
	return rc;

}

void smb346_battery_gauge_register(struct smb346_battery_gauge *batt_gauge)
{
	msm_batt_gauge = batt_gauge;
}
EXPORT_SYMBOL(smb346_battery_gauge_register);

void smb346_battery_gauge_unregister(struct smb346_battery_gauge *batt_gauge)
{
	msm_batt_gauge = NULL;
}
EXPORT_SYMBOL(smb346_battery_gauge_unregister);

#ifdef MPP11_IRQ_ENABLE_FEATURE
//add by chendx 2011-09-01 
int smb346_mpp11_irq_register(void)
{
    int ret=0;
    ret = smb346_detection_setup();
	if (ret){
		pr_err("%s MPP10 /MPP11 set failed!!\n",__func__);
		return -1;
	}

	ret = get_head_set_status();

	if(!ret) {
		if (is_chg_plugged_in()){
			pr_info("%s: usb is plugin, %d\n",__func__, ret);
			smb346_hsusb_enable(1);
		}else{
		    pr_info("%s: usb is Not plugin, %d\n",__func__, ret);
			smb346_hsusb_enable(0);
		}
	}
	smb346_chg.mpp11_irq = gpio_to_irq(MPP11_VALID_GPIO);
	   
	/*mpp11 irq request*/
	ret = request_threaded_irq(smb346_chg.mpp11_irq, NULL,
				   _smb346_valid_handler,
				   IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
				   "smb346_chg_valid", NULL);
	if (ret){
		pr_err("%s request_threaded_irq failed for %d rc =%d\n",
			__func__, MPP11_VALID_GPIO, ret);
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL(smb346_mpp11_irq_register);
#endif

static int __devinit smb346_charger_probe(struct platform_device *pdev)
{
	int ret=0;
	struct smb346_charger_mux *msm_chg;
	struct smb346_platform_pdata *pdata;
	
	pr_info("smb346_charger_probe\n");

	smb346_chg.dev = &pdev->dev;
	
	if(pdev->dev.platform_data == NULL)
	{
	       ERR_CHG("%s platform data is NULL\n", __func__);
		ret = -ENOMEM;
		goto out;
	}
	pdata = (struct smb346_platform_pdata *)pdev->dev.platform_data;

	/* kzalloc msm_chg */
	msm_chg = kzalloc(sizeof(*msm_chg), GFP_KERNEL);
	if (msm_chg == NULL) {
		ERR_CHG("%s kzalloc msm_chg failed\n", __func__);
		ret = -ENOMEM;
		goto out;
	}
	msm_chg->smb346_chg = &smb346_chg;
	dev_set_drvdata(msm_chg->smb346_chg->dev, msm_chg);
	/* kzalloc msm_chg->queue, and init workqueue thread */
	msm_chg->queue = kzalloc(sizeof(struct msm_charger_event)
				* MSM_CHG_MAX_EVENTS,
				GFP_KERNEL);
	if (!msm_chg->queue) {
		ERR_CHG("%s kzalloc msm_charger_event failed\n", __func__);
		ret = -ENOMEM;
		kfree(msm_chg);
		goto out;
	}
	
	msm_chg->tail = 0;
	msm_chg->head = 0;
	spin_lock_init(&msm_chg->queue_lock);
	msm_chg->queue_count = 0;
	INIT_WORK(&msm_chg->queue_work, process_events);
	msm_chg->event_wq_thread = create_singlethread_workqueue("msm_charger_eventd");
	if (!msm_chg->event_wq_thread) {
		ERR_CHG("%s create_workqueue msm_charger_eventd failed\n", __func__);
		ret = -ENOMEM;
		kfree(msm_chg->queue);
		kfree(msm_chg);
		goto out;
	}

	#ifndef FEATURE_AUTO_RECHARGE
	if (msm_chg->max_voltage == 0)
		msm_chg->max_voltage = DEFAULT_BATT_MAX_V;
	if (msm_chg->min_voltage == 0)
		msm_chg->min_voltage = DEFAULT_BATT_MIN_V;
	if (msm_chg->resume_voltage == 0)
		msm_chg->resume_voltage = DEFAULT_BATT_RESUME_V;
	#endif

	/*set smb346 7bits i2c address*/
	msm_chg->smb346_i2c_address = SMB346_I2C_ADDRESS;
	
	/*init smb346 lock*/
	mutex_init(&smb346_chg.smb346_lock);

	/* init mutex*/
	#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&msm_chg->smb346_wake_lock, WAKE_LOCK_SUSPEND, "smb346");
	#endif

	/* init timer for charge task*/
	charge_task_timer_init(msm_chg);
	/* this is for battery calibration */
	
#if 0
	if (is_suspend_threhold(msm_chg)) {
		vbatt_set_capacity(msm_chg);
	}
	else {
		update_psy_status(msm_chg);
	}	
#else
	if (is_suspend_threhold(msm_chg)){
		if (smb346_suspend_mode_on(msm_chg) == 0){
			smb346_battery_power_on_voltaget_set(msm_chg);
			battery_power_on_voltage_is_real = true;
			smb346_suspend_mode_off(msm_chg);
		}
		else{
			smb346_battery_power_on_voltaget_set(msm_chg);
			battery_power_on_voltage_is_real = false;
		}
	}
	else{
		smb346_battery_power_on_voltaget_set(msm_chg);
		battery_power_on_voltage_is_real = false;
	}
#endif	
	/* init charger and battery value */
	smb346_info_val_init(msm_chg);
	/* init register value. */
	smb346_reg_init_config(msm_chg);

	/* write EN register to stop charging to init status when boot. */
	ret = smb346_update(msm_chg, COMMAND_REGISTER_A, CHARGING_DISABLED, CHARGEING_ENABLED);
	if (ret){
		pr_err("%s: stop charging failed!\n",__func__);
	}

	ret = gpio_request(pdata->smb346_irq_gpio, "smb346_irq_gpio");
	if (ret) {
		pr_err("%s: gpio_request failed for pdata->smb346_irq_gpio\n",
								__func__);
	}

    ret = gpio_direction_input(pdata->smb346_irq_gpio);
	if (ret < 0) {
			pr_err("Failed to configure input direction for GPIO %d, error %d\n",
						pdata->smb346_irq_gpio, ret);
		    if (pdata->smb346_irq_gpio)
				gpio_free(pdata->smb346_irq_gpio);
	}
	
	smb346_chg.smb346_irq= gpio_to_irq(pdata->smb346_irq_gpio);

	/*chg_smb346_irq irq request*/
	ret = request_threaded_irq(smb346_chg.smb346_irq, NULL,
				   smb346_irq,
				   IRQF_DISABLED | IRQF_TRIGGER_FALLING,
				   "smb346_interrupt", NULL);
	if (ret) {
		pr_err("%s request_threaded_irq failed for %d rc =%d\n",
			__func__, smb346_chg.smb346_irq, ret);
		free_irq(smb346_chg.smb346_irq, NULL);
		goto out;
	}else{	
		disable_irq_nosync( smb346_chg.smb346_irq);
	}
	
	//enable_irq(smb346_chg.smb346_irq);
	
       if(smb346_chg.dev){
		/* creat log sys file for debug use */
		ret = device_create_file(smb346_chg.dev, &dev_attr_smb346_log);
		if (ret < 0) {
			ERR_CHG("%s: creat smb346 log file failed ret = %d\n", __func__, ret);
			device_remove_file(smb346_chg.dev, &dev_attr_smb346_log);
		}

		/* creat log sys file for suspend mode */
		ret = device_create_file(smb346_chg.dev, &dev_attr_smb346_suspend_mode);
		if (ret < 0) {
			ERR_CHG("%s: creat smb346 suspend mode failed ret = %d\n", __func__, ret);
			device_remove_file(smb346_chg.dev, &dev_attr_smb346_suspend_mode);
		}
		
		ret = device_create_file(smb346_chg.dev, &dev_attr_smb346_start_stop_charging);
		if (ret < 0) {
			ERR_CHG("%s: creat smb346_start_stop_charging failed ret = %d\n", __func__, ret);
			device_remove_file(smb346_chg.dev, &dev_attr_smb346_start_stop_charging);
		}
        #ifdef FTM_CHARGE_MODE_FEATURE
		ret = device_create_file(smb346_chg.dev, &dev_attr_smb346_chg);
		if (ret < 0) {
			ERR_CHG("%s: creat smb346_charge_mode_set failed ret = %d\n", __func__, ret);
			device_remove_file(smb346_chg.dev, &dev_attr_smb346_chg);
		}
		#endif
		ret = device_create_file(smb346_chg.dev, &dev_attr_smb346_start_stop_charging_conditional_while_calling);
		if (ret < 0){
			ERR_CHG("%s:create smb346_start_stop_charging_conditional_while_calling failed ret = %d\n", __func__, ret);
			device_remove_file(smb346_chg.dev, &dev_attr_smb346_start_stop_charging_conditional_while_calling);
		}
       }
	else
	{
	        ERR_CHG("%s:dev is NULL\n",__func__);
       }

	/* this global value is for usb charger detect use */	
	usb_msm_chg = msm_chg;

	/* add by songjie for startup charger type detect 2012-05-18 */
	if(is_chg_plugged_in()){
		//if chg plugged in, set charger status Charging
		smb346_charger_relative_battery_status_set(msm_chg, POWER_SUPPLY_STATUS_CHARGING);
		smb346_charging_status_set(msm_chg, SMB_CHG_STATUS__CHARGING);
	}
	/* end by songjie 2012-05-18 */	
	
	/*update battery info*/
	update_psy_status(usb_msm_chg);

	/* start timer for update heartbeat*/
	add_listener_for_update_heartbeat(msm_chg);

	/*check if FTM mode*/
	if(pdata){
		if(pdata->get_ftm_mode() == true){
			pr_err("%s: FTM mode ,suspend charge when timer expired!!\n ",__func__);
			hrtimer_start(&msm_chg->ftm_charge_suspend_timer, ktime_set(FTM_CHARGER_MODE_TIMER,0), HRTIMER_MODE_REL);	
		}
	}

	/*add by chendx 2011-11-24 for battery low suspend shutdown */
    ret = pm8xxx_batt_alarm_disable(PM8XXX_BATT_ALARM_UPPER_COMPARATOR);
	if (!ret)
		ret = pm8xxx_batt_alarm_disable(
			PM8XXX_BATT_ALARM_LOWER_COMPARATOR);
	if (ret) {
		pr_err("%s: unable to set batt alarm state\n", __func__);
		goto out;
	}

	/*
	 * The batt-alarm driver requires sane values for both min / max,
	 * regardless of whether they're both activated.
	 */
	ret = pm8xxx_batt_alarm_threshold_set(
			PM8XXX_BATT_ALARM_LOWER_COMPARATOR, VOL_LOW_RESUME_MV);
	if (!ret)
		ret = pm8xxx_batt_alarm_threshold_set(
			PM8XXX_BATT_ALARM_UPPER_COMPARATOR, 4300);
	if (ret) {
		pr_err("%s: unable to set batt alarm threshold\n", __func__);
		goto out;
	}

	ret = pm8xxx_batt_alarm_hold_time_set(
				PM8XXX_BATT_ALARM_HOLD_TIME_16_MS);
	if (ret) {
		pr_err("%s: unable to set batt alarm hold time\n", __func__);
		goto out;
	}

	/* PWM enabled at 2Hz */
	ret = pm8xxx_batt_alarm_pwm_rate_set(1, 7, 4);
	if (ret) {
		pr_err("%s: unable to set batt alarm pwm rate\n", __func__);
		goto out;
	}

	ret = pm8xxx_batt_alarm_register_notifier(&alarm_notifier);
	if (ret) {
		pr_err("%s: unable to register alarm notifier\n", __func__);
		goto out;
	}
	
	smb346_input = input_allocate_device();
	smb346_input->name = "smb346-chip";
	if (!smb346_input) {
		pr_err("%s: could not allocate input device\n", __func__);
		goto out;
	}

    input_set_capability(smb346_input, EV_KEY, KEY_POWER);

	ret = input_register_device(smb346_input);
	if (ret< 0) {
		pr_err("%s: can not register input device\n",
				__func__);
		input_free_device(smb346_input);	
		goto out;
	}
	 /*end by chendx 2011-11-24 for battery low suspend shutdown */

	 /*add by chendx reason stanard_mhl_wait init 2012-05-29*/
      init_completion(&stanard_mhl_wait);
	 /*add by chendx reason stanard_mhl_wait init 2012-05-29*/

	//wangjc
	_smb346_valid_handler(smb346_chg.mpp11_irq, NULL);
	 
	return 0;
	
out:
	return ret;
}

static int __devexit smb346_charger_remove(struct platform_device *pdev)
{
	struct smb346_charger_mux *msm_chg = usb_msm_chg;
	//struct smb346_charger_chip *chip = platform_get_drvdata(pdev);
	struct smb346_platform_pdata *pdata;
	int ret = -1;

	ERR_CHG("%s\n", __func__);

	/* destroy mutex */
	#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&msm_chg->smb346_wake_lock);
	#endif
	
	input_unregister_device(smb346_input);
	input_free_device(smb346_input);
	
	/* free irq */
	free_irq(smb346_chg.smb346_irq, NULL );

	/* destroy work thread data */
	flush_workqueue(msm_chg->event_wq_thread);
	destroy_workqueue(msm_chg->event_wq_thread);
	kfree(msm_chg->queue);

	/* remove log sys file */
	device_remove_file(smb346_chg.dev, &dev_attr_smb346_log);

    #ifdef FTM_CHARGE_MODE_FEATURE
	/* remove chg mode sys file */
	device_remove_file(smb346_chg.dev, &dev_attr_smb346_chg);
	#endif

	/* remove usb suspend sys file */
	device_remove_file(smb346_chg.dev, &dev_attr_smb346_suspend_mode);	
	device_remove_file(smb346_chg.dev, &dev_attr_smb346_start_stop_charging_conditional_while_calling);
	/* power off relevant ldo */
	pdata = smb346_chg.dev->platform_data;
	if (pdata && pdata->power) 
	{
		ret = pdata->power(0);
		if (ret < 0) 
		{
			ERR_CHG( "smb346_remove power off failed\n");
			return -1;
		}
	}	
	
	return 0;
}

static int smb346_suspend(struct platform_device *pdev,
				  pm_message_t state)
{
	struct smb346_charger_mux *msm_chg = usb_msm_chg;
    //   pr_info("%s:\n",__func__);
	/* cancel charging heart-beat work */
	if(msm_chg){
		remove_listener_for_update_heartbeat(msm_chg);
	}else{
		pr_err("%s: Error,msm_chg is NULL\n",__func__);
		return 0;
	}

	enable_irq_wake(smb346_chg.mpp11_irq);

	return 0;
}

static int smb346_resume(struct platform_device *pdev)
{
	struct smb346_charger_mux *msm_chg = usb_msm_chg;
       pr_info("%s:\n",__func__);

	disable_irq_wake(smb346_chg.mpp11_irq);
	   
	/* start charging heart-beat work time:UPDATE_TIME_S 10s */
	if(msm_chg){
		add_listener_for_update_heartbeat(msm_chg);
	}else{
		pr_err("%s: Error,msm_chg is NULL\n",__func__);
		return 0;
	}
	
	return 0;
}

static struct platform_driver smb346_charger_driver = {
	.probe = smb346_charger_probe,
	.remove = __devexit_p(smb346_charger_remove),
	.driver = {
		   .name = "smb346-charger",
		   .owner = THIS_MODULE,
	},
	.suspend  = smb346_suspend,
	.resume	  = smb346_resume,
};

static int __init smb346_charger_init(void)
{
	return platform_driver_register(&smb346_charger_driver);
}

static void __exit smb346_charger_exit(void)
{
	platform_driver_unregister(&smb346_charger_driver);
}

late_initcall(smb346_charger_init);
module_exit(smb346_charger_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("SMB346 CHARGING chip driver");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:smb346 charger");
