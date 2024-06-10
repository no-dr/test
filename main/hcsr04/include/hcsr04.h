#ifndef _HCSR04_H_
#define _HCSR04_H_

#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "driver/timer.h"
#include "hal/timer_types.h"

#define TIMER_DIVIDER         (80)  /* Hardware timer clock divider 80 */  
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  /* convert counter value to seconds, 80M hz */ 

#define TIRG_PIN GPIO_NUM_16
#define ECHO_PIN GPIO_NUM_17

void hcsr04_gpio_init(void);

#ifdef __cplusplus
extern "C"
{

void hcsr04_timer_init(void);
void hcsr04_delay_us(uint16_t us);

}
#endif /* __cplusplus */

#endif /* _HCSR04_H_ */