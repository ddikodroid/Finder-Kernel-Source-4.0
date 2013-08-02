/* Copyright (c) 2010, Code Aurora Forum. All rights reserved.
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

 history:
 20110924 liujinshui move the setting arrays to  s5k8aay_reg.c
 */

#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <mach/camera.h>
#include "s5k8aay.h"

#define Q8    0x00000100

//#define  REG_S5K8AAY_MODEL_ID 0x0000
#define  S5K8AAY_MODEL_ID     0x08AA

#define S5K8AAY_DRIVER_DBG 1
#undef CDBG
#if S5K8AAY_DRIVER_DBG
#define CDBG(fmt, args...) printk(KERN_INFO "msm_subcamera_kernel: " fmt, ##args)
#else
#define CDBG(fmt, args...) do{}while(0)
#endif

static struct  s5k8aay_work_t *s5k8aay_sensorw;
static struct  i2c_client *s5k8aay_client;
static struct s5k8aay_ctrl_t *s5k8aay_ctrl;
static DECLARE_WAIT_QUEUE_HEAD(s5k8aay_wait_queue);
DEFINE_MUTEX(s5k8aay_mut);

/*=============================================================*/

static int s5k8aay_i2c_rxdata(unsigned short saddr,
	unsigned char *rxdata, int length)
{
	int retry_cnt = 0;
	int rc;

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
			.len   = length,
			.buf   = rxdata,
		},
	};
	
	do {
		rc = i2c_transfer(s5k8aay_client->adapter, msgs, 2);
		if (rc > 0)
			break;	
		retry_cnt++;
	} while (retry_cnt < 3);
	
	
	if (rc < 0) {
		pr_err("%s :failed!:%d %d\n", __func__, rc, retry_cnt);
		return -EIO;
	}
	
	return 0;
}

static int32_t s5k8aay_i2c_txdata(unsigned short saddr,
				unsigned char *txdata, int length)
{
	int retry_cnt = 0;
	int rc;

	struct i2c_msg msg[] = {
		{
			.addr = saddr,
			.flags = 0,
			.len = length,
			.buf = txdata,
		 },
	};
	do {
		rc = i2c_transfer(s5k8aay_client->adapter, msg, 1);
		if (rc > 0)
			break;
		retry_cnt++;
	} while (retry_cnt < 3);
	
	if (rc < 0) {
		pr_err("%s :failed: %d %d\n", __func__, rc, retry_cnt);		
		return -EIO;
	}

	return 0;
}

static int32_t s5k8aay_i2c_read(unsigned short raddr,
	unsigned short *rdata)
{
	int32_t rc = 0;
	unsigned char buf[2];
	if (!rdata)
		return -EIO;
	memset(buf, 0, sizeof(buf));
	buf[0] = (raddr & 0xFF00) >> 8;
	buf[1] = (raddr & 0x00FF);
	rc = s5k8aay_i2c_rxdata(s5k8aay_client->addr, buf, 2);
	if (rc < 0) {
		pr_err("%s :0x%x failed!\n", __func__, raddr);
		return rc;
	}
	*rdata = buf[0] << 8 | buf[1];
	return rc;
}

static int32_t s5k8aay_i2c_write(unsigned short raddr,
	unsigned short rdata)
{
	int32_t rc = -EIO;
	unsigned char buf[4];
	
	memset(buf, 0, sizeof(buf));
	buf[0] = (raddr & 0xFF00)>>8;
	buf[1] = (raddr & 0x00FF);
	buf[2] = (rdata & 0xFF00)>>8;
	buf[3] = (rdata & 0x00FF);
	rc = s5k8aay_i2c_txdata(s5k8aay_client->addr , buf, 4);
	if (rc < 0)
		pr_err("%s :failed, addr = 0x%x, val = 0x%x!\n", __func__, raddr, rdata);
	
	return rc;
}

