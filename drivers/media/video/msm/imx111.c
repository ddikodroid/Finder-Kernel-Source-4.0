/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <mach/gpio.h>
#include <mach/camera.h>
#include "imx111.h"


#define REG_GROUPED_PARAMETER_HOLD			    0x0104
#define GROUPED_PARAMETER_HOLD_OFF			    0x00
#define GROUPED_PARAMETER_HOLD				    0x01
#define REG_MODE_SELECT						    0x0100
#define MODE_SELECT_STANDBY_MODE			    0x00
#define MODE_SELECT_STREAM					    0x01
/* Integration Time */
#define REG_COARSE_INTEGRATION_TIME_HI		    0x0202
#define REG_COARSE_INTEGRATION_TIME_LO		    0x0203
/* Gain */
#define REG_ANALOGUE_GAIN_CODE_GLOBAL_HI	    0x0204
#define REG_ANALOGUE_GAIN_CODE_GLOBAL_LO	    0x0205

/* mode setting */
#define REG_FRAME_LENGTH_LINES_HI               0x0340
#define REG_FRAME_LENGTH_LINES_LO	            0x0341

#define	IMX111_STEPS_NEAR_TO_CLOSEST_INF		30 //42
#define	IMX111_TOTAL_STEPS_NEAR_TO_FAR			30 //42

#define REG_VCM_DAMP_STBY				        0x3400
#define REG_VCM_CODE_LO_8BITS			        0x3402
#define REG_VCM_CODE_HI_2BITS			        0x3403
#define VCM_ENABLE				                0x00
#define VCM_DISABLE				                0x02
#define VCM_DAMP_CNTRL_ON		                0x01
#define VCM_DAMP_CNTRL_OFF		                0x00

/*-----------------zhangkw add start for sony module ------------*/
#define REG_BANK_ADDR                           0x34C9

#define REG_INF_AF_POSITION_H                   0x350E
#define REG_INF_AF_POSITION_L                   0x350F

#define REG_1M_AF_POSITION_H                    0x3510
#define REG_1M_AF_POSITION_L                    0x3511

#define REG_100MM_AF_POSITION_H                 0x3512
#define REG_100MM_AF_POSITION_L                 0x3513

#define REG_START_CURRENT_H                     0x3514
#define REG_START_CURRENT_L                     0x3515

#define REG_SENSITIVITY_H                       0x3516
#define REG_SENSITIVITY_L                       0x3517

static uint16_t imx111_vcm_start_current;
static uint16_t imx111_vcm_end_current ;
/*--------------zhangkw add end for sony module--------------------*/

#define VCM_I2C_ADDR 	                        0x0C
#define imx111_nl_region_boundary1  		    3

// SONY Module
#define imx111_nl_region_code_per_step1         (imx111_vcm_start_current /imx111_nl_region_boundary1)
#define imx111_l_region_code_per_step           ((1023-imx111_nl_region_code_per_step1\
									                    *imx111_nl_region_boundary1) \
									                  /IMX111_TOTAL_STEPS_NEAR_TO_FAR)

// LITEON Module 
#define VCM_START_CODE 	                        160
#define VCM_END_CODE	                        800
#define VCM_THRETH 		                        160   //200//336
#define imx111_nl_region_code_per_step_adi5823  (VCM_START_CODE / imx111_nl_region_boundary1)   
#define imx111_l_region_code_per_step_adi5823   ((VCM_END_CODE - VCM_START_CODE                 \
                                                  + IMX111_TOTAL_STEPS_NEAR_TO_FAR              \
                                                  - imx111_nl_region_boundary1 - 2)             \
                                                  / (IMX111_TOTAL_STEPS_NEAR_TO_FAR - imx111_nl_region_boundary1 - 1))


static uint8_t  damping_threshold = 50;
static uint8_t  damping_fine_step = 25;

static uint32_t VCM_TRES = 13333;           //response period. in uS. 75Hz
static uint16_t imx111_step_position_table[IMX111_TOTAL_STEPS_NEAR_TO_FAR+1];

#define IMX111_DRIVER_DBG 0
#undef CDBG
#if IMX111_DRIVER_DBG
#define CDBG(fmt, args...) printk(KERN_INFO "msm_camera_kernel: " fmt, ##args)
#else
#define CDBG(fmt, args...) do{}while(0)
#endif

#define VCM_USE_DW9714L 1
/*============================================================================
							 TYPE DECLARATIONS
============================================================================*/

/* 16bit address - 8 bit context register structure */
#define	IMX111_OFFSET	                        5
#define	Q8		                                0x00000100
#define	IMX111_DEFAULT_MASTER_CLK_RATE	        24000000

/* Full	Size */
#define	IMX111_FULL_SIZE_WIDTH                  3280
#define	IMX111_FULL_SIZE_HEIGHT		            2464
#define	IMX111_HRZ_FULL_BLK_PIXELS	            256
#define	IMX111_VER_FULL_BLK_LINES	            38    //26
#define	IMX111_FULL_SIZE_DUMMY_PIXELS	        0
#define	IMX111_FULL_SIZE_DUMMY_LINES	        0

#define	IMX111_VIDEO_SIZE_WIDTH	                2104  //3280
#define	IMX111_VIDEO_SIZE_HEIGHT	            1184  //2464
#define	IMX111_VIDEO_SIZE_DUMMY_PIXELS	        0
#define	IMX111_VIDEO_SIZE_DUMMY_LINES		    0
#define	IMX111_HRZ_VIDEO_BLK_PIXELS	            1432  //256
#define	IMX111_VER_VIDEO_BLK_LINES	            66    //26

/* Quarter Size	*/
#define	IMX111_QTR_SIZE_WIDTH	                1640  //3280
#define	IMX111_QTR_SIZE_HEIGHT	                1232  //2464
#define	IMX111_QTR_SIZE_DUMMY_PIXELS	        0
#define	IMX111_QTR_SIZE_DUMMY_LINES		        0
#define	IMX111_HRZ_QTR_BLK_PIXELS	            256   //128//256
#define	IMX111_VER_QTR_BLK_LINES	            22    //18//26


struct imx111_work_t {
	struct work_struct work;
};

static struct imx111_work_t *imx111_sensorw;
static struct i2c_client *imx111_client;
static int32_t config_csi;


struct imx111_ctrl_t {
	const struct  msm_camera_sensor_info *sensordata;

	uint32_t sensormode;
	uint32_t fps_divider;	    /* init to 1 * 0x00000400 */
	uint32_t pict_fps_divider;	/* init to 1 * 0x00000400 */
	uint16_t fps;

	int16_t curr_lens_pos;
	int16_t curr_step_pos;
	uint16_t my_reg_gain;
	uint32_t my_reg_line_count;
	uint16_t total_lines_per_frame;
    #if 0
	enum imx111_resolution_t prev_res;
	enum imx111_resolution_t pict_res;
	enum imx111_resolution_t curr_res;
    #else
	uint16_t prev_res;
	uint16_t pict_res;
	uint16_t curr_res;
	#endif
	enum imx111_test_mode_t  set_test;
	unsigned short imgaddr;
};

static int8_t camera_module_type = CAMERA_MODULE_SONY;

static uint8_t imx111_delay_msecs_stdby = 5;
static uint16_t imx111_delay_msecs_stream = 10;

static struct imx111_ctrl_t *imx111_ctrl;
static DECLARE_WAIT_QUEUE_HEAD(imx111_wait_queue);
DEFINE_MUTEX(imx111_mut);

/*=============================================================*/

