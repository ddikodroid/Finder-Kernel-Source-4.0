/*
 *  tsl2773.c - Linux kernel modules for ambient light sensor
 *
 *  Copyright (C) 2009 wangjc <wjc@oppo.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>

#include <linux/fs.h>
#include <linux/major.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/input.h>
#include <linux/errno.h>
#include <linux/sysctl.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/kthread.h>
#include <mach/gpio.h>

#include <linux/tsl2773.h> 

#include <linux/mfd/pmic8058.h>

#include <linux/proc_fs.h>

#include <linux/wakelock.h>

#define DRIVER_VERSION    "1.1.1"

#define TSL_DEBUG
#ifdef TSL_DEBUG
#define PRINTK(format, arg...) do { \
    printk(KERN_INFO"%s: " format "\n" , __func__ , ## arg); \
} while (0)
#else
#undef PRINTK
#define PRINTK(format, arg...)
#endif
#define PRINTK_ERR(format, arg...)   printk(KERN_ERR"%s: " format "\n" , __func__ , ## arg)

#define PRINT_LOG	0
#define PRINTK_IF(test,format, arg...) if(test ==1) { \
    printk(KERN_INFO"%s: " format "\n" , __func__ , ## arg); \
} 

/* OPPO 2010-10-30 Laijl Add begin for USE MACRO instead REG & CMD  */
#define TSL_REG_EN              0x00
#define TSL_REG_ATIME       0x01
#define TSL_REG_PTIME       0x02
#define TSL_REG_WTIME     0x03
#define TSL_REG_AILTL       0x04
#define TSL_REG_AILTH       0x05
#define TSL_REG_AIHTL       0x06
#define TSL_REG_AIHTH       0x07
#define TSL_REG_PILTL       0x08
#define TSL_REG_PILTH       0x09
#define TSL_REG_PIHTL       0x0A
#define TSL_REG_PIHTH       0x0B
#define TSL_REG_PERS            0x0C
#define TSL_REG_CONFIG      0x0D
#define TSL_REG_PPCOUNT     0x0E
#define TSL_REG_CONTROL     0x0F
#define TSL_REG_REV         0x11
#define TSL_REG_ID      0x12
#define TSL_REG_STATUS     0x13
#define TSL_REG_CDATAL     0x14
#define TSL_REG_CDATAH     0x15
#define TSL_REG_IRDATAL    0x16
#define TSL_REG_IRDATAH     0x17
#define TSL_REG_PDATAL     0x18
#define TSL_REG_PDATAH     0x19

#define TSL_CMD     0x80
#define TSL_OP_REP     0x00
#define TSL_OP_INC     0x20
#define TSL_OP_SF      0x60

#define TSL_ISR_PC              0x05
#define TSL_ISR_AC              0x06
#define TSL_ISR_APC              0x07

#define ASL_READ_COUNT             5    //ALS read data times

#define ISR_MASK_PRO           0x21
#define ISR_MASK_ASL             0x11
/* OPPO 2010-10-30 Laijl Add end */

#define TSL_GA  60  // Glass Attenuation of ALS

#define DRIVER_CURRENT  0x0   //0--100mA, 1--50mA, 2--25mA, 3--12.5mA,
#define P_COUNT               0x02  //0x02  2 pules

#define P_TIME                  0xff    //0xff  Set Prox Integration time = 2.72ms
#define W_TIME                 0xff   //0xee=50ms, 0xff= 2.72ms //Set Wait time 
#define P_CYCLE_W          0x0    //
#define P_GAIN                  ((DRIVER_CURRENT<<6)|0x22)    // 7:6 driver current, 5:4 diod choose, 1:0 gain

/* for report ALS event and PROX event */
#define REPORT_PROX   1
#define REPORT_ASL      0

static int A_TIME = 0xEE;  //Set ALS Integration time = 50(48.96ms)


//int g_sensor_prempt;

/*enum state_diagram     //don't use state instead of als_flag and p_flag
{
    SLEEP = 0,
    //START,
    PROX,
    //WAIT,
    ALS,
    PROXandALS,//laijl add
    state_end
}; */

static atomic_t als_flag = ATOMIC_INIT( 0 );
static atomic_t p_flag = ATOMIC_INIT( 0 );
static u16 current_object = 0;    /* current PROX sensor state, 0: far  1: near*/


static u16 PROX_LOWER_THRESHOLD = 0;
static u16 PROX_UPPER_THRESHOLD = 1023;

#define PROX_ABS_HIGH       0xffff
#define PROX_ABS_LOW        0x0


//#define	CLOSE_PERMISSION
/****************************************************************************/
#define ALS_MAX     11980 //31200--89 , 11980

#define PM8058_GPIO_PM_TO_SYS(pm_gpio)		(pm_gpio + 173)
#define LIGHT_SENSOR_PM_GPIO   		PM8058_GPIO_PM_TO_SYS(33)

static int saturation;
static int add_lux = 1;
static int sun_lux = 0;
static int lux_final = 0;

static char flag_suspend = 0;
//static int prox_wakecount = -1;


#define CALL_NORMAL				0x00
#define CALL_SUSPEND				0x01
#define CALL_WAKEUPBYPROX		0x02


static struct workqueue_struct *tsl2773_wq;

struct tsl2773_data
{
    struct i2c_client *client;
    struct work_struct work;
    struct delayed_work delay_work;
    struct delayed_work prox_work;
    struct delayed_work lock_work;	
    struct input_dev *input_dev;
    //enum state_diagram state;
    int ( *power )(void ); /* OPPO Laijl modify at 2010-10-11,20:06:05 */	
    int (*power_off) (void);
};

