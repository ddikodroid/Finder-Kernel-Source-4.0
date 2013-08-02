/*
 * Copyright (C) 2012-2013 OPPO, Inc.
 * Author: chendx <cdx@oppo.com>
 * accessory charger and wireless charger charge to main battery
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
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/mfd/pmic8058.h>
#include <linux/semaphore.h> 

#include <asm/system.h>
#include <asm/io.h>
#include <mach/msm_iomap.h>
#include <mach/scm-io.h>

#ifndef CONFIG_GPIOLIB
#include "gpio_chip.h"
#endif
#include <mach/board-msm8660.h>
#include <linux/smb346-charger.h>
#ifdef CONFIG_TOUCHSCREEN_ATMEL_QT602240E
#include <linux/qt602240_ts.h>
#endif

#define UPDATE_COULOMB_POLL_FEATURE

/*define return value*/
#define NO_ERROR			0
#define READ_ERROR			-1
#define ARGC_INVALID_ERROR	-2
#define NOREADY_ERROR		-3
#define INVALID_ERROR		-1
#define VALID_CAPACITY		0

#define READ_OPERATE 0x8F
#define WRITE_OPERATE 0x0F

#define ACCESSORY_CHG_EN   0x01
#define USB_CHG_INPUT_CURRENT   0x03<<1
#define ACCESSORY_BATTERY_TEMP   0x03<<3
#define SHUTDOWN_STATUS   0x01<<5
#define ACCESSORY_CHG_DC_EN   0x01<<6

#define RW_FLAG   0x01<<7

#define ACCESSORY_CHG_EN_MASK 0x01
#define ACCESSORY_CHG_DISABLE_MASK 0x00

#define USBIN_INPUT_SUPPORT_MASK 0x03<<1
#define USBIN_INPUT_SUPPORT_500MA 0x01<<1
#define USBIN_INPUT_SUPPORT_800MA 0x03<<1
#define USBIN_INPUT_SUPPORT_0MA 0x00<<1

#define ACCESSORY_TEMPERATE_MASK 0x03<<3
#define ACCESSORY_TEMPERATE_NORMAL 0x00<<3
#define ACCESSORY_TEMPERATE_HIGH 0x02<<3
#define ACCESSORY_TEMPERATE_LOW 0x01<<3

#define SHUTDOWN_STATUS_MASK 0x01<<5
#define SHUTDOWN_STATUS_SHUTDOWN 0x01<<5
#define SHUTDOWN_STATUS_NORMAL 0x00<<5

#define ACCESSORY_CHG_DC_EN_MASK   0x01<<6

#define ACCESSORY_CHG_DC_DISABLE_MASK   0x00<<6
#define ACCESSORY_CHG_DC_ENABLE_MASK   0x01<<6
#define ACCESSORY_CHG_CONNECT_MASK   0x01<<6
#define ACCESSORY_CHARGE_COMPELETE  0x1<<15
#define WIRELESS_PRESENT 0x1<<14
#define ACCESSORY_DC_ON 0x1<<13
#define ACCESSORY_CONNECT 0x1<<10
#define ACCESSORY_CHARGE_STATUS_MS 0x1<<11
#define ACCESSORY_CHARGE_STATUS_LS 0x1<<12
#define ACCESSORY_VBATT 0x003FF

#define PM8058_GPIO07	6  /*PMIC8058 GPIO number 07*/
#define ACCESSORY_WIRE_GPIO	PM8058_GPIO_PM_TO_SYS(PM8058_GPIO07)

#define RESPONSE	500		/*Response time, device to host RESPONSE*T_UNIT*/
#define ACCESSORY_TEST_DELAY 200000000 /*200ms*/
#define ACCESSORY_DET_TIME 2 
#define ACCESSORY_UPDATE_TIME 6
#define ACCESSORY_COULOMB_POLL ((HZ)*ACCESSORY_UPDATE_TIME)
#define ACCESSORY_COULOMB_POLL_INIT ((HZ)*5)//wangjc 10->5
#define USB_CHG_EVENT_TIME 2

#define SMB346_ACIL_CURRENT300MA 300 /*USBIN charge input current300mA*/
#define SMB346_ACIL_CURRENT500MA 500 /*USBIN charge input current500mA*/
#define SMB346_ACIL_CURRENT900MA 900 /*USBIN charge input current900mA*/

#define DC_DC_ON_MVLOTS 3620

/*define delay*/
#define CYCH		200		//Cycle time, host to device
#define CYCD		205		//Cycle time, device to host
#define HW1			150		//Host sends 1 to device
#define HW0			50		//Host sends 0 to device

#define ACCESSORY_BATTERY_COMPELETE_COUNTER 5
#define ACCESSORY_CHG_COMPELETE_MV 4150

#define ACCESSORY_MAX_EVENTS 16


struct hrtimer usb_chg_connect_timer;
struct hrtimer charge_compelete_timer;

struct hrtimer accessory_test_timer;

#ifdef UPDATE_COULOMB_POLL_FEATURE
static struct timer_list timer_hdq;
#endif

typedef enum{
	ACCESSORY_FAST_CHG_0MA,
	ACCESSORY_FAST_CHG_500MA,
	ACCESSORY_FAST_CHG_300MA,
	ACCESSORY_FAST_CHG_900MA,
}smb346_acil_result;

typedef enum{
	ACCESSORY__CHG_STATUS__INVALID,	
	ACCESSORY__CHG_STATUS__CHARGING,
	ACCESSORY__CHG_STATUS__DISCHARGING,
	ACCESSORY__CHG_STATUS__NOT_CHARGING,
	ACCESSORY__CHG_STATUS__CHARGING_COMPLETED
}charging_status_type;

struct pm8058_gpio_cfg {
	int        		gpio;
	struct pm_gpio cfg;
};

enum accessory_hardware_event {
	TX_ACCESSORY_DATA,
	RX_ACCESSORY_DATA,
};


struct accessory_event {
	enum accessory_hardware_event event;
};


struct accessory_battery{
	struct device *dev;
	bool batt_test_flag;
	int sn_irq;
    spinlock_t lock; /* protect access to irq read data from accessory */
	int accessory_flag;/*accessory battery is present*/
	int accessory_vbatt;
	int charge_compelete;
	int wireless_present;
	int accessory_dc_on;
	int accessory_data;
	int disconnected_counter;
	int mains_charge_allow;
	int mains_charge_compelete_counter;/*mains battery charge compelete counter*/
	int charging_compelete_counter;/*accessory battery charging compelete counter*/
	int stop_timer; /*start 5s timer to update accessory info only when accessory is connect*/
	u8 data_pre;
	charging_status_type charging_status;
	int chg_mode;
	int dc_mode;
	int main_capacity;/*record the main capacity when start accessory charging.*/
	int mains_capacity_pre;

	/* msm charger work queue thread relative value */
	struct accessory_event *queue;
	int tail;
	int head;
	spinlock_t queue_lock;
	int queue_count;
	struct work_struct queue_work;
	struct workqueue_struct *event_wq_thread;
};

static struct accessory_battery *accessory_batt = NULL;
static bool need_to_tx_again = false;

/*extern functions*/
extern bool get_usb_chg_status(void);
extern int get_main_battery_capacity(void);
extern chg_charger_hardware_type get_charger_type(void);
extern bool get_mains_battery_charge_status(void);
static int accessory_is_charge_complete(void);
int accessory_is_wirelees_present(void);
static int rx_data(u16 *data,bool *response);
static int tx_data(u8 data);
static int tx_host_data_to_device(void);