static int imx111_i2c_rxdata(unsigned short saddr,
	unsigned char *rxdata, int length)
{
	struct i2c_msg msgs[] = {
		{
			.addr  = saddr,
			.flags = 0,
			.len   = 2,
			.buf   = rxdata,
		},
		{
			.addr  = saddr,
			.flags = I2C_M_RD,
			.len   = 2,
			.buf   = rxdata,
		},
	};
	if (i2c_transfer(imx111_client->adapter, msgs, 2) < 0) {
		CDBG("imx111_i2c_rxdata failed!\n");
		return -EIO;
	}
	return 0;
}

static int32_t imx111_i2c_txdata(unsigned short saddr,
				unsigned char *txdata, int length)
{
	struct i2c_msg msg[] = {
		{
			.addr = saddr,
			.flags = 0,
			.len = length,
			.buf = txdata,
		 },
	};
	if (i2c_transfer(imx111_client->adapter, msg, 1) < 0) {
		CDBG("imx111_i2c_txdata faild 0x%x\n", imx111_client->addr);
		return -EIO;
	}

	return 0;
}

static int32_t imx111_i2c_read(unsigned short raddr,
	unsigned short *rdata, int rlen)
{
	int32_t rc = 0;
	unsigned char buf[2];
	if (!rdata)
		return -EIO;
	memset(buf, 0, sizeof(buf));
	buf[0] = (raddr & 0xFF00) >> 8;
	buf[1] = (raddr & 0x00FF);
	rc = imx111_i2c_rxdata(imx111_client->addr, buf, rlen);
	if (rc < 0) {
		CDBG("imx111_i2c_read 0x%x failed!\n", raddr);
		return rc;
	}
	*rdata = (rlen == 2 ? buf[0] << 8 | buf[1] : buf[0]);
	return rc;
}

static int32_t imx111_i2c_write_b_sensor(unsigned short waddr, uint8_t bdata)
{
	int32_t rc = -EFAULT;
	unsigned char buf[3];
	memset(buf, 0, sizeof(buf));
	buf[0] = (waddr & 0xFF00) >> 8;
	buf[1] = (waddr & 0x00FF);
	buf[2] = bdata;
	rc = imx111_i2c_txdata(imx111_client->addr, buf, 3);
	if (rc < 0) {
		CDBG("i2c_write_b failed, addr = 0x%x, val = 0x%x!\n",
			waddr, bdata);
	}
	return rc;
}

static int32_t imx111_i2c_write_w_table(struct imx111_i2c_reg_conf const
					 *reg_conf_tbl, int num)
{
	int i;
	int32_t rc = -EIO;
	for (i = 0; i < num; i++) {
		rc = imx111_i2c_write_b_sensor(reg_conf_tbl->waddr,
			reg_conf_tbl->wdata);
		if (rc < 0)
			break;
		reg_conf_tbl++;
	}
	return rc;
}

static void imx111_get_pict_fps(uint16_t fps, uint16_t *pfps)
{
	/* input fps is preview fps in Q8 format */
	uint16_t preview_frame_length_lines, snapshot_frame_length_lines;
	uint32_t divider;
	/* Total frame_length_lines for preview */
	preview_frame_length_lines = IMX111_QTR_SIZE_HEIGHT +
		IMX111_VER_QTR_BLK_LINES;
	/* Total frame_length_lines for snapshot */
	snapshot_frame_length_lines = IMX111_FULL_SIZE_HEIGHT +
		IMX111_VER_FULL_BLK_LINES;

	divider = preview_frame_length_lines * 0x00010000/
		snapshot_frame_length_lines;

	/*Verify PCLK settings and frame sizes.*/
	*pfps = (uint16_t) ((uint32_t)fps * divider / 0x10000);
	/* 2 is the ratio of no.of snapshot channels
	to number of preview channels */

}

static uint16_t imx111_get_prev_lines_pf(void)
{
	if (imx111_ctrl->prev_res == SENSOR_QTR_SIZE)
		return IMX111_QTR_SIZE_HEIGHT + IMX111_VER_QTR_BLK_LINES;
	else if (imx111_ctrl->prev_res == SENSOR_1080P_SIZE)
		return IMX111_VIDEO_SIZE_HEIGHT + IMX111_VER_VIDEO_BLK_LINES;
	else
		return IMX111_FULL_SIZE_HEIGHT + IMX111_VER_FULL_BLK_LINES;

}

static uint16_t imx111_get_prev_pixels_pl(void)
{
	if (imx111_ctrl->prev_res == SENSOR_QTR_SIZE)
		return IMX111_QTR_SIZE_WIDTH + IMX111_HRZ_QTR_BLK_PIXELS;
	else if (imx111_ctrl->prev_res == SENSOR_QTR_SIZE)
		return IMX111_VIDEO_SIZE_WIDTH + IMX111_HRZ_VIDEO_BLK_PIXELS;
	else
		return IMX111_FULL_SIZE_WIDTH + IMX111_HRZ_FULL_BLK_PIXELS;
}

static uint16_t imx111_get_pict_lines_pf(void)
{
		if (imx111_ctrl->pict_res == SENSOR_QTR_SIZE)
			return IMX111_QTR_SIZE_HEIGHT +
				IMX111_VER_QTR_BLK_LINES;
		else if (imx111_ctrl->pict_res == SENSOR_1080P_SIZE)
			return IMX111_VIDEO_SIZE_HEIGHT +
				IMX111_VER_VIDEO_BLK_LINES;
		else
			return IMX111_FULL_SIZE_HEIGHT +
				IMX111_VER_FULL_BLK_LINES;
}

static uint16_t imx111_get_pict_pixels_pl(void)
{
	if (imx111_ctrl->pict_res == SENSOR_QTR_SIZE)
		return IMX111_QTR_SIZE_WIDTH +
			IMX111_HRZ_QTR_BLK_PIXELS;
	else if (imx111_ctrl->pict_res == SENSOR_1080P_SIZE)
		return IMX111_VIDEO_SIZE_WIDTH +
			IMX111_HRZ_VIDEO_BLK_PIXELS;
	else
		return IMX111_FULL_SIZE_WIDTH +
			IMX111_HRZ_FULL_BLK_PIXELS;
}

static uint32_t imx111_get_pict_max_exp_lc(void)
{
	if (imx111_ctrl->pict_res == SENSOR_QTR_SIZE)
		return (IMX111_QTR_SIZE_HEIGHT +
			IMX111_VER_QTR_BLK_LINES)*24;
	else if (imx111_ctrl->pict_res == SENSOR_1080P_SIZE)
		return (IMX111_VIDEO_SIZE_HEIGHT +
			IMX111_VER_VIDEO_BLK_LINES)*24;
	else
		return (IMX111_FULL_SIZE_HEIGHT +
			IMX111_VER_FULL_BLK_LINES)*24;
}

static int32_t imx111_set_fps(struct fps_cfg	*fps)
{
	uint16_t total_lines_per_frame;
	int32_t rc = 0;
	imx111_ctrl->fps_divider = fps->fps_div;
	imx111_ctrl->pict_fps_divider = fps->pict_fps_div;

	if (imx111_ctrl->curr_res  == SENSOR_QTR_SIZE)
		total_lines_per_frame = (uint16_t)(((IMX111_QTR_SIZE_HEIGHT +
		IMX111_VER_QTR_BLK_LINES) *
		imx111_ctrl->fps_divider) / 0x400);
	else if (imx111_ctrl->curr_res  == SENSOR_1080P_SIZE)
		total_lines_per_frame = (uint16_t)(((IMX111_VIDEO_SIZE_HEIGHT +
		IMX111_VER_VIDEO_BLK_LINES) *
		imx111_ctrl->fps_divider) / 0x400);
	else
		total_lines_per_frame = (uint16_t)(((IMX111_FULL_SIZE_HEIGHT +
			IMX111_VER_FULL_BLK_LINES) *
			imx111_ctrl->pict_fps_divider) / 0x400);

	rc = imx111_i2c_write_b_sensor(REG_FRAME_LENGTH_LINES_HI,
		((total_lines_per_frame & 0xFF00) >> 8));

	rc = imx111_i2c_write_b_sensor(REG_FRAME_LENGTH_LINES_LO,
		(total_lines_per_frame & 0x00FF));

	return rc;
}

