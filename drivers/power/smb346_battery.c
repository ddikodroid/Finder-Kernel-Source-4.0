/* Copyright (c) 2009-2010, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

/*
 * this needs to be before <linux/kernel.h> is loaded,
 * and <linux/sched.h> loads <linux/kernel.h>
 */
#define DEBUG  1
#define DVT_12001

#include <linux/earlysuspend.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/smb346-charger.h>
#include <asm/atomic.h>

#include <mach/msm_rpcrouter.h>
#include <mach/msm_battery.h>
#include <linux/rtc.h>
#include <linux/mfd/pm8xxx/batt-alarm.h>


#define BATTERY_LOW            	3500
#define BATTERY_HIGH           	4300

#define BATT_RPC_TIMEOUT    5000	/* 5 sec */
#define INVALID_BATT_HANDLE    -1

#define FEATURE_FROYO
#define THOUSAND	1000
#define TEN			10

#define BOOTING_COUNT	6 // 30s	
#define INCREASING_COUNT	10	//one min
#define LOWPOWER_INCREASING_COUNT	5	//half min
#define BATT_VOLT_FILTER_LENGTH	 60
#define BATT_VOLT_REPORT_RATE	10

#define CHG_COMPENSATE	50
#define RESUME_COMPENSATE	20

static u32 batt_filter_index = 0;
static u32 batt_volt_history[BATT_VOLT_FILTER_LENGTH] = {0, 0, 0};
static u32 poll_time = BOOTING_COUNT;

static u32 fulegague_capacity_last = 0; 


#define vbat_abs(x, y) ((x < y) ? y-x : x-y)
#define vbat_min(x,y)  ((x) < (y) ? (x):(y))

#define DEBUG_BATT(fmt, ...) \
	({ if (is_show_batt_log()) printk(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__); 0; })
#define ERR_BATT(fmt, ...) printk(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__)

struct smb346_battery {
	struct smb346_battery_platform_data *pdata;
	struct device *dev;
};

//#define INTERVAL_CHARGING_FEATURE

static struct smb346_battery smb346_batt;

extern bool get_charging_status(void);
extern int smb346_mpp11_irq_register(void);
#ifdef CONFIG_ACCESSORY_BATTERY
extern int get_accessory_capacity(void);
extern int get_accessory_charge_status(void);
#endif

/*battery fuelgague function*/
static struct oppo_battery_fuelgauge *batt_fuelgauge = NULL;
/*battery fuelgague function*/

struct msm_battery_info {
	u32 voltage_max_design;
	u32 voltage_min_design;
	u32 chg_api_version;
	u32 batt_technology;
	u32 batt_api_version;

	u32 avail_chg_sources;
	u32 current_chg_source;

	u32 batt_status;
	u32 batt_health;
	u32 charger_valid;
	u32 batt_valid;
	u32 batt_capacity; /* in percentage */
	u32 fulegague_capacity; /* in percentage */

	u32 charger_status;
	u32 charger_type;
	u32 battery_status;
	u32 battery_level;
	u32 battery_voltage; /* in millie volts */
	u32 battery_temp;  /* in celsius */

	u32 charger_voltage; /*in millie volts*/
	u32 charger_current; /*in mA*/
	bool is_adapt_completed;
	//bool is_charging_while_sleep;
	//u32 sleep_charge_second;

	u32(*calculate_capacity) (u32 voltage);

	s32 batt_handle;

	struct power_supply *msm_psy_ac;
	struct power_supply *msm_psy_usb;
	struct power_supply *msm_psy_batt;
	struct power_supply *current_ps;

	struct msm_rpc_client *batt_client;
	struct msm_rpc_endpoint *chg_ep;

	wait_queue_head_t wait_q;

	u32 vbatt_modify_reply_avail;

	struct early_suspend early_suspend;

};

static struct msm_battery_info msm_batt_info = {
	.batt_handle = INVALID_BATT_HANDLE,
	.charger_status = CHARGER_STATUS_BAD,
	.charger_type = CHARGER_TYPE_NONE,
	.battery_status = BATTERY_STATUS_GOOD,
	.battery_level = BATTERY_LEVEL_GOOD,
	.battery_voltage = BATTERY_HIGH,
	.batt_capacity = 100,
	.fulegague_capacity = 100,
	.batt_status = POWER_SUPPLY_STATUS_DISCHARGING,
	.batt_health = POWER_SUPPLY_HEALTH_GOOD,
	.batt_valid  = 1,
	.battery_temp = 23,
	/* OPPO 2010-10-08 wangjc Add begin for add voltage & current param */
	.charger_voltage = 5000,
	.charger_current = 0,
	/* OPPO 2010-10-08 wangjc Add end */
	//.is_charging_while_sleep = false,
	//.sleep_charge_second = 0,
	.vbatt_modify_reply_avail = 0,
};

static enum power_supply_property msm_power_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static char *msm_power_supplied_to[] = {
	"battery",
};


#define BATTERY_DEFAULT_VOLTAGE 3900
#define BATTERY_DEFAULT_TEMP 230
static int get_prop_battery_mvolts(void)
{
    if (batt_fuelgauge && batt_fuelgauge->get_battery_mvolts) {
		return batt_fuelgauge->get_battery_mvolts();
	}else{
	        pr_err("%s:Warning: batt_fuelgauge not ready!\n",__func__);
		 return BATTERY_DEFAULT_VOLTAGE;
	}
}

static int get_battery_temperature(void)
{
	if (batt_fuelgauge && batt_fuelgauge->get_battery_temperature) {
		return batt_fuelgauge->get_battery_temperature();
	}else{
	        pr_err("%s:Warning: batt_fuelgauge not ready!\n",__func__);
		 return BATTERY_DEFAULT_TEMP;
	}
}

int get_prop_vchg_movlts(void)
{
	if (batt_fuelgauge && batt_fuelgauge->get_vchg_mvolts) {
		return batt_fuelgauge->get_vchg_mvolts();
	}else{
	        pr_err("%s:Warning: batt_fuelgauge not ready!\n",__func__);
		 return 0;
	}
}
EXPORT_SYMBOL(get_prop_vchg_movlts);

static int get_prop_ichg_current(void)
{
	if (batt_fuelgauge && batt_fuelgauge->get_ichg_current) {
		return batt_fuelgauge->get_ichg_current();
	}else{
	        pr_err("%s:Warning: batt_fuelgauge not ready!\n",__func__);
		 return 0;
	}
}

//Todo
extern bool fuelgague_is_present(void);
extern void fuelgague_is_present_check(void);

static int set_prop_full_capacity(void)
{
	if (batt_fuelgauge && batt_fuelgauge->set_full_capacity) {
		return batt_fuelgauge->set_full_capacity();
	}else{
	        pr_err("%s:Warning: batt_fuelgauge not ready!\n",__func__);
		 return -ENODEV;
	}
}

