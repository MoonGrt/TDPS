#include <rtdevice.h>
#include <stdio.h>

#define PULSE1_ENCODER_DEV_NAME "pulse2"  // 脉冲编码器名称
#define PULSE2_ENCODER_DEV_NAME "pulse3"  // 脉冲编码器名称

#define PERIOD_TIME  10      // 采样周期 ms
#define WHEEL_DISTANCE 124   // 124mm
#define PULSE_PER_REVOLUTION 1040   // 每转的脉冲数
#define GEAR_RATIO 20               // 减速比
#define WHEEL_DIAMETER_MM 48        // 轮胎直径（mm）
#define WHEEL_CIRCUMFERENCE_MM (WHEEL_DIAMETER_MM * 3.1415926535) // 轮胎周长(mm)
#define PULSE_COUNT_THRESHOLD 1040/2
#define DISTANCE_PER_PULSE_MM (WHEEL_CIRCUMFERENCE_MM / PULSE_PER_REVOLUTION) // 计算每脉冲对应的位移（mm）

rt_device_t pulse_encoder_r = RT_NULL, pulse_encoder_l = RT_NULL;   // 脉冲编码器设备句柄
//double linear_sp = 0.0, angular_sp = 0.0;

rt_thread_t encoder_thread = NULL;

static void encoder_th(void *parameter)
{
    rt_err_t ret = RT_EOK;
    rt_int32_t count_r = 0, count_l = 0;
    double disp_r = 0.0, disp_l = 0.0, sp_r = 0.0, sp_l = 0.0;
    
    // 查找脉冲编码器设备
    pulse_encoder_l = rt_device_find(PULSE2_ENCODER_DEV_NAME);
    if (pulse_encoder_l == RT_NULL)
    {
        rt_kprintf("pulse encoder sample run failed! can't find %s device!", PULSE2_ENCODER_DEV_NAME);
        return;
    }
    // 以只读方式打开设备
    ret = rt_device_open(pulse_encoder_l, RT_DEVICE_OFLAG_RDONLY);
    if (ret != RT_EOK)
    {
        rt_kprintf("open %s device failed!\n", PULSE2_ENCODER_DEV_NAME);
        return;
    }

    // 查找脉冲编码器设备
    pulse_encoder_r = rt_device_find(PULSE1_ENCODER_DEV_NAME);
    if (pulse_encoder_r == RT_NULL)
    {
        rt_kprintf("pulse encoder sample run failed! can't find %s device!", PULSE1_ENCODER_DEV_NAME);
        return;
    }
    // 以只读方式打开设备
    ret = rt_device_open(pulse_encoder_r, RT_DEVICE_OFLAG_RDONLY);
    if (ret != RT_EOK)
    {
        rt_kprintf("open %s device failed!\n", PULSE1_ENCODER_DEV_NAME);
        return;
    }

    while(1)
    {
        rt_thread_mdelay(100);

        // 读取脉冲编码器计数值
        rt_device_read(pulse_encoder_l, 0, &count_l, 1);
        rt_device_read(pulse_encoder_r, 0, &count_r, 1);

        // 清空脉冲编码器计数值
        rt_device_control(pulse_encoder_l, PULSE_ENCODER_CMD_CLEAR_COUNT, RT_NULL);
        rt_device_control(pulse_encoder_r, PULSE_ENCODER_CMD_CLEAR_COUNT, RT_NULL);

        // 如果脉冲计数值异常-则丢弃数据
        if (count_l <= -PULSE_COUNT_THRESHOLD || count_l >= PULSE_COUNT_THRESHOLD)
            continue;
        if (count_r <= -PULSE_COUNT_THRESHOLD || count_r >= PULSE_COUNT_THRESHOLD)
            continue;

        // 计算速度
        disp_l = count_l * DISTANCE_PER_PULSE_MM;
        disp_r = count_r * DISTANCE_PER_PULSE_MM;
        sp_l = disp_l / PERIOD_TIME;
        sp_r = disp_r / PERIOD_TIME;
//        linear_sp = (sp_r + sp_l) / 2.0;
//        angular_sp = (sp_r - sp_l) / WHEEL_DISTANCE * 1000;

        // 打印数据
        rt_kprintf("count: %d %d\n", count_l, count_r);
//        printf("sp: %.2fm %.2fm\n", sp_l, sp_r);
    }

    rt_device_close(pulse_encoder_l);
    rt_device_close(pulse_encoder_r);
    return;
}

int encoder_init(void)
{
    encoder_thread = rt_thread_create("encoder_th", encoder_th, RT_NULL, 512, 9, 5);
    if (encoder_thread != RT_NULL)   /* 如果获得线程控制块，启动这个线程 */
        rt_thread_startup(encoder_thread); // 启动线程
    else
        return RT_ERROR;

    rt_kprintf("encoder init success\n");
    return RT_EOK;
}
MSH_CMD_EXPORT(encoder_init, encoder driver);