/* Debug levels */
#define base_level 0	//wangjc modify for del log.
#define ERROR_LEVEL     1
#define DEBUG_LEVEL     2

#define print_chg(level, ...) \
	do { \
		if (base_level  >= (level)) \
			printk(__VA_ARGS__); \
	} while (0) 

/* 
 * pm8058 gpio config routine.
 *
 * @param gpio: gpio control line
 * @param gpio_value: value set to gpio
 */
static int pmic8058_gpio__config(int gpio,int gpio_value,int direction)
{
    int ret=0;
	struct pm8058_gpio_cfg gpio_cfgs[] = 
	{
		{
			gpio,
			{
					.direction	= direction,
					.output_buffer	= PM_GPIO_OUT_BUF_CMOS,
					.output_value	= gpio_value,
					
					.pull		= PM_GPIO_PULL_UP_30,
					.vin_sel	= PM8058_GPIO_VIN_VPH,
					.out_strength	= PM_GPIO_STRENGTH_LOW,
					.function	= PM_GPIO_FUNC_NORMAL,
					
					.inv_int_pol	= 0,
					.disable_pin = 0,
			}
		}
   	};
	
	ret = pm8xxx_gpio_config(gpio_cfgs[0].gpio,
		&(gpio_cfgs[0].cfg));

	if(ret){
		print_chg(ERROR_LEVEL,"%s: pm8xxx_gpio_config failed!,gpio =%d\n",
			 __func__,gpio_cfgs[0].gpio);
		return ret;
	}

	return 0;
		
}

//1tick is 1us
static void tickDelay(int tick)
{
     udelay(tick);
}
static int outp(unsigned port, int databyte)
{
	 gpio_set_value_cansleep(port,databyte);
     return 0;	 
}
static int inp(unsigned port)
{
     int temp=0;
     temp = gpio_get_value_cansleep(port);
     return temp;
}

// Host Generate a reset singnal to device
//  ___200us_____
// |                   |___120us___|
static void WireTouchReset(void)
{
        //send reset singal to device
        tickDelay(0);//0us
        outp(ACCESSORY_WIRE_GPIO,0x01); // Drives DQ HIGH
        tickDelay(200);//200us
        outp(ACCESSORY_WIRE_GPIO,0x00); // Releases the bus
        tickDelay(120);//120us
}

/**
*@ Wire_WriteByte()
*-->LSB
*-->MSB
*/
static void Wire_WriteByte(u8 data)
{
	int bitMask = 0x01;
	int i=0;

	//write 8bit data
	for(i=0;i<8;i++)
	{
		if (data & bitMask){
			outp(ACCESSORY_WIRE_GPIO,0x01);
			tickDelay(HW1);	//about 150
			outp(ACCESSORY_WIRE_GPIO,0x00);
			tickDelay(CYCH - HW1);//50us
		}else{
			outp(ACCESSORY_WIRE_GPIO,0x01); 
			tickDelay(HW0);	//about 50
			outp(ACCESSORY_WIRE_GPIO,0x00); 
			tickDelay(CYCH - HW0);//about 150us
		}

		/*send next bit*/
		bitMask = bitMask << 1;
	}

}

/**
**  
**	bit 1's high level between 120 to 180:,low level between 20 to 80
           ______150us______  
*         |                         |__50us__|  
*
*	bit 0's low level between 120 t0 180:,high level between 20 to 80
                                      __50us___
*         |_____150us______|            |  

*/
#define HIGH_TIME_MAX 180
#define HIGH_TIME_MIN 120

#define BLOW_TIME_MIN 30
#define LOW_TIME_MAX 70

/**
*@ Wire_ReadByte()
*-->LSB
*-->MSB
*/
static bool Wire_ReadByte(u8 *pBuff)
{
	int bitMask = 0x01;
	u8 data = 0xff;
	int valDQ = -1;
	long precise_time = 0;
	struct timeval tv;
	int delayTime=RESPONSE;
	int i=0;
	
	for(i =0;i<8;i++)
	{
		/*get current precise time (unit: us) at the beginning of a bit*/
		do_gettimeofday(&tv); 
		precise_time = tv.tv_usec;
				
		udelay(75);           
		valDQ = inp(ACCESSORY_WIRE_GPIO);

		/*if read 0 set relevant bit in buffer*/
		if(valDQ == 0)         // read bit 0
		{
			data = data & (~bitMask);
			
			/*wait high levle or time out*/
			delayTime = RESPONSE;
		    while(!(inp(ACCESSORY_WIRE_GPIO)) && (--delayTime));
			if(delayTime){
				do_gettimeofday(&tv);
				if(tv.tv_usec - precise_time > 250)        //time out
				{
					goto readerror;
				}
			}
		}else{
			/*wait for falling edge, except the last bit which has no falling edge*/
			delayTime = RESPONSE;
			while((inp(ACCESSORY_WIRE_GPIO)) && (--delayTime));

			if(delayTime){
				do_gettimeofday(&tv);
				if(tv.tv_usec - precise_time > 200)    // time out 
				{
					goto readerror;
				}
			}

			/*wait high levle or time out*/
			delayTime = RESPONSE;
		    while(!(inp(ACCESSORY_WIRE_GPIO)) && (--delayTime));

			if(delayTime)
			{
				do_gettimeofday(&tv);
				if(tv.tv_usec - precise_time > 250)        //time out
				{
					goto readerror;
				}
			}
		}

		/*shift bitMask to read new bit*/
		bitMask = bitMask << 1;
	}//while(bitMask)

	*pBuff = data;
	return true;

readerror:
		return false;
}

/* 
* wire_waitforResponse routine.
*
* function:
*	host wait for device's response singnal.
* parameter:
*	null
* return:
*	true if responded,otherwise false
*    device send reset singnal to host
*                   ______400us______
*  |__150us__|                          |__120us_|
*/

static bool wire_waitforResponse(void)
{
	int delayTime = RESPONSE;
	
    tickDelay(300);//300us
	
	/*wait high levle or time out*/
	while(!(inp(ACCESSORY_WIRE_GPIO)) && (--delayTime));

	if(delayTime){
       /*wait low levle or time out*/
	   delayTime = RESPONSE;
	   while((inp(ACCESSORY_WIRE_GPIO)) && (--delayTime));
	   
	   if(delayTime){
		    /*wait high levle or time out*/
		    while(!(inp(ACCESSORY_WIRE_GPIO)) && (--delayTime));
			if(!delayTime){
				print_chg(ERROR_LEVEL,"%s: wait reset high level failed!\n", 
					   __func__);
				return false;
			}
				
	   	    return true; //ACK
	   	    
	   }else{
	      	print_chg(ERROR_LEVEL,"%s: wait low level failed,device not ready\n", 
					__func__);
	        return false;	//time out
	   }
    }else{	
		print_chg(ERROR_LEVEL,"%s: wait high level failed,device not ready\n", 
				__func__);
        return false;	// time out
    }
	
}