static int get_prop_battery_capacity(void)
{
	if (batt_fuelgauge && batt_fuelgauge->get_battery_capacity) {
		return batt_fuelgauge->get_battery_capacity();
	}else{
	        pr_err("%s:Warning: batt_fuelgauge not ready!\n",__func__);
		 return 30;
	}
}
static int msm_power_get_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  union power_supply_propval *val)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		if (psy->type == POWER_SUPPLY_TYPE_MAINS) {
			val->intval = msm_batt_info.current_chg_source & AC_CHG
			    ? 1 : 0;
		}
		
		if (psy->type == POWER_SUPPLY_TYPE_USB) {
			val->intval = msm_batt_info.current_chg_source & USB_CHG
			    ? 1 : 0;
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static struct power_supply msm_psy_ac = {
	.name = "smb346-ac",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.supplied_to = msm_power_supplied_to,
	.num_supplicants = ARRAY_SIZE(msm_power_supplied_to),
	.properties = msm_power_props,
	.num_properties = ARRAY_SIZE(msm_power_props),
	.get_property = msm_power_get_property,
};

static struct power_supply msm_psy_usb = {
	.name = "smb346-usb",
	.type = POWER_SUPPLY_TYPE_USB,
	.supplied_to = msm_power_supplied_to,
	.num_supplicants = ARRAY_SIZE(msm_power_supplied_to),
	.properties = msm_power_props,
	.num_properties = ARRAY_SIZE(msm_power_props),
	.get_property = msm_power_get_property,
};

static enum power_supply_property msm_batt_power_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CHARGE_TYPE,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CHARGE_NOW,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_ACCESSORY_PRESENT,
	POWER_SUPPLY_PROP_ACCESSORY_STATUS,
	POWER_SUPPLY_PROP_ACCESSORY_CAPACITY,
};
#ifdef CONFIG_ACCESSORY_BATTERY
extern int get_accessory_present_status(void);
extern int get_accessory_charge_status(void);
extern int get_accessory_capacity(void);
#endif
static int msm_batt_power_get_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       union power_supply_propval *val)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = msm_batt_info.batt_status;
		break;
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		val->intval = POWER_SUPPLY_CHARGE_TYPE_FAST;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = msm_batt_info.batt_health;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = msm_batt_info.batt_valid;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		val->intval = msm_batt_info.voltage_max_design;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
		val->intval = msm_batt_info.voltage_min_design;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
              val->intval = msm_batt_info.battery_voltage * 1000;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		//Todo
		if(fuelgague_is_present()){
			if (msm_batt_info.battery_level == BATTERY_LEVEL_FULL){
				set_prop_full_capacity();
		        val->intval = 100;
	        }else{
				 val->intval = msm_batt_info.fulegague_capacity;
	        }
			DEBUG_BATT("%s: update fulegague capacity %d%%\n",__func__,msm_batt_info.fulegague_capacity);
		}else{
			if(msm_batt_info.battery_voltage > 3500)
				val->intval = 30;//msm_batt_info.batt_capacity;
			else
				val->intval = msm_batt_info.batt_capacity;
		}
		break;
        case POWER_SUPPLY_PROP_CHARGE_NOW:
		val->intval = msm_batt_info.charger_voltage;
		break;
	 case POWER_SUPPLY_PROP_TEMP:
	 	val->intval = msm_batt_info.battery_temp;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = msm_batt_info.charger_current;
		break;
#ifdef CONFIG_ACCESSORY_BATTERY
	case POWER_SUPPLY_PROP_ACCESSORY_PRESENT:
		val->intval = get_accessory_present_status();
		break;
	case POWER_SUPPLY_PROP_ACCESSORY_STATUS:
		val->intval = get_accessory_charge_status();
		break;
	case POWER_SUPPLY_PROP_ACCESSORY_CAPACITY:
		val->intval = get_accessory_capacity();
		break;
#endif
	default:
		return -EINVAL;
	}
	return 0;
}

static struct power_supply msm_psy_batt = {
	.name = "battery",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = msm_batt_power_props,
	.num_properties = ARRAY_SIZE(msm_batt_power_props),
	.get_property = msm_batt_power_get_property,
};

static void msm_batt_qury_adc_battery_params(struct smb346_battery_adc_params_type* smb346_battery__params)
{
	int ichg__value=0;
	int vbatt__vol=3900;
	int vbatt__temp=0;
    //pr_debug("%s:update battery adc params\n",__func__);
	   
    /*update battery info*/
	vbatt__vol = 	get_prop_battery_mvolts();
	smb346_battery__params->current_batt_adc_voltage = vbatt__vol;

	vbatt__temp = get_battery_temperature();
	smb346_battery__params->batt_temperature = vbatt__temp ;
	
	//vchg = get_prop_vchg_movlts();
	//smb346_battery__params->vchg_mv = vchg;
	
	ichg__value = get_prop_ichg_current();
	smb346_battery__params->ichg_mv = ichg__value;	
	smb346_battery__params->batt_capacity = get_prop_battery_capacity();;

	return;
}

typedef struct
{
   s32 x;
   s32 y;
} VoltagetoCapacity;

#ifndef DVT_12001
static const VoltagetoCapacity v2c_map[] =
{
   { 3400,      0 },	//@0
   { 3500,      1 },	//@1--red
   { 3550,      7 },	//@2
   { 3600,    12 },	//@3
   { 3620,    16 },	//@4--orange
   { 3641,    20 },	//@5
   { 3661,    24 },	//@6
   { 3682,    28 },	//@7
   { 3702,    35 },	//@8--green
   { 3723,    42 },	//@9
   { 3743,    49 },	//@10
   { 3763,    56 },	//@11
   { 3784,    60 },	//@12
   { 3801,    64 },	//@13
   { 3825,    68 },	//@14
   { 3845,    72 },	//@15
   { 3866,    75 },	//@16
   { 3886,    77 },	//@17
   { 3907,    80 },	//@18
   { 3927,    82 },	//@19
   { 3948,    85 },	//@20
   { 3968,    87 },	//@21
   { 3989,    90 },	//@22
   { 4009,    92 },	//@23
   { 4029,    94 },	//@24
   { 4050,    96 },	//@25
   { 4160,  100 }	//FULL
};
#else
static const VoltagetoCapacity v2c_map[] =
{
   //11075 provide by HW Team 2011-10-18
   { 3400,     0 },	//@0	
   { 3500,     1 },	//@1
   { 3550,     7 },	//@2	
   { 3600,    12 },	//@3	
   { 3618,    16 },	//@4
   { 3649,    20 },	//@5
   { 3673,    24 },	//@6
   { 3686,    28 },	//@7
   { 3695,    32},	//@8
   { 3702,    36 },	//@9
   { 3711,    40 },	//@10	
   { 3722,    44 },	//@11
   { 3735,    48 },	//@12	
   { 3750,    52 },	//@13
   { 3767,    56 },	//@14
   { 3788,    60 },	//@15
   { 3812,    64 },	//@16
   { 3838,    68 },	//@17
   { 3866,    72 },	//@18
   { 3894,    76 },	//@19
   { 3923,    80 },	//@20
   { 3964,    84 },	//@21
   { 4001,    88 },	//@22	
   { 4035,    92 },	//@23
   { 4075,    96 },	//@24
   { 4135,  100 },	//FULL
};
#endif
#define BATT_POWER_ON_VOL_3P6  		3600
#define BATT_POWER_ON_VOL_3P673 		3673
#define BATT_POWER_ON_VOL_3P686 		3686
#define BATT_POWER_ON_VOL_3P722 		3722
#define BATT_POWER_ON_VOL_3P788 		3788
#define BATT_POWER_ON_VOL_3P923		3923
#define BATT_POWER_ON_VOL_4P135		4135

