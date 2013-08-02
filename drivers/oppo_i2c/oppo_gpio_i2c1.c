/*
 * Copyright (C) 2010 OPPO, Inc.
 * Author: Zhang Yuanfei <zhangyf@oppo.com>
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

/*typedef some data type*/
//typedef unsigned char   BYTE;
typedef unsigned char   UCHAR;
typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned long   UINT32;
typedef unsigned long long  UINT64;
typedef signed char   CHAR;
typedef signed char   INT8;
typedef signed short  INT16;
typedef signed long   INT32;
//typedef UINT8  BOOL;
typedef float  FLOAT;
typedef double  DOUBLE;


#define TRUE 1
#define FALSE 0


/*define sda and scl*/

#define PM8058_GPIO08	7//PM8058_GPIO_PM_TO_SYS(7)	
#define PM8058_GPIO15	14//PM8058_GPIO_PM_TO_SYS(14)	
#define GPIO_I2C_SCL	PM8058_GPIO08
#define GPIO_I2C_SDA	PM8058_GPIO15


//define delay
#define GPIO_I2C_RETRY_COUNT 	3
#define GPIO_I2C_BUS_DELAY		255		// time out bus arbitration
#define GPIO_I2C_ACK_DELAY  	200     // time out for acknowledge check
#define GPIO_I2C_CLK_DELAY  	7		// about 17khz 

#define   FG_SEQREAD    1	//read continuously
#define   FG_RANDREAD   0	//read only one byte
//#else
#define DEFAULT_SIF_CLK_DIV  0x64		//30kHz


/*define global variable*/
static UCHAR gpio_i2c_initialized = 0;			//gpio_i2c initiated flag
static struct semaphore	protectSemaphore;

#define Delay2us(x)		udelay(2*x)
extern struct pm_gpio_chip *pm8058_gpio_chip;
extern int oppo_pm_gpio_get(struct pm_gpio_chip *chip, unsigned gpio);
extern int oppo_pm_gpio_set(struct pm_gpio_chip *chip,unsigned gpio, int value);
#define pm_gpio_set oppo_pm_gpio_set
#define pm_gpio_get oppo_pm_gpio_get
/*define macros to operate sda and scl*/
#define READ_SIF_SDA  	oppo_pm_gpio_get(pm8058_gpio_chip, GPIO_I2C_SDA)
#define CLR_SIF_SDA  	oppo_pm_gpio_set(pm8058_gpio_chip,GPIO_I2C_SDA, 0)
#define SET_SIF_SDA     oppo_pm_gpio_set(pm8058_gpio_chip,GPIO_I2C_SDA, 1)

#define READ_SIF_SCL  	oppo_pm_gpio_get(pm8058_gpio_chip, GPIO_I2C_SCL)
#define CLR_SIF_SCL  	oppo_pm_gpio_set(pm8058_gpio_chip,GPIO_I2C_SCL, 0)
#define SET_SIF_SCL     oppo_pm_gpio_set(pm8058_gpio_chip,GPIO_I2C_SCL, 1)

/*declare extern struct and functions which located in kernel/arch/arm/pm8058-gpio.c*/



struct pm8058_gpio_cfg {
	int        		gpio;
	struct pm_gpio cfg;
	};
/* 
 * pm8058 gpio config routine.
 *
 * @param gpio: gpio control line
 * @param gpio_value: value set to gpio
 */
static void pmic8058_i2c_gpio_config(int gpio,int gpio_value)
{

	struct pm8058_gpio_cfg gpio_cfgs[] = 
	{
		{
			gpio,
			{
					.direction	= PM_GPIO_DIR_BOTH,
					.output_buffer	= PM_GPIO_OUT_BUF_CMOS,
					.output_value	= gpio_value,
					
					.pull		= PM_GPIO_PULL_UP_30,
					.vin_sel	= PM8058_GPIO_VIN_L6,
					.out_strength	= PM_GPIO_STRENGTH_LOW,
					.function	= PM_GPIO_FUNC_NORMAL,
					
					.inv_int_pol	= 0,
					.disable_pin = 0,
			}
		}
   	};
	
	pm8xxx_gpio_config(gpio_cfgs[0].gpio,
		&(gpio_cfgs[0].cfg));
	
}