static int32_t imx111_write_exp_gain(uint16_t gain, uint32_t line)
{
	static uint16_t max_legal_gain  = 0x00F0;
	uint8_t gain_msb, gain_lsb;
	uint8_t intg_time_msb, intg_time_lsb;
	uint8_t frame_length_line_msb, frame_length_line_lsb;
	uint16_t frame_length_lines;
	int32_t rc = -1;

	CDBG("imx111_write_exp_gain : gain = %d line = %d", gain, line);
	if (imx111_ctrl->curr_res  == SENSOR_QTR_SIZE)
		frame_length_lines = IMX111_QTR_SIZE_HEIGHT +
			IMX111_VER_QTR_BLK_LINES;
	else if (imx111_ctrl->curr_res  == SENSOR_1080P_SIZE)
		frame_length_lines = IMX111_VIDEO_SIZE_HEIGHT +
			IMX111_VER_VIDEO_BLK_LINES;
	else
		frame_length_lines = IMX111_FULL_SIZE_HEIGHT +
			IMX111_VER_FULL_BLK_LINES;

	if (line > (frame_length_lines - IMX111_OFFSET))
		frame_length_lines = line + IMX111_OFFSET;

	CDBG("imx111 setting line = %d\n", line);


	CDBG("imx111 setting frame_length_lines = %d\n",
					frame_length_lines);

	if (gain > max_legal_gain)
		/* range: 0 to 224 */
		gain = max_legal_gain;

	/* update gain registers */
	gain_msb = (uint8_t) ((gain & 0xFF00) >> 8);
	gain_lsb = (uint8_t) (gain & 0x00FF);

	frame_length_line_msb = (uint8_t) ((frame_length_lines & 0xFF00) >> 8);
	frame_length_line_lsb = (uint8_t) (frame_length_lines & 0x00FF);

	/* update line count registers */
	intg_time_msb = (uint8_t) ((line & 0xFF00) >> 8);
	intg_time_lsb = (uint8_t) (line & 0x00FF);

	rc = imx111_i2c_write_b_sensor(REG_GROUPED_PARAMETER_HOLD,
					GROUPED_PARAMETER_HOLD);
	if (rc < 0)
		return rc;
	CDBG("imx111 setting REG_ANALOGUE_GAIN_CODE_GLOBAL_HI = 0x%X\n",
					gain_msb);
	rc = imx111_i2c_write_b_sensor(REG_ANALOGUE_GAIN_CODE_GLOBAL_HI,
					gain_msb);
	if (rc < 0)
		return rc;
	CDBG("imx111 setting REG_ANALOGUE_GAIN_CODE_GLOBAL_LO = 0x%X\n",
					gain_lsb);
	rc = imx111_i2c_write_b_sensor(REG_ANALOGUE_GAIN_CODE_GLOBAL_LO,
					gain_lsb);
	if (rc < 0)
		return rc;

	CDBG("imx111 setting REG_FRAME_LENGTH_LINES_HI = 0x%X\n",
					frame_length_line_msb);
	rc = imx111_i2c_write_b_sensor(REG_FRAME_LENGTH_LINES_HI,
			frame_length_line_msb);
	if (rc < 0)
		return rc;

	CDBG("imx111 setting REG_FRAME_LENGTH_LINES_LO = 0x%X\n",
			frame_length_line_lsb);
	rc = imx111_i2c_write_b_sensor(REG_FRAME_LENGTH_LINES_LO,
			frame_length_line_lsb);
	if (rc < 0)
		return rc;

	CDBG("imx111 setting REG_COARSE_INTEGRATION_TIME_HI = 0x%X\n",
					intg_time_msb);
	rc = imx111_i2c_write_b_sensor(REG_COARSE_INTEGRATION_TIME_HI,
					intg_time_msb);
	if (rc < 0)
		return rc;

	CDBG("imx111 setting REG_COARSE_INTEGRATION_TIME_LO = 0x%X\n",
					intg_time_lsb);
	rc = imx111_i2c_write_b_sensor(REG_COARSE_INTEGRATION_TIME_LO,
					intg_time_lsb);
	if (rc < 0)
		return rc;

	rc = imx111_i2c_write_b_sensor(REG_GROUPED_PARAMETER_HOLD,
					GROUPED_PARAMETER_HOLD_OFF);
	if (rc < 0)
		return rc;

	return rc;
}

static int32_t imx111_set_pict_exp_gain(uint16_t gain, uint32_t line)
{
	int32_t rc = 0;
	rc = imx111_write_exp_gain(gain, line);
	return rc;
}

// SONY IMX111 Module 
static int dw9714l_write(u8 val_high, u8 val_low)
{
	u8 buf[2];
	struct i2c_msg msg[] = {
		{
		  .addr  = VCM_I2C_ADDR,
		  .flags = 0,
		  .len   = 2,
		  .buf   = buf,
		},
	};

	buf[0] = val_high;
	buf[1] = val_low;

	if (!imx111_client) {
		pr_err("%s : imx111_client not initialized!\n", __func__);
		return -EIO;
	}
	if (i2c_transfer(imx111_client->adapter, msg, 1) < 0) {
		pr_err("%s : faild 0x%x\n", __func__, VCM_I2C_ADDR);
		return -EIO;
	}

	/*oppo zhangkw add for teset*/
	//printk("%s() buf[0] buf[1]: 0x%x 0x%x\n", __func__, buf[0], buf[1]);

	return 0;
}

// LITEON Module
static int ad5823_read(u8 reg)
{
	u8 buf[1];
	struct i2c_msg msgs[] = {
		{
			.addr  = VCM_I2C_ADDR,
			.flags = 0,
			.len   = 1,
			.buf   = buf,
		},
		{
			.addr  = VCM_I2C_ADDR,
			.flags = I2C_M_RD,
			.len   = 1,
			.buf   = buf,
		},
	};

	buf[0] = reg;
	if (!imx111_client) {
		CDBG("ad5823_write : imx111_client not initialized!\n");
		return -EIO;
	}
	
	if (i2c_transfer(imx111_client->adapter, msgs, 2) < 0) {
		CDBG("ad5823_read faild 0x%x\n", VCM_I2C_ADDR);
		return -EIO;
	}
	return buf[0];
}

static int ad5823_write(u8 reg, u8 val)
{
	u8 buf[2];
	struct i2c_msg msg[] = {
		{
		 .addr = VCM_I2C_ADDR,
		 .flags = 0,
		 .len = 2,
		 .buf = buf,
		 },
	};

	buf[0] = reg;
	buf[1] = val;

	if (!imx111_client) {
		CDBG("ad5823_write : imx111_client not initialized!\n");
		return -EIO;
	}
	if (i2c_transfer(imx111_client->adapter, msg, 1) < 0) {
		CDBG("ad5823_write faild 0x%x\n", VCM_I2C_ADDR);
		return -EIO;
	}
	return 0;
}