#define BATT_POWER_ON_VOL_COMPENSATE_10MV 10
#define BATT_POWER_ON_VOL_COMPENSATE_15MV 15
#define BATT_POWER_ON_VOL_COMPENSATE_20MV 20
#define BATT_POWER_ON_VOL_COMPENSATE_25MV 25
#define BATT_POWER_ON_VOL_COMPENSATE_30MV 30

#define BATT_POWER_ON_CHARGING_USB_PC_VOL_COMPENSATE 60
#define BATT_POWER_ON_CHARGING_USB_WALL_VOL_COMPENSATE 150
#define BATT_POWER_ON_CHARGING_NON_STANDARD_VOL_COMPENSATE 60

extern chg_charger_hardware_type get_charger_type(void);
extern int get_battery_power_on_voltage(void);
extern bool get_battery_power_on_voltage_is_real(void);

static u32 batt_power_on_voltage_compensate(u32 batt_vol_now)
{
	u32 batt_vol_after_compensate = get_battery_power_on_voltage();

	ERR_BATT("%s:power on with charging vol =%d\n", __func__, batt_vol_after_compensate);
	if (get_battery_power_on_voltage_is_real() == false){
		ERR_BATT("%s:power on voltage is not true value\n", __func__);
		if (true == get_charging_status()){
			switch (get_charger_type()){
				case CHARGER_TYPE_USB_WALL:
					batt_vol_after_compensate -= BATT_POWER_ON_CHARGING_USB_WALL_VOL_COMPENSATE;
					break;
				case CHARGER_TYPE_USB_PC:
					batt_vol_after_compensate -= BATT_POWER_ON_CHARGING_USB_PC_VOL_COMPENSATE;
					break;
				case CHARGER_TYPE_NON_STANDARD:
					batt_vol_after_compensate -= BATT_POWER_ON_CHARGING_NON_STANDARD_VOL_COMPENSATE;
					break;
				default:
					break;
			}
		}
	}
	else
		ERR_BATT("%s:power on voltage is true value\n", __func__);
	
	if (batt_vol_now < BATT_POWER_ON_VOL_3P6){
		batt_vol_after_compensate += BATT_POWER_ON_VOL_COMPENSATE_30MV;
	}else if (batt_vol_now < BATT_POWER_ON_VOL_3P673 - 10){
		batt_vol_after_compensate += BATT_POWER_ON_VOL_COMPENSATE_25MV;
	}else if (batt_vol_now < BATT_POWER_ON_VOL_3P686){
		batt_vol_after_compensate += BATT_POWER_ON_VOL_COMPENSATE_25MV;
	}else if (batt_vol_now < BATT_POWER_ON_VOL_3P722){
		batt_vol_after_compensate += BATT_POWER_ON_VOL_COMPENSATE_15MV;
	}else if (batt_vol_now < BATT_POWER_ON_VOL_3P788){
	 	batt_vol_after_compensate += BATT_POWER_ON_VOL_COMPENSATE_25MV;
	}else if (batt_vol_now < BATT_POWER_ON_VOL_3P923){
		batt_vol_after_compensate += BATT_POWER_ON_VOL_COMPENSATE_25MV;
	}else if (batt_vol_now < BATT_POWER_ON_VOL_4P135){
		batt_vol_after_compensate += BATT_POWER_ON_VOL_COMPENSATE_15MV;
	}
	ERR_BATT("%s:power on with charging after compenstate vol =%d\n", __func__, batt_vol_after_compensate);
	return batt_vol_after_compensate;
}

/**
 * OPPO 2010-10-08 wangjc Add
 * DESCRIPTION: - map liner integer to another integer 
 * @Param: VoltagetoCapacity, nTableSize, input, output
 * @Return: int
 */ 
static int map_liner_int32_to_int32(const VoltagetoCapacity *paPts, u32 nTableSize, s32 input, s32 *output)
{
	bool bDescending = true;
	u32 nSearchIdx = 0;

	if ((paPts == NULL) || (output == NULL))
	{
		return -EINVAL;
	}

	/* Check if table is descending or ascending */
	if (nTableSize > 1)
	{
		if (paPts[0].x < paPts[1].x)
		{
			bDescending = false;
		}
	}

	while (nSearchIdx < nTableSize)
	{
		if ( (bDescending == true) && (paPts[nSearchIdx].x < input) )
		{
			/* table entry is less than measured value and table is descending, stop */
			break;
		}
		else if ( (bDescending == false) && (paPts[nSearchIdx].x > input) )
		{
			/* table entry is greater than measured value and table is ascending, stop */
			break;
		}
		else
		{
			nSearchIdx++;
		}
	}

	if (nSearchIdx == 0)
	{
		*output = paPts[0].y;
	}
	else if (nSearchIdx == nTableSize)
	{
		*output = paPts[nTableSize-1].y;
	}
	else
	{
		/* result is between search_index and search_index-1 */
		/* interpolate linearly */
		*output = (
		       ( (s32)
		           (
		            (paPts[nSearchIdx].y - paPts[nSearchIdx-1].y)
		             * (input - paPts[nSearchIdx-1].x)
		           )
		           / (paPts[nSearchIdx].x - paPts[nSearchIdx-1].x)
		       )
		       + paPts[nSearchIdx-1].y
		     );
	}
   return 0;
}

static u32 msm_batt_capacity(u32 current_voltage)
{
	u32 capacity;
	int ret;

	ret = map_liner_int32_to_int32(v2c_map, sizeof(v2c_map)/sizeof(v2c_map[0]), current_voltage, &capacity);
	if(ret)
		return ret;

	return capacity;
}

/**
 * OPPO 2011-09-23 shijiangwei Add
 * DESCRIPTION: - get the battery voltage decreasing rate count.
 * @param: u32 pre_vol, u32 now_vol.
 * @return: voltage decreasing count.
 * our battery voltage  decrease with corresponding current
 *	100mA	200mA	300mA	400mA	500mA	600mA	
 *	18.3		33.4		51.6		66.3		82.5		102.3
 */ 