/*
  * Function : BOOL gpio_i2c_sendByte(BYTE bValue)
  * Description : Send Routine
  *             timing : SCL ___|^|___|^|__~__|^|___|^|__
  *                       SDA __/D7 \_/D6 \_~_/D0 \_/ACK\_
  * @Param : bValue(8-bit output data)
  * Return : TRUE  : successful with ACK from slave
  *          FALSE  : bus (SCL = 0) or ACK failure
*/

static BOOL gpio_i2c_sendByte(BYTE bValue)
{
    BYTE bBitMask = 0x80;

    // step 1 : 8-bit data transmission
    while(bBitMask)
    {
        if(bBitMask & bValue)
        {
            SET_SIF_SDA;
        }
        else
        {
            CLR_SIF_SDA;
        }

        Delay2us(GPIO_I2C_CLK_DELAY);
        SET_SIF_SCL;						// data clock in
        Delay2us(GPIO_I2C_CLK_DELAY);
        CLR_SIF_SCL;						// ready for next clock in
        Delay2us(GPIO_I2C_CLK_DELAY);		//mark by CM 20080201
        bBitMask = bBitMask >> 1;			// MSB first & timing delay
    }

    // step 2 : slave acknowledge check
    SET_SIF_SDA;							// release SDA for ACK polling
    Delay2us(GPIO_I2C_CLK_DELAY);
    SET_SIF_SCL;							// start ACK polling
    Delay2us(GPIO_I2C_CLK_DELAY);

    bBitMask = GPIO_I2C_ACK_DELAY;			// time out protection
    while(READ_SIF_SDA && (--bBitMask))
    {
        Delay2us(GPIO_I2C_CLK_DELAY);		// wait for ACK, SDA=0 or bitMask=0->jump to this loop
    }
    
    CLR_SIF_SCL;							// end ACK polling
    Delay2us(GPIO_I2C_CLK_DELAY);			//mark by CM 20080201

    if(bBitMask)
    {
        return(TRUE);						// return TRUE if ACK detected
    }
    else
    {
		pr_err("%s: failed to send byte\n", __func__);
        return(FALSE);						// return FALSE if time out
    }
}

/*
 * Function : BOOL gpio_i2c_receiveByte(BYTE *ptrValue, BOOL isSequentialed)
 * Description : Receive Routine
 *               timing : SCL ___|^|___|^|__~__|^|___|^|__
 *                        SDA __/D7 \_/D6 \_~_/D0 \_/ACK\_
 * @Param *ptrValue: 8-bit input pointer value
 * @Param isSequentialed: (8-bit input pointer value)
 * Return    : NONE
*/
static BOOL gpio_i2c_receiveByte(BYTE *ptrValue, BOOL isSequentialed) 
{
    BYTE bBitMask = 0x80;

    *ptrValue = 0;                 // reset data buffer
    SET_SIF_SDA;                      // make sure SDA released
    Delay2us(GPIO_I2C_CLK_DELAY);

    // step 1 : 8-bit data reception
    while(bBitMask)
    {
        SET_SIF_SCL;                    // data clock out
        Delay2us(GPIO_I2C_CLK_DELAY);
     
        if(READ_SIF_SDA)
        {
            *ptrValue = *ptrValue | bBitMask;   // Get all data
        }                                   // non-zero bits to buffer

        CLR_SIF_SCL;                            // ready for next clock out
        Delay2us(GPIO_I2C_CLK_DELAY);
        bBitMask = bBitMask >> 1;           // shift bit mask & clock delay
    }

    // step 2 : acknowledgement to slave
    if(isSequentialed)
    {
        CLR_SIF_SDA;                            // ACK here for Sequential Read
    }
    else
    {
        SET_SIF_SDA;                            // NACK here (for single byte read)
    }

    Delay2us(GPIO_I2C_CLK_DELAY);
    SET_SIF_SCL;                    // NACK clock out
    Delay2us(GPIO_I2C_CLK_DELAY);
    CLR_SIF_SCL;                    // ready for next clock out
    Delay2us(GPIO_I2C_CLK_DELAY);
    SET_SIF_SDA;                    // release SDA
    Delay2us(GPIO_I2C_CLK_DELAY);
    return TRUE;
}


