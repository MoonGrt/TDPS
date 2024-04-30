#include <rtdevice.h>
#include <stdlib.h>

#define MOTOR_UART_NAME "uart3"

#define UP 0x47    //'G'
#define DOWN 0x4b  //'K'
#define LEFT 0x48  //'H'
#define RIGHT 0x4a //'J'
#define STOP 0x49  //'I'

#define PWM1_DEV_NAME "pwm4"  /* PWM设备名称 */
#define PWM1_DEV_CHANNEL 1    /* PWM通道 */
#define PWM2_DEV_NAME "pwm4"  /* PWM设备名称 */
#define PWM2_DEV_CHANNEL 2    /* PWM通道 */
#define PWM3_DEV_NAME "pwm4"  /* PWM设备名称 */
#define PWM3_DEV_CHANNEL 3    /* PWM通道 */
#define PWM4_DEV_NAME "pwm4"  /* PWM设备名称 */
#define PWM4_DEV_CHANNEL 4    /* PWM通道 */
struct rt_device_pwm *pwm1_dev, *pwm2_dev, *pwm3_dev, *pwm4_dev; /* PWM设备句柄 */

rt_thread_t motor_thread = NULL;
static rt_device_t motor_uart;
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

static void motor_th(void *parameter)
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

        while (rt_device_read(motor_uart, 0, &direction, 1) != 1)
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

int motor_init(void)
{
    /* step1：查找串口设备 */
    motor_uart = rt_device_find(MOTOR_UART_NAME);
    /* step2：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    rt_device_control(motor_uart, RT_DEVICE_CTRL_CONFIG, (void *)&uart_config);

    /* step3：打开串口设备。以中断接收及轮询发送模式打开串口设备  中断接收数据 ==>> 之后可改为dma*/
    rt_device_open(motor_uart, RT_DEVICE_FLAG_INT_RX);
    rt_device_set_rx_indicate(motor_uart, rx_callback);

    sem = rt_sem_create("rx_sem", 0, RT_IPC_FLAG_FIFO);
    if (sem == RT_NULL)
    {
        rt_kprintf("rt_sem_create failed...\n");
        return -1;
    }

    motor_thread = rt_thread_create("motor_th", motor_th, RT_NULL, 512, 20, 5);
    if (motor_thread != RT_NULL)   /* 如果获得线程控制块，启动这个线程 */
        rt_thread_startup(motor_thread); // 启动线程
    else
        return RT_ERROR;

    rt_kprintf("motor init success\n");
    return RT_EOK;
}
MSH_CMD_EXPORT(motor_init, motor drive)

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
MSH_CMD_EXPORT(pwm_test, pwm test)
