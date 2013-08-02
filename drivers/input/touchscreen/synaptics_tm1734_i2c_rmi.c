/**
 * Copyright 2008-2010 OPPO Mobile Comm Corp., Ltd, All rights reserved.
 * FileName: synamptics_tm1734_i2c_rmi.c
 * ModuleName:
 * Author: tanrf
 * Create Date: 2010-12-06
 * Description:
 * History:
   <version >  <time>  <author>  <desc>
   0.1		20101206    tanrf	modified from synamptics_tm1200_i2c_rmi.c
*/

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/synaptics_i2c_rmi.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <asm/uaccess.h>
#include <linux/leds-pmic8058.h>
#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>


#define PRODUCT_ID_CHAR "TM1734-001"

#define REG_MAP_START		0xDD
#define REG_MAP_END		0xEF
#define REG_MAP_NUM		(REG_MAP_END - REG_MAP_START + 1)

#define F11_QUERY_BASE				0
#define F11_COMMAND_BASE			1
#define F11_CONTROL_BASE			2
#define F11_DATA_BASE				3
#define F11_Interrupt_Source_Count	4
#define F11_Function_Exists			5
#define F01_QUERY_BASE				6
#define F01_COMMAND_BASE			7
#define F01_CONTROL_BASE			8
#define F01_DATA_BASE				9
#define F01_Interrupt_Source_Count	10
#define F01_Function_Exists			11
#define F34_QUERY_BASE				12
#define F34_COMMAND_BASE			13
#define F34_CONTROL_BASE			14
#define F34_DATA_BASE				15
#define F34_Interrupt_Source_Count	16
#define F34_Function_Exists			17
#define PDT_Properties				18

#define REG_Device_Control 		(ts->registers[F01_CONTROL_BASE]+0)	
#define REG_RESET 				(ts->registers[F01_COMMAND_BASE]+0)
#define REG_REZERO 				(ts->registers[F11_COMMAND_BASE]+0)

#define REG_Product_Properties	(ts->registers[F01_QUERY_BASE]+2)
#define REG_Family_Query		(ts->registers[F01_QUERY_BASE]+3)
#define REG_firmware_year		(ts->registers[F01_QUERY_BASE]+4)
#define REG_firmware_month		(ts->registers[F01_QUERY_BASE]+5)
#define REG_firmware_day		(ts->registers[F01_QUERY_BASE]+6)

#define REG_INT_ENABLE			(ts->registers[F01_CONTROL_BASE]+1)	
#define REG_INT_STATUS			(ts->registers[F01_DATA_BASE]+1)
#define REG_DEV_STATUS			(ts->registers[F01_DATA_BASE]+0)
#define REG_RPORT_MODE			(ts->registers[F11_CONTROL_BASE]+0)
#define REG_DELTA_X_THRESHOLD		(ts->registers[F11_CONTROL_BASE]+2)
#define REG_DELTA_Y_THRESHOLD		(ts->registers[F11_CONTROL_BASE]+3)
#define REG_MAX_X_LO			(ts->registers[F11_CONTROL_BASE]+6)
#define REG_MAX_X_HI			(ts->registers[F11_CONTROL_BASE]+7)
#define REG_MAX_Y_LO			(ts->registers[F11_CONTROL_BASE]+8)
#define REG_MAX_Y_HI			(ts->registers[F11_CONTROL_BASE]+9)
#define REG_2D_INT_ENABLE1		(ts->registers[F11_CONTROL_BASE]+10)
#define REG_2D_INT_ENABLE2		(ts->registers[F11_CONTROL_BASE]+11)
#define REG_PID_START			(ts->registers[F01_QUERY_BASE]+11)	
#define REG_2D_Sensitivity 		(ts->registers[F11_CONTROL_BASE]+67)
#define REG_2D_Reporting_Mode	(ts->registers[F11_QUERY_BASE]+1)

/* OPPO 2011-10-20 Van Add registers begin */
#define F34_FLASH_QUERY03				0x0097  		//Block Size 0
#define F34_FLASH_QUERY04				0x0098		//Block Size 1
#define F34_FLASH_QUERY05				0x0099 	 //Firmware Block Count 0
#define F34_FLASH_QUERY06				0x009A 	//Firmware Block Count 1
#define F34_FLASH_QUERY07				0x009B 	//Configuration Block Count 0
#define F34_FLASH_QUERY08				0x009C 	//Configuration Block Count 1
#define F34_FLASH_QUERY00				0x0094   //Bootloader ID 0
#define F34_FLASH_QUERY01				0x0095  //Bootloader ID 1
#define F34_FLASH_DATA02_0			0x0002
#define F34_FLASH_DATA02_1			0x0003
#define F34_FLASH_DATA03				0x0012   //Flash Control

#define F11_2D_CTRL02					0x004F  //2D Delta-X Thresh
#define F11_2D_CTRL03					0x0050  //2D Delta-X Thresh

#define PDT_P00_F11_2D_QUERY_BASE		0x00DD
#define PDT_P00_F11_2D_COMMAND_BASE		0x00DE
#define PDT_P00_F11_2D_CONTROL_BASE		0x00DF
#define PDT_P00_F11_2D_DATA_BASE			0x00E0
#define PDT_P00_F11_2D_INTERRUPTS			0x00E1
#define PDT_P00_F11_2D_EXISTS				0x00E2

#define PDT_P00_F01_RMI_QUERY_BASE		0x00E3
#define PDT_P00_F01_RMI_COMMAND_BASE	0x00E4
#define PDT_P00_F01_RMI_CONTROL_BASE		0x00E5
#define PDT_P00_F01_RMI_DATA_BASE		0x00E6
#define PDT_P00_F01_RMI_INTERRUPTS		0x00E7
#define PDT_P00_F01_RMI_EXISTS				0x00E8

#define PDT_P00_F34_FLASH_QUERY_BASE		0x00E9
#define PDT_P00_F34_FLASH_COMMAND_BASE	0x00EA
#define PDT_P00_F34_FLASH_CONTROL_BASE	0x00EB
#define PDT_P00_F34_FLASH_DATA_BASE		0x00EC
#define PDT_P00_F34_FLASH_INTERRUPTS		0x00ED
#define PDT_P00_F34_FLASH_EXISTS			0x00EE
#define P00_PDT_PROPERTIES					0x00EF

#define F34_FLASH_DATA00				0x0000
#define F34_FLASH_DATA01				0x0001
/* OPPO 2011-10-20 Van Add registers end */


//#define POLLING_TOUCH_PAD
//#define SYNA_1734_SLEEP
#define SYNA_EARLYSUSPEND

//#define SYNA_FW_WAIT_ATTN
//#define SYNA_FW_READ_REG_INT
#define DELAY_IN_SYNA_FW

#define FIRST_FINGER_HOLD 0

#define TS_ATMEL_224E  1
#define TS_SYNA_1734   2

/*virtual key support */
static struct kobject *Syna_properties_kobj;

struct finger {
	int x;
	int y;
	int touch_major;
	int width_major;
	int id;
};

static struct finger fingers[MAX_FINGERS] = {};

static uint8_t Old_finger_state[MAX_FINGERS] = {0};
static uint8_t finger_state[MAX_FINGERS] = {0};

#if FIRST_FINGER_HOLD
static bool bfirst_finger;
static int first_finger_x;
static int first_finger_y;
#endif

/* OPPO 2011-05-27 huanggd Add begin for avoid tp i2c err when system wakeup */
static int is_tp_suspended = 0;
static int Syna_Program_state = 0;
static int Syna_logo_level = 2;
static int Syna_adjust_pixels = 0;

/* OPPO 2011-05-27 huanggd Add end */
struct synaptics_ts_data 
{
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	int use_irq;

	#ifdef POLLING_TOUCH_PAD
	struct hrtimer timer;
	#endif
	//struct work_struct  work;
	struct delayed_work delay_work;
	struct task_struct	*synaptics_update;
	struct completion	synaptics_update_stop;
	uint8_t registers[REG_MAP_NUM];
	uint16_t data_ready;
	uint16_t drop_data;
	uint16_t cmd;
	uint16_t Touch_key_height;
	uint16_t version;
	uint16_t max[2];
	int snap_state[2][2];
	int snap_down_on[2];
	int snap_down_off[2];
	int snap_up_on[2];
	int snap_up_off[2];
	int snap_down[2];
	int snap_up[2];
	uint32_t flags;
	int (*power)(int on);
	struct early_suspend early_suspend;
};

static struct synaptics_ts_data   *syna_ts_data;

static DECLARE_WAIT_QUEUE_HEAD(synaptics_thread_wq);


#ifdef SYNA_EARLYSUSPEND
#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_ts_early_suspend(struct early_suspend *h);
static void synaptics_ts_late_resume(struct early_suspend *h);
#endif
#endif

int release_pointer_handle(struct synaptics_ts_data *ts);


static int Syna_load_fw(struct synaptics_ts_data *ts, const char *fn);
extern int Check_Touchscreen_type(int type);

#define print_ts(level, ...) \
	do { \
		if (Syna_logo_level >= (level)) \
			printk(__VA_ARGS__); \
	} while (0) 