/*
  * Function : BOOL gpio_i2c_start(BYTE bValue)
  * Description : Start Routine
  *              timing : SCL ^^^^|___|^|___|^|__~__|^|___|^|___|^|__~
  *                       SDA ^^|____/A6 \_/A5 \_~_/A0 \_/R/W\_/ACK\_~
  *                            (S)
  *                             |<--- start condition
  * @Param bDevice: 7-bit slave address + (R/W) bit
  * @Return    : TRUE  : successful with ACK from slave
  *              FALSE  : bus (SCL = 0) or ACK failure
*/
static BOOL gpio_i2c_start(BYTE bDevice)
{
    SET_SIF_SDA;            // make sure SDA released
    Delay2us(GPIO_I2C_CLK_DELAY);
    SET_SIF_SCL;            // make sure SCL released
    Delay2us(GPIO_I2C_CLK_DELAY);

    CLR_SIF_SDA;          // start condition here
    Delay2us(GPIO_I2C_CLK_DELAY);
    CLR_SIF_SCL;          // ready for clocking
    Delay2us(GPIO_I2C_CLK_DELAY);

    return(gpio_i2c_sendByte(bDevice));// slave address & R/W transmission
}

/*
 * Function : BOOL gpio_i2c_stop(void)
 * Description : Stop Routine
 *               timing : SCL ___|^^^^^
 *                        SDA xx___|^^^
 *                                (P)
 *                                 |<--- stop condition
 *  Parameter : NONE
 *  Return    : NONE
*/
static void gpio_i2c_stop(void) 
{
    CLR_SIF_SDA;          // ready for stop condition
    Delay2us(GPIO_I2C_CLK_DELAY);
    SET_SIF_SCL;          // ready for stop condition
    Delay2us(GPIO_I2C_CLK_DELAY);
    SET_SIF_SDA;          // stop condition here
    Delay2us(GPIO_I2C_CLK_DELAY);

	//return (TRUE);
}


/*
 * Function : BOOL gpio_i2c1_readData(BYTE bDevice, BYTE bData_Addr,
 *                                   BYTE bDataCount, BYTE *prData)
 * Description :		DataRead Routine
 * @Param bDevice:		Device Address
 * @Param u2Data_Addr:	register Address
 * @Param bDataCount:	Data Content Cont
 * @Param *prData:		Data Content Pointer
 * Return TRUE  : successful with ACK from slave
 *        FALSE  : bus (SCL = 0) or ACK failure
*/
BOOL gpio_i2c1_readData(BYTE bDevice, UINT16 u2Data_Addr, BYTE bDataCount,BYTE *prData) 
{
    BYTE bIndex,bOrg_device;
    BYTE bData_Addr; 

    if (gpio_i2c_initialized == 0)
    {
        pr_err("%s: gpio_i2c not initialized\n", __func__);
		return FALSE;
    }

	/*if invalid device address return false*/
	if(bDevice >= 128)
	{
		return(FALSE);
	}
	
	/*wait semaphore*/ 
	down(&protectSemaphore);
	
	bOrg_device=bDevice;
	for(bIndex=0;bIndex<GPIO_I2C_RETRY_COUNT;bIndex++)
	{
		/*Step 1 : Dummy Write*/ 
		bDevice=bOrg_device;	
		Delay2us(5);	
		
		bDevice = bDevice << 1;   // Shift the 7-bit address to 7+1 format
		
		if(!gpio_i2c_start(bDevice))  // Write Command
		{
			gpio_i2c_stop();     
			pr_err("%s: start fail\n", __func__); 
			continue; //Start fail
		}

		bData_Addr = (BYTE)(u2Data_Addr&0xff);
		
		if(!gpio_i2c_sendByte(bData_Addr))// register Address
		{
			gpio_i2c_stop();    
			pr_err("%s: send register address fail\n", __func__);  
			continue;// Data Address Fail
		}
		
		// Step 2 : Real Read
		bDevice = bDevice + 1;    // Shift the 7-bit address to 7+1 format
		if(!gpio_i2c_start(bDevice))  // Read Command
		{
			gpio_i2c_stop();      
			continue; // Start fail
		}
		
		while (bDataCount)
		{
			if (bDataCount == 1)
			{
				if(!gpio_i2c_receiveByte(prData++, FG_RANDREAD))
				{
					gpio_i2c_stop();
					break;
				}  // Data Content Read
			}
			else
			{
				if(!gpio_i2c_receiveByte(prData++, FG_SEQREAD))
				{
					gpio_i2c_stop();
					break;
				}  // Data Content Read
			}
			bDataCount--;
		}
		/*send fail, retry*/
		if(bDataCount)
		{
			continue;
		}
		
		// Step 3 : Stop
		gpio_i2c_stop();
		// release semaphore
		up(&protectSemaphore); 
		return (TRUE);
	}//for(bIndex=0;bIndex<GPIO_I2C_RETRY_COUNT;bIndex++)
	
	gpio_i2c_stop();
	// release semaphore
	up(&protectSemaphore);
    return(FALSE);


}


