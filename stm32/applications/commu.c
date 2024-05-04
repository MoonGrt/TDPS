#include <rtdevice.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MOTOR_UART_NAME "uart3"

rt_thread_t uart_decoder_thread = NULL;
rt_device_t commu_uart;
static rt_sem_t sem_decoder;
extern rt_sem_t sem_motorctrl;

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
    rt_sem_release(sem_decoder);
    return RT_EOK;
}

uint8_t Rx_buffer[4];
extern uint8_t sp_data[4];
void uart_decoder_th(void *parameter)
{
    uint8_t buffer;
    static uint8_t RecCmd_Step = 0, index = 0;

    /* step1：查找串口设备 */
    commu_uart = rt_device_find(MOTOR_UART_NAME);
    /* step2：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    rt_device_control(commu_uart, RT_DEVICE_CTRL_CONFIG, (void *)&uart_config);
    /* step3：打开串口设备。以中断接收及轮询发送模式打开串口设备  中断接收数据 ==>> 之后可改为dma*/
    rt_device_open(commu_uart, RT_DEVICE_FLAG_INT_RX);
    rt_device_set_rx_indicate(commu_uart, rx_callback);

    while (1)
    {
        while (rt_device_read(commu_uart, 0, &buffer, 1) != 1)
            rt_sem_take(sem_decoder, RT_WAITING_FOREVER);

        switch (RecCmd_Step)
        {
            case 0:
                if (buffer == 0xAA)
                    RecCmd_Step = 1;
                break;
            case 1:
                if (buffer == 0x55)
                    RecCmd_Step = 2;
                else
                    RecCmd_Step = 0;
                break;
            case 2:
                Rx_buffer[index] = buffer;
                index++;
                if (index == 4)
                {
//                    uint16_t distance = 1234;
//                    rt_device_write(commu_uart, 0, &distance, 2);
                    memcpy(sp_data, Rx_buffer, 4);
                    rt_sem_release(sem_motorctrl);
                    RecCmd_Step = index = 0;
                }
                break;
            default:
                break;
        }
    }
}

int commu_init(void)
{
    sem_decoder = rt_sem_create("sem_decoder", 0, RT_IPC_FLAG_FIFO);
    if (sem_decoder == RT_NULL)
    {
        rt_kprintf("sem_uart create failed...\n");
        return -1;
    }

    uart_decoder_thread = rt_thread_create("uart_decoder_th", uart_decoder_th, RT_NULL, 512, 20, 5);
    if (uart_decoder_thread != RT_NULL)   /* 如果获得线程控制块，启动这个线程 */
        rt_thread_startup(uart_decoder_thread); // 启动线程
    else
        return RT_ERROR;

    rt_kprintf("commu init success\n");
    return RT_EOK;
}
MSH_CMD_EXPORT(commu_init, motor drive)
