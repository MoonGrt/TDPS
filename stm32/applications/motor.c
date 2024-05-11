#include <rtdevice.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MOTOR_UART_NAME "uart3"

#define PWM1_DEV_NAME "pwm4"  /* PWM设备名称 */
#define PWM1_DEV_CHANNEL 1    /* PWM通道 */
#define PWM2_DEV_NAME "pwm4"  /* PWM设备名称 */
#define PWM2_DEV_CHANNEL 2    /* PWM通道 */
#define PWM3_DEV_NAME "pwm4"  /* PWM设备名称 */
#define PWM3_DEV_CHANNEL 3    /* PWM通道 */
#define PWM4_DEV_NAME "pwm4"  /* PWM设备名称 */
#define PWM4_DEV_CHANNEL 4    /* PWM通道 */
struct rt_device_pwm *pwm1_dev, *pwm2_dev, *pwm3_dev, *pwm4_dev; /* PWM设备句柄 */

rt_thread_t motor_ctrl_thread = NULL;
rt_sem_t sem_motorctrl; // 不加static让其他文件能extern

uint8_t sp_data[4];
float sp_r, sp_l;
void motor_ctrl_th(void *parameter)
{
    rt_uint32_t period = 1000000, pulse1, pulse2, pulse3, pulse4; /* 1KHz周期为1ms，这里单位是纳秒ns，1ms等于10的6次方纳秒ns*/
    pulse1 = pulse2 = pulse3 = pulse4 = 0;
//    pulse1 = 600000;
//    pulse2 = 700000;
//    pulse3 = 800000;
//    pulse4 = 900000;

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

    while(1)
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

        rt_sem_take(sem_motorctrl, RT_WAITING_FOREVER);

        sp_l = (float)sp_data[0] + (float)sp_data[1] / 100.0; // 将整数部分和小数部分合并
        sp_r = (float)sp_data[2] + (float)sp_data[3] / 100.0; // 将整数部分和小数部分合并
//        rt_kprintf("%d %d %d %d\n", sp_data[0], sp_data[1], sp_data[2], sp_data[3]);
        rt_kprintf("%d %d\n", (int)(sp_l * 10000), (int)(sp_r * 10000));

//        pulse1 = (int)(sp_r * 10000);  // 有问题
        pulse2 = (int)(sp_r * 10000);
        pulse3 = (int)(sp_l * 10000);
//        pulse4 = (int)(sp_l * 10000);  // 有问题
    }
}

int motor_init(void)
{
    sem_motorctrl = rt_sem_create("sem_motorctrl", 0, RT_IPC_FLAG_FIFO);
    if (sem_motorctrl == RT_NULL)
    {
        rt_kprintf("sem_motorctrl create failed...\n");
        return -1;
    }

    motor_ctrl_thread = rt_thread_create("motor_ctrl_th", motor_ctrl_th, RT_NULL, 512, 20, 5);
    if (motor_ctrl_thread != RT_NULL)   /* 如果获得线程控制块，启动这个线程 */
        rt_thread_startup(motor_ctrl_thread); // 启动线程
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
    pulse1 = pulse3 = pulse;
    pulse2 = pulse4 = 0;

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