#ifdef SYNA_FW_WAIT_ATTN
static int Read_Irq_ATTN(void)
{
	int rc;
	rc = gpio_get_value(TS_PEN_IRQ_GPIO);
	return rc;
}
#endif
/* OPPO 2011-05-27 huanggd Add begin for avoid tp i2c err when system wakeup */
static void syna_irq_set_locked(struct i2c_client *client, int enable_disable, int in_isr)
{
	static int syna_irq_status = 1;

	if (enable_disable == syna_irq_status) {
		print_ts(DEBUG_TRACE, "%s: irq is configed, status=%d\n", __func__, enable_disable);
		return;
	}

	syna_irq_status = enable_disable;
	
	if (enable_disable) {
		enable_irq(client->irq);
	} else {
		if (in_isr) 
			disable_irq_nosync(client->irq);
		else
			disable_irq(client->irq);
	}
}
/* OPPO 2011-05-27 huanggd Add end */

/*2011-11-08 van add virtual key support start*/
static ssize_t vk_syna_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	switch(syna_ts_data->version)
	{
		case 0x107:
			return sprintf(buf,
		             __stringify(EV_KEY) ":" __stringify(KEY_SEARCH)  ":68:845:80:50"
		         ":" __stringify(EV_KEY) ":" __stringify(KEY_MENU)   ":184:845:80:50"
		         ":" __stringify(EV_KEY) ":" __stringify(KEY_HOME)   ":302:845:80:50"
		         ":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":416:845:80:50"
		         "\n");
			break;
				
		case 0x100:
			return sprintf(buf,
		             __stringify(EV_KEY) ":" __stringify(KEY_MENU)  ":51:800:100:50"
		         ":" __stringify(EV_KEY) ":" __stringify(KEY_HOME)   ":240:800:100:50"
		         ":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":429:800:100:50"
		         "\n");
			break;
			
		default:
			return sprintf(buf,
		             __stringify(EV_KEY) ":" __stringify(KEY_MENU)  ":51:845:100:50"
		         ":" __stringify(EV_KEY) ":" __stringify(KEY_HOME)   ":240:845:100:50"
		         ":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":429:845:100:50"
		         "\n");
			break;
	}
}

static struct kobj_attribute vk_syna_attr = {
	.attr = {
		.name = "virtualkeys.syna_touch",
		.mode = S_IRUGO,
	},
	.show = &vk_syna_show,
};

static struct attribute *syna_properties_attrs[] = {
	&vk_syna_attr.attr,
	NULL
};

static struct attribute_group syna_properties_attr_group = {
	.attrs = syna_properties_attrs,
};
/*2011-11-08 van add virtual key support end*/

