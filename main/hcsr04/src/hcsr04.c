#include "hcsr04.h"

void hcsr04_gpio_init(void)
{
    gpio_pad_select_gpio(TIRG_PIN); 
    gpio_pad_select_gpio(ECHO_PIN);
    gpio_set_direction(TIRG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);
}

//set 80m hz /80 ,1m hz tick 
void hcsr04_timer_init(void)
{
    /* Select and initialize basic parameters of the timer */
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_DIS,
        .auto_reload = TIMER_AUTORELOAD_DIS,
    }; // default clock source is APB

    timer_init(TIMER_GROUP_0, TIMER_0, &config);
    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
    timer_start(TIMER_GROUP_0, TIMER_0);
}
 
void hcsr04_delay_us(uint16_t us)
{
    uint64_t timer_counter_value = 0;
    uint64_t timer_counter_update = 0;
    //uint32_t delay_ccount = 200 * us;
    
    timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &timer_counter_value);
    timer_counter_update = timer_counter_value + (us << 2);
    do {
        timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &timer_counter_value);
    } while (timer_counter_value < timer_counter_update);
}