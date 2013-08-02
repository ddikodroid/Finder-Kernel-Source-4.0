/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of Code Aurora Forum, Inc. nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef IMX111_H
#define IMX111_H
#include <linux/types.h>
#include <mach/board.h>
#include <media/msm_camera.h>

//#define PREVIEW_FULLSIZE 1

extern struct imx111_reg imx111_regs;

struct imx111_i2c_reg_conf {
	unsigned short waddr;
	unsigned short wdata;
};

struct imx111_reg {

	struct imx111_i2c_reg_conf const *init_tbl;
	uint16_t inittbl_size;
	struct imx111_i2c_reg_conf const *prev_tbl;
	uint16_t prevtbl_size;
	struct imx111_i2c_reg_conf const *snap_tbl;
	uint16_t snaptbl_size;
	struct imx111_i2c_reg_conf const *vcm_tbl;
	uint16_t vcmtbl_size;
	//start added by tjc 2011-12-16 for other size table used in video mode
	struct imx111_i2c_reg_conf const *video_tbl;
	uint16_t videotbl_size;
	//end added by tjc 2011-12-16 for other size table used in video mode


};


enum imx111_test_mode_t {
	TEST_OFF,
	TEST_1,
	TEST_2,
	TEST_3
};

//start deleted by tjc 2011-12-16 
#if 0
enum imx111_resolution_t {
	QTR_SIZE,
	FULL_SIZE,
	INVALID_SIZE
};
enum imx111_setting {
	RES_PREVIEW,
	RES_CAPTURE
};
#endif
//end deleted by tjc 2011-12-16 
enum imx111_reg_update {
	/* Sensor egisters that need to be updated during initialization */
	REG_INIT,
	/* Sensor egisters that needs periodic I2C writes */
	UPDATE_PERIODIC,
	/* All the sensor Registers will be updated */
	UPDATE_ALL,
	/* Not valid update */
	UPDATE_INVALID
};


#endif /* IMX111_H */
