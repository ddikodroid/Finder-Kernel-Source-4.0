/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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
 
#ifndef S5K8AAY_H
#define S5K8AAY_H
#include <linux/types.h>
#include <mach/board.h>
#undef CDBG
//#define S5K8AAY_DEBUG
#ifdef S5K8AAY_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do{}while(0)
#endif

enum s5k8aay_width {
	WORD_LEN,
	BYTE_LEN
};

struct s5k8aay_i2c_reg_conf {
	unsigned short waddr;
	unsigned short wdata;
	enum s5k8aay_width width;
	unsigned short mdelay_time;
};

enum s5k8aay_resolution_t {
	QTR_SIZE,
	FULL_SIZE,
	INVALID_SIZE
};

enum s5k8aay_setting {
	RES_PREVIEW,
	RES_CAPTURE
};

enum s5k8aay_reg_update {
	/* Sensor egisters that need to be updated during initialization */
	REG_INIT,
	/* Sensor egisters that needs periodic I2C writes */
	UPDATE_PERIODIC,
	/* All the sensor Registers will be updated */
	UPDATE_ALL,
	/* Not valid update */
	UPDATE_INVALID
};

struct s5k8aay_work_t {
	struct work_struct work;
};

struct s5k8aay_ctrl_t {
	const struct  msm_camera_sensor_info *sensordata;
	uint32_t sensormode;
	enum s5k8aay_resolution_t prev_res;
	enum s5k8aay_resolution_t pict_res;
	enum s5k8aay_resolution_t curr_res;
	uint32_t reg_init_flg;
};

struct s5k8aay_reg {
	const struct s5k8aay_i2c_reg_conf *reg_init;
	const unsigned short reg_init_size;
	const struct s5k8aay_i2c_reg_conf *reg_preview;
	const unsigned short reg_preview_size;
	const struct s5k8aay_i2c_reg_conf *reg_snapshot;
	const unsigned short reg_snapshot_size;
};

extern struct s5k8aay_reg s5k8aay_regs;
#endif