u32 vol_dec_cnt(u32 pre_vol, u32 now_vol)
{
	u32 diff_vol = vbat_abs(now_vol, pre_vol);

/* OPPO 2011-11-23 shijiangwei Modify begin for reason */
#if 0
	//current (100, 200)
	if (diff_vol < 15)
		return 8;
	//current (200, 300)
	else if (diff_vol < 33)
		return 5;
	//current (300, 400)
	else if (diff_vol < 48)
		return 4;	
	//current (400, 500)
	else if (diff_vol < 64)
		return 3;
	//current (500, 600)
	else if (diff_vol < 84)
		return 2;
	//current (600+)
	else
		return 1;	
#else
	//current (200, 300)
	if (diff_vol < 18)
		return 5;
	//current (300, 400)
	else if (diff_vol < 33)
		return 4;	
	//current (400, 500)
	else if (diff_vol < 49)
		return 3;
	//current (500, 600)
	else if (diff_vol < 69)
		return 2;
	//current (600+)
	else
		return 1;		
#endif
/* OPPO 2011-11-23 shijiangwei Modify end */
}

/**
 * OPPO 2011-09-26 shijiangwei Add
 * DESCRIPTION: - Power Battery voltage compensation.
 * @param: vol.
 * @return: u32.
 */ 
u32 vol_filter_avg(void)
{
	u32 count = 0;
	u32 sum = 0;
	u32 index = 0;
	u32 vol_base = 0;

	for(index = 0; index < (BATT_VOLT_FILTER_LENGTH); index++)
	{
		if(batt_volt_history[index] == 0)
			continue;		
		sum += batt_volt_history[index];
		count++;
	}
	vol_base = sum / count;	

	return vol_base;
}
/**
 * OPPO 2011-10-06 shijiangwei Add
 * DESCRIPTION: - batt volt filter fuction.
 * @param: void.
 * @return: None.
 */ 
static u32 batt_volt_filter(u32 batt_volt_now, bool not_charging)
{
	u32 vol = 0;
	u32 batt_volt_capa;
	u32 batt_volt_pre;
	u32 dec_cnt = 0;
	u32 temp_index = 0;
	u32 batt_volt_temp = 0;

	DEBUG_BATT("batt_filter_index = %d\n", batt_filter_index);

	batt_volt_pre = batt_volt_history[(batt_filter_index + BATT_VOLT_FILTER_LENGTH -1)%BATT_VOLT_FILTER_LENGTH];	
	if (not_charging) {
		if (poll_time != 0) {
			vol = batt_power_on_voltage_compensate(batt_volt_now);
			poll_time--;
		}
		else 
		{
			if (batt_volt_pre > 3650) {
				dec_cnt = vol_dec_cnt(batt_volt_pre, batt_volt_now);
				DEBUG_BATT("dec_cnt = %d\n", dec_cnt);
				if (batt_filter_index % dec_cnt == 0) {
					if (batt_volt_now < batt_volt_pre)
						vol = batt_volt_pre - 1;
					else
						vol = batt_volt_pre + 1;
				}
				else {
					vol = batt_volt_pre;
				}
			}
			else {
				vol = batt_volt_now;
			}
		}		
	}
	else {
		if (poll_time != 0) {
			vol = batt_power_on_voltage_compensate(batt_volt_now);
			poll_time--;
		}
		else {
			
			if (batt_volt_pre > 3500) 
			{
				for(temp_index = 0; temp_index < BATT_VOLT_FILTER_LENGTH; temp_index++)
				{
					if (batt_volt_history[(batt_filter_index+temp_index)%BATT_VOLT_FILTER_LENGTH] != 0)
						break;
				}
				
				temp_index = (batt_filter_index+temp_index)%BATT_VOLT_FILTER_LENGTH;
				batt_volt_temp = batt_volt_history[temp_index];
				vol = (batt_volt_now + batt_volt_temp) / 2;	

			}
			else {
				vol = batt_volt_now;
				batt_volt_capa = batt_volt_now;
				memset(batt_volt_history, 0, BATT_VOLT_FILTER_LENGTH * sizeof(u32));
				batt_filter_index = 0;
				batt_volt_history[batt_filter_index++] = vol;
				return batt_volt_capa;
			}
		}
	}
			
	batt_filter_index %= BATT_VOLT_FILTER_LENGTH;
	batt_volt_history[batt_filter_index++] = vol;
	batt_volt_capa = vol_filter_avg();

	DEBUG_BATT("temp_index = %d\n", temp_index);
	DEBUG_BATT("batt_volt_temp = %d\n", batt_volt_temp);
	DEBUG_BATT("batt_volt_read = %d\n", batt_volt_now);
	DEBUG_BATT("batt_volt_pre = %d\n", batt_volt_pre);	
	DEBUG_BATT("vol = %d\n", vol);
	DEBUG_BATT("batt_volt_capa = %d\n", batt_volt_capa);
	
	return batt_volt_capa;
}
#define MAX_CAPACITY 100
#define MAX_UINT 65535
#define DISCHARGING_CURRENT_200_MA	200
#define DISCHARGING_CURRENT_350_MA	350
#define DISCHARGING_CURRENT_500_MA	500
#define DISCHARGING_CURRENT_650_MA	650

