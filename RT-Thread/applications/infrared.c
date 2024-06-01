#include <rtdevice.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INFRARED_NAME "uart2"

rt_thread_t infrared_thread = NULL;
rt_device_t infrared_uart;
rt_sem_t sem_infrared;
int8_t stop_flag = 0;

struct serial_configure infrared_uart_config = {
    BAUD_RATE_9600,   /* 115200 bits/s */
    DATA_BITS_8,        /* 8 databits */
    STOP_BITS_1,        /* 1 stopbit */
    PARITY_NONE,        /* No parity  */
    BIT_ORDER_LSB,      /* LSB first sent */
    NRZ_NORMAL,         /* Normal mode */
    RT_SERIAL_RB_BUFSZ, /* Buffer size */
    0};

// 接收回调函数
uint8_t revc_data;
rt_err_t infrared_rx_callback(rt_device_t dev, rt_size_t size)
{
    rt_device_read(infrared_uart, 0, &revc_data, 1);
    if (revc_data == 0xF1)
        rt_kprintf("send\n");
    if (revc_data == 0x22)
    {
        stop_flag = 0;
        rt_kprintf("go go go\n");
    }
    return RT_EOK;
}

uint8_t send_code[5] = {0xfa, 0xf1, 0x22, 0x33, 0x44};
void infrared_th(void *parameter)
{
    /* step1：查找串口设备 */
    infrared_uart = rt_device_find(INFRARED_NAME);
    /* step2：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    rt_device_control(infrared_uart, RT_DEVICE_CTRL_CONFIG, (void *)&infrared_uart_config);
    /* step3：打开串口设备。以中断接收及轮询发送模式打开串口设备  中断接收数据 ==>> 之后可改为dma*/
    rt_device_open(infrared_uart, RT_DEVICE_FLAG_INT_RX);
    rt_device_set_rx_indicate(infrared_uart, infrared_rx_callback);

    rt_sem_take(sem_infrared, RT_WAITING_FOREVER);
    while (1)
    {
        if (stop_flag)
        {
            rt_kprintf("let me go\n");
            rt_device_write(infrared_uart, 0, &send_code, 5);
            rt_thread_mdelay(5000);
        }
//        if()
    }
}

int infrared_init(void)
{
    sem_infrared = rt_sem_create("sem_infrared", 0, RT_IPC_FLAG_FIFO);
    if (sem_infrared == RT_NULL)
    {
        rt_kprintf("sem_infrared create failed...\n");
        return -1;
    }

    infrared_thread = rt_thread_create("infrared_th", infrared_th, RT_NULL, 512, 21, 5);
    if (infrared_thread != RT_NULL)   /* 如果获得线程控制块，启动这个线程 */
        rt_thread_startup(infrared_thread); // 启动线程
    else
        return RT_ERROR;

    rt_kprintf("infrared init success\n");
    return RT_EOK;
}
MSH_CMD_EXPORT(infrared_init, infrared ray)

static void infrared_test(int argc, char** argv)
{
    rt_device_write(infrared_uart, 0, &send_code, 5);
}
MSH_CMD_EXPORT(infrared_test, infrared test)