static int read_byte(u16 *data)
{
	u16 DATA=0;
	u16 VERIFY_DATA=0;
	u8 buffer=0;
	
	//read data LS BIT
	if(Wire_ReadByte(&buffer)){
	  DATA = buffer;
	}else{
	  goto err_out;
	}
		
	//read data MS BIT
    if(Wire_ReadByte(&buffer)){
	  DATA = (buffer << 8) + DATA;
	}else{
	  goto err_out;
	}
		
	//read verify data LS BIT
	if(Wire_ReadByte(&buffer)){
	  VERIFY_DATA = buffer;
	}else{
	  goto err_out;
	}
		
	//read verify data MS BIT
    if(Wire_ReadByte(&buffer)){
	  VERIFY_DATA = (buffer << 8) + VERIFY_DATA;
	}else{
	  goto err_out;
	}
		
	VERIFY_DATA = (~VERIFY_DATA) & 0xffff;
	if(DATA != VERIFY_DATA){
	   print_chg(ERROR_LEVEL,"%s: data and verify data verify failed!!\n",__func__);
	   goto err_out;
	}else{
	   *data = DATA;
	}

	return NO_ERROR;

err_out:  
    return READ_ERROR;
}

/* 
* wire_readData routine.
*
* function:
*	host read a electricity parameter from device
* parameter:
*	reg --- device register value
*     data ---- read data from device
* return:
*	return a integer 
* note:
*/
/*The 8 bit is 1*/
static int wire_readData(u8 reg_op,u16 *data,bool *response)
{
	int ret;
	bool isReady = false;
	u16 dat=0;

	unsigned long flags;

	spin_lock_irqsave(&accessory_batt->lock, flags);
	
    /*config ACCESSORY_WIRE_GPIO to output before write data to device */
    ret = pmic8058_gpio__config(ACCESSORY_WIRE_GPIO,0,PM_GPIO_DIR_OUT);
	if(ret){
		print_chg(ERROR_LEVEL,"%s: config ACCESSORY_WIRE_GPIO DIR OUT  failed!\n",
				__func__);
		goto err_out;
	}
		
	//reset device
    WireTouchReset(); // Reset the 1-Wire bus

    //send register data
	Wire_WriteByte(reg_op);
	//send verify register data (~reg)
	reg_op = ~reg_op;
	Wire_WriteByte(reg_op);

    /*config ACCESSORY_WIRE_GPIO to input before read data to device */
    ret = pmic8058_gpio__config(ACCESSORY_WIRE_GPIO,0,PM_GPIO_DIR_IN);
	if(ret){
		print_chg(ERROR_LEVEL,"%s: config ACCESSORY_WIRE_GPIO DIR IN  failed!\n",
				__func__);
		goto err_out;
	}
	
	//wait for response
	isReady = false;
	isReady = wire_waitforResponse();
	*response = isReady;

	//receive data form device and verify the data
	if (isReady)
	{
		ret = read_byte(&dat);
		if(ret == NO_ERROR)
			*data = dat;
		else
			goto err_out;
	}else{
		print_chg(ERROR_LEVEL,"%s: device no response,try again!\n", __func__);
		goto err_out;
	}

	spin_unlock_irqrestore(&accessory_batt->lock, flags);

    return NO_ERROR;
	
err_out:  
	spin_unlock_irqrestore(&accessory_batt->lock, flags);
    return READ_ERROR;
}

/* 
* wire_writeData routine.
*
* function:
*	host write a electricity parameter to device
* parameter:
*	reg --- device register value
*     data ---- write data from device
* return:
*	return a integer 
* note:
*/
/*The 8 bit is 0*/
static int wire_writeData(u8 data)
{
    int ret;
	bool isReady = false;
	u16 dat=0;
	unsigned long flags;

	spin_lock_irqsave(&accessory_batt->lock, flags);
	
	data &=0x7f; 
	//disable_irq_nosync( accessory_batt->sn_irq);
    /*config ACCESSORY_WIRE_GPIO to output before write data to device */
    ret = pmic8058_gpio__config(ACCESSORY_WIRE_GPIO,0,PM_GPIO_DIR_OUT);
	if(ret){
		print_chg(ERROR_LEVEL,"%s: config ACCESSORY_WIRE_GPIO DIR OUT  failed!\n",__func__);
		goto err_out;
	}
	/*Reset the 1-Wire bus*/
    WireTouchReset(); 

    /*send register data*/
	Wire_WriteByte(data);
	/*send verify register data (~reg)*/
	data = ~data;
	Wire_WriteByte(data);

    /*config ACCESSORY_WIRE_GPIO to input before read data to device */
    ret = pmic8058_gpio__config(ACCESSORY_WIRE_GPIO,0,PM_GPIO_DIR_IN);
	if(ret){
		print_chg(ERROR_LEVEL,"%s: config ACCESSORY_WIRE_GPIO DIR IN  failed!\n",__func__);
		goto err_out;
	}
	
	/*wait for response*/
	isReady = false;
	isReady = wire_waitforResponse();
	if (isReady)
	{
		ret = read_byte(&dat);
		if(ret == READ_ERROR)
			goto err_out;
	}else{
		print_chg(ERROR_LEVEL,"%s: device no response,try again!\n", __func__);
		goto err_out;
	}
	//enable_irq(accessory_batt->sn_irq);
	spin_unlock_irqrestore(&accessory_batt->lock, flags);
    return NO_ERROR;
	
err_out:  
	spin_unlock_irqrestore(&accessory_batt->lock, flags);
 	//enable_irq(accessory_batt->sn_irq);
    return READ_ERROR;
    
}

/**
*host receive 16bits from device
*@data:read data from device
*@response:device response status
*@Bit1-10 : accessory battery mvolts
*@Bit11 : accessory connect flag 1=accessory connect
*@Bit12-13: not use
*@Bit14:accessory DC-DC status flag,1=DC-DC on,0=DC-DC off
*@Bit15:wireless charge status flag,1=wireless charger connect,0=wireless charger disconnect
*@Bit16:accessory charge status flag 1=accessory battery charge ok 0=accessory battery is
*          charging
*/
static int rx_data(u16 *data,bool *response)
{
	int ret=0;
	
	ret = wire_readData(READ_OPERATE,data,response);
	if(ret){
		print_chg(ERROR_LEVEL,"%s: read data failed!\n",__func__);
		return -1;
	}
	
    print_chg(DEBUG_LEVEL,"%s: receive data sucessfully,data =0x%0x\n",__func__,*data);
	return 0;
}

/**
*host transmit 8bits to device
*@data:write data to device
*@Bit1: accessory battery charge en flag,1=charge enable,0=charge disable
*@Bit2-3:USB charge input current,11=800mA,10=500mA,01=100mA,00=0mA
*@Bit4-5:temperate status,01=high temperate,10=low temperate,00=normal temperate
*@Bit6: host status flag 1=shutdown,0=normal
*@Bit7:DC-DC enable,1= DC-DC enable ON,0=DC-DC off
*@Bit8:RW flag,1=write,0=read
*/
static int tx_data(u8 data)
{
	int ret=0;
	
	ret = wire_writeData(data);
	if(ret){
		print_chg(ERROR_LEVEL,"%s: write data failed!\n",__func__);
		return -1;
	}
	
    print_chg(DEBUG_LEVEL,"%s: transmit data sucessfully,data =0x%0x\n",__func__,data);
	return 0;
}

/**
*  rx_data_update()
*  rx_data-data
*  val :mask bit value of data
*  mask:
*/
static u8 rx_data_update( u8 rx_data,u8 val, u8 mask)
{	
	rx_data = (rx_data & ~mask) | val;
    return rx_data;
}

/*conver accessory battery voltage from adc to mvolts*/
static int adc_vol_conver(int adc_value)
{
    int vbatt=0;
    /*conver vbatt adc to voltage*/
	vbatt = adc_value*2048*3/1024;	
	return vbatt;
}



/*Accessory Battery fliter Begin*/