struct light_data
{
    u16 light;
    u16 proximity;
/* OPPO 2010-12-24 Laijl Add begin for engeering mode */
    u16 ADC_PROX;
/* OPPO 2010-12-24 Laijl Add end */
};

static struct light_data lights;
struct tsl2773_data *tsl_data = NULL;

static struct wake_lock prox_wake_lock;

static inline void init_suspend(void)
{
	wake_lock_init(&prox_wake_lock, WAKE_LOCK_SUSPEND, "tsl2773");
}

static inline void deinit_suspend(void)
{
	wake_lock_destroy(&prox_wake_lock);
}

static inline void prevent_suspend(void)
{
	wake_lock(&prox_wake_lock);
}

static inline void allow_suspend(void)
{
	wake_unlock(&prox_wake_lock);
}

/*
static ssize_t Proc_write( struct file *filp, const char __user *buff,
                        unsigned long len, void *data )
{
	char proc_data[32] = {0};  

		if (copy_from_user( &proc_data, buff, len )) {
		printk("proc_write read data error.\n");
		return -EFAULT;
	}

	if(proc_data[0] == 'L')
	{
		g_sensor_prempt = proc_data[1] -0x30;
	}
	
	return len;
}


static struct proc_dir_entry *proc_entry;
static int init_proc(struct tsl2773_data *data)
{
	int ret=0;
	
	proc_entry = create_proc_entry( "Sensor_proc_write", 0666, NULL );
	proc_entry->data = data;
	
	if (proc_entry == NULL) {
		ret = -ENOMEM;
	  	printk( "Couldn't create proc entry\n");
	} else {
		proc_entry->write_proc = Proc_write;
		//proc_entry->owner = THIS_MODULE;
	}
	PRINTK( " sensor create proc entry OK!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  
	return ret;
}
*/
static void tsl_report_value( int flag )
{
    struct tsl2773_data *data = tsl_data;
	
    //printk("huyu -----------p_flag = %d,  als_flag = %d\n",atomic_read( &p_flag ) ,atomic_read( &als_flag ) );

    /* Report ambient information */
    if ( atomic_read( &als_flag ) && (flag == REPORT_ASL) /*&& (g_sensor_prempt & 0x01) == 0*/)
    {
      input_report_abs( data->input_dev, ABS_HAT1X, lights.light );
    }

    /* Report proximity information */
    if ( atomic_read( &p_flag ) &&(flag == REPORT_PROX)/*&& (g_sensor_prempt & 0x02) == 0*/)
    {
      input_report_abs( data->input_dev, ABS_HAT1Y, lights.proximity );
    }
	//printk("huyu-next--888888888888888888 light=%d\n", lights.light);
	//printk("huyu-next--888888888888888888 proximity=%d\n", lights.proximity);

    input_sync( data->input_dev );
}

static int get_gain( int data )
{
    switch ( data )
    {
      case 0:
        return 1;
      case 1:
        return 8;
      case 2:
        return 16;
      case 3:
        return 120;
      default:
        return 0;
    }
}


static int AtimeToMS( int data )
{
    return ( ( 256 - data ) * 272 / 100 );
}

#define Pcount 20
static int  tsl2771_calibrate(u16 *cali_data)
{
    int i, prox_data;
    int PD_sum ,PD_mean , PD_max;
    u16 buf[3];
    
    PD_sum=0;
    PD_mean = 0;
    PD_max = 0;	

    printk(KERN_INFO"read prox_mean for the consistency!\n");    

    mdelay(10);
    for (i = 0; i < Pcount; i++)
    {
        prox_data = i2c_smbus_read_word_data(tsl_data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PDATAL/*0xb8*/);
        if(PD_max < prox_data)
        {
            PD_max = prox_data;
        }
        PD_sum +=prox_data;
	//printk(KERN_INFO"prox_data = %d \n",  prox_data);
	mdelay(50);
    }
    PD_mean =PD_sum/Pcount;

    buf[0] = 0;
    buf[1] = 0;
    buf[2] = PD_mean;

    memcpy(cali_data, &(buf[0]), sizeof(u16) * 3);

    return 0;
}

/* OPPO 2011-05-27 dongfeilong Add begin for calibrate at call */
#define Timely_Pcount 5
static int prox_mean = 1023;
static int PD_sum = 0;
static int PD_count = 1;

#ifdef CLOSE_PERMISSION
static int flag_firststart = 1;
#endif