static int capacity_to_voltage_map_with_150ma[MAX_CAPACITY + 1] = {
	3398,3416,3486,3539,3583,3610,3621,3630,3635,3639,
	3643,3646,3649,3653,3662,3673,3682,3691,3699,3705,
	3710,3714,3719,3722,3726,3728,3731,3733,3735,3736,
	3739,3740,3742,3744,3746,3747,3750,3752,3754,3756,
	3758,3761,3763,3766,3769,3771,3777,3781,3784,3788,
	3791,3795,3799,3803,3808,3813,3819,3826,3831,3838,
	3844,3851,3857,3864,3870,3876,3882,3888,3894,3900,
	3906,3912,3919,3925,3932,3938,3945,3952,3959,3966,
	3973,3980,3987,3996,4003,4011,4019,4027,4035,4044,
	4053,4061,4070,4079,4089,4098,4108,4117,4129,4141,
	4173
};
static int capacity_to_voltage_map_with_300ma[MAX_CAPACITY + 1] = {
	3470,3521,3559,3580,3592,3601,3608,3613,3618,3623,
	3627,3633,3642,3652,3661,3668,3675,3681,3686,3690,
	3694,3698,3701,3704,3706,3709,3711,3713,3715,3718,
	3720,3722,3724,3726,3728,3730,3732,3734,3736,3739,
	3742,3745,3747,3751,3754,3757,3761,3764,3768,3772,
	3776,3780,3784,3789,3793,3798,3804,3809,3815,3821,
	3827,3833,3840,3847,3853,3860,3867,3874,3880,3887,
	3894,3901,3908,3915,3922,3929,3936,3944,3951,3959,
	3967,3975,3983,3991,4000,4008,4017,4025,4034,4043,
	4052,4061,4071,4081,4090,4101,4112,4123,4135,4150,
	4280
};
static int capacity_to_voltage_map_with_450ma[MAX_CAPACITY + 1] = {
	3427,3370,3480,3520,3545,3560,3571,3580,3586,3598,
	3603,3608,3615,3623,3631,3639,3646,3652,3657,3662,
	3667,3671,3674,3677,3681,3683,3686,3689,3691,3694,
	3696,3698,3701,3703,3706,3708,3710,3713,3715,3718,
	3721,3724,3727,3730,3733,3736,3739,3743,3746,3750,
	3754,3758,3761,3766,3770,3775,3779,3784,3789,3794,
	3799,3805,3810,3816,3823,3828,3835,3841,3848,3854,
	3861,3868,3874,3881,3888,3895,3902,3909,3917,3924,
	3932,3939,3947,3955,3963,3971,3980,3988,3996,4005,
	4014,4023,4032,4042,4051,4061,4071,4081,4092,4107,
	4172
};
static int capacity_to_voltage_map_with_600ma[MAX_CAPACITY + 1] = {
	3471,3499,3518,3532,3543,3553,3561,3568,3574,3580,
	3586,3593,3600,3608,3615,3622,3627,3632,3637,3641,
	3645,3649,3653,3656,3659,3662,3665,3668,3670,3673,
	3676,3679,3681,3684,3687,3689,3692,3695,3698,3701,
	3704,3707,3710,3714,3717,3720,3724,3728,3732,3736,
	3740,3744,3749,3753,3754,3765,3767,3772,3777,3783,
	3788,3793,3799,3805,3812,3818,3824,3830,3837,3844,
	3851,3857,3865,3872,3879,3886,3894,3901,3910,3918,
	3925,3933,3942,3950,3959,3967,3976,3984,3993,3997,
	4002,4011,4020,4030,4039,4049,4061,4071,4082,4111,
	4197
};
static int capacity_to_voltage_map_with_750ma[MAX_CAPACITY + 1] = {
	3428,3442,3471,3491,3506,3520,3539,3547,3554,3560,
	3563,3566,3573,3586,3590,3593,3599,3604,3609,3614,
	3619,3623,3626,3631,3634,3637,3641,3644,3647,3653,
	3656,3658,3662,3664,3667,3670,3673,3676,3679,3682,
	3685,3687,3689,3692,3696,3699,3703,3706,3710,3714,
	3719,3722,3726,3735,3740,3744,3749,3754,3759,3764,
	3770,3775,3780,3786,3792,3797,3805,3811,3817,3823,
	3830,3837,3843,3850,3857,3864,3871,3880,3887,3895,
	3903,3911,3919,3927,3937,3953,3954,3963,3971,3975,
	3980,3989,3999,4017,4027,4031,4039,4049,4060,4074,
	4091
};
static void msm_batt_update_psy_status(struct smb346_charger_info_type* smb346_charger_info,
	struct smb346_battery_adc_params_type* smb346_battery_adc_params)
{
	struct	power_supply	*supp = NULL;
	u32  battery_voltage = 0;
	s32 charger_current = 0;
	s32 temperature = 0;
	u32 charger_voltage = 0;
	u32 battery_voltage_capacity = 0;
	u32 batt_capacity = 0;
	u32 batt_health = 0;
	bool not_charging = true;
	char *batt_status_string[] = {"unknow", "charing", "discharing", "not charing", "full"};
	chg_charger_hardware_type charger_type = CHARGER_TYPE_NONE;
	chg_charger_status_type charger_status = CHARGER_STATUS_INVALID;

#ifdef CONFIG_OPPO_MODIFY
	u32 fulegague_capacity = smb346_battery_adc_params->batt_capacity;
	u32 voltage_capacity  = 0;
	static int capacity_count = 0;
	int index_cap = 0;
	int discharging_current= 0;
	int *map_table = NULL;
	static int vol_last_index = 0;
	int  avg_count= 0;
	static uint fulegague_vol_last[3] = {0,0,0};
	uint fulegague_voltage_avg = 0;

#endif
#if 1//to make sure that the capacity is not less than that before charger connected
	static bool is_charger_just_connected = false;
	static u32 batt_last_capacity = 0;

	if(fuelgague_is_present() == false){
		fuelgague_is_present_check();
	}
///
#ifdef CONFIG_OPPO_MODIFY
	if (fulegague_capacity >= 98) {
		fulegague_capacity = 100;
	}
	else {
		fulegague_capacity = 99*fulegague_capacity/97;
	}
	
	if (get_charging_status() == true){
		if (fulegague_capacity_last != 0) {
			if(abs(fulegague_capacity-fulegague_capacity_last) > 1){
				if(fulegague_capacity>fulegague_capacity_last)
					fulegague_capacity = fulegague_capacity_last+1;
				else
					fulegague_capacity = fulegague_capacity_last;
			}
		}
		goto out;
	}

	if (fulegague_capacity_last != 0) {
		if(abs(fulegague_capacity-fulegague_capacity_last) > 1){
			if(fulegague_capacity>fulegague_capacity_last)
				fulegague_capacity = fulegague_capacity_last;
			else
				fulegague_capacity = fulegague_capacity_last-1;
		}
	}
	
	if (fulegague_capacity >= 30){
		goto out;
	}else{
		discharging_current = MAX_UINT - smb346_battery_adc_params->ichg_mv;
		DEBUG_BATT("discharging current=%d ma\n", discharging_current);
		if (discharging_current <= DISCHARGING_CURRENT_200_MA)
			map_table = capacity_to_voltage_map_with_150ma;
		else if (discharging_current <= DISCHARGING_CURRENT_350_MA)
			map_table = capacity_to_voltage_map_with_300ma;
		else if (discharging_current <= DISCHARGING_CURRENT_500_MA)
			map_table = capacity_to_voltage_map_with_450ma;
		else if (discharging_current <= DISCHARGING_CURRENT_650_MA)
			map_table = capacity_to_voltage_map_with_600ma;
		else {
			map_table = capacity_to_voltage_map_with_750ma;
		}
		 //calculate avg voltage in 3 times
		fulegague_vol_last[vol_last_index++] = smb346_battery_adc_params->current_batt_adc_voltage;
		if (vol_last_index > 2)
			vol_last_index = 0;
		for(index_cap = 0; index_cap < 3; index_cap++){
				if (fulegague_vol_last[index_cap] != 0){
					fulegague_voltage_avg += fulegague_vol_last[index_cap];
					avg_count++;
				}
		}
		fulegague_voltage_avg = fulegague_voltage_avg / avg_count;
		pr_info("fulegague_voltage_avg = %d mv\n", fulegague_voltage_avg);
		if (fulegague_capacity == 0 && fulegague_voltage_avg > 3500){
			fulegague_capacity = 1;
			ERR_BATT("fulegague_capacity read from hardware is 0,but avg is larger than 3500mv ,so we set fulegague_capacity to 1");
			goto out;
		}
		//get hte capacity that the avg voltage map form different discharging current
		for (index_cap = 0; index_cap < MAX_CAPACITY; index_cap++){
			if (map_table[index_cap] >= fulegague_voltage_avg &&  fulegague_voltage_avg < map_table[index_cap + 1 ] ){
				voltage_capacity = index_cap;
				break;
			}
		}
		pr_info("voltage_capactiy from map:%d, fulegague_capacity %d\n", voltage_capacity, fulegague_capacity);
		if (voltage_capacity - fulegague_capacity >= 0)
			goto out;
		else if (fulegague_capacity - voltage_capacity >= 6){
			ERR_BATT("the gap that capacity from voltage and fulegague is large,we use capacity as the reference from voltage");
			capacity_count++;
			if (capacity_count == 2 ){
				if (fulegague_capacity_last == 0)
					fulegague_capacity -= 1;
				else
					fulegague_capacity = fulegague_capacity_last -1;
				capacity_count = 0;
				if (fulegague_capacity <= voltage_capacity)
					fulegague_capacity = voltage_capacity;
			}
		}else
			capacity_count = 0;
	}
out:
		msm_batt_info.fulegague_capacity = fulegague_capacity;
		fulegague_capacity_last = msm_batt_info.fulegague_capacity;
#endif
///
	charger_type = msm_batt_info.charger_type;
	if (charger_type == CHARGER_TYPE_INVALID && 
		smb346_charger_info->charger_type != CHARGER_TYPE_INVALID){
		ERR_BATT("%s: charger is just connected\n", __func__);
		is_charger_just_connected = true;
	}else{
		is_charger_just_connected = false;
	}
	
