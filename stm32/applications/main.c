#include <rtthread.h>
#include <rtdevice.h>
#include <stdlib.h>
#include "drv_common.h"
#include "board.h"
#include "sensor.h"
#include "sensor_hc_sr04.h"

#define UP 0x47    //'G'
#define DOWN 0x4b  //'K'
#define LEFT 0x48  //'H'
#define RIGHT 0x4a //'J'
#define STOP 0x49  //'I'

//#define SR04_TRIG_PIN GET_PIN(A, 4)
//#define SR04_ECHO_PIN GET_PIN(A, 5)
#define SR04_TRIG_PIN GET_PIN(B, 4)
#define SR04_ECHO_PIN GET_PIN(B, 5)

/* defined the LED0 pin: PC13 */
#define LED_PIN GET_PIN(C, 13)

#define HWTIMER_DEV_NAME "timer1"

#define PWM1_DEV_NAME "pwm4"  /* PWM设备名称 */
#define PWM1_DEV_CHANNEL 1    /* PWM通道 */
#define PWM2_DEV_NAME "pwm4"  /* PWM设备名称 */
#define PWM2_DEV_CHANNEL 2    /* PWM通道 */
#define PWM3_DEV_NAME "pwm4"  /* PWM设备名称 */
#define PWM3_DEV_CHANNEL 3    /* PWM通道 */
#define PWM4_DEV_NAME "pwm4"  /* PWM设备名称 */
#define PWM4_DEV_CHANNEL 4    /* PWM通道 */
struct rt_device_pwm *pwm1_dev, *pwm2_dev, *pwm3_dev, *pwm4_dev; /* PWM设备句柄 */

rt_thread_t motor = NULL, sr04_thread = NULL, led_thread = NULL;
static rt_device_t uart1, uart2, uart3;
static rt_sem_t sem;

struct serial_configure uart_config = {
    BAUD_RATE_115200,   /* 115200 bits/s */
    DATA_BITS_8,        /* 8 databits */
    STOP_BITS_1,        /* 1 stopbit */
    PARITY_NONE,        /* No parity  */
    BIT_ORDER_LSB,      /* LSB first sent */
    NRZ_NORMAL,         /* Normal mode */
    RT_SERIAL_RB_BUFSZ, /* Buffer size */
    0};

// 接收回调函数
rt_err_t rx_callback(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(sem);
    return RT_EOK;
}

static void motor_ctrl(void *parameter)
{
    rt_uint32_t period, pulse1, pulse2, pulse3, pulse4;
    char direction;
    period = 1000000; /* 1KHz周期为1ms,这里单位是纳秒ns，1ms等于10的6次方纳秒ns*/
//    pulse1 = pulse2 = pulse3 = pulse4 = 0;
    pulse1 = 500000;
    pulse2 = 0;
    pulse3 = 700000;
    pulse4 = 0;

    /* 查找设备 */
    pwm1_dev = (struct rt_device_pwm *)rt_device_find(PWM1_DEV_NAME);
    if (pwm1_dev == RT_NULL)
        rt_kprintf("can't find %s device!\n", PWM1_DEV_NAME);
    pwm2_dev = (struct rt_device_pwm *)rt_device_find(PWM2_DEV_NAME);
    if (pwm2_dev == RT_NULL)
        rt_kprintf("can't find %s device!\n", PWM2_DEV_NAME);
    pwm3_dev = (struct rt_device_pwm *)rt_device_find(PWM3_DEV_NAME);
    if (pwm3_dev == RT_NULL)
        rt_kprintf("can't find %s device!\n", PWM3_DEV_NAME);
    pwm4_dev = (struct rt_device_pwm *)rt_device_find(PWM4_DEV_NAME);
    if (pwm4_dev == RT_NULL)
        rt_kprintf("can't find %s device!\n", PWM4_DEV_NAME);

    while (1)
    {
        /* 设置PWM周期和脉冲宽度 */
        rt_pwm_set(pwm1_dev, PWM1_DEV_CHANNEL, period, pulse1);
        rt_pwm_set(pwm2_dev, PWM2_DEV_CHANNEL, period, pulse2);
        rt_pwm_set(pwm3_dev, PWM3_DEV_CHANNEL, period, pulse3);
        rt_pwm_set(pwm4_dev, PWM4_DEV_CHANNEL, period, pulse4);
        /* 使能设备 */
        rt_pwm_enable(pwm1_dev, PWM1_DEV_CHANNEL);
        rt_pwm_enable(pwm2_dev, PWM2_DEV_CHANNEL);
        rt_pwm_enable(pwm3_dev, PWM3_DEV_CHANNEL);
        rt_pwm_enable(pwm4_dev, PWM4_DEV_CHANNEL);

        while (rt_device_read(uart1, 0, &direction, 1) != 1)
            rt_sem_take(sem, RT_WAITING_FOREVER);
        rt_kprintf("%c", direction);

        if (direction == UP)
        {
            pulse1 = pulse3 = 0;
            pulse2 = pulse4 = 300000;
        }
        if (direction == DOWN)
        {
            pulse1 = pulse3 = 300000;
            pulse2 = pulse4 = 0;
        }
        if (direction == LEFT)
        {
            pulse2 = pulse3 = 0;
            pulse1 = pulse4 = 300000;
        }
        if (direction == RIGHT)
        {
            pulse1 = pulse4 = 0;
            pulse2 = pulse3 = 300000;
        }
        if (direction == STOP)
            pulse1 = pulse2 = pulse3 = pulse4 = 0;
    }
}