/*
 * Function : BOOL gpio_i2c1_writeData(BYTE bDevice, BYTE bData_Addr,
 *                                   BYTE bDataCount, BYTE *prData)
 * Description :ByteWrite Routine
 * @Param bDevice:		Device Address
 * @Param bData_Addr:	register Address
 * @Param bDataCount:	Data Content Cont
 * @Param *prData:		Data Content Pointer
 * Return    : TRUE  : successful with ACK from slave
 *               FALSE  : bus (SCL = 0) or ACK failure
*/

BOOL gpio_i2c1_writeData(BYTE bDevice, UINT16 u2Data_Addr,BYTE bDataCount, BYTE *prData)
{
    BYTE bOrg_device;
    BYTE bIndex,bDataFail=0,bOri_count,bData_Addr;

    if (gpio_i2c_initialized == 0)
    {
        pr_err("%s: gpio_i2c not initialized\n", __func__);
		return 0;
    }
	
	if(bDevice >= 128)
	{
		return(FALSE);             // Device Address exceeds the range
	}
	
	// wait semaphore
	down(&protectSemaphore);
	
	bOrg_device=bDevice;	
	bOri_count=bDataCount;

	for(bIndex=0;bIndex<GPIO_I2C_RETRY_COUNT;bIndex++)
	{
		bDevice=bOrg_device;	
		
		Delay2us(5);
		bDevice = bDevice << 1;      // Shift the 7-bit address to 7+1 format
		
		if(!gpio_i2c_start(bDevice))     // Write Command
		{
			gpio_i2c_stop();//kenny 2006/3/21
			continue;// Device Address exceeds the range
		}

		bData_Addr = (BYTE)(u2Data_Addr&0xff);
		
		if(!gpio_i2c_sendByte(bData_Addr))// Word Address
		{
			gpio_i2c_stop();      
			continue;// Data Address Fail
		}
		
		bDataFail=0;
		bDataCount=bOri_count;	
		while(bDataCount)
		{
			if(!gpio_i2c_sendByte(*(prData++))) // Data Content Write
			{
				bDataFail=1;	
				gpio_i2c_stop();//kenny 2006/3/21
				break;// Device Address exceeds the range
			}
			
			bDataCount--;
		}//while(bDataCount)

		if(bDataFail)
            continue;
		
		gpio_i2c_stop();
		up(&protectSemaphore); // release semaphore

		return(TRUE);
	}//for(bIndex=0;bIndex<GPIO_I2C_RETRY_COUNT;bIndex++)
	
	/*retry fail*/
	gpio_i2c_stop();
	up(&protectSemaphore); //release semaphore

    return(FALSE); 	
}


/*
 * initializes I2C interface routine.
 *
 * @return value:0--success; 1--error.
 *
 */
static int __init gpio_i2c1_init(void)
{

	if (gpio_i2c_initialized == 0)
	{
		gpio_i2c_initialized = 1;	//set initialize flag
		sema_init(&protectSemaphore,1);	//initialize semaphore
		
		/*config pm8058_gpio*/
		pmic8058_i2c_gpio_config(PM8058_GPIO_PM_TO_SYS(PM8058_GPIO08),1);
		pmic8058_i2c_gpio_config(PM8058_GPIO_PM_TO_SYS(PM8058_GPIO15),1);

		return 0;
	} 
	else
	{
		pr_err("%s: gpio_i2c1 initialize fail\n", __func__);
		return 1;
	}
	
}

static void __exit gpio_i2c1_exit(void)
{
    gpio_i2c_initialized = 0;
}

module_init(gpio_i2c1_init);
module_exit(gpio_i2c1_exit);

#ifdef MODULE
#include <linux/compile.h>
#endif

MODULE_INFO(build, UTS_VERSION);
MODULE_AUTHOR("Zhang Yanfei");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("gpio i2c module");
MODULE_ALIAS("a simple module");

EXPORT_SYMBOL(gpio_i2c1_readData);
EXPORT_SYMBOL(gpio_i2c1_writeData);