static int32_t imx111_af_i2c_read(uint16_t *data)
{
	int16_t code_val_msb, code_val_lsb;
	int16_t rc = 0;

	if (camera_module_type == CAMERA_MODULE_LITEON) {
		code_val_msb = ad5823_read(0x04);
		if (code_val_msb < 0)
		{
			CDBG("Unable to read VCM position MSB\n");
			return rc;
		}
		code_val_lsb = ad5823_read(0x05);
		if ( code_val_lsb < 0)
		{
			CDBG("Unable to read VCM position LSB\n");
			return rc;
		}
	} 
	
	*data = ((code_val_msb&0x03)<<8) | code_val_lsb;
	return rc;
}

static int32_t imx111_af_i2c_write(uint16_t data)
{
    if(camera_module_type == CAMERA_MODULE_SONY){
    	int32_t rc = 0;
    	uint8_t reg1, reg2; 
    	uint16_t temp;

    	//printk("%s : data = 0x%x \n", __func__, data);

    	temp = (data & 0x03F0);
    	reg1 = (temp >> 4);

    	temp = (data & 0x000F);
    	reg2 = ((temp << 4) | 0x05);

    	//printk("%s : write reg1 = 0x%x; reg2 = 0x%x \n", __func__, reg1, reg2);

    	rc = dw9714l_write(reg1, reg2);
    	if (rc < 0)
    	{
    		pr_err("%s : Unable to write i2c code \n", __func__);
    		return rc;
    	}
    }else{
        
        uint8_t code_val_msb, code_val_lsb;
        int32_t rc = 0;
        
        data = (data & 0x03ff);
        code_val_msb = (data >> 8);
        code_val_lsb = data & 0xFF;
        
        code_val_msb  |= 0x04; //RING_CTRL
        
        rc = ad5823_write(0x04, code_val_msb);
        if (rc < 0)
        {
            CDBG("Unable to write code_val_msb = %d\n", code_val_msb);
            return rc;
        }
        rc = ad5823_write(0x05, code_val_lsb);
        if ( rc < 0)
        {
            CDBG("Unable to write code_val_lsb = %d\n", code_val_lsb);
            return rc;
        }
        
        return rc;
    }

	return 0;
} 

#define USE_DUMPING
static void AF_LensMove(int position) 
{
    if (camera_module_type == CAMERA_MODULE_SONY) {
        uint16_t damping_time_wait = VCM_TRES/1000/2 + 1;

    	if(imx111_af_i2c_write(position) < 0) {
            
    	    goto err;
        }

        mdelay(damping_time_wait);
    }else{

     #ifdef USE_DUMPING
     	const uint8_t use_threshold_damping = 1;
     	uint16_t target_dist, small_step=0, next_lens_position;
     	int16_t step_direction;
     #endif	
     	int16_t curr_lens_pos;
     	uint16_t damping_time_wait = 2*VCM_TRES/1000 + 1;
     
     	if (imx111_af_i2c_read(&curr_lens_pos) < 0) {
     		goto err;
     	}
     	
     #ifdef USE_DUMPING
     	if (position > curr_lens_pos) {
     		target_dist = position - curr_lens_pos; 
     		step_direction = 1;
     	} else {
     		target_dist = curr_lens_pos - position; 
     		step_direction = -1;
     	}
     
     	if (target_dist == 0) {
     		return;
     	} 
     
     	if(use_threshold_damping /*&& (step_direction < 0)*/
     			&& (target_dist > damping_threshold)) {
     		small_step = damping_fine_step;
     		//damping_time_wait *= 2;
     	} else {
     		small_step = target_dist;
     		if (small_step == 0) {
     			small_step = 1;
     		}
     		//damping_time_wait = 5;
     	}
     
     	if (small_step != 0) {
     		for (next_lens_position = curr_lens_pos + (step_direction * small_step);
     				(step_direction * next_lens_position) <= (step_direction * position);
     				next_lens_position += (step_direction * small_step)) {
     			if(imx111_af_i2c_write(next_lens_position) < 0) {
     				goto err;
     			}
     			curr_lens_pos = next_lens_position;
     			if(imx111_ctrl->curr_lens_pos != position){
     				mdelay(damping_time_wait);
     			}
     			if (abs(curr_lens_pos-position) < small_step) {
     				break;
     			}
     		}
     	}
     #endif
     	if(curr_lens_pos != position) {
     		if(imx111_af_i2c_write(position) < 0) {
     			goto err;
     		}
     		mdelay(damping_time_wait);
     	}
    }
    
	return;
err:
	printk("AF_LensMove failed!\n");
	return;
}

static int32_t imx111_get_OTP(void)
{
	int32_t rc = 0;
	uint16_t  position_inf, position_1m, position_100mm, start_current, sensitivity;
	unsigned short value_h, value_l;

	//printk("%s : start.............................. \n", __func__);
	
	rc = imx111_i2c_write_b_sensor(REG_BANK_ADDR, 0x01);
	if (rc < 0) {
		pr_err("%s : write or read otp failed \n", __func__);
		goto err;
	}
	rc = imx111_i2c_read(REG_INF_AF_POSITION_H, &value_h, 1);
	if (rc < 0) {
		pr_err("%s : write or read otp failed \n", __func__);
		goto err;
	}
	rc = imx111_i2c_read(REG_INF_AF_POSITION_L, &value_l, 1);
	if (rc < 0) {
		pr_err("%s : write or read otp failed \n", __func__);
		goto err;
	}
	position_inf = (value_h << 8)|value_l;

	rc = imx111_i2c_write_b_sensor(REG_BANK_ADDR, 0x02);
	if (rc < 0) {
		pr_err("%s : write or read otp failed \n", __func__);
		goto err;
	}
	rc = imx111_i2c_read(REG_1M_AF_POSITION_H, &value_h, 1);
	if (rc < 0) {
		pr_err("%s : write or read otp failed \n", __func__);
		goto err;
	}
	rc = imx111_i2c_read(REG_1M_AF_POSITION_L, &value_l, 1);
	if (rc < 0) {
		pr_err("%s : write or read otp failed \n", __func__);
		goto err;
	}
	position_1m = (value_h << 8)|value_l;

	rc = imx111_i2c_read(REG_100MM_AF_POSITION_H, &value_h, 1);
	if (rc < 0) {
		pr_err("%s : write or read otp failed \n", __func__);
		goto err;
	}
	rc = imx111_i2c_read(REG_100MM_AF_POSITION_L, &value_l, 1);
	if (rc < 0) {
		pr_err("%s : write or read otp failed \n", __func__);
		goto err;
	}
	position_100mm = (value_h << 8)|value_l;
	if(position_100mm > 0)
		imx111_vcm_end_current = position_100mm + 60;

	rc = imx111_i2c_read(REG_START_CURRENT_H, &value_h, 1);
	if (rc < 0) {
		pr_err("%s : write or read otp failed \n", __func__);
		goto err;
	}
	rc = imx111_i2c_read(REG_START_CURRENT_L, &value_l, 1);
	if (rc < 0) {
		pr_err("%s : write or read otp failed \n", __func__);
		goto err;
	}
	start_current = (value_h << 8)|value_l;
	if(start_current >= 60)
		imx111_vcm_start_current = start_current - 60;

	rc = imx111_i2c_read(REG_SENSITIVITY_H, &value_h, 1);
	rc = imx111_i2c_read(REG_SENSITIVITY_L, &value_l, 1);
	sensitivity = (value_h << 8)|value_l;

	//printk("%s : vcm_value.position_inf = %d \n", __func__, position_inf);
	//printk("%s : vcm_value.position_1m = %d \n", __func__, position_1m);
	//printk("%s : vcm_value.position_100mm = %d \n", __func__, position_100mm);
	//printk("%s : vcm_value.start_current = %d \n", __func__, start_current);
	//printk("%s : vcm_value.sensitivity = %d \n", __func__, sensitivity);
	
	//printk("%s : imx111_vcm_start_current = %d \n", __func__, imx111_vcm_start_current);
	//printk("%s : imx111_vcm_end_current = %d \n", __func__, imx111_vcm_end_current);

	return rc;
err:
	imx111_vcm_start_current = 165;
	imx111_vcm_end_current   = 1023;
	return 0;
		
}