static void prox_work_func( struct work_struct *work )
{
	int prox_data;
	int PD_temp;
	int asl_data;

	prox_data = i2c_smbus_read_word_data(tsl_data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PDATAL/*0xb8*/);
	//printk(KERN_INFO"prox_data = %d \n", prox_data);
	PD_sum += prox_data;

	if (prox_data < 10)
	{
		PD_sum -= prox_data;
		PD_count -= 1;
	}

	if (PD_count < Timely_Pcount)
	{
		PD_count = PD_count + 1;
	}
	else
	{		
		PD_temp = PD_sum /Timely_Pcount;
		PD_sum = 0;
		PD_count = 1;
		//printk(KERN_INFO"prox_temp = %d \n", PD_temp);
		asl_data = i2c_smbus_read_word_data( tsl_data->client,	TSL_CMD|TSL_OP_INC|TSL_REG_CDATAL/*0xb6*/);

		if (current_object == 0 && asl_data < (( saturation /5)*4))
		{
			if (prox_mean < 750 && (PD_temp > prox_mean + 60))
			{
				i2c_smbus_write_word_data( tsl_data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PILTL/*0xa8*/, PROX_ABS_LOW);
				i2c_smbus_write_word_data( tsl_data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PIHTL/*0xaa*/, PROX_UPPER_THRESHOLD);
				//printk(KERN_INFO"set threshold aaaaaaaaaaaaaaaaaa--------PROX_LOWER_THRESHOLD = %d PROX_UPPER_THRESHOLD = %d  \n", PROX_LOWER_THRESHOLD, PROX_UPPER_THRESHOLD);
			}
			if ((PD_temp < prox_mean + 60) && PD_temp < 800 )
			{
				prox_mean = PD_temp; 
				PROX_LOWER_THRESHOLD = prox_mean + 40;
				PROX_UPPER_THRESHOLD = prox_mean + 60;  
				//printk(KERN_INFO"PROX_LOWER_THRESHOLD = %d PROX_UPPER_THRESHOLD = %d  \n", PROX_LOWER_THRESHOLD, PROX_UPPER_THRESHOLD);
				i2c_smbus_write_word_data( tsl_data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PILTL/*0xa8*/, PROX_ABS_LOW);
				i2c_smbus_write_word_data( tsl_data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PIHTL/*0xaa*/, PROX_UPPER_THRESHOLD);
				//printk(KERN_INFO"set threshold bbbbbbbbbbbbbbbb--------PROX_LOWER_THRESHOLD = %d PROX_UPPER_THRESHOLD = %d  \n", PROX_LOWER_THRESHOLD, PROX_UPPER_THRESHOLD);
			}
			if(PD_temp == 1023)
			{
				//sensor启动之前即挡住，确保触发一次中�?				PRINTK_IF(PRINT_LOG,"huyu----set PROX_UPPER_THRESHOLD = 800");
				PROX_LOWER_THRESHOLD = 700;
				PROX_UPPER_THRESHOLD = 800;  
				i2c_smbus_write_word_data( tsl_data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PILTL/*0xa8*/, PROX_ABS_LOW);
				i2c_smbus_write_word_data( tsl_data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PIHTL/*0xaa*/, PROX_UPPER_THRESHOLD);
			
			}
#ifdef CLOSE_PERMISSION
			if(PD_temp == 1023 && flag_firststart)
			{
				PRINTK_IF(PRINT_LOG,"huyu----start with close");
				flag_firststart = 2;
				current_object = 1;
				lights.proximity = 2;
				tsl_report_value(REPORT_PROX);
				PROX_LOWER_THRESHOLD = PD_temp -50;
				PROX_UPPER_THRESHOLD = PD_temp;
				i2c_smbus_write_word_data( tsl_data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PILTL/*0xa8*/, PROX_LOWER_THRESHOLD);
				i2c_smbus_write_word_data( tsl_data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PIHTL/*0xaa*/, PROX_ABS_HIGH );
				i2c_smbus_write_byte(tsl_data->client, TSL_CMD|TSL_OP_SF |TSL_ISR_APC/*0xe7*/ ); /*clear interrupt*/
			}
#endif
		}
		//printk(KERN_INFO"prox_mean = %d current_object = %d  \n", prox_mean, current_object);
	}
	if ((current_object == 0) && atomic_read( &p_flag ))
	{	
		schedule_delayed_work(&tsl_data->prox_work, msecs_to_jiffies(50));
	}
	//printk(KERN_INFO"PROX_LOWER_THRESHOLD = %d PROX_UPPER_THRESHOLD = %d	\n", PROX_LOWER_THRESHOLD, PROX_UPPER_THRESHOLD);
#if 0
	if(prox_wakecount >= 0)
	{
		if(prox_wakecount == 10)
		{
			PRINTK_IF(PRINT_LOG,"huyu --------tsl_interrupt work allow_suspend()!--\n");
			allow_suspend();
			prox_wakecount = -1;
		}
		else
		{
			prox_wakecount ++ ;
		}	
	}
#endif	
}
static void lock_work_func( struct work_struct *work )
{
	PRINTK_IF(PRINT_LOG,"huyu --------tsl_interrupt work allow_suspend()!--\n");
	allow_suspend();
}
/* OPPO 2011-05-27 dongfeilong Add end */