	batt_last_capacity = msm_batt_info.batt_capacity;
	DEBUG_BATT("%s:batt last capacity ist %u\n", __func__, batt_last_capacity);
#endif	
	temperature = smb346_battery_adc_params->batt_temperature;
	charger_voltage = smb346_battery_adc_params->vchg_mv;
	battery_voltage = smb346_battery_adc_params->current_batt_adc_voltage;
	charger_current = smb346_battery_adc_params->ichg_mv;
	
	charger_type = smb346_charger_info->charger_type;
	
	charger_status = smb346_charger_info->charger_status;
	batt_health = smb346_charger_info->batt_health;

	/* battery voltage and capacity relatively treation */
#if 0
	not_charging = ((charger_status == CHARGER_STATUS_INVALID) || 
		(charger_status ==CHARGER_STATUS_BAD) || 
		(batt_health == POWER_SUPPLY_HEALTH_OVERVOLTAGE));
#else
	if (get_charging_status() == true)
		not_charging = false;
	else
		not_charging = true;
#endif

	if (!not_charging) {
		#ifdef FEATURE_SUSPEND_CHARGE
		battery_voltage -= CHG_COMPENSATE;
		#endif
	}
	
	msm_batt_info.battery_voltage = battery_voltage;
	battery_voltage_capacity = batt_volt_filter(battery_voltage, not_charging);
	batt_capacity = msm_batt_info.calculate_capacity(battery_voltage_capacity);

	if (msm_batt_info.batt_status != smb346_charger_info->batt_status) {
		DEBUG_BATT("batt_status_pre = %s\n", batt_status_string[msm_batt_info.batt_status]);
		DEBUG_BATT("batt_status_now = %s\n", batt_status_string[smb346_charger_info->batt_status]);
	}

	/* update battery info and charger info */
	msm_batt_info.battery_temp = temperature;
	msm_batt_info.charger_voltage = charger_voltage;
	msm_batt_info.charger_current = charger_current;
	
	msm_batt_info.batt_status = smb346_charger_info->batt_status;
	msm_batt_info.batt_health = smb346_charger_info->batt_health;
	msm_batt_info.batt_valid = smb346_charger_info->batt_valid;
	msm_batt_info.battery_status = smb346_charger_info->battery_status;
	msm_batt_info.battery_level = smb346_charger_info->battery_level;
	msm_batt_info.is_adapt_completed = smb346_charger_info->is_adapt_completed;	

	if (msm_batt_info.charger_type != charger_type) {
		msm_batt_info.charger_type = charger_type;
		
		if (charger_type == CHARGER_TYPE_USB_PC ||
		    charger_type == CHARGER_TYPE_USB_CARKIT) {
			msm_batt_info.current_chg_source = USB_CHG;
			supp = &msm_psy_usb;
		}
		else if (charger_type == CHARGER_TYPE_USB_WALL || 
			charger_type == CHARGER_TYPE_NON_STANDARD ||
			charger_type == CHARGER_TYPE_USB_HDMI ||
			charger_type == CHARGER_TYPE_ACCESSORY) {
			msm_batt_info.current_chg_source = AC_CHG;
			supp = &msm_psy_ac;
		}else {
			msm_batt_info.current_chg_source = 0;
			supp = &msm_psy_batt;
		}
	}
	else
		supp = NULL;

	if (msm_batt_info.charger_status != charger_status) {
		msm_batt_info.charger_status = charger_status;
		
		if (charger_status == CHARGER_STATUS_GOOD ||
		    charger_status == CHARGER_STATUS_WEAK) {
			if (msm_batt_info.current_chg_source) {
				if (msm_batt_info.current_chg_source & AC_CHG)
					supp = &msm_psy_ac;
				else
					supp = &msm_psy_usb;
			}
		} else {
			supp = &msm_psy_batt;
		}
	}

	if (not_charging) {
		if(poll_time) {
			msm_batt_info.batt_capacity = batt_capacity;
			//poll_time = 0;
		}	
		else {
			msm_batt_info.batt_capacity = vbat_min(batt_capacity, msm_batt_info.batt_capacity);
		}
	}
	else {
		if(poll_time) {
			msm_batt_info.batt_capacity = batt_capacity;
			//poll_time = 0;
		}		
		else if (batt_capacity > msm_batt_info.batt_capacity)
		{
			if (msm_batt_info.batt_capacity >= 6) {
				if (batt_filter_index%INCREASING_COUNT == 0) {
					msm_batt_info.batt_capacity += 1;
				}
			}
			else {
				if (batt_filter_index%LOWPOWER_INCREASING_COUNT == 0) {
					msm_batt_info.batt_capacity += 1;
				}				
			}
		}
	}
#if 0
	if (msm_batt_info.batt_capacity == 100 &&
		smb346_charger_info->battery_level != BATTERY_LEVEL_FULL
		&& smb346_charger_info->charger_type != CHARGER_TYPE_INVALID){
		msm_batt_info.batt_capacity = 99;
		ERR_BATT("%s:batt_capacity is 100%%,but charger complete detect is not finished yet\n", __func__);	
	}
#endif
	if (smb346_charger_info->battery_level == BATTERY_LEVEL_FULL){
		msm_batt_info.batt_capacity = 100;
	}
#if 1
	if (is_charger_just_connected)
		if (msm_batt_info.batt_capacity < batt_last_capacity)
			msm_batt_info.batt_capacity = batt_last_capacity;
	if (not_charging)
		if (msm_batt_info.batt_capacity > batt_last_capacity)
			msm_batt_info.batt_capacity = batt_last_capacity;
#endif