static int vcm_drive_init(void)
{
	int ret = 0;

    //reset
    ret = ad5823_write(0x01, 0x01);
    if (ret) {
        ret = -1;
        goto err;
    }
    
    ret = ad5823_write(0x04, 0x04);
    if (ret) {
        ret = -1;
        goto err;
    }
    
    if (VCM_TRES > 6579) {
        //mode
        ret = ad5823_write(0x02, 0x00/*0x01*/);
        if (ret) {
            ret = -1;
            goto err;
        }
        
        //move time
        ret = ad5823_write(0x03, VCM_TRES*10/512 - 128);
        if (ret) {
            ret = -1;
            goto err;
        }
    } else {
        //mode
        ret = ad5823_write(0x02, 0x40/*0x41*/);
        if (ret) {
            ret = -1;
            goto err;
        }
        
        //move time
        ret = ad5823_write(0x03, VCM_TRES*10/512 - 1);
        if (ret) {
            ret = -1;
            goto err;
        }
    }
    
    //threshold
    ret = ad5823_write(0x06, VCM_THRETH>>8);
    if (ret) {
        ret = -1;
        goto err;
    }
    ret = ad5823_write(0x07, VCM_THRETH&0xFF);
    if (ret) {
        ret = -1;
        goto err;
    }
	
	return ret;
err:
	printk(KERN_ERR "vcm_drive_init failed!\n");
	return ret;
}

static int16_t imx111_af_init(void)
{
	uint8_t i;

    imx111_step_position_table[0] = 0;

    if (camera_module_type == CAMERA_MODULE_SONY) {

        imx111_get_OTP();
    	printk("%s : step = %d \n", __func__, imx111_l_region_code_per_step);

    	for (i = 1; i <= IMX111_TOTAL_STEPS_NEAR_TO_FAR; i++) {
    		if (i <= imx111_nl_region_boundary1)
    			imx111_step_position_table[i] =
    			imx111_step_position_table[i-1] +
    			imx111_nl_region_code_per_step1;
    		else
    			imx111_step_position_table[i] =
    			imx111_step_position_table[i-1] +
    			imx111_l_region_code_per_step;

    		if (imx111_step_position_table[i] > 1023)
    			imx111_step_position_table[i] = 1023;

    		printk("%s imx111_step_position_table[%d] = %d \n", __func__, i, imx111_step_position_table[i]);
    	}
    }else{

	    for(i = 1; i <= (IMX111_TOTAL_STEPS_NEAR_TO_FAR-1); i++)
    	{
    		if (i <= imx111_nl_region_boundary1) {
    			imx111_step_position_table[i] = imx111_step_position_table[i-1];
    		} else if (i == (imx111_nl_region_boundary1 + 1)) {
    			imx111_step_position_table[i] = VCM_START_CODE + imx111_l_region_code_per_step_adi5823;;
    		} else {
    			imx111_step_position_table[i] = imx111_step_position_table[i-1] + imx111_l_region_code_per_step_adi5823;
    		}

    		if (imx111_step_position_table[i] > VCM_END_CODE) {
    			imx111_step_position_table[i] = VCM_END_CODE;
    		}
    	}

    	damping_threshold = imx111_l_region_code_per_step_adi5823 * 6;
    	damping_fine_step = imx111_l_region_code_per_step_adi5823 * 6/*2*/;
    	vcm_drive_init();

    }
    
	return 0;
}

#if VCM_USE_DW9714L
//#define IMX111_VCM_DEBUG
#ifdef IMX111_VCM_DEBUG
ssize_t vcm_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	uint16_t pos;
	
	if (!imx111_ctrl) {
		return sprintf(buf, "imx111 is not opened!\n");
	}

	if (imx111_af_i2c_read(&pos)) {
		return sprintf(buf, "imx111_af_i2c_read failed!\n");
	} else {
		return sprintf(buf, "%d\n", pos);
	}
}

ssize_t vcm_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	unsigned long val = simple_strtoul(buf, NULL, 10);

	if (val>1023) {
		printk("vcm range: 0 ~ 1023\n");
		return -EINVAL;
	}

	if (!imx111_ctrl) {
		printk("imx111 is not opened!\n");
		return count;
	}
	
	if (imx111_af_i2c_write(val) < 0) {
		printk("imx111_af_i2c_write failed!\n");
	} 
	return count;
}


static DEVICE_ATTR(vcm, S_IWUSR | S_IRUGO,
		   vcm_show, vcm_store);

static struct attribute *imx111_vcm_attributes[] = {
	&dev_attr_vcm.attr,
	NULL
};

static const struct attribute_group imx111_attr_group = {
	.attrs = imx111_vcm_attributes,
};
#endif
#endif

static int32_t imx111_move_focus(int direction,
	int32_t num_steps)
{

	int8_t step_direction;
	int8_t dest_step_position;
	uint16_t dest_lens_position/*, target_dist, small_step*/;
	//int16_t next_lens_position;
	int32_t rc = 0;
	if (num_steps == 0) {
		return 0;
	}

	if ( direction == MOVE_NEAR ) {
		step_direction = 1;
	} else if ( direction == MOVE_FAR) {
		step_direction = -1;
	} else {
		CDBG("Illegal focus direction\n");
		return -EINVAL;
	}

	dest_step_position = imx111_ctrl->curr_step_pos + (step_direction * num_steps);
	if (dest_step_position < 0)	{
		dest_step_position = 0;
	} else if (dest_step_position > IMX111_TOTAL_STEPS_NEAR_TO_FAR)	{
		dest_step_position = IMX111_TOTAL_STEPS_NEAR_TO_FAR;
	}

	dest_lens_position = imx111_step_position_table[dest_step_position];
	
	CDBG("moving focus to dest_step_position %d ...\n", dest_step_position);
	AF_LensMove(dest_lens_position);
	
	/* Storing the current lens Position */
	imx111_ctrl->curr_lens_pos = dest_lens_position;
	imx111_ctrl->curr_step_pos = dest_step_position;
	CDBG("done\n");
	return rc;
}


static int32_t imx111_set_default_focus(void)
{
	AF_LensMove(imx111_step_position_table[0]);
	imx111_ctrl->curr_step_pos = 0;
	imx111_ctrl->curr_lens_pos = imx111_step_position_table[0];
	return 0;
}