typedef struct
{
   s32 x;
   s32 y;
} VoltagetoCapacitytype;

/*Accessory battery VoltageTocapacity*/
static const VoltagetoCapacitytype v2c_map[] =
{
	{ 3500,      0 },	 //@0
	{ 3534, 	 1 },	 //@1--red
	{ 3565, 	 7 },	 //@2
	{ 3584,    12 }, //@3
	{ 3600,    16 }, //@4--orange
	{ 3609,    20 }, //@5
	{ 3620,    24 }, //@6
	{ 3633,    28 }, //@7
	{ 3650,    35 }, //@8--green
	{ 3671,    42 }, //@9
	{ 3699,    49 }, //@10
	{ 3725,    56 }, //@11
	{ 3744,    60 }, //@12
	{ 3769,    64 }, //@13
	{ 3791,    68 }, //@14
	{ 3813,    72 }, //@15
	{ 3840,    75 }, //@16
	{ 3852,    77 }, //@17
	{ 3872,    80 }, //@18
	{ 3886,    82 }, //@19
	{ 3910,    85 }, //@20
	{ 3934,    87 }, //@21
	{ 3960,    90 }, //@22
	{ 3979,    92 }, //@23
	{ 3996,    94 }, //@24
	{ 4019,    96 }, //@25
	{ 4100,   100 }	 //FULL

};

#define ACC_BATT_VOLT_FILTER_LENGTH	 10
static u32 acc_batt_filter_index = 0;
static u32 acc_batt_volt_history[ACC_BATT_VOLT_FILTER_LENGTH] = {0, 0, 0};
#define acc_batt_abs(x, y) ((x < y) ? y-x : x-y)
#define ABS_MVOLTS 200
#define ACC_VBATT_MVOLTS_LOW 3600

static u32 acc_vol_filter_avg(void)
{
	u32 count = 0;
	u32 sum = 0;
	u32 index = 0;
	u32 vol_base = 0;

	for(index = 0; index < ACC_BATT_VOLT_FILTER_LENGTH; index++)
	{
		if(acc_batt_volt_history[index] == 0)
			continue;		
		sum += acc_batt_volt_history[index];
		count++;
	}
	vol_base = sum / count;	

	return vol_base;
}

static int acc_volt_filter_clean(void)
{
	memset(acc_batt_volt_history, 0, ACC_BATT_VOLT_FILTER_LENGTH * sizeof(u32));
	acc_batt_filter_index = 0;

	return 0;
}

static u32 acc_batt_volt_filter(u32 batt_volt_now, bool not_charging)
{
	u32 vol = 0;
	u32 batt_volt_capa;
	u32 batt_volt_pre;

	batt_volt_pre = acc_batt_volt_history[(acc_batt_filter_index + ACC_BATT_VOLT_FILTER_LENGTH -1)%ACC_BATT_VOLT_FILTER_LENGTH];	
	if(batt_volt_pre && accessory_batt->accessory_vbatt >= ACC_VBATT_MVOLTS_LOW) {
		if(batt_volt_now < batt_volt_pre)
			vol = batt_volt_pre -1;
		else
			vol = batt_volt_pre +1;
	}
	else {
		vol = batt_volt_now;
	}
			
	acc_batt_filter_index %= ACC_BATT_VOLT_FILTER_LENGTH;
	acc_batt_volt_history[acc_batt_filter_index++] = vol;
	batt_volt_capa = acc_vol_filter_avg();

	printk("batt_filter_index_pre = %d,%dmv,%dmv,%dmv\n", 
						acc_batt_filter_index-1,batt_volt_capa,batt_volt_now,batt_volt_pre);
	
	return batt_volt_capa;
}

static int map_vol_to_capacity(const VoltagetoCapacitytype *paPts,
	         u32 nTableSize, s32 input, s32 *output)
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

static u32 accessory_batt_capacity(u32 current_voltage)
{
	u32 capacity;
	int ret;

	ret = map_vol_to_capacity(v2c_map, 
							sizeof(v2c_map)/sizeof(v2c_map[0]), 
							current_voltage,
							&capacity);
	if(ret)
		return ret;

	return capacity;
}
/*Accessory Battery fliter Feature end*/

charging_status_type accessory_charging_status_get(void)
{
	if(accessory_batt)
		return accessory_batt->charging_status;
	else
		return ACCESSORY__CHG_STATUS__INVALID;
}
EXPORT_SYMBOL(accessory_charging_status_get);

static void accessory_charging_status_set(struct accessory_battery *acc_batt, charging_status_type status)
{
	acc_batt->charging_status = status;	
}

static void __dump_accessory_log(void)
{
	char *batt_status_string[] = {"Invalid","charging", "discharging",
		"not charging","charge compelete"};

    print_chg(DEBUG_LEVEL,"========__dump_accessory_log=====\n");
    print_chg(DEBUG_LEVEL,"%s:charge compelete=%d,wireless_present =%d\n",
		__func__,accessory_batt->charge_compelete,accessory_batt->wireless_present);
	
	print_chg(DEBUG_LEVEL,"%s:accessory dc_on=%d,vbatt=%dmv\n",
		__func__,accessory_batt->accessory_dc_on,accessory_batt->accessory_vbatt);

	print_chg(DEBUG_LEVEL,"%s:accessory_batt->chg_mode=%d,dc_mode=%d\n",
		__func__,accessory_batt->chg_mode,accessory_batt->dc_mode);

	print_chg(DEBUG_LEVEL,"Accessory BATT charge status = %s,connect status =%d\n", 
		batt_status_string[accessory_batt->charging_status],accessory_batt->accessory_flag);
}

static int wireless_charger_connect(u16 rx_data)
{
	if(rx_data & WIRELESS_PRESENT)
	    accessory_batt->wireless_present = 1 ; 
	else
		accessory_batt->wireless_present = 0 ; 

    return 0;
}

static int accessory_status_update(u16 rx_data)
{
    int ret=0;
	int adc_value=0;
	u32 vbatt=0;

	if(rx_data == 0)
		return -1;
																					
	if(rx_data & ACCESSORY_CHARGE_COMPELETE)
		accessory_batt->charge_compelete =1 ; 
	else
		accessory_batt->charge_compelete =0 ; 

	wireless_charger_connect(rx_data);
	
	if(rx_data & ACCESSORY_DC_ON)
	    accessory_batt->accessory_dc_on=1 ; 
	else
		accessory_batt->accessory_dc_on=0 ; 

	/*check accessory charge status*/
	if((rx_data & ACCESSORY_CHARGE_STATUS_MS) && (rx_data & ACCESSORY_CHARGE_STATUS_LS))
		accessory_charging_status_set(accessory_batt, ACCESSORY__CHG_STATUS__CHARGING);
	else if(rx_data & ACCESSORY_CHARGE_COMPELETE)
		accessory_charging_status_set(accessory_batt, ACCESSORY__CHG_STATUS__CHARGING_COMPLETED);
	else
		accessory_charging_status_set(accessory_batt, ACCESSORY__CHG_STATUS__NOT_CHARGING);

	if(accessory_batt->accessory_flag){
		/*Accessory Batt present ,change vbatt ADC to voltage*/
		adc_value = rx_data & ACCESSORY_VBATT;
		vbatt = adc_vol_conver(adc_value);
		if(accessory_charging_status_get() == ACCESSORY__CHG_STATUS__CHARGING)
			accessory_batt->accessory_vbatt = (int)acc_batt_volt_filter(vbatt,false);
		else
			accessory_batt->accessory_vbatt = (int)acc_batt_volt_filter(vbatt,true);
		
	    __dump_accessory_log();
	}
	
	return ret;
}

