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



#include "s5k8aay.h"

static struct s5k8aay_i2c_reg_conf s5k8aay_init_array[] = {

// Modify the Gamma and AWB thresh.
//$MIPI[Width:1280,Height:960,Format:YUV422,Lane:1,ErrorCheck:0,PolarityData:0,PolarityClock:0,Buffer:2]

//============================================================
//	 00.History
//============================================================
//	 2011 : EVT1
//	 20111109 : LSI CSE Standard
//	 20111110 : Shading, AWB, Contrast Tuning
//	 20111114 : AE Weight Tuning
//	 20111229 : BPC Tuning, Shading Tuning
//	 20120128 : CCM Tuning, AE Weight Tuning, AE Target Tuning
//	 20120319 : Skintone Test, CCM Tuning
//	 20120320 : ColorShading Tuning, Modify TnP Setting
//	 20120323 : CCM Tuning, AWB Grid offset Tuning
//	 20120330 : Modigy AWB indoor boundary & AWB Grid offset
//	 20120402 : SKT VT-Call Tuning
//	 20120404 : NearGray Tuning
//	 20120406 : Shading Alpha Tuning
//	 20120407 : Outdoor Skintone Tuning
//============================================================


//*************************************/
// 01.Start Setting					  */
//*************************************/
    {0xFCFC, 0xD000, WORD_LEN, 0},
    {0x0010, 0x0001, WORD_LEN, 0},// S/W Reset */
    {0xFCFC, 0x0000, WORD_LEN, 0},
    {0x0000, 0x0000, WORD_LEN, 0},// Simmian bug workaround */