static int32_t imx111_sensor_setting(int update_type, int rt)
{
	int32_t rc = 0;
	struct msm_camera_csi_params imx111_csi_params;
	switch (update_type) {
	case REG_INIT:
		if (rt < SENSOR_QTR_SIZE ||  rt >= SENSOR_INVALID_SIZE)
			return rc;
		
		CDBG("Sensor setting Init = %d\n", rt);
		/* reset fps_divider */
		imx111_ctrl->fps = 30 * Q8;
		imx111_ctrl->fps_divider = 1 * 0x400;
		CDBG("%s: %d\n", __func__, __LINE__);
		/* stop streaming */
		rc = imx111_i2c_write_b_sensor(REG_MODE_SELECT,
			MODE_SELECT_STANDBY_MODE);
		if (rc < 0)
			return rc;

		/*imx111_delay_msecs_stdby*/
		msleep(imx111_delay_msecs_stdby);
		rc = imx111_i2c_write_b_sensor(
			REG_GROUPED_PARAMETER_HOLD,
			GROUPED_PARAMETER_HOLD);
			if (rc < 0)
				return rc;


		rc = imx111_i2c_write_w_table(imx111_regs.init_tbl,
			imx111_regs.inittbl_size);
		if (rc < 0)
			return rc;
        
		rc = imx111_i2c_write_b_sensor(
			REG_GROUPED_PARAMETER_HOLD,
			GROUPED_PARAMETER_HOLD_OFF);
		if (rc < 0)
			return rc;

        CDBG("%s: %d\n", __func__, __LINE__);
		/*imx111_delay_msecs_stdby*/
		msleep(imx111_delay_msecs_stdby);

		return rc;
		break;

	case UPDATE_PERIODIC:
		if (rt < SENSOR_QTR_SIZE ||  rt >= SENSOR_INVALID_SIZE)
			return rc;
		
			CDBG("%s: %d\n", __func__, __LINE__);

			/* config mipi csi controller */
			if (config_csi == 0) {
				imx111_csi_params.lane_cnt = 2;
				imx111_csi_params.data_format = CSI_10BIT;
				imx111_csi_params.lane_assign = 0xe4;
				imx111_csi_params.dpcm_scheme = 0;
				imx111_csi_params.settle_cnt = 0x14;

				rc = msm_camio_csi_config(&imx111_csi_params);
				if (rc < 0)
					CDBG("config csi controller failed\n");

				msleep(imx111_delay_msecs_stream);
				config_csi = 1;
				/* stop streaming */
				rc = imx111_i2c_write_b_sensor(REG_MODE_SELECT,
					MODE_SELECT_STANDBY_MODE);
				if (rc < 0)
					return rc;
				msleep(imx111_delay_msecs_stdby);
				rc = imx111_i2c_write_b_sensor(
					REG_GROUPED_PARAMETER_HOLD,
					GROUPED_PARAMETER_HOLD);
					if (rc < 0)
						return rc;

				/* write mode settings */
				if (rt == SENSOR_QTR_SIZE) {
					rc = imx111_i2c_write_w_table(
						imx111_regs.prev_tbl,
						imx111_regs.prevtbl_size);
					if (rc < 0)
						return rc;
				} else if (rt == SENSOR_1080P_SIZE) {
					rc = imx111_i2c_write_w_table(
						imx111_regs.video_tbl,
						imx111_regs.videotbl_size);
					if (rc < 0)
						return rc;
				}else {
					rc = imx111_i2c_write_w_table(
						imx111_regs.snap_tbl,
						imx111_regs.snaptbl_size);
					if (rc < 0)
						return rc;
				}

				rc = imx111_i2c_write_b_sensor(
					REG_GROUPED_PARAMETER_HOLD,
					GROUPED_PARAMETER_HOLD_OFF);
				if (rc < 0)
					return rc;
				//CDBG("IMX111 Turn on streaming\n");

				/* turn on streaming */
				//rc = imx111_i2c_write_b_sensor(REG_MODE_SELECT,
					//MODE_SELECT_STREAM);
				//if (rc < 0)
					//return rc;
				//msleep(imx111_delay_msecs_stream);
				/* stop streaming */
				//pr_err("will standby\n");
				rc = imx111_i2c_write_b_sensor(REG_MODE_SELECT,
					MODE_SELECT_STANDBY_MODE);
				if (rc < 0)
					return rc;
			} else {
				/* stop streaming */
				rc = imx111_i2c_write_b_sensor(REG_MODE_SELECT,
					MODE_SELECT_STANDBY_MODE);
				if (rc < 0)
					return rc;
				msleep(imx111_delay_msecs_stdby);
				rc = imx111_i2c_write_b_sensor(
					REG_GROUPED_PARAMETER_HOLD,
					GROUPED_PARAMETER_HOLD);
					if (rc < 0)
						return rc;
					
				printk("%s res:%d\n",__func__,rt);
				
				/* write mode settings */
				if (rt == SENSOR_QTR_SIZE) {
					rc = imx111_i2c_write_w_table(
						imx111_regs.prev_tbl,
						imx111_regs.prevtbl_size);
					if (rc < 0)
						return rc;
				}else if (rt == SENSOR_1080P_SIZE) {
					rc = imx111_i2c_write_w_table(
						imx111_regs.video_tbl,
						imx111_regs.videotbl_size);
					if (rc < 0)
						return rc;
				} else {
					rc = imx111_i2c_write_w_table(
						imx111_regs.snap_tbl,
						imx111_regs.snaptbl_size);
					if (rc < 0)
						return rc;
				}

				rc = imx111_i2c_write_b_sensor(
					REG_GROUPED_PARAMETER_HOLD,
					GROUPED_PARAMETER_HOLD_OFF);
				if (rc < 0)
					return rc;
				//pr_err("IMX111 Turn on streaming\n");

				/* turn on streaming */
				rc = imx111_i2c_write_b_sensor(REG_MODE_SELECT,
					MODE_SELECT_STREAM);
				if (rc < 0)
					return rc;
				msleep(imx111_delay_msecs_stream);
			}
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}
static int32_t imx111_start_config(void)
{
	int32_t	rc = 0;
	rc = imx111_i2c_write_b_sensor(REG_MODE_SELECT,
		MODE_SELECT_STREAM);
	msleep(5);
	if(rc < 0)
		pr_err("%s: failed\n", __func__);
	return rc;
}
static int32_t imx111_video_config(int mode)
{

	int32_t	rc = 0;
	/* change sensor resolution	if needed */
	#if 0 /*deleted by tjc 2011-12-16 */
	if (imx111_ctrl->prev_res == QTR_SIZE)
		rc = imx111_sensor_setting(
		UPDATE_PERIODIC, RES_PREVIEW);
	else
		rc = imx111_sensor_setting(
		UPDATE_PERIODIC, RES_CAPTURE);
	#endif
	rc = imx111_sensor_setting(UPDATE_PERIODIC, imx111_ctrl->prev_res);

	if (rc < 0)
		return rc;

	imx111_ctrl->curr_res = imx111_ctrl->prev_res;
	imx111_ctrl->sensormode = mode;
	return rc;
}

static int32_t imx111_snapshot_config(int mode)
{
	int32_t rc = 0;
	/* change sensor resolution if needed */
#if 0	
#if 0
	if (imx111_ctrl->curr_res != imx111_ctrl->pict_res) {
#else
	if (1) {
#endif
		if (imx111_ctrl->pict_res == QTR_SIZE)
			rc = imx111_sensor_setting(
			UPDATE_PERIODIC, RES_PREVIEW);
		else
			rc = imx111_sensor_setting(
			UPDATE_PERIODIC, RES_CAPTURE);

		if (rc < 0)
			return rc;
}
#else
	rc = imx111_sensor_setting(UPDATE_PERIODIC, imx111_ctrl->pict_res);

	if (rc < 0)
		return rc;

#endif
	imx111_ctrl->curr_res = imx111_ctrl->pict_res;
	imx111_ctrl->sensormode = mode;
	return rc;
}

static int32_t imx111_raw_snapshot_config(int mode)
{
	int32_t rc = 0;
	/* change sensor resolution if needed */
	if (imx111_ctrl->curr_res != imx111_ctrl->pict_res) {
		#if 0
		if (imx111_ctrl->pict_res == QTR_SIZE)
			rc = imx111_sensor_setting(
			UPDATE_PERIODIC, RES_PREVIEW);
		else
			rc = imx111_sensor_setting(
			UPDATE_PERIODIC, RES_CAPTURE);
		#else
		rc = imx111_sensor_setting(UPDATE_PERIODIC, imx111_ctrl->pict_res);
		#endif

		if (rc < 0)
			return rc;

	}
	imx111_ctrl->curr_res = imx111_ctrl->pict_res;
	imx111_ctrl->sensormode = mode;
	return rc;
}


static int32_t imx111_set_sensor_mode(int mode,
	int res)
{
	int32_t rc = 0;
	switch (mode) {
	case SENSOR_PREVIEW_MODE:
		imx111_ctrl->prev_res = res;
		rc = imx111_video_config(mode);
		break;
	case SENSOR_SNAPSHOT_MODE:
		imx111_ctrl->pict_res = res;
		rc = imx111_snapshot_config(mode);
		break;
	case SENSOR_RAW_SNAPSHOT_MODE:
		imx111_ctrl->pict_res = res;
		rc = imx111_raw_snapshot_config(mode);
		break;
	case SENSOR_PREVIEW_S_MODE:
		rc = imx111_start_config();
		break;
	default:
		rc = -EINVAL;
		break;
	}
	rc = imx111_start_config();
	return rc;
}

static int32_t imx111_power_down(const struct msm_camera_sensor_info *data)
{
	imx111_i2c_write_b_sensor(REG_VCM_DAMP_STBY,
		VCM_DISABLE | VCM_DAMP_CNTRL_OFF);
	imx111_i2c_write_b_sensor(REG_MODE_SELECT,
		MODE_SELECT_STANDBY_MODE);
	msleep(100);
	msm_camio_clk_rate_set(0);
	msleep(5);
	gpio_set_value_cansleep(data->sensor_reset, 0);
	gpio_free(data->sensor_reset);
	return 0;

}

static int32_t imx111_power_on(const struct msm_camera_sensor_info *data)
{
	/* enable mclk first */
	msm_camio_clk_rate_set(IMX111_DEFAULT_MASTER_CLK_RATE);
	msleep(20);
	
	if(!gpio_request(data->sensor_reset, "imx111")) {
		gpio_direction_output(data->sensor_reset, 0);
		msleep(50);
		gpio_set_value_cansleep(data->sensor_reset, 1);
		msleep(50);
		return 0;
	} else {
		return -EINVAL;
	}

}

static int imx111_probe_init_done(const struct msm_camera_sensor_info *data)
{
	return imx111_power_down(data);
}
static int imx111_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
	int32_t rc = 0;
	unsigned short chipidl, chipidh;
	printk("%s: %d\n", __func__, __LINE__);

	rc = imx111_power_on(data);
	if (!rc) {
		printk(" imx111_probe_power_on\n");
	} else {
		printk(" imx111_probe_power_on falied\n");
		goto init_probe_done;
	}
	msleep(20);
	/* 3. Read sensor Model ID: */
	rc = imx111_i2c_read(0x0000, &chipidh, 1);
	if (rc < 0) {
		printk(" imx111_probe_init_sensor 3\n");
		goto init_probe_fail;
	}
	rc = imx111_i2c_read(0x0001, &chipidl, 1);
	if (rc < 0) {
		printk(" imx111_probe_init_sensor 4\n");
		goto init_probe_fail;
	}
	printk("imx111 model_id = 0x%x  0x%x\n", chipidh, chipidl);
	/* 4. Compare sensor ID to IMX111 ID: */
	if (chipidh != 0x01 || chipidl != 0x11) {
		rc = -ENODEV;
		printk("imx111_probe_init_sensor fail chip id doesnot match\n");
		goto init_probe_fail;
	}
	goto init_probe_done;
init_probe_fail:
	printk(" imx111_probe_init_sensor fails\n");
	imx111_probe_init_done(data);
init_probe_done:
	printk(" imx111_probe_init_sensor finishes\n");
	return rc;
	}

/*OPPO sunjianbo 2011-12-28 ESD*/
static int imx111_sensor_reset(const struct msm_camera_sensor_info *data)
{
	int rc = 0;
	struct msm_camera_device_platform_data *camdev = data->pdata;
	int is_subcam = camdev->is_subcamera;
	
	mutex_lock(&imx111_mut);

	pr_info("%s\n",__func__);
	/*pause the csi*/
	msm_camio_camif_pad_reset_csi(0);
	msleep(20);
	imx111_power_down(data);
	msleep(20);
	msm_camera_vreg_ctl(0, is_subcam);
	msleep(300);
	msm_camera_vreg_ctl(1, is_subcam);
	
	rc = imx111_probe_init_sensor(data);
	if(rc < 0) {
		pr_err("%s reset failed!\n",__func__);
		/*need to restore clock status*/
		msm_camio_camif_pad_reset_csi(1);
		mutex_unlock(&imx111_mut);
		return rc;
	}
	
	rc = imx111_sensor_setting(REG_INIT, imx111_ctrl->prev_res);
	if (rc < 0) {
		pr_err("%s: reg init failed!\n",__func__);
		/*need to restore clock status*/
		msm_camio_camif_pad_reset_csi(1);	
		mutex_unlock(&imx111_mut);
		return rc;
	}
	/*restart csi now*/
	msm_camio_camif_pad_reset_csi(1);	
	config_csi = 0;
	
	switch (imx111_ctrl->sensormode) {
	case SENSOR_PREVIEW_MODE:
		rc = imx111_sensor_setting(UPDATE_PERIODIC, imx111_ctrl->prev_res);
		if (rc < 0) 
			pr_err("%s: restore video  mode failed!\n",__func__);
		break;
	case SENSOR_SNAPSHOT_MODE:
	case SENSOR_RAW_SNAPSHOT_MODE:
		rc = imx111_sensor_setting(UPDATE_PERIODIC, imx111_ctrl->pict_res);
		if (rc < 0) 
			pr_err("%s: restore snapshot mode failed!\n",__func__);
		break;
	default:
		rc = -EINVAL;
	}
	mutex_unlock(&imx111_mut);
	return rc;
}
/*ESD end*/

int imx111_sensor_open_init(const struct msm_camera_sensor_info *data)
{
	int32_t rc = 0;
	
	/*OPPO sunjianbo 2011-12-28 ESD*/
	if(imx111_ctrl)
		return imx111_sensor_reset(data);
	/*end*/

	CDBG("%s: %d\n", __func__, __LINE__);
	imx111_ctrl = kzalloc(sizeof(struct imx111_ctrl_t), GFP_KERNEL);
	if (!imx111_ctrl) {
		CDBG("imx111_init failed!\n");
		rc = -ENOMEM;
		goto init_done;
	}
	imx111_ctrl->fps_divider = 1 * 0x00000400;
	imx111_ctrl->pict_fps_divider = 1 * 0x00000400;
	imx111_ctrl->fps = 30 * Q8;
	imx111_ctrl->set_test = TEST_OFF;
	imx111_ctrl->prev_res = SENSOR_QTR_SIZE;
	imx111_ctrl->pict_res = SENSOR_FULL_SIZE;
	imx111_ctrl->curr_res = SENSOR_INVALID_SIZE;
	config_csi = 0;

	if (data)
		imx111_ctrl->sensordata = data;
	CDBG("%s: %d\n", __func__, __LINE__);

	rc = imx111_probe_init_sensor(data);
	if (rc < 0) {
		CDBG("Calling imx111_sensor_open_init fail\n");
		goto probe_fail;
	}
    
	CDBG("%s: %d\n", __func__, __LINE__);
	rc = imx111_sensor_setting(REG_INIT, SENSOR_QTR_SIZE);
	CDBG("%s: %d\n", __func__, __LINE__);
	if (rc < 0)
		goto init_fail;
    
	rc = imx111_af_init();
	if (rc < 0)
		goto init_fail;
	else
		goto init_done;
    
probe_fail:
	CDBG("%s probe failed\n", __func__);
	kfree(imx111_ctrl);
	imx111_ctrl = NULL;
	return rc;
init_fail:
	CDBG(" imx111_sensor_open_init fail\n");
	CDBG("%s: %d\n", __func__, __LINE__);
	imx111_probe_init_done(data);
	kfree(imx111_ctrl);
	imx111_ctrl = NULL;
init_done:
CDBG("%s: %d\n", __func__, __LINE__);
	CDBG("imx111_sensor_open_init done\n");
	return rc;
}

static int imx111_init_client(struct i2c_client *client)
{
	/* Initialize the MSM_CAMI2C Chip */
	init_waitqueue_head(&imx111_wait_queue);
	return 0;
}

static const struct i2c_device_id imx111_i2c_id[] = {
	{"imx111", 0},
	{ }
};

static int imx111_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		CDBG("i2c_check_functionality failed\n");
		goto probe_failure;
	}

	imx111_sensorw = kzalloc(sizeof(struct imx111_work_t), GFP_KERNEL);
	if (!imx111_sensorw) {
		CDBG("kzalloc failed.\n");
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, imx111_sensorw);
	imx111_init_client(client);
	imx111_client = client;

	msleep(50);
	