#if 0
static int Syna_fw_write(struct i2c_client *client,
			     const u8 *data, unsigned int frame_size)
{
	if (i2c_master_send(client, data, frame_size) != frame_size) {
		print_ts(DEBUG_INFO_ERROR,"%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}
#endif

static int Syna_load_fw(struct synaptics_ts_data *ts, const char *fn)
{
	unsigned int  i, j, block, firmware, configuration;
	unsigned int  data_high8, data_low8;
	unsigned int  ret = 0;

	struct i2c_client *client = ts->client;
	const struct firmware *fw = NULL;
	unsigned int frame_size;
	unsigned int pos = 0;

	#if 1
	ret = request_firmware(&fw, fn, &(ts->client->dev));
	if (ret) {
		print_ts(DEBUG_INFO_ERROR,"Unable to open firmware %s\n", fn);
		return ret;
	}
	else
	{
		print_ts(DEBUG_INFO_TS,"succeed open firmware %s\n", fn);
	}
	#endif

	Syna_Program_state = SYNA_PROGRAM_STATE_STARTED;
	//i2c_smbus_write_byte_data(client, REG_INT_ENABLE, 0x0f);   //enable all interrupts : Analog  Abs0 Status Flash
	
	//Read block size and image file count
	//Block size  
	data_low8  = i2c_smbus_read_byte_data(client, F34_FLASH_QUERY03);	
	data_high8 = i2c_smbus_read_byte_data(client, F34_FLASH_QUERY04);
	block = (data_high8 <<8) + data_low8;
	print_ts(DEBUG_TRACE,"The block size is %d\n", block);

	//Firmware size  
	data_low8  = i2c_smbus_read_byte_data(client, F34_FLASH_QUERY05);	
	data_high8 = i2c_smbus_read_byte_data(client, F34_FLASH_QUERY06);
	firmware = (data_high8 <<8) + data_low8;
	print_ts(DEBUG_TRACE,"The firmware size is %d\n", firmware);

	//Config size
	data_low8  = i2c_smbus_read_byte_data(client, F34_FLASH_QUERY07);	 
	data_high8 = i2c_smbus_read_byte_data(client, F34_FLASH_QUERY08);
	configuration = (data_high8 <<8) + data_low8;
	print_ts(DEBUG_TRACE,"The configuration size is %d\n", configuration);

	// Enter Flash
	//Step1    Read Bootloader ID  
	data_low8  = i2c_smbus_read_byte_data(client, F34_FLASH_QUERY00);	
	data_high8 = i2c_smbus_read_byte_data(client, F34_FLASH_QUERY01);	
	print_ts(DEBUG_TRACE,"data_low8 and data_high8 value is 0x%x , 0x%x\n", data_low8,data_high8);			

	//Step2    Write bootloader ID
	i2c_smbus_write_byte_data(client, F34_FLASH_DATA02_0, data_low8);
	i2c_smbus_write_byte_data(client, F34_FLASH_DATA02_1, data_high8);	
         
	//Step3    Issue program enable
	i2c_smbus_write_byte_data(client, F34_FLASH_DATA03, 0X0F);
	
	//Step4    Wait ATTN	
	#ifdef SYNA_FW_WAIT_ATTN
	while(Read_Irq_ATTN() == 1)		
	{
		print_ts(DEBUG_INFO_TS,"PE waiting irq low\n");
	};	
	#endif

	#ifdef DELAY_IN_SYNA_FW
	mdelay(20);
	#endif

	#ifdef SYNA_FW_READ_REG_INT
	ret = i2c_smbus_read_byte_data(client, REG_INT_STATUS);	
	print_ts(DEBUG_INFO_TS, "The 1 flash interrupt bit is %d\n", ret);
	while((ret & 0x01) != 0x01)
	{
		ret = i2c_smbus_read_byte_data(client, REG_INT_STATUS);	
		print_ts(DEBUG_INFO_TS, "The Enter flash interrupt bit is %d\n", ret);
	}
	#endif

	//Step6  Re-scan PDT
	i2c_smbus_read_byte_data(client, PDT_P00_F34_FLASH_QUERY_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F34_FLASH_COMMAND_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F34_FLASH_CONTROL_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F34_FLASH_DATA_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F34_FLASH_INTERRUPTS);
	i2c_smbus_read_byte_data(client, PDT_P00_F34_FLASH_EXISTS);
	
	i2c_smbus_read_byte_data(client, PDT_P00_F01_RMI_QUERY_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F01_RMI_COMMAND_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F01_RMI_CONTROL_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F01_RMI_DATA_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F01_RMI_INTERRUPTS);
	i2c_smbus_read_byte_data(client, PDT_P00_F01_RMI_EXISTS);
	
	i2c_smbus_read_byte_data(client, PDT_P00_F11_2D_QUERY_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F11_2D_COMMAND_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F11_2D_CONTROL_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F11_2D_DATA_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F11_2D_INTERRUPTS);
	i2c_smbus_read_byte_data(client, PDT_P00_F11_2D_EXISTS);


	//Step5    Check Status, the value should be 0x80
	ret = i2c_smbus_read_byte_data(client, F34_FLASH_DATA03);	
	print_ts(DEBUG_TRACE, "The status(Enter Flash) of Flash Data3 is %x\n", ret);
	
	// Program         
	//Step1    Read bootloader ID
	//data_low8  = i2c_smbus_read_byte_data(client, F34_FLASH_QUERY03);	
	//data_high8 = i2c_smbus_read_byte_data(client, F34_FLASH_QUERY04);	
	print_ts(DEBUG_TRACE, "Erase data_low8 and data_high8 value is 0x%x , 0x%x\n", 
	i2c_smbus_read_byte_data(client, F34_FLASH_QUERY03),
	i2c_smbus_read_byte_data(client, F34_FLASH_QUERY04));			
	
	//Step2    Write bootloader ID       
	i2c_smbus_write_byte_data(client, F34_FLASH_DATA02_0, data_low8);
	i2c_smbus_write_byte_data(client, F34_FLASH_DATA02_1, data_high8);	
	
	//Step3    Erase                  
	print_ts(DEBUG_INFO_TS, "Erase\n");	
	i2c_smbus_write_byte_data(client, F34_FLASH_DATA03, 0X03);

	//Step4    Wait ATTN 	
	#ifdef SYNA_FW_WAIT_ATTN
	while(Read_Irq_ATTN() == 1)		
	{
		print_ts(DEBUG_INFO_TS,"Erase waiting irq low\n");
	};	
	#endif

	#ifdef DELAY_IN_SYNA_FW
	mdelay(1000);
	#endif


	#ifdef SYNA_FW_READ_REG_INT
	ret = i2c_smbus_read_byte_data(client, REG_INT_STATUS);	
	print_ts(DEBUG_INFO_TS, "The 2 flash interrupt bit is %d\n", ret);
	while((ret & 0x01) != 0x01)
	{
		ret = i2c_smbus_read_byte_data(client, REG_INT_STATUS);	
		print_ts(DEBUG_INFO_TS, "The Program flash interrupt bit is %d\n", ret);
	}
	#endif	

	//Step5    Check Status, the value should be 0x80
	ret = i2c_smbus_read_byte_data(client, F34_FLASH_DATA03);	
	print_ts(DEBUG_TRACE, "The status(Program) of Flash Data3 is %x\n", ret);

	////////////////////
	//return 0;
	////////////////////

	frame_size = 16;
	pos = pos + 0x100; //firmware start from offset 0x100
	Syna_Program_state = SYNA_PROGRAM_STATE_WRITING;
	
//Step6  Image area
	for(j = 0; j < firmware; j++)
	{
		// a)      Wrtie Block Number
		i2c_smbus_write_byte_data(client, F34_FLASH_DATA00, (j & 0x00ff));
		i2c_smbus_write_byte_data(client, F34_FLASH_DATA01, ((j & 0xff00)>>8));	
		
		//b)       Write Data
		#if 1
		for(i = 0; i < block; i++) 
		{
			i2c_smbus_write_byte_data(client, (0x02 + i), *(fw->data + pos));
			pos++;
		}
		#else
		Syna_fw_write(client, fw->data+ pos, frame_size);
		pos += frame_size;
		#endif

		//c)       Issue write 
		i2c_smbus_write_byte_data(client, F34_FLASH_DATA03, 0X02);
		
		//d)       Wait ATTN
		#ifdef SYNA_FW_WAIT_ATTN
		while(Read_Irq_ATTN() == 1)		
		{
			print_ts(DEBUG_INFO_TS,"Image block waiting irq low\n");
		};	
		#endif

		#ifdef DELAY_IN_SYNA_FW
		mdelay(10);
		#endif


		#ifdef SYNA_FW_READ_REG_INT
		ret = i2c_smbus_read_byte_data(client, REG_INT_STATUS);	
		while((ret & 0x01) != 0x01)
		{
			ret = i2c_smbus_read_byte_data(client, REG_INT_STATUS);	
			print_ts(DEBUG_INFO_TS, "The Image flash interrupt bit is %d\n", ret);
		}
		#endif

		
		if(pos%2000 == 0)
		{
			ret = i2c_smbus_read_byte_data(client, F34_FLASH_DATA03);	
			print_ts(DEBUG_INFO_TS, "The status(Image) of Flash Data3 is 0x %x, pos = %d\n", ret, pos);		
		}
	}    
	
	//Step7 Confiure data
	for(j = 0; j < configuration; j++)
	{          	         
		// a)
		i2c_smbus_write_byte_data(client, F34_FLASH_DATA00, (j & 0x00ff));
		i2c_smbus_write_byte_data(client, F34_FLASH_DATA01, ((j & 0xff00)>>8));	
		
		//b)     
		for(i = 0; i < block; i++) 
		{
			i2c_smbus_write_byte_data(client, (0x02 + i), *(fw->data + pos));
			pos++;
		}
		
		//c)    
		i2c_smbus_write_byte_data(client, F34_FLASH_DATA03, 0X06);

		//d)   
		#ifdef SYNA_FW_WAIT_ATTN
		while(Read_Irq_ATTN() == 1)		
		{
			print_ts(DEBUG_INFO_TS,"Confiure block waiting irq low\n");
		};	
		#endif

		#ifdef DELAY_IN_SYNA_FW
		mdelay(10);
		#endif

		#ifdef SYNA_FW_READ_REG_INT
		ret = i2c_smbus_read_byte_data(client, REG_INT_STATUS);	
		while((ret & 0x01) != 0x01)
		{
			ret = i2c_smbus_read_byte_data(client, REG_INT_STATUS);	
			print_ts(DEBUG_INFO_TS, "The Confiure flash interrupt bit is %d\n", ret);
		}
		#endif

		if(pos%1000 == 0)
		{
			ret = i2c_smbus_read_byte_data(client, F34_FLASH_DATA03);	
			print_ts(DEBUG_INFO_TS, "The status(Confiure) of Flash Data3 is 0x %x, pos = %d\n", ret, pos);	
		}
	}     

	//Disable
	//Step1    Issue Reset
	i2c_smbus_write_byte_data(client, REG_RESET, 0x01);
	
	//Step2    Wait ATTN
	#ifdef SYNA_FW_WAIT_ATTN
	while(Read_Irq_ATTN() == 1)		
	{
		print_ts(DEBUG_INFO_TS,"Reset waiting irq low\n");
	};	
	#endif

	#ifdef DELAY_IN_SYNA_FW
	mdelay(1000);
	#endif

	#ifdef SYNA_FW_READ_REG_INT
	ret = i2c_smbus_read_byte_data(client, REG_INT_STATUS);	
	print_ts(DEBUG_INFO_TS, "The 3 flash interrupt bit is %d\n", ret);
	while((ret & 0x01) != 0x01)
	{
		ret = i2c_smbus_read_byte_data(client, REG_INT_STATUS);	
		print_ts(DEBUG_INFO_TS, "The Disable flash interrupt bit is %d\n", ret);
	}
	#endif

	i2c_smbus_read_byte_data(client, PDT_P00_F34_FLASH_QUERY_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F34_FLASH_COMMAND_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F34_FLASH_CONTROL_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F34_FLASH_DATA_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F34_FLASH_INTERRUPTS);
	i2c_smbus_read_byte_data(client, PDT_P00_F34_FLASH_EXISTS);
	
	i2c_smbus_read_byte_data(client, PDT_P00_F01_RMI_QUERY_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F01_RMI_COMMAND_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F01_RMI_CONTROL_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F01_RMI_DATA_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F01_RMI_INTERRUPTS);
	i2c_smbus_read_byte_data(client, PDT_P00_F01_RMI_EXISTS);
	
	i2c_smbus_read_byte_data(client, PDT_P00_F11_2D_QUERY_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F11_2D_COMMAND_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F11_2D_CONTROL_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F11_2D_DATA_BASE);
	i2c_smbus_read_byte_data(client, PDT_P00_F11_2D_INTERRUPTS);
	i2c_smbus_read_byte_data(client, PDT_P00_F11_2D_EXISTS);

	//i2c_smbus_write_byte_data(client, REG_INT_ENABLE, 0x04);   //enable all interrupts  Analog  Abs0 Status Flash
	
	//Step3    Check status	
	ret = i2c_smbus_read_byte_data(client, F34_FLASH_DATA03);	
	print_ts(DEBUG_TRACE, "The status(Disable) of Flash Data3 is %x\n", ret);	

	if((ret & 0x0f) == 0)
	{
		ret = i2c_smbus_read_byte_data(client, REG_DEV_STATUS);	
		print_ts(DEBUG_TRACE, "The status of Device status is %x\n", ret);	

		if((ret & 0x40) == 0)
		{
			print_ts(DEBUG_INFO_TS, "The Fw update successful ! \n");
			Syna_Program_state = SYNA_PROGRAM_STATE_SUCCEED;
			release_firmware(fw);
			return 0;
		}
		else
		{
			print_ts(DEBUG_INFO_TS, "The Fw update fail !\n");
			Syna_Program_state = SYNA_PROGRAM_STATE_FAILED;
		}
	}
	else
	{
		print_ts(DEBUG_INFO_TS, "The Fw update fail !\n");
		Syna_Program_state = SYNA_PROGRAM_STATE_FAILED;
	}

//out:
	release_firmware(fw);

	return ret;
}



static int synaptics_init_panel(struct synaptics_ts_data *ts)
{
	int ret = 0;

	ret = i2c_smbus_write_byte_data(ts->client, PAGE_SELECT, 0x00);
	/* OPPO 2011-04-22 zhangqiang Add begin for making the ts to run at full power */
	ret |= i2c_smbus_write_byte_data(ts->client, REG_Device_Control, 0x84); 
	/* OPPO 2011-04-22 zhangqiang Add end */

	#if 1
	ret |= i2c_smbus_write_byte_data(ts->client, REG_RPORT_MODE, 0x01);

	ret |= i2c_smbus_write_byte_data(ts->client, REG_2D_Sensitivity, 0x0f); //the 2D Sensitivity 

	ret |= i2c_smbus_write_byte_data(ts->client, REG_DELTA_X_THRESHOLD, 0x02);   //Delta X
	ret |= i2c_smbus_write_byte_data(ts->client, REG_DELTA_Y_THRESHOLD, 0x02);   //Delta Y
	#endif
	
	//ret |= i2c_smbus_write_byte_data(ts->client, REG_2D_INT_ENABLE1, 0x00);

	if (ret < 0)
		print_ts(DEBUG_INFO_ERROR, "synaptics_init_panel: i2c_smbus_write_byte_data failed\n");

	return ret;
}


static int synaptics_set_int_mask(struct synaptics_ts_data *ts, int enable)
{
	uint8_t int_msk;
	int ret;

	i2c_smbus_write_byte_data(ts->client, PAGE_SELECT, 0x00);

	if(enable) 
	{
		int_msk = 0x07;

		ret = i2c_smbus_read_byte_data(ts->client, REG_INT_STATUS);  //clear the interrupt now
		if(ret < 0) 
		{
			print_ts(DEBUG_INFO_ERROR, "synaptics_set_int_mask :clear interrupt bits failed\n");
		}
	} 
	else 
	{
		int_msk = 0x00;
	}	

	ret = i2c_smbus_write_byte_data(ts->client, REG_INT_ENABLE, int_msk);
	if(ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR, "synaptics_set_int_mask: enable or disable abs interrupt failed, int_msk = 0x%x\n",int_msk);
	}
	return ret;	
}

static int synaptics_get_register(struct synaptics_ts_data *ts)
{
	uint8_t buf0[2];
	struct i2c_msg msg[2];
	int ret;
	
	msg[0].addr = ts->client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = buf0;
	buf0[0] = REG_MAP_START;
	msg[1].addr = ts->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = REG_MAP_NUM;
	msg[1].buf = ts->registers;
	
	ret = i2c_transfer(ts->client->adapter, msg, 2);
	if (ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR, "synaptics read reg map info falied\n");
	}

	return ret;
}

static int synaptics_detetct_product(struct synaptics_ts_data *ts)
{
	uint8_t buf0[2];
	uint8_t buf1[strlen(PRODUCT_ID_CHAR)+1];
	struct i2c_msg msg[2];
	int ret ;

	memset(buf1,0 , sizeof(buf1));
	msg[0].addr = ts->client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = buf0;
	buf0[0] = REG_PID_START;
	msg[1].addr = ts->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = strlen(PRODUCT_ID_CHAR);
	msg[1].buf = buf1;
	
	ret = i2c_transfer(ts->client->adapter, msg, 2);
	if (ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR, "synaptics_read_product_id: failed to read product info\n");
	}
	
	print_ts(DEBUG_INFO_TS, "synaptics product id: %s \n",buf1);

	return ret;
}

