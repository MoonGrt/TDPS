#include "board.h"
#include "sensor_hc_sr04.h"

#define HWTIMER_DEV_NAME "timer1"
#define SR04_NAME "pr_sr04"

#define SR04_TRIG_PIN GET_PIN(B, 4)
#define SR04_ECHO_PIN GET_PIN(B, 5)

rt_thread_t sr04_thread = NULL;
extern rt_device_t commu_uart;

uint16_t distance, timestamp;

void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  /* System interrupt init*/

  /** NOJTAG: JTAG-DP Disabled and SW-DP Enabled
  */
  __HAL_AFIO_REMAP_SWJ_NOJTAG();

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

static void sr04_th(void *parameter)
{
    rt_device_t dev = RT_NULL;
    struct rt_sensor_data sensor_data;
    rt_size_t res;

    dev = rt_device_find(SR04_NAME);
    if (dev == RT_NULL)
    {
        rt_kprintf("Can't find device:%s\n", SR04_NAME);
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
        {
            distance = (uint16_t)sensor_data.data.proximity;
            timestamp = (uint16_t)sensor_data.timestamp;
            rt_kprintf("distance:%3d.%dcm, timestamp:%5d\n", distance / 10, distance % 10, sensor_data.timestamp);
            rt_device_write(commu_uart, 0, &distance, 2);
        }
        rt_thread_mdelay(500);
    }
}

int sr04_init(void)
{
    /* B4引脚重定义  */
    HAL_MspInit();

    sr04_thread = rt_thread_create("sr04_th", sr04_th, RT_NULL, 512, 24, 5);
    if (sr04_thread != RT_NULL)   /* 如果获得线程控制块，启动这个线程 */
        rt_thread_startup(sr04_thread); // 启动线程
    else
        return RT_ERROR;

    rt_kprintf("sr04 init success\n");
    return RT_EOK;
}
MSH_CMD_EXPORT(sr04_init, sr04 drive)

static void sr04_test(int argc, char** argv)
{
    rt_device_t dev = RT_NULL;
    struct rt_sensor_data sensor_data;
    rt_size_t res;

    dev = rt_device_find(SR04_NAME);
    if (dev == RT_NULL)
    {
        rt_kprintf("Can't find device:%s\n", SR04_NAME);
        return;
    }

    if (rt_device_open(dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("open device failed!\n");
        return;
    }
    rt_device_control(dev, RT_SENSOR_CTRL_SET_ODR, (void *)100);

    res = rt_device_read(dev, 0, &sensor_data, 1);
    if (res != 1)
    {
        rt_kprintf("read data failed!size is %d\n", res);
        rt_device_close(dev);
        return;
    }
    else
    {
        distance = sensor_data.data.proximity;
        timestamp = sensor_data.timestamp;
        rt_kprintf("distance:%3d.%dcm, timestamp:%5d\n", distance / 10, distance % 10, sensor_data.timestamp);
    }
}
MSH_CMD_EXPORT(sr04_test, sr04 test)

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
