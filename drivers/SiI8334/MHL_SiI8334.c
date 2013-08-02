/***********************************************************************************/
/* File Name: MHL_SiI8334.c */
/* File Description: this file is used to make sii8334 driver to be added in kernel or module. */

/*  Copyright (c) 2002-2010, Silicon Image, Inc.  All rights reserved.             */
/*  No part of this work may be reproduced, modified, distributed, transmitted,    */
/*  transcribed, or translated into any language or computer format, in any form   */
/*  or by any means without written permission of: Silicon Image, Inc.,            */
/*  1060 East Arques Avenue, Sunnyvale, California 94085                           */
/***********************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <linux/kobject.h>
#include <linux/io.h>
#include <linux/kthread.h>

#include <linux/bug.h>
#include <linux/err.h>
#include <linux/i2c.h>

#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include "MHL_SiI8334.h"
#include "si_mhl_tx_api.h"
#include "si_mhl_tx.h"
#include "si_drv_mhl_tx.h"
#include "si_mhl_defs.h"



//interrupt mode or polling mode for 8334 driver
#define SiI8334DRIVER_INTERRUPT_MODE   1

//Debug test
#undef dev_info
#define dev_info dev_err
#define MHL_DRIVER_NAME "sii8334drv"

#define MHL_DRIVER_MINOR_MAX   1
#define EVENT_POLL_INTERVAL_30_MS	30

#define MHL_RESUME_DELAY	msecs_to_jiffies(100)


#ifdef CONFIG_OPPO_MODIFY
//added by zhangyue on 2011-11-17 for adding the power control interface in /sys filesystem
//and both add the ability of  in suspend state totally shutdown the mhl module.
//you can search this macro for all the added code
#ifndef mhl_power_dbg
#define mhl_power_dbg	1
#endif
#ifdef mhl_power_dbg
#include <linux/sched.h>

struct mhl_state_desc mhl_state;
#else
extern mhlTx_config_t	mhlTxConfig;
#endif 
#endif
/***** public type definitions ***********************************************/

typedef struct {
    struct task_struct	*pTaskStruct;
    uint8_t				pendingEvent;		// event data wait for retrieval
    uint8_t				pendingEventData;	// by user mode application

} MHL_DRIVER_CONTEXT_T, *PMHL_DRIVER_CONTEXT_T;


/***** global variables ********************************************/

MHL_DRIVER_CONTEXT_T gDriverContext;


struct platform_data {
    void (*reset) (void);
#ifdef mhl_power_dbg
    int (* mhl_power_on)(void);
    int (* mhl_power_off)(void);	
    int (* mhl_irq_config)(void);
#endif	
	//added by zhangyue on 2012-05-03 for making USB switch to mhl function
	int (* mhl_swtich)(int on);
};

static struct platform_data *Sii8334_plat_data;

//added by zhangyue on 2012-05-03 for making USB switch to mhl function start
int switch_to_mhl(int on)
{
	int ret = 0;
	if(Sii8334_plat_data->mhl_swtich != NULL){
		ret = Sii8334_plat_data->mhl_swtich(on);		
	}
	return ret;			
}
//added by zhangyue on 2012-05-03 for making USB switch to mhl function end

bool_t	vbusPowerState = true;		// false: 0 = vbus output on; true: 1 = vbus output off;

static bool_t match_id(const struct i2c_device_id *id, const struct i2c_client *client)
{
    if (strcmp(client->name, id->name) == 0){
        return true;
    }    
    return false;
}

#ifdef CONFIG_OPPO_MODIFY
//added by zhangyue on 2011-12-19 for debugging irq problems
void mhl_set_irq_locked(bool enable_disable, bool inIsr)
{
	static bool mhl_irq_status = true;

	if (enable_disable == mhl_irq_status) {
		TX_DEBUG_PRINT(( "%s: irq is configed, status=%d\n", __func__, enable_disable));
		return;
	}

	mhl_irq_status = enable_disable;
	
	if (enable_disable) {
		enable_irq(sii8334_PAGE_TPI->irq);
	} else {
		if (inIsr) {
			disable_irq_nosync(sii8334_PAGE_TPI->irq);
		}else{
			disable_irq(sii8334_PAGE_TPI->irq);
		}
	}
}
#endif

