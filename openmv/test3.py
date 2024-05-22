THRESHOLD = (5, 70, -23, 15, -57, 0) # Grayscale threshold for dark things...
import sensor, image, time, machine
from PID import PID
rho_pid = PID(p=0.4, i=0)
theta_pid = PID(p=0.001, i=0)

sensor.reset()
sensor.set_vflip(True)
sensor.set_hmirror(True)
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQQVGA) # 80x60 (4,800 pixels) - O(N^2) max = 2,3040,000.
#sensor.set_windowing([0,20,80,40])
sensor.skip_frames(time = 2000)     # Warning: If you use QQVGA it may take seconds
# sensor.set_auto_gain(False)  # 关闭自动自动增益。默认开启的。
# sensor.set_auto_whitebal(False)  # 关闭白平衡。白平衡是默认开启的，在颜色识别中，一定要关闭白平衡。
clock = time.clock()                # to process a frame sometimes.


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


# 获取稀疏直方图：用稀疏直方图降低运算量
def get_sparse_his(binary_array):
    his = list()
    for i in range(39):
        real_column = (i + 1) * 8 - 1
        his.append(0)

        index = real_column

        his[i] += binary_array[index]  # 为了减少乘法运算量，将第一步不加320的运算在循环之外算

        for j in range(239):
            index = index + 320
            his[i] += binary_array[index]
    return his

# 获取稀疏直方图：用稀疏直方图降低运算量
def get_sparse_his(his):
    his = list()
    min()
    return his


while(True):
    clock.tick()
    # img = sensor.snapshot()
    img = sensor.snapshot().binary([THRESHOLD])

    print(clock.fps())