static int32_t s5k8aay_sensor_reg_setting(int update_type, int rt)
{
	int32_t i, array_length;
	int32_t rc = 0;
	struct msm_camera_csi_params s5k8aay_csi_params;

	switch (update_type) {
	case REG_INIT:
		CDBG("%s: REG_INIT E\n", __func__);
		array_length = s5k8aay_regs.reg_init_size;
		for (i = 0; i < array_length; i++) {
			rc = s5k8aay_i2c_write(
				s5k8aay_regs.reg_init[i].waddr,
					s5k8aay_regs.reg_init[i].wdata);
			if (rc < 0)
				return rc;
			if (s5k8aay_regs.reg_init[i].mdelay_time != 0)
				msleep(s5k8aay_regs.reg_init[i].mdelay_time);
		}
		msleep(20); 
		CDBG("%s: REG_INIT X\n", __func__);
		break;

	case UPDATE_PERIODIC:
		if (rt == RES_PREVIEW || rt == RES_CAPTURE) { 
			CDBG("%s: UPDATE_PERIODIC E\n", __func__);
			if(s5k8aay_ctrl->reg_init_flg == 1){				
				CDBG("%s :s5k8aay_ctrl->reg_init_flg == 1\n", __func__);
				s5k8aay_csi_params.lane_cnt = 1;
				s5k8aay_csi_params.data_format = CSI_8BIT;
				s5k8aay_csi_params.lane_assign = 0xe4;
				s5k8aay_csi_params.dpcm_scheme = 0;
				s5k8aay_csi_params.settle_cnt = 0x14;

				rc = msm_camio_csi_config(&s5k8aay_csi_params);
				msleep(10);

				array_length = s5k8aay_regs.reg_init_size;
				for (i = 0; i < array_length; i++) {
					rc = s5k8aay_i2c_write(
						s5k8aay_regs.reg_init[i].waddr,
							s5k8aay_regs.reg_init[i].wdata);
					if (rc < 0)
						return rc;
					if (s5k8aay_regs.reg_init[i].mdelay_time != 0)
						msleep(s5k8aay_regs.reg_init[i].mdelay_time);
				}
				msleep(20); 
				
				s5k8aay_ctrl->reg_init_flg = 0;
			}

			if(rt == RES_PREVIEW ) {
				CDBG("%s :RES_PREVIEW\n", __func__);
				array_length = s5k8aay_regs.reg_preview_size;
				for (i = 0; i < array_length; i++) {
					rc = s5k8aay_i2c_write(
						s5k8aay_regs.reg_preview[i].waddr,
							s5k8aay_regs.reg_preview[i].wdata);
					if (rc < 0)
						return rc;
					if (s5k8aay_regs.reg_preview[i].mdelay_time != 0)
						msleep(s5k8aay_regs.reg_preview[i].mdelay_time);
				}				
			}else if(rt == RES_CAPTURE ) {
				CDBG("%s :RES_CAPTURE\n", __func__);
				array_length = s5k8aay_regs.reg_snapshot_size;
				for (i = 0; i < array_length; i++) {
					rc = s5k8aay_i2c_write(s5k8aay_regs.reg_snapshot[i].waddr,
						s5k8aay_regs.reg_snapshot[i].wdata);
					if (rc < 0)
						return rc;
					if (s5k8aay_regs.reg_snapshot[i].mdelay_time != 0)
						msleep(s5k8aay_regs.reg_snapshot[i].mdelay_time);
				}
			}
			CDBG("%s :UPDATE_PERIODIC X\n", __func__);
		} 
		break;
	default:
		rc = -EINVAL;
		break;
	}
	return rc;
}