#ifdef IMX111_VCM_DEBUG
	/* Register sysfs hooks */
	rc = sysfs_create_group(&client->dev.kobj, &imx111_attr_group);
	if (rc) {
		printk(KERN_ERR "imx111_i2c_probe: sysfs_create_group failed!\n");
	}
#endif

	CDBG("imx111_probe success! rc = %d\n", rc);
	return 0;

probe_failure:
	CDBG("imx111_probe failed! rc = %d\n", rc);
	return rc;
}

static int __exit imx111_remove(struct i2c_client *client)
{
	struct imx111_work_t_t *sensorw = i2c_get_clientdata(client);
	free_irq(client->irq, sensorw);
	imx111_client = NULL;
	kfree(sensorw);
	return 0;
}

static struct i2c_driver imx111_i2c_driver = {
	.id_table = imx111_i2c_id,
	.probe  = imx111_i2c_probe,
	.remove = __exit_p(imx111_i2c_remove),
	.driver = {
		.name = "imx111",
	},
};

static void imx111_module_probe(void)  
{   
    int rc = 0;

	//check if ad5823 exist
    rc = ad5823_read(0x02);	
	if(rc < 0) {
		camera_module_type = CAMERA_MODULE_SONY ;
	} else {
		camera_module_type = CAMERA_MODULE_LITEON ;
	}
	printk(KERN_INFO "%s, type %d\n", __func__, camera_module_type);
}