/* read ALS data and reprot the average data repeatedly */
static void delay_work_func( struct work_struct *work )
{
    struct tsl2773_data *data;
    int adc0, adc1;
    int lux;
    int ratio, temp1;
    int irf = 0;

	if ( atomic_read( &p_flag ) == 1 )
	{
		return;
	}

    data = tsl_data;

    adc0 = i2c_smbus_read_word_data( data->client, TSL_CMD|TSL_OP_INC|TSL_REG_CDATAL/*0xb4*/);
    if ( adc0 < 0 )
    {
      PRINTK_ERR( " read adc0 err-----------------------------------------------------------------------------------!!!!!als_flag = %d\n",atomic_read( &als_flag )  );
	  return;		//i2c data线被拉低后，一直报错会导致死机
    }
    adc1 = i2c_smbus_read_word_data( data->client,  TSL_CMD|TSL_OP_INC|TSL_REG_IRDATAL/*0xb6*/);
    if ( adc1 < 0 )
    {
      PRINTK_ERR( " read adc1 err-----------------------------------------------------------------------------------!!!!!als_flag = %d\n",atomic_read( &als_flag ) );
	  return;
    }

    if ( adc0 < adc1 )
    {
      adc0 = i2c_smbus_read_word_data( data->client, TSL_CMD|TSL_OP_INC|TSL_REG_CDATAL/*0xb4*/ );
      if ( adc0 < 0 )
      {
        PRINTK_ERR( " read adc0 err-----------------------------------------------------------------------------------!!!!!als_flag = %d\n",atomic_read( &als_flag ) );
		return;
      }
    }

    if ( adc0 >= saturation )
    {
      lux = ALS_MAX;
    }
    else
    {
      if ( adc0 != 0 )
      {
        ratio = ( adc1 * 100 ) / adc0;
      }
      else
      {
        ratio = 100;
      }

      if ( ratio > 0 && ratio <= 30 )
      {
        irf = ( 100000 - 1846 * ratio ) / 100;
      }
      else if ( ratio > 30 && ratio <= 38 )
      {
        irf = ( 126800 - 2740 * ratio ) / 100;
      }
      else if ( ratio > 38 && ratio <= 45 )
      {
        irf = ( 74900 - 1374 * ratio ) / 100;
      }
      else if ( ratio > 45 && ratio <= 54 )
      {
        irf = ( 47700 - 769 * ratio ) / 100;
      }
      else if ( ratio > 54 )
      {
        irf = 0;
      }

      temp1 = AtimeToMS( A_TIME ) * get_gain( ( P_GAIN )&0x03 )*TSL_GA;

      lux = 1 * 52 * irf * adc0 / temp1;

      if(lux > ALS_MAX)
      {
        lux = ALS_MAX;
      }

    }
    sun_lux = sun_lux + lux;
	
    if ( add_lux < ASL_READ_COUNT )
      {
        add_lux = add_lux + 1;
      }
      else
      {
        lux_final = sun_lux / add_lux;
        add_lux = 1;
        sun_lux = 0;
/* OPPO 2011-07-19 dongfeilong Add begin for the event can't report 0 first time in the dark state */
	//if ( 0 == lux_final)
	if ( 1 >= lux_final)
	{
		
		if(lights.light == 1)
		{
			lights.light = 2;
		}
		else
			lights.light = 1;
	}
	else
	{
		lights.light = lux_final;
	}
/* OPPO 2011-07-19 dongfeilong Add end */
        tsl_report_value(REPORT_ASL);
      }
	  
      schedule_delayed_work( &data->delay_work, msecs_to_jiffies( 53 ) );

    return;
}

/* init sensor and start work func for als_flag and p_flag */
static int start_sensor( struct tsl2773_data *data)
{
    lights.proximity = 15;
    saturation = 1024 * ( 256 - 0xee );
    i2c_smbus_write_byte_data( data->client, TSL_CMD|TSL_REG_EN/*0x80*/, 0x01 );  //power up
    i2c_smbus_write_byte_data( data->client, TSL_CMD| TSL_REG_ATIME/*0x81*/, A_TIME );  //Set ALS Integration time = 50(48.96ms)
    i2c_smbus_write_byte_data( data->client, TSL_CMD|TSL_REG_PTIME /*0x82*/, P_TIME );  //Set Prox Integration time = 2.72ms
    i2c_smbus_write_byte_data( data->client,  TSL_CMD|TSL_REG_WTIME/*0x83*/, W_TIME );  //Set Prox Wait time = 2.7ms

    if (current_object == 0)
    {
    	i2c_smbus_write_word_data( data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PILTL/*0xa8*/, PROX_ABS_LOW );    //low threshold
    	i2c_smbus_write_word_data( data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PIHTL/*0xaa*/, PROX_ABS_HIGH);  //high threshold
	i2c_smbus_write_byte_data( data->client, TSL_CMD|TSL_REG_PERS /*0x8c*/, 0x73 );  // 3 proximity values,or 3 ALS velues , out of range,
    }
    else
    {
	i2c_smbus_write_byte_data( data->client, TSL_CMD|TSL_REG_PERS /*0x8c*/, 0x13 );  // 3 proximity values,or 3 ALS velues , out of range,
    }

    //i2c_smbus_write_byte_data( data->client, TSL_CMD|TSL_REG_PERS /*0x8c*/, 0x23 );  // 3 proximity values,or 3 ALS velues , out of range,
    i2c_smbus_write_byte_data( data->client, TSL_CMD|TSL_REG_CONFIG/*0x8d*/, P_CYCLE_W ); //Set Proximity Cycle Wait 1×
    i2c_smbus_write_byte_data( data->client, TSL_CMD|TSL_REG_PPCOUNT/*0x8e*/, P_COUNT ); //Set Proximity Count= 02
    i2c_smbus_write_byte_data( data->client, TSL_CMD|TSL_REG_CONTROL/*0x8f*/, P_GAIN );  //Set gain = 16,LED Drive=100mA、Proximity uses the channel 1 diode

    msleep( 3 );//laijl add
    PRINTK_IF(PRINT_LOG,"huyu -----------p_flag = %d,  als_flag = %d\n",atomic_read( &p_flag ) ,atomic_read( &als_flag ) );
    if ( atomic_read( &p_flag ) == 1 )
    {    
      i2c_smbus_write_byte_data( data->client, TSL_CMD|TSL_REG_EN/*0x80*/, 0x27 ); //power up，ADC、proximity Enable, prox interrupt enable
      PD_sum = 0;
      PD_count = 1;
#ifdef CLOSE_PERMISSION
      flag_firststart = 1;
#endif
      schedule_delayed_work(&data->prox_work, msecs_to_jiffies( 13 ));
      if (atomic_read( &als_flag ) == 1)
      {
		  PRINTK_IF(PRINT_LOG,"huyu-----cancel_delayed_work ---------cancel light sensor as Psensor is working !\n");
      	cancel_delayed_work_sync(&data->delay_work);
      }
    }
    else if ( atomic_read( &als_flag ) == 1 )
    {   
		PRINTK_IF(PRINT_LOG,"huyu-----schedule_delayed_work ---------start light sensor !\n");
      i2c_smbus_write_byte_data( data->client, TSL_CMD|TSL_REG_EN/*0x80*/, 0x0b ); //power up
      schedule_delayed_work( &data->delay_work, msecs_to_jiffies( 203 ) );
    }
    else
    {
		PRINTK_IF(PRINT_LOG,"huyu-----cancel_delayed_work ---------cancel light sensor when no sensor is working !\n");
      i2c_smbus_write_byte_data( data->client, TSL_CMD|TSL_REG_EN/*0x80*/, 0x00 );  //power down
      cancel_delayed_work(&data->delay_work);
    }
    return 0;
}