static void Refresh_Touchscreen_param(struct synaptics_ts_data *ts)
{
	int ret = 0;
	uint16_t max_x, max_y;

	synaptics_get_register(ts);
	
	ret = i2c_smbus_read_byte_data(ts->client, REG_Product_Properties);
	if (ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR, "i2c_smbus_write_byte_data failed, line=%d\n", __LINE__);
	}
	ts->version = ret << 8;
	
	ret = i2c_smbus_read_byte_data(ts->client, REG_Family_Query);
	if (ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR, "i2c_smbus_write_byte_data failed, line=%d\n", __LINE__);
	}
	ts->version |= ret;
	
	//read product id 
	synaptics_detetct_product(ts);

	//read max_x
	ret = i2c_smbus_read_byte_data(ts->client, REG_MAX_X_HI);
	if (ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR, "i2c_smbus_write_byte_data failed, line=%d\n", __LINE__);
	}
	max_x = (ret&0x0f) << 8;
	
	ret = i2c_smbus_read_byte_data(ts->client, REG_MAX_X_LO);
	if (ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR, "i2c_smbus_write_byte_data failed, line=%d\n", __LINE__);
	}
	max_x |= (ret&0xff);	
	ts->max[0] = max_x;
	print_ts(DEBUG_TRACE, "synatics_ts_probe: max_x=0x%x\n", max_x);
	
	//read max_y
	ret = i2c_smbus_read_byte_data(ts->client, REG_MAX_Y_HI);
	if (ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR, "i2c_smbus_write_byte_data failed, line=%d\n", __LINE__);
	}
	max_y = (ret&0x0f) << 8;

	ret = i2c_smbus_read_byte_data(ts->client, REG_MAX_Y_LO);
	if (ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR, "i2c_smbus_write_byte_data failed, line=%d\n", __LINE__);
	}
	max_y |= (ret&0xff);	
	ts->max[1] = max_y;
	print_ts(DEBUG_TRACE, "synatics_ts_probe: max_y=0x%x\n", max_y);

	//swap x and y if needed
	if (ts->flags & SYNAPTICS_SWAP_XY)
		swap(max_x, max_y);


	switch(ts->version)
	{
		case 0x107:
			 ts->Touch_key_height = 170;
			break;
				
		case 0x100:
			ts->Touch_key_height = 2;
			break;

		default:
		 	ts->Touch_key_height = 140;
			break;
	}

}

static void Refresh_input_param(struct synaptics_ts_data *ts)
{
	uint16_t max_x, max_y;
	max_x = ts->max[0];
	max_y = ts->max[1];
	
	/* Single touch */
	#if 1
	input_set_abs_params(ts->input_dev, ABS_X, 0, max_x + 1, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_Y, 0, max_y- ts->Touch_key_height, 0, 0);
	#endif
	/* Multitouch */
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, max_x + 1, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, max_y- ts->Touch_key_height, 0, 0);

}

#ifdef POLLING_TOUCH_PAD
static enum hrtimer_restart synaptics_ts_timer_func(struct hrtimer *timer)
{
	struct synaptics_ts_data *ts = container_of(timer, struct synaptics_ts_data, timer);

	ts->data_ready = 1;
	wake_up(&synaptics_thread_wq);
	hrtimer_start(&ts->timer, ktime_set(0, 12500000), HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}
#endif

static void synaptics_hardware_reset(struct synaptics_ts_data *ts)
{
	int ret = 0;

	print_ts(DEBUG_INFO_TS, "synaptics do hardware reset\n");
	/*if hardware reset is used, hold the I2C bus to avoid I2C err*/
	rt_mutex_lock(&ts->client->adapter->bus_lock);
	if(ts->power)
		ret = ts->power(0xEF);//i2c_smbus_write_byte_data(ts->client, REG_RESET, 0x01);
		
	if(ret < 0)
		print_ts(DEBUG_INFO_ERROR, "synaptics reset failed!\n");
	
	msleep(200);
	rt_mutex_unlock(&ts->client->adapter->bus_lock);

}

static int synaptics_software_reset(struct synaptics_ts_data *ts)
{
	int ret;	
	ret = i2c_smbus_write_byte_data(ts->client, REG_RESET, 0x01);
	if (ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR,  "synaptics_software_reset failed, line=%d\n", __LINE__);
	}
	
	return ret;
}

static irqreturn_t synaptics_ts_irq_handler(int irq, void *dev_id)
{
	struct synaptics_ts_data *ts = dev_id;

	print_ts(DEBUG_TRACE, "synaptics_ts_irq_handler\n");

/* OPPO 2011-05-27 huanggd Modify begin for avoid tp i2c err when system wakeup */
	if (is_tp_suspended) {
		print_ts(DEBUG_TRACE, "synaptics_ts_irq_handler suspended\n");
		return IRQ_HANDLED;
	}

	//disable_irq_nosync(ts->client->irq);
	syna_irq_set_locked(ts->client, 0, 1);
/* OPPO 2011-05-27 huanggd Modify end */
	ts->data_ready = 1;
	wake_up(&synaptics_thread_wq);
	
	return IRQ_HANDLED;
}

/***2011-4-30 OPPO sunjianbo add reset interface****/
static void synaptics_ts_delay_work(struct work_struct *work)
{
	struct synaptics_ts_data *ts = \
		container_of(work, struct synaptics_ts_data, delay_work.work);
	int ret;
	if(ts->cmd == SYNA_TS_CMD_RESET) {
		/*clear the touchpanel data*/
		input_mt_sync(ts->input_dev);
		input_sync(ts->input_dev);
		
		ret = i2c_smbus_write_byte_data(ts->client, REG_RESET, 0x01);
		if(ret < 0)
			print_ts(DEBUG_INFO_ERROR, "synaptics reset failed!\n");
		/*do delayed init*/
		ts->cmd =SYNA_TS_CMD_INIT;
		schedule_delayed_work(&ts->delay_work, msecs_to_jiffies(6000));
	} else if( ts->cmd == SYNA_TS_CMD_INIT) {
		print_ts(DEBUG_TRACE, "synaptics do reinit now!\n");
		synaptics_init_panel(ts);
		synaptics_set_int_mask( ts, 1);
	} else if( ts->cmd == SYNA_TS_CMD_RESUME_200MS) {
		print_ts(DEBUG_TRACE, "synaptics clear drop_data 200ms after later resume\n");
		ts->drop_data = 0;
		ts->cmd = 4;
		release_pointer_handle(ts);		
		schedule_delayed_work(&ts->delay_work, msecs_to_jiffies(3000));
	}else if( ts->cmd == 4) {
		ts->cmd = SYNA_TS_CMD_INIT;
		print_ts(DEBUG_TRACE, "synaptics reset cmd aviliable\n");
		schedule_delayed_work(&ts->delay_work, msecs_to_jiffies(3000));
	}
	
}