static int32_t s5k8aay_set_sensor_mode(int mode,
	int res)
{
	int32_t rc = 0;
	
	switch (mode) {
	case SENSOR_PREVIEW_MODE: 
		if (s5k8aay_sensor_reg_setting(UPDATE_PERIODIC, RES_PREVIEW) < 0)
			return rc;
		s5k8aay_ctrl->curr_res = s5k8aay_ctrl->prev_res;
		s5k8aay_ctrl->sensormode = mode;
		break;
		
	case SENSOR_SNAPSHOT_MODE:
	case SENSOR_RAW_SNAPSHOT_MODE: 
		if (s5k8aay_sensor_reg_setting(UPDATE_PERIODIC, RES_CAPTURE) < 0)
			return rc;
		s5k8aay_ctrl->curr_res = s5k8aay_ctrl->prev_res;
		s5k8aay_ctrl->sensormode = mode;
		break;
		
	default:
		rc = -EINVAL;
		break;
	}
	return rc;
}

static int32_t s5k8aay_power_down(void)
{
	return 0;
}

static int s5k8aay_sensor_chip_init(const struct msm_camera_sensor_info *data)
{
	uint16_t model_id;
	int32_t rc = 0;

	msleep(2);
	rc = s5k8aay_i2c_write(0x002C, 0x0000);
		if(rc < 0)
			pr_err("%s: write to reg 0x0028 failed!\n", __func__);
	rc = s5k8aay_i2c_write(0x002E, 0x0040);
		if(rc < 0)
			pr_err("%s: write to reg 0x002A failed!\n", __func__);
	rc = s5k8aay_i2c_read(0x0F12, &model_id);
		if(rc < 0) {
			pr_err("%s: read to reg 0x0F12 failed!\n", __func__);
			goto init_probe_fail;
		}
	CDBG("%s :model_id = 0x%x\n", __func__, model_id);
	if (model_id != S5K8AAY_MODEL_ID) {
		rc = -ENODEV;
		pr_err("%s : chip id doesnot match\n", __func__);
		goto init_probe_fail;
	}
	goto init_probe_done;
init_probe_fail:
	pr_err("%s : fail\n", __func__);
init_probe_done:
	return rc;
}

int s5k8aay_sensor_init(const struct msm_camera_sensor_info *data)
{
	int32_t rc = 0;

	if(s5k8aay_ctrl)
		return 0;

	CDBG("%s :E\n", __func__);
	s5k8aay_ctrl = kzalloc(sizeof(struct s5k8aay_ctrl_t), GFP_KERNEL);
	if (!s5k8aay_ctrl) {
		CDBG("%s :kzalloc fail\n", __func__);
		rc = -ENOMEM;
		goto init_done;
	}
	s5k8aay_ctrl->prev_res = QTR_SIZE;
	s5k8aay_ctrl->pict_res = FULL_SIZE;
	s5k8aay_ctrl->curr_res = INVALID_SIZE;
	s5k8aay_ctrl->reg_init_flg = 1;

	if (data)
		s5k8aay_ctrl->sensordata = data;

	msm_camio_clk_rate_set(24000000);
	msleep(20);

	rc = s5k8aay_sensor_chip_init(data);
/*deleted by Liu Jinshui 20120523 start*/
/*remove unnecessary initial*/
#if 0
	if (rc < 0) {
		goto init_fail;
	}

	
	rc = s5k8aay_sensor_reg_setting(REG_INIT, RES_PREVIEW);
#endif
/*deleted by Liu Jinshui 20120523 end*/
	if (rc < 0) {
		goto init_fail;
	} else  
		goto init_done;
init_fail:
	pr_err("%s :fail\n", __func__);
	kfree(s5k8aay_ctrl);
	s5k8aay_ctrl = NULL;
init_done:
	CDBG("%s :X\n", __func__);
	return rc;
}

static int s5k8aay_init_client(struct i2c_client *client)
{
	init_waitqueue_head(&s5k8aay_wait_queue);
	return 0;
}

static const struct i2c_device_id s5k8aay_i2c_id[] = {
	{"s5k8aay", 0},
	{ }
};

static int s5k8aay_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	CDBG("%s :E\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("%s :i2c_check_functionality failed\n", __func__);
		goto probe_failure;
	}

	s5k8aay_sensorw = kzalloc(sizeof(struct s5k8aay_work_t), GFP_KERNEL);
	if (!s5k8aay_sensorw) {
		pr_err("%s :kzalloc failed\n", __func__);
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, s5k8aay_sensorw);
	s5k8aay_init_client(client);
	s5k8aay_client = client;

	msleep(50);

	CDBG("%s :X\n", __func__);
	return 0;