#if 1 /*OPPO 2011-6-17 sunjianbo add*/
extern void Light_Sensor_reinit(void)
{
	if(tsl_data) {
		start_sensor(tsl_data);
	}	
}
#endif

/* prox near and far interrupt work func */
static void tsl_work_func( struct work_struct *work )
{
    struct tsl2773_data *data = container_of( work, struct tsl2773_data, work );
    int prox_data, asl_data;   
    //printk("huyu ----------%s---------p_flag = %d,\n",__func__,atomic_read( &p_flag ) );
   
    if (atomic_read( &p_flag )  == 1)
    {
    	    asl_data = i2c_smbus_read_word_data( data->client,  TSL_CMD|TSL_OP_INC|TSL_REG_CDATAL/*0xb6*/);

	    if (asl_data < (( saturation /5)*4) && current_object == 0)
	    {
			cancel_delayed_work_sync(&tsl_data->prox_work);		
	    	current_object = 1;
		printk(KERN_INFO"tsl_interrupt near\n" );
		lights.proximity = 2;//current_object;
		prox_data = i2c_smbus_read_word_data(data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PDATAL/*0xb8*/);
		if (prox_data > 1000)
		{
			PROX_LOWER_THRESHOLD = PROX_LOWER_THRESHOLD + 50;
			PROX_UPPER_THRESHOLD = PROX_UPPER_THRESHOLD + 50;
			i2c_smbus_write_word_data( data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PILTL/*0xa8*/, PROX_LOWER_THRESHOLD);
			i2c_smbus_write_word_data( data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PIHTL/*0xaa*/, PROX_ABS_HIGH );
		}
		else 
		{	
			i2c_smbus_write_word_data( data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PILTL/*0xa8*/, PROX_LOWER_THRESHOLD);
			i2c_smbus_write_word_data( data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PIHTL/*0xaa*/, PROX_ABS_HIGH );
		}
	   }
	    else if (current_object == 1)
	    {
	        current_object = 0;
	        printk(KERN_INFO"tsl_interrupt far\n" );
	        lights.proximity = 15;//current_object;  
#ifdef CLOSE_PERMISSION
	        if(flag_firststart == 2)
	        {
			prox_data = i2c_smbus_read_word_data(data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PDATAL/*0xb8*/);
			PRINTK_IF(PRINT_LOG,"huyu -------start with close then away---PROX_UPPER_THRESHOLD = %d---------\n",prox_data);
		        flag_firststart = 0;
	        	PROX_UPPER_THRESHOLD = prox_data;
			schedule_delayed_work(&tsl_data->prox_work, msecs_to_jiffies(50));
				
	        }
#endif			
		/*---------------------------------------------------------------------------*/
		if((flag_suspend & CALL_WAKEUPBYPROX) != 0)
		{
			//prox_wakecount = 0;
			flag_suspend = CALL_NORMAL;
			prevent_suspend();
			PRINTK_IF(PRINT_LOG,"huyu --------tsl_interrupt work prevent_suspend()!--\n");
			schedule_delayed_work( &data->lock_work, msecs_to_jiffies(1000));
		}
		/*---------------------------------------------------------------------------*/
		i2c_smbus_write_word_data( data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PILTL/*0xa8*/, PROX_ABS_LOW);
		i2c_smbus_write_word_data( data->client, TSL_CMD|TSL_OP_INC|TSL_REG_PIHTL/*0xaa*/, PROX_UPPER_THRESHOLD);
		//reset the threshold,
		schedule_delayed_work(&tsl_data->prox_work, msecs_to_jiffies(50));
	     }
	     tsl_report_value(REPORT_PROX);
    }


    //PRINTK_IF(PRINT_LOG,"PROX_LOWER_THRESHOLD = %d PROX_UPPER_THRESHOLD = %d  \n", PROX_LOWER_THRESHOLD, PROX_UPPER_THRESHOLD);
    i2c_smbus_write_byte(data->client, TSL_CMD|TSL_OP_SF |TSL_ISR_APC/*0xe7*/ ); /*clear interrupt*/
}

static irqreturn_t tsl_interrupt( int irq, void *dev_id )
{
    struct tsl2773_data *data = dev_id;

    PRINTK_IF(PRINT_LOG,"huyu ----------%s---------\n",__func__);
	PRINTK_IF(PRINT_LOG,"huyu --------flag_suspend = %x--\n", flag_suspend);
    disable_irq_nosync(data->client->irq);
    if((flag_suspend & CALL_SUSPEND) != 0)
    {
	flag_suspend |= CALL_WAKEUPBYPROX;
    }
    else if(flag_suspend == CALL_NORMAL)
    {
    	queue_work(tsl2773_wq, &data->work);
    }
    enable_irq(data->client->irq);

    return IRQ_HANDLED;
}