static bool_t Sii8334_mhl_reset(void)
{
#ifdef mhl_power_dbg
    //commented by zhangyue on 2011-11-14 for adding power saving in suspend state
#else
    Sii8334_plat_data = sii8334_PAGE_TPI->dev.platform_data;
#endif    
    if (Sii8334_plat_data->reset){
        Sii8334_plat_data->reset();
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// Function:    HalTimerWait
// Description: Waits for the specified number of milliseconds, using timer 0.
//------------------------------------------------------------------------------

void HalTimerWait ( uint16_t ms )
{
	msleep(ms);
}

#ifdef ZY_IMPLEMENT_RCP
void input_report_rcp_key(uint8_t rcp_keycode, int up_down)
{
    rcp_keycode &= 0x7F;
    switch ( rcp_keycode )
    {
    case MHL_RCP_CMD_SELECT:
        input_report_key(mhl_state.rmt_input, KEY_SELECT, up_down);
        TX_DEBUG_PRINT(( "\nSelect received\n\n" ));
        break;
    case MHL_RCP_CMD_UP:
        input_report_key(mhl_state.rmt_input, KEY_UP, up_down);
        TX_DEBUG_PRINT(( "\nUp received\n\n" ));
        break;
    case MHL_RCP_CMD_DOWN:
        input_report_key(mhl_state.rmt_input, KEY_DOWN, up_down);
        TX_DEBUG_PRINT(( "\nDown received\n\n" ));
        break;
    case MHL_RCP_CMD_LEFT:
        input_report_key(mhl_state.rmt_input, KEY_LEFT, up_down);
        TX_DEBUG_PRINT(( "\nLeft received\n\n" ));
        break;
    case MHL_RCP_CMD_RIGHT:
        input_report_key(mhl_state.rmt_input, KEY_RIGHT, up_down);
        TX_DEBUG_PRINT(( "\nRight received\n\n" ));
        break;
    case MHL_RCP_CMD_ROOT_MENU:
        input_report_key(mhl_state.rmt_input, KEY_MENU, up_down);
        TX_DEBUG_PRINT(( "\nRoot Menu received\n\n" ));
        break;
    case MHL_RCP_CMD_EXIT:
        input_report_key(mhl_state.rmt_input, KEY_EXIT, up_down);
        TX_DEBUG_PRINT(( "\nExit received\n\n" ));
        break;
    case MHL_RCP_CMD_NUM_0:
        input_report_key(mhl_state.rmt_input, KEY_NUMERIC_0, up_down);
        TX_DEBUG_PRINT(( "\nNumber 0 received\n\n" ));
        break;
    case MHL_RCP_CMD_NUM_1:
        input_report_key(mhl_state.rmt_input, KEY_NUMERIC_1, up_down);
        TX_DEBUG_PRINT(( "\nNumber 1 received\n\n" ));
        break;
    case MHL_RCP_CMD_NUM_2:
        input_report_key(mhl_state.rmt_input, KEY_NUMERIC_2, up_down);
        TX_DEBUG_PRINT(( "\nNumber 2 received\n\n" ));
        break;	
    case MHL_RCP_CMD_NUM_3:
        input_report_key(mhl_state.rmt_input, KEY_NUMERIC_3, up_down);
        TX_DEBUG_PRINT(( "\nNumber 3 received\n\n" ));
        break;	
    case MHL_RCP_CMD_NUM_4:
        input_report_key(mhl_state.rmt_input, KEY_NUMERIC_4, up_down);
        TX_DEBUG_PRINT(( "\nNumber 4 received\n\n" ));
        break;
    case MHL_RCP_CMD_NUM_5:
        input_report_key(mhl_state.rmt_input, KEY_NUMERIC_5, up_down);
        TX_DEBUG_PRINT(( "\nNumber 5 received\n\n" ));
        break;	
    case MHL_RCP_CMD_NUM_6:
        input_report_key(mhl_state.rmt_input, KEY_NUMERIC_6, up_down);
        TX_DEBUG_PRINT(( "\nNumber 6 received\n\n" ));
        break;
    case MHL_RCP_CMD_NUM_7:
        input_report_key(mhl_state.rmt_input, KEY_NUMERIC_7, up_down);
        TX_DEBUG_PRINT(( "\nNumber 7 received\n\n" ));
        break;
    case MHL_RCP_CMD_NUM_8:
        input_report_key(mhl_state.rmt_input, KEY_NUMERIC_8, up_down);
        TX_DEBUG_PRINT(( "\nNumber 8 received\n\n" ));
        break;
    case MHL_RCP_CMD_NUM_9:
        input_report_key(mhl_state.rmt_input, KEY_NUMERIC_9, up_down);
        TX_DEBUG_PRINT(( "\nNumber 9 received\n\n" ));
        break;
    case MHL_RCP_CMD_DOT:
        input_report_key(mhl_state.rmt_input, KEY_DOT, up_down);
        TX_DEBUG_PRINT(( "\nDot received\n\n" ));
        break;
    case MHL_RCP_CMD_ENTER:
        input_report_key(mhl_state.rmt_input, KEY_ENTER, up_down);
        TX_DEBUG_PRINT(( "\nEnter received\n\n" ));
        break;
    case MHL_RCP_CMD_CLEAR:
        input_report_key(mhl_state.rmt_input, KEY_CLEAR, up_down);
        TX_DEBUG_PRINT(( "\nClear received\n\n" ));
        break;
    case MHL_RCP_CMD_SOUND_SELECT:
        input_report_key(mhl_state.rmt_input, KEY_SOUND, up_down);
        TX_DEBUG_PRINT(( "\nSound Select received\n\n" ));
        break;
    case MHL_RCP_CMD_PLAY:
        input_report_key(mhl_state.rmt_input, KEY_PLAY, up_down);
        TX_DEBUG_PRINT(( "\nPlay received\n\n" ));
        break;
    case MHL_RCP_CMD_PAUSE:
        input_report_key(mhl_state.rmt_input, KEY_PAUSE, up_down);
        TX_DEBUG_PRINT(( "\nPause received\n\n" ));
        break;
    case MHL_RCP_CMD_STOP:
        input_report_key(mhl_state.rmt_input, KEY_STOP, up_down);
        TX_DEBUG_PRINT(( "\nStop received\n\n" ));
        break;
    case MHL_RCP_CMD_FAST_FWD:
        input_report_key(mhl_state.rmt_input, KEY_FASTFORWARD, up_down);
        TX_DEBUG_PRINT(( "\nFastfwd received\n\n" ));
        break;
    case MHL_RCP_CMD_REWIND:
        input_report_key(mhl_state.rmt_input, KEY_REWIND, up_down);
        TX_DEBUG_PRINT(( "\nRewind received\n\n" ));
        break;
    case MHL_RCP_CMD_EJECT:
        input_report_key(mhl_state.rmt_input, KEY_EJECTCD, up_down);
        TX_DEBUG_PRINT(( "\nEject received\n\n" ));
        break;
    case MHL_RCP_CMD_FWD:
        input_report_key(mhl_state.rmt_input, KEY_FORWARD, up_down);
        TX_DEBUG_PRINT(( "\nForward received\n\n" ));
        break;
    case MHL_RCP_CMD_BKWD:
        input_report_key(mhl_state.rmt_input, KEY_BACK, up_down);
        TX_DEBUG_PRINT(( "\nBackward received\n\n" ));
        break;
    case MHL_RCP_CMD_PLAY_FUNC:
        //input_report_key(mhl_state.rmt_input, KEY_PL, up_down);
        TX_DEBUG_PRINT(( "\nPlay Function received\n\n" ));
    break;
    case MHL_RCP_CMD_PAUSE_PLAY_FUNC:
        //input_report_key(mhl_state.rmt_input, KEY_PLAYPAUSE, up_down);
        TX_DEBUG_PRINT(( "\nPause_Play Function received\n\n" ));
        break;
    case MHL_RCP_CMD_STOP_FUNC:
        //input_report_key(mhl_state.rmt_input, KEY_STOP, up_down);
        TX_DEBUG_PRINT(( "\nStop Function received\n\n" ));
        break;
    case MHL_RCP_CMD_F1:
        input_report_key(mhl_state.rmt_input, KEY_F1, up_down);
        TX_DEBUG_PRINT(( "\nF1 received\n\n" ));
        break;
    case MHL_RCP_CMD_F2:
        input_report_key(mhl_state.rmt_input, KEY_F2, up_down);
        TX_DEBUG_PRINT(( "\nF2 received\n\n" ));
        break;
    case MHL_RCP_CMD_F3:
        input_report_key(mhl_state.rmt_input, KEY_F3, up_down);
        TX_DEBUG_PRINT(( "\nF3 received\n\n" ));
        break;
    case MHL_RCP_CMD_F4:
        input_report_key(mhl_state.rmt_input, KEY_F4, up_down);
        TX_DEBUG_PRINT(( "\nF4 received\n\n" ));
        break;
    case MHL_RCP_CMD_F5:
        input_report_key(mhl_state.rmt_input, KEY_F5, up_down);
        TX_DEBUG_PRINT(( "\nF5 received\n\n" ));
        break;
    default:
        break;
    }	
		
    //added by zhangyue on 2011-12-28 for improving mhl RCP start
    input_sync(mhl_state.rmt_input);
    //added by zhangyue on 2011-12-28 for improving mhl RCP end
}
void input_report_mhl_rcp_key(uint8_t rcp_keycode)
{
    //added by zhangyue on 2011-12-28 for improving mhl RCP start
    input_report_rcp_key(rcp_keycode & 0x7F, 1);
    input_report_rcp_key(rcp_keycode & 0x7F, 0);
    //added by zhangyue on 2011-12-28 for improving mhl RCP end
 
    //commented by zhangyue on 2011-12-28 for improving mhl RCP start
    #if 0	
    if(rcp_keycode & 0x80){
        input_report_rcp_key(rcp_keycode & 0x7F, 1);
    }else{
        input_report_rcp_key(rcp_keycode & 0x7F, 0);
    }
    #endif
    //commented by zhangyue on 2011-12-28 for improving mhl RCP end
}
#endif
//------------------------------------------------------------------------------
//Function:     AppRcpDemo
//Description:  This function is supposed to provide a demo code to elicit how to call RCP API function.
//------------------------------------------------------------------------------

#define	APP_DEMO_RCP_SEND_KEY_CODE 0x44

void	AppRcpDemo( uint8_t event, uint8_t eventParameter)
{
    uint8_t 	rcpKeyCode;
    
    //printf("App: Got event = %02X, eventParameter = %02X\n", (int)event, (int)eventParameter);
    
    switch( event )
    {
    case	MHL_TX_EVENT_DISCONNECTION:
        printk("App: Got event = MHL_TX_EVENT_DISCONNECTION\n");
        #ifdef CONFIG_OPPO_MODIFY
        //added by zhangyue on 2011-11-01 for adding the auto hpd feature
        #ifdef zy_auto_hpd
	  //deleted by zhangyue on 2011-12-23 for debug audio problem start
        //external_common_enable_hpd_feature(false);
	  //deleted by zhangyue on 2011-12-23 for debug audio problem end
        #endif
        #endif
        break;
    
    case	MHL_TX_EVENT_CONNECTION:
        printk("App: Got event = MHL_TX_EVENT_CONNECTION\n");
        #ifdef CONFIG_OPPO_MODIFY
        //added by zhangyue on 2011-11-01 for adding the auto hpd feature
        #ifdef zy_auto_hpd
        //modified by zhangyue on 2012-01-09 for debug the first plugin mhl can not output video start
        
        //external_common_enable_hpd_feature(true);
        //modified by zhangyue on 2012-01-09 for debug the first plugin mhl can not output video start
        #endif
        #endif
        break;
    
    case	MHL_TX_EVENT_RCP_READY:
        // Demo RCP key code PLAY
        rcpKeyCode = APP_DEMO_RCP_SEND_KEY_CODE;
        printk("App: Got event = MHL_TX_EVENT_RCP_READY...Sending RCP (%02X)\n", (int) rcpKeyCode);
    
    
        if( (0 == (MHL_FEATURE_RCP_SUPPORT & eventParameter)) ){
            printk( "App: Peer does NOT support RCP\n" );
        }
        if( (0 == (MHL_FEATURE_RAP_SUPPORT & eventParameter)) ){
            printk( "App: Peer does NOT support RAP\n" );
        }
        if( (0 == (MHL_FEATURE_SP_SUPPORT & eventParameter)) ){
            printk( "App: Peer does NOT support WRITE_BURST\n" );
        }
        
        
        //
        // If RCP engine is ready, send one code
        //
        if( SiiMhlTxRcpSend( rcpKeyCode )){
            printk("App: SiiMhlTxRcpSend (%02X)\n", (int) rcpKeyCode);
        }else{
            printk("App: SiiMhlTxRcpSend (%02X) Returned Failure.\n", (int) rcpKeyCode);
        }
        
        break;
    
    case	MHL_TX_EVENT_RCP_RECEIVED:
        //
        // Check if we got an RCP. Application can perform the operation here
        // and send RCPK or RCPE. For now, we send the RCPK
        //
        #ifdef ZY_IMPLEMENT_RCP
        rcpKeyCode = eventParameter;
        #else
        rcpKeyCode = eventParameter & 0x7F;
        #endif
        printk("App1: Received an RCP key code = %02X\n", (int)rcpKeyCode );
        
        // Added RCP key printf and interface with UI. //by oscar 20101217
        #ifdef ZY_IMPLEMENT_RCP
        input_report_mhl_rcp_key(rcpKeyCode);
	  #endif
        
        SiiMhlTxRcpkSend(rcpKeyCode);
        break;
        
    case	MHL_TX_EVENT_RCPK_RECEIVED:
        printk("App: Received an RCPK = %02X\n",MHL_TX_EVENT_RCPK_RECEIVED);
        break;
    
    case	MHL_TX_EVENT_RCPE_RECEIVED:
        printk("App: Received an RCPE = %02X\n",MHL_TX_EVENT_RCPE_RECEIVED);
        break;
    
    default:
        break;
    }
}


#if (VBUS_POWER_CHK == ENABLE)
///////////////////////////////////////////////////////////////////////////////
//
// AppVbusControl
//
// This function or macro is invoked from MhlTx driver to ask application to
// control the VBUS power. If powerOn is sent as non-zero, one should assume
// peer does not need power so quickly remove VBUS power.
//
// if value of "powerOn" is 0, then application must turn the VBUS power on
// within 50ms of this call to meet MHL specs timing.
//
// Application module must provide this function.
///////////////////////////////////////////
void	AppVbusControl( bool_t powerOn )
{
    if( powerOn ){
        MHLSinkOrDonglePowerStatusCheck();
        printk("App: Peer's POW bit is set. Turn the VBUS power OFF here.\n");
    }
    else{
        printk("App: Peer's POW bit is cleared. Turn the VBUS power ON here.\n");
    }
}
#endif

#ifdef SiI8334DRIVER_INTERRUPT_MODE

struct work_struct	*sii8334work = NULL;
#ifdef mhl_power_dbg
struct delayed_work *ResumeWork = NULL;
#endif 

#ifdef zy_dbg_irq
//deleted by zhangyue for debug irq problems on 2011-12-07
#else
static spinlock_t sii8334_lock = SPIN_LOCK_UNLOCKED;
#endif
extern uint8_t	fwPowerState;

static void work_queue(struct work_struct *work)
{	
    uint8_t Int_count=0;
    uint8_t 				event;
    uint8_t 				eventParam;
    #ifdef CONFIG_OPPO_MODIFY
    //modified by zhangyue on 2011-11-12 for concentrating modified code
    //deleted by zhangyue on 2011-12-16 as Silicon had solved the first abnormal Interrrupt start
    #if 0  
    #ifdef mhl_power_dbg
    if( 0 == mhl_state.initInt){
        mhl_state.initInt = 1;
        enable_irq(sii8334_PAGE_TPI->irq);
        printk(KERN_INFO "%s:the value of mhl_state.initInt is %d", __func__, mhl_state.initInt);
        return;
    }
    #else
    //added by zhangyue for debugging the boot on problem(boot not correct) on 2011-11-14	
    if( 0 == mhlTxConfig.initInt){
        mhlTxConfig.initInt = 1;
        enable_irq(sii8334_PAGE_TPI->irq);
        printk(KERN_INFO "%s:the value of mhlTxConfig.initInt is %d", __func__, mhlTxConfig.initInt);
        return;
    }
    //modified by zhangyue on 2011-11-12 end
    #endif
    #endif
    //deleted by zhangyue on 2011-12-16 as Silicon had solved the first abnormal Interrrupt end
    #endif
    //for(Int_count=0;Int_count<15;Int_count++){
		printk(KERN_INFO "%s:%d:Int_count=%d::::::::Sii8334 interrupt happened\n", __func__,__LINE__,Int_count);
        SiiMhlTxGetEvents(&event, &eventParam);
        
        if( MHL_TX_EVENT_NONE != event ){
            AppRcpDemo( event, eventParam);
        }
        //msleep(10);	//commented by zhangy on 2011-10-17 for update the new code of Silicon
        //if(POWER_STATE_D3 == fwPowerState)
            //break;
    //}
    //modified by zhangyue on 2011-12-19 for debugging the mhl irq problem start
    mhl_set_irq_locked(true, false);
    //enable_irq(sii8334_PAGE_TPI->irq);
    //modified by zhangyue on 2011-12-19 for debugging the mhl irq problem end
}

#ifdef mhl_power_dbg
static void ResumeWorkFunc(struct work_struct *work)
{
    printk(KERN_INFO "%s Resume MHL start.\n", __func__);

	Sii8334_plat_data->reset();

	msleep(1000);
    //deleted by zhangyue on 2011-12-15 for solving the hpd problems start
    //enable_irq(sii8334_PAGE_TPI->irq);
    //deleted by zhangyue on 2011-12-15 for solving the hpd problems end
    SiiMhlTxInitialize();
    //added by zhangyue on 2011-12-19 for debugging the mhl irq problem start
    printk(KERN_INFO "%s Init Register End.\n", __func__);
    HalTimerWait(30);//wait 30ms before enable registers
    mhl_set_irq_locked(true, false);
    //added by zhangyue on 2011-12-19 for debugging the mhl irq problem end
}
#endif
static irqreturn_t Sii8334_mhl_interrupt(int irq, void *dev_id)
{
//    unsigned long lock_flags = 0;	 
    //modified by zhangyue on 2011-12-19 for debugging the mhl irq problem start
    mhl_set_irq_locked(false, true);
    //disable_irq_nosync(irq);
    //modified by zhangyue on 2011-12-19 for debugging the mhl irq problem end
    #ifdef zy_dbg_irq
    //deleted by zhangyue on 2011-12-07 for debugging irq problems
    #else
    spin_lock_irqsave(&sii8334_lock, lock_flags);
    #endif
    //printk("The sii8334 interrupt handeler is working..\n");  
    printk("The most of sii8334 interrupt work will be done by following tasklet..\n");  
    
    schedule_work(sii8334work);
    
    //printk("The sii8334 interrupt's top_half has been done and bottom_half will be processed..\n");  
    #ifdef zy_dbg_irq
    //deleted by zhangyue on 2011-12-07 for debugging irq problems
    #else
    spin_unlock_irqrestore(&sii8334_lock, lock_flags);
    #endif
    return IRQ_HANDLED;
}
#else
/*****************************************************************************/
/**
 *  @brief Thread function that periodically polls for MHLTx events.
 *
 *  @param[in]	data	Pointer to driver context structure
 *
 *  @return		Always returns zero when the thread exits.
 *
 *****************************************************************************/
static int EventThread(void *data)
{
    uint8_t 				event;
    uint8_t 				eventParam;
    
    
    printk("%s EventThread starting up\n", MHL_DRIVER_NAME);
    
    while (true){
        if (kthread_should_stop()){
            printk("%s EventThread exiting\n", MHL_DRIVER_NAME);
            break;
        }
        
        HalTimerWait(EVENT_POLL_INTERVAL_30_MS);
        
        SiiMhlTxGetEvents(&event, &eventParam);
        
        if( MHL_TX_EVENT_NONE != event ) {
            AppRcpDemo( event, eventParam);
        }
    }
    return 0;
}


/***** public functions ******************************************************/


/*****************************************************************************/
/**
 * @brief Start drivers event monitoring thread.
 *
 *****************************************************************************/
void StartEventThread(void)
{
    gDriverContext.pTaskStruct = kthread_run(EventThread,
							          &gDriverContext,
							          MHL_DRIVER_NAME);
}



/*****************************************************************************/
/**
 * @brief Stop driver's event monitoring thread.
 *
 *****************************************************************************/
void  StopEventThread(void)
{
    kthread_stop(gDriverContext.pTaskStruct);

}
#endif

static struct i2c_device_id mhl_Sii8334_idtable[] = {
    {"sii8334_PAGE_TPI", 0},
    {"sii8334_PAGE_TX_L0", 0},
    {"sii8334_PAGE_TX_L1", 0},
    {"sii8334_PAGE_TX_2", 0},
    {"sii8334_PAGE_TX_3", 0},
    {"sii8334_PAGE_CBUS", 0},
};

#ifdef ZY_IMPLEMENT_RCP
void mhl_init_rmt_input_dev(void)
{
    if(NULL == mhl_state.rmt_input){
        return;		
    }
    mhl_state.rmt_input->name = "mhl_rcp";
    set_bit(EV_KEY,mhl_state.rmt_input->evbit);
    set_bit(KEY_SELECT, mhl_state.rmt_input->keybit);
    set_bit(KEY_UP, mhl_state.rmt_input->keybit);
    set_bit(KEY_DOWN, mhl_state.rmt_input->keybit);
    set_bit(KEY_LEFT, mhl_state.rmt_input->keybit);
    set_bit(KEY_RIGHT, mhl_state.rmt_input->keybit);
    //set_bit(KEY_RIGHT_UP, mhl_state.rmt_input->keybit);
    //set_bit(KEY_RIGHT_DOWN , mhl_state.rmt_input->keybit);
    //set_bit(KEY_LEFT_UP,mhl_state.rmt_input->keybit);
    //set_bit(KEY_LEFT_DOWN, mhl_state.rmt_input->keybit);
	
    set_bit(KEY_MENU, mhl_state.rmt_input->keybit);
    //set_bit(KEY_SETUP, mhl_state.rmt_input->keybit);
    
    set_bit(KEY_EXIT, mhl_state.rmt_input->keybit);
	
    //set_bit(KEY_CONTEXT_MENU ,mhl_state.rmt_input->keybit);
    //set_bit(KEY_FAVORITES, mhl_state.rmt_input->keybit);
    //set_bit(KEY_EXIT, mhl_state.rmt_input->keybit);
    set_bit(KEY_NUMERIC_0, mhl_state.rmt_input->keybit);
    set_bit(KEY_NUMERIC_1,mhl_state.rmt_input->keybit);
    set_bit(KEY_NUMERIC_2, mhl_state.rmt_input->keybit);
    set_bit(KEY_NUMERIC_3, mhl_state.rmt_input->keybit);
    set_bit(KEY_NUMERIC_4, mhl_state.rmt_input->keybit);
    set_bit(KEY_NUMERIC_5, mhl_state.rmt_input->keybit);
    set_bit(KEY_NUMERIC_6,mhl_state.rmt_input->keybit);
    set_bit(KEY_NUMERIC_7, mhl_state.rmt_input->keybit);
    set_bit(KEY_NUMERIC_8, mhl_state.rmt_input->keybit);
    set_bit(KEY_NUMERIC_9, mhl_state.rmt_input->keybit);
	
    //set_bit(KEY_DOT, mhl_state.rmt_input->keybit);
    set_bit(KEY_ENTER, mhl_state.rmt_input->keybit);
    set_bit(KEY_CLEAR, mhl_state.rmt_input->keybit);
    //set_bit(KEY_CHANNELUP, mhl_state.rmt_input->keybit);
    //set_bit(KEY_CHANNELDOWN, mhl_state.rmt_input->keybit);
    //set_bit(KEY_CHANNEL_PREV, mhl_state.rmt_input->keybit);
    
    set_bit(KEY_SOUND, mhl_state.rmt_input->keybit);
    //set_bit(KEY_INFO, mhl_state.rmt_input->keybit);
    //set_bit(KEY_HELP, mhl_state.rmt_input->keybit);
    //set_bit(KEY_PAGEUP,mhl_state.rmt_input->keybit);
    //set_bit(KEY_PAGEDOWN, mhl_state.rmt_input->keybit);
    //set_bit(KEY_VOLUMEUP, mhl_state.rmt_input->keybit);
    //set_bit(KEY_VOLUMEDOWN, mhl_state.rmt_input->keybit);
    //set_bit(KEY_MUTE, mhl_state.rmt_input->keybit);
    
    set_bit(KEY_PLAY, mhl_state.rmt_input->keybit);
    set_bit(KEY_STOP, mhl_state.rmt_input->keybit);
    set_bit(KEY_PAUSE, mhl_state.rmt_input->keybit);
	
    //set_bit(KEY_RECORD,mhl_state.rmt_input->keybit);
    
    set_bit(KEY_REWIND, mhl_state.rmt_input->keybit);
    set_bit(KEY_FASTFORWARD, mhl_state.rmt_input->keybit);
    set_bit(KEY_EJECTCD, mhl_state.rmt_input->keybit);
    set_bit(KEY_FORWARD, mhl_state.rmt_input->keybit);
    set_bit(KEY_BACK, mhl_state.rmt_input->keybit);
	
    //set_bit(KEY_ANGLE, mhl_state.rmt_input->keybit);
    //set_bit(KEY_RESTART, mhl_state.rmt_input->keybit);
    //set_bit(KEY_PLAYPAUSE, mhl_state.rmt_input->keybit);
}

#endif
/*
 * i2c client ftn.
 */
static int __devinit mhl_Sii8334_probe(struct i2c_client *client,
			const struct i2c_device_id *dev_id)
{
    int ret = 0;
    if(match_id(&mhl_Sii8334_idtable[0], client)){
        sii8334_PAGE_TPI = client;
        dev_info(&client->adapter->dev, "attached %s "
                "into i2c adapter successfully\n", dev_id->name);
    }
    /*
    else if(match_id(&mhl_Sii8334_idtable[1], client)){
        sii8334_PAGE_TX_L0 = client;
        dev_info(&client->adapter->dev, "attached %s "
                "into i2c adapter successfully \n", dev_id->name);
    }
    */
    else if(match_id(&mhl_Sii8334_idtable[2], client)){
        sii8334_PAGE_TX_L1 = client;
        dev_info(&client->adapter->dev, "attached %s "
                "into i2c adapter successfully \n", dev_id->name);
    }else if(match_id(&mhl_Sii8334_idtable[3], client)){
        sii8334_PAGE_TX_2 = client;
        dev_info(&client->adapter->dev, "attached %s "
                "into i2c adapter successfully\n", dev_id->name);
    }else if(match_id(&mhl_Sii8334_idtable[4], client)){
        sii8334_PAGE_TX_3 = client;
        dev_info(&client->adapter->dev, "attached %s "
                "into i2c adapter successfully\n", dev_id->name);
    }else if(match_id(&mhl_Sii8334_idtable[5], client))    {
        sii8334_PAGE_CBUS = client;
        dev_info(&client->adapter->dev, "attached %s "
            "into i2c adapter successfully\n", dev_id->name);
    }else{
        dev_info(&client->adapter->dev, "invalid i2c adapter: can not found dev_id matched\n");
        return -EIO;
    }
    
    
    if(sii8334_PAGE_TPI != NULL 
      //&&sii8334_PAGE_TX_L0 != NULL 
      &&sii8334_PAGE_TX_L1 != NULL 
      && sii8334_PAGE_TX_2 != NULL
      && sii8334_PAGE_TX_3 != NULL
      && sii8334_PAGE_CBUS != NULL){
        
        Sii8334_plat_data = sii8334_PAGE_TPI->dev.platform_data;
        #ifdef mhl_power_dbg
        ret = Sii8334_plat_data->mhl_power_on();
        if(ret){
            printk("%s:call mhl_power_on failed.\n", __func__);	
        }
	  //added by zhangyue on 2011-11-21 for config irq	
	  Sii8334_plat_data->mhl_irq_config();
	  //added by zhangyue on 2011-11-21 end
        #endif
        if(false == Sii8334_mhl_reset()){
            printk("/nCan't find the reset function in your platform file============================================\n");
            return -EIO;
        }
        
        // Announce on RS232c port.
        //
        printk("\n============================================\n");
        printk("SiI-8334 Driver Version based on 8051 driver Version 1.14 \n");
        printk("============================================\n");
        
        //
        // Initialize the registers as required. Setup firmware vars.
        //
        
	  SiiMhlTxInitialize();
        
        #ifdef SiI8334DRIVER_INTERRUPT_MODE
        #ifdef zy_dbg_err_handle
        sii8334work = kmalloc(sizeof(*sii8334work), GFP_KERNEL);
         if(NULL == sii8334work){
            printk(KERN_ERR "NO memory for allocating sii8334work.\n");
            ret = -ENOMEM;
            goto ERR_ALLOC_INT_WORK;
        }
        #else
        sii8334work = kmalloc(sizeof(*sii8334work), GFP_ATOMIC);
        #endif
        INIT_WORK(sii8334work, work_queue); 
        #ifdef mhl_power_dbg
        #ifdef zy_dbg_err_handle
        ResumeWork = kmalloc(sizeof(*ResumeWork ),  GFP_ATOMIC);
        if(NULL == ResumeWork){
            printk(KERN_ERR "NO memory for allocating ResumeWork.\n");
	      ret = -ENOMEM;
            goto ERR_ALLOC_RESUM_WORK;
        }
        #else
        ResumeWork = kmalloc(sizeof(*ResumeWork ),  GFP_ATOMIC);
        #endif
        INIT_DELAYED_WORK(ResumeWork, ResumeWorkFunc);
        #endif
        #ifdef ZY_IMPLEMENT_RCP
        mhl_state.rmt_input = input_allocate_device();
        if(NULL == mhl_state.rmt_input){
            printk(KERN_INFO "Bad input_allocate_device()\n");
	      ret = -ENOMEM;
	      goto ERR_ALLOC_INPUT;
        }
	
        mhl_init_rmt_input_dev();
        //Register with the Input subsystem	
        ret = input_register_device(mhl_state.rmt_input);
        if(ret){
            printk(KERN_INFO "Register input device failed.\n ");
            goto ERR_REG_INPUT;
        }
        #endif
        #ifdef zy_dbg_irq
	  //modified by zhangyue on 2011-12-07 
	  //for debugging, because some time, the irq handler not run when the irq came.
	  ret = request_threaded_irq(sii8334_PAGE_TPI->irq, NULL, Sii8334_mhl_interrupt, IRQ_TYPE_LEVEL_LOW |IRQF_ONESHOT, 
        	sii8334_PAGE_TPI->name, sii8334_PAGE_TPI); 
	 #else
        ret = request_irq(sii8334_PAGE_TPI->irq, Sii8334_mhl_interrupt, IRQ_TYPE_LEVEL_LOW,
        sii8334_PAGE_TPI->name, sii8334_PAGE_TPI); 
	  #endif
	  
        if (ret){
            printk(KERN_INFO "%s:%d:Sii8334 interrupt failed\n", __func__,__LINE__);	
            #ifdef zy_dbg_err_handle
            goto ERR_REQ_IRQ;
            #endif
            //free_irq(irq, iface);
        }else{
            #ifdef zy_usrspace_enalbe_mhl_irq
		//modified by zhangyue on 2011-12-26 for debugging the mhl charging	start
		disable_irq(sii8334_PAGE_TPI->irq);
		HalTimerWait(40);//sleep 40ms
		enable_irq(sii8334_PAGE_TPI->irq);
            //disable_irq(sii8334_PAGE_TPI->irq);
		//modified by zhangyue on 2011-12-26 for debugging the mhl charging	end
	      #else
            enable_irq_wake(sii8334_PAGE_TPI->irq);
	      #endif  
            //printk(KERN_INFO "%s:%d:Sii8334 interrupt successed\n", __func__,__LINE__);	
        }
        #ifdef zy_usrspace_enalbe_mhl_irq
        mhl_state.irq_Inited = false;
	 #endif
        #ifdef mhl_power_dbg
        mhl_state.state = true;
        #endif
        #else
        StartEventThread();		/* begin monitoring for events if using polling mode*/
        #endif
        #ifdef zy_dbg_err_handle
        goto ERR_ALLOC_INT_WORK;
        #endif
    ERR_REQ_IRQ:
        input_unregister_device(mhl_state.rmt_input);
    ERR_REG_INPUT:
        input_free_device(mhl_state.rmt_input);
    ERR_ALLOC_INPUT:
        kfree(ResumeWork);
    ERR_ALLOC_RESUM_WORK:
        kfree(sii8334work);
    }
ERR_ALLOC_INT_WORK:
    return ret;
}

#ifdef zy_usrspace_enalbe_mhl_irq
bool read_mhl_irq_Init_state()
{
	return mhl_state.irq_Inited;
}
int write_mhl_irq_Init_state()
{
	if(mhl_state.irq_Inited)
		return 0;
	//enable_irq(sii8334_PAGE_TPI->irq);
	external_common_enable_hpd_feature(true);
	mhl_state.irq_Inited = true;
	return 0;
}
#endif
static int mhl_Sii8334_remove(struct i2c_client *client)
{
    dev_info(&client->adapter->dev, "detached s5p_mhl "
             "from i2c adapter successfully\n");
    
    return 0;
}

static int mhl_Sii8334_suspend(struct i2c_client *cl, pm_message_t mesg)
{
#ifdef mhl_power_dbg
    int ret;

    mhl_state.state = false;
    if(match_id(&mhl_Sii8334_idtable[5], cl)){
	  //modified by zhangyue on 2011-11-21 for debugging
        //disable_irq_wake(sii8334_PAGE_TPI->irq);
        //modified by zhangyue on 2011-12-19 for debug mhl irq problem start
        //mhl_set_irq_locked(false, false);
        //disable_irq(sii8334_PAGE_TPI->irq);
        //modified by zhangyue on 2011-12-19 for debug mhl irq problem end
	  //modified by zhangyue on 2011-11-21 end
	  //added by zhangyue on 2011-11-22 for cancel the ResumeWork func thread.
	  //as here, we decide to power off mhl module.
	  
	  ret = cancel_delayed_work_sync(ResumeWork);
        mhl_set_irq_locked(false, false);
	  		
	  if(ret){
            //printk(" %s Cancel the delayed work failed, the ret value is %d.\n", __func__, ret);
	  }
        if(Sii8334_plat_data->mhl_power_off == NULL){
            printk(" func %s is null.\n", __func__);
        }
		
        ret = Sii8334_plat_data->mhl_power_off();
         if(ret){
            printk("%s call mhl_power_off failed.\n", __func__);
            return ret;
        }
    }
#endif
    return 0;
};

static int mhl_Sii8334_resume(struct i2c_client *cl)
{
#ifdef mhl_power_dbg
    int ret;
    if(match_id(&mhl_Sii8334_idtable[5], cl))	{
        ret = Sii8334_plat_data->mhl_power_on();
	  if(ret){
            printk("%s call mhl_power_on  failed, return value is  %d \n", __func__, ret);
            return ret;    
        }
    	  mhl_state.state = true;
	  //HalTimerWait(5);//sleep 5 ms
/* OPPO 2012-07-20 wangjc Delete begin for reason */
#if 0
	  Sii8334_plat_data->reset();
#endif
/* OPPO 2012-07-20 wangjc Delete end */
  	  //added by zhangyue on 2011-12-15 for debugging the hpd problem start
  	  //deleted by zhangyue on 2011-12-19 for debugging mhl irq problems start
	  //enable_irq(sii8334_PAGE_TPI->irq);
	  //deleted by zhangyue on 2011-12-19 for debugging mhl irq problems end
	  //added by zhangyue on 2011-12-15 for debugging the hpd problem end
        schedule_delayed_work(ResumeWork, MHL_RESUME_DELAY);    
    }
#endif
    return 0;
};

#ifdef CONFIG_OPPO_MODIFY
//added by zhangyue on 2012-01-12 for tackling the camera and mhl coexist problem start
bool is_mhl_connected()
{
	return mhl_state.mhl_cnt_st && mhl_state.irq_Inited;
}
EXPORT_SYMBOL(is_mhl_connected);
//added by zhangyue on 2012-01-12 for tackling the camera and mhl coexist problem start
#endif
MODULE_DEVICE_TABLE(i2c, mhl_Sii8334_idtable);

static struct i2c_driver mhl_Sii8334_driver = {
    .driver = {
        .name = "Sii8334_Driver",
    },
    .id_table 	= mhl_Sii8334_idtable,
    .probe 		= mhl_Sii8334_probe,
    .remove 	= __devexit_p(mhl_Sii8334_remove),
    
    .suspend	= mhl_Sii8334_suspend,
    .resume 	= mhl_Sii8334_resume,
};

static int __init mhl_Sii8334_init(void)
{
    return i2c_add_driver(&mhl_Sii8334_driver);
}

static void __exit mhl_Sii8334_exit(void)
{
    i2c_del_driver(&mhl_Sii8334_driver);
}


late_initcall(mhl_Sii8334_init);
module_exit(mhl_Sii8334_exit);

MODULE_VERSION("1.0");
MODULE_AUTHOR("gary <qiang.yuan@siliconimage.com>, Silicon image SZ office, Inc.");
MODULE_DESCRIPTION("sii8334 transmitter Linux driver");
MODULE_ALIAS("platform:MHL_sii8334");

