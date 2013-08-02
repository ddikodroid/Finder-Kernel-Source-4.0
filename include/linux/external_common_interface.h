/*
 * Copyright (C) 2011-2012 OPPO, Inc.
 * Author: zhangyue <zhangyue@oppo.com>
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
#ifndef __EXTERNAL_COMMON_INTERFACE_H_
#define __EXTERNAL_COMMON_INTERFACE_H_

/* added by zhangyue on 2011-11-01 */
#ifdef CONFIG_OPPO_MODIFY
//added by zhangyue for adding the functionality:enable the hpd feature automatically. on 2011-11-01
//you can search this marcro to know all the code added for this issue.
#define zy_auto_hpd 1
#endif

#ifdef CONFIG_OPPO_MODIFY
//added by zhangyue for adding the ability of enable mhl irq from usersapce, you can search this marcro to know all the code added
#define zy_usrspace_enalbe_mhl_irq 1
#endif
#ifdef CONFIG_OPPO_MODIFY
#ifdef zy_auto_hpd
//added by zhangyue on 2011-11-01
int external_common_enable_hpd_feature(bool on);
#endif
#endif
#ifdef zy_usrspace_enalbe_mhl_irq
bool read_mhl_irq_Init_state(void);
int write_mhl_irq_Init_state(void);
#endif
/* end by zhangyue 2011-11-01 */

//added by zhangyue on 2011-12-14 for adding irq depth and state reading capability in user space start
#ifdef CONFIG_OPPO_MODIFY
//added by zhangyue on 2012-01-10 for camera and mhl coexist problems start
/******************************************************
*func: notify the camrea state to hdmi moudle to solve the camera and hdmi coexits problems
*@on[input]: indicate the camera state, true means the camera is open, and false means the camrea is closed
*return value: 0 -- indicate the call succeed, otherwise failed.*/
int external_common_camera(bool on);
bool is_mhl_connected(void);
//added by zhangyue on 2012-01-10 for camera and mhl coexist problems end
#endif
#endif /* __EXTERNAL_COMMON_INTERFACE_H_ */