ssize_t synaptics_proc_write( struct file *filp, const char __user *buff,
                        unsigned long len, void *data )
{
	int ret;
	char proc_syna_data[32];  
	
	struct synaptics_ts_data *ts = data;
	if (copy_from_user( &proc_syna_data, buff, len )) {
		print_ts(DEBUG_INFO_ERROR, "synaptics read reset data error.\n");
		return -EFAULT;
	}

	/*skip the first reset cmd*/
	if(ts->cmd >= 3) {
		ts->drop_data = 0;
		print_ts(DEBUG_INFO_TS, "synaptics cmd pending\n");
		return len;
	}

	if(proc_syna_data[0] == 'L') 
	{
		Syna_adjust_pixels = (proc_syna_data[1]-0x30)*10 + (proc_syna_data[2]-0x30); 

		print_ts(DEBUG_INFO_TS, "syna adjust up %d pixels\n", Syna_adjust_pixels);
	}

	if (proc_syna_data[0] == '1') {
		print_ts(DEBUG_TRACE, "synaptics reset now!\n");
		ts->cmd = SYNA_TS_CMD_RESET;
		cancel_delayed_work_sync(&ts->delay_work);
		schedule_delayed_work(&ts->delay_work, msecs_to_jiffies(20));
	} 
	else if(proc_syna_data[0] == '2') 
	{
		print_ts(DEBUG_INFO_TS, "synaptics start rezero !\n");
		cancel_delayed_work_sync(&ts->delay_work);
		ret = i2c_smbus_write_byte_data(ts->client, REG_REZERO, 0x01);
		if(ret < 0)
			print_ts(DEBUG_INFO_ERROR, "synaptics rezero failed!\n");		
	} 
	else if(proc_syna_data[0] == '3') 
	{
		if(Syna_logo_level != 3) 
		{
			print_ts(DEBUG_INFO_TS, "synaptics change logo_level to 3!\n");
			print_ts(DEBUG_INFO_TS, "synaptics_ts_probe: Product  Version 0x%x\n", ts->version);

			print_ts(DEBUG_INFO_TS, "synaptics_ts_probe: firmware year =%d, month =%d , data =%d\n",
				i2c_smbus_read_byte_data(ts->client, REG_firmware_year),
				i2c_smbus_read_byte_data(ts->client, REG_firmware_month),
				i2c_smbus_read_byte_data(ts->client, REG_firmware_day)
			);
			print_ts(DEBUG_INFO_TS, "max_x = %d, max_y = %d\n", ts->max[0], ts->max[1]);

			for(ret = 0; ret <REG_MAP_NUM; ret++)
				print_ts(DEBUG_INFO_TS, "reg %d is 0x%x\n",ret,ts->registers[ret]);
			
			ret = i2c_smbus_read_byte_data(ts->client, REG_DEV_STATUS);
			print_ts(DEBUG_INFO_TS, "REG_DEV_STATUS 0x%x\n", ret);
			ret = i2c_smbus_read_byte_data(ts->client, REG_Device_Control);
			print_ts(DEBUG_INFO_TS, "REG_Device_Control 0x%x\n", ret);
			ret = i2c_smbus_read_byte_data(ts->client, REG_INT_ENABLE);
			print_ts(DEBUG_INFO_TS, "REG_INT_ENABLE 0x%x\n", ret);
			ret = i2c_smbus_read_byte_data(ts->client, REG_2D_INT_ENABLE1);
			print_ts(DEBUG_INFO_TS, "REG_2D_INT_ENABLE1 0x%x\n", ret);

			print_ts(DEBUG_INFO_TS, "synaptics_init_panel: Report Mode =%d, Delta X =%d , Delta Y =%d, Sensitivity = %d, F11_2D_Q01 = %d\n",
				i2c_smbus_read_byte_data(ts->client, 0x4d),
				i2c_smbus_read_byte_data(ts->client, 0X4f),
				i2c_smbus_read_byte_data(ts->client, 0x50),
				i2c_smbus_read_byte_data(ts->client, 0x90),
				i2c_smbus_read_byte_data(ts->client, 0xB3)
			);
			Syna_logo_level = 3;
		}
		else
		{
			print_ts(DEBUG_INFO_TS, "synaptics change logo_level to 2!\n");
			Syna_logo_level = 2;
		}
	} 
	else if(proc_syna_data[0] == '4') 
	{
		ret = Syna_load_fw(ts, SYNA_FW_0_NAME);
		if (ret) 
		{
			print_ts(DEBUG_INFO_ERROR, "firmware %s update failed(%d)\n", SYNA_FW_0_NAME, ret);
		} 
		Refresh_Touchscreen_param(ts);
		Refresh_input_param(ts);
	}
	else if(proc_syna_data[0] == '5') 
	{
		ret = Syna_load_fw(ts, SYNA_FW_1_NAME);
		if (ret) 
		{
			print_ts(DEBUG_INFO_ERROR, "firmware %s update failed(%d)\n", SYNA_FW_1_NAME, ret);
		} 
		Refresh_Touchscreen_param(ts);
		Refresh_input_param(ts);
	}
	else if(proc_syna_data[0] == '6') 
	{
		ret = Syna_load_fw(ts, SYNA_FW_2_NAME);
		if (ret) 
		{
			print_ts(DEBUG_INFO_ERROR, "firmware %s update failed(%d)\n", SYNA_FW_2_NAME, ret);
		} 
		Refresh_Touchscreen_param(ts);
		Refresh_input_param(ts);
	}
	
	return len;
}


static struct proc_dir_entry *proc_entry;

int init_synaptics_proc(struct synaptics_ts_data *ts)
{
	int ret=0;
	
	proc_entry = create_proc_entry( "syna_proc_write", 0666, NULL );
	proc_entry->data = ts;
	
	if (proc_entry == NULL) {
		ret = -ENOMEM;
	  	print_ts(DEBUG_INFO_ERROR, "init_synaptics_proc: Couldn't create proc entry\n");
	} else {
		proc_entry->write_proc = synaptics_proc_write;
		//proc_entry->owner = THIS_MODULE;
	}
  
	return ret;
}


#if FIRST_FINGER_HOLD
static bool Check_first_finger(void)
{
	int i;
	int old_fingers = 0, current_fingers = 0;

	for(i = 0; i < MAX_FINGERS; i++) {
		if(Old_finger_state[i])
			old_fingers++;
	}

	for(i = 0; i < MAX_FINGERS; i++) {
		if(finger_state[i])
			current_fingers++;
	}

	if(old_fingers==0 && current_fingers==1)
	{
		first_finger_x = fingers[0].x; //第一个点是finger 0，注意某些报report ID的触摸屏可能不是
		first_finger_y = fingers[0].y;
		
		print_ts(DEBUG_TRACE, "first_finger detected\n");
		return true;
	}

	if(bfirst_finger && current_fingers==1)
	{
		print_ts(DEBUG_TRACE, "first_finger keeping\n");
		return true;
	}
	else
	{
		return 0;
	}
}
#endif

static bool Check_finger_valid(int finger)  //如果finger_state 和Old_finger_state都是0，说明该点已弹起
{
	if(finger_state[finger] || Old_finger_state[finger])
		return true;
	else
		return 0;
}

static void Pre_Process_fingers(uint8_t *buf, struct synaptics_ts_data *ts)
{
	uint16_t f0_x=0,f0_y=0,f0_z=0;
	uint8_t f0_wx=0,f0_wy=0;
	int i, index, data_offset;
	int finger_state_bit;
	
	data_offset = MAX_FINGERS / 4 + 2;
	index = data_offset;

	for(i = 0; i < MAX_FINGERS; i++) {
		
		finger_state_bit = (i%4)*2;		
		finger_state[i] = (buf[1 + i/4] & (0x03<<finger_state_bit))>>finger_state_bit;

		/*if(!Check_finger_valid(i))
	       {
	       	  index += FINGER_Package;
	                continue;
	       }*/
			
		f0_x = (buf[index + 0]<<4) | (buf[index + 2]&0x0f);
		f0_y = (buf[index + 1]<<4) | ((buf[index + 02]&0xf0)>>4);
		f0_z = buf[index + 4];
		f0_wx = (buf[index + 3] & 0x0f);
		f0_wy = ((buf[index + 3]&0xf0) >>4);

		fingers[i].x = f0_x;
		fingers[i].y = f0_y;
		fingers[i].touch_major = f0_z;
		fingers[i].width_major = (f0_wx+ f0_wy) / 2;
		fingers[i].id = i;

		if(Syna_adjust_pixels && fingers[i].y> Syna_adjust_pixels)
			fingers[i].y = fingers[i].y - Syna_adjust_pixels;
		
		index += FINGER_Package;
	}
}