    {0xFCFC, 0xD000, WORD_LEN, 0},
    {0x1030, 0x0000, WORD_LEN, 0},// contint_host_int */
    {0x0014, 0x0001, WORD_LEN, 10},



//*************************************/
// 02.Analog Setting & ASP Control    */
//*************************************/

//*************************************/
// 03.Trap and Patch                  */
//*************************************/
// Start of Patch data */
    {0x0028, 0x7000, WORD_LEN, 0},
    {0x002A, 0x2470, WORD_LEN, 0},
    {0x0F12, 0xB510, WORD_LEN, 0},
    {0x0F12, 0x490E, WORD_LEN, 0},
    {0x0F12, 0x480E, WORD_LEN, 0},
    {0x0F12, 0xF000, WORD_LEN, 0},
    {0x0F12, 0xF9ED, WORD_LEN, 0},
    {0x0F12, 0x490E, WORD_LEN, 0},
    {0x0F12, 0x480E, WORD_LEN, 0},
    {0x0F12, 0xF000, WORD_LEN, 0},
    {0x0F12, 0xF9E9, WORD_LEN, 0},
    {0x0F12, 0x490E, WORD_LEN, 0},
    {0x0F12, 0x480E, WORD_LEN, 0},
    {0x0F12, 0x6341, WORD_LEN, 0},
    {0x0F12, 0x490E, WORD_LEN, 0},
    {0x0F12, 0x480F, WORD_LEN, 0},
    {0x0F12, 0xF000, WORD_LEN, 0},
    {0x0F12, 0xF9E2, WORD_LEN, 0},
    {0x0F12, 0x490E, WORD_LEN, 0},
    {0x0F12, 0x480F, WORD_LEN, 0},
    {0x0F12, 0xF000, WORD_LEN, 0},
    {0x0F12, 0xF9DE, WORD_LEN, 0},
    {0x0F12, 0x490E, WORD_LEN, 0},
    {0x0F12, 0x480F, WORD_LEN, 0},
    {0x0F12, 0xF000, WORD_LEN, 0},
    {0x0F12, 0xF9DA, WORD_LEN, 0},
    {0x0F12, 0x480E, WORD_LEN, 0},
    {0x0F12, 0x490F, WORD_LEN, 0},
    {0x0F12, 0x6448, WORD_LEN, 0},
    {0x0F12, 0xBC10, WORD_LEN, 0},
    {0x0F12, 0xBC08, WORD_LEN, 0},
    {0x0F12, 0x4718, WORD_LEN, 0},
    {0x0F12, 0x27CC, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x8EDD, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0x2744, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x8725, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0x26E4, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x0080, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x2638, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0xA6EF, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0x2604, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0xA0F1, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0x25D0, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x058F, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0x24E4, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x403E, WORD_LEN, 0},
    {0x0F12, 0xE92D, WORD_LEN, 0},
    {0x0F12, 0x00DD, WORD_LEN, 0},
    {0x0F12, 0xEB00, WORD_LEN, 0},
    {0x0F12, 0x2000, WORD_LEN, 0},
    {0x0F12, 0xE3A0, WORD_LEN, 0},
    {0x0F12, 0x1002, WORD_LEN, 0},
    {0x0F12, 0xE3A0, WORD_LEN, 0},
    {0x0F12, 0x0F86, WORD_LEN, 0},
    {0x0F12, 0xE3A0, WORD_LEN, 0},
    {0x0F12, 0x00DC, WORD_LEN, 0},
    {0x0F12, 0xEB00, WORD_LEN, 0},
    {0x0F12, 0x200A, WORD_LEN, 0},
    {0x0F12, 0xE3A0, WORD_LEN, 0},
    {0x0F12, 0x1000, WORD_LEN, 0},
    {0x0F12, 0xE28D, WORD_LEN, 0},
    {0x0F12, 0x0E3F, WORD_LEN, 0},
    {0x0F12, 0xE3A0, WORD_LEN, 0},
    {0x0F12, 0x00DB, WORD_LEN, 0},
    {0x0F12, 0xEB00, WORD_LEN, 0},
    {0x0F12, 0x2001, WORD_LEN, 0},
    {0x0F12, 0xE3A0, WORD_LEN, 0},
    {0x0F12, 0x1002, WORD_LEN, 0},
    {0x0F12, 0xE3A0, WORD_LEN, 0},
    {0x0F12, 0x0F86, WORD_LEN, 0},
    {0x0F12, 0xE3A0, WORD_LEN, 0},
    {0x0F12, 0x00D4, WORD_LEN, 0},
    {0x0F12, 0xEB00, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE5DD, WORD_LEN, 0},
    {0x0F12, 0x00C3, WORD_LEN, 0},
    {0x0F12, 0xE350, WORD_LEN, 0},
    {0x0F12, 0x0027, WORD_LEN, 0},
    {0x0F12, 0x1A00, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},
    {0x0F12, 0xE5DD, WORD_LEN, 0},
    {0x0F12, 0x003C, WORD_LEN, 0},
    {0x0F12, 0xE350, WORD_LEN, 0},
    {0x0F12, 0x0024, WORD_LEN, 0},
    {0x0F12, 0x1A00, WORD_LEN, 0},
    {0x0F12, 0x02E0, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x1000, WORD_LEN, 0},
    {0x0F12, 0xE5D0, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE351, WORD_LEN, 0},
    {0x0F12, 0x0003, WORD_LEN, 0},
    {0x0F12, 0x1A00, WORD_LEN, 0},
    {0x0F12, 0x12D4, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x10B8, WORD_LEN, 0},
    {0x0F12, 0xE1D1, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE351, WORD_LEN, 0},
    {0x0F12, 0x001C, WORD_LEN, 0},
    {0x0F12, 0x0A00, WORD_LEN, 0},
    {0x0F12, 0x1000, WORD_LEN, 0},
    {0x0F12, 0xE3A0, WORD_LEN, 0},
    {0x0F12, 0x1000, WORD_LEN, 0},
    {0x0F12, 0xE5C0, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE3A0, WORD_LEN, 0},
    {0x0F12, 0x1002, WORD_LEN, 0},
    {0x0F12, 0xE28D, WORD_LEN, 0},
    {0x0F12, 0x0015, WORD_LEN, 0},
    {0x0F12, 0xEA00, WORD_LEN, 0},
    {0x0F12, 0x2000, WORD_LEN, 0},
    {0x0F12, 0xE5D1, WORD_LEN, 0},
    {0x0F12, 0x3001, WORD_LEN, 0},
    {0x0F12, 0xE5D1, WORD_LEN, 0},
    {0x0F12, 0x3403, WORD_LEN, 0},
    {0x0F12, 0xE182, WORD_LEN, 0},
    {0x0F12, 0xC2A8, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x2080, WORD_LEN, 0},
    {0x0F12, 0xE08C, WORD_LEN, 0},
    {0x0F12, 0xE7B4, WORD_LEN, 0},
    {0x0F12, 0xE1D2, WORD_LEN, 0},
    {0x0F12, 0x039E, WORD_LEN, 0},
    {0x0F12, 0xE004, WORD_LEN, 0},
    {0x0F12, 0xE80F, WORD_LEN, 0},
    {0x0F12, 0xE3E0, WORD_LEN, 0},
    {0x0F12, 0x4624, WORD_LEN, 0},
    {0x0F12, 0xE00E, WORD_LEN, 0},
    {0x0F12, 0x47B4, WORD_LEN, 0},
    {0x0F12, 0xE1C2, WORD_LEN, 0},
    {0x0F12, 0x4004, WORD_LEN, 0},
    {0x0F12, 0xE280, WORD_LEN, 0},
    {0x0F12, 0xC084, WORD_LEN, 0},
    {0x0F12, 0xE08C, WORD_LEN, 0},
    {0x0F12, 0x47B4, WORD_LEN, 0},
    {0x0F12, 0xE1DC, WORD_LEN, 0},
    {0x0F12, 0x0493, WORD_LEN, 0},
    {0x0F12, 0xE004, WORD_LEN, 0},
    {0x0F12, 0x4624, WORD_LEN, 0},
    {0x0F12, 0xE00E, WORD_LEN, 0},
    {0x0F12, 0x47B4, WORD_LEN, 0},
    {0x0F12, 0xE1CC, WORD_LEN, 0},
    {0x0F12, 0xC8B4, WORD_LEN, 0},
    {0x0F12, 0xE1D2, WORD_LEN, 0},
    {0x0F12, 0x039C, WORD_LEN, 0},
    {0x0F12, 0xE003, WORD_LEN, 0},
    {0x0F12, 0x3623, WORD_LEN, 0},
    {0x0F12, 0xE00E, WORD_LEN, 0},
    {0x0F12, 0x38B4, WORD_LEN, 0},
    {0x0F12, 0xE1C2, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},
    {0x0F12, 0xE280, WORD_LEN, 0},
    {0x0F12, 0x1002, WORD_LEN, 0},
    {0x0F12, 0xE281, WORD_LEN, 0},
    {0x0F12, 0x0004, WORD_LEN, 0},
    {0x0F12, 0xE350, WORD_LEN, 0},
    {0x0F12, 0xFFE7, WORD_LEN, 0},
    {0x0F12, 0xBAFF, WORD_LEN, 0},
    {0x0F12, 0x403E, WORD_LEN, 0},
    {0x0F12, 0xE8BD, WORD_LEN, 0},
    {0x0F12, 0xFF1E, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0x4010, WORD_LEN, 0},
    {0x0F12, 0xE92D, WORD_LEN, 0},
    {0x0F12, 0x00AB, WORD_LEN, 0},
    {0x0F12, 0xEB00, WORD_LEN, 0},
    {0x0F12, 0x0248, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x00B2, WORD_LEN, 0},
    {0x0F12, 0xE1D0, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE350, WORD_LEN, 0},
    {0x0F12, 0x0004, WORD_LEN, 0},
    {0x0F12, 0x0A00, WORD_LEN, 0},
    {0x0F12, 0x0080, WORD_LEN, 0},
    {0x0F12, 0xE310, WORD_LEN, 0},
    {0x0F12, 0x0002, WORD_LEN, 0},
    {0x0F12, 0x1A00, WORD_LEN, 0},
    {0x0F12, 0x1234, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},
    {0x0F12, 0xE3A0, WORD_LEN, 0},
    {0x0F12, 0x0DB2, WORD_LEN, 0},
    {0x0F12, 0xE1C1, WORD_LEN, 0},
    {0x0F12, 0x4010, WORD_LEN, 0},
    {0x0F12, 0xE8BD, WORD_LEN, 0},
    {0x0F12, 0xFF1E, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0x4010, WORD_LEN, 0},
    {0x0F12, 0xE92D, WORD_LEN, 0},
    {0x0F12, 0x4000, WORD_LEN, 0},
    {0x0F12, 0xE590, WORD_LEN, 0},
    {0x0F12, 0x0004, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0x009F, WORD_LEN, 0},
    {0x0F12, 0xEB00, WORD_LEN, 0},
    {0x0F12, 0x0214, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE5D0, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE350, WORD_LEN, 0},
    {0x0F12, 0x0002, WORD_LEN, 0},
    {0x0F12, 0x0A00, WORD_LEN, 0},
    {0x0F12, 0x0004, WORD_LEN, 0},
    {0x0F12, 0xE594, WORD_LEN, 0},
    {0x0F12, 0x00A0, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0x0004, WORD_LEN, 0},
    {0x0F12, 0xE584, WORD_LEN, 0},
    {0x0F12, 0x4010, WORD_LEN, 0},
    {0x0F12, 0xE8BD, WORD_LEN, 0},
    {0x0F12, 0xFF1E, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0x4070, WORD_LEN, 0},
    {0x0F12, 0xE92D, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE590, WORD_LEN, 0},
    {0x0F12, 0x0800, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0x0820, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0x4041, WORD_LEN, 0},
    {0x0F12, 0xE280, WORD_LEN, 0},
    {0x0F12, 0x01E0, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x11B8, WORD_LEN, 0},
    {0x0F12, 0xE1D0, WORD_LEN, 0},
    {0x0F12, 0x51B6, WORD_LEN, 0},
    {0x0F12, 0xE1D0, WORD_LEN, 0},
    {0x0F12, 0x0005, WORD_LEN, 0},
    {0x0F12, 0xE041, WORD_LEN, 0},
    {0x0F12, 0x0094, WORD_LEN, 0},
    {0x0F12, 0xE000, WORD_LEN, 0},
    {0x0F12, 0x1D11, WORD_LEN, 0},
    {0x0F12, 0xE3A0, WORD_LEN, 0},
    {0x0F12, 0x008D, WORD_LEN, 0},
    {0x0F12, 0xEB00, WORD_LEN, 0},
    {0x0F12, 0x11C0, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x1000, WORD_LEN, 0},
    {0x0F12, 0xE5D1, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE351, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0x0A00, WORD_LEN, 0},
    {0x0F12, 0x00A0, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0x21A8, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x3FB0, WORD_LEN, 0},
    {0x0F12, 0xE1D2, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE353, WORD_LEN, 0},
    {0x0F12, 0x0003, WORD_LEN, 0},
    {0x0F12, 0x0A00, WORD_LEN, 0},
    {0x0F12, 0x31A4, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x5BB2, WORD_LEN, 0},
    {0x0F12, 0xE1C3, WORD_LEN, 0},
    {0x0F12, 0xC000, WORD_LEN, 0},
    {0x0F12, 0xE085, WORD_LEN, 0},
    {0x0F12, 0xCBB4, WORD_LEN, 0},
    {0x0F12, 0xE1C3, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE351, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0x0A00, WORD_LEN, 0},
    {0x0F12, 0x0080, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0x1DBC, WORD_LEN, 0},
    {0x0F12, 0xE1D2, WORD_LEN, 0},
    {0x0F12, 0x3EB4, WORD_LEN, 0},
    {0x0F12, 0xE1D2, WORD_LEN, 0},
    {0x0F12, 0x2EB2, WORD_LEN, 0},
    {0x0F12, 0xE1D2, WORD_LEN, 0},
    {0x0F12, 0x0193, WORD_LEN, 0},
    {0x0F12, 0xE001, WORD_LEN, 0},
    {0x0F12, 0x0092, WORD_LEN, 0},
    {0x0F12, 0xE000, WORD_LEN, 0},
    {0x0F12, 0x2811, WORD_LEN, 0},
    {0x0F12, 0xE3A0, WORD_LEN, 0},
    {0x0F12, 0x0194, WORD_LEN, 0},
    {0x0F12, 0xE001, WORD_LEN, 0},
    {0x0F12, 0x0092, WORD_LEN, 0},
    {0x0F12, 0xE000, WORD_LEN, 0},
    {0x0F12, 0x11A1, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0x01A0, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0x0072, WORD_LEN, 0},
    {0x0F12, 0xEB00, WORD_LEN, 0},
    {0x0F12, 0x1160, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x02B4, WORD_LEN, 0},
    {0x0F12, 0xE1C1, WORD_LEN, 0},
    {0x0F12, 0x4070, WORD_LEN, 0},
    {0x0F12, 0xE8BD, WORD_LEN, 0},
    {0x0F12, 0xFF1E, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0x4010, WORD_LEN, 0},
    {0x0F12, 0xE92D, WORD_LEN, 0},
    {0x0F12, 0x006E, WORD_LEN, 0},
    {0x0F12, 0xEB00, WORD_LEN, 0},
    {0x0F12, 0x2148, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x14B0, WORD_LEN, 0},
    {0x0F12, 0xE1D2, WORD_LEN, 0},
    {0x0F12, 0x0080, WORD_LEN, 0},
    {0x0F12, 0xE311, WORD_LEN, 0},
    {0x0F12, 0x0005, WORD_LEN, 0},
    {0x0F12, 0x0A00, WORD_LEN, 0},
    {0x0F12, 0x013C, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x00B0, WORD_LEN, 0},
    {0x0F12, 0xE1D0, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},
    {0x0F12, 0xE350, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},
    {0x0F12, 0x9A00, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},
    {0x0F12, 0xE3A0, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xEA00, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE3A0, WORD_LEN, 0},
    {0x0F12, 0x3110, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE5C3, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE5D3, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE350, WORD_LEN, 0},
    {0x0F12, 0x0003, WORD_LEN, 0},
    {0x0F12, 0x0A00, WORD_LEN, 0},
    {0x0F12, 0x0080, WORD_LEN, 0},
    {0x0F12, 0xE3C1, WORD_LEN, 0},
    {0x0F12, 0x110C, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x04B0, WORD_LEN, 0},
    {0x0F12, 0xE1C2, WORD_LEN, 0},
    {0x0F12, 0x00B2, WORD_LEN, 0},
    {0x0F12, 0xE1C1, WORD_LEN, 0},
    {0x0F12, 0x4010, WORD_LEN, 0},
    {0x0F12, 0xE8BD, WORD_LEN, 0},
    {0x0F12, 0xFF1E, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0x41F0, WORD_LEN, 0},
    {0x0F12, 0xE92D, WORD_LEN, 0},
    {0x0F12, 0x1000, WORD_LEN, 0},
    {0x0F12, 0xE590, WORD_LEN, 0},
    {0x0F12, 0xC801, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0xC82C, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0x1004, WORD_LEN, 0},
    {0x0F12, 0xE590, WORD_LEN, 0},
    {0x0F12, 0x1801, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0x1821, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0x4008, WORD_LEN, 0},
    {0x0F12, 0xE590, WORD_LEN, 0},
    {0x0F12, 0x500C, WORD_LEN, 0},
    {0x0F12, 0xE590, WORD_LEN, 0},
    {0x0F12, 0x2004, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0x3005, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0x000C, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0x004E, WORD_LEN, 0},
    {0x0F12, 0xEB00, WORD_LEN, 0},
    {0x0F12, 0x60A0, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x00B2, WORD_LEN, 0},
    {0x0F12, 0xE1D6, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE350, WORD_LEN, 0},
    {0x0F12, 0x000E, WORD_LEN, 0},
    {0x0F12, 0x0A00, WORD_LEN, 0},
    {0x0F12, 0x00B8, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x05B4, WORD_LEN, 0},
    {0x0F12, 0xE1D0, WORD_LEN, 0},
    {0x0F12, 0x0002, WORD_LEN, 0},
    {0x0F12, 0xE350, WORD_LEN, 0},
    {0x0F12, 0x000A, WORD_LEN, 0},
    {0x0F12, 0x1A00, WORD_LEN, 0},
    {0x0F12, 0x70AC, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x10F4, WORD_LEN, 0},
    {0x0F12, 0xE1D6, WORD_LEN, 0},
    {0x0F12, 0x26B0, WORD_LEN, 0},
    {0x0F12, 0xE1D7, WORD_LEN, 0},
    {0x0F12, 0x00F0, WORD_LEN, 0},
    {0x0F12, 0xE1D4, WORD_LEN, 0},
    {0x0F12, 0x0044, WORD_LEN, 0},
    {0x0F12, 0xEB00, WORD_LEN, 0},
    {0x0F12, 0x00B0, WORD_LEN, 0},
    {0x0F12, 0xE1C4, WORD_LEN, 0},
    {0x0F12, 0x26B0, WORD_LEN, 0},
    {0x0F12, 0xE1D7, WORD_LEN, 0},
    {0x0F12, 0x10F6, WORD_LEN, 0},
    {0x0F12, 0xE1D6, WORD_LEN, 0},
    {0x0F12, 0x00F0, WORD_LEN, 0},
    {0x0F12, 0xE1D5, WORD_LEN, 0},
    {0x0F12, 0x003F, WORD_LEN, 0},
    {0x0F12, 0xEB00, WORD_LEN, 0},
    {0x0F12, 0x00B0, WORD_LEN, 0},
    {0x0F12, 0xE1C5, WORD_LEN, 0},
    {0x0F12, 0x41F0, WORD_LEN, 0},
    {0x0F12, 0xE8BD, WORD_LEN, 0},
    {0x0F12, 0xFF1E, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0x4010, WORD_LEN, 0},
    {0x0F12, 0xE92D, WORD_LEN, 0},
    {0x0F12, 0x4000, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0x1004, WORD_LEN, 0},
    {0x0F12, 0xE594, WORD_LEN, 0},
    {0x0F12, 0x0040, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x00B0, WORD_LEN, 0},
    {0x0F12, 0xE1D0, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE350, WORD_LEN, 0},
    {0x0F12, 0x0008, WORD_LEN, 0},
    {0x0F12, 0x0A00, WORD_LEN, 0},
    {0x0F12, 0x005C, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x3001, WORD_LEN, 0},
    {0x0F12, 0xE1A0, WORD_LEN, 0},
    {0x0F12, 0x2068, WORD_LEN, 0},
    {0x0F12, 0xE590, WORD_LEN, 0},
    {0x0F12, 0x0054, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0x1005, WORD_LEN, 0},
    {0x0F12, 0xE3A0, WORD_LEN, 0},
    {0x0F12, 0x0032, WORD_LEN, 0},
    {0x0F12, 0xEB00, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE584, WORD_LEN, 0},
    {0x0F12, 0x4010, WORD_LEN, 0},
    {0x0F12, 0xE8BD, WORD_LEN, 0},
    {0x0F12, 0xFF1E, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE594, WORD_LEN, 0},
    {0x0F12, 0x0030, WORD_LEN, 0},
    {0x0F12, 0xEB00, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xE584, WORD_LEN, 0},
    {0x0F12, 0xFFF9, WORD_LEN, 0},
    {0x0F12, 0xEAFF, WORD_LEN, 0},
    {0x0F12, 0x28E8, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x3370, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x1272, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x1728, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x112C, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x28EC, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x122C, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0xF200, WORD_LEN, 0},
    {0x0F12, 0xD000, WORD_LEN, 0},
    {0x0F12, 0x2340, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x0E2C, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0xF400, WORD_LEN, 0},
    {0x0F12, 0xD000, WORD_LEN, 0},
    {0x0F12, 0x0CDC, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x20D4, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x06D4, WORD_LEN, 0},
    {0x0F12, 0x7000, WORD_LEN, 0},
    {0x0F12, 0x4778, WORD_LEN, 0},
    {0x0F12, 0x46C0, WORD_LEN, 0},
    {0x0F12, 0xC000, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0xFF1C, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0xC091, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xC000, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0xFF1C, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0x0467, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xC000, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0xFF1C, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0x2FA7, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xC000, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0xFF1C, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0xCB1F, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xC000, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0xFF1C, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0x058F, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xC000, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0xFF1C, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0xA0F1, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xF004, WORD_LEN, 0},
    {0x0F12, 0xE51F, WORD_LEN, 0},
    {0x0F12, 0xD14C, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xC000, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0xFF1C, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0x2B43, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xC000, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0xFF1C, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0x8725, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xC000, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0xFF1C, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0x6777, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xC000, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0xFF1C, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0x8E49, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0xC000, WORD_LEN, 0},
    {0x0F12, 0xE59F, WORD_LEN, 0},
    {0x0F12, 0xFF1C, WORD_LEN, 0},
    {0x0F12, 0xE12F, WORD_LEN, 0},
    {0x0F12, 0x8EDD, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0x96FF, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
// End of Patch Data(Last : 700027C2h) */
// Total Size 852 (0x0354)             */
// Addr : 2470 , Size : 850(352h)      */

//==================================================================================
// 04.Analog Setting & APS Control
//==================================================================================
//This register is for FACTORY ONLY. 
//If you change it without prior notification
//YOU are RESPONSIBLE for the FAILURE that will happen in the future
    {0x0028, 0x7000, WORD_LEN, 0},
    {0x002A, 0x0E38, WORD_LEN, 0},
    {0x0F12, 0x0476, WORD_LEN, 0},	//senHal_RegCompBiasNormSf //CDS bias
    {0x0F12, 0x0476, WORD_LEN, 0},	//senHal_RegCompBiasYAv //CDS bias
    {0x002A, 0x0AA0, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},	//setot_bUseDigitalHbin //1-Digital, 0-Analog
    {0x002A, 0x0E2C, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},	//senHal_bUseAnalogVerAv //2-Adding/averaging, 1-Y-Avg, 0-PLA
    {0x002A, 0x0E66, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},	//senHal_RegBlstEnNorm
    {0x002A, 0x1250, WORD_LEN, 0},
    {0x0F12, 0xFFFF, WORD_LEN, 0}, 	//senHal_Bls_nSpExpLines
    {0x002A, 0x1202, WORD_LEN, 0},
    {0x0F12, 0x0010, WORD_LEN, 0}, 	//senHal_Dblr_VcoFreqMHZ
//ADLC Filter
    {0x002A, 0x1288, WORD_LEN, 0},
    {0x0F12, 0x020F, WORD_LEN, 0},	//gisp_dadlc_ResetFilterValue
    {0x0F12, 0x1C02, WORD_LEN, 0},	//gisp_dadlc_SteadyFilterValue
    {0x0F12, 0x0006, WORD_LEN, 0},	//gisp_dadlc_NResetIIrFrames	

//*************************************/
// 05.OTP Control                     */
//*************************************/
    {0x002A, 0x3378, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},// Tune_TP_bReMultGainsByNvm */

//*********************************************************************************
// 06.GAS (Grid Anti-Shading)                                                     
//*********************************************************************************
    {0x002A, 0x1326, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},	// gisp_gos_Enable 
    {0x002A, 0x063A, WORD_LEN, 0},
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_0__0_ Horizon 
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_0__1_         
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_0__2_         
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_0__3_         
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_1__0_ IncandA 
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_1__1_         
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_1__2_         
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_1__3_         
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_2__0_ WW      
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_2__1_         
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_2__2_         
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_2__3_         
    {0x0F12, 0x00E8, WORD_LEN, 0},	// TVAR_ash_GASAlpha_3__0_ CW      
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_3__1_         
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_3__2_         
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_3__3_         
    {0x0F12, 0x00C8, WORD_LEN, 0},	// TVAR_ash_GASAlpha_4__0_ D50     
    {0x0F12, 0x00F8, WORD_LEN, 0},	// TVAR_ash_GASAlpha_4__1_         
    {0x0F12, 0x00F8, WORD_LEN, 0},	// TVAR_ash_GASAlpha_4__2_         
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_4__3_         
    {0x0F12, 0x00F0, WORD_LEN, 0},	// TVAR_ash_GASAlpha_5__0_ D65     
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_5__1_         
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_5__2_         
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_5__3_         
    {0x0F12, 0x00F0, WORD_LEN, 0},	// TVAR_ash_GASAlpha_6__0_ D75     
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_6__1_         
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_6__2_         
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASAlpha_6__3_         
    {0x002A, 0x067A, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_0__0_ Horizon 
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_0__1_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_0__2_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_0__3_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_1__0_ IncandA 
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_1__1_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_1__2_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_1__3_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_2__0_ WW      
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_2__1_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_2__2_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_2__3_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_3__0_ CW      
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_3__1_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_3__2_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_3__3_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_4__0_ D50     
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_4__1_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_4__2_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_4__3_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_5__0_ D65     
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_5__1_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_5__2_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_5__3_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_6__0_ D75     
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_6__1_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_6__2_         
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASBeta_6__3_         
    {0x002A, 0x06BA, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},	//ash_bLumaMode 
    {0x002A, 0x0632, WORD_LEN, 0},
    {0x0F12, 0x0100, WORD_LEN, 0},	// ash_CGrasAlphas_0_ 
    {0x0F12, 0x0100, WORD_LEN, 0},	// ash_CGrasAlphas_1_ 
    {0x0F12, 0x0100, WORD_LEN, 0},	// ash_CGrasAlphas_2_ 
    {0x0F12, 0x0100, WORD_LEN, 0},	// ash_CGrasAlphas_3_ 
    {0x002A, 0x0672, WORD_LEN, 0},
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASOutdoorAlpha_0_ 
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASOutdoorAlpha_1_ 
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASOutdoorAlpha_2_ 
    {0x0F12, 0x0100, WORD_LEN, 0},	// TVAR_ash_GASOutdoorAlpha_3_ 
    {0x002A, 0x06B2, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASOutdoorBeta_0_ 
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASOutdoorBeta_1_ 
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASOutdoorBeta_2_ 
    {0x0F12, 0x0000, WORD_LEN, 0},	// ash_GASOutdoorBeta_3_ 
    {0x002A, 0x06D0, WORD_LEN, 0},
    {0x0F12, 0x000D, WORD_LEN, 0},	// ash_uParabolicScalingA
    {0x0F12, 0x000F, WORD_LEN, 0},	// ash_uParabolicScalingB
    {0x002A, 0x06CC, WORD_LEN, 0},
    {0x0F12, 0x0280, WORD_LEN, 0},	// ash_uParabolicCenterX 
    {0x0F12, 0x01E0, WORD_LEN, 0},	// ash_uParabolicCenterY 
    {0x002A, 0x06C6, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},	// ash_bParabolicEstimation 
    {0x002A, 0x0624, WORD_LEN, 0},
    {0x0F12, 0x009D, WORD_LEN, 0},	// TVAR_ash_AwbAshCord_0_ Horizon 
    {0x0F12, 0x00D5, WORD_LEN, 0},	// TVAR_ash_AwbAshCord_1_ IncandA 
    {0x0F12, 0x0103, WORD_LEN, 0},	// TVAR_ash_AwbAshCord_2_ WW      
    {0x0F12, 0x0128, WORD_LEN, 0},	// TVAR_ash_AwbAshCord_3_ CW      
    {0x0F12, 0x0166, WORD_LEN, 0},	// TVAR_ash_AwbAshCord_4_ D50     
    {0x0F12, 0x0193, WORD_LEN, 0},	// TVAR_ash_AwbAshCord_5_ D65     
    {0x0F12, 0x01A0, WORD_LEN, 0},	// TVAR_ash_AwbAshCord_6_ D75  

    {0x002A, 0x347C, WORD_LEN, 0},
    {0x0F12, 0x013B, WORD_LEN, 0},	// 011A 012F Tune_wbt_GAS_0_   
    {0x0F12, 0x0116, WORD_LEN, 0},	// 011A 0111 Tune_wbt_GAS_1_   
    {0x0F12, 0x00D9, WORD_LEN, 0},	// 00EE 00D6 Tune_wbt_GAS_2_   
    {0x0F12, 0x00A6, WORD_LEN, 0},	// 00C1 009E Tune_wbt_GAS_3_   
    {0x0F12, 0x0082, WORD_LEN, 0},	// 009E 007A Tune_wbt_GAS_4_   
    {0x0F12, 0x006C, WORD_LEN, 0},	// 008A 0064 Tune_wbt_GAS_5_   
    {0x0F12, 0x0065, WORD_LEN, 0},	// 0083 005D Tune_wbt_GAS_6_   
    {0x0F12, 0x006C, WORD_LEN, 0},	// 008A 0063 Tune_wbt_GAS_7_   
    {0x0F12, 0x0080, WORD_LEN, 0},	// 009E 007B Tune_wbt_GAS_8_   
    {0x0F12, 0x00A3, WORD_LEN, 0},	// 00BF 00A3 Tune_wbt_GAS_9_   
    {0x0F12, 0x00D4, WORD_LEN, 0},	// 00E5 00E3 Tune_wbt_GAS_10_  
    {0x0F12, 0x010D, WORD_LEN, 0},	// 00F9 013B Tune_wbt_GAS_11_  
    {0x0F12, 0x012E, WORD_LEN, 0},	// 0124 018B Tune_wbt_GAS_12_  
    {0x0F12, 0x0138, WORD_LEN, 0},	// 0126 012B Tune_wbt_GAS_13_  
    {0x0F12, 0x0104, WORD_LEN, 0},	// 010E 00F6 Tune_wbt_GAS_14_  
    {0x0F12, 0x00BE, WORD_LEN, 0},	// 00D3 00B2 Tune_wbt_GAS_15_  
    {0x0F12, 0x0088, WORD_LEN, 0},	// 009F 007B Tune_wbt_GAS_16_  
    {0x0F12, 0x0062, WORD_LEN, 0},	// 007C 0057 Tune_wbt_GAS_17_  
    {0x0F12, 0x004D, WORD_LEN, 0},	// 0068 0042 Tune_wbt_GAS_18_  
    {0x0F12, 0x0046, WORD_LEN, 0},	// 0061 0039 Tune_wbt_GAS_19_  
    {0x0F12, 0x004C, WORD_LEN, 0},	// 0068 003F Tune_wbt_GAS_20_  
    {0x0F12, 0x0060, WORD_LEN, 0},	// 007E 0053 Tune_wbt_GAS_21_  
    {0x0F12, 0x0084, WORD_LEN, 0},	// 00A3 007A Tune_wbt_GAS_22_  
    {0x0F12, 0x00B8, WORD_LEN, 0},	// 00C9 00B6 Tune_wbt_GAS_23_  
    {0x0F12, 0x00F9, WORD_LEN, 0},	// 00F0 0110 Tune_wbt_GAS_24_  
    {0x0F12, 0x012C, WORD_LEN, 0},	// 0131 016A Tune_wbt_GAS_25_  
    {0x0F12, 0x011A, WORD_LEN, 0},	// 011C 0114 Tune_wbt_GAS_26_  
    {0x0F12, 0x00DB, WORD_LEN, 0},	// 00EB 00D4 Tune_wbt_GAS_27_  
    {0x0F12, 0x0093, WORD_LEN, 0},	// 00AA 008F Tune_wbt_GAS_28_  
    {0x0F12, 0x005F, WORD_LEN, 0},	// 0075 005A Tune_wbt_GAS_29_  
    {0x0F12, 0x003C, WORD_LEN, 0},	// 0053 0035 Tune_wbt_GAS_30_  
    {0x0F12, 0x0027, WORD_LEN, 0},	// 003F 0020 Tune_wbt_GAS_31_  
    {0x0F12, 0x0020, WORD_LEN, 0},	// 0038 0019 Tune_wbt_GAS_32_  
    {0x0F12, 0x0026, WORD_LEN, 0},	// 0040 001F Tune_wbt_GAS_33_  
    {0x0F12, 0x003A, WORD_LEN, 0},	// 0055 0032 Tune_wbt_GAS_34_  
    {0x0F12, 0x005C, WORD_LEN, 0},	// 007A 0056 Tune_wbt_GAS_35_  
    {0x0F12, 0x008E, WORD_LEN, 0},	// 00A6 008E Tune_wbt_GAS_36_  
    {0x0F12, 0x00D2, WORD_LEN, 0},	// 00D5 00E6 Tune_wbt_GAS_37_  
    {0x0F12, 0x010E, WORD_LEN, 0},	// 0126 0142 Tune_wbt_GAS_38_  
    {0x0F12, 0x0101, WORD_LEN, 0},	// 00F6 0102 Tune_wbt_GAS_39_  
    {0x0F12, 0x00BF, WORD_LEN, 0},	// 00C0 00BE Tune_wbt_GAS_40_  
    {0x0F12, 0x0077, WORD_LEN, 0},	// 007D 0077 Tune_wbt_GAS_41_  
    {0x0F12, 0x0044, WORD_LEN, 0},	// 004D 0045 Tune_wbt_GAS_42_  
    {0x0F12, 0x0023, WORD_LEN, 0},	// 002C 0022 Tune_wbt_GAS_43_  
    {0x0F12, 0x0011, WORD_LEN, 0},	// 001B 000D Tune_wbt_GAS_44_  
    {0x0F12, 0x000C, WORD_LEN, 0},	// 0017 0006 Tune_wbt_GAS_45_  
    {0x0F12, 0x0010, WORD_LEN, 0},	// 001B 000A Tune_wbt_GAS_46_  
    {0x0F12, 0x0022, WORD_LEN, 0},	// 002D 001D Tune_wbt_GAS_47_  
    {0x0F12, 0x0043, WORD_LEN, 0},	// 004E 003F Tune_wbt_GAS_48_  
    {0x0F12, 0x0074, WORD_LEN, 0},	// 0080 0075 Tune_wbt_GAS_49_  
    {0x0F12, 0x00B7, WORD_LEN, 0},	// 00C1 00CC Tune_wbt_GAS_50_  
    {0x0F12, 0x00F7, WORD_LEN, 0},	// 00FF 0126 Tune_wbt_GAS_51_  
    {0x0F12, 0x00FC, WORD_LEN, 0},	// 00EA 00FB Tune_wbt_GAS_52_  
    {0x0F12, 0x00B7, WORD_LEN, 0},	// 00B0 00B7 Tune_wbt_GAS_53_  
    {0x0F12, 0x006F, WORD_LEN, 0},	// 006D 0070 Tune_wbt_GAS_54_  
    {0x0F12, 0x003C, WORD_LEN, 0},	// 003D 003D Tune_wbt_GAS_55_  
    {0x0F12, 0x001C, WORD_LEN, 0},	// 001E 001B Tune_wbt_GAS_56_  
    {0x0F12, 0x000A, WORD_LEN, 0},	// 000F 0007 Tune_wbt_GAS_57_  
    {0x0F12, 0x0004, WORD_LEN, 0},	// 000A 0000 Tune_wbt_GAS_58_  
    {0x0F12, 0x000A, WORD_LEN, 0},	// 0010 0004 Tune_wbt_GAS_59_  
    {0x0F12, 0x001B, WORD_LEN, 0},	// 001E 0015 Tune_wbt_GAS_60_  
    {0x0F12, 0x003B, WORD_LEN, 0},	// 003E 0034 Tune_wbt_GAS_61_  
    {0x0F12, 0x006C, WORD_LEN, 0},	// 006F 006A Tune_wbt_GAS_62_  
    {0x0F12, 0x00B0, WORD_LEN, 0},	// 00B2 00C0 Tune_wbt_GAS_63_  
    {0x0F12, 0x00F2, WORD_LEN, 0},	// 00F1 011B Tune_wbt_GAS_64_  
    {0x0F12, 0x00EF, WORD_LEN, 0},	// 00E0 0102 Tune_wbt_GAS_65_  
    {0x0F12, 0x00AB, WORD_LEN, 0},	// 00A6 00BA Tune_wbt_GAS_66_  
    {0x0F12, 0x0065, WORD_LEN, 0},	// 0063 0073 Tune_wbt_GAS_67_  
    {0x0F12, 0x0034, WORD_LEN, 0},	// 0033 0040 Tune_wbt_GAS_68_  
    {0x0F12, 0x0015, WORD_LEN, 0},	// 0016 001D Tune_wbt_GAS_69_  
    {0x0F12, 0x0004, WORD_LEN, 0},	// 0008 0009 Tune_wbt_GAS_70_  
    {0x0F12, 0x0000, WORD_LEN, 0},	// 0003 0001 Tune_wbt_GAS_71_  
    {0x0F12, 0x0004, WORD_LEN, 0},	// 0009 0006 Tune_wbt_GAS_72_  
    {0x0F12, 0x0013, WORD_LEN, 0},	// 0016 0017 Tune_wbt_GAS_73_  
    {0x0F12, 0x0033, WORD_LEN, 0},	// 0035 0039 Tune_wbt_GAS_74_  
    {0x0F12, 0x0063, WORD_LEN, 0},	// 0066 006F Tune_wbt_GAS_75_  
    {0x0F12, 0x00A5, WORD_LEN, 0},	// 00A7 00C8 Tune_wbt_GAS_76_  
    {0x0F12, 0x00E5, WORD_LEN, 0},	// 00E9 011E Tune_wbt_GAS_77_  
    {0x0F12, 0x00F7, WORD_LEN, 0},	// 00D5 0111 Tune_wbt_GAS_78_  
    {0x0F12, 0x00B4, WORD_LEN, 0},	// 009D 00C8 Tune_wbt_GAS_79_  
    {0x0F12, 0x006D, WORD_LEN, 0},	// 005D 0081 Tune_wbt_GAS_80_  
    {0x0F12, 0x003C, WORD_LEN, 0},	// 002F 004D Tune_wbt_GAS_81_  
    {0x0F12, 0x001C, WORD_LEN, 0},	// 0010 0028 Tune_wbt_GAS_82_  
    {0x0F12, 0x000B, WORD_LEN, 0},	// 0004 0014 Tune_wbt_GAS_83_  
    {0x0F12, 0x0005, WORD_LEN, 0},	// 0001 000B Tune_wbt_GAS_84_  
    {0x0F12, 0x000A, WORD_LEN, 0},	// 0005 0010 Tune_wbt_GAS_85_  
    {0x0F12, 0x001B, WORD_LEN, 0},	// 0013 0022 Tune_wbt_GAS_86_  
    {0x0F12, 0x003B, WORD_LEN, 0},	// 0031 0047 Tune_wbt_GAS_87_  
    {0x0F12, 0x006B, WORD_LEN, 0},	// 005F 007E Tune_wbt_GAS_88_  
    {0x0F12, 0x00AD, WORD_LEN, 0},	// 00A1 00D8 Tune_wbt_GAS_89_  
    {0x0F12, 0x00ED, WORD_LEN, 0},	// 00DF 0131 Tune_wbt_GAS_90_  
    {0x0F12, 0x010B, WORD_LEN, 0},	// 00E9 0129 Tune_wbt_GAS_91_  
    {0x0F12, 0x00CB, WORD_LEN, 0},	// 00B2 00E4 Tune_wbt_GAS_92_  
    {0x0F12, 0x0085, WORD_LEN, 0},	// 006F 009E Tune_wbt_GAS_93_  
    {0x0F12, 0x0051, WORD_LEN, 0},	// 003F 0068 Tune_wbt_GAS_94_  
    {0x0F12, 0x002F, WORD_LEN, 0},	// 0020 0041 Tune_wbt_GAS_95_  
    {0x0F12, 0x001C, WORD_LEN, 0},	// 000F 002B Tune_wbt_GAS_96_  
    {0x0F12, 0x0016, WORD_LEN, 0},	// 000B 0023 Tune_wbt_GAS_97_  
    {0x0F12, 0x001C, WORD_LEN, 0},	// 0010 0028 Tune_wbt_GAS_98_  
    {0x0F12, 0x002E, WORD_LEN, 0},	// 0021 003B Tune_wbt_GAS_99_  
    {0x0F12, 0x004F, WORD_LEN, 0},	// 0041 0063 Tune_wbt_GAS_100_ 
    {0x0F12, 0x0081, WORD_LEN, 0},	// 0071 00A0 Tune_wbt_GAS_101_ 
    {0x0F12, 0x00C4, WORD_LEN, 0},	// 00B4 00FD Tune_wbt_GAS_102_ 
    {0x0F12, 0x0102, WORD_LEN, 0},	// 00F6 015A Tune_wbt_GAS_103_ 
    {0x0F12, 0x0119, WORD_LEN, 0},	// 00F9 014A Tune_wbt_GAS_104_ 
    {0x0F12, 0x00DF, WORD_LEN, 0},	// 00C2 010D Tune_wbt_GAS_105_ 
    {0x0F12, 0x009B, WORD_LEN, 0},	// 0082 00CA Tune_wbt_GAS_106_ 
    {0x0F12, 0x0067, WORD_LEN, 0},	// 0053 008F Tune_wbt_GAS_107_ 
    {0x0F12, 0x0045, WORD_LEN, 0},	// 0033 0066 Tune_wbt_GAS_108_ 
    {0x0F12, 0x0030, WORD_LEN, 0},	// 0021 004F Tune_wbt_GAS_109_ 
    {0x0F12, 0x0029, WORD_LEN, 0},	// 001C 0048 Tune_wbt_GAS_110_ 
    {0x0F12, 0x002F, WORD_LEN, 0},	// 0022 004E Tune_wbt_GAS_111_ 
    {0x0F12, 0x0043, WORD_LEN, 0},	// 0035 0067 Tune_wbt_GAS_112_ 
    {0x0F12, 0x0066, WORD_LEN, 0},	// 0056 008F Tune_wbt_GAS_113_ 
    {0x0F12, 0x0098, WORD_LEN, 0},	// 0087 00D1 Tune_wbt_GAS_114_ 
    {0x0F12, 0x00D9, WORD_LEN, 0},	// 00CA 0132 Tune_wbt_GAS_115_ 
    {0x0F12, 0x010F, WORD_LEN, 0},	// 0107 018D Tune_wbt_GAS_116_ 
    {0x0F12, 0x0138, WORD_LEN, 0},	// 0108 0159 Tune_wbt_GAS_117_ 
    {0x0F12, 0x010C, WORD_LEN, 0},	// 00E0 0141 Tune_wbt_GAS_118_ 
    {0x0F12, 0x00CB, WORD_LEN, 0},	// 00A2 0103 Tune_wbt_GAS_119_ 
    {0x0F12, 0x0097, WORD_LEN, 0},	// 0072 00C9 Tune_wbt_GAS_120_ 
    {0x0F12, 0x0073, WORD_LEN, 0},	// 0052 009E Tune_wbt_GAS_121_ 
    {0x0F12, 0x005C, WORD_LEN, 0},	// 0040 0087 Tune_wbt_GAS_122_ 
    {0x0F12, 0x0054, WORD_LEN, 0},	// 003A 007D Tune_wbt_GAS_123_ 
    {0x0F12, 0x005B, WORD_LEN, 0},	// 0041 0087 Tune_wbt_GAS_124_ 
    {0x0F12, 0x0070, WORD_LEN, 0},	// 0055 00A2 Tune_wbt_GAS_125_ 
    {0x0F12, 0x0096, WORD_LEN, 0},	// 0077 00D4 Tune_wbt_GAS_126_ 
    {0x0F12, 0x00C9, WORD_LEN, 0},	// 00A8 011A Tune_wbt_GAS_127_ 
    {0x0F12, 0x0106, WORD_LEN, 0},	// 00E7 017D Tune_wbt_GAS_128_ 
    {0x0F12, 0x012D, WORD_LEN, 0},	// 011E 01CF Tune_wbt_GAS_129_ 
    {0x0F12, 0x0147, WORD_LEN, 0},	// 0123 0181 Tune_wbt_GAS_130_ 
    {0x0F12, 0x012F, WORD_LEN, 0},	// 0100 0169 Tune_wbt_GAS_131_ 
    {0x0F12, 0x00F8, WORD_LEN, 0},	// 00CB 0140 Tune_wbt_GAS_132_ 
    {0x0F12, 0x00C5, WORD_LEN, 0},	// 009A 0106 Tune_wbt_GAS_133_ 
    {0x0F12, 0x00A1, WORD_LEN, 0},	// 0079 00DD Tune_wbt_GAS_134_ 
    {0x0F12, 0x008B, WORD_LEN, 0},	// 0067 00C5 Tune_wbt_GAS_135_ 
    {0x0F12, 0x0083, WORD_LEN, 0},	// 0060 00BE Tune_wbt_GAS_136_ 
    {0x0F12, 0x008B, WORD_LEN, 0},	// 0068 00C8 Tune_wbt_GAS_137_ 
    {0x0F12, 0x00A0, WORD_LEN, 0},	// 007B 00E6 Tune_wbt_GAS_138_ 
    {0x0F12, 0x00C2, WORD_LEN, 0},	// 009D 011B Tune_wbt_GAS_139_ 
    {0x0F12, 0x00F3, WORD_LEN, 0},	// 00CD 015F Tune_wbt_GAS_140_ 
    {0x0F12, 0x0124, WORD_LEN, 0},	// 0108 01BC Tune_wbt_GAS_141_ 
    {0x0F12, 0x0139, WORD_LEN, 0},	// 0131 0206 Tune_wbt_GAS_142_ 
    {0x0F12, 0x0093, WORD_LEN, 0},	// 006C 00A8 Tune_wbt_GAS_143_ 
    {0x0F12, 0x007E, WORD_LEN, 0},	// 006E 008F Tune_wbt_GAS_144_ 
    {0x0F12, 0x0062, WORD_LEN, 0},	// 005D 006F Tune_wbt_GAS_145_ 
    {0x0F12, 0x004D, WORD_LEN, 0},	// 004C 0054 Tune_wbt_GAS_146_ 
    {0x0F12, 0x003E, WORD_LEN, 0},	// 0040 0041 Tune_wbt_GAS_147_ 
    {0x0F12, 0x0034, WORD_LEN, 0},	// 0037 0036 Tune_wbt_GAS_148_ 
    {0x0F12, 0x0030, WORD_LEN, 0},	// 0035 0033 Tune_wbt_GAS_149_ 
    {0x0F12, 0x0032, WORD_LEN, 0},	// 0036 0037 Tune_wbt_GAS_150_ 
    {0x0F12, 0x003B, WORD_LEN, 0},	// 003F 0045 Tune_wbt_GAS_151_ 
    {0x0F12, 0x0049, WORD_LEN, 0},	// 004B 005A Tune_wbt_GAS_152_ 
    {0x0F12, 0x005C, WORD_LEN, 0},	// 0053 007A Tune_wbt_GAS_153_ 
    {0x0F12, 0x0077, WORD_LEN, 0},	// 0053 00AA Tune_wbt_GAS_154_ 
    {0x0F12, 0x008A, WORD_LEN, 0},	// 0070 00E2 Tune_wbt_GAS_155_ 
    {0x0F12, 0x0093, WORD_LEN, 0},	// 0075 009E Tune_wbt_GAS_156_ 
    {0x0F12, 0x0077, WORD_LEN, 0},	// 006D 007D Tune_wbt_GAS_157_ 
    {0x0F12, 0x0059, WORD_LEN, 0},	// 0056 005A Tune_wbt_GAS_158_ 
    {0x0F12, 0x0042, WORD_LEN, 0},	// 0043 0041 Tune_wbt_GAS_159_ 
    {0x0F12, 0x0032, WORD_LEN, 0},	// 0037 002E Tune_wbt_GAS_160_ 
    {0x0F12, 0x0027, WORD_LEN, 0},	// 002F 0022 Tune_wbt_GAS_161_ 
    {0x0F12, 0x0024, WORD_LEN, 0},	// 002C 001F Tune_wbt_GAS_162_ 
    {0x0F12, 0x0026, WORD_LEN, 0},	// 002F 0022 Tune_wbt_GAS_163_ 
    {0x0F12, 0x002F, WORD_LEN, 0},	// 0038 0030 Tune_wbt_GAS_164_ 
    {0x0F12, 0x003D, WORD_LEN, 0},	// 0045 0044 Tune_wbt_GAS_165_ 
    {0x0F12, 0x0052, WORD_LEN, 0},	// 004E 0063 Tune_wbt_GAS_166_ 
    {0x0F12, 0x006E, WORD_LEN, 0},	// 0055 0091 Tune_wbt_GAS_167_ 
    {0x0F12, 0x008B, WORD_LEN, 0},	// 007F 00CB Tune_wbt_GAS_168_ 
    {0x0F12, 0x0083, WORD_LEN, 0},	// 0077 0093 Tune_wbt_GAS_169_ 
    {0x0F12, 0x0064, WORD_LEN, 0},	// 0062 006D Tune_wbt_GAS_170_ 
    {0x0F12, 0x0046, WORD_LEN, 0},	// 004A 004A Tune_wbt_GAS_171_ 
    {0x0F12, 0x0030, WORD_LEN, 0},	// 0037 0031 Tune_wbt_GAS_172_ 
    {0x0F12, 0x0020, WORD_LEN, 0},	// 002A 001E Tune_wbt_GAS_173_ 
    {0x0F12, 0x0016, WORD_LEN, 0},	// 0021 0013 Tune_wbt_GAS_174_ 
    {0x0F12, 0x0011, WORD_LEN, 0},	// 001E 000F Tune_wbt_GAS_175_ 
    {0x0F12, 0x0014, WORD_LEN, 0},	// 0021 0013 Tune_wbt_GAS_176_ 
    {0x0F12, 0x001E, WORD_LEN, 0},	// 002B 001F Tune_wbt_GAS_177_ 
    {0x0F12, 0x002D, WORD_LEN, 0},	// 003B 0034 Tune_wbt_GAS_178_ 
    {0x0F12, 0x0041, WORD_LEN, 0},	// 0046 0051 Tune_wbt_GAS_179_ 
    {0x0F12, 0x005D, WORD_LEN, 0},	// 0051 007C Tune_wbt_GAS_180_ 
    {0x0F12, 0x007C, WORD_LEN, 0},	// 007F 00B7 Tune_wbt_GAS_181_ 
    {0x0F12, 0x0077, WORD_LEN, 0},	// 0062 008A Tune_wbt_GAS_182_ 
    {0x0F12, 0x0057, WORD_LEN, 0},	// 004B 0061 Tune_wbt_GAS_183_ 
    {0x0F12, 0x0039, WORD_LEN, 0},	// 0034 003E Tune_wbt_GAS_184_ 
    {0x0F12, 0x0024, WORD_LEN, 0},	// 0022 0026 Tune_wbt_GAS_185_ 
    {0x0F12, 0x0014, WORD_LEN, 0},	// 0014 0013 Tune_wbt_GAS_186_ 
    {0x0F12, 0x000A, WORD_LEN, 0},	// 000E 0008 Tune_wbt_GAS_187_ 
    {0x0F12, 0x0007, WORD_LEN, 0},	// 000C 0004 Tune_wbt_GAS_188_ 
    {0x0F12, 0x0009, WORD_LEN, 0},	// 000D 0007 Tune_wbt_GAS_189_ 
    {0x0F12, 0x0012, WORD_LEN, 0},	// 0016 0013 Tune_wbt_GAS_190_ 
    {0x0F12, 0x0021, WORD_LEN, 0},	// 0024 0027 Tune_wbt_GAS_191_ 
    {0x0F12, 0x0036, WORD_LEN, 0},	// 0036 0045 Tune_wbt_GAS_192_ 
    {0x0F12, 0x0051, WORD_LEN, 0},	// 004E 0070 Tune_wbt_GAS_193_ 
    {0x0F12, 0x0070, WORD_LEN, 0},	// 0069 00A7 Tune_wbt_GAS_194_ 
    {0x0F12, 0x0077, WORD_LEN, 0},	// 005F 0085 Tune_wbt_GAS_195_ 
    {0x0F12, 0x0056, WORD_LEN, 0},	// 0048 005D Tune_wbt_GAS_196_ 
    {0x0F12, 0x0038, WORD_LEN, 0},	// 002F 003A Tune_wbt_GAS_197_ 
    {0x0F12, 0x0022, WORD_LEN, 0},	// 001C 0021 Tune_wbt_GAS_198_ 
    {0x0F12, 0x0013, WORD_LEN, 0},	// 0010 0010 Tune_wbt_GAS_199_ 
    {0x0F12, 0x0009, WORD_LEN, 0},	// 000B 0004 Tune_wbt_GAS_200_ 
    {0x0F12, 0x0005, WORD_LEN, 0},	// 0008 0000 Tune_wbt_GAS_201_ 
    {0x0F12, 0x0008, WORD_LEN, 0},	// 000B 0003 Tune_wbt_GAS_202_ 
    {0x0F12, 0x0011, WORD_LEN, 0},	// 0010 000E Tune_wbt_GAS_203_ 
    {0x0F12, 0x0020, WORD_LEN, 0},	// 001E 0021 Tune_wbt_GAS_204_ 
    {0x0F12, 0x0035, WORD_LEN, 0},	// 0031 003F Tune_wbt_GAS_205_ 
    {0x0F12, 0x0051, WORD_LEN, 0},	// 0049 006B Tune_wbt_GAS_206_ 
    {0x0F12, 0x0071, WORD_LEN, 0},	// 0066 00A1 Tune_wbt_GAS_207_ 
    {0x0F12, 0x006E, WORD_LEN, 0},	// 005B 0089 Tune_wbt_GAS_208_ 
    {0x0F12, 0x004E, WORD_LEN, 0},	// 0043 0060 Tune_wbt_GAS_209_ 
    {0x0F12, 0x0032, WORD_LEN, 0},	// 002B 003D Tune_wbt_GAS_210_ 
    {0x0F12, 0x001C, WORD_LEN, 0},	// 0019 0023 Tune_wbt_GAS_211_ 
    {0x0F12, 0x000D, WORD_LEN, 0},	// 000C 0012 Tune_wbt_GAS_212_ 
    {0x0F12, 0x0004, WORD_LEN, 0},	// 0007 0006 Tune_wbt_GAS_213_ 
    {0x0F12, 0x0000, WORD_LEN, 0},	// 0004 0002 Tune_wbt_GAS_214_ 
    {0x0F12, 0x0003, WORD_LEN, 0},	// 0007 0005 Tune_wbt_GAS_215_ 
    {0x0F12, 0x000B, WORD_LEN, 0},	// 000D 0011 Tune_wbt_GAS_216_ 
    {0x0F12, 0x001A, WORD_LEN, 0},	// 001B 0025 Tune_wbt_GAS_217_ 
    {0x0F12, 0x002F, WORD_LEN, 0},	// 002E 0043 Tune_wbt_GAS_218_ 
    {0x0F12, 0x0049, WORD_LEN, 0},	// 0046 0070 Tune_wbt_GAS_219_ 
    {0x0F12, 0x0068, WORD_LEN, 0},	// 0062 00A4 Tune_wbt_GAS_220_ 
    {0x0F12, 0x0072, WORD_LEN, 0},	// 0052 0091 Tune_wbt_GAS_221_ 
    {0x0F12, 0x0053, WORD_LEN, 0},	// 003D 0067 Tune_wbt_GAS_222_ 
    {0x0F12, 0x0037, WORD_LEN, 0},	// 0026 0044 Tune_wbt_GAS_223_ 
    {0x0F12, 0x0021, WORD_LEN, 0},	// 0014 002B Tune_wbt_GAS_224_ 
    {0x0F12, 0x0012, WORD_LEN, 0},	// 0007 0019 Tune_wbt_GAS_225_ 
    {0x0F12, 0x0009, WORD_LEN, 0},	// 0002 000D Tune_wbt_GAS_226_ 
    {0x0F12, 0x0005, WORD_LEN, 0},	// 0001 0009 Tune_wbt_GAS_227_ 
    {0x0F12, 0x0008, WORD_LEN, 0},	// 0002 000C Tune_wbt_GAS_228_ 
    {0x0F12, 0x0010, WORD_LEN, 0},	// 0007 0018 Tune_wbt_GAS_229_ 
    {0x0F12, 0x001F, WORD_LEN, 0},	// 0015 002D Tune_wbt_GAS_230_ 
    {0x0F12, 0x0034, WORD_LEN, 0},	// 0028 004B Tune_wbt_GAS_231_ 
    {0x0F12, 0x004E, WORD_LEN, 0},	// 0040 007B Tune_wbt_GAS_232_ 
    {0x0F12, 0x006C, WORD_LEN, 0},	// 005C 00B0 Tune_wbt_GAS_233_ 
    {0x0F12, 0x007F, WORD_LEN, 0},	// 005E 00A1 Tune_wbt_GAS_234_ 
    {0x0F12, 0x0060, WORD_LEN, 0},	// 0049 0077 Tune_wbt_GAS_235_ 
    {0x0F12, 0x0043, WORD_LEN, 0},	// 0030 0054 Tune_wbt_GAS_236_ 
    {0x0F12, 0x002D, WORD_LEN, 0},	// 001E 003B Tune_wbt_GAS_237_ 
    {0x0F12, 0x001D, WORD_LEN, 0},	// 0010 0029 Tune_wbt_GAS_238_ 
    {0x0F12, 0x0013, WORD_LEN, 0},	// 0009 001C Tune_wbt_GAS_239_ 
    {0x0F12, 0x0010, WORD_LEN, 0},	// 0007 0018 Tune_wbt_GAS_240_ 
    {0x0F12, 0x0013, WORD_LEN, 0},	// 0009 001B Tune_wbt_GAS_241_ 
    {0x0F12, 0x001C, WORD_LEN, 0},	// 0012 0029 Tune_wbt_GAS_242_ 
    {0x0F12, 0x002B, WORD_LEN, 0},	// 0020 003F Tune_wbt_GAS_243_ 
    {0x0F12, 0x0040, WORD_LEN, 0},	// 0032 005F Tune_wbt_GAS_244_ 
    {0x0F12, 0x005A, WORD_LEN, 0},	// 004B 008F Tune_wbt_GAS_245_ 
    {0x0F12, 0x0079, WORD_LEN, 0},	// 0069 00C6 Tune_wbt_GAS_246_ 
    {0x0F12, 0x0082, WORD_LEN, 0},	// 0066 00B1 Tune_wbt_GAS_247_ 
    {0x0F12, 0x0066, WORD_LEN, 0},	// 004E 008E Tune_wbt_GAS_248_ 
    {0x0F12, 0x0049, WORD_LEN, 0},	// 0037 006D Tune_wbt_GAS_249_ 
    {0x0F12, 0x0035, WORD_LEN, 0},	// 0026 0050 Tune_wbt_GAS_250_ 
    {0x0F12, 0x0025, WORD_LEN, 0},	// 0019 003D Tune_wbt_GAS_251_ 
    {0x0F12, 0x001B, WORD_LEN, 0},	// 0011 0031 Tune_wbt_GAS_252_ 
    {0x0F12, 0x0017, WORD_LEN, 0},	// 000F 002F Tune_wbt_GAS_253_ 
    {0x0F12, 0x0019, WORD_LEN, 0},	// 0012 0032 Tune_wbt_GAS_254_ 
    {0x0F12, 0x0023, WORD_LEN, 0},	// 001B 0042 Tune_wbt_GAS_255_ 
    {0x0F12, 0x0033, WORD_LEN, 0},	// 0028 0058 Tune_wbt_GAS_256_ 
    {0x0F12, 0x0046, WORD_LEN, 0},	// 003B 007A Tune_wbt_GAS_257_ 
    {0x0F12, 0x0060, WORD_LEN, 0},	// 0054 00AC Tune_wbt_GAS_258_ 
    {0x0F12, 0x007B, WORD_LEN, 0},	// 0072 00E5 Tune_wbt_GAS_259_ 
    {0x0F12, 0x0092, WORD_LEN, 0},	// 006A 00BD Tune_wbt_GAS_260_ 
    {0x0F12, 0x007C, WORD_LEN, 0},	// 0058 00AA Tune_wbt_GAS_261_ 
    {0x0F12, 0x0060, WORD_LEN, 0},	// 0041 008B Tune_wbt_GAS_262_ 
    {0x0F12, 0x004B, WORD_LEN, 0},	// 0030 006E Tune_wbt_GAS_263_ 
    {0x0F12, 0x003C, WORD_LEN, 0},	// 0025 005A Tune_wbt_GAS_264_ 
    {0x0F12, 0x0032, WORD_LEN, 0},	// 001E 004F Tune_wbt_GAS_265_ 
    {0x0F12, 0x002D, WORD_LEN, 0},	// 001B 004B Tune_wbt_GAS_266_ 
    {0x0F12, 0x0030, WORD_LEN, 0},	// 001F 0052 Tune_wbt_GAS_267_ 
    {0x0F12, 0x0039, WORD_LEN, 0},	// 0027 0062 Tune_wbt_GAS_268_ 
    {0x0F12, 0x0049, WORD_LEN, 0},	// 0034 007D Tune_wbt_GAS_269_ 
    {0x0F12, 0x005D, WORD_LEN, 0},	// 0046 00A2 Tune_wbt_GAS_270_ 
    {0x0F12, 0x0076, WORD_LEN, 0},	// 005D 00D6 Tune_wbt_GAS_271_ 
    {0x0F12, 0x008C, WORD_LEN, 0},	// 0078 010C Tune_wbt_GAS_272_ 
    {0x0F12, 0x009F, WORD_LEN, 0},	// 007C 00E5 Tune_wbt_GAS_273_ 
    {0x0F12, 0x008F, WORD_LEN, 0},	// 006A 00C7 Tune_wbt_GAS_274_ 
    {0x0F12, 0x0077, WORD_LEN, 0},	// 0055 00B1 Tune_wbt_GAS_275_ 
    {0x0F12, 0x0061, WORD_LEN, 0},	// 0043 0093 Tune_wbt_GAS_276_ 
    {0x0F12, 0x0052, WORD_LEN, 0},	// 0037 007F Tune_wbt_GAS_277_ 
    {0x0F12, 0x0048, WORD_LEN, 0},	// 0030 0074 Tune_wbt_GAS_278_ 
    {0x0F12, 0x0043, WORD_LEN, 0},	// 002E 0071 Tune_wbt_GAS_279_ 
    {0x0F12, 0x0047, WORD_LEN, 0},	// 0030 0077 Tune_wbt_GAS_280_ 
    {0x0F12, 0x0050, WORD_LEN, 0},	// 0039 0089 Tune_wbt_GAS_281_ 
    {0x0F12, 0x005E, WORD_LEN, 0},	// 0045 00A7 Tune_wbt_GAS_282_ 
    {0x0F12, 0x0071, WORD_LEN, 0},	// 0056 00CC Tune_wbt_GAS_283_ 
    {0x0F12, 0x0086, WORD_LEN, 0},	// 006C 00FE Tune_wbt_GAS_284_ 
    {0x0F12, 0x0097, WORD_LEN, 0},	// 0084 0132 Tune_wbt_GAS_285_ 
    {0x0F12, 0x0093, WORD_LEN, 0},	// 006E 00A8 Tune_wbt_GAS_286_ 
    {0x0F12, 0x007C, WORD_LEN, 0},	// 006D 008D Tune_wbt_GAS_287_ 
    {0x0F12, 0x005F, WORD_LEN, 0},	// 005B 006C Tune_wbt_GAS_288_ 
    {0x0F12, 0x0049, WORD_LEN, 0},	// 0046 004E Tune_wbt_GAS_289_ 
    {0x0F12, 0x003A, WORD_LEN, 0},	// 003A 003C Tune_wbt_GAS_290_ 
    {0x0F12, 0x0030, WORD_LEN, 0},	// 0033 0032 Tune_wbt_GAS_291_ 
    {0x0F12, 0x002C, WORD_LEN, 0},	// 002D 002D Tune_wbt_GAS_292_ 
    {0x0F12, 0x002F, WORD_LEN, 0},	// 0032 0032 Tune_wbt_GAS_293_ 
    {0x0F12, 0x0037, WORD_LEN, 0},	// 0039 0040 Tune_wbt_GAS_294_ 
    {0x0F12, 0x0045, WORD_LEN, 0},	// 0047 0056 Tune_wbt_GAS_295_ 
    {0x0F12, 0x005A, WORD_LEN, 0},	// 004F 0076 Tune_wbt_GAS_296_ 
    {0x0F12, 0x0075, WORD_LEN, 0},	// 0050 00A8 Tune_wbt_GAS_297_ 
    {0x0F12, 0x008A, WORD_LEN, 0},	// 006E 00E6 Tune_wbt_GAS_298_ 
    {0x0F12, 0x0094, WORD_LEN, 0},	// 0077 00A2 Tune_wbt_GAS_299_ 
    {0x0F12, 0x0077, WORD_LEN, 0},	// 006C 007C Tune_wbt_GAS_300_ 
    {0x0F12, 0x0057, WORD_LEN, 0},	// 0054 0059 Tune_wbt_GAS_301_ 
    {0x0F12, 0x0040, WORD_LEN, 0},	// 0040 003E Tune_wbt_GAS_302_ 
    {0x0F12, 0x002F, WORD_LEN, 0},	// 0033 002A Tune_wbt_GAS_303_ 
    {0x0F12, 0x0024, WORD_LEN, 0},	// 002B 0020 Tune_wbt_GAS_304_ 
    {0x0F12, 0x0020, WORD_LEN, 0},	// 0027 001B Tune_wbt_GAS_305_ 
    {0x0F12, 0x0023, WORD_LEN, 0},	// 002A 0020 Tune_wbt_GAS_306_ 
    {0x0F12, 0x002D, WORD_LEN, 0},	// 0034 002D Tune_wbt_GAS_307_ 
    {0x0F12, 0x003B, WORD_LEN, 0},	// 0041 0042 Tune_wbt_GAS_308_ 
    {0x0F12, 0x0051, WORD_LEN, 0},	// 004C 0061 Tune_wbt_GAS_309_ 
    {0x0F12, 0x006E, WORD_LEN, 0},	// 0052 0092 Tune_wbt_GAS_310_ 
    {0x0F12, 0x008C, WORD_LEN, 0},	// 007E 00CE Tune_wbt_GAS_311_ 
    {0x0F12, 0x0085, WORD_LEN, 0},	// 0078 0094 Tune_wbt_GAS_312_ 
    {0x0F12, 0x0066, WORD_LEN, 0},	// 0063 006F Tune_wbt_GAS_313_ 
    {0x0F12, 0x0046, WORD_LEN, 0},	// 0049 004B Tune_wbt_GAS_314_ 
    {0x0F12, 0x002F, WORD_LEN, 0},	// 0035 002F Tune_wbt_GAS_315_ 
    {0x0F12, 0x001F, WORD_LEN, 0},	// 0028 001D Tune_wbt_GAS_316_ 
    {0x0F12, 0x0014, WORD_LEN, 0},	// 001E 0011 Tune_wbt_GAS_317_ 
    {0x0F12, 0x000F, WORD_LEN, 0},	// 001B 000D Tune_wbt_GAS_318_ 
    {0x0F12, 0x0012, WORD_LEN, 0},	// 001F 0012 Tune_wbt_GAS_319_ 
    {0x0F12, 0x001C, WORD_LEN, 0},	// 0028 001D Tune_wbt_GAS_320_ 
    {0x0F12, 0x002B, WORD_LEN, 0},	// 0037 0033 Tune_wbt_GAS_321_ 
    {0x0F12, 0x0040, WORD_LEN, 0},	// 0044 0051 Tune_wbt_GAS_322_ 
    {0x0F12, 0x005C, WORD_LEN, 0},	// 0050 007F Tune_wbt_GAS_323_ 
    {0x0F12, 0x007D, WORD_LEN, 0},	// 0080 00BA Tune_wbt_GAS_324_ 
    {0x0F12, 0x007A, WORD_LEN, 0},	// 0064 008B Tune_wbt_GAS_325_ 
    {0x0F12, 0x005A, WORD_LEN, 0},	// 004E 0064 Tune_wbt_GAS_326_ 
    {0x0F12, 0x003A, WORD_LEN, 0},	// 0035 0040 Tune_wbt_GAS_327_ 
    {0x0F12, 0x0024, WORD_LEN, 0},	// 0021 0025 Tune_wbt_GAS_328_ 
    {0x0F12, 0x0014, WORD_LEN, 0},	// 0013 0013 Tune_wbt_GAS_329_ 
    {0x0F12, 0x0009, WORD_LEN, 0},	// 000D 0007 Tune_wbt_GAS_330_ 
    {0x0F12, 0x0006, WORD_LEN, 0},	// 000B 0003 Tune_wbt_GAS_331_ 
    {0x0F12, 0x0008, WORD_LEN, 0},	// 000C 0006 Tune_wbt_GAS_332_ 
    {0x0F12, 0x0011, WORD_LEN, 0},	// 0014 0012 Tune_wbt_GAS_333_ 
    {0x0F12, 0x0020, WORD_LEN, 0},	// 0022 0027 Tune_wbt_GAS_334_ 
    {0x0F12, 0x0036, WORD_LEN, 0},	// 0036 0046 Tune_wbt_GAS_335_ 
    {0x0F12, 0x0051, WORD_LEN, 0},	// 004D 0073 Tune_wbt_GAS_336_ 
    {0x0F12, 0x0072, WORD_LEN, 0},	// 006B 00AB Tune_wbt_GAS_337_ 
    {0x0F12, 0x007B, WORD_LEN, 0},	// 0066 008B Tune_wbt_GAS_338_ 
    {0x0F12, 0x0059, WORD_LEN, 0},	// 004C 0062 Tune_wbt_GAS_339_ 
    {0x0F12, 0x003A, WORD_LEN, 0},	// 0032 003F Tune_wbt_GAS_340_ 
    {0x0F12, 0x0023, WORD_LEN, 0},	// 001E 0023 Tune_wbt_GAS_341_ 
    {0x0F12, 0x0012, WORD_LEN, 0},	// 0010 0010 Tune_wbt_GAS_342_ 
    {0x0F12, 0x0008, WORD_LEN, 0},	// 000A 0004 Tune_wbt_GAS_343_ 
    {0x0F12, 0x0004, WORD_LEN, 0},	// 0007 0000 Tune_wbt_GAS_344_ 
    {0x0F12, 0x0007, WORD_LEN, 0},	// 000B 0003 Tune_wbt_GAS_345_ 
    {0x0F12, 0x000F, WORD_LEN, 0},	// 0010 000E Tune_wbt_GAS_346_ 
    {0x0F12, 0x001F, WORD_LEN, 0},	// 001E 0022 Tune_wbt_GAS_347_ 
    {0x0F12, 0x0035, WORD_LEN, 0},	// 0032 0041 Tune_wbt_GAS_348_ 
    {0x0F12, 0x0051, WORD_LEN, 0},	// 004B 006E Tune_wbt_GAS_349_ 
    {0x0F12, 0x0072, WORD_LEN, 0},	// 006B 00A4 Tune_wbt_GAS_350_ 
    {0x0F12, 0x0073, WORD_LEN, 0},	// 0061 008E Tune_wbt_GAS_351_ 
    {0x0F12, 0x0053, WORD_LEN, 0},	// 0049 0065 Tune_wbt_GAS_352_ 
    {0x0F12, 0x0034, WORD_LEN, 0},	// 002F 0040 Tune_wbt_GAS_353_ 
    {0x0F12, 0x001D, WORD_LEN, 0},	// 001B 0025 Tune_wbt_GAS_354_ 
    {0x0F12, 0x000E, WORD_LEN, 0},	// 000E 0013 Tune_wbt_GAS_355_ 
    {0x0F12, 0x0004, WORD_LEN, 0},	// 0008 0006 Tune_wbt_GAS_356_ 
    {0x0F12, 0x0000, WORD_LEN, 0},	// 0004 0001 Tune_wbt_GAS_357_ 
    {0x0F12, 0x0002, WORD_LEN, 0},	// 0008 0005 Tune_wbt_GAS_358_ 
    {0x0F12, 0x000A, WORD_LEN, 0},	// 000D 0010 Tune_wbt_GAS_359_ 
    {0x0F12, 0x001A, WORD_LEN, 0},	// 001C 0025 Tune_wbt_GAS_360_ 
    {0x0F12, 0x002F, WORD_LEN, 0},	// 002F 0044 Tune_wbt_GAS_361_ 
    {0x0F12, 0x004A, WORD_LEN, 0},	// 0047 0074 Tune_wbt_GAS_362_ 
    {0x0F12, 0x006A, WORD_LEN, 0},	// 0067 00AA Tune_wbt_GAS_363_ 
    {0x0F12, 0x0077, WORD_LEN, 0},	// 005A 0097 Tune_wbt_GAS_364_ 
    {0x0F12, 0x0058, WORD_LEN, 0},	// 0043 006D Tune_wbt_GAS_365_ 
    {0x0F12, 0x0039, WORD_LEN, 0},	// 002B 0048 Tune_wbt_GAS_366_ 
    {0x0F12, 0x0022, WORD_LEN, 0},	// 0017 002D Tune_wbt_GAS_367_ 
    {0x0F12, 0x0012, WORD_LEN, 0},	// 0009 0019 Tune_wbt_GAS_368_ 
    {0x0F12, 0x0008, WORD_LEN, 0},	// 0004 000E Tune_wbt_GAS_369_ 
    {0x0F12, 0x0004, WORD_LEN, 0},	// 0002 0009 Tune_wbt_GAS_370_ 
    {0x0F12, 0x0007, WORD_LEN, 0},	// 0004 000C Tune_wbt_GAS_371_ 
    {0x0F12, 0x000F, WORD_LEN, 0},	// 0008 0018 Tune_wbt_GAS_372_ 
    {0x0F12, 0x001E, WORD_LEN, 0},	// 0016 002E Tune_wbt_GAS_373_ 
    {0x0F12, 0x0034, WORD_LEN, 0},	// 002A 004E Tune_wbt_GAS_374_ 
    {0x0F12, 0x004F, WORD_LEN, 0},	// 0042 007D Tune_wbt_GAS_375_ 
    {0x0F12, 0x006F, WORD_LEN, 0},	// 005F 00B5 Tune_wbt_GAS_376_ 
    {0x0F12, 0x0083, WORD_LEN, 0},	// 0066 00A6 Tune_wbt_GAS_377_ 
    {0x0F12, 0x0064, WORD_LEN, 0},	// 0050 007C Tune_wbt_GAS_378_ 
    {0x0F12, 0x0045, WORD_LEN, 0},	// 0035 0058 Tune_wbt_GAS_379_ 
    {0x0F12, 0x002E, WORD_LEN, 0},	// 0022 003E Tune_wbt_GAS_380_ 
    {0x0F12, 0x001D, WORD_LEN, 0},	// 0013 0029 Tune_wbt_GAS_381_ 
    {0x0F12, 0x0012, WORD_LEN, 0},	// 000A 001D Tune_wbt_GAS_382_ 
    {0x0F12, 0x000F, WORD_LEN, 0},	// 0008 0018 Tune_wbt_GAS_383_ 
    {0x0F12, 0x0011, WORD_LEN, 0},	// 000B 001B Tune_wbt_GAS_384_ 
    {0x0F12, 0x001A, WORD_LEN, 0},	// 0014 0028 Tune_wbt_GAS_385_ 
    {0x0F12, 0x002A, WORD_LEN, 0},	// 0021 003F Tune_wbt_GAS_386_ 
    {0x0F12, 0x003F, WORD_LEN, 0},	// 0035 0062 Tune_wbt_GAS_387_ 
    {0x0F12, 0x005B, WORD_LEN, 0},	// 004D 0093 Tune_wbt_GAS_388_ 
    {0x0F12, 0x007B, WORD_LEN, 0},	// 006E 00CC Tune_wbt_GAS_389_ 
    {0x0F12, 0x0087, WORD_LEN, 0},	// 006E 00B9 Tune_wbt_GAS_390_ 
    {0x0F12, 0x006A, WORD_LEN, 0},	// 0057 0094 Tune_wbt_GAS_391_ 
    {0x0F12, 0x004B, WORD_LEN, 0},	// 003E 0071 Tune_wbt_GAS_392_ 
    {0x0F12, 0x0036, WORD_LEN, 0},	// 002B 0052 Tune_wbt_GAS_393_ 
    {0x0F12, 0x0025, WORD_LEN, 0},	// 001C 003D Tune_wbt_GAS_394_ 
    {0x0F12, 0x0019, WORD_LEN, 0},	// 0013 0031 Tune_wbt_GAS_395_ 
    {0x0F12, 0x0015, WORD_LEN, 0},	// 0011 002D Tune_wbt_GAS_396_ 
    {0x0F12, 0x0017, WORD_LEN, 0},	// 0013 0031 Tune_wbt_GAS_397_ 
    {0x0F12, 0x0022, WORD_LEN, 0},	// 001D 0040 Tune_wbt_GAS_398_ 
    {0x0F12, 0x0031, WORD_LEN, 0},	// 002B 0058 Tune_wbt_GAS_399_ 
    {0x0F12, 0x0045, WORD_LEN, 0},	// 003D 007B Tune_wbt_GAS_400_ 
    {0x0F12, 0x0060, WORD_LEN, 0},	// 0057 00AE Tune_wbt_GAS_401_ 
    {0x0F12, 0x007D, WORD_LEN, 0},	// 0077 00EA Tune_wbt_GAS_402_ 
    {0x0F12, 0x0096, WORD_LEN, 0},	// 0072 00C2 Tune_wbt_GAS_403_ 
    {0x0F12, 0x007F, WORD_LEN, 0},	// 005F 00AE Tune_wbt_GAS_404_ 
    {0x0F12, 0x0061, WORD_LEN, 0},	// 0047 008E Tune_wbt_GAS_405_ 
    {0x0F12, 0x004B, WORD_LEN, 0},	// 0035 006F Tune_wbt_GAS_406_ 
    {0x0F12, 0x003B, WORD_LEN, 0},	// 0028 0059 Tune_wbt_GAS_407_ 
    {0x0F12, 0x002F, WORD_LEN, 0},	// 0020 004E Tune_wbt_GAS_408_ 
    {0x0F12, 0x002A, WORD_LEN, 0},	// 001D 0049 Tune_wbt_GAS_409_ 
    {0x0F12, 0x002D, WORD_LEN, 0},	// 0020 004F Tune_wbt_GAS_410_ 
    {0x0F12, 0x0036, WORD_LEN, 0},	// 0029 005E Tune_wbt_GAS_411_ 
    {0x0F12, 0x0046, WORD_LEN, 0},	// 0035 007A Tune_wbt_GAS_412_ 
    {0x0F12, 0x005B, WORD_LEN, 0},	// 0047 009F Tune_wbt_GAS_413_ 
    {0x0F12, 0x0075, WORD_LEN, 0},	// 0060 00D5 Tune_wbt_GAS_414_ 
    {0x0F12, 0x008D, WORD_LEN, 0},	// 007D 010C Tune_wbt_GAS_415_ 
    {0x0F12, 0x00A1, WORD_LEN, 0},	// 0084 00E5 Tune_wbt_GAS_416_ 
    {0x0F12, 0x0091, WORD_LEN, 0},	// 0072 00C8 Tune_wbt_GAS_417_ 
    {0x0F12, 0x0077, WORD_LEN, 0},	// 005B 00B0 Tune_wbt_GAS_418_ 
    {0x0F12, 0x0060, WORD_LEN, 0},	// 0046 0091 Tune_wbt_GAS_419_ 
    {0x0F12, 0x0050, WORD_LEN, 0},	// 003A 007D Tune_wbt_GAS_420_ 
    {0x0F12, 0x0044, WORD_LEN, 0},	// 0031 0070 Tune_wbt_GAS_421_ 
    {0x0F12, 0x0040, WORD_LEN, 0},	// 002E 006D Tune_wbt_GAS_422_ 
    {0x0F12, 0x0043, WORD_LEN, 0},	// 0032 0074 Tune_wbt_GAS_423_ 
    {0x0F12, 0x004C, WORD_LEN, 0},	// 0039 0086 Tune_wbt_GAS_424_ 
    {0x0F12, 0x005A, WORD_LEN, 0},	// 0046 00A4 Tune_wbt_GAS_425_ 
    {0x0F12, 0x006D, WORD_LEN, 0},	// 0056 00CA Tune_wbt_GAS_426_ 
    {0x0F12, 0x0084, WORD_LEN, 0},	// 006E 00FE Tune_wbt_GAS_427_ 
    {0x0F12, 0x0094, WORD_LEN, 0},	// 0087 0134 Tune_wbt_GAS_428_ 
    {0x0F12, 0x0072, WORD_LEN, 0},	// 004C 009F Tune_wbt_GAS_429_ 
    {0x0F12, 0x0063, WORD_LEN, 0},	// 004C 0089 Tune_wbt_GAS_430_ 
    {0x0F12, 0x004C, WORD_LEN, 0},	// 0041 0067 Tune_wbt_GAS_431_ 
    {0x0F12, 0x003A, WORD_LEN, 0},	// 002F 004E Tune_wbt_GAS_432_ 
    {0x0F12, 0x002D, WORD_LEN, 0},	// 0024 003E Tune_wbt_GAS_433_ 
    {0x0F12, 0x0025, WORD_LEN, 0},	// 001D 0033 Tune_wbt_GAS_434_ 
    {0x0F12, 0x0023, WORD_LEN, 0},	// 001B 0031 Tune_wbt_GAS_435_ 
    {0x0F12, 0x0025, WORD_LEN, 0},	// 001E 0036 Tune_wbt_GAS_436_ 
    {0x0F12, 0x002C, WORD_LEN, 0},	// 0024 0045 Tune_wbt_GAS_437_ 
    {0x0F12, 0x0038, WORD_LEN, 0},	// 0032 0058 Tune_wbt_GAS_438_ 
    {0x0F12, 0x004A, WORD_LEN, 0},	// 0037 0074 Tune_wbt_GAS_439_ 
    {0x0F12, 0x005F, WORD_LEN, 0},	// 0038 00A0 Tune_wbt_GAS_440_ 
    {0x0F12, 0x006B, WORD_LEN, 0},	// 004C 00C9 Tune_wbt_GAS_441_ 
    {0x0F12, 0x0079, WORD_LEN, 0},	// 005B 0098 Tune_wbt_GAS_442_ 
    {0x0F12, 0x0065, WORD_LEN, 0},	// 0056 0077 Tune_wbt_GAS_443_ 
    {0x0F12, 0x004A, WORD_LEN, 0},	// 0041 0055 Tune_wbt_GAS_444_ 
    {0x0F12, 0x0037, WORD_LEN, 0},	// 0030 003C Tune_wbt_GAS_445_ 
    {0x0F12, 0x0029, WORD_LEN, 0},	// 0026 002A Tune_wbt_GAS_446_ 
    {0x0F12, 0x0021, WORD_LEN, 0},	// 001F 0022 Tune_wbt_GAS_447_ 
    {0x0F12, 0x001D, WORD_LEN, 0},	// 001C 001F Tune_wbt_GAS_448_ 
    {0x0F12, 0x001F, WORD_LEN, 0},	// 001F 0024 Tune_wbt_GAS_449_ 
    {0x0F12, 0x0027, WORD_LEN, 0},	// 0027 0030 Tune_wbt_GAS_450_ 
    {0x0F12, 0x0033, WORD_LEN, 0},	// 0033 0044 Tune_wbt_GAS_451_ 
    {0x0F12, 0x0044, WORD_LEN, 0},	// 003C 0060 Tune_wbt_GAS_452_ 
    {0x0F12, 0x005E, WORD_LEN, 0},	// 0041 008B Tune_wbt_GAS_453_ 
    {0x0F12, 0x006E, WORD_LEN, 0},	// 0060 00B2 Tune_wbt_GAS_454_ 
    {0x0F12, 0x006A, WORD_LEN, 0},	// 005E 0088 Tune_wbt_GAS_455_ 
    {0x0F12, 0x0055, WORD_LEN, 0},	// 004F 0065 Tune_wbt_GAS_456_ 
    {0x0F12, 0x003A, WORD_LEN, 0},	// 003A 0044 Tune_wbt_GAS_457_ 
    {0x0F12, 0x0028, WORD_LEN, 0},	// 002A 002C Tune_wbt_GAS_458_ 
    {0x0F12, 0x001A, WORD_LEN, 0},	// 001D 001B Tune_wbt_GAS_459_ 
    {0x0F12, 0x0011, WORD_LEN, 0},	// 0016 0012 Tune_wbt_GAS_460_ 
    {0x0F12, 0x000D, WORD_LEN, 0},	// 0014 000F Tune_wbt_GAS_461_ 
    {0x0F12, 0x000F, WORD_LEN, 0},	// 0016 0013 Tune_wbt_GAS_462_ 
    {0x0F12, 0x0017, WORD_LEN, 0},	// 0021 001E Tune_wbt_GAS_463_ 
    {0x0F12, 0x0024, WORD_LEN, 0},	// 002D 0032 Tune_wbt_GAS_464_ 
    {0x0F12, 0x0035, WORD_LEN, 0},	// 0038 004E Tune_wbt_GAS_465_ 
    {0x0F12, 0x004E, WORD_LEN, 0},	// 0040 0078 Tune_wbt_GAS_466_ 
    {0x0F12, 0x0061, WORD_LEN, 0},	// 0066 00A2 Tune_wbt_GAS_467_ 
    {0x0F12, 0x0061, WORD_LEN, 0},	// 004A 007F Tune_wbt_GAS_468_ 
    {0x0F12, 0x004A, WORD_LEN, 0},	// 003C 005B Tune_wbt_GAS_469_ 
    {0x0F12, 0x0031, WORD_LEN, 0},	// 0028 0039 Tune_wbt_GAS_470_ 
    {0x0F12, 0x001E, WORD_LEN, 0},	// 0019 0021 Tune_wbt_GAS_471_ 
    {0x0F12, 0x0011, WORD_LEN, 0},	// 000D 0012 Tune_wbt_GAS_472_ 
    {0x0F12, 0x0008, WORD_LEN, 0},	// 0008 0007 Tune_wbt_GAS_473_ 
    {0x0F12, 0x0005, WORD_LEN, 0},	// 0007 0004 Tune_wbt_GAS_474_ 
    {0x0F12, 0x0007, WORD_LEN, 0},	// 0007 0006 Tune_wbt_GAS_475_ 
    {0x0F12, 0x000E, WORD_LEN, 0},	// 000F 0012 Tune_wbt_GAS_476_ 
    {0x0F12, 0x001B, WORD_LEN, 0},	// 001C 0024 Tune_wbt_GAS_477_ 
    {0x0F12, 0x002D, WORD_LEN, 0},	// 002C 0041 Tune_wbt_GAS_478_ 
    {0x0F12, 0x0045, WORD_LEN, 0},	// 0042 006A Tune_wbt_GAS_479_ 
    {0x0F12, 0x0059, WORD_LEN, 0},	// 0054 0092 Tune_wbt_GAS_480_ 
    {0x0F12, 0x0062, WORD_LEN, 0},	// 004B 007B Tune_wbt_GAS_481_ 
    {0x0F12, 0x004B, WORD_LEN, 0},	// 003C 0057 Tune_wbt_GAS_482_ 
    {0x0F12, 0x0031, WORD_LEN, 0},	// 0027 0036 Tune_wbt_GAS_483_ 
    {0x0F12, 0x001E, WORD_LEN, 0},	// 0017 001E Tune_wbt_GAS_484_ 
    {0x0F12, 0x0010, WORD_LEN, 0},	// 000C 000F Tune_wbt_GAS_485_ 
    {0x0F12, 0x0008, WORD_LEN, 0},	// 0007 0003 Tune_wbt_GAS_486_ 
    {0x0F12, 0x0004, WORD_LEN, 0},	// 0006 0000 Tune_wbt_GAS_487_ 
    {0x0F12, 0x0006, WORD_LEN, 0},	// 0008 0002 Tune_wbt_GAS_488_ 
    {0x0F12, 0x000E, WORD_LEN, 0},	// 000D 000D Tune_wbt_GAS_489_ 
    {0x0F12, 0x001B, WORD_LEN, 0},	// 001A 0020 Tune_wbt_GAS_490_ 
    {0x0F12, 0x002E, WORD_LEN, 0},	// 002B 003C Tune_wbt_GAS_491_ 
    {0x0F12, 0x0046, WORD_LEN, 0},	// 0041 0067 Tune_wbt_GAS_492_ 
    {0x0F12, 0x005A, WORD_LEN, 0},	// 0054 008D Tune_wbt_GAS_493_ 
    {0x0F12, 0x005B, WORD_LEN, 0},	// 0049 007E Tune_wbt_GAS_494_ 
    {0x0F12, 0x0045, WORD_LEN, 0},	// 003A 005A Tune_wbt_GAS_495_ 
    {0x0F12, 0x002C, WORD_LEN, 0},	// 0025 0038 Tune_wbt_GAS_496_ 
    {0x0F12, 0x001A, WORD_LEN, 0},	// 0015 0022 Tune_wbt_GAS_497_ 
    {0x0F12, 0x000C, WORD_LEN, 0},	// 000A 0013 Tune_wbt_GAS_498_ 
    {0x0F12, 0x0003, WORD_LEN, 0},	// 0005 0006 Tune_wbt_GAS_499_ 
    {0x0F12, 0x0000, WORD_LEN, 0},	// 0003 0001 Tune_wbt_GAS_500_ 
    {0x0F12, 0x0002, WORD_LEN, 0},	// 0006 0004 Tune_wbt_GAS_501_ 
    {0x0F12, 0x0009, WORD_LEN, 0},	// 000B 000F Tune_wbt_GAS_502_ 
    {0x0F12, 0x0016, WORD_LEN, 0},	// 0018 0023 Tune_wbt_GAS_503_ 
    {0x0F12, 0x0029, WORD_LEN, 0},	// 0029 003E Tune_wbt_GAS_504_ 
    {0x0F12, 0x0040, WORD_LEN, 0},	// 003E 006A Tune_wbt_GAS_505_ 
    {0x0F12, 0x0054, WORD_LEN, 0},	// 0050 0091 Tune_wbt_GAS_506_ 
    {0x0F12, 0x005F, WORD_LEN, 0},	// 0044 0085 Tune_wbt_GAS_507_ 
    {0x0F12, 0x004A, WORD_LEN, 0},	// 0033 0060 Tune_wbt_GAS_508_ 
    {0x0F12, 0x0031, WORD_LEN, 0},	// 0021 0041 Tune_wbt_GAS_509_ 
    {0x0F12, 0x001F, WORD_LEN, 0},	// 0011 002A Tune_wbt_GAS_510_ 
    {0x0F12, 0x0010, WORD_LEN, 0},	// 0005 0019 Tune_wbt_GAS_511_ 
    {0x0F12, 0x0008, WORD_LEN, 0},	// 0002 000D Tune_wbt_GAS_512_ 
    {0x0F12, 0x0004, WORD_LEN, 0},	// 0001 0008 Tune_wbt_GAS_513_ 
    {0x0F12, 0x0007, WORD_LEN, 0},	// 0003 000A Tune_wbt_GAS_514_ 
    {0x0F12, 0x000E, WORD_LEN, 0},	// 0008 0016 Tune_wbt_GAS_515_ 
    {0x0F12, 0x001B, WORD_LEN, 0},	// 0014 002A Tune_wbt_GAS_516_ 
    {0x0F12, 0x002E, WORD_LEN, 0},	// 0025 0047 Tune_wbt_GAS_517_ 
    {0x0F12, 0x0045, WORD_LEN, 0},	// 0039 0074 Tune_wbt_GAS_518_ 
    {0x0F12, 0x0059, WORD_LEN, 0},	// 004D 009A Tune_wbt_GAS_519_ 
    {0x0F12, 0x006C, WORD_LEN, 0},	// 0050 0097 Tune_wbt_GAS_520_ 
    {0x0F12, 0x0057, WORD_LEN, 0},	// 0041 0070 Tune_wbt_GAS_521_ 
    {0x0F12, 0x003E, WORD_LEN, 0},	// 002C 0052 Tune_wbt_GAS_522_ 
    {0x0F12, 0x002A, WORD_LEN, 0},	// 001C 003C Tune_wbt_GAS_523_ 
    {0x0F12, 0x001B, WORD_LEN, 0},	// 0011 0028 Tune_wbt_GAS_524_ 
    {0x0F12, 0x0012, WORD_LEN, 0},	// 0009 001D Tune_wbt_GAS_525_ 
    {0x0F12, 0x000F, WORD_LEN, 0},	// 0008 0019 Tune_wbt_GAS_526_ 
    {0x0F12, 0x0011, WORD_LEN, 0},	// 000A 001A Tune_wbt_GAS_527_ 
    {0x0F12, 0x0019, WORD_LEN, 0},	// 0012 0026 Tune_wbt_GAS_528_ 
    {0x0F12, 0x0027, WORD_LEN, 0},	// 001F 003B Tune_wbt_GAS_529_ 
    {0x0F12, 0x0039, WORD_LEN, 0},	// 002F 005A Tune_wbt_GAS_530_ 
    {0x0F12, 0x0050, WORD_LEN, 0},	// 0045 0089 Tune_wbt_GAS_531_ 
    {0x0F12, 0x0063, WORD_LEN, 0},	// 005A 00AF Tune_wbt_GAS_532_ 
    {0x0F12, 0x006F, WORD_LEN, 0},	// 0056 00A7 Tune_wbt_GAS_533_ 
    {0x0F12, 0x005C, WORD_LEN, 0},	// 0048 0088 Tune_wbt_GAS_534_ 
    {0x0F12, 0x0044, WORD_LEN, 0},	// 0035 006B Tune_wbt_GAS_535_ 
    {0x0F12, 0x0031, WORD_LEN, 0},	// 0026 0050 Tune_wbt_GAS_536_ 
    {0x0F12, 0x0023, WORD_LEN, 0},	// 0019 003D Tune_wbt_GAS_537_ 
    {0x0F12, 0x0019, WORD_LEN, 0},	// 0012 0032 Tune_wbt_GAS_538_ 
    {0x0F12, 0x0016, WORD_LEN, 0},	// 0011 002E Tune_wbt_GAS_539_ 
    {0x0F12, 0x0017, WORD_LEN, 0},	// 0012 0030 Tune_wbt_GAS_540_ 
    {0x0F12, 0x0020, WORD_LEN, 0},	// 001B 003E Tune_wbt_GAS_541_ 
    {0x0F12, 0x002E, WORD_LEN, 0},	// 0027 0052 Tune_wbt_GAS_542_ 
    {0x0F12, 0x0040, WORD_LEN, 0},	// 0037 0073 Tune_wbt_GAS_543_ 
    {0x0F12, 0x0055, WORD_LEN, 0},	// 004C 00A1 Tune_wbt_GAS_544_ 
    {0x0F12, 0x0064, WORD_LEN, 0},	// 0060 00C7 Tune_wbt_GAS_545_ 
    {0x0F12, 0x007E, WORD_LEN, 0},	// 0058 00AE Tune_wbt_GAS_546_ 
    {0x0F12, 0x0071, WORD_LEN, 0},	// 0050 00A4 Tune_wbt_GAS_547_ 
    {0x0F12, 0x0059, WORD_LEN, 0},	// 003D 0088 Tune_wbt_GAS_548_ 
    {0x0F12, 0x0046, WORD_LEN, 0},	// 002E 006D Tune_wbt_GAS_549_ 
    {0x0F12, 0x0039, WORD_LEN, 0},	// 0023 0059 Tune_wbt_GAS_550_ 
    {0x0F12, 0x002F, WORD_LEN, 0},	// 001C 004D Tune_wbt_GAS_551_ 
    {0x0F12, 0x002A, WORD_LEN, 0},	// 001B 0049 Tune_wbt_GAS_552_ 
    {0x0F12, 0x002D, WORD_LEN, 0},	// 001D 004E Tune_wbt_GAS_553_ 
    {0x0F12, 0x0035, WORD_LEN, 0},	// 0024 005B Tune_wbt_GAS_554_ 
    {0x0F12, 0x0043, WORD_LEN, 0},	// 002F 0073 Tune_wbt_GAS_555_ 
    {0x0F12, 0x0054, WORD_LEN, 0},	// 003E 0095 Tune_wbt_GAS_556_ 
    {0x0F12, 0x0069, WORD_LEN, 0},	// 0052 00C4 Tune_wbt_GAS_557_ 
    {0x0F12, 0x0074, WORD_LEN, 0},	// 0062 00E9 Tune_wbt_GAS_558_ 
    {0x0F12, 0x0083, WORD_LEN, 0},	// 0060 00D5 Tune_wbt_GAS_559_ 
    {0x0F12, 0x007D, WORD_LEN, 0},	// 0057 00C1 Tune_wbt_GAS_560_ 
    {0x0F12, 0x0068, WORD_LEN, 0},	// 0048 00AB Tune_wbt_GAS_561_ 
    {0x0F12, 0x0055, WORD_LEN, 0},	// 0039 008D Tune_wbt_GAS_562_ 
    {0x0F12, 0x0048, WORD_LEN, 0},	// 002E 0079 Tune_wbt_GAS_563_ 
    {0x0F12, 0x003E, WORD_LEN, 0},	// 0028 0070 Tune_wbt_GAS_564_ 
    {0x0F12, 0x003A, WORD_LEN, 0},	// 0027 006A Tune_wbt_GAS_565_ 
    {0x0F12, 0x003D, WORD_LEN, 0},	// 0029 006F Tune_wbt_GAS_566_ 
    {0x0F12, 0x0045, WORD_LEN, 0},	// 002F 0080 Tune_wbt_GAS_567_ 
    {0x0F12, 0x0051, WORD_LEN, 0},	// 0039 0096 Tune_wbt_GAS_568_ 
    {0x0F12, 0x0061, WORD_LEN, 0},	// 0047 00B9 Tune_wbt_GAS_569_ 
    {0x0F12, 0x0072, WORD_LEN, 0},	// 0059 00E2 Tune_wbt_GAS_570_ 
    {0x0F12, 0x0077, WORD_LEN, 0},	// 0067 010F Tune_wbt_GAS_571_ 
    {0x002A, 0x1348, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},	// gisp_gras_Enable 

//==================================================================================
// 07. Analog Setting 2 
//==================================================================================   
//This register is for FACTORY ONLY. 
//If you change it without prior notification
//YOU are RESPONSIBLE for the FAILURE that will happen in the future      
    {0x002A, 0x1278, WORD_LEN, 0},
    {0x0F12, 0xAAF0, WORD_LEN, 0},	//gisp_dadlc_config //Ladlc mode average
    {0x002A, 0x3370, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},	//afit_bUseNormBrForAfit //0:Noise Index, 1:Normal Brightness

//*************************************/
// 08.AF Setting                      */
//*************************************/

//*************************************/
// 09.AWB-BASIC setting               */
//*************************************/

// For WB Calibration */
    {0x002A, 0x0B36, WORD_LEN, 0},
    {0x0F12, 0x0005, WORD_LEN, 0},// awbb_IndoorGrZones_ZInfo_m_GridStep */
    {0x002A, 0x0B3A, WORD_LEN, 0},
    {0x0F12, 0x00F3, WORD_LEN, 0},// awbb_IndoorGrZones_ZInfo_m_BMin */
    {0x0F12, 0x02CB, WORD_LEN, 0},// awbb_IndoorGrZones_ZInfo_m_BMax */
    {0x002A, 0x0B38, WORD_LEN, 0},
    {0x0F12, 0x0010, WORD_LEN, 0},// awbb_IndoorGrZones_ZInfo_m_GridSz */
    {0x002A, 0x0AE6, WORD_LEN, 0},
    {0x0F12, 0x0385, WORD_LEN, 0},// 0352 03E1 awbb_IndoorGrZones_m_BGrid_0__m_left   */
    {0x0F12, 0x03D8, WORD_LEN, 0},// 038C 0413 awbb_IndoorGrZones_m_BGrid_0__m_right  */
    {0x0F12, 0x032A, WORD_LEN, 0},// 0321 039E awbb_IndoorGrZones_m_BGrid_1__m_left   */
    {0x0F12, 0x03C5, WORD_LEN, 0},// 03A6 0416 awbb_IndoorGrZones_m_BGrid_1__m_right  */
    {0x0F12, 0x02F5, WORD_LEN, 0},// 02EC 0367 awbb_IndoorGrZones_m_BGrid_2__m_left   */
    {0x0F12, 0x039D, WORD_LEN, 0},// 03A0 03F3 awbb_IndoorGrZones_m_BGrid_2__m_right  */
    {0x0F12, 0x02D3, WORD_LEN, 0},// 02CA 032D awbb_IndoorGrZones_m_BGrid_3__m_left   */
    {0x0F12, 0x0372, WORD_LEN, 0},// 038D 03C5 awbb_IndoorGrZones_m_BGrid_3__m_right  */
    {0x0F12, 0x02B1, WORD_LEN, 0},// 02A8 02FD awbb_IndoorGrZones_m_BGrid_4__m_left   */
    {0x0F12, 0x033E, WORD_LEN, 0},// 036E 038F awbb_IndoorGrZones_m_BGrid_4__m_right  */
    {0x0F12, 0x028A, WORD_LEN, 0},// 0281 02D3 awbb_IndoorGrZones_m_BGrid_5__m_left   */
    {0x0F12, 0x0322, WORD_LEN, 0},// 0344 0365 awbb_IndoorGrZones_m_BGrid_5__m_right  */
    {0x0F12, 0x0268, WORD_LEN, 0},// 025F 02AA awbb_IndoorGrZones_m_BGrid_6__m_left   */
    {0x0F12, 0x02FD, WORD_LEN, 0},// 0327 033E awbb_IndoorGrZones_m_BGrid_6__m_right  */
    {0x0F12, 0x0248, WORD_LEN, 0},// 023F 028D awbb_IndoorGrZones_m_BGrid_7__m_left   */
    {0x0F12, 0x02EF, WORD_LEN, 0},// 0302 0310 awbb_IndoorGrZones_m_BGrid_7__m_right  */
    {0x0F12, 0x022F, WORD_LEN, 0},// 0226 0271 awbb_IndoorGrZones_m_BGrid_8__m_left   */
    {0x0F12, 0x02D5, WORD_LEN, 0},// 02DC 02F1 awbb_IndoorGrZones_m_BGrid_8__m_right  */
    {0x0F12, 0x0219, WORD_LEN, 0},// 0210 025A awbb_IndoorGrZones_m_BGrid_9__m_left   */
    {0x0F12, 0x02C2, WORD_LEN, 0},// 02B9 02D2 awbb_IndoorGrZones_m_BGrid_9__m_right  */
    {0x0F12, 0x0206, WORD_LEN, 0},// 01FD 0249 awbb_IndoorGrZones_m_BGrid_10__m_left  */
    {0x0F12, 0x02A3, WORD_LEN, 0},// 029A 02B9 awbb_IndoorGrZones_m_BGrid_10__m_right */
    {0x0F12, 0x01F0, WORD_LEN, 0},// 01E7 0238 awbb_IndoorGrZones_m_BGrid_11__m_left  */
    {0x0F12, 0x0286, WORD_LEN, 0},// 027D 02A2 awbb_IndoorGrZones_m_BGrid_11__m_right */
    {0x0F12, 0x01E3, WORD_LEN, 0},// 01DA 021B awbb_IndoorGrZones_m_BGrid_12__m_left  */
    {0x0F12, 0x0268, WORD_LEN, 0},// 025F 0289 awbb_IndoorGrZones_m_BGrid_12__m_right */
    {0x0F12, 0x01D6, WORD_LEN, 0},// 01CD 0200 awbb_IndoorGrZones_m_BGrid_13__m_left  */
    {0x0F12, 0x024E, WORD_LEN, 0},// 0245 026C awbb_IndoorGrZones_m_BGrid_13__m_right */
    {0x0F12, 0x01DD, WORD_LEN, 0},// 01D4 01FC awbb_IndoorGrZones_m_BGrid_14__m_left  */
    {0x0F12, 0x022A, WORD_LEN, 0},// 0221 024F awbb_IndoorGrZones_m_BGrid_14__m_right */
    {0x0F12, 0x0210, WORD_LEN, 0},// 0207 021E awbb_IndoorGrZones_m_BGrid_15__m_left  */
    {0x0F12, 0x01F2, WORD_LEN, 0},// 01E9 022C awbb_IndoorGrZones_m_BGrid_15__m_right */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 0000 awbb_IndoorGrZones_m_BGrid_16__m_left  */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 0000 awbb_IndoorGrZones_m_BGrid_16__m_right */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 0000 awbb_IndoorGrZones_m_BGrid_17__m_left  */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 0000 awbb_IndoorGrZones_m_BGrid_17__m_right */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 0000 awbb_IndoorGrZones_m_BGrid_18__m_left  */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 0000 awbb_IndoorGrZones_m_BGrid_18__m_right */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 0000 awbb_IndoorGrZones_m_BGrid_19__m_left  */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 0000 awbb_IndoorGrZones_m_BGrid_19__m_right */

    {0x002A, 0x0BAA, WORD_LEN, 0},
    {0x0F12, 0x0006, WORD_LEN, 0},// awbb_LowBrGrZones_ZInfo_m_GridStep */
    {0x002A, 0x0BAE, WORD_LEN, 0},
    {0x0F12, 0x00CC, WORD_LEN, 0},// 010E awbb_LowBrGrZones_ZInfo_m_BMin */
    {0x0F12, 0x02F3, WORD_LEN, 0},// 02E9 awbb_LowBrGrZones_ZInfo_m_BMax */
    {0x002A, 0x0BAC, WORD_LEN, 0},
    {0x0F12, 0x000A, WORD_LEN, 0},// 0009 awbb_LowBrGrZones_ZInfo_m_GridSz */
    {0x002A, 0x0B7A, WORD_LEN, 0},
    {0x0F12, 0x036C, WORD_LEN, 0},// 0374 038C awbb_LowBrGrZones_m_BGrid_0__m_left   */
    {0x0F12, 0x03C6, WORD_LEN, 0},// 03CE 03DA awbb_LowBrGrZones_m_BGrid_0__m_right  */
    {0x0F12, 0x02EE, WORD_LEN, 0},// 02F6 030E awbb_LowBrGrZones_m_BGrid_1__m_left   */
    {0x0F12, 0x03F9, WORD_LEN, 0},// 0401 03E9 awbb_LowBrGrZones_m_BGrid_1__m_right  */
    {0x0F12, 0x02BE, WORD_LEN, 0},// 02C6 02A2 awbb_LowBrGrZones_m_BGrid_2__m_left   */
    {0x0F12, 0x03DF, WORD_LEN, 0},// 03E7 03C2 awbb_LowBrGrZones_m_BGrid_2__m_right  */
    {0x0F12, 0x027A, WORD_LEN, 0},// 0282 0259 awbb_LowBrGrZones_m_BGrid_3__m_left   */
    {0x0F12, 0x03AE, WORD_LEN, 0},// 03B6 038A awbb_LowBrGrZones_m_BGrid_3__m_right  */
    {0x0F12, 0x0234, WORD_LEN, 0},// 023C 0218 awbb_LowBrGrZones_m_BGrid_4__m_left   */
    {0x0F12, 0x0376, WORD_LEN, 0},// 037E 0352 awbb_LowBrGrZones_m_BGrid_4__m_right  */
    {0x0F12, 0x0204, WORD_LEN, 0},// 020C 01F4 awbb_LowBrGrZones_m_BGrid_5__m_left   */
    {0x0F12, 0x033E, WORD_LEN, 0},// 0346 02E1 awbb_LowBrGrZones_m_BGrid_5__m_right  */
    {0x0F12, 0x01E0, WORD_LEN, 0},// 01E8 01D7 awbb_LowBrGrZones_m_BGrid_6__m_left   */
    {0x0F12, 0x02CD, WORD_LEN, 0},// 02D5 028E awbb_LowBrGrZones_m_BGrid_6__m_right  */
    {0x0F12, 0x01C3, WORD_LEN, 0},// 01CB 01CB awbb_LowBrGrZones_m_BGrid_7__m_left   */
    {0x0F12, 0x027A, WORD_LEN, 0},// 0282 0258 awbb_LowBrGrZones_m_BGrid_7__m_right  */
    {0x0F12, 0x01B7, WORD_LEN, 0},// 01BF 022B awbb_LowBrGrZones_m_BGrid_8__m_left   */
    {0x0F12, 0x0244, WORD_LEN, 0},// 024C 01CC awbb_LowBrGrZones_m_BGrid_8__m_right  */
    {0x0F12, 0x01FE, WORD_LEN, 0},// 01F8 0000 awbb_LowBrGrZones_m_BGrid_9__m_left   */
    {0x0F12, 0x01DD, WORD_LEN, 0},// 0201 0000 awbb_LowBrGrZones_m_BGrid_9__m_right  */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 0000 awbb_LowBrGrZones_m_BGrid_10__m_left  */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 0000 awbb_LowBrGrZones_m_BGrid_10__m_right */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 0000 awbb_LowBrGrZones_m_BGrid_11__m_left  */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 0000 awbb_LowBrGrZones_m_BGrid_11__m_right */
    {0x002A, 0x0B70, WORD_LEN, 0},
    {0x0F12, 0x0005, WORD_LEN, 0},// awbb_OutdoorGrZones_ZInfo_m_GridStep */
    {0x002A, 0x0B74, WORD_LEN, 0},
    {0x0F12, 0x01F9, WORD_LEN, 0},// awbb_OutdoorGrZones_ZInfo_m_BMin */
    {0x0F12, 0x0286, WORD_LEN, 0},// awbb_OutdoorGrZones_ZInfo_m_BMax */
    {0x002A, 0x0B72, WORD_LEN, 0},
    {0x0F12, 0x0006, WORD_LEN, 0},// awbb_OutdoorGrZones_ZInfo_m_GridSz */
    {0x002A, 0x0B40, WORD_LEN, 0},
    {0x0F12, 0x0252, WORD_LEN, 0},// 029E awbb_OutdoorGrZones_m_BGrid_0__m_left   */
    {0x0F12, 0x0269, WORD_LEN, 0},// 02C8 awbb_OutdoorGrZones_m_BGrid_0__m_right  */
    {0x0F12, 0x022B, WORD_LEN, 0},// 0281 awbb_OutdoorGrZones_m_BGrid_1__m_left   */
    {0x0F12, 0x0288, WORD_LEN, 0},// 02C8 awbb_OutdoorGrZones_m_BGrid_1__m_right  */
    {0x0F12, 0x0214, WORD_LEN, 0},// 0266 awbb_OutdoorGrZones_m_BGrid_2__m_left   */
    {0x0F12, 0x0286, WORD_LEN, 0},// 02AC awbb_OutdoorGrZones_m_BGrid_2__m_right  */
    {0x0F12, 0x0205, WORD_LEN, 0},// 0251 awbb_OutdoorGrZones_m_BGrid_3__m_left   */
    {0x0F12, 0x026E, WORD_LEN, 0},// 028E awbb_OutdoorGrZones_m_BGrid_3__m_right  */
    {0x0F12, 0x020B, WORD_LEN, 0},// 023D awbb_OutdoorGrZones_m_BGrid_4__m_left   */
    {0x0F12, 0x0251, WORD_LEN, 0},// 0275 awbb_OutdoorGrZones_m_BGrid_4__m_right  */
    {0x0F12, 0x0237, WORD_LEN, 0},// 0228 awbb_OutdoorGrZones_m_BGrid_5__m_left   */
    {0x0F12, 0x0225, WORD_LEN, 0},// 025D awbb_OutdoorGrZones_m_BGrid_5__m_right  */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0228 awbb_OutdoorGrZones_m_BGrid_6__m_left   */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0243 awbb_OutdoorGrZones_m_BGrid_6__m_right  */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 awbb_OutdoorGrZones_m_BGrid_7__m_left   */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 awbb_OutdoorGrZones_m_BGrid_7__m_right  */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 awbb_OutdoorGrZones_m_BGrid_8__m_left   */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 awbb_OutdoorGrZones_m_BGrid_8__m_right  */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 awbb_OutdoorGrZones_m_BGrid_9__m_left   */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 awbb_OutdoorGrZones_m_BGrid_9__m_right  */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 awbb_OutdoorGrZones_m_BGrid_10__m_left  */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 awbb_OutdoorGrZones_m_BGrid_10__m_right */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 awbb_OutdoorGrZones_m_BGrid_11__m_left  */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 awbb_OutdoorGrZones_m_BGrid_11__m_right */
    {0x002A, 0x0BC8, WORD_LEN, 0},
    {0x0F12, 0x0005, WORD_LEN, 0},// awbb_CWSkinZone_ZInfo_m_GridStep */
    {0x002A, 0x0BCC, WORD_LEN, 0},
    {0x0F12, 0x010F, WORD_LEN, 0},// awbb_CWSkinZone_ZInfo_m_BMin */
    {0x0F12, 0x018F, WORD_LEN, 0},// awbb_CWSkinZone_ZInfo_m_BMax */
    {0x002A, 0x0BCA, WORD_LEN, 0},
    {0x0F12, 0x0005, WORD_LEN, 0},// awbb_CWSkinZone_ZInfo_m_GridSz */
    {0x002A, 0x0BB4, WORD_LEN, 0},
    {0x0F12, 0x03E7, WORD_LEN, 0},// awbb_CWSkinZone_m_BGrid_0__m_left  */
    {0x0F12, 0x03F8, WORD_LEN, 0},// awbb_CWSkinZone_m_BGrid_0__m_right */
    {0x0F12, 0x03A7, WORD_LEN, 0},// awbb_CWSkinZone_m_BGrid_1__m_left  */
    {0x0F12, 0x03FC, WORD_LEN, 0},// awbb_CWSkinZone_m_BGrid_1__m_right */
    {0x0F12, 0x0352, WORD_LEN, 0},// awbb_CWSkinZone_m_BGrid_2__m_left  */
    {0x0F12, 0x03D0, WORD_LEN, 0},// awbb_CWSkinZone_m_BGrid_2__m_right */
    {0x0F12, 0x0322, WORD_LEN, 0},// awbb_CWSkinZone_m_BGrid_3__m_left  */
    {0x0F12, 0x039E, WORD_LEN, 0},// awbb_CWSkinZone_m_BGrid_3__m_right */
    {0x0F12, 0x032B, WORD_LEN, 0},// awbb_CWSkinZone_m_BGrid_4__m_left  */
    {0x0F12, 0x034D, WORD_LEN, 0},// awbb_CWSkinZone_m_BGrid_4__m_right */
    {0x002A, 0x0BE6, WORD_LEN, 0},
    {0x0F12, 0x0006, WORD_LEN, 0},// awbb_DLSkinZone_ZInfo_m_GridStep */
    {0x002A, 0x0BEA, WORD_LEN, 0},
    {0x0F12, 0x019E, WORD_LEN, 0},// awbb_DLSkinZone_ZInfo_m_BMin */
    {0x0F12, 0x0257, WORD_LEN, 0},// awbb_DLSkinZone_ZInfo_m_BMax */
    {0x002A, 0x0BE8, WORD_LEN, 0},
    {0x0F12, 0x0004, WORD_LEN, 0},// awbb_DLSkinZone_ZInfo_m_GridSz */
    {0x002A, 0x0BD2, WORD_LEN, 0},
    {0x0F12, 0x030B, WORD_LEN, 0},// awbb_DLSkinZone_m_BGrid_0__m_left  */
    {0x0F12, 0x0323, WORD_LEN, 0},// awbb_DLSkinZone_m_BGrid_0__m_right */
    {0x0F12, 0x02C3, WORD_LEN, 0},// awbb_DLSkinZone_m_BGrid_1__m_left  */
    {0x0F12, 0x030F, WORD_LEN, 0},// awbb_DLSkinZone_m_BGrid_1__m_right */
    {0x0F12, 0x0288, WORD_LEN, 0},// awbb_DLSkinZone_m_BGrid_2__m_left  */
    {0x0F12, 0x02E5, WORD_LEN, 0},// awbb_DLSkinZone_m_BGrid_2__m_right */
    {0x0F12, 0x026A, WORD_LEN, 0},// awbb_DLSkinZone_m_BGrid_3__m_left  */
    {0x0F12, 0x02A2, WORD_LEN, 0},// awbb_DLSkinZone_m_BGrid_3__m_right */
    {0x0F12, 0x0000, WORD_LEN, 0},// awbb_DLSkinZone_m_BGrid_4__m_left  */
    {0x0F12, 0x0000, WORD_LEN, 0},// awbb_DLSkinZone_m_BGrid_4__m_right */
    {0x002A, 0x0C2C, WORD_LEN, 0},
    {0x0F12, 0x0128, WORD_LEN, 0},// awbb_IntcR */
    {0x0F12, 0x0122, WORD_LEN, 0},// awbb_IntcB */
    {0x002A, 0x0BFC, WORD_LEN, 0},
    {0x0F12, 0x0378, WORD_LEN, 0},// 03AD awbb_IndoorWP_0__r */
    {0x0F12, 0x011E, WORD_LEN, 0},// 013F awbb_IndoorWP_0__b */
    {0x0F12, 0x02F0, WORD_LEN, 0},// 0341 awbb_IndoorWP_1__r */
    {0x0F12, 0x0184, WORD_LEN, 0},// 017B awbb_IndoorWP_1__b */
    {0x0F12, 0x0313, WORD_LEN, 0},// 038D awbb_IndoorWP_2__r */
    {0x0F12, 0x0158, WORD_LEN, 0},// 014B awbb_IndoorWP_2__b */
    {0x0F12, 0x02BA, WORD_LEN, 0},// 02C3 awbb_IndoorWP_3__r */
    {0x0F12, 0x01BA, WORD_LEN, 0},// 01CC awbb_IndoorWP_3__b */
    {0x0F12, 0x0231, WORD_LEN, 0},// 0241 awbb_IndoorWP_4__r */
    {0x0F12, 0x0252, WORD_LEN, 0},// 027F awbb_IndoorWP_4__b */
    {0x0F12, 0x0237, WORD_LEN, 0},// 0241 awbb_IndoorWP_5__r */
    {0x0F12, 0x024C, WORD_LEN, 0},// 027F awbb_IndoorWP_5__b */
    {0x0F12, 0x020F, WORD_LEN, 0},// 0214 awbb_IndoorWP_6__r */
    {0x0F12, 0x0279, WORD_LEN, 0},// 02A8 awbb_IndoorWP_6__b */
    {0x0F12, 0x0268, WORD_LEN, 0},// 0270 255 awbb_OutdoorWP_r */
    {0x0F12, 0x021A, WORD_LEN, 0},// 0210 25B awbb_OutdoorWP_b */
    {0x002A, 0x0C4C, WORD_LEN, 0},
    {0x0F12, 0x0450, WORD_LEN, 0},// awbb_MvEq_RBthresh */
    {0x002A, 0x0C58, WORD_LEN, 0},
    {0x0F12, 0x059C, WORD_LEN, 0},// awbb_MvEq_RBthresh */
    {0x002A, 0x0BF8, WORD_LEN, 0},
    {0x0F12, 0x01AE, WORD_LEN, 0},// awbb_LowTSep_m_RminusB */
    {0x002A, 0x0C28, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},// awbb_SkinPreference */
    {0x002A, 0x0CAC, WORD_LEN, 0},
    {0x0F12, 0x0050, WORD_LEN, 0},// awbb_OutDMaxIncr */
    {0x002A, 0x0C28, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},// awbb_SkinPreference */ 

    {0x002A, 0x20BA, WORD_LEN, 0},
    {0x0F12, 0x0006, WORD_LEN, 0},// Lowtemp bypass */

    {0x002A, 0x0D0E, WORD_LEN, 0},
    {0x0F12, 0x00B8, WORD_LEN, 0},// awbb_GridCoeff_R_2 */
    {0x0F12, 0x00B2, WORD_LEN, 0},// awbb_GridCoeff_B_2 */
    {0x002A, 0x0CFE, WORD_LEN, 0},
    {0x0F12, 0x0FAB, WORD_LEN, 0},// 0FAB awbb_GridConst_2_0_ */
    {0x0F12, 0x0FF5, WORD_LEN, 0},// 0FF5 0FF5 awbb_GridConst_2_1_ */
    {0x0F12, 0x10BB, WORD_LEN, 0},// 10BB 10BB awbb_GridConst_2_2_ */
    {0x0F12, 0x1117, WORD_LEN, 0},// 1117 1123 1153 awbb_GridConst_2_3_ */
    {0x0F12, 0x116D, WORD_LEN, 0},// 116D 11C5 awbb_GridConst_2_4_ */
    {0x0F12, 0x11D5, WORD_LEN, 0},// 122A awbb_GridConst_2_5_ */
    {0x0F12, 0x00A9, WORD_LEN, 0},// awbb_GridCoeff_R_1 */
    {0x0F12, 0x00C0, WORD_LEN, 0},// awbb_GridCoeff_B_1 */
    {0x002A, 0x0CF8, WORD_LEN, 0},
    {0x0F12, 0x02CC, WORD_LEN, 0},// awbb_GridConst_1_0_ */
    {0x0F12, 0x031E, WORD_LEN, 0},// awbb_GridConst_1_1_ */
    {0x0F12, 0x0359, WORD_LEN, 0},// awbb_GridConst_1_2_ */

    {0x002A, 0x0CB0, WORD_LEN, 0},
    {0x0F12, 0x0030, WORD_LEN, 0},// 0000 awbb_GridCorr_R_0__0_ */
    {0x0F12, 0x0040, WORD_LEN, 0},// 0000 awbb_GridCorr_R_0__1_ */
    {0x0F12, 0x0060, WORD_LEN, 0},// 0078 awbb_GridCorr_R_0__2_ */
    {0x0F12, 0x0040, WORD_LEN, 0},// 00AA awbb_GridCorr_R_0__3_ */
    {0x0F12, 0x0008, WORD_LEN, 0},// 0000 awbb_GridCorr_R_0__4_ */
    {0x0F12, 0x0008, WORD_LEN, 0},// 0000 awbb_GridCorr_R_0__5_ */

    {0x0F12, 0x0030, WORD_LEN, 0},// 0000 awbb_GridCorr_R_1__0_ */
    {0x0F12, 0x0040, WORD_LEN, 0},// 0096 awbb_GridCorr_R_1__1_ */
    {0x0F12, 0x0060, WORD_LEN, 0},// 0000 awbb_GridCorr_R_1__2_ */
    {0x0F12, 0x0040, WORD_LEN, 0},// 0000 awbb_GridCorr_R_1__3_ */
    {0x0F12, 0x0008, WORD_LEN, 0},// 0000 awbb_GridCorr_R_1__4_ */
    {0x0F12, 0x0008, WORD_LEN, 0},// 0000 awbb_GridCorr_R_1__5_ */

    {0x0F12, 0x0030, WORD_LEN, 0},// 00E6 awbb_GridCorr_R_2__0_ */
    {0x0F12, 0x0040, WORD_LEN, 0},// 0000 awbb_GridCorr_R_2__1_ */
    {0x0F12, 0x0060, WORD_LEN, 0},// 0000 awbb_GridCorr_R_2__2_ */
    {0x0F12, 0x0040, WORD_LEN, 0},// 0000 awbb_GridCorr_R_2__3_ */
    {0x0F12, 0x0008, WORD_LEN, 0},// 0000 awbb_GridCorr_R_2__4_ */
    {0x0F12, 0x0008, WORD_LEN, 0},// 0000 awbb_GridCorr_R_2__5_ */

    {0x0F12, 0x0018, WORD_LEN, 0},// 0000 awbb_GridCorr_B_0__0_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 awbb_GridCorr_B_0__1_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0064 awbb_GridCorr_B_0__2_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 awbb_GridCorr_B_0__3_ */
    {0x0F12, 0xFF80, WORD_LEN, 0},// 0000 awbb_GridCorr_B_0__4_ */
    {0x0F12, 0xFEC0, WORD_LEN, 0},// 0000 awbb_GridCorr_B_0__5_ */

    {0x0F12, 0x0018, WORD_LEN, 0},// 0000 awbb_GridCorr_B_1__0_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0032 awbb_GridCorr_B_1__1_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 awbb_GridCorr_B_1__2_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 awbb_GridCorr_B_1__3_ */
    {0x0F12, 0xFF80, WORD_LEN, 0},// FF38 awbb_GridCorr_B_1__4_ */
    {0x0F12, 0xFEC0, WORD_LEN, 0},// 0000 awbb_GridCorr_B_1__5_ */

    {0x0F12, 0x0018, WORD_LEN, 0},// 0000 awbb_GridCorr_B_2__0_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0032 awbb_GridCorr_B_2__1_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 awbb_GridCorr_B_2__2_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 awbb_GridCorr_B_2__3_ */
    {0x0F12, 0xFF80, WORD_LEN, 0},// 0000 awbb_GridCorr_B_2__4_ */
    {0x0F12, 0xFEC0, WORD_LEN, 0},// 0000 awbb_GridCorr_B_2__5_ */

    {0x002A, 0x0D30, WORD_LEN, 0},
    {0x0F12, 0x0002, WORD_LEN, 0},// awbb_GridEnable */

    {0x002A, 0x3372, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},// awbb_bUseOutdoorGrid */
    {0x0F12, 0x0000, WORD_LEN, 0},// awbb_OutdoorGridCorr_R */
    {0x0F12, 0x0000, WORD_LEN, 0},// awbb_OutdoorGridCorr_B */

// For Outdoor Detector */
    {0x002A, 0x0C86, WORD_LEN, 0},
    {0x0F12, 0x0005, WORD_LEN, 0},// awbb_OutdoorDetectionZone_ZInfo_m_GridSz */
    {0x002A, 0x0C70, WORD_LEN, 0},
    {0x0F12, 0xFF7B, WORD_LEN, 0},// awbb_OutdoorDetectionZone_m_BGrid_0__m_left */
    {0x0F12, 0x00CE, WORD_LEN, 0},// awbb_OutdoorDetectionZone_m_BGrid_0__m_right */
    {0x0F12, 0xFF23, WORD_LEN, 0},// awbb_OutdoorDetectionZone_m_BGrid_1__m_left */
    {0x0F12, 0x010D, WORD_LEN, 0},// awbb_OutdoorDetectionZone_m_BGrid_1__m_right */
    {0x0F12, 0xFEF3, WORD_LEN, 0},// awbb_OutdoorDetectionZone_m_BGrid_2__m_left */
    {0x0F12, 0x012C, WORD_LEN, 0},// awbb_OutdoorDetectionZone_m_BGrid_2__m_right */
    {0x0F12, 0xFED7, WORD_LEN, 0},// awbb_OutdoorDetectionZone_m_BGrid_3__m_left */
    {0x0F12, 0x014E, WORD_LEN, 0},// awbb_OutdoorDetectionZone_m_BGrid_3__m_right */
    {0x0F12, 0xFEBB, WORD_LEN, 0},// awbb_OutdoorDetectionZone_m_BGrid_4__m_left */
    {0x0F12, 0x0162, WORD_LEN, 0},// awbb_OutdoorDetectionZone_m_BGrid_4__m_right */
    {0x0F12, 0x1388, WORD_LEN, 0},// awbb_OutdoorDetectionZone_ZInfo_m_AbsGridStep */
    {0x002A, 0x0C8A, WORD_LEN, 0},
    {0x0F12, 0x4ACB, WORD_LEN, 0},// awbb_OutdoorDetectionZone_ZInfo_m_MaxNB */
    {0x002A, 0x0C88, WORD_LEN, 0},
    {0x0F12, 0x0A7C, WORD_LEN, 0},// awbb_OutdoorDetectionZone_ZInfo_m_NBoffs */

    {0x002A, 0x0CA0, WORD_LEN, 0},
    {0x0F12, 0x0033, WORD_LEN, 0}, //awbb_GnCurPntImmunity

    {0x002A, 0x0CA4, WORD_LEN, 0},
    {0x0F12, 0x0033, WORD_LEN, 0}, //awbb_GnCurPntLongJump
    {0x0F12, 0x0180, WORD_LEN, 0}, //awbb_GainsMaxMove
    {0x0F12, 0x0002, WORD_LEN, 0}, //awbb_GnMinMatchToJump

//==================================================================================
// 10.Clock Setting


//==================================================================================
// Input Clock (Mclk) 
    {0x002A, 0x012E, WORD_LEN, 0},
    {0x0F12, 0x5DC0, WORD_LEN, 0},	// REG_TC_IPRM_InClockLSBs 
    {0x0F12, 0x0000, WORD_LEN, 0},	// REG_TC_IPRM_InClockMSBs 
    {0x002A, 0x0146, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},	// REG_TC_IPRM_UseNPviClocks 
    {0x0F12, 0x0001, WORD_LEN, 0},	// REG_TC_IPRM_UseNMipiClocks

// System Clock & Output clock (Pclk)
    {0x002A, 0x014C, WORD_LEN, 0},
    {0x0F12, 0x2AF8, WORD_LEN, 0},	// REG_TC_IPRM_OpClk4KHz_0 
    {0x002A, 0x0152, WORD_LEN, 0},
    {0x0F12, 0x55F0, WORD_LEN, 0},	// REG_TC_IPRM_MinOutRate4KHz_0 
    {0x002A, 0x014E, WORD_LEN, 0},
    {0x0F12, 0x55F0, WORD_LEN, 0},	// REG_TC_IPRM_MaxOutRate4KHz_0 
    {0x0F12, 0x00FA, WORD_LEN, 0},

    {0x002A, 0x0164, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},	// REG_TC_IPRM_InitParamsUpdated 

//*************************************/
// 11.Auto Flicker Detection          */
//*************************************/
    {0x0028, 0x7000, WORD_LEN, 0},
    {0x002A, 0x03F4, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},//REG_SF_USER_FlickerQuant */
    {0x0F12, 0x0001, WORD_LEN, 0},//REG_SF_USER_FlickerQuantChanged */
    {0x002A, 0x0408, WORD_LEN, 0},
    {0x0F12, 0x067F, WORD_LEN, 0},//REG_TC_DBG_AutoAlgEnBits all AA are on */

//*************************************/
// 12.AE Setting                      */
//*************************************/

    {0x002A, 0x0D40, WORD_LEN, 0},
    {0x0F12, 0x0038, WORD_LEN, 0}, //TVAR_ae_BrAve */

// For LT Calibration */
    {0x002A, 0x0D46, WORD_LEN, 0},
    {0x0F12, 0x000F, WORD_LEN, 0},// ae_StatMode */

    {0x002A, 0x0440, WORD_LEN, 0},
    {0x0F12, 0x3410, WORD_LEN, 0},// lt_uMaxExp_0_ */
    {0x002A, 0x0444, WORD_LEN, 0},
    {0x0F12, 0x6820, WORD_LEN, 0},// lt_uMaxExp_1_ */
    {0x002A, 0x0448, WORD_LEN, 0},
    {0x0F12, 0x8227, WORD_LEN, 0},// lt_uMaxExp_2_ */
    {0x002A, 0x044C, WORD_LEN, 0},
    {0x0F12, 0xC350, WORD_LEN, 0},// lt_uMaxExp_3_ */
    {0x002A, 0x0450, WORD_LEN, 0},
    {0x0F12, 0x3410, WORD_LEN, 0},// lt_uCapMaxExp_0_ */
    {0x002A, 0x0454, WORD_LEN, 0},
    {0x0F12, 0x6820, WORD_LEN, 0},// lt_uCapMaxExp_1_ */
    {0x002A, 0x0458, WORD_LEN, 0},
    {0x0F12, 0x8227, WORD_LEN, 0},// lt_uCapMaxExp_2_ */
    {0x002A, 0x045C, WORD_LEN, 0},
    {0x0F12, 0xC350, WORD_LEN, 0},// lt_uCapMaxExp_3_ */
    {0x002A, 0x0460, WORD_LEN, 0},
    {0x0F12, 0x01B0, WORD_LEN, 0},// lt_uMaxAnGain_0_ */
    {0x0F12, 0x01B0, WORD_LEN, 0},// lt_uMaxAnGain_1_ */
    {0x0F12, 0x0280, WORD_LEN, 0},// lt_uMaxAnGain_2_ */
    {0x0F12, 0x0600, WORD_LEN, 0},// lt_uMaxAnGain_3_ */
    {0x0F12, 0x0100, WORD_LEN, 0},// B0 0100 lt_uMaxDigGain */
    {0x0F12, 0x3000, WORD_LEN, 0},// lt_uMaxTotGain */
    {0x002A, 0x042E, WORD_LEN, 0},
    {0x0F12, 0x012E, WORD_LEN, 0},// lt_uLimitHigh */
    {0x0F12, 0x00D5, WORD_LEN, 0},// lt_uLimitLow */
    {0x002A, 0x0DE0, WORD_LEN, 0},
    {0x0F12, 0x0002, WORD_LEN, 0},// ae_Fade2BlackEnable F2B off, F2W on */

// For Illum Type Calibration */
// WRITE #SARR_IllumType_0_  0078 */
// WRITE #SARR_IllumType_1_  00C3 */
// WRITE #SARR_IllumType_2_  00E9 */
// WRITE #SARR_IllumType_3_  0128 */
// WRITE #SARR_IllumType_4_  016F */
// WRITE #SARR_IllumType_5_  0195 */
// WRITE #SARR_IllumType_6_  01A4 */
// WRITE #SARR_IllumTypeF_0_  0100 */
// WRITE #SARR_IllumTypeF_1_  0100 */
// WRITE #SARR_IllumTypeF_2_  0110 */
// WRITE #SARR_IllumTypeF_3_  00E5 */
// WRITE #SARR_IllumTypeF_4_  0100 */
// WRITE #SARR_IllumTypeF_5_  00ED */
// WRITE #SARR_IllumTypeF_6_  00ED */

    {0x002A, 0x0D38, WORD_LEN, 0},// bp_uMaxBrightnessFactor */
    {0x0F12, 0x019C, WORD_LEN, 0},
    {0x002A, 0x0D3E, WORD_LEN, 0},// bp_uMinBrightnessFactor */
    {0x0F12, 0x02A0, WORD_LEN, 0},

//*********************************************************************************
// 13.AE Weight (Normal)                                                           
//*********************************************************************************
    {0x002A, 0x0D4E, WORD_LEN, 0},
    {0x0F12, 0x0100, WORD_LEN, 0}, //ae_WeightTbl_16_0_  
    {0x0F12, 0x0001, WORD_LEN, 0}, //ae_WeightTbl_16_1_  
    {0x0F12, 0x0100, WORD_LEN, 0}, //ae_WeightTbl_16_2_  
    {0x0F12, 0x0001, WORD_LEN, 0}, //ae_WeightTbl_16_3_  
    {0x0F12, 0x0101, WORD_LEN, 0}, //ae_WeightTbl_16_4_  
    {0x0F12, 0x0101, WORD_LEN, 0}, //ae_WeightTbl_16_5_  
    {0x0F12, 0x0101, WORD_LEN, 0}, //ae_WeightTbl_16_6_  
    {0x0F12, 0x0101, WORD_LEN, 0}, //ae_WeightTbl_16_7_  
    {0x0F12, 0x0101, WORD_LEN, 0}, //ae_WeightTbl_16_8_  
    {0x0F12, 0x0201, WORD_LEN, 0}, //ae_WeightTbl_16_9_  
    {0x0F12, 0x0202, WORD_LEN, 0}, //ae_WeightTbl_16_10_ 
    {0x0F12, 0x0101, WORD_LEN, 0}, //ae_WeightTbl_16_11_ 
    {0x0F12, 0x0101, WORD_LEN, 0}, //ae_WeightTbl_16_12_ 
    {0x0F12, 0x0303, WORD_LEN, 0}, //ae_WeightTbl_16_13_ 
    {0x0F12, 0x0303, WORD_LEN, 0}, //ae_WeightTbl_16_14_ 
    {0x0F12, 0x0101, WORD_LEN, 0}, //ae_WeightTbl_16_15_ 
    {0x0F12, 0x0101, WORD_LEN, 0}, //ae_WeightTbl_16_16_ 
    {0x0F12, 0x0503, WORD_LEN, 0}, //ae_WeightTbl_16_17_ 
    {0x0F12, 0x0305, WORD_LEN, 0}, //ae_WeightTbl_16_18_ 
    {0x0F12, 0x0101, WORD_LEN, 0}, //ae_WeightTbl_16_19_ 
    {0x0F12, 0x0101, WORD_LEN, 0}, //ae_WeightTbl_16_20_ 
    {0x0F12, 0x0402, WORD_LEN, 0}, //ae_WeightTbl_16_21_ 
    {0x0F12, 0x0204, WORD_LEN, 0}, //ae_WeightTbl_16_22_ 
    {0x0F12, 0x0101, WORD_LEN, 0}, //ae_WeightTbl_16_23_ 
    {0x0F12, 0x0101, WORD_LEN, 0}, //ae_WeightTbl_16_24_ 
    {0x0F12, 0x0201, WORD_LEN, 0}, //ae_WeightTbl_16_25_ 
    {0x0F12, 0x0102, WORD_LEN, 0}, //ae_WeightTbl_16_26_ 
    {0x0F12, 0x0101, WORD_LEN, 0}, //ae_WeightTbl_16_27_ 
    {0x0F12, 0x0100, WORD_LEN, 0}, //ae_WeightTbl_16_28_ 
    {0x0F12, 0x0101, WORD_LEN, 0}, //ae_WeightTbl_16_29_ 
    {0x0F12, 0x0101, WORD_LEN, 0}, //ae_WeightTbl_16_30_ 
    {0x0F12, 0x0001, WORD_LEN, 0}, //ae_WeightTbl_16_31_ 

//*************************************/
// 14.Flash Setting                   */
//*************************************/

//*************************************/
// 15.CCM Setting                     */
//*************************************/
    {0x002A, 0x33A4, WORD_LEN, 0},
    {0x0F12, 0x01D0, WORD_LEN, 0},//01D0// Tune_wbt_BaseCcms_0__0_  */
    {0x0F12, 0xFFA1, WORD_LEN, 0},//FFA1// Tune_wbt_BaseCcms_0__1_  */
    {0x0F12, 0xFFFA, WORD_LEN, 0},//FFFA// Tune_wbt_BaseCcms_0__2_  */
    {0x0F12, 0xFF6F, WORD_LEN, 0},//FF6F// Tune_wbt_BaseCcms_0__3_  */
    {0x0F12, 0x0140, WORD_LEN, 0},//0140// Tune_wbt_BaseCcms_0__4_  */
    {0x0F12, 0xFF49, WORD_LEN, 0},//FF49// Tune_wbt_BaseCcms_0__5_  */
    {0x0F12, 0xFFC1, WORD_LEN, 0},//FFC1// Tune_wbt_BaseCcms_0__6_  */
    {0x0F12, 0x001F, WORD_LEN, 0},//001F// Tune_wbt_BaseCcms_0__7_  */
    {0x0F12, 0x01BD, WORD_LEN, 0},//01BD// Tune_wbt_BaseCcms_0__8_  */
    {0x0F12, 0x013F, WORD_LEN, 0},//013F// Tune_wbt_BaseCcms_0__9_  */
    {0x0F12, 0x00E1, WORD_LEN, 0},//00E1// Tune_wbt_BaseCcms_0__10_ */
    {0x0F12, 0xFF43, WORD_LEN, 0},//FF43// Tune_wbt_BaseCcms_0__11_ */
    {0x0F12, 0x0191, WORD_LEN, 0},//0191// Tune_wbt_BaseCcms_0__12_ */
    {0x0F12, 0xFFC0, WORD_LEN, 0},//FFC0// Tune_wbt_BaseCcms_0__13_ */
    {0x0F12, 0x01B7, WORD_LEN, 0},//01B7// Tune_wbt_BaseCcms_0__14_ */
    {0x0F12, 0xFF30, WORD_LEN, 0},//FF30// Tune_wbt_BaseCcms_0__15_ */
    {0x0F12, 0x015F, WORD_LEN, 0},//015F// Tune_wbt_BaseCcms_0__16_ */
    {0x0F12, 0x0106, WORD_LEN, 0},//0106// Tune_wbt_BaseCcms_0__17_ */
    {0x0F12, 0x01D0, WORD_LEN, 0},//01D0// Tune_wbt_BaseCcms_1__0_  */
    {0x0F12, 0xFFA1, WORD_LEN, 0},//FFA1// Tune_wbt_BaseCcms_1__1_  */
    {0x0F12, 0xFFFA, WORD_LEN, 0},//FFFA// Tune_wbt_BaseCcms_1__2_  */
    {0x0F12, 0xFF6F, WORD_LEN, 0},//FF6F// Tune_wbt_BaseCcms_1__3_  */
    {0x0F12, 0x0140, WORD_LEN, 0},//0140// Tune_wbt_BaseCcms_1__4_  */
    {0x0F12, 0xFF49, WORD_LEN, 0},//FF49// Tune_wbt_BaseCcms_1__5_  */
    {0x0F12, 0xFFC1, WORD_LEN, 0},//FFC1// Tune_wbt_BaseCcms_1__6_  */
    {0x0F12, 0x001F, WORD_LEN, 0},//001F// Tune_wbt_BaseCcms_1__7_  */
    {0x0F12, 0x01BD, WORD_LEN, 0},//01BD// Tune_wbt_BaseCcms_1__8_  */
    {0x0F12, 0x013F, WORD_LEN, 0},//013F// Tune_wbt_BaseCcms_1__9_  */
    {0x0F12, 0x00E1, WORD_LEN, 0},//00E1// Tune_wbt_BaseCcms_1__10_ */
    {0x0F12, 0xFF43, WORD_LEN, 0},//FF43// Tune_wbt_BaseCcms_1__11_ */
    {0x0F12, 0x0191, WORD_LEN, 0},//0191// Tune_wbt_BaseCcms_1__12_ */
    {0x0F12, 0xFFC0, WORD_LEN, 0},//FFC0// Tune_wbt_BaseCcms_1__13_ */
    {0x0F12, 0x01B7, WORD_LEN, 0},//01B7// Tune_wbt_BaseCcms_1__14_ */
    {0x0F12, 0xFF30, WORD_LEN, 0},//FF30// Tune_wbt_BaseCcms_1__15_ */
    {0x0F12, 0x015F, WORD_LEN, 0},//015F// Tune_wbt_BaseCcms_1__16_ */
    {0x0F12, 0x0106, WORD_LEN, 0},//0106// Tune_wbt_BaseCcms_1__17_ */
    {0x0F12, 0x01C4, WORD_LEN, 0},//01C4// 01F3 01C2 01D0 Tune_wbt_BaseCcms_2__0_  */
    {0x0F12, 0xFFAB, WORD_LEN, 0},//FFAB// FFCB FF93 FFA1 Tune_wbt_BaseCcms_2__1_  */
    {0x0F12, 0xFFFC, WORD_LEN, 0},//FFFC// 0031 001C FFFA Tune_wbt_BaseCcms_2__2_  */
    {0x0F12, 0xFF6E, WORD_LEN, 0},//FF6E// FF6F Tune_wbt_BaseCcms_2__3_  */
    {0x0F12, 0x0145, WORD_LEN, 0},//0145// 0140 Tune_wbt_BaseCcms_2__4_  */
    {0x0F12, 0xFF4A, WORD_LEN, 0},//FF4A// FF49 Tune_wbt_BaseCcms_2__5_  */
    {0x0F12, 0xFFE1, WORD_LEN, 0},//FFE1// FFE3 FFC1 Tune_wbt_BaseCcms_2__6_  */
    {0x0F12, 0xFFF6, WORD_LEN, 0},//FFF6// FFF9 001F Tune_wbt_BaseCcms_2__7_  */
    {0x0F12, 0x01BD, WORD_LEN, 0},//01BD// 01C1 01BD Tune_wbt_BaseCcms_2__8_  */
    {0x0F12, 0x013E, WORD_LEN, 0},//013E// 013F Tune_wbt_BaseCcms_2__9_  */
    {0x0F12, 0x00E4, WORD_LEN, 0},//00E4// 00E1 Tune_wbt_BaseCcms_2__10_ */
    {0x0F12, 0xFF46, WORD_LEN, 0},//FF46// FF43 Tune_wbt_BaseCcms_2__11_ */
    {0x0F12, 0x0190, WORD_LEN, 0},//0190// 0191 Tune_wbt_BaseCcms_2__12_ */
    {0x0F12, 0xFFBC, WORD_LEN, 0},//FFBC// FFC0 Tune_wbt_BaseCcms_2__13_ */
    {0x0F12, 0x01B5, WORD_LEN, 0},//01B5// 01B7 Tune_wbt_BaseCcms_2__14_ */
    {0x0F12, 0xFF30, WORD_LEN, 0},//FF30// FF30 Tune_wbt_BaseCcms_2__15_ */
    {0x0F12, 0x015E, WORD_LEN, 0},//015E// 015F Tune_wbt_BaseCcms_2__16_ */
    {0x0F12, 0x0103, WORD_LEN, 0},//0103// 0106 Tune_wbt_BaseCcms_2__17_ */
    {0x0F12, 0x01C4, WORD_LEN, 0},//01C4// 01D0 Tune_wbt_BaseCcms_3__0_  */
    {0x0F12, 0xFFAB, WORD_LEN, 0},//FFAB// FFA1 Tune_wbt_BaseCcms_3__1_  */
    {0x0F12, 0xFFFC, WORD_LEN, 0},//FFFC// FFFA Tune_wbt_BaseCcms_3__2_  */
    {0x0F12, 0xFF6E, WORD_LEN, 0},//FF6E// FF6F Tune_wbt_BaseCcms_3__3_  */
    {0x0F12, 0x0145, WORD_LEN, 0},//0145// 0140 Tune_wbt_BaseCcms_3__4_  */
    {0x0F12, 0xFF4A, WORD_LEN, 0},//FF4A// FF49 Tune_wbt_BaseCcms_3__5_  */
    {0x0F12, 0xFFE1, WORD_LEN, 0},//FFE1// FFE3 Tune_wbt_BaseCcms_3__6_  */
    {0x0F12, 0xFFF6, WORD_LEN, 0},//FFF6// FFF9 Tune_wbt_BaseCcms_3__7_  */
    {0x0F12, 0x01BD, WORD_LEN, 0},//01BD// 01C1 Tune_wbt_BaseCcms_3__8_  */
    {0x0F12, 0x013E, WORD_LEN, 0},//013E// 013F Tune_wbt_BaseCcms_3__9_  */
    {0x0F12, 0x00E4, WORD_LEN, 0},//00E4// 00E1 Tune_wbt_BaseCcms_3__10_ */
    {0x0F12, 0xFF46, WORD_LEN, 0},//FF46// FF43 Tune_wbt_BaseCcms_3__11_ */
    {0x0F12, 0x0190, WORD_LEN, 0},//0190// 0191 Tune_wbt_BaseCcms_3__12_ */
    {0x0F12, 0xFFBC, WORD_LEN, 0},//FFBC// FFC0 Tune_wbt_BaseCcms_3__13_ */
    {0x0F12, 0x01B5, WORD_LEN, 0},//01B5// 01B7 Tune_wbt_BaseCcms_3__14_ */
    {0x0F12, 0xFF30, WORD_LEN, 0},//FF30// FF30 Tune_wbt_BaseCcms_3__15_ */
    {0x0F12, 0x015E, WORD_LEN, 0},//015E// 015F Tune_wbt_BaseCcms_3__16_ */
    {0x0F12, 0x0103, WORD_LEN, 0},//0103// 0106 Tune_wbt_BaseCcms_3__17_ */
    {0x0F12, 0x01C6, WORD_LEN, 0},//01C6// 01BF 01E9 01C7 01BF 01C7 01BF Tune_wbt_BaseCcms_4__0_  */
    {0x0F12, 0xFFC2, WORD_LEN, 0},//FFC2// FFBF FFE4 FFA5 FFBF FFA5 FFBF Tune_wbt_BaseCcms_4__1_  */
    {0x0F12, 0x0004, WORD_LEN, 0},//0004// FFFE 0031 0006 FFFE 0006 FFFE Tune_wbt_BaseCcms_4__2_  */
    {0x0F12, 0xFF6F, WORD_LEN, 0},//FF6F// FF6D FF6F FF6D Tune_wbt_BaseCcms_4__3_  */
    {0x0F12, 0x01C9, WORD_LEN, 0},//01C9// 01B4 01C9 01B4 Tune_wbt_BaseCcms_4__4_  */
    {0x0F12, 0xFF4F, WORD_LEN, 0},//FF4F// FF66 FF4F FF66 Tune_wbt_BaseCcms_4__5_  */
    {0x0F12, 0xFFDB, WORD_LEN, 0},//FFDB// FFCA FFDB FFCA Tune_wbt_BaseCcms_4__6_  */
    {0x0F12, 0xFFC0, WORD_LEN, 0},//FFC0// FFCE FFC0 FFCE Tune_wbt_BaseCcms_4__7_  */
    {0x0F12, 0x019D, WORD_LEN, 0},//019D// 017B 019D 017B Tune_wbt_BaseCcms_4__8_  */
    {0x0F12, 0x0136, WORD_LEN, 0},//0136// Tune_wbt_BaseCcms_4__9_  */
    {0x0F12, 0x0132, WORD_LEN, 0},//0132// Tune_wbt_BaseCcms_4__10_ */
    {0x0F12, 0xFF85, WORD_LEN, 0},//FF85// Tune_wbt_BaseCcms_4__11_ */
    {0x0F12, 0x018B, WORD_LEN, 0},//018B// Tune_wbt_BaseCcms_4__12_ */
    {0x0F12, 0xFF73, WORD_LEN, 0},//FF73// Tune_wbt_BaseCcms_4__13_ */
    {0x0F12, 0x0191, WORD_LEN, 0},//0191// Tune_wbt_BaseCcms_4__14_ */
    {0x0F12, 0xFF3F, WORD_LEN, 0},//FF3F// Tune_wbt_BaseCcms_4__15_ */
    {0x0F12, 0x015B, WORD_LEN, 0},//015B// Tune_wbt_BaseCcms_4__16_ */
    {0x0F12, 0x00D0, WORD_LEN, 0},//00D0// Tune_wbt_BaseCcms_4__17_ */

    {0x0F12, 0x01C6, WORD_LEN, 0},//01C6// 01BF 01E9 01C7 01BF 01C7 01BF Tune_wbt_BaseCcms_5__0_  */
    {0x0F12, 0xFFC2, WORD_LEN, 0},//FFC2// FFBF FFE4 FFA5 FFBF FFA5 FFBF Tune_wbt_BaseCcms_5__1_  */
    {0x0F12, 0x0004, WORD_LEN, 0},//0004// FFFE 0031 0006 FFFE 0006 FFFE Tune_wbt_BaseCcms_5__2_  */
    {0x0F12, 0xFF56, WORD_LEN, 0},//FF6F// FF6D FF6F FF6D Tune_wbt_BaseCcms_5__3_  */
    {0x0F12, 0x01C9, WORD_LEN, 0},//01C9// 01B4 01C9 01B4 Tune_wbt_BaseCcms_5__4_  */
    {0x0F12, 0xFF68, WORD_LEN, 0},//FF4F// FF66 FF4F FF66 Tune_wbt_BaseCcms_5__5_  */
    {0x0F12, 0xFFDB, WORD_LEN, 0},//FFDB// FFCA FFDB FFCA FFE7 FFCA Tune_wbt_BaseCcms_5__6_  */
    {0x0F12, 0xFFC0, WORD_LEN, 0},//FFC0// FFCE FFC0 FFCE FFC2 FFCE Tune_wbt_BaseCcms_5__7_  */
    {0x0F12, 0x019D, WORD_LEN, 0},//019D// 017B 019D 017B 018C 017B Tune_wbt_BaseCcms_5__8_  */
    {0x0F12, 0x0140, WORD_LEN, 0},//0136// Tune_wbt_BaseCcms_5__9_  */
    {0x0F12, 0x012C, WORD_LEN, 0},//0132// Tune_wbt_BaseCcms_5__10_ */
    {0x0F12, 0xFF97, WORD_LEN, 0},//FF85// Tune_wbt_BaseCcms_5__11_ */
    {0x0F12, 0x018B, WORD_LEN, 0},//018B// Tune_wbt_BaseCcms_5__12_ */
    {0x0F12, 0xFF73, WORD_LEN, 0},//FF73// Tune_wbt_BaseCcms_5__13_ */
    {0x0F12, 0x0191, WORD_LEN, 0},//0191// Tune_wbt_BaseCcms_5__14_ */
    {0x0F12, 0xFF3F, WORD_LEN, 0},//FF3F// Tune_wbt_BaseCcms_5__15_ */
    {0x0F12, 0x015B, WORD_LEN, 0},//015B// Tune_wbt_BaseCcms_5__16_ */
    {0x0F12, 0x00D0, WORD_LEN, 0},//00D0// Tune_wbt_BaseCcms_5__17_ */

    {0x002A, 0x3380, WORD_LEN, 0},
    {0x0F12, 0x01D6, WORD_LEN, 0}, //023C// 021B 021B 023D 01E9 01C7 01BF 01AC Tune_wbt_OutdoorCcm_0_   */
    {0x0F12, 0xFF94, WORD_LEN, 0}, //FFE0// FFE9 0010 0038 FFE4 FFA5 FFBF FFD7 Tune_wbt_OutdoorCcm_1_   */
    {0x0F12, 0xFFCC, WORD_LEN, 0}, //0052// 0050 0073 0085 0031 0006 FFFE 0019 Tune_wbt_OutdoorCcm_2_   */
    {0x0F12, 0xFF1F, WORD_LEN, 0}, //FF6F// FF6D Tune_wbt_OutdoorCcm_3_   */
    {0x0F12, 0x021F, WORD_LEN, 0}, //01C9// 01B4 Tune_wbt_OutdoorCcm_4_   */
    {0x0F12, 0xFF1F, WORD_LEN, 0}, //FF4F// FF66 Tune_wbt_OutdoorCcm_5_   */
    {0x0F12, 0xFFE4, WORD_LEN, 0}, //0000// FFE9 FFDB FFD5 FFD5 FFE7 FFCA Tune_wbt_OutdoorCcm_6_   */
    {0x0F12, 0xFFED, WORD_LEN, 0}, //FFA4// FFBA FFC0 FFD6 FFD6 FFC2 FFCE Tune_wbt_OutdoorCcm_7_   */
    {0x0F12, 0x01C8, WORD_LEN, 0}, //01A5// 01AC 019D 018D 018D 018C 017B Tune_wbt_OutdoorCcm_8_   */
    {0x0F12, 0x0138, WORD_LEN, 0}, //0125// 0132 Tune_wbt_OutdoorCcm_9_   */
    {0x0F12, 0x0140, WORD_LEN, 0}, //013B// 012E Tune_wbt_OutdoorCcm_10_  */
    {0x0F12, 0xFF8A, WORD_LEN, 0}, //FF8D// FF8D Tune_wbt_OutdoorCcm_11_  */
    {0x0F12, 0x0210, WORD_LEN, 0}, //018B// Tune_wbt_OutdoorCcm_12_  */
    {0x0F12, 0xFF5D, WORD_LEN, 0}, //FF73// Tune_wbt_OutdoorCcm_13_  */
    {0x0F12, 0x0244, WORD_LEN, 0}, //0191// Tune_wbt_OutdoorCcm_14_  */
    {0x0F12, 0xFF10, WORD_LEN, 0}, //FF3F// Tune_wbt_OutdoorCcm_15_  */
    {0x0F12, 0x0190, WORD_LEN, 0}, //015B// Tune_wbt_OutdoorCcm_16_  */
    {0x0F12, 0x0145, WORD_LEN, 0}, //00D0// Tune_wbt_OutdoorCcm_17_  */
    {0x002A, 0x0612, WORD_LEN, 0},
    {0x0F12, 0x009D, WORD_LEN, 0},// SARR_AwbCcmCord_0_       */
    {0x0F12, 0x00D5, WORD_LEN, 0},// SARR_AwbCcmCord_1_       */
    {0x0F12, 0x0103, WORD_LEN, 0},// SARR_AwbCcmCord_2_       */
    {0x0F12, 0x0128, WORD_LEN, 0},// SARR_AwbCcmCord_3_       */
    {0x0F12, 0x0166, WORD_LEN, 0},// SARR_AwbCcmCord_4_       */
    {0x0F12, 0x0193, WORD_LEN, 0},// SARR_AwbCcmCord_5_       *

//*************************************/
// 17.GAMMA                           */
//*************************************/
// For Pre Post Gamma Calibration */
    {0x002A, 0x0538, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_0_   */
    {0x0F12, 0x001F, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_1_   */
    {0x0F12, 0x0035, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_2_   */
    {0x0F12, 0x005A, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_3_   */
    {0x0F12, 0x0095, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_4_   */
    {0x0F12, 0x00E6, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_5_   */
    {0x0F12, 0x0121, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_6_   */
    {0x0F12, 0x0139, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_7_   */
    {0x0F12, 0x0150, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_8_   */
    {0x0F12, 0x0177, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_9_   */
    {0x0F12, 0x019A, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_10_  */
    {0x0F12, 0x01BB, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_11_  */
    {0x0F12, 0x01DC, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_12_  */
    {0x0F12, 0x0219, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_13_  */
    {0x0F12, 0x0251, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_14_  */
    {0x0F12, 0x02B3, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_15_  */
    {0x0F12, 0x030A, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_16_  */
    {0x0F12, 0x035F, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_17_  */
    {0x0F12, 0x03B1, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_18_  */
    {0x0F12, 0x03FF, WORD_LEN, 0},// seti_uGammaLutPreDemNoBin_19_  */
    {0x0F12, 0x0000, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_0_  */
    {0x0F12, 0x0001, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_1_  */
    {0x0F12, 0x0001, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_2_  */
    {0x0F12, 0x0002, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_3_  */
    {0x0F12, 0x0004, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_4_  */
    {0x0F12, 0x000A, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_5_  */
    {0x0F12, 0x0012, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_6_  */
    {0x0F12, 0x0016, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_7_  */
    {0x0F12, 0x001A, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_8_  */
    {0x0F12, 0x0024, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_9_  */
    {0x0F12, 0x0031, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_10_ */
    {0x0F12, 0x003E, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_11_ */
    {0x0F12, 0x004E, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_12_ */
    {0x0F12, 0x0075, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_13_ */
    {0x0F12, 0x00A8, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_14_ */
    {0x0F12, 0x0126, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_15_ */
    {0x0F12, 0x01BE, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_16_ */
    {0x0F12, 0x0272, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_17_ */
    {0x0F12, 0x0334, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_18_ */
    {0x0F12, 0x03FF, WORD_LEN, 0},// seti_uGammaLutPostDemNoBin_19_ */

// For Gamma Calibration */

    {0x002A, 0x0498, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__0_   */
    {0x0F12, 0x0001, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__1_   */
    {0x0F12, 0x000B, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__2_   */
    {0x0F12, 0x0026, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__3_   */
    {0x0F12, 0x006F, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__4_   */
    {0x0F12, 0x00D5, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__5_   */
    {0x0F12, 0x0127, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__6_   */
    {0x0F12, 0x014C, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__7_   */
    {0x0F12, 0x016E, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__8_   */
    {0x0F12, 0x01A5, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__9_   */
    {0x0F12, 0x01D3, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__10_  */
    {0x0F12, 0x01FB, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__11_  */
    {0x0F12, 0x021F, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__12_  */
    {0x0F12, 0x0260, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__13_  */
    {0x0F12, 0x029A, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__14_  */
    {0x0F12, 0x02F7, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__15_  */
    {0x0F12, 0x034D, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__16_  */
    {0x0F12, 0x0395, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__17_  */
    {0x0F12, 0x03CE, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__18_  */
    {0x0F12, 0x03FF, WORD_LEN, 0},// SARR_usDualGammaLutRGBIndoor_0__19_  */
    {0x0F12, 0x0000, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__0_  */
    {0x0F12, 0x0004, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__1_  */
    {0x0F12, 0x000C, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__2_  */
    {0x0F12, 0x0024, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__3_  */
    {0x0F12, 0x006E, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__4_  */
    {0x0F12, 0x00D1, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__5_  */
    {0x0F12, 0x0119, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__6_  */
    {0x0F12, 0x0139, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__7_  */
    {0x0F12, 0x0157, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__8_  */
    {0x0F12, 0x018E, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__9_  */
    {0x0F12, 0x01C3, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__10_ */
    {0x0F12, 0x01F3, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__11_ */
    {0x0F12, 0x021F, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__12_ */
    {0x0F12, 0x0269, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__13_ */
    {0x0F12, 0x02A6, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__14_ */
    {0x0F12, 0x02FF, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__15_ */
    {0x0F12, 0x0351, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__16_ */
    {0x0F12, 0x0395, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__17_ */
    {0x0F12, 0x03CE, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__18_ */
    {0x0F12, 0x03FF, WORD_LEN, 0},// SARR_usDualGammaLutRGBOutdoor_0__19_ */

//*************************************/
// 16.AFIT                            */
//*************************************/
    {0x002A, 0x3370, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},// afit_bUseNormBrForAfit */

    {0x002A, 0x06D4, WORD_LEN, 0},
    {0x0F12, 0x0032, WORD_LEN, 0},// afit_uNoiseIndInDoor_0_ */
    {0x0F12, 0x0078, WORD_LEN, 0},// afit_uNoiseIndInDoor_1_ */
    {0x0F12, 0x00C8, WORD_LEN, 0},// afit_uNoiseIndInDoor_2_ */
    {0x0F12, 0x0190, WORD_LEN, 0},// afit_uNoiseIndInDoor_3_ */
    {0x0F12, 0x028C, WORD_LEN, 0},// afit_uNoiseIndInDoor_4_ */

    {0x002A, 0x0734, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_0__0_  Brightness[0] */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_0__1_  Contrast[0] */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_0__2_  Saturation[0] */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_0__3_  Sharp_Blur[0] */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_0__4_  */
    {0x0F12, 0x0078, WORD_LEN, 0},// AfitBaseVals_0__5_  */
    {0x0F12, 0x012C, WORD_LEN, 0},// AfitBaseVals_0__6_  */
    {0x0F12, 0x03FF, WORD_LEN, 0},// AfitBaseVals_0__7_  */
    {0x0F12, 0x0014, WORD_LEN, 0},// AfitBaseVals_0__8_  */
    {0x0F12, 0x0064, WORD_LEN, 0},// AfitBaseVals_0__9_  */
    {0x0F12, 0x000C, WORD_LEN, 0},// AfitBaseVals_0__10_ */
    {0x0F12, 0x0010, WORD_LEN, 0},// AfitBaseVals_0__11_ */
    {0x0F12, 0x01E6, WORD_LEN, 0},// AfitBaseVals_0__12_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_0__13_ */

    {0x0F12, 0x0070, WORD_LEN, 0},// AfitBaseVals_0__14_ */
    {0x0F12, 0x01FF, WORD_LEN, 0},// AfitBaseVals_0__15_ */
    {0x0F12, 0x0144, WORD_LEN, 0},// AfitBaseVals_0__16_ */
    {0x0F12, 0x000F, WORD_LEN, 0},// AfitBaseVals_0__17_ */
    {0x0F12, 0x000A, WORD_LEN, 0},// AfitBaseVals_0__18_ */
    {0x0F12, 0x0073, WORD_LEN, 0},// AfitBaseVals_0__19_ */
    {0x0F12, 0x0087, WORD_LEN, 0},// AfitBaseVals_0__20_ */
    {0x0F12, 0x0014, WORD_LEN, 0},// AfitBaseVals_0__21_ */
    {0x0F12, 0x000A, WORD_LEN, 0},// AfitBaseVals_0__22_ */
    {0x0F12, 0x0023, WORD_LEN, 0},// AfitBaseVals_0__23_ */
    {0x0F12, 0x001E, WORD_LEN, 0},// AfitBaseVals_0__24_ */
    {0x0F12, 0x0014, WORD_LEN, 0},// AfitBaseVals_0__25_ */
    {0x0F12, 0x000A, WORD_LEN, 0},// AfitBaseVals_0__26_ */
    {0x0F12, 0x0023, WORD_LEN, 0},// AfitBaseVals_0__27_ */
    {0x0F12, 0x0046, WORD_LEN, 0},// AfitBaseVals_0__28_ */
    {0x0F12, 0x2B32, WORD_LEN, 0},// AfitBaseVals_0__29_ */
    {0x0F12, 0x0601, WORD_LEN, 0},// AfitBaseVals_0__30_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_0__31_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_0__32_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_0__33_ */
    {0x0F12, 0x00FF, WORD_LEN, 0},// AfitBaseVals_0__34_ */
    {0x0F12, 0x07FF, WORD_LEN, 0},// AfitBaseVals_0__35_ */
    {0x0F12, 0xFFFF, WORD_LEN, 0},// AfitBaseVals_0__36_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_0__37_ */
    {0x0F12, 0x050D, WORD_LEN, 0},// AfitBaseVals_0__38_ */
    {0x0F12, 0x1E80, WORD_LEN, 0},// AfitBaseVals_0__39_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_0__40_ */
    {0x0F12, 0x1408, WORD_LEN, 0},// 1408 AfitBaseVals_0__41_ iNearGrayDesat[0] */
    {0x0F12, 0x0214, WORD_LEN, 0},// AfitBaseVals_0__42_ */
    {0x0F12, 0xFF01, WORD_LEN, 0},// AfitBaseVals_0__43_ */
    {0x0F12, 0x180F, WORD_LEN, 0},// AfitBaseVals_0__44_ */
    {0x0F12, 0x0001, WORD_LEN, 0},// AfitBaseVals_0__45_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_0__46_ */
    {0x0F12, 0x8003, WORD_LEN, 0},// AfitBaseVals_0__47_ */
    {0x0F12, 0x0094, WORD_LEN, 0},// AfitBaseVals_0__48_ */
    {0x0F12, 0x0580, WORD_LEN, 0},// AfitBaseVals_0__49_ */
    {0x0F12, 0x0280, WORD_LEN, 0},// AfitBaseVals_0__50_ */
    {0x0F12, 0x0310, WORD_LEN, 0},// AfitBaseVals_0__51_ iClustThresh_H[0] iClustMulT_H[0] */
    {0x0F12, 0x3186, WORD_LEN, 0},// AfitBaseVals_0__52_ */
    {0x0F12, 0x707E, WORD_LEN, 0},// AfitBaseVals_0__53_ */
    {0x0F12, 0x0A02, WORD_LEN, 0},// AfitBaseVals_0__54_ */
    {0x0F12, 0x080A, WORD_LEN, 0},// AfitBaseVals_0__55_ */
    {0x0F12, 0x0500, WORD_LEN, 0},// AfitBaseVals_0__56_ */
    {0x0F12, 0x032D, WORD_LEN, 0},// AfitBaseVals_0__57_ */
    {0x0F12, 0x324E, WORD_LEN, 0},// AfitBaseVals_0__58_ */
    {0x0F12, 0x001E, WORD_LEN, 0},// AfitBaseVals_0__59_ */
    {0x0F12, 0x0200, WORD_LEN, 0},// AfitBaseVals_0__60_ */
    {0x0F12, 0x0103, WORD_LEN, 0},// AfitBaseVals_0__61_ */
    {0x0F12, 0x010C, WORD_LEN, 0},// AfitBaseVals_0__62_ */
    {0x0F12, 0x9696, WORD_LEN, 0},// AfitBaseVals_0__63_ */
    {0x0F12, 0x4646, WORD_LEN, 0},// AfitBaseVals_0__64_ */
    {0x0F12, 0x0802, WORD_LEN, 0},// AfitBaseVals_0__65_ */
    {0x0F12, 0x0802, WORD_LEN, 0},// AfitBaseVals_0__66_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_0__67_ */
    {0x0F12, 0x030F, WORD_LEN, 0},// AfitBaseVals_0__68_ */
    {0x0F12, 0x3202, WORD_LEN, 0},// AfitBaseVals_0__69_ */
    {0x0F12, 0x0F1E, WORD_LEN, 0},// AfitBaseVals_0__70_ */
    {0x0F12, 0x020F, WORD_LEN, 0},// AfitBaseVals_0__71_ */
    {0x0F12, 0x0103, WORD_LEN, 0},// AfitBaseVals_0__72_ */
    {0x0F12, 0x010C, WORD_LEN, 0},// AfitBaseVals_0__73_ */
    {0x0F12, 0x9696, WORD_LEN, 0},// AfitBaseVals_0__74_ */
    {0x0F12, 0x4646, WORD_LEN, 0},// AfitBaseVals_0__75_ */
    {0x0F12, 0x0802, WORD_LEN, 0},// AfitBaseVals_0__76_ */
    {0x0F12, 0x0802, WORD_LEN, 0},// AfitBaseVals_0__77_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_0__78_ */
    {0x0F12, 0x030F, WORD_LEN, 0},// AfitBaseVals_0__79_ */
    {0x0F12, 0x3202, WORD_LEN, 0},// AfitBaseVals_0__80_ */
    {0x0F12, 0x0F1E, WORD_LEN, 0},// AfitBaseVals_0__81_ */
    {0x0F12, 0x020F, WORD_LEN, 0},// AfitBaseVals_0__82_ */
    {0x0F12, 0x0003, WORD_LEN, 0},// AfitBaseVals_0__83_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_1__0_  Brightness[1] */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_1__1_  Contrast[1] */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_1__2_  Saturation[1] */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_1__3_  Sharp_Blur[1] */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_1__4_  */
    {0x0F12, 0x006A, WORD_LEN, 0},// AfitBaseVals_1__5_  */
    {0x0F12, 0x012C, WORD_LEN, 0},// AfitBaseVals_1__6_  */
    {0x0F12, 0x03FF, WORD_LEN, 0},// AfitBaseVals_1__7_  */
    {0x0F12, 0x0014, WORD_LEN, 0},// AfitBaseVals_1__8_  */
    {0x0F12, 0x0064, WORD_LEN, 0},// AfitBaseVals_1__9_  */
    {0x0F12, 0x000C, WORD_LEN, 0},// AfitBaseVals_1__10_ */
    {0x0F12, 0x0010, WORD_LEN, 0},// AfitBaseVals_1__11_ */
    {0x0F12, 0x01E6, WORD_LEN, 0},// AfitBaseVals_1__12_ */
    {0x0F12, 0x03FF, WORD_LEN, 0},// AfitBaseVals_1__13_ */
    {0x0F12, 0x0070, WORD_LEN, 0},// AfitBaseVals_1__14_ */
    {0x0F12, 0x007D, WORD_LEN, 0},// AfitBaseVals_1__15_ */
    {0x0F12, 0x0064, WORD_LEN, 0},// AfitBaseVals_1__16_ */
    {0x0F12, 0x0014, WORD_LEN, 0},// AfitBaseVals_1__17_ */
    {0x0F12, 0x000A, WORD_LEN, 0},// AfitBaseVals_1__18_ */
    {0x0F12, 0x0073, WORD_LEN, 0},// AfitBaseVals_1__19_ */
    {0x0F12, 0x0087, WORD_LEN, 0},// AfitBaseVals_1__20_ */
    {0x0F12, 0x0014, WORD_LEN, 0},// AfitBaseVals_1__21_ */
    {0x0F12, 0x000A, WORD_LEN, 0},// AfitBaseVals_1__22_ */
    {0x0F12, 0x0023, WORD_LEN, 0},// AfitBaseVals_1__23_ */
    {0x0F12, 0x001E, WORD_LEN, 0},// AfitBaseVals_1__24_ */
    {0x0F12, 0x0014, WORD_LEN, 0},// AfitBaseVals_1__25_ */
    {0x0F12, 0x000A, WORD_LEN, 0},// AfitBaseVals_1__26_ */
    {0x0F12, 0x0023, WORD_LEN, 0},// AfitBaseVals_1__27_ */
    {0x0F12, 0x001E, WORD_LEN, 0},// AfitBaseVals_1__28_ */
    {0x0F12, 0x2B32, WORD_LEN, 0},// AfitBaseVals_1__29_ */
    {0x0F12, 0x0601, WORD_LEN, 0},// AfitBaseVals_1__30_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_1__31_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_1__32_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_1__33_ */
    {0x0F12, 0x00FF, WORD_LEN, 0},// AfitBaseVals_1__34_ */
    {0x0F12, 0x07FF, WORD_LEN, 0},// AfitBaseVals_1__35_ */
    {0x0F12, 0xFFFF, WORD_LEN, 0},// AfitBaseVals_1__36_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_1__37_ */
    {0x0F12, 0x050D, WORD_LEN, 0},// AfitBaseVals_1__38_ */
    {0x0F12, 0x1E80, WORD_LEN, 0},// AfitBaseVals_1__39_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_1__40_ */
    {0x0F12, 0x1403, WORD_LEN, 0},// 1408 AfitBaseVals_1__41_ iNearGrayDesat[1] */
    {0x0F12, 0x0214, WORD_LEN, 0},// AfitBaseVals_1__42_ */
    {0x0F12, 0xFF01, WORD_LEN, 0},// AfitBaseVals_1__43_ */
    {0x0F12, 0x180F, WORD_LEN, 0},// AfitBaseVals_1__44_ */
    {0x0F12, 0x0002, WORD_LEN, 0},// AfitBaseVals_1__45_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_1__46_ */
    {0x0F12, 0x8003, WORD_LEN, 0},// AfitBaseVals_1__47_ */
    {0x0F12, 0x0080, WORD_LEN, 0},// AfitBaseVals_1__48_ */
    {0x0F12, 0x0080, WORD_LEN, 0},// AfitBaseVals_1__49_ */
    {0x0F12, 0x0180, WORD_LEN, 0},// AfitBaseVals_1__50_ */
    {0x0F12, 0x0310, WORD_LEN, 0},// AfitBaseVals_1__51_ iClustThresh_H[1] iClustMulT_H[1] */
    {0x0F12, 0x1E65, WORD_LEN, 0},// AfitBaseVals_1__52_ */
    {0x0F12, 0x1A24, WORD_LEN, 0},// AfitBaseVals_1__53_ */
    {0x0F12, 0x0A03, WORD_LEN, 0},// AfitBaseVals_1__54_ */
    {0x0F12, 0x080A, WORD_LEN, 0},// AfitBaseVals_1__55_ */
    {0x0F12, 0x0500, WORD_LEN, 0},// AfitBaseVals_1__56_ */
    {0x0F12, 0x032D, WORD_LEN, 0},// AfitBaseVals_1__57_ */
    {0x0F12, 0x324D, WORD_LEN, 0},// AfitBaseVals_1__58_ */
    {0x0F12, 0x001E, WORD_LEN, 0},// AfitBaseVals_1__59_ */
    {0x0F12, 0x0200, WORD_LEN, 0},// AfitBaseVals_1__60_ */
    {0x0F12, 0x0103, WORD_LEN, 0},// AfitBaseVals_1__61_ */
    {0x0F12, 0x010C, WORD_LEN, 0},// AfitBaseVals_1__62_ */
    {0x0F12, 0x9696, WORD_LEN, 0},// AfitBaseVals_1__63_ */
    {0x0F12, 0x2F34, WORD_LEN, 0},// AfitBaseVals_1__64_ */
    {0x0F12, 0x0504, WORD_LEN, 0},// AfitBaseVals_1__65_ */
    {0x0F12, 0x080F, WORD_LEN, 0},// AfitBaseVals_1__66_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_1__67_ */
    {0x0F12, 0x030F, WORD_LEN, 0},// AfitBaseVals_1__68_ */
    {0x0F12, 0x3208, WORD_LEN, 0},// AfitBaseVals_1__69_ */
    {0x0F12, 0x0F1E, WORD_LEN, 0},// AfitBaseVals_1__70_ */
    {0x0F12, 0x020F, WORD_LEN, 0},// AfitBaseVals_1__71_ */
    {0x0F12, 0x0103, WORD_LEN, 0},// AfitBaseVals_1__72_ */
    {0x0F12, 0x010C, WORD_LEN, 0},// AfitBaseVals_1__73_ */
    {0x0F12, 0x9696, WORD_LEN, 0},// AfitBaseVals_1__74_ */
    {0x0F12, 0x1414, WORD_LEN, 0},// AfitBaseVals_1__75_ */
    {0x0F12, 0x0504, WORD_LEN, 0},// AfitBaseVals_1__76_ */
    {0x0F12, 0x080F, WORD_LEN, 0},// AfitBaseVals_1__77_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_1__78_ */
    {0x0F12, 0x030F, WORD_LEN, 0},// AfitBaseVals_1__79_ */
    {0x0F12, 0x3208, WORD_LEN, 0},// AfitBaseVals_1__80_ */
    {0x0F12, 0x0F1E, WORD_LEN, 0},// AfitBaseVals_1__81_ */
    {0x0F12, 0x020F, WORD_LEN, 0},// AfitBaseVals_1__82_ */
    {0x0F12, 0x0003, WORD_LEN, 0},// AfitBaseVals_1__83_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_2__0_  Brightness[2] */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_2__1_  Contrast[2] */
    {0x0F12, 0xFFFC, WORD_LEN, 0},// 0000 AfitBaseVals_2__2_  Saturation[2] */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_2__3_  Sharp_Blur[2] */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_2__4_  */
    {0x0F12, 0x0064, WORD_LEN, 0},// AfitBaseVals_2__5_  */
    {0x0F12, 0x012C, WORD_LEN, 0},// AfitBaseVals_2__6_  */
    {0x0F12, 0x03FF, WORD_LEN, 0},// AfitBaseVals_2__7_  */
    {0x0F12, 0x0014, WORD_LEN, 0},// AfitBaseVals_2__8_  */
    {0x0F12, 0x0064, WORD_LEN, 0},// AfitBaseVals_2__9_  */
    {0x0F12, 0x000C, WORD_LEN, 0},// AfitBaseVals_2__10_ */
    {0x0F12, 0x0010, WORD_LEN, 0},// AfitBaseVals_2__11_ */
    {0x0F12, 0x01E6, WORD_LEN, 0},// AfitBaseVals_2__12_ */
    {0x0F12, 0x03FF, WORD_LEN, 0},// AfitBaseVals_2__13_ */
    {0x0F12, 0x0070, WORD_LEN, 0},// AfitBaseVals_2__14_ */
    {0x0F12, 0x007D, WORD_LEN, 0},// AfitBaseVals_2__15_ */
    {0x0F12, 0x0064, WORD_LEN, 0},// AfitBaseVals_2__16_ */
    {0x0F12, 0x0032, WORD_LEN, 0},// 0096 AfitBaseVals_2__17_ */
    {0x0F12, 0x0096, WORD_LEN, 0},// AfitBaseVals_2__18_ */
    {0x0F12, 0x0073, WORD_LEN, 0},// AfitBaseVals_2__19_ */
    {0x0F12, 0x0087, WORD_LEN, 0},// AfitBaseVals_2__20_ */
    {0x0F12, 0x0014, WORD_LEN, 0},// AfitBaseVals_2__21_ */
    {0x0F12, 0x0019, WORD_LEN, 0},// AfitBaseVals_2__22_ */
    {0x0F12, 0x0023, WORD_LEN, 0},// AfitBaseVals_2__23_ */
    {0x0F12, 0x001E, WORD_LEN, 0},// AfitBaseVals_2__24_ */
    {0x0F12, 0x0014, WORD_LEN, 0},// AfitBaseVals_2__25_ */
    {0x0F12, 0x0019, WORD_LEN, 0},// AfitBaseVals_2__26_ */
    {0x0F12, 0x0023, WORD_LEN, 0},// AfitBaseVals_2__27_ */
    {0x0F12, 0x001E, WORD_LEN, 0},// AfitBaseVals_2__28_ */
    {0x0F12, 0x2B32, WORD_LEN, 0},// AfitBaseVals_2__29_ */
    {0x0F12, 0x0601, WORD_LEN, 0},// AfitBaseVals_2__30_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_2__31_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_2__32_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_2__33_ */
    {0x0F12, 0x00FF, WORD_LEN, 0},// AfitBaseVals_2__34_ */
    {0x0F12, 0x07FF, WORD_LEN, 0},// AfitBaseVals_2__35_ */
    {0x0F12, 0xFFFF, WORD_LEN, 0},// AfitBaseVals_2__36_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_2__37_ */
    {0x0F12, 0x050D, WORD_LEN, 0},// AfitBaseVals_2__38_ */
    {0x0F12, 0x1E80, WORD_LEN, 0},// AfitBaseVals_2__39_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_2__40_ */
    {0x0F12, 0x0A03, WORD_LEN, 0},// 0A08 AfitBaseVals_2__41_ iNearGrayDesat[2] */
    {0x0F12, 0x0200, WORD_LEN, 0},// AfitBaseVals_2__42_ */
    {0x0F12, 0xFF01, WORD_LEN, 0},// AfitBaseVals_2__43_ */
    {0x0F12, 0x180F, WORD_LEN, 0},// AfitBaseVals_2__44_ */
    {0x0F12, 0x0002, WORD_LEN, 0},// AfitBaseVals_2__45_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_2__46_ */
    {0x0F12, 0x8003, WORD_LEN, 0},// AfitBaseVals_2__47_ */
    {0x0F12, 0x0080, WORD_LEN, 0},// AfitBaseVals_2__48_ */
    {0x0F12, 0x0080, WORD_LEN, 0},// AfitBaseVals_2__49_ */
    {0x0F12, 0x0180, WORD_LEN, 0},// AfitBaseVals_2__50_ */
    {0x0F12, 0x0210, WORD_LEN, 0},// AfitBaseVals_2__51_ iClustThresh_H[2] iClustMulT_H[2] */
    {0x0F12, 0x1E4B, WORD_LEN, 0},// AfitBaseVals_2__52_ */
    {0x0F12, 0x1A24, WORD_LEN, 0},// AfitBaseVals_2__53_ */
    {0x0F12, 0x0A05, WORD_LEN, 0},// AfitBaseVals_2__54_ */
    {0x0F12, 0x080A, WORD_LEN, 0},// AfitBaseVals_2__55_ */
    {0x0F12, 0x0500, WORD_LEN, 0},// AfitBaseVals_2__56_ */
    {0x0F12, 0x032D, WORD_LEN, 0},// AfitBaseVals_2__57_ */
    {0x0F12, 0x324D, WORD_LEN, 0},// AfitBaseVals_2__58_ */
    {0x0F12, 0x001E, WORD_LEN, 0},// AfitBaseVals_2__59_ */
    {0x0F12, 0x0200, WORD_LEN, 0},// AfitBaseVals_2__60_ */
    {0x0F12, 0x0103, WORD_LEN, 0},// AfitBaseVals_2__61_ */
    {0x0F12, 0x010C, WORD_LEN, 0},// AfitBaseVals_2__62_ */
    {0x0F12, 0x9696, WORD_LEN, 0},// AfitBaseVals_2__63_ */
    {0x0F12, 0x1E23, WORD_LEN, 0},// AfitBaseVals_2__64_ */
    {0x0F12, 0x0505, WORD_LEN, 0},// AfitBaseVals_2__65_ */
    {0x0F12, 0x080F, WORD_LEN, 0},// AfitBaseVals_2__66_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_2__67_ */
    {0x0F12, 0x030F, WORD_LEN, 0},// AfitBaseVals_2__68_ */
    {0x0F12, 0x3208, WORD_LEN, 0},// AfitBaseVals_2__69_ */
    {0x0F12, 0x0F1E, WORD_LEN, 0},// AfitBaseVals_2__70_ */
    {0x0F12, 0x020F, WORD_LEN, 0},// AfitBaseVals_2__71_ */
    {0x0F12, 0x0103, WORD_LEN, 0},// AfitBaseVals_2__72_ */
    {0x0F12, 0x010C, WORD_LEN, 0},// AfitBaseVals_2__73_ */
    {0x0F12, 0x9696, WORD_LEN, 0},// AfitBaseVals_2__74_ */
    {0x0F12, 0x1E23, WORD_LEN, 0},// AfitBaseVals_2__75_ */
    {0x0F12, 0x0505, WORD_LEN, 0},// AfitBaseVals_2__76_ */
    {0x0F12, 0x080F, WORD_LEN, 0},// AfitBaseVals_2__77_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_2__78_ */
    {0x0F12, 0x030F, WORD_LEN, 0},// AfitBaseVals_2__79_ */
    {0x0F12, 0x3208, WORD_LEN, 0},// AfitBaseVals_2__80_ */
    {0x0F12, 0x0F1E, WORD_LEN, 0},// AfitBaseVals_2__81_ */
    {0x0F12, 0x020F, WORD_LEN, 0},// AfitBaseVals_2__82_ */
    {0x0F12, 0x0003, WORD_LEN, 0},// AfitBaseVals_2__83_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_3__0_  Brightness[3] */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 AfitBaseVals_3__1_  Contrast[3] */
    {0x0F12, 0xFFFA, WORD_LEN, 0},// AfitBaseVals_3__2_  Saturation[3] */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_3__3_  Sharp_Blur[3] */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_3__4_  */
    {0x0F12, 0x0064, WORD_LEN, 0},// AfitBaseVals_3__5_  */
    {0x0F12, 0x012C, WORD_LEN, 0},// AfitBaseVals_3__6_  */
    {0x0F12, 0x03FF, WORD_LEN, 0},// AfitBaseVals_3__7_  */
    {0x0F12, 0x0014, WORD_LEN, 0},// AfitBaseVals_3__8_  */
    {0x0F12, 0x0064, WORD_LEN, 0},// AfitBaseVals_3__9_  */
    {0x0F12, 0x000C, WORD_LEN, 0},// AfitBaseVals_3__10_ */
    {0x0F12, 0x0010, WORD_LEN, 0},// AfitBaseVals_3__11_ */
    {0x0F12, 0x01E6, WORD_LEN, 0},// AfitBaseVals_3__12_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_3__13_ */
    {0x0F12, 0x0070, WORD_LEN, 0},// AfitBaseVals_3__14_ */
    {0x0F12, 0x007D, WORD_LEN, 0},// AfitBaseVals_3__15_ */
    {0x0F12, 0x0064, WORD_LEN, 0},// AfitBaseVals_3__16_ */
    {0x0F12, 0x0032, WORD_LEN, 0},// 0096 AfitBaseVals_3__17_ */
    {0x0F12, 0x0096, WORD_LEN, 0},// AfitBaseVals_3__18_ */
    {0x0F12, 0x0073, WORD_LEN, 0},// AfitBaseVals_3__19_ */
    {0x0F12, 0x009F, WORD_LEN, 0},// AfitBaseVals_3__20_ */
    {0x0F12, 0x0028, WORD_LEN, 0},// AfitBaseVals_3__21_ */
    {0x0F12, 0x0028, WORD_LEN, 0},// AfitBaseVals_3__22_ */
    {0x0F12, 0x0023, WORD_LEN, 0},// AfitBaseVals_3__23_ */
    {0x0F12, 0x0037, WORD_LEN, 0},// AfitBaseVals_3__24_ */
    {0x0F12, 0x0028, WORD_LEN, 0},// AfitBaseVals_3__25_ */
    {0x0F12, 0x0028, WORD_LEN, 0},// AfitBaseVals_3__26_ */
    {0x0F12, 0x0023, WORD_LEN, 0},// AfitBaseVals_3__27_ */
    {0x0F12, 0x0037, WORD_LEN, 0},// AfitBaseVals_3__28_ */
    {0x0F12, 0x2B32, WORD_LEN, 0},// AfitBaseVals_3__29_ */
    {0x0F12, 0x0601, WORD_LEN, 0},// AfitBaseVals_3__30_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_3__31_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_3__32_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_3__33_ */
    {0x0F12, 0x00FF, WORD_LEN, 0},// AfitBaseVals_3__34_ */
    {0x0F12, 0x07A0, WORD_LEN, 0},// AfitBaseVals_3__35_ */
    {0x0F12, 0xFFFF, WORD_LEN, 0},// AfitBaseVals_3__36_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_3__37_ */
    {0x0F12, 0x050D, WORD_LEN, 0},// AfitBaseVals_3__38_ */
    {0x0F12, 0x1E80, WORD_LEN, 0},// AfitBaseVals_3__39_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_3__40_ */
    {0x0F12, 0x0A03, WORD_LEN, 0},// 0A08 AfitBaseVals_3__41_ iNearGrayDesat[3] */
    {0x0F12, 0x0200, WORD_LEN, 0},// AfitBaseVals_3__42_ */
    {0x0F12, 0xFF01, WORD_LEN, 0},// AfitBaseVals_3__43_ */
    {0x0F12, 0x180F, WORD_LEN, 0},// AfitBaseVals_3__44_ */
    {0x0F12, 0x0001, WORD_LEN, 0},// AfitBaseVals_3__45_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_3__46_ */
    {0x0F12, 0x8003, WORD_LEN, 0},// AfitBaseVals_3__47_ */
    {0x0F12, 0x0080, WORD_LEN, 0},// AfitBaseVals_3__48_ */
    {0x0F12, 0x0080, WORD_LEN, 0},// AfitBaseVals_3__49_ */
    {0x0F12, 0x0180, WORD_LEN, 0},// AfitBaseVals_3__50_ */
    {0x0F12, 0x0110, WORD_LEN, 0},// AfitBaseVals_3__51_ iClustThresh_H[3] iClustMulT_H[3] */
    {0x0F12, 0x1E32, WORD_LEN, 0},// AfitBaseVals_3__52_ */
    {0x0F12, 0x1A24, WORD_LEN, 0},// AfitBaseVals_3__53_ */
    {0x0F12, 0x0A05, WORD_LEN, 0},// AfitBaseVals_3__54_ */
    {0x0F12, 0x080A, WORD_LEN, 0},// AfitBaseVals_3__55_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_3__56_ */
    {0x0F12, 0x0328, WORD_LEN, 0},// AfitBaseVals_3__57_ */
    {0x0F12, 0x324C, WORD_LEN, 0},// AfitBaseVals_3__58_ */
    {0x0F12, 0x001E, WORD_LEN, 0},// AfitBaseVals_3__59_ */
    {0x0F12, 0x0200, WORD_LEN, 0},// AfitBaseVals_3__60_ */
    {0x0F12, 0x0103, WORD_LEN, 0},// AfitBaseVals_3__61_ */
    {0x0F12, 0x010C, WORD_LEN, 0},// AfitBaseVals_3__62_ */
    {0x0F12, 0x9696, WORD_LEN, 0},// AfitBaseVals_3__63_ */
    {0x0F12, 0x0F0F, WORD_LEN, 0},// AfitBaseVals_3__64_ */
    {0x0F12, 0x0307, WORD_LEN, 0},// AfitBaseVals_3__65_ */
    {0x0F12, 0x080F, WORD_LEN, 0},// AfitBaseVals_3__66_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_3__67_ */
    {0x0F12, 0x030F, WORD_LEN, 0},// AfitBaseVals_3__68_ */
    {0x0F12, 0x3208, WORD_LEN, 0},// AfitBaseVals_3__69_ */
    {0x0F12, 0x0F1E, WORD_LEN, 0},// AfitBaseVals_3__70_ */
    {0x0F12, 0x020F, WORD_LEN, 0},// AfitBaseVals_3__71_ */
    {0x0F12, 0x0103, WORD_LEN, 0},// AfitBaseVals_3__72_ */
    {0x0F12, 0x010C, WORD_LEN, 0},// AfitBaseVals_3__73_ */
    {0x0F12, 0x9696, WORD_LEN, 0},// AfitBaseVals_3__74_ */
    {0x0F12, 0x0F0F, WORD_LEN, 0},// AfitBaseVals_3__75_ */
    {0x0F12, 0x0307, WORD_LEN, 0},// AfitBaseVals_3__76_ */
    {0x0F12, 0x080F, WORD_LEN, 0},// AfitBaseVals_3__77_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_3__78_ */
    {0x0F12, 0x030F, WORD_LEN, 0},// AfitBaseVals_3__79_ */
    {0x0F12, 0x3208, WORD_LEN, 0},// AfitBaseVals_3__80_ */
    {0x0F12, 0x0F1E, WORD_LEN, 0},// AfitBaseVals_3__81_ */
    {0x0F12, 0x020F, WORD_LEN, 0},// AfitBaseVals_3__82_ */
    {0x0F12, 0x0003, WORD_LEN, 0},// AfitBaseVals_3__83_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_4__0_  Brightness[4] */
    {0x0F12, 0x0000, WORD_LEN, 0},// 0000 AfitBaseVals_4__1_  Contrast[4] */
    {0x0F12, 0xFFF8, WORD_LEN, 0},// AfitBaseVals_4__2_  Saturation[4] */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_4__3_  Sharp_Blur[4] */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_4__4_  */
    {0x0F12, 0x0028, WORD_LEN, 0},// AfitBaseVals_4__5_  */
    {0x0F12, 0x012C, WORD_LEN, 0},// AfitBaseVals_4__6_  */
    {0x0F12, 0x03FF, WORD_LEN, 0},// AfitBaseVals_4__7_  */
    {0x0F12, 0x0014, WORD_LEN, 0},// AfitBaseVals_4__8_  */
    {0x0F12, 0x0064, WORD_LEN, 0},// AfitBaseVals_4__9_  */
    {0x0F12, 0x000C, WORD_LEN, 0},// AfitBaseVals_4__10_ */
    {0x0F12, 0x0010, WORD_LEN, 0},// AfitBaseVals_4__11_ */
    {0x0F12, 0x01E6, WORD_LEN, 0},// AfitBaseVals_4__12_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_4__13_ */
    {0x0F12, 0x0070, WORD_LEN, 0},// AfitBaseVals_4__14_ */
    {0x0F12, 0x0087, WORD_LEN, 0},// AfitBaseVals_4__15_ */
    {0x0F12, 0x0073, WORD_LEN, 0},// AfitBaseVals_4__16_ */
    {0x0F12, 0x0032, WORD_LEN, 0},// 0096 AfitBaseVals_4__17_ */
    {0x0F12, 0x0096, WORD_LEN, 0},// AfitBaseVals_4__18_ */
    {0x0F12, 0x0073, WORD_LEN, 0},// AfitBaseVals_4__19_ */
    {0x0F12, 0x00B4, WORD_LEN, 0},// AfitBaseVals_4__20_ */
    {0x0F12, 0x0028, WORD_LEN, 0},// AfitBaseVals_4__21_ */
    {0x0F12, 0x0028, WORD_LEN, 0},// AfitBaseVals_4__22_ */
    {0x0F12, 0x0023, WORD_LEN, 0},// AfitBaseVals_4__23_ */
    {0x0F12, 0x0046, WORD_LEN, 0},// AfitBaseVals_4__24_ */
    {0x0F12, 0x0028, WORD_LEN, 0},// AfitBaseVals_4__25_ */
    {0x0F12, 0x0028, WORD_LEN, 0},// AfitBaseVals_4__26_ */
    {0x0F12, 0x0023, WORD_LEN, 0},// AfitBaseVals_4__27_ */
    {0x0F12, 0x0046, WORD_LEN, 0},// AfitBaseVals_4__28_ */
    {0x0F12, 0x2B23, WORD_LEN, 0},// AfitBaseVals_4__29_ */
    {0x0F12, 0x0601, WORD_LEN, 0},// AfitBaseVals_4__30_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_4__31_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_4__32_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_4__33_ */
    {0x0F12, 0x00FF, WORD_LEN, 0},// AfitBaseVals_4__34_ */
    {0x0F12, 0x0B84, WORD_LEN, 0},// AfitBaseVals_4__35_ */
    {0x0F12, 0xFFFF, WORD_LEN, 0},// AfitBaseVals_4__36_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_4__37_ */
    {0x0F12, 0x050D, WORD_LEN, 0},// AfitBaseVals_4__38_ */
    {0x0F12, 0x1E80, WORD_LEN, 0},// AfitBaseVals_4__39_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_4__40_ */
    {0x0F12, 0x0A03, WORD_LEN, 0},// 0A08 AfitBaseVals_4__41_ iNearGrayDesat[4] */
    {0x0F12, 0x0200, WORD_LEN, 0},// AfitBaseVals_4__42_ */
    {0x0F12, 0xFF01, WORD_LEN, 0},// AfitBaseVals_4__43_ */
    {0x0F12, 0x180F, WORD_LEN, 0},// AfitBaseVals_4__44_ */
    {0x0F12, 0x0001, WORD_LEN, 0},// AfitBaseVals_4__45_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_4__46_ */
    {0x0F12, 0x8003, WORD_LEN, 0},// AfitBaseVals_4__47_ */
    {0x0F12, 0x0080, WORD_LEN, 0},// AfitBaseVals_4__48_ */
    {0x0F12, 0x0080, WORD_LEN, 0},// AfitBaseVals_4__49_ */
    {0x0F12, 0x0180, WORD_LEN, 0},// AfitBaseVals_4__50_ */
    {0x0F12, 0x0110, WORD_LEN, 0},// AfitBaseVals_4__51_ iClustThresh_H[4] iClustMulT_H[4] */
    {0x0F12, 0x1E1E, WORD_LEN, 0},// AfitBaseVals_4__52_ */
    {0x0F12, 0x1419, WORD_LEN, 0},// AfitBaseVals_4__53_ */
    {0x0F12, 0x0A0A, WORD_LEN, 0},// AfitBaseVals_4__54_ */
    {0x0F12, 0x0800, WORD_LEN, 0},// AfitBaseVals_4__55_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_4__56_ */
    {0x0F12, 0x0328, WORD_LEN, 0},// AfitBaseVals_4__57_ */
    {0x0F12, 0x324C, WORD_LEN, 0},// AfitBaseVals_4__58_ */
    {0x0F12, 0x001E, WORD_LEN, 0},// AfitBaseVals_4__59_ */
    {0x0F12, 0x0200, WORD_LEN, 0},// AfitBaseVals_4__60_ */
    {0x0F12, 0x0103, WORD_LEN, 0},// AfitBaseVals_4__61_ */
    {0x0F12, 0x010C, WORD_LEN, 0},// AfitBaseVals_4__62_ */
    {0x0F12, 0x6464, WORD_LEN, 0},// AfitBaseVals_4__63_ */
    {0x0F12, 0x0F0F, WORD_LEN, 0},// AfitBaseVals_4__64_ */
    {0x0F12, 0x0307, WORD_LEN, 0},// AfitBaseVals_4__65_ */
    {0x0F12, 0x080F, WORD_LEN, 0},// AfitBaseVals_4__66_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_4__67_ */
    {0x0F12, 0x030F, WORD_LEN, 0},// AfitBaseVals_4__68_ */
    {0x0F12, 0x3208, WORD_LEN, 0},// AfitBaseVals_4__69_ */
    {0x0F12, 0x0F1E, WORD_LEN, 0},// AfitBaseVals_4__70_ */
    {0x0F12, 0x020F, WORD_LEN, 0},// AfitBaseVals_4__71_ */
    {0x0F12, 0x0103, WORD_LEN, 0},// AfitBaseVals_4__72_ */
    {0x0F12, 0x010C, WORD_LEN, 0},// AfitBaseVals_4__73_ */
    {0x0F12, 0x6464, WORD_LEN, 0},// AfitBaseVals_4__74_ */
    {0x0F12, 0x0F0F, WORD_LEN, 0},// AfitBaseVals_4__75_ */
    {0x0F12, 0x0307, WORD_LEN, 0},// AfitBaseVals_4__76_ */
    {0x0F12, 0x080F, WORD_LEN, 0},// AfitBaseVals_4__77_ */
    {0x0F12, 0x0000, WORD_LEN, 0},// AfitBaseVals_4__78_ */
    {0x0F12, 0x030F, WORD_LEN, 0},// AfitBaseVals_4__79_ */
    {0x0F12, 0x3208, WORD_LEN, 0},// AfitBaseVals_4__80_ */
    {0x0F12, 0x0F1E, WORD_LEN, 0},// AfitBaseVals_4__81_ */
    {0x0F12, 0x020F, WORD_LEN, 0},// AfitBaseVals_4__82_ */
    {0x0F12, 0x0003, WORD_LEN, 0},// AfitBaseVals_4__83_ */
    {0x0F12, 0x7F5E, WORD_LEN, 0},// ConstAfitBaseVals_0_ */
    {0x0F12, 0xFEEE, WORD_LEN, 0},// ConstAfitBaseVals_1_ */
    {0x0F12, 0xD9B7, WORD_LEN, 0},// ConstAfitBaseVals_2_ */
    {0x0F12, 0x0472, WORD_LEN, 0},// ConstAfitBaseVals_3_ */
    {0x0F12, 0x0001, WORD_LEN, 0},// ConstAfitBaseVals_4_ */

//*************************************/
// 18.JPEG Thumnail Setting           */
//*************************************/

//*************************************/
// 19.Input Size Setting              */
//*************************************/

//*********************************************************************************
// 20.Preview & Capture Configration Setting                                       
//*********************************************************************************
// Preview config[0] 640X480  15~7.5fps //
    {0x002A, 0x01BE, WORD_LEN, 0},
    {0x0F12, 0x0500, WORD_LEN, 0},	// REG_0TC_PCFG_usWidth 500:1280; 280:640
    {0x0F12, 0x03C0, WORD_LEN, 0},	// REG_0TC_PCFG_usHeight 3C0:960; 1E0:480
    {0x0F12, 0x0005, WORD_LEN, 0},	// REG_0TC_PCFG_Format 5:YUV422; 7:RAW10  
    {0x002A, 0x01C8, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},	// REG_0TC_PCFG_uClockInd 
    {0x002A, 0x01C4, WORD_LEN, 0},
    {0x0F12, 0x0042, WORD_LEN, 0},	// REG_0TC_PCFG_PVIMask 52:YUV422, 42:RAW10 
    {0x002A, 0x01D4, WORD_LEN, 0},
    {0x0F12, 0x0002, WORD_LEN, 0},	// REG_0TC_PCFG_FrRateQualityType  1b:FR(bin) 2b:Quality(no-bin) 
    {0x002A, 0x01D2, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},	// REG_0TC_PCFG_usFrTimeType  0:dynamic; 1:fixed not accurate; 2:fixed accurate 
    {0x002A, 0x01D8, WORD_LEN, 0},
    {0x0F12, 0x028A, WORD_LEN, 0},	// REG_0TC_PCFG_usMaxFrTimeMsecMult10  30fps-014D; 15fps-029A; 7.5-0535; 6.0-0682; 3.75-0A6A
    {0x002A, 0x01D6, WORD_LEN, 0},
    {0x0F12, 0x014D, WORD_LEN, 0},// REG_0TC_PCFG_usMinFrTimeMsecMult10 
    {0x002A, 0x01E8, WORD_LEN, 0},
    {0x0F12, 0x0003, WORD_LEN, 0},	// REG_0TC_PCFG_uPrevMirror 
    {0x0F12, 0x0000, WORD_LEN, 0},	// REG_0TC_PCFG_uCaptureMirror 

// Capture config[0] 1280x960  7.5fps 
    {0x002A, 0x02AE, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},	// Capture mode AE On 
    {0x002A, 0x02B0, WORD_LEN, 0},
    {0x0F12, 0x0500, WORD_LEN, 0},	// REG_0TC_CCFG_usWidth 500:1280; 280:640
    {0x0F12, 0x03C0, WORD_LEN, 0},	// REG_0TC_CCFG_usHeight 3C0:960; 1E0:480
    {0x0F12, 0x0005, WORD_LEN, 0},	// REG_0TC_CCFG_Format 5:YUV422; 7:RAW10  
    {0x002A, 0x02BA, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},	// REG_0TC_CCFG_uClockInd
    {0x002A, 0x02B6, WORD_LEN, 0},
    {0x0F12, 0x0042, WORD_LEN, 0},	// REG_0TC_CCFG_PVIMask 52:YUV422; 42:RAW10 
    {0x002A, 0x02C6, WORD_LEN, 0},
    {0x0F12, 0x0002, WORD_LEN, 0},	// REG_0TC_CCFG_FrRateQualityType  1b:FR(bin) 2b:Quality(no-bin)
    {0x002A, 0x02C4, WORD_LEN, 0},
    {0x0F12, 0x0002, WORD_LEN, 0},	// REG_0TC_CCFG_usFrTimeType  0:dynamic; 1:fixed not accurate;  2:fixed accurate 
    {0x002A, 0x02CA, WORD_LEN, 0},
    {0x0F12, 0x014D, WORD_LEN, 0},	// REG_0TC_CCFG_usMaxFrTimeMsecMult10  30fps-014D; 15fps-029A; 7.5-0535; 6.0-0682; 3.75-0A6A 
    {0x002A, 0x02C8, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},	// REG_0TC_CCFG_usMinFrTimeMsecMult10 

//*************************************/
// 21.Select Cofigration Display      */
//*************************************/
    {0x002A, 0x01A8, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},// REG_TC_GP_ActivePreviewConfig */
    {0x002A, 0x01AA, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},// REG_TC_GP_PreviewConfigChanged */
    {0x002A, 0x019E, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},// REG_TC_GP_EnablePreview */
    {0x0F12, 0x0001, WORD_LEN, 0},// REG_TC_GP_EnablePreviewChanged */

    {0x0028, 0xD000, WORD_LEN, 0},
    {0x002A, 0x1000, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 150},	// Set host interrupt 
};

static struct s5k8aay_i2c_reg_conf s5k8aay_preview_array[] = {

 #if 0
#if 0
    //
    {0x0028, 0x7000, WORD_LEN, 0},
    {0x002A, 0x01B0, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},  // 0x01B0: index number of active capture config
    {0x002A, 0x01A6, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},  // 0x01A6: Set this flag when sending a new configuration
#endif
#if 1
//preview
    {0x002A, 0x01A8, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x002A, 0x01AC, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},
    {0x002A, 0x01A6, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},
    {0x002A, 0x01AA, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},  // 0x01AA: synchronize FW with new preview configuration
    {0x002A, 0x019E, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},  // 0x019E: Enable preview output
    {0x0F12, 0x0001, WORD_LEN, 150},  // 0x01A0: Enable Preview Changed
#endif

#if 0
//capture
    {0x002A, 0x01B0, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x002A, 0x01A6, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},
    {0x002A, 0x01B2, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},  // 0x01B2: Synchronize FW with new capture configuration
    {0x002A, 0x01A2, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},  // 0x01A2: Enable capture
    {0x0F12, 0x0001, WORD_LEN, 150},  // 0x01A4: Enable capture changed
#endif
#endif
};

static struct s5k8aay_i2c_reg_conf s5k8aay_snapshot_array[] = {
#if 0
    //
    {0x0028, 0x7000, WORD_LEN, 0},
    {0x002A, 0x01B0, WORD_LEN, 0},
    {0x0F12, 0x0000, WORD_LEN, 0},  // 0x01B0: index number of active capture config
    {0x002A, 0x01A6, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},  // 0x01A6: Set this flag when sending a new configuration
    {0x002A, 0x01B2, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},  // 0x01B2: Synchronize FW with new capture configuration
    {0x002A, 0x01A2, WORD_LEN, 0},
    {0x0F12, 0x0001, WORD_LEN, 0},  // 0x01A2: Enable capture
    {0x0F12, 0x0001, WORD_LEN, 150},  // 0x01A4: Enable capture changed
#endif
};


struct s5k8aay_reg s5k8aay_regs = {
    .reg_init = &s5k8aay_init_array[0],

    .reg_init_size = ARRAY_SIZE(s5k8aay_init_array),

    .reg_preview = &s5k8aay_preview_array[0],

    .reg_preview_size = ARRAY_SIZE(s5k8aay_preview_array),

    .reg_snapshot = &s5k8aay_snapshot_array[0],

    .reg_snapshot_size = ARRAY_SIZE(s5k8aay_snapshot_array),
};