static int tsl2773_init_client( struct i2c_client *client )
{
    struct tsl2773_data *data = i2c_get_clientdata( client );
    int err = 0;
	

    err = i2c_smbus_write_byte_data( data->client, TSL_CMD|TSL_REG_EN/*0x80*/, 0x01 );  //power up
    if ( err < 0 )
    {
      return err;
    }
    //err = request_irq( client->irq, tsl_interrupt, IRQF_DISABLED | IRQF_TRIGGER_FALLING, "tsl2773", data );
	
	err = request_threaded_irq(client->irq,NULL, tsl_interrupt,
				IRQF_TRIGGER_FALLING,
				"tsl2773", data);
	
    if ( err < 0 )
    {
      PRINTK_ERR( " request irq failed\n" );
    }
    mdelay( 1 );

    return err;
}

static int tsl_dev_open( struct inode *inode, struct file *file )
{  
    //printk(KERN_INFO" TSL2773 open!");
    return nonseekable_open( inode, file );
}

static int tsl_dev_release( struct inode *inode, struct file *file )
{   
    return 0;
}

static long tsl_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    void __user *argp = ( void __user * )arg;
    u16 cali[3];
    short flag;
    s32 ret = 0;
    struct tsl2773_data *data = tsl_data;

    PRINTK_IF(0, " TSL2773 ioctl CMD=%x,  ", cmd );

    switch ( cmd )
    {
      case ECS_IOCTL_APP_SET_ALSFLAG:
        if ( copy_from_user( &flag, argp, sizeof( flag ) ) )
        {
          return -EFAULT;
        }

        if ( flag < 0 || flag > 1 )
        {
          return -EINVAL;
        }
        atomic_set( &als_flag, flag );
		PRINTK_IF(PRINT_LOG,"ECS_IOCTL_APP_SET_ALSFLAG----------44444444-------flag = %d\n",flag);
        ret = start_sensor( data );
        break;

      case ECS_IOCTL_APP_GET_ALSFLAG:
        flag = atomic_read( &als_flag );
        if ( copy_to_user( argp, &flag, sizeof( flag ) ) )
        {
          return -EFAULT;
        }
        break;

      case ECS_IOCTL_APP_SET_PFLAG:
        if ( copy_from_user( &flag, argp, sizeof( flag ) ) )
        {
          return -EFAULT;
        }

        if ( flag < 0 || flag > 1 )
        {
          return -EINVAL;
        }
        atomic_set( &p_flag, flag );
/*add by zwx for wake up AP when phone leave face during phone and AP put into sleep mode 2011-03-12 start*/
		if (1 == flag)
		{
			enable_irq_wake( data->client->irq);
			prox_mean = 1023;
		}
		else
		{
			disable_irq_wake(data->client->irq);
/* OPPO 2011-05-04 dongfeilong Add begin for reset current prox state */
			cancel_delayed_work(&data->prox_work);
			current_object = 0;
/* OPPO 2011-05-04 dongfeilong Add end */
		}
/*add by zwx for wake up AP when phone leave face during phone and AP put into sleep mode 2011-03-12 end*/
	  	PRINTK_IF(PRINT_LOG,"ECS_IOCTL_APP_SET_PFLAG----------5555555555555-------flag = %d\n",flag);
        ret = start_sensor( data );
        break;

      case ECS_IOCTL_APP_GET_PFLAG:
        flag = atomic_read( &p_flag );
        if ( copy_to_user( argp, &flag, sizeof( flag ) ) )
        {
          return -EFAULT;
        }
        break;

      case ECS_IOCTL_APP_CALIBRATE:
        ret = tsl2771_calibrate(cali);
        if(ret < 0)
        {
          return -EFAULT;
        }
        if (copy_to_user(argp, cali, sizeof(u16) * 3))
        {
          return -EFAULT;
        }
       break;
      default:
        break;
    }

    return ret;
}

static ssize_t tsl_dev_read( struct file *filp, char __user *buf, size_t count, loff_t *f_pos )
{
    int len = 0;
    int old_data1 = 0;
    len = sizeof( struct light_data );

    if ( count > len )
    {
      count = len;
    }
/* OPPO 2010-12-24 Laijl Add begin for reason */
    old_data1= i2c_smbus_read_word_data( tsl_data->client,  TSL_CMD|TSL_OP_INC|TSL_REG_PDATAL );
    lights.ADC_PROX =old_data1;
/* OPPO 2010-12-24 Laijl Add end */

    return copy_to_user( buf, &lights, count ) ? -EFAULT : count;
}

static struct file_operations tsl_dev_fops = {
	.owner = THIS_MODULE,
	.open = tsl_dev_open,
	.release = tsl_dev_release,
	.read = tsl_dev_read,
	.unlocked_ioctl	 = tsl_dev_ioctl,
};

static struct miscdevice tsl_dev_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "lsensor_dev",
	.fops = &tsl_dev_fops,
};

void setAlsflag(int flag)
{
	struct tsl2773_data *data = tsl_data;
	int ret;

	atomic_set( &als_flag, flag );
	ret = start_sensor( data );

}
void setPflag(int flag)
{
	struct tsl2773_data *data = tsl_data;
	int ret;
			atomic_set( &p_flag, flag );
	/*add by zwx for wake up AP when phone leave face during phone and AP put into sleep mode 2011-03-12 start*/
			if (1 == flag)
			{
				enable_irq_wake( data->client->irq);
				prox_mean = 1023;
			}
			else
			{
				disable_irq_wake(data->client->irq);
	/* OPPO 2011-05-04 dongfeilong Add begin for reset current prox state */
				cancel_delayed_work(&data->prox_work);
				current_object = 0;
	/* OPPO 2011-05-04 dongfeilong Add end */
			}
	/*add by zwx for wake up AP when phone leave face during phone and AP put into sleep mode 2011-03-12 end*/
			ret = start_sensor( data );

}