	DEBUG_BATT("BATT: voltage = %u mV [capacity = %d%%(%dmV), compute capacity = %d]\n",
		 battery_voltage, msm_batt_info.batt_capacity, battery_voltage_capacity, batt_capacity);
	
	if (!supp){
		supp = &msm_psy_batt;
	}

	if(supp){
		msm_batt_info.current_ps = supp;
		DEBUG_BATT("%s:BATT: Supply = %s\n", __func__,supp->name);
		power_supply_changed(supp);
	}
	
}


static int msm_batt_cleanup(void)
{
	int rc = 0;

	if (msm_batt_info.msm_psy_ac)
		power_supply_unregister(msm_batt_info.msm_psy_ac);
	if (msm_batt_info.msm_psy_usb)
		power_supply_unregister(msm_batt_info.msm_psy_usb);
	if (msm_batt_info.msm_psy_batt)
		power_supply_unregister(msm_batt_info.msm_psy_batt);

	return rc;
}

static struct smb346_battery_gauge smb346_batt_gauge = {
	.qury_adc_params = msm_batt_qury_adc_battery_params,
	.update_psy_status = msm_batt_update_psy_status,
};

void smb346_battery_fuelgauge_register(struct oppo_battery_fuelgauge *fuelgauge)
{
	batt_fuelgauge = fuelgauge;
}
EXPORT_SYMBOL(smb346_battery_fuelgauge_register);

void smb346_battery_fuelgauge_unregister(struct oppo_battery_fuelgauge *fuelgauge)
{
	batt_fuelgauge = NULL;
}
EXPORT_SYMBOL(smb346_battery_fuelgauge_unregister);

