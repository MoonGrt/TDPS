#include <rtdevice.h>
#include "board.h"

/* defined the LED0 pin: PC13 */
#define LED_PIN GET_PIN(C, 13)

//static rt_device_t uart1, uart2, uart3;

int main(void)
{
    motor_init();
    commu_init();
    sr04_init();
//    encoder_init();

    /* set LED0 pin mode to output */
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    while(1)
    {
        rt_pin_write(LED_PIN, PIN_LOW);
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
    }

    return RT_EOK;
}
