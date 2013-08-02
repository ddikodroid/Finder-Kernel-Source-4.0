/*
 * Definitions for akm8975 compass chip.
 */
#ifndef TSL2773_H
#define TSL2773_H

#include <linux/ioctl.h>


#define AKMIO                           0xA1


/* IOCTLs for AKM library */
#define ECS_IOCTL_WRITE                 _IOW(AKMIO, 0x02, char[5])
#define ECS_IOCTL_READ                  _IOWR(AKMIO, 0x03, char[5])
#define ECS_IOCTL_GETDATA               _IOR(AKMIO, 0x08, char[RBUFF_SIZE])
#define ECS_IOCTL_SET_YPR               _IOW(AKMIO, 0x0C, short[12])
#define ECS_IOCTL_GET_OPEN_STATUS       _IOR(AKMIO, 0x0D, int)
#define ECS_IOCTL_GET_CLOSE_STATUS      _IOR(AKMIO, 0x0E, int)
#define ECS_IOCTL_GET_DELAY             _IOR(AKMIO, 0x30, short)

/* IOCTLs for APPs */
#define ECS_IOCTL_APP_SET_MFLAG		_IOW(AKMIO, 0x11, short)
#define ECS_IOCTL_APP_GET_MFLAG		_IOW(AKMIO, 0x12, short)
#define ECS_IOCTL_APP_SET_AFLAG		_IOW(AKMIO, 0x13, short)
#define ECS_IOCTL_APP_GET_AFLAG		_IOR(AKMIO, 0x14, short)
#define ECS_IOCTL_APP_SET_DELAY		_IOW(AKMIO, 0x18, short)
#define ECS_IOCTL_APP_GET_DELAY		ECS_IOCTL_GET_DELAY
/* Set raw magnetic vector flag */
#define ECS_IOCTL_APP_SET_MVFLAG	_IOW(AKMIO, 0x19, short)
/* Get raw magnetic vector flag */
#define ECS_IOCTL_APP_GET_MVFLAG	_IOR(AKMIO, 0x1A, short)
#define ECS_IOCTL_APP_SET_TFLAG         _IOR(AKMIO, 0x15, short)

#define ECS_IOCTL_APP_SET_ALSFLAG		_IOW(AKMIO, 0x20, short)	/*ambient sensor*/
#define ECS_IOCTL_APP_GET_ALSFLAG		_IOR(AKMIO, 0x21, short)
#define ECS_IOCTL_APP_SET_PFLAG		_IOW(AKMIO, 0x22, short)	/*proximity sensor*/
#define ECS_IOCTL_APP_GET_PFLAG		_IOR(AKMIO, 0x23, short)
#define ECS_IOCTL_APP_SET_AFLAG_AK		_IOW(AKMIO, 0x24, short)
#define ECS_IOCTL_APP_GET_AFLAG_AK		_IOR(AKMIO, 0x25, short)
#define ECS_IOCTL_APP_CALIBRATE		_IOR(AKMIO, 0x26, short[3])


struct light_sensor_platform_data {
	int (*power_on) (void);
	int (*power_off) (void);
};



#endif