static int synaptics_ts_thread(void *data)
{
	struct synaptics_ts_data *ts = data;
	struct i2c_msg msg[2];
	struct sched_param param = { .sched_priority = 1 };
	
	uint8_t start_reg;
	uint8_t *buf; 
	int i, ret, data_offset, data_size;
	uint8_t  hardware_reset = 0;

	data_offset = MAX_FINGERS / 4 + 2;
	data_size = sizeof(uint8_t)*(MAX_FINGERS *5 + data_offset + 5);
	
	buf = kmalloc(data_size, GFP_KERNEL);

	if (!buf) {
		print_ts(DEBUG_INFO_ERROR, "%s: out of memory\n", __func__);
		return 0;
	}

	print_ts(DEBUG_TRACE, "%s: %d bytes of memory ready!\n", __func__,data_size);
	
	sched_setscheduler(current, SCHED_RR, &param);

	
	while (!kthread_should_stop()) {
		
		set_current_state(TASK_INTERRUPTIBLE);
		
		if(is_tp_suspended) {
			wait_event_interruptible(synaptics_thread_wq,ts->data_ready);
		} else {
			ret = wait_event_interruptible_timeout(synaptics_thread_wq,ts->data_ready, HZ*3);

			if(kthread_should_stop())
				break;

			if (is_tp_suspended) {
				print_ts(DEBUG_TRACE, "%s: tp is suspended\n", __func__);
				continue;
			}
			
			if(ret == 0) {

				if(hardware_reset) {
					synaptics_hardware_reset(ts);
					hardware_reset = 1;
					continue;
				}
				
				ret = i2c_smbus_read_word_data(ts->client, REG_DEV_STATUS);
				if (ret < 0) {
					print_ts(DEBUG_INFO_ERROR, "synaptics check dev failed(time out)\n");
					hardware_reset++;
				}
				
				if ( ret & 0x0200) {
					if((ret & 0x0F)  == 0x03) {
						print_ts(DEBUG_INFO_ERROR, "synaptics Device failture detect(time out)\n");
						if(hardware_reset)
							synaptics_hardware_reset(ts);
						else
							synaptics_software_reset(ts);
						hardware_reset = 1;	
					} else if((ret & 0x0F)  == 0x01 ||(ret & 0x80)) {
						print_ts(DEBUG_TRACE, "synaptics need init, init now!(time out)\n");
						ret = synaptics_init_panel(ts);
						if(ret < 0) {
							print_ts(DEBUG_INFO_ERROR, "synaptics do inint failed(time out)\n");
							continue;
						}
						synaptics_set_int_mask( ts, 1);
						hardware_reset = 0;
					} else {
						print_ts(DEBUG_INFO_ERROR, "synaptics unknow status 0x%x(time out)\n",ret);
					}
				}
				continue;
			}
		}
		
		if(kthread_should_stop())
			break;
		
		ts->data_ready = 0;
		
		/* OPPO 2011-05-27 huanggd Add begin for avoid tp i2c err when system wakeup */
		if (is_tp_suspended) {
			print_ts(DEBUG_TRACE, "%s: tp is suspended.  do nothing!!!\n\n", __func__);
			continue;
		}
		/* OPPO 2011-05-27 huanggd Add end */
		
		//memset(buf, 0, sizeof(buf));
		start_reg = REG_INT_STATUS;	//reg 0x14~0x1f
		
		msg[0].addr = ts->client->addr;
		msg[0].flags = 0;
		msg[0].len = 1;
		msg[0].buf = &start_reg;
		msg[1].addr = ts->client->addr;
		msg[1].flags = I2C_M_RD;
		msg[1].len = data_size;
		msg[1].buf = buf;

		ret = i2c_transfer(ts->client->adapter, msg, 2);
		if (ret < 0) 
		{
			print_ts(DEBUG_INFO_ERROR, "synaptics_ts_work_func: i2c_transfer failed\n");
			hardware_reset++;
			if(hardware_reset >10) {
				print_ts(DEBUG_INFO_TS, "synaptics th do hardware reset forced\n");
				synaptics_hardware_reset(ts);
				hardware_reset = 1;
				goto end;
			}
		} else  if( buf[0] & 0x02) {
			ret = i2c_smbus_read_byte_data(ts->client, REG_DEV_STATUS);
			if (ret < 0) 
			{
				print_ts(DEBUG_INFO_ERROR, "synaptics check dev status failed.\n");
				hardware_reset++;
			}
			if((ret & 0x0F)  == 0x03) {
				print_ts(DEBUG_INFO_ERROR, "synaptics Device failture detect\n");
				if(hardware_reset)
					synaptics_hardware_reset(ts);
				else
					synaptics_software_reset(ts);
				hardware_reset = 1;
			} else if((ret & 0x0F)  == 0x01 || (ret & 0x80)){
				print_ts(DEBUG_TRACE, "synaptics need init, init now!\n");
				ret = synaptics_init_panel(ts);
				if (ret < 0) {
					print_ts(DEBUG_INFO_ERROR, "synaptics do init failed\n");
					hardware_reset++;
					goto end;
				}
				synaptics_set_int_mask( ts, 1);
				hardware_reset = 0;
			} else {
				print_ts(DEBUG_INFO_ERROR, "synaptics unknow status 0x%x\n",ret);
			}
		} else if (buf[0] & 0x04) {

			if(ts->drop_data) {
				/*OPPO 2011-6-3 sunjianbo 
				* while touchpanel resume, may be some 
				* noise data received ,drop it in 200ms(van add)
				*/
				ts->drop_data --;
				//print_ts(DEBUG_TRACE, "synaptics drop data\n");
				goto end;
			}
					
			hardware_reset = 0;

			//预处理
			Pre_Process_fingers(buf, ts);

			#if FIRST_FINGER_HOLD
			bfirst_finger = Check_first_finger();

			if(bfirst_finger)
			{
					if(abs(fingers[0].x-first_finger_x) < FIRST_FINGER_HOLD && abs(fingers[0].y-first_finger_y) < FIRST_FINGER_HOLD)
					{
		                    	      input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID,  fingers[0].id);
			                    input_report_abs(ts->input_dev, ABS_MT_POSITION_X, first_finger_x);
			                    input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, first_finger_y);
			                    input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, fingers[0].touch_major);
					      input_report_abs(ts->input_dev,ABS_MT_WIDTH_MAJOR, fingers[0].width_major);
					      input_mt_sync(ts->input_dev);
					      input_sync(ts->input_dev);
					      Old_finger_state[0] = finger_state[0];
					      goto end;
					}
					else
						bfirst_finger = 0;
			}
			#endif

			for(i = 0; i < MAX_FINGERS; i++) {

				if(!Check_finger_valid(i))
	                    {
	                        continue;
	                    }

	                    if(fingers[i].touch_major == 0)
	                    {
	                    	      input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, fingers[i].id);
		                    input_report_abs(ts->input_dev, ABS_MT_POSITION_X, fingers[i].x);
		                    input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, fingers[i].y);
		                    input_report_abs(ts->input_dev, ABS_MT_PRESSURE, 0);
							input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
					
					print_ts(DEBUG_TRACE, "up x=%d, y=%d\n,  up", fingers[i].x, fingers[i].y);
	                    	}
	                    else
	                    {
	                    	      input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID,  fingers[i].id);
		                    input_report_abs(ts->input_dev, ABS_MT_POSITION_X, fingers[i].x);
		                    input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, fingers[i].y);
		                    input_report_abs(ts->input_dev, ABS_MT_PRESSURE, fingers[i].touch_major);
							input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, fingers[i].touch_major);
							input_report_abs(ts->input_dev,ABS_MT_WIDTH_MAJOR, fingers[i].width_major);
				
					print_ts(DEBUG_TRACE, "dn x=%d, y=%d\n  down", fingers[i].x, fingers[i].y);
					print_ts(DEBUG_TRACE, "dn id=%d,  x=%d, y=%d, tm=%d, wm=%d\n",  fingers[i].id, fingers[i].x, fingers[i].y, fingers[i].touch_major, fingers[i].width_major);
	                     }
				
				input_mt_sync(ts->input_dev);
	                    Old_finger_state[i] = finger_state[i];
			}

			#if 0
			if(finger_report == 0)
				input_mt_sync(ts->input_dev);
			#endif
			
			input_sync(ts->input_dev);
		}

end:
		if (ts->use_irq) 
		{
	/* OPPO 2011-05-27 huanggd Modify begin for avoid tp i2c err when system wakeup */
			//enable_irq(ts->client->irq);
			syna_irq_set_locked(ts->client, 1, 0);
	/* OPPO 2011-05-27 huanggd Modify end */
		}    
	}

	complete_all(&ts->synaptics_update_stop);
	kfree(buf);
	
	return 0;

}

static int syna_dev_open(struct inode *inode, struct file *file)
{
	print_ts(DEBUG_TRACE, "syna_dev_open\n");
	return 0;
}

static int syna_dev_release(struct inode *inode, struct file *file)
{
	print_ts(DEBUG_TRACE, "syna_dev_release\n");
	return 0;
}