static int __devinit tsl2773_probe( struct i2c_client *client, const struct i2c_device_id *id )
{
    struct i2c_adapter *adapter = to_i2c_adapter( client->dev.parent );
    struct tsl2773_data *data;
    struct light_sensor_platform_data *pdata;
#if 1
    int sensor_ID = 0;
#endif
    int err = 0;

    if ( !i2c_check_functionality( adapter, I2C_FUNC_SMBUS_BYTE ) )
    {
      err = -EIO;
      goto exit;
    }

    data = kzalloc( sizeof( struct tsl2773_data ), GFP_KERNEL );
    if ( !data )
    {
      err = -ENOMEM;
      goto exit;
    }
    /* OPPO Laijl modify at 2010-10-12,16:29:20 */
    pdata = client->dev.platform_data;
    data->power = pdata->power_on;
    data->power_off = pdata->power_off;
#if 1
    if ( data->power )
    {
#if 0    
      //err = data->power( 1 );
      err = data->power( );
#else
      err = data->power( );
	  msleep(10);
	  data->power_off();
	  msleep(100);	  
      err = data->power( );
#endif
      if ( err < 0 )
      {
        PRINTK_ERR( "tsl2773_probe power on failed\n" );
        goto exit;
      }
    }
#endif
    /* OPPO Laijl modify at 2010-10-12,16:29:20 */
    INIT_WORK( &data->work, tsl_work_func );
    INIT_DELAYED_WORK( &data->delay_work, delay_work_func );
    INIT_DELAYED_WORK(&data->prox_work, prox_work_func );
    INIT_DELAYED_WORK(&data->lock_work, lock_work_func );

    client->irq = gpio_to_irq(LIGHT_SENSOR_PM_GPIO);
    data->client = client;
    i2c_set_clientdata( client, data );

    /* Initialize the TSL2773 chip */
    err = tsl2773_init_client( client );
    if ( err )
    {
      PRINTK_ERR( "tsl2773_init_client err\n" );
      goto exit_kfree;
    }
#if 0		
    sensor_ID = i2c_smbus_read_byte_data( client, TSL_CMD|TSL_REG_ID/*0x92*/ );
    if ( sensor_ID != 0x29 )
    {
      PRINTK_ERR( "TSL light sensor is not tmd27713 !!! " );
      goto exit_kfree;
    }
    else
    {
      PRINTK( "tsl2773_probe TSL light sensor detect ,ID=%0xx !!! ", sensor_ID);
    }
#else
	sensor_ID = i2c_smbus_read_byte_data( client, TSL_CMD|TSL_REG_ID/*0x92*/ );
	if ( sensor_ID == 0x29 )
	{
	  PRINTK( "TSL light sensor is tmd27713  !!! " );
	  
	}
	else if ( sensor_ID == 0x20 )
	{
		PRINTK( "TSL light sensor is tmd27711  !!! " );
	}

#endif
    data->input_dev = input_allocate_device();
    if ( !data->input_dev )
    {
      err = -ENOMEM;
      PRINTK_ERR( "tsl2773_probe: Failed to allocate input device\n" );
      goto exit_input_dev_alloc_failed;
    }

    set_bit( EV_ABS, data->input_dev->evbit );

    /* ambient */
    input_set_abs_params( data->input_dev, ABS_HAT1X, 0, 20000, 0, 0 );
    /* proximity */
    input_set_abs_params( data->input_dev, ABS_HAT1Y, 0, 1, 0, 0 );

    data->input_dev->name = "light";

    err = input_register_device( data->input_dev );

    if ( err )
    {
      PRINTK_ERR( "tsl2773_probe: Unable to register input device: %s\n", data->input_dev->name );
      goto exit_input_register_device_failed;
    }

    err = misc_register( &tsl_dev_device );
    if ( err )
    {
      PRINTK_ERR( "tsl_dev_device register failed\n" );
      goto sensors_device_register_failed;
    }

	init_suspend();

	//g_sensor_prempt = 0;
	//err = init_proc(tsl_data);
	
	PRINTK( "TSL light sensor is tmd27711  OK!!! " );

    tsl_data = data;
//	start_sensor(data);
	//setPflag(1);
	//setAlsflag(1);
	//printk("huyu-next--aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
  return 0;
exit_input_register_device_failed:
sensors_device_register_failed:
    input_free_device(data->input_dev);
exit_kfree:
exit_input_dev_alloc_failed:
    kfree(data);	
exit:
	return err;	
}

static int __devexit tsl2773_remove( struct i2c_client *client )
{
    struct tsl2773_data *data = i2c_get_clientdata( client );
	PRINTK_IF(PRINT_LOG,"huyu ------%s------\n",__func__);
    input_unregister_device( data->input_dev );
    misc_deregister( &tsl_dev_device );
	deinit_suspend();
    kfree( data );

    return 0;
}

#ifdef CONFIG_PM
static int tsl2773_pm_suspend(struct device *dev)
{
    struct i2c_client *client;
    struct tsl2773_data *data; 

	PRINTK_IF(PRINT_LOG,"huyu ------%s------\n",__func__);
    client = to_i2c_client(dev);
    data= i2c_get_clientdata( client );

    i2c_smbus_write_byte( client, TSL_CMD|TSL_OP_SF|TSL_ISR_APC/*0xe7*/ ); /*clear interrupt*/
    
/* OPPO 2011-03-02 wangjc Add begin for not power down when proximity sensor is used. */
	if(atomic_read( &p_flag ) != 1 )
	{
		PRINTK_IF(PRINT_LOG,"huyu ------%s----prox is not working,powerdown!--\n",__func__);
	    cancel_delayed_work_sync(&data->delay_work);
	    disable_irq(data->client->irq);		
	    if ( data->power_off)
	    {
	    	 data->power_off();
	    }
	}
	else 
	{
	     cancel_delayed_work(&data->prox_work);
	     flag_suspend |= CALL_SUSPEND;

		 
	}
/* OPPO 2011-03-02 wangjc Add end */

    return 0;
}

