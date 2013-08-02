

#ifndef _SSL3252_H_

#define _SSL3252_H_

/*deleted by Liu Jinshui 20120220 start*/
#ifndef CONFIG_OPPO_MODIFY
struct ssl3252_platform_data {

	int flash_en;

	int torch_tx1;

	int led_tx2;

	void (*dev_startup)(void);

	void (*dev_shutdown)(void);

};
#endif
/*deleted by Liu Jinshui 20120220 end*/

int oppo_led_control(unsigned state);

//int ssl3252_dev_init(int power_on);

#endif