/*accessory charge to mains,battery capacity >=80,charge compelete*/
#define ACCESSORY_COMPELETE_COUNTER 5
static void accessory_charge_to_mains_compelete_handle(u16 rx_data)
{
    int mains_capacity=0;

	if(!accessory_batt)
		return;
	
    mains_capacity = get_main_battery_capacity();
	if(accessory_batt->mains_capacity_pre >= ACCESSORY_RECHG_LEVEL && 
		mains_capacity < ACCESSORY_RECHG_LEVEL) {
		if(!accessory_batt->mains_charge_allow)
			accessory_batt->mains_charge_allow = 1;
		
		need_to_tx_again = true;
	}

	accessory_batt->mains_capacity_pre = mains_capacity;
	
    print_chg(DEBUG_LEVEL,"%s: Mains battery capacity =[%d%%],%d\n",
		__func__,mains_capacity,accessory_batt->mains_charge_allow);

}

static int mains_battery_charge_allow(void)
{
   print_chg(DEBUG_LEVEL,"%s:accessory_batt->mains_charge_allow =%d\n",
   	         __func__,accessory_batt->mains_charge_allow);
   return accessory_batt->mains_charge_allow;
}

int set_origin_main_capacity(void)
{
	if(accessory_batt)
		accessory_batt->main_capacity = get_main_battery_capacity();
	
	return 0;
}

static int get_accessory_chg_mode(void)
{
    return accessory_batt->chg_mode;
}
static int get_accessory_dc_mode(void)
{
    return accessory_batt->dc_mode;
}