static long syna_dev_ioctl(struct file *file,
		     unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int version;
	char *fn;
	

	switch (cmd) {
	case SYNA_GET_VERSION:
		if (copy_to_user((void *)arg, &(syna_ts_data->version),
				 sizeof(syna_ts_data->version))) {
			ret = -EFAULT;
			print_ts(DEBUG_INFO_TS, "SYNA_GET_VERSION copy_to_user error\n");
		}
		break;
		
	case SYNA_GET_FW_VERSION:
		break;
		
	case SYNA_PROGRAM_START:
		if (copy_from_user
			(&version, (void *)arg, sizeof(version))) {
			ret = -EFAULT;
			print_ts(DEBUG_INFO_TS, "SYNA_PROGRAM_START copy_from_user error\n");
			break;
		}
		
		switch (version) {
		case TOUCH_SCREEN_VERION_0:
			fn = SYNA_FW_0_NAME;
			break;
		case TOUCH_SCREEN_VERION_1:
			fn = SYNA_FW_1_NAME;
			break;
		case TOUCH_SCREEN_VERION_2:
			fn = SYNA_FW_2_NAME;
			break;
		default:
			fn = SYNA_FW_1_NAME;
			break;
		}

		ret = Syna_load_fw(syna_ts_data, fn);
		if (ret) 
		{
			print_ts(DEBUG_INFO_ERROR, "firmware %s update failed(%d)\n", fn, ret);
		} 
		else 
		{
			Refresh_Touchscreen_param(syna_ts_data);
			Refresh_input_param(syna_ts_data);
		}
		break;
		
	case SYNA_GET_PROGRAM_STATE:
		if (copy_to_user((void *)arg, &Syna_Program_state,
				 sizeof(Syna_Program_state))) {
			ret = -EFAULT;
			print_ts(DEBUG_INFO_TS, "SYNA_GET_PROGRAM_STATE copy_to_user error\n");
		}
		break;

	default:
		return -EIO;
	}

	return ret;
}

static int synaptics_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct synaptics_ts_data *ts;
	struct synaptics_i2c_rmi_platform_data *pdata;
	int fuzz_x, fuzz_y, fuzz_p, fuzz_w;
	int inactive_area_left;
	int inactive_area_right;
	int inactive_area_top;
	int inactive_area_bottom;
	int snap_left_on;
	int snap_left_off;
	int snap_right_on;
	int snap_right_off; 
	int snap_top_on;
	int snap_top_off;
	int snap_bottom_on;
	int snap_bottom_off;
	uint16_t max_x, max_y;
	int ret = 0;
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
	{
		print_ts(DEBUG_INFO_ERROR, "synaptics_ts_probe: need I2C_FUNC_I2C\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) 
	{
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

	syna_ts_data = ts;

	init_completion(&ts->synaptics_update_stop);
	
	INIT_DELAYED_WORK(&ts->delay_work, synaptics_ts_delay_work);
	
	ts->client = client;
	i2c_set_clientdata(client, ts);

	pdata = client->dev.platform_data;
	if (pdata)
		ts->power = pdata->power;

	#if 0
	if (ts->power) 
	{
		ret = ts->power(1);
		if (ret < 0) 
		{
			print_ts(DEBUG_INFO_ERROR, "synaptics_ts_probe power on failed\n");
			goto err_power_failed;
		}
	}
	msleep(250);
	#endif

	ret = i2c_smbus_write_byte_data(ts->client, PAGE_SELECT, 0x00);
	if (ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR,  "i2c_smbus_write_byte_data failed, line=%d\n", __LINE__);
		goto err_detect_failed;
	}

	Refresh_Touchscreen_param(ts);
	max_x = ts->max[0];
	max_y = ts->max[1];

	//disable interrupt
	ret = synaptics_set_int_mask(ts, 0);
	if(ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR, " synaptics_ts_probe: disable interrupt failed\n");
		goto err_power_failed;;
	}

	if (pdata) 
	{
		while (pdata->version > ts->version)
			pdata++;
		ts->flags = pdata->flags;
		inactive_area_left = pdata->inactive_left;
		inactive_area_right = pdata->inactive_right;
		inactive_area_top = pdata->inactive_top;
		inactive_area_bottom = pdata->inactive_bottom;
		snap_left_on = pdata->snap_left_on;
		snap_left_off = pdata->snap_left_off;
		snap_right_on = pdata->snap_right_on;
		snap_right_off = pdata->snap_right_off;
		snap_top_on = pdata->snap_top_on;
		snap_top_off = pdata->snap_top_off;
		snap_bottom_on = pdata->snap_bottom_on;
		snap_bottom_off = pdata->snap_bottom_off;
		fuzz_x = pdata->fuzz_x;
		fuzz_y = pdata->fuzz_y;
		fuzz_p = pdata->fuzz_p;
		fuzz_w = pdata->fuzz_w;
	} 
	else 
	{
		inactive_area_left = 0;
		inactive_area_right = 0;
		inactive_area_top = 0;
		inactive_area_bottom = 0;
		snap_left_on = 0;
		snap_left_off = 0;
		snap_right_on = 0;
		snap_right_off = 0;
		snap_top_on = 0;
		snap_top_off = 0;
		snap_bottom_on = 0;
		snap_bottom_off = 0;
		fuzz_x = 0;
		fuzz_y = 0;
		fuzz_p = 0;
		fuzz_w = 0;
	}


reconfig:
	//config tm1734: set report rate, sleep mode 
	ret = synaptics_init_panel(ts); 
	if (ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR, "synaptics_init_panel failed\n");
		goto err_detect_failed;
	}

	//check configuring is successful
	ret = i2c_smbus_read_byte_data(ts->client, REG_DEV_STATUS);
	if (ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR, "i2c_smbus_write_byte_data failed, line=%d\n", __LINE__);
		goto err_detect_failed;
	}
	if (ret & 0x80) 
	{
		print_ts(DEBUG_INFO_ERROR, "configuring TM1734 error! reconfig...\n");
		goto reconfig;
	}

	ret = i2c_smbus_read_byte_data(ts->client, REG_2D_Reporting_Mode);
	if (ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR, "i2c_smbus_write_byte_data failed, line=%d\n", __LINE__);
		goto err_detect_failed;
	}


	//initialize input device
	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) 
	{
		ret = -ENOMEM;
		print_ts(DEBUG_INFO_ERROR, "synaptics_ts_probe: Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}
	ts->input_dev->name = SYNAPTICS_I2C_RMI_NAME;	

	inactive_area_left = inactive_area_left * max_x / 0x10000;
	inactive_area_right = inactive_area_right * max_x / 0x10000;
	inactive_area_top = inactive_area_top * max_y / 0x10000;
	inactive_area_bottom = inactive_area_bottom * max_y / 0x10000;
	snap_left_on = snap_left_on * max_x / 0x10000;
	snap_left_off = snap_left_off * max_x / 0x10000;
	snap_right_on = snap_right_on * max_x / 0x10000;
	snap_right_off = snap_right_off * max_x / 0x10000;
	snap_top_on = snap_top_on * max_y / 0x10000;
	snap_top_off = snap_top_off * max_y / 0x10000;
	snap_bottom_on = snap_bottom_on * max_y / 0x10000;
	snap_bottom_off = snap_bottom_off * max_y / 0x10000;
	fuzz_x = fuzz_x * max_x / 0x10000;
	fuzz_y = fuzz_y * max_y / 0x10000;

	ts->snap_down[!!(ts->flags & SYNAPTICS_SWAP_XY)] = -inactive_area_left;
	ts->snap_up[!!(ts->flags & SYNAPTICS_SWAP_XY)] = max_x + inactive_area_right;
	ts->snap_down[!(ts->flags & SYNAPTICS_SWAP_XY)] = -inactive_area_top;
	ts->snap_up[!(ts->flags & SYNAPTICS_SWAP_XY)] = max_y + inactive_area_bottom;
	ts->snap_down_on[!!(ts->flags & SYNAPTICS_SWAP_XY)] = snap_left_on;
	ts->snap_down_off[!!(ts->flags & SYNAPTICS_SWAP_XY)] = snap_left_off;
	ts->snap_up_on[!!(ts->flags & SYNAPTICS_SWAP_XY)] = max_x - snap_right_on;
	ts->snap_up_off[!!(ts->flags & SYNAPTICS_SWAP_XY)] = max_x - snap_right_off;
	ts->snap_down_on[!(ts->flags & SYNAPTICS_SWAP_XY)] = snap_top_on;
	ts->snap_down_off[!(ts->flags & SYNAPTICS_SWAP_XY)] = snap_top_off;
	ts->snap_up_on[!(ts->flags & SYNAPTICS_SWAP_XY)] = max_y - snap_bottom_on;
	ts->snap_up_off[!(ts->flags & SYNAPTICS_SWAP_XY)] = max_y - snap_bottom_off;

	/* Single touch */
	#if 1
	input_set_abs_params(ts->input_dev, ABS_X, 0, max_x + 1, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_Y, 0, max_y- ts->Touch_key_height, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_PRESSURE, 0, 255,
			     0, 0);
	input_set_abs_params(ts->input_dev, ABS_TOOL_WIDTH, 0, 255,
			     0, 0);
	#endif

	/* Multitouch */
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, max_x + 1, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, max_y- ts->Touch_key_height, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255,0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_PRESSURE, 0, 255,0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 15,0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, MAX_FINGERS,
			     0, 0);
	
	__set_bit(EV_ABS, ts->input_dev->evbit);
	__set_bit(EV_SYN, ts->input_dev->evbit);
	__set_bit(EV_KEY, ts->input_dev->evbit);
	__set_bit(EV_MSC, ts->input_dev->evbit);

	__set_bit(KEY_SEARCH, ts->input_dev->keybit);
	__set_bit(KEY_HOME, ts->input_dev->keybit);
	__set_bit(KEY_MENU, ts->input_dev->keybit);
	__set_bit(KEY_BACK, ts->input_dev->keybit);
	
	ts->input_dev->mscbit[0] = BIT_MASK(MSC_GESTURE);

	/* virtual keys */
	Syna_properties_kobj = kobject_create_and_add("board_properties",
				NULL);
	if (Syna_properties_kobj)
		ret = sysfs_create_group(Syna_properties_kobj,
			&syna_properties_attr_group);
	
	if (!Syna_properties_kobj || ret)
		print_ts(DEBUG_INFO_ERROR, "%s: failed to create board_properties\n", __func__);
	/* virtual keys */

	ret = input_register_device(ts->input_dev);
	if (ret) 
	{
		print_ts(DEBUG_INFO_ERROR, "synaptics_ts_probe: Unable to register %s input device\n", ts->input_dev->name);
		goto err_input_register_device_failed;
	}

	if (client->irq) 
	{
		ret = request_irq(client->irq, synaptics_ts_irq_handler, IRQF_TRIGGER_FALLING, client->name, ts);
		if (ret == 0) 
		{
			ret = synaptics_set_int_mask(ts, 1);
			if(ret < 0) 
			{
				print_ts(DEBUG_INFO_ERROR, "synaptics_ts_probe: failed to enable synaptics interrupt!\n");
				free_irq(client->irq, ts);
				goto err_input_register_device_failed;
			}
			ts->use_irq = 1;			
		} 
		else 
		{
			print_ts(DEBUG_INFO_ERROR, "synaptics_ts_probe: request_irq failed\n");
		}
	} 
	else 
	{
		#ifdef POLLING_TOUCH_PAD
		hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		ts->timer.function = synaptics_ts_timer_func;
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
		#endif
	}

#ifdef SYNA_EARLYSUSPEND
#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 10;
	ts->early_suspend.suspend = synaptics_ts_early_suspend;
	ts->early_suspend.resume = synaptics_ts_late_resume;
	//ts->early_suspend.name = "syna_touch";
	register_early_suspend(&ts->early_suspend);
#endif
#endif

	ret = init_synaptics_proc(ts);

	/*launch the thread*/
	ts->synaptics_update = kthread_run(synaptics_ts_thread, ts,
					SYNAPTICS_I2C_RMI_NAME);
	
	if (IS_ERR(ts->synaptics_update))
		goto err_input_register_device_failed;

	print_ts(DEBUG_TRACE, "synaptics_ts_probe: Start touchscreen %s in %s mode\n", 
		ts->input_dev->name, ts->use_irq ? "interrupt" : "polling");


	return 0;

err_input_register_device_failed:
	input_free_device(ts->input_dev);

err_input_dev_alloc_failed:
err_detect_failed:
err_power_failed:
	
	/* OPPO 2011-04-22 zhangqiang Add begin to close the ts power when ts probe failed for esd handle */
	if (ts->power) 
	{
		ret = ts->power(0);
		if (ret < 0) 
		{
			print_ts(DEBUG_INFO_ERROR, "synaptics_ts_suspend power off failed\n");
		}
	}	
	/* OPPO 2011-04-22 zhangqiang Add end */	
	
	kfree(ts);
err_alloc_data_failed:
err_check_functionality_failed:

	return ret;
}