static int imx111_sensor_get_module_type(void __user *argp) 
{
	int rc = 0;
	int32_t type = camera_module_type;

	if (copy_to_user((void *)argp,
		&type, sizeof(int32_t)))
		rc = -EFAULT;

	return rc;
}

int imx111_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cdata;
	long   rc = 0;
	if (copy_from_user(&cdata,
		(void *)argp,
		sizeof(struct sensor_cfg_data)))
		return -EFAULT;
	mutex_lock(&imx111_mut);
	CDBG("imx111_sensor_config: cfgtype = %d\n",
	cdata.cfgtype);
		switch (cdata.cfgtype) {
		case CFG_GET_PICT_FPS:
			imx111_get_pict_fps(
				cdata.cfg.gfps.prevfps,
				&(cdata.cfg.gfps.pictfps));

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_PREV_L_PF:
			cdata.cfg.prevl_pf =
			imx111_get_prev_lines_pf();

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_PREV_P_PL:
			cdata.cfg.prevp_pl =
				imx111_get_prev_pixels_pl();

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_PICT_L_PF:
			cdata.cfg.pictl_pf =
				imx111_get_pict_lines_pf();

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_PICT_P_PL:
			cdata.cfg.pictp_pl =
				imx111_get_pict_pixels_pl();

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_PICT_MAX_EXP_LC:
			cdata.cfg.pict_max_exp_lc =
				imx111_get_pict_max_exp_lc();

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_SET_FPS:
		case CFG_SET_PICT_FPS:
			rc = imx111_set_fps(&(cdata.cfg.fps));
			break;

		case CFG_SET_EXP_GAIN:
			rc =
				imx111_write_exp_gain(
					cdata.cfg.exp_gain.gain,
					cdata.cfg.exp_gain.line);
			break;

		case CFG_SET_PICT_EXP_GAIN:
			rc =
				imx111_set_pict_exp_gain(
				cdata.cfg.exp_gain.gain,
				cdata.cfg.exp_gain.line);
			break;

		case CFG_SET_MODE:
			rc = imx111_set_sensor_mode(cdata.mode,
					cdata.rs);
			break;

		case CFG_PWR_DOWN:
			rc = imx111_power_down(imx111_ctrl->sensordata);
			break;
		case CFG_GET_AF_MAX_STEPS:
			cdata.max_steps = IMX111_STEPS_NEAR_TO_CLOSEST_INF;
			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;
		case CFG_MOVE_FOCUS:
			rc =
				imx111_move_focus(
				cdata.cfg.focus.dir,
				cdata.cfg.focus.steps);
			break;

		case CFG_SET_DEFAULT_FOCUS:
			rc =
				imx111_set_default_focus();
			break;

		case CFG_SET_EFFECT:
		default:
			rc = -EFAULT;
			break;
		}

	mutex_unlock(&imx111_mut);

	return rc;
}

static int imx111_sensor_release(void)
{
	int rc = -EBADF;
	mutex_lock(&imx111_mut);
	imx111_set_default_focus();
	imx111_power_down(imx111_ctrl->sensordata);
	kfree(imx111_ctrl);
	imx111_ctrl = NULL;
	CDBG("imx111_release completed\n");
	mutex_unlock(&imx111_mut);

	return rc;
}

static int imx111_sensor_probe(const struct msm_camera_sensor_info *info,
		struct msm_sensor_ctrl *s)
{
	int rc = 0;
	rc = i2c_add_driver(&imx111_i2c_driver);
	if (rc < 0 || imx111_client == NULL) {
		rc = -ENOTSUPP;
		goto probe_fail;
	}
	rc = imx111_probe_init_sensor(info);
	if (rc < 0)
		goto probe_fail;
    
    imx111_module_probe();

    s->s_init = imx111_sensor_open_init;
	s->s_release = imx111_sensor_release;
	s->s_config  = imx111_sensor_config;
    s->s_get_module_type = imx111_sensor_get_module_type;
	s->s_mount_angle = info->sensor_platform_info->mount_angle;
	imx111_probe_init_done(info);

	return rc;

probe_fail:
	CDBG("imx111_sensor_probe: SENSOR PROBE FAILS!\n");
	return rc;
}

static int __imx111_probe(struct platform_device *pdev)
{
	return msm_camera_drv_start(pdev, imx111_sensor_probe);
}

static struct platform_driver msm_camera_driver = {
	.probe = __imx111_probe,
	.driver = {
		.name = "msm_camera_imx111",
		.owner = THIS_MODULE,
	},
};

static int __init imx111_init(void)
{
	return platform_driver_register(&msm_camera_driver);
}

module_init(imx111_init);

void imx111_exit(void)
{
	i2c_del_driver(&imx111_i2c_driver);
}


MODULE_DESCRIPTION("Sony 8 MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");