probe_failure:
	pr_err("%s :failed! rc = %d\n", __func__, rc);
	return rc;
}

static int __exit s5k8aay_remove(struct i2c_client *client)
{
	struct s5k8aay_work_t_t *sensorw = i2c_get_clientdata(client);

	CDBG("%s :\n", __func__);
	free_irq(client->irq, sensorw);
	s5k8aay_client = NULL;
	kfree(sensorw);
	return 0;
}

static struct i2c_driver s5k8aay_i2c_driver = {
	.id_table = s5k8aay_i2c_id,
	.probe  = s5k8aay_i2c_probe,
	.remove = __exit_p(s5k8aay_remove),
	.driver = {
		.name = "s5k8aay",
	},
};

int s5k8aay_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cdata;
	long   rc = 0;
	
	if (copy_from_user(&cdata,
		(void *)argp,
		sizeof(struct sensor_cfg_data)))
		return -EFAULT;
	
	mutex_lock(&s5k8aay_mut);
	CDBG("%s : cfgtype = %d\n", __func__, cdata.cfgtype);
	switch (cdata.cfgtype) {
	case CFG_SET_MODE:
		rc = s5k8aay_set_sensor_mode(cdata.mode,
			cdata.rs);
		break;
	case CFG_PWR_DOWN:
		rc = s5k8aay_power_down();
		break;
	default:
		rc = -EFAULT;
		break;
	}

	mutex_unlock(&s5k8aay_mut);

	return rc;
}

static int s5k8aay_sensor_release(void)
{
	int rc = -EBADF;
	mutex_lock(&s5k8aay_mut);
	s5k8aay_power_down();
	gpio_set_value_cansleep(s5k8aay_ctrl->sensordata->sensor_pwd, 1);
	gpio_set_value_cansleep(s5k8aay_ctrl->sensordata->sensor_reset, 0);
	msm_camio_clk_rate_set(0);
	kfree(s5k8aay_ctrl);
	s5k8aay_ctrl = NULL;
	CDBG("%s :done\n", __func__);
	mutex_unlock(&s5k8aay_mut); 
	return rc;
}

static int s5k8aay_probe(const struct msm_camera_sensor_info *info,
		struct msm_sensor_ctrl *s)
{
	int rc = 0;
	rc = i2c_add_driver(&s5k8aay_i2c_driver);
	if (rc < 0 || s5k8aay_client == NULL) {
		rc = -ENOTSUPP;
		goto probe_fail;
	}
	msm_camio_clk_rate_set(24000000);
	rc = s5k8aay_sensor_chip_init(info);
	if (rc < 0)
		goto probe_fail;
	s->s_init = s5k8aay_sensor_init;
	s->s_release = s5k8aay_sensor_release;
	s->s_config  = s5k8aay_sensor_config;
	s->s_camera_type = FRONT_CAMERA_2D;
	s->s_mount_angle = info->sensor_platform_info->mount_angle;
	msm_camio_clk_rate_set(0);
	return rc;

probe_fail:
	pr_err("%s :fail\n", __func__);
	i2c_del_driver(&s5k8aay_i2c_driver);
	return rc;
}

static int __s5k8aay_probe(struct platform_device *pdev)
{

	return msm_camera_drv_start(pdev, s5k8aay_probe);
}

static struct platform_driver msm_camera_driver = {
	.probe = __s5k8aay_probe,
	.driver = {
		.name = "msm_camera_s5k8aay",
		.owner = THIS_MODULE,
	},
};

static int __init s5k8aay_init(void)
{
	return platform_driver_register(&msm_camera_driver);
}

module_init(s5k8aay_init);

MODULE_DESCRIPTION("MICRON 2M YUV Sensor driver");
MODULE_LICENSE("GPL v2");