static int tsl2773_pm_resume(struct device *dev)
{
    struct i2c_client *client;
    struct tsl2773_data *data;
    client = to_i2c_client(dev);
    data= i2c_get_clientdata( client );
	PRINTK_IF(PRINT_LOG,"huyu ------%s------\n",__func__);

/* OPPO 2011-03-02 wangjc Modify begin for reconfig sensor when 
no proximity sensor is used because the power is down when suspend. */
	if(atomic_read( &p_flag ) != 1 )
	{
	
		PRINTK_IF(PRINT_LOG,"huyu --------prox is not working,powerON!--\n");
	    if ( data->power )
	    {
	      	data->power( );
	    }
		enable_irq(data->client->irq);
	}
	else
	{
		i2c_smbus_write_byte( client, TSL_CMD|TSL_OP_SF|TSL_ISR_APC/*0xe7*/ ); /*clear interrupt*/
		PD_sum = 0;
		PD_count = 1;
		schedule_delayed_work( &data->prox_work, msecs_to_jiffies(400));
		PRINTK_IF(PRINT_LOG,"huyu --------flag_suspend = %x--\n", flag_suspend);
		if((flag_suspend & CALL_WAKEUPBYPROX) != 0)
		{
			PRINTK_IF(PRINT_LOG,"huyu --------tsl_interrupt work when ap is down!--\n");
			queue_work(tsl2773_wq, &data->work);
		}
		else if((flag_suspend & CALL_SUSPEND) != 0)
		{
			//press power key to resume (or another call coming)
			PRINTK_IF(PRINT_LOG,"huyu --------reset flag_suspend !--\n");
			flag_suspend = CALL_NORMAL;
		}
	}
/* OPPO 2011-03-02 wangjc Modify end */

    return 0;
}	
#else

static int tsl2773_suspend( struct i2c_client *client, pm_message_t mesg )
{
    struct tsl2773_data *data = i2c_get_clientdata( client );
	PRINTK_IF(PRINT_LOG,"huyu ------%s------\n",__func__);
    atomic_set( &suspend_flag, 1 );
    PRINTK("  ");
    i2c_smbus_write_byte( client, TSL_CMD|TSL_OP_SF|TSL_ISR_APC/*0xe7*/ ); /*clear interrupt*/

/* OPPO 2011-03-02 wangjc Add begin for not power down when proximity sensor is used. */
	if(atomic_read( &p_flag ) != 1 )
	{
	    //if ( data->power )
	    //{
	      /* OPPO Laijl add at 2010-10-12,16:45:04 */
	     // data->power( 0 );
	    //}
	}
/* OPPO 2011-03-02 wangjc Add end */

    return 0;
}

static int tsl2773_resume( struct i2c_client *client )
{
    struct tsl2773_data *data = i2c_get_clientdata( client );
	PRINTK_IF(PRINT_LOG,"huyu ------%s------\n",__func__);

/* OPPO 2011-03-02 wangjc Modify begin for for reconfig sensor when 
no proximity sensor is used because the power is down when suspend.  */
	if(atomic_read( &p_flag ) != 1 )
	{
	    //if ( data->power )
	    //{
	      /* OPPO Laijl add at 2010-10-12,16:45:13 */
	     // data->power( 1 );
	    //}

		if(atomic_read( &als_flag ) == 1 )
	    {
	        start_sensor(data, ALS);
	    }
	}
	else
	{
		i2c_smbus_write_byte( client, TSL_CMD|TSL_OP_SF|TSL_ISR_APC/*0xe7*/ ); /*clear interrupt*/
	}
/* OPPO 2011-03-02 wangjc Modify end */

    atomic_set( &suspend_flag, 0 );
    return 0;
}
#endif


static const struct i2c_device_id tsl2773_id[] = {
	{ "tsl2773", 0 },
	{ }
};
MODULE_DEVICE_TABLE( i2c, tsl2773_id );

#ifdef CONFIG_PM
static struct dev_pm_ops TSL2773_pm = {
	.suspend = tsl2773_pm_suspend,
	.resume  = tsl2773_pm_resume,
};
#endif

static struct i2c_driver tsl2773_driver = {
	.driver = {
		.name	= "tsl2773",
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM
		.pm = &TSL2773_pm,
#endif
	},
#ifndef CONFIG_PM
	.suspend = tsl2773_suspend,
	.resume	= tsl2773_resume,
#endif	
	.probe	= tsl2773_probe,
	.remove	= __devexit_p(tsl2773_remove),
	.id_table = tsl2773_id,
};


static int __init tsl2773_init( void )
{
	tsl2773_wq = create_singlethread_workqueue("tsl2773_wq");
	if (!tsl2773_wq)
	    return -ENOMEM;
	return i2c_add_driver( &tsl2773_driver );
}

static void __exit tsl2773_exit( void )
{
    i2c_del_driver( &tsl2773_driver );
    if (tsl2773_wq)
        destroy_workqueue(tsl2773_wq);
}


MODULE_AUTHOR( "wangjc <wjc@oppo.com>" );
MODULE_DESCRIPTION( "TSL2773 ambient light sensor driver" );
MODULE_LICENSE( "GPL" );
MODULE_VERSION( DRIVER_VERSION );

module_init( tsl2773_init );
module_exit( tsl2773_exit );