int smb346_vchg_mvolts_update(void)
{
	/*update battery info*/
	/*get vchg mvolts*/
	msm_batt_info.charger_voltage = get_prop_vchg_movlts(); 
	pr_info("%s: init charger voltage for power off charge,=%dmv\n",__func__,msm_batt_info.charger_voltage);
	
	/*update vchg mvolts bootime 2012-03-15*/
	if(msm_batt_info.msm_psy_batt){  
		power_supply_changed(msm_batt_info.msm_psy_batt);
	}else{
	    pr_err("%s: smb346_battery not inited!\n",__func__);
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL(smb346_vchg_mvolts_update);

int smb346_hsusb_enable(int on)
{
	if(smb346_batt.pdata->hsusb_enable != NULL)
	{
		if(on)
		    smb346_batt.pdata->hsusb_enable(1);
		else
			smb346_batt.pdata->hsusb_enable(0);
	}else{
		 pr_err("%s: hsusb_enable is NULL\n",__func__);
		 return -1;
	}
	return 0;
}
EXPORT_SYMBOL(smb346_hsusb_enable);

/*uart switch feature*/
int smb346_hsuart_enable(int on)
{
	if(smb346_batt.pdata->hsuart_enable != NULL)
	{
		if(on)
		    smb346_batt.pdata->hsuart_enable(1);
		else
			smb346_batt.pdata->hsuart_enable(0);
	}else{
		 pr_err("%s: hsuart_enable is NULL\n",__func__);
		 return -1;
	}
	return 0;
}
EXPORT_SYMBOL(smb346_hsuart_enable);

/*
    hsuart mode on val is 199 
    hsuart mode off val is 0
*/
//@/sys/devices/platform/smb346-battery/hsuart_switch
static ssize_t hsuart_switch_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size) {
	unsigned long val = simple_strtoul(buf, NULL, 10);
	
	if(val == 199){
		smb346_hsuart_enable(1);
	    pr_info("%s:hsuart switch on \n",__func__);
	}else{
		smb346_hsuart_enable(0);
		pr_info("%s:hsuart switch off \n",__func__);
	}
	
	return size;
}
static DEVICE_ATTR(hsuart_switch, 0644, NULL, hsuart_switch_store);
/*uart switch feature*/

static int __devinit msm_batt_probe(struct platform_device *pdev)
{
	int rc;
	
	pr_info("%s!!\n", __func__);

	if (pdev->id != -1) {
		dev_err(&pdev->dev,
			"%s: MSM chipsets Can only support one"
			" battery ", __func__);
		return -EINVAL;
	}

	smb346_batt.dev = &pdev->dev;

	if(pdev->dev.platform_data == NULL)
	{
	       pr_err("%s platform data is NULL\n", __func__);
		rc = -ENOMEM;
		return rc;
	}
	smb346_batt.pdata = (struct smb346_battery_platform_data *)pdev->dev.platform_data;

	rc = power_supply_register(&pdev->dev, &msm_psy_ac);
	if (rc < 0) {
		dev_err(&pdev->dev,
			"%s: power_supply_register failed"
			" rc = %d\n", __func__, rc);
		msm_batt_cleanup();
		return rc;
	}
	msm_batt_info.msm_psy_ac = &msm_psy_ac;
	
	rc = power_supply_register(NULL, &msm_psy_usb);
	if (rc < 0) {
		dev_err(&pdev->dev,
			"%s: power_supply_register failed"
			" rc = %d\n", __func__, rc);
		msm_batt_cleanup();
		return rc;
	}
	msm_batt_info.msm_psy_usb = &msm_psy_usb;

	if (!msm_batt_info.msm_psy_ac && !msm_batt_info.msm_psy_usb) {

		dev_err(&pdev->dev,
			"%s: No external Power supply(AC or USB)"
			"is avilable\n", __func__);
		msm_batt_cleanup();
		return -ENODEV;
	}

	msm_batt_info.voltage_max_design = smb346_batt.pdata->voltage_max_design;
	msm_batt_info.voltage_min_design = smb346_batt.pdata->voltage_min_design;
	msm_batt_info.batt_technology = smb346_batt.pdata->batt_technology;
	msm_batt_info.calculate_capacity = smb346_batt.pdata->calculate_capacity;

	if (!msm_batt_info.voltage_min_design)
		msm_batt_info.voltage_min_design = BATTERY_LOW;
	if (!msm_batt_info.voltage_max_design)
		msm_batt_info.voltage_max_design = BATTERY_HIGH;

	if (msm_batt_info.batt_technology == POWER_SUPPLY_TECHNOLOGY_UNKNOWN)
		msm_batt_info.batt_technology = POWER_SUPPLY_TECHNOLOGY_LION;

	if (!msm_batt_info.calculate_capacity)
		msm_batt_info.calculate_capacity = msm_batt_capacity;

	rc = power_supply_register(&pdev->dev, &msm_psy_batt);
	if (rc < 0) {
		dev_err(&pdev->dev, "%s: power_supply_register failed"
			" rc=%d\n", __func__, rc);
		msm_batt_cleanup();
		return rc;
	}
	msm_batt_info.msm_psy_batt = &msm_psy_batt;

      /*update battery info 2012-03-15*/
	if(msm_batt_info.msm_psy_batt)  
		power_supply_changed(msm_batt_info.msm_psy_batt);

       smb346_battery_gauge_register(&smb346_batt_gauge);

    /*uart switch feature*/
	if(smb346_batt.dev){
		/* creat sys file for uart switch */
		rc = device_create_file(smb346_batt.dev, &dev_attr_hsuart_switch);
		if (rc < 0) {
			pr_err("%s: creat dev_attr_hsuart_switch file failed ret = %d\n", __func__, rc);
			device_remove_file(smb346_batt.dev, &dev_attr_hsuart_switch);
		}
    }
	else
	{
	    pr_err("%s:dev is NULL\n",__func__);
    }
    /*uart switch feature*/

	if(smb346_batt.pdata->ldo_init != NULL)
	{
	        pr_info("%s: smb346 battery ldo18 init\n",__func__);
	        smb346_batt.pdata->ldo_init();
	}

	if(smb346_batt.pdata->ldo_power != NULL)
	{
		smb346_batt.pdata->ldo_power(1);
	}else{
	    pr_err("%s: ldo18 power is NULL\n",__func__);
	}

	//register irq for charger detect when bootime
	smb346_mpp11_irq_register();

	return 0;
}

static int __devexit msm_batt_remove(struct platform_device *pdev)
{
	int rc;
	
	rc = msm_batt_cleanup();
	if (rc < 0) {
		dev_err(&pdev->dev,
			"%s: msm_batt_cleanup  failed rc=%d\n", __func__, rc);
		return rc;
	}

	smb346_battery_gauge_unregister(&smb346_batt_gauge);
	return 0;
}
static int msm_batt_suspend(struct platform_device *pdev, pm_message_t state)
{
	int rc;

	//ERR_BATT("enter %s\n", __func__);
	
	if(smb346_batt.pdata->ldo_power != NULL)
	{
		smb346_batt.pdata->ldo_power(0);
	}else{
		pr_err("%s: ldo18 power is NULL\n",__func__);
	}
	
	/*add by chendx 2011-11-24 for battery low suspend shutdown */
	rc = pm8xxx_batt_alarm_disable(
				PM8XXX_BATT_ALARM_UPPER_COMPARATOR);
	if (!rc)
		rc = pm8xxx_batt_alarm_enable(
				PM8XXX_BATT_ALARM_LOWER_COMPARATOR);
	if (rc) {
		pr_err("%s: unable to set batt alarm state\n", __func__);
	}
	/*end by chendx 2011-11-24 for battery low suspend shutdown */   

	//ERR_BATT("exit %s\n", __func__);
	return 0;
}
static int msm_batt_resume(struct platform_device *pdev)
{
	int rc;
	
	ERR_BATT("enter %s\n", __func__);
	/*add by chendx 2011-11-24 for battery low suspend shutdown */
	rc = pm8xxx_batt_alarm_disable(
				PM8XXX_BATT_ALARM_UPPER_COMPARATOR);
	if (!rc)
		rc = pm8xxx_batt_alarm_disable(
				PM8XXX_BATT_ALARM_LOWER_COMPARATOR);
	if (rc){
		pr_err("%s: unable to set batt alarm state\n", __func__);
	}
	/*end by chendx 2011-11-24 for battery low suspend shutdown */  
	
	if(smb346_batt.pdata->ldo_power != NULL)
	{
		smb346_batt.pdata->ldo_power(1);
	}else{
	    pr_err("%s: ldo18 power is NULL\n",__func__);
	}

	ERR_BATT("exit %s\n", __func__);
	return 0;
}

/*
 *  4.0 I2c-qup driver resume is later, update qury_adc_params fail at msm_batt_resume.
 *  So that  update qury_adc_params when fulegague resume. 
*/
int updata_batt_params_fulegague_resume(void)
{
	u32 battery_voltage_now = 0;
	u32 battery_voltage_pre = 0;
	u32 fulegague_capacity = 0;
	struct smb346_battery_adc_params_type cur_battery_adc_params;

	smb346_batt_gauge.qury_adc_params(&cur_battery_adc_params);
	cur_battery_adc_params.current_batt_adc_voltage -= RESUME_COMPENSATE;
	battery_voltage_now = cur_battery_adc_params.current_batt_adc_voltage;
	battery_voltage_pre = batt_volt_history[(batt_filter_index + BATT_VOLT_FILTER_LENGTH -1)%BATT_VOLT_FILTER_LENGTH];
	DEBUG_BATT("battery_voltage_now = %dmv,battery_voltage_pre = %dmv\n", battery_voltage_now,battery_voltage_pre);
	pr_info("fulegague params:%d%%,vol=%dmv",cur_battery_adc_params.batt_capacity,cur_battery_adc_params.current_batt_adc_voltage);
	
	 /* notify that the voltage has changed
	 * the read of the capacity will trigger a
	 * voltage read when resume*/
	fulegague_capacity = cur_battery_adc_params.batt_capacity;
	if (fulegague_capacity >= 98) {
		fulegague_capacity = 100;
	}
	else {
		fulegague_capacity = 99*fulegague_capacity/97;
	}

	if(fulegague_capacity > fulegague_capacity_last)
		fulegague_capacity = fulegague_capacity_last;

	 msm_batt_info.fulegague_capacity = fulegague_capacity;
	 fulegague_capacity_last = msm_batt_info.fulegague_capacity;
	 power_supply_changed(&msm_psy_batt);

	 return 0;
}

EXPORT_SYMBOL(updata_batt_params_fulegague_resume);

static struct platform_driver msm_batt_driver = {
	.probe = msm_batt_probe,
	.remove = __devexit_p(msm_batt_remove),
	.suspend = msm_batt_suspend,
	.resume = msm_batt_resume,	
	.driver = {
		   .name = "smb346-battery",
		   .owner = THIS_MODULE,
		   },
};

static int __devinit msm_batt_init_rpc(void)
{
	int rc;
	
	rc = platform_driver_register(&msm_batt_driver);

	if (rc < 0)
		ERR_BATT("%s: FAIL: platform_driver_register. rc = %d\n",
		       __func__, rc);

	return rc;
}

static int __init msm_batt_init(void)
{
	int rc;

	pr_debug("%s: enter\n", __func__);

	rc = msm_batt_init_rpc();

	if (rc < 0) {
		ERR_BATT("%s: FAIL: msm_batt_init_rpc.  rc=%d\n", __func__, rc);
		msm_batt_cleanup();
		return rc;
	}

	return 0;
}

static void __exit msm_batt_exit(void)
{
	platform_driver_unregister(&msm_batt_driver);
}

module_init(msm_batt_init);
module_exit(msm_batt_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("shijiangwei <sjw@oppo.com>");
MODULE_DESCRIPTION("Battery driver for MSM8260 platform.");
MODULE_VERSION("1.0");
