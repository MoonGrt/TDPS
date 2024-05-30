import sensor, image, time, machine, math
uart = machine.UART(3, 115200)

# 似乎没法中断接收
#def uart_callback(line):
#    if uart.any():
#        data = uart.read(1)  # 读取一个字节的数据
#        print(data)  # 打印数据到终端上
#uart.irq(handler=uart_callback)

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

# 发送左右轮速度数据
def send_gate():
    uart.writechar(int(170))  # 数据包头 AA
    uart.writechar(int(1))


send_gate()