static int synaptics_ts_remove(struct i2c_client *client)
{
	struct synaptics_ts_data *ts = i2c_get_clientdata(client);

	kthread_stop(ts->synaptics_update);
	wait_for_completion(&ts->synaptics_update_stop);

	unregister_early_suspend(&ts->early_suspend);
	
	if (ts->use_irq)
		free_irq(client->irq, ts);
	else
	{
		#ifdef POLLING_TOUCH_PAD
		hrtimer_cancel(&ts->timer);
		#endif
	}

	input_unregister_device(ts->input_dev);
	kfree(ts);
	return 0;
}

static int synaptics_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret;
	struct synaptics_ts_data *ts = i2c_get_clientdata(client);
	is_tp_suspended = 1;
	syna_irq_set_locked(client, 0, 0);
	
	cancel_delayed_work_sync(&ts->delay_work);
	
	ret = synaptics_set_int_mask(ts, 0);
	if(ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR, "synaptics_ts_suspend: cannot disable interrupt\n");
		
	}

	#ifdef SYNA_1734_SLEEP
	ret = i2c_smbus_write_byte_data(client, REG_Device_Control, 0x01); 	//sleep : device control[0:1]:0x01
	if (ret < 0) 
	{
		print_ts(DEBUG_INFO_ERROR, "synaptics_ts_suspend: control tm1734 to sleep failed\n");
	}
	#else
	if (ts->power) 
	{
		ret = ts->power(0);
		if (ret < 0) 
		{
			print_ts(DEBUG_INFO_ERROR, "synaptics_ts_suspend power off failed\n");
		}
	}
	#endif

	ts->data_ready = 1;
	wake_up(&synaptics_thread_wq);
	return 0;
}
//huqiao changed start
int release_pointer_handle(struct synaptics_ts_data *ts)
{	 
	int i;
	for( i= 0; i<MAX_FINGERS; ++i )
	 {
		 if(fingers[i].touch_major== -1 )
		{
			continue;
		 }
		 
	  	 input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, fingers[i].id);
		 input_report_abs(ts->input_dev, ABS_MT_POSITION_X, fingers[i].x);
		 input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, fingers[i].y);
		 input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
         //input_report_abs(ts->input_dev,ABS_MT_WIDTH_MAJOR, 0);
		input_mt_sync(ts->input_dev);

		fingers[i].touch_major= -1;
	}
	input_sync(ts->input_dev);
return 1;
}
//huqiao changed end

static int synaptics_ts_resume(struct i2c_client *client)
{
	int ret;
	struct synaptics_ts_data *ts = i2c_get_clientdata(client);

	#ifdef SYNA_1734_SLEEP
	//synaptics_set_int_mask( ts, 1);
	#else
	if (ts->power) 
	{
		ret = ts->power(1);
		if (ret < 0)
			print_ts(DEBUG_INFO_ERROR, "synaptics_ts_resume power on failed\n");
	}
	#endif

	ts->drop_data = 10;
	is_tp_suspended = 0;
	syna_irq_set_locked(client, 1, 0);
	//synaptics_software_reset(ts);
	
	ts->cmd = SYNA_TS_CMD_RESUME_200MS;
	schedule_delayed_work(&ts->delay_work, msecs_to_jiffies(200));
	
	return 0;
}

#ifdef SYNA_EARLYSUSPEND
#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_ts_early_suspend(struct early_suspend *h)
{
	struct synaptics_ts_data *ts;
	ts = container_of(h, struct synaptics_ts_data, early_suspend);
	synaptics_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void synaptics_ts_late_resume(struct early_suspend *h)
{
	struct synaptics_ts_data *ts;
	ts = container_of(h, struct synaptics_ts_data, early_suspend);
	synaptics_ts_resume(ts->client);
}
#endif
#endif

static const struct i2c_device_id synaptics_ts_id[] = 
{
	{ SYNAPTICS_I2C_RMI_NAME, 0 },
	{ }
};

static struct i2c_driver synaptics_ts_driver = 
{
	.probe		= synaptics_ts_probe,
	.remove		= synaptics_ts_remove,
#ifndef SYNA_EARLYSUSPEND
	.suspend		= synaptics_ts_suspend,
	.resume		= synaptics_ts_resume,
#endif
	.id_table		= synaptics_ts_id,
	.driver = 
	{
		.name	= SYNAPTICS_I2C_RMI_NAME,
	},
};

static struct file_operations syna_dev_fops = {
	.owner		= THIS_MODULE,
	.open		= syna_dev_open,
	.unlocked_ioctl		= syna_dev_ioctl,
	.release	= syna_dev_release,
};

struct miscdevice syna_control_device = {
	.minor	= 255,
	.name	= "syna_touch",
	.fops	= &syna_dev_fops,
};

static int __devinit synaptics_ts_init(void)
{
	if(!Check_Touchscreen_type(TS_SYNA_1734))
		return -EINVAL;
	//misc_register(&syna_control_device);
	return i2c_add_driver(&synaptics_ts_driver);
}

static void __exit synaptics_ts_exit(void)
{
	i2c_del_driver(&synaptics_ts_driver);
}

module_init(synaptics_ts_init);
module_exit(synaptics_ts_exit);

MODULE_DESCRIPTION("Synaptics tm1734 Touchscreen Driver");
MODULE_LICENSE("GPL");