static void sr04_th(void *parameter)
{
    rt_device_t dev = RT_NULL;
    struct rt_sensor_data sensor_data;
    rt_size_t res;

    dev = rt_device_find(parameter);
    if (dev == RT_NULL)
    {
        rt_kprintf("Can't find device:%s\n", parameter);
        return;
    }

    if (rt_device_open(dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("open device failed!\n");
        return;
    }
    rt_device_control(dev, RT_SENSOR_CTRL_SET_ODR, (void *)100);

    while (1)
    {
        res = rt_device_read(dev, 0, &sensor_data, 1);
        if (res != 1)
        {
            rt_kprintf("read data failed!size is %d\n", res);
            rt_device_close(dev);
            return;
        }
        else
            rt_kprintf("distance:%3d.%dcm, timestamp:%5d\n", sensor_data.data.proximity / 10, sensor_data.data.proximity % 10, sensor_data.timestamp);
        rt_thread_mdelay(1000);
    }
}

static void led_th(void *parameter)
{
    /* set LED0 pin mode to output */
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    while (1)
    {
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}

int main(void)
{
    HAL_MspInit();
//    /* step1：查找串口设备 */
//    uart1 = rt_device_find("uart2");
//
//    /* step2：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
//    rt_device_control(uart1, RT_DEVICE_CTRL_CONFIG, (void *)&uart_config);
//
//    /* step3：打开串口设备。以中断接收及轮询发送模式打开串口设备           中断接收数据 ==>> 之后可改为dma*/
//    rt_device_open(uart1, RT_DEVICE_FLAG_INT_RX);
//    rt_device_set_rx_indicate(uart1, rx_callback);

//    /* step1：查找串口设备 */
//    uart2 = rt_device_find("uart3");
//
//    /* step2：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
//    rt_device_control(uart2, RT_DEVICE_CTRL_CONFIG, (void *)&uart_config);
//
//    /* step3：打开串口设备。以中断接收及轮询发送模式打开串口设备           中断接收数据 ==>> 之后可改为dma*/
//    rt_device_open(uart2, RT_DEVICE_FLAG_INT_RX);
//    rt_device_set_rx_indicate(uart2, rx_callback);

    sem = rt_sem_create("rx_sem", 0, RT_IPC_FLAG_FIFO);
    if (sem == RT_NULL)
    {
        rt_kprintf("rt_sem_create failed...\n");
        return -1;
    }

    motor = rt_thread_create("motor_ctrl", motor_ctrl, RT_NULL, 512, 25, 5);
    if (motor != RT_NULL) /* 如果获得线程控制块，启动这个线程 */
        rt_thread_startup(motor); // 启动线程1

//    sr04_thread = rt_thread_create("sr04", sr04_th, "pr_sr04", 512, 24, 5);
//    if (sr04_thread != RT_NULL)
//        rt_thread_startup(sr04_thread);

    led_thread = rt_thread_create("led", led_th, RT_NULL, 512, 24, 5);
    if (led_thread != RT_NULL)
        rt_thread_startup(led_thread);

    return RT_EOK;
}

int rt_hw_sr04_port(void)
{
    struct rt_sensor_config cfg;
    rt_base_t pins[2] = {SR04_TRIG_PIN, SR04_ECHO_PIN};

    cfg.intf.dev_name = HWTIMER_DEV_NAME;
    cfg.intf.user_data = (void *)pins;
    rt_hw_sr04_init("sr04", &cfg);

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_sr04_port);

static void pwm_test(int argc, char** argv)
{
    rt_uint32_t pulse;
    if (argc != 2)
        pulse = 900000;
    else
        pulse = atol(argv[1]);

    rt_uint32_t period, pulse1, pulse2, pulse3, pulse4;
    period = 1000000; /* 1KHz周期为1ms，这里单位是纳秒ns，1ms等于10的6次方纳秒ns*/
    pulse1 = pulse2 = pulse;
    pulse3 = pulse4 = 0;

    /* 设置PWM周期和脉冲宽度 */
    rt_pwm_set(pwm1_dev, PWM1_DEV_CHANNEL, period, pulse1);
    rt_pwm_set(pwm2_dev, PWM2_DEV_CHANNEL, period, pulse2);
    rt_pwm_set(pwm3_dev, PWM3_DEV_CHANNEL, period, pulse3);
    rt_pwm_set(pwm4_dev, PWM4_DEV_CHANNEL, period, pulse4);
    /* 使能设备 */
    rt_pwm_enable(pwm1_dev, PWM1_DEV_CHANNEL);
    rt_pwm_enable(pwm2_dev, PWM2_DEV_CHANNEL);
    rt_pwm_enable(pwm3_dev, PWM3_DEV_CHANNEL);
    rt_pwm_enable(pwm4_dev, PWM4_DEV_CHANNEL);

}
MSH_CMD_EXPORT(pwm_test, pwm_test)