static bool is_smart_mode_capable_of_charging(void)
{
	if(get_accessory_chg_mode()== SMART_CHARGE_MODE) {
		if(get_main_battery_capacity() < ACCESSORY_RECHG_LEVEL) {
			return true;
		}
		else if(accessory_batt->main_capacity < ACCESSORY_RECHG_LEVEL) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

static bool is_acc_chg_status_capable_of_charging(void)
{
	pr_info("%s charge allow:%d, chg_mode:%d\n", __func__, mains_battery_charge_allow(),
		get_accessory_chg_mode());
   if(mains_battery_charge_allow()&&
   		(is_smart_mode_capable_of_charging() ||
				(get_accessory_chg_mode()== SMART_CHARGE_MODE_INVALID &&
				get_accessory_dc_mode()== DC_DC_ON_MODE)))
   	  return true;
   else
   	  return false;
}

/*accessory battery charge compelete*/
static void accessory_batt_charge_compelete_handle(u16 rx_data)
{
	if(!accessory_batt)
		return;
	
   if(accessory_charging_status_get() == ACCESSORY__CHG_STATUS__CHARGING){
	   if(accessory_batt->accessory_vbatt >= ACCESSORY_CHG_COMPELETE_MV){
	   	   accessory_batt->charging_compelete_counter++;
	   }else{
	   	   accessory_batt->charging_compelete_counter=0;
	   }
	   
	   if(accessory_batt->charging_compelete_counter >= 
	   							ACCESSORY_BATTERY_COMPELETE_COUNTER)
	   {
	   	   /**
	   	     * continute 5times check accessory vbatt high than ACCESSORY_CHG_COMPELETE_MV and
	   	     * get charge compelete bit
		     */
		   print_chg(DEBUG_LEVEL,"Accessory Battery charge done@@@@\n");
	       accessory_charging_status_set(accessory_batt, ACCESSORY__CHG_STATUS__CHARGING_COMPLETED);   	
	   }
   }  
}

static int accessory_is_charge_complete(void)
{
    return accessory_batt->charge_compelete;
}

int accessory_is_wirelees_present(void)
{
	if(accessory_batt)
    	return accessory_batt->wireless_present;
	else
		return 0;
}

int get_accessory_capacity(void)
{
	if(accessory_batt)
	    return accessory_batt_capacity(accessory_batt->accessory_vbatt);
	return 0;
}
EXPORT_SYMBOL(get_accessory_capacity);

int get_accessory_present_status(void)
{
	if(accessory_batt)
	    return accessory_batt->accessory_flag;
	else
		return 0;
}
EXPORT_SYMBOL(get_accessory_present_status);

int get_accessory_charge_status(void)
{
	if(accessory_batt)
	    return accessory_batt->charging_status;
	else
		return ACCESSORY__CHG_STATUS__INVALID;
}
EXPORT_SYMBOL(get_accessory_charge_status);

int usb_charger_connect(int on)
{
	if(!accessory_batt)
		return 0;
	
	if(accessory_batt->accessory_flag){
		/*200 to handle usb charger remove and plugin status*/
		hrtimer_start(&usb_chg_connect_timer, 
		               ktime_set(USB_CHG_EVENT_TIME,0), 
		               HRTIMER_MODE_REL);	
	}
	return 0;
}
EXPORT_SYMBOL(usb_charger_connect);


/**
* @usb_chg_charging_complete()
* Mains battery charge compelete delay func
*/

int usb_chg_charging_complete(void)
{
	if(!accessory_batt)
		return 0;
	
	if(accessory_batt->accessory_flag){
		/*200 to handle usb charger remove and plugin status*/
		hrtimer_start(&charge_compelete_timer, 
		               ktime_set(USB_CHG_EVENT_TIME,0), 
		               HRTIMER_MODE_REL);	
	}
	return 0;
}
EXPORT_SYMBOL(usb_chg_charging_complete);



static int is_dc_on_mode(void)
{
	pr_info("%s chg_mode:%d, dc_mode:%d\n", __func__, accessory_batt->chg_mode, accessory_batt->dc_mode);
	if(accessory_batt->chg_mode == SMART_CHARGE_MODE ||
		(accessory_batt->chg_mode == SMART_CHARGE_MODE_INVALID &&
			accessory_batt->dc_mode == DC_DC_ON_MODE))
		return 1;
	else
		return 0;

}

static int tx_host_data_to_device(void)
{
	u8 data=0x00;
	bool usb_status = false;
	chg_charger_hardware_type charger_type = CHARGER_TYPE_INVALID ;
	int ret=0;
	
	usb_status = get_usb_chg_status();
	
	/*update accessory charge en (bit1)*/
	if(usb_status){
		if(accessory_batt->wireless_present){
			if(!accessory_is_charge_complete() || accessory_batt->accessory_vbatt < 4000)
			    data = rx_data_update(data,
									ACCESSORY_CHG_EN_MASK,
									ACCESSORY_CHG_EN_MASK);
		}else if(get_mains_battery_charge_status()){
			if(!accessory_is_charge_complete() || accessory_batt->accessory_vbatt < 4000)
				data = rx_data_update(data,
									ACCESSORY_CHG_EN_MASK,
									ACCESSORY_CHG_EN_MASK);
		}else{
			data = rx_data_update(data,
								ACCESSORY_CHG_DISABLE_MASK,
								ACCESSORY_CHG_EN_MASK);
		}
		
	}else{
		if(accessory_batt->wireless_present) {
			charger_type = get_charger_type();
			if(charger_type == CHARGER_TYPE_INVALID || charger_type == CHARGER_TYPE_NONE) {
				pr_info("%s type invalid\n", __func__);
				if(!accessory_is_charge_complete() || accessory_batt->accessory_vbatt < 4000) {
					pr_info("%s start wireless charging when type invalid\n", __func__);
					data = rx_data_update(data,
										ACCESSORY_CHG_EN_MASK,
										ACCESSORY_CHG_EN_MASK);
				}
			}
			else if(get_mains_battery_charge_status()) {
				if(!accessory_is_charge_complete() || accessory_batt->accessory_vbatt < 4000)
					data = rx_data_update(data,
										ACCESSORY_CHG_EN_MASK,
										ACCESSORY_CHG_EN_MASK);
			}
			else {
				data = rx_data_update(data,
								ACCESSORY_CHG_DISABLE_MASK,
								ACCESSORY_CHG_EN_MASK);
			}
		}
		else {
			data = rx_data_update(data,
								ACCESSORY_CHG_DISABLE_MASK,
								ACCESSORY_CHG_EN_MASK);
		}
	}
	
	if(data&ACCESSORY_CHG_EN_MASK)
	{
	    /*charge enable ,but not charge now*/
		if(get_accessory_charge_status() != ACCESSORY__CHG_STATUS__CHARGING)
			  accessory_batt->data_pre = 0xff;
	}else{
	    if(get_accessory_charge_status() == ACCESSORY__CHG_STATUS__CHARGING)
			  accessory_batt->data_pre = 0xff;
	}
	
	charger_type = get_charger_type();
	pr_info("%s charger_type:%d\n", __func__, charger_type);
	/*update input current limit (bit2,bit3)*/
	if(charger_type == CHARGER_TYPE_USB_PC ||
		charger_type == CHARGER_TYPE_NON_STANDARD ||
		charger_type == CHARGER_TYPE_USB_HDMI){
		data = rx_data_update(data,
							USBIN_INPUT_SUPPORT_500MA,
							USBIN_INPUT_SUPPORT_MASK);
	}else if(charger_type == CHARGER_TYPE_USB_WALL){
		data = rx_data_update(data,
							USBIN_INPUT_SUPPORT_800MA,
							USBIN_INPUT_SUPPORT_MASK);

	}else{
		data = rx_data_update(data,
							USBIN_INPUT_SUPPORT_0MA,
							USBIN_INPUT_SUPPORT_MASK);
	}
	
	/*update temperate (bit4,bit5)*/
	data = rx_data_update(data,
						ACCESSORY_TEMPERATE_NORMAL,
						ACCESSORY_TEMPERATE_MASK);
	
	/*update shutdown status (bit6)*/
	data = rx_data_update(data,
							SHUTDOWN_STATUS_NORMAL,
							SHUTDOWN_STATUS_MASK);
	
	/*update accessory charge DC-DC on (bit7)*/
	if(is_dc_on_mode()){
		if(usb_status){
			 data = rx_data_update(data,
			 						ACCESSORY_CHG_DC_DISABLE_MASK,
			 						ACCESSORY_CHG_DC_EN_MASK);
		}else{
			if(!accessory_batt->wireless_present){
				if(is_acc_chg_status_capable_of_charging())
			    	data = rx_data_update(data,
			    	                      ACCESSORY_CHG_DC_ENABLE_MASK,
			    	                      ACCESSORY_CHG_DC_EN_MASK);
				else
					data = rx_data_update(data,
											ACCESSORY_CHG_DC_DISABLE_MASK,
											ACCESSORY_CHG_DC_EN_MASK);
					
			}else{
				data = rx_data_update(data,
									ACCESSORY_CHG_DC_DISABLE_MASK,
									ACCESSORY_CHG_DC_EN_MASK);
			}
	    }
	}else{
		data = rx_data_update(data,
				 		ACCESSORY_CHG_DC_DISABLE_MASK,
				 		ACCESSORY_CHG_DC_EN_MASK);
	}

	ret = tx_data(data);
	if(ret)
		need_to_tx_again = true;
	else
		need_to_tx_again = false;

		
	print_chg(DEBUG_LEVEL,"%s: transmit data=0x%0x,data_pre=0x%0x\n",
				__func__,data,accessory_batt->data_pre);
	
	return 0;

}

#define ACCESSORY_BATT_CONNECT_COUNTER 3
#define ACCESSORY_BATT_UPDATA_DATA_TIME 5


static void accessory_update(void)
{
	int ret=0;
	int i=0;
	u16 accessory_data=0x00;
	bool response=true;
	/*updata get accessory data ACCESSORY_UPDATE_TIME*5 */
	static int updata_counter=0;
	int wireless_present;

	ret = rx_data(&accessory_data,&response);
	if(!response && ret){
			if(accessory_batt->accessory_flag){
			 /**
		        *  Accessory battery disConnect @
		        */
				for(i=0;i<3;i++){
					ret = rx_data(&accessory_data,&response);
					if(!ret || response){
						/*transmit to device successfully*/
						break;
					}
					
					accessory_batt->disconnected_counter ++;	
					if(accessory_batt->disconnected_counter >= ACCESSORY_BATT_CONNECT_COUNTER)
					{
					     accessory_batt->accessory_flag = 0;
						 accessory_batt->disconnected_counter = 0;
						 print_chg(DEBUG_LEVEL,"%s: accessory is remove!!!\n",__func__);
						 /*Clean accessory battery voltage filter*/
						 acc_volt_filter_clean();
					}
					mdelay(10);
				}
				
			}
	}else{
		updata_counter++;
		updata_counter = updata_counter%ACCESSORY_BATT_UPDATA_DATA_TIME;
	
        print_chg(DEBUG_LEVEL,"%s: Rx data successfully present=%d,=0x%0x,%d\n",
		__func__,accessory_batt->accessory_flag,accessory_data,accessory_batt->disconnected_counter);
	
	    accessory_batt->disconnected_counter=0;
		if(!accessory_batt->accessory_flag)
		{
		    /**
		        *  Accessory battery Connect @
		        */
			accessory_batt->accessory_flag = 1;
			/*send host data to device*/
			tx_host_data_to_device();
			print_chg(DEBUG_LEVEL,"%s: Accessory Batt CONNECTED#\n",__func__);

			/*Clean accessory battery voltage filter*/
			acc_volt_filter_clean();
		}
		
	}

	wireless_present = accessory_batt->wireless_present;
	
	/*update accessory data ACCESSORY_UPDATE_TIME */
	accessory_status_update(accessory_data);

	if(wireless_present != accessory_batt->wireless_present) {
		need_to_tx_again = true;
	}
	
	if(!ret && updata_counter==0){
		/*check mains battery is charge OK*/
		accessory_charge_to_mains_compelete_handle(accessory_data);

		/*check accessory battery is charge ok*/
		accessory_batt_charge_compelete_handle(accessory_data);
	}

	if(!ret && need_to_tx_again)
		tx_host_data_to_device();
}


static void handle_event(struct accessory_battery *accessory_batt, int event)
{
	switch (event) {
		case TX_ACCESSORY_DATA:
		{
			tx_host_data_to_device();
			break;
		}
		case RX_ACCESSORY_DATA:
		{
			accessory_update();
			break;
		}
	}
}

static int accessory_dequeue_event(struct accessory_battery *accessory_batt, struct accessory_event **event)
{
	unsigned long flags;

	spin_lock_irqsave(&accessory_batt->queue_lock, flags);
	if (accessory_batt->queue_count == 0) {
		spin_unlock_irqrestore(&accessory_batt->queue_lock, flags);
		return -EINVAL;
	}
	//pr_info("%s dequeueing %d,queue count =%d\n", __func__, (*event)->event,accessory_batt->queue_count);
	*event = &accessory_batt->queue[accessory_batt->head];
	accessory_batt->head = (accessory_batt->head + 1) % ACCESSORY_MAX_EVENTS;

	accessory_batt->queue_count--;
	spin_unlock_irqrestore(&accessory_batt->queue_lock, flags);
	return 0;
}

static int accessory_enqueue_event(struct accessory_battery *accessory_batt, enum accessory_hardware_event event)
{
	unsigned long flags;

	spin_lock_irqsave(&accessory_batt->queue_lock, flags);
	if (accessory_batt->queue_count == ACCESSORY_MAX_EVENTS) {
		spin_unlock_irqrestore(&accessory_batt->queue_lock, flags);
		return -EAGAIN;
	}
	
       //pr_info("%s queueing %d,queue count =%d\n", __func__, event,accessory_batt->queue_count);
	   
	accessory_batt->queue[accessory_batt->tail].event = event;
	accessory_batt->tail = (accessory_batt->tail + 1)%ACCESSORY_MAX_EVENTS;
	accessory_batt->queue_count++;
	spin_unlock_irqrestore(&accessory_batt->queue_lock, flags);
	return 0;
}

static void process_events(struct work_struct *work)
{
	struct accessory_battery *accessory_batt = container_of(work, struct accessory_battery, queue_work);	
	struct accessory_event *event;
	int rc;

	do {
		rc = accessory_dequeue_event(accessory_batt, &event);
		if (!rc)
			handle_event(accessory_batt, event->event);
	} while (!rc);
}

static int accessory_notify_event(struct accessory_battery *accessory_batt, enum accessory_hardware_event event)
{
	accessory_enqueue_event(accessory_batt, event);
	queue_work(accessory_batt->event_wq_thread, &accessory_batt->queue_work);
	return 0;
}

static enum hrtimer_restart usb_chg_event_delay__func(struct hrtimer *timer)
{
	accessory_notify_event(accessory_batt, TX_ACCESSORY_DATA);
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart charge_compelete_delay__func(struct hrtimer *timer)
{
	//wangjc
	accessory_batt->mains_charge_allow = 0;
	
	accessory_notify_event(accessory_batt, TX_ACCESSORY_DATA);
	return HRTIMER_NORESTART;
}

static irqreturn_t sn_irq( int irq, void *dev_id )
{
	return IRQ_HANDLED;
}

#ifdef UPDATE_COULOMB_POLL_FEATURE
static void accessory_update_timer__func(unsigned long data)
{
	accessory_notify_event(accessory_batt, RX_ACCESSORY_DATA);
	mod_timer(&timer_hdq, jiffies + ACCESSORY_COULOMB_POLL);
}
#endif

static enum hrtimer_restart accessory_test_delay__func(struct hrtimer *timer)
{
	static int ftm_counter=0;

	print_chg(DEBUG_LEVEL,"%s:\n",__func__);

	ftm_counter%=2;

	/*send 1Hz pwm to led test accessory battery on DQ line*/
	if(accessory_batt->batt_test_flag){
		print_chg(DEBUG_LEVEL,"%s: send 1Hz PWM ftm_counter =%d\n",
				__func__,ftm_counter);
		
		if(ftm_counter)
			outp(ACCESSORY_WIRE_GPIO,0x01); /*Drives DQ high*/
		else
			outp(ACCESSORY_WIRE_GPIO,0x00); /*Drives DQ low*/
		ftm_counter++;	
		
		hrtimer_start(&accessory_test_timer, 
			 		   ktime_set(0,ACCESSORY_TEST_DELAY), 
			 		   HRTIMER_MODE_REL);	

	}
	
	return HRTIMER_NORESTART;	
}

bool get_accessory_ftm_mode(void)
{
   return 1;
   return accessory_batt->batt_test_flag;
}
EXPORT_SYMBOL(get_accessory_ftm_mode);

static void accessory_timer_init(void)
{
	/* accessory battery test timer init */
	hrtimer_init(&accessory_test_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	accessory_test_timer.function = accessory_test_delay__func;

	/* usb charger event timer init */
	hrtimer_init(&usb_chg_connect_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	usb_chg_connect_timer.function = usb_chg_event_delay__func;

	/*USBIN charger Mains charge compelete timmr init */
	hrtimer_init(&charge_compelete_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	charge_compelete_timer.function = charge_compelete_delay__func;
	
    #ifdef UPDATE_COULOMB_POLL_FEATURE
	init_timer(&timer_hdq);
	timer_hdq.function = accessory_update_timer__func;
	timer_hdq.expires = jiffies + ACCESSORY_COULOMB_POLL_INIT;
	add_timer(&timer_hdq);
	#endif
}

/*path@/sys/devices/platform/accessory-battery/accessory_batt_test*/
static ssize_t accessory_batt_test_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size) {
		
	int ret=0;
	unsigned long val = simple_strtoul(buf, NULL, 10);
	if(val == 199) {
		print_chg(DEBUG_LEVEL,"%s: accessory battery ftm test on \n",__func__);
		
		/*config ACCESSORY_WIRE_GPIO to output before write data to device */
		ret = pmic8058_gpio__config(ACCESSORY_WIRE_GPIO,0,PM_GPIO_DIR_OUT);
		if(ret){
			print_chg(ERROR_LEVEL,"%s: config failed!\n",__func__);
			return READ_ERROR;
		}
		accessory_batt->batt_test_flag = true;
		hrtimer_start(&accessory_test_timer, 
				ktime_set(0,ACCESSORY_TEST_DELAY), 
				HRTIMER_MODE_REL);	
	}else{
		print_chg(DEBUG_LEVEL,"%s: accessory battery ftm test off \n",__func__);
		outp(ACCESSORY_WIRE_GPIO,0x00); /* Drives DQ low*/
		/*config ACCESSORY_WIRE_GPIO to input before read data to device */
		ret = pmic8058_gpio__config(ACCESSORY_WIRE_GPIO,0,PM_GPIO_DIR_IN);
		if(ret){
			print_chg(ERROR_LEVEL,"%s: config failed!\n",__func__);
			return READ_ERROR;
		}
		accessory_batt->batt_test_flag = false;
	}
	
	return size;
}
static DEVICE_ATTR(accessory_batt_test, 0644, NULL, accessory_batt_test_store);

/**
* path@/sys/devices/platform/accessory-battery/chg_mode_control
* val = 0 close smart charge feature
* val = 1 open smart charge feature
* val = 2 open DC-DC 5V(only setting with close smart charge mode)
* val = 3 close DC-DC 5V(only setting with close smart charge mode)
*/
static ssize_t chg_mode_control_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size) {
	unsigned long val = simple_strtoul(buf, NULL, 10);
	int mains_capacity=0;

	print_chg(DEBUG_LEVEL,"%s: ######=%ld\n",__func__,val);
	if(val ==0){
		print_chg(DEBUG_LEVEL,"%s: close smart charge control\n",__func__);
		accessory_batt->chg_mode = SMART_CHARGE_MODE_INVALID;
		if (accessory_batt->dc_mode == DC_DC_ON_MODE)
			accessory_batt->mains_charge_allow = 1;
		else if (accessory_batt->dc_mode == DC_DC_OFF_MODE)
			accessory_batt->mains_charge_allow = 0;
		
		accessory_notify_event(accessory_batt, TX_ACCESSORY_DATA);
	}else if(val == 1){
		mains_capacity = get_main_battery_capacity();
		print_chg(DEBUG_LEVEL,"%s: open smart charge control\n",__func__);
		accessory_batt->chg_mode = SMART_CHARGE_MODE;
		if(mains_capacity < ACCESSORY_RECHG_LEVEL)
			accessory_batt->mains_charge_allow = 1;
		else
			accessory_batt->mains_charge_allow = 0;
		
		accessory_notify_event(accessory_batt, TX_ACCESSORY_DATA);
	}else if(val == 2){
		print_chg(DEBUG_LEVEL,"%s: open DC-DC\n",__func__);
		accessory_batt->dc_mode = DC_DC_ON_MODE;
		if(accessory_batt->chg_mode == SMART_CHARGE_MODE_INVALID)
			accessory_batt->mains_charge_allow = 1;
		
		accessory_notify_event(accessory_batt, TX_ACCESSORY_DATA);
	}else if(val == 3){
		print_chg(DEBUG_LEVEL,"%s: close DC-DC\n",__func__);
		accessory_batt->dc_mode = DC_DC_OFF_MODE;
		if(accessory_batt->chg_mode == SMART_CHARGE_MODE_INVALID)
			accessory_batt->mains_charge_allow = 0;
		
		accessory_notify_event(accessory_batt, TX_ACCESSORY_DATA);
	}
	return size;
}
static DEVICE_ATTR(chg_mode_control, 0666, NULL, chg_mode_control_store);



static int __devinit accessory_battery_probe(struct platform_device *pdev)
{
	struct accessory_battery *acc_batt;
	int ret=0;
    print_chg(DEBUG_LEVEL,"%s: accessory battery probe\n",__func__);

	/* kzalloc msm_chg */
	acc_batt = kzalloc(sizeof(*acc_batt), GFP_KERNEL);
	if (acc_batt == NULL) {
		ret = -ENOMEM;
		return ret;
	}

	acc_batt->dev = &pdev->dev;
	acc_batt->batt_test_flag = false;
	spin_lock_init(&acc_batt->lock);

	accessory_timer_init();

    /*init accessory_batt status*/
	acc_batt->wireless_present =0;
	acc_batt->accessory_dc_on=0;
	acc_batt->accessory_vbatt=3500;
	acc_batt->accessory_data =0x00;
	acc_batt->disconnected_counter=0;
	acc_batt->mains_charge_allow = 0;
	acc_batt->stop_timer=1;
	acc_batt->data_pre = 0xff;
	acc_batt->chg_mode = SMART_CHARGE_MODE;
	acc_batt->dc_mode = DC_DC_OFF_MODE;
	acc_batt->main_capacity = get_main_battery_capacity();
	acc_batt->mains_capacity_pre = acc_batt->main_capacity;
	
	accessory_charging_status_set(acc_batt, ACCESSORY__CHG_STATUS__NOT_CHARGING);

	if(acc_batt->dev){
		ret = device_create_file(acc_batt->dev, &dev_attr_accessory_batt_test);
		if (ret < 0) {
			print_chg(ERROR_LEVEL,"%s: creat accessory_batt log file failed ret = %d\n", 
					__func__, ret);
			device_remove_file(acc_batt->dev, &dev_attr_accessory_batt_test);
		}
		ret = device_create_file(acc_batt->dev, &dev_attr_chg_mode_control);
		if (ret < 0) {
			print_chg(ERROR_LEVEL,"%s: creat accessory_batt log file failed ret = %d\n",
					__func__, ret);
			device_remove_file(acc_batt->dev, &dev_attr_chg_mode_control);
		}
    }else{
	    print_chg(ERROR_LEVEL,"%s:dev is NULL\n",__func__);
    }

	/*register SN irq*/
	ret = gpio_request(ACCESSORY_WIRE_GPIO, "ACCESSORY_SN_IRQ");
	if (ret) {
		print_chg(ERROR_LEVEL,"%s: gpio_request failed for ACCESSORY_SN_IRQ\n",__func__);
	}
	
	ret = pmic8058_gpio__config(ACCESSORY_WIRE_GPIO,0,PM_GPIO_DIR_IN);
	if(ret){
		print_chg(ERROR_LEVEL,"%s: config ACCESSORY_WIRE_GPIO DIR IN  failed!\n",__func__);
		gpio_free(ACCESSORY_WIRE_GPIO);
	}
	
	acc_batt->sn_irq= gpio_to_irq(ACCESSORY_WIRE_GPIO);

	/*sn_irq irq request*/
	ret = request_threaded_irq(acc_batt->sn_irq, NULL,
				   sn_irq,
				   IRQF_DISABLED | IRQF_TRIGGER_RISING,
				   "accessory_sn_interrupt", NULL);
	if (ret){
		print_chg(ERROR_LEVEL,"%s request_threaded_irq failed for %d rc =%d\n",
			__func__, acc_batt->sn_irq, ret);
		free_irq(acc_batt->sn_irq, NULL);
	}else{	
		disable_irq_nosync( acc_batt->sn_irq);
	}

	acc_batt->queue = kzalloc(sizeof(struct accessory_event)
				* ACCESSORY_MAX_EVENTS,
				GFP_KERNEL);
	if (!acc_batt->queue) {
		ret = -ENOMEM;
		kfree(acc_batt);
		goto out;
	}
	
	acc_batt->tail = 0;
	acc_batt->head = 0;
	spin_lock_init(&acc_batt->queue_lock);
	acc_batt->queue_count = 0;
	INIT_WORK(&acc_batt->queue_work, process_events);
	acc_batt->event_wq_thread = create_singlethread_workqueue("accessory_eventd");
	if (!acc_batt->event_wq_thread) {
		ret = -ENOMEM;
		kfree(acc_batt->queue);
		kfree(acc_batt);
		goto out;
	}

	accessory_batt = acc_batt;
out:
	return ret;

}

static int __devexit accessory_battery_remove(struct platform_device *pdev)
{
	struct accessory_battery *acc_batt = accessory_batt;
	
	/* destroy work thread data */
	flush_workqueue(acc_batt->event_wq_thread);
	destroy_workqueue(acc_batt->event_wq_thread);
	kfree(acc_batt->queue);
	
    /* remove log sys file */
	device_remove_file(acc_batt->dev, &dev_attr_accessory_batt_test);
	/* remove log sys file */
	device_remove_file(acc_batt->dev, &dev_attr_chg_mode_control);
	return 0;
}

static int accessory_suspend(struct platform_device *pdev,
				  pm_message_t state)
{
	#ifdef UPDATE_COULOMB_POLL_FEATURE
	cancel_work_sync(&accessory_batt->queue_work);
	del_timer_sync(&timer_hdq);
	#endif
	return 0;
}

static int accessory_resume(struct platform_device *pdev)
{
	#ifdef UPDATE_COULOMB_POLL_FEATURE
	add_timer(&timer_hdq);
	#endif
	return 0;
}

static struct platform_driver accessory_battery_driver = {
	.probe = accessory_battery_probe,
	.remove = __devexit_p(accessory_battery_remove),
	.driver = {
		   .name = "accessory-battery",
		   .owner = THIS_MODULE,
	},
	.suspend  = accessory_suspend,
	.resume	  = accessory_resume,
};

static int __init accessory_battery_init(void)
{
	return platform_driver_register(&accessory_battery_driver);
}

static void __exit accessory_battery_exit(void)
{
	platform_driver_unregister(&accessory_battery_driver);
}

late_initcall(accessory_battery_init);
module_exit(accessory_battery_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("accessory battery driver");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:accessory battery");
