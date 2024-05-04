#include <rtdevice.h>
#include "board.h"

/* defined the LED0 pin: PC13 */
#define LED_PIN GET_PIN(C, 13)

//static rt_device_t uart1, uart2, uart3;

int main(void)
{
//    /* step1：查找串口设备 */
//    uart1 = rt_device_find("uart1");
//
//    /* step2：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
//    rt_device_control(uart1, RT_DEVICE_CTRL_CONFIG, (void *)&uart_config);
//
//    /* step3：打开串口设备。以中断接收及轮询发送模式打开串口设备           中断接收数据 ==>> 之后可改为dma*/
//    rt_device_open(uart1, RT_DEVICE_FLAG_INT_RX);
//    rt_device_set_rx_indicate(uart1, rx_callback);

    motor_init();
//    sr04_init();
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
