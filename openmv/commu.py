#import sensor, image, time, machine, math
#uart = machine.UART(3, 115200)

## 似乎没法中断接收
##def uart_callback(line):
##    if uart.any():
##        data = uart.read(1)  # 读取一个字节的数据
##        print(data)  # 打印数据到终端上
##uart.irq(handler=uart_callback)

## 发送左右轮速度数据
#def send_sp(sp_l, sp_r):  # 传入数据为pwm百分比，格式为 xx.xx（两位整数；两位小数）
#    sp_l_int = int(sp_l)  # 获取整数部分
#    sp_l_dec = sp_l - sp_l_int  # 获取小数部分
#    sp_r_int = int(sp_r)  # 获取整数部分
#    sp_r_dec = sp_r - sp_r_int  # 获取小数部分

#    uart.writechar(int(170))  # 数据包头 AA 55
#    uart.writechar(int(85))
#    uart.writechar(min(100, int(sp_l_int)))  # 发送整数部分
#    uart.writechar(int(round(sp_l_dec * 100)))  # 将小数部分乘以100后转化为整数发送, 两位小数
#    uart.writechar(min(100, int(sp_r_int)))  # 发送整数部分
#    uart.writechar(int(round(sp_r_dec * 100)))  # 将小数部分乘以100后转化为整数发送, 两位小数


#i = 0
#while(True):
#    sp_l = 20.12 + i * 10
#    sp_r = 20.34 + i * 10
#    send_sp(sp_l, sp_r)
##    send_sp(50, 30)
#    time.sleep(2)  # 延时一段时间再发送下一个浮点数

#    i = (i + 1) % 4

#    # 轮询读取uart，距离 cm
#    if uart.any():
#        distance_bytes = uart.read(1)  # 读取一个字节的数据
#        distance = int.from_bytes(distance_bytes, 'big')  # 将字节转换为整数
#        print(distance)



# 多线程
import sensor, image, time, machine, math
import uasyncio as asyncio
uart = machine.UART(3, 115200)

# 发送左右轮速度数据
def send_sp(sp_l, sp_r):  # 传入数据为pwm百分比，格式为 xx.xx（两位整数；两位小数）
    sp_l_int = int(sp_l)  # 获取整数部分
    sp_l_dec = sp_l - sp_l_int  # 获取小数部分
    sp_r_int = int(sp_r)  # 获取整数部分
    sp_r_dec = sp_r - sp_r_int  # 获取小数部分

    uart.writechar(int(170))  # 数据包头 AA 55
    uart.writechar(int(85))
    uart.writechar(min(100, int(sp_l_int)))  # 发送整数部分
    uart.writechar(int(round(sp_l_dec * 100)))  # 将小数部分乘以100后转化为整数发送, 两位小数
    uart.writechar(min(100, int(sp_r_int)))  # 发送整数部分
    uart.writechar(int(round(sp_r_dec * 100)))  # 将小数部分乘以100后转化为整数发送, 两位小数

async def distance_get():
    while True:
        # 轮询读取uart，距离 cm
        if uart.any():
            distance_bytes = uart.read(1)  # 读取一个字节的数据
            distance = int.from_bytes(distance_bytes, 'big')  # 将字节转换为整数
            print(distance)
        await asyncio.sleep_ms(100)

async def sp_ctrl():
    i = 0
    while(True):
        sp_l = 20.12 + i * 10
        sp_r = 20.34 + i * 10
        send_sp(sp_l, sp_r)
    #    send_sp(50, 30)
        await asyncio.sleep_ms(2000)  # 延时一段时间再发送下一个浮点数

        i = (i + 1) % 4

loop = asyncio.get_event_loop()
loop.create_task(distance_get())
loop.create_task(sp_ctrl())
loop.run_forever()


