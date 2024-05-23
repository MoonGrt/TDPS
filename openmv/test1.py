import sensor, image, time, machine, pyb
from image import SEARCH_EX, SEARCH_DS

uart = machine.UART(3, 115200)

# 更改此值以调整曝光。试试10.0 / 0.1 /等。
EXPOSURE_TIME_SCALE = 2

sensor.reset()                      # 复位并初始化传感器。
sensor.set_pixformat(sensor.RGB565) # 设置图像色彩格式，有RGB565色彩图和GRAYSCALE灰度图两种
sensor.set_framesize(sensor.QQVGA)   # 将图像大小设置为QVGA (160x120)

# 打印出初始曝光时间以进行比较。
print("Initial exposure == %d" % sensor.get_exposure_us())

sensor.skip_frames(time = 2000)     # 等待设置生效。
clock = time.clock()                # 创建一个时钟对象来跟踪FPS帧率。

# 您必须关闭自动增益控制和自动白平衡，否则他们将更改图像增益以撤消您放置的任何曝光设置...
sensor.set_auto_gain(False)
sensor.set_auto_whitebal(False)
# 需要让以上设置生效
sensor.skip_frames(time = 500)

current_exposure_time_in_microseconds = sensor.get_exposure_us()
print("Current Exposure == %d" % current_exposure_time_in_microseconds)

# 默认情况下启用自动曝光控制（AEC）。调用以下功能可禁用传感器自动曝光控制。
# 另外“exposure_us”参数在AEC被禁用后覆盖自动曝光值。
sensor.set_auto_exposure(False, \
    exposure_us = int(current_exposure_time_in_microseconds * EXPOSURE_TIME_SCALE))

print("New exposure == %d" % sensor.get_exposure_us())
# sensor.get_exposure_us()以微秒为单位返回精确的相机传感器曝光时间。
# 然而，这可能与命令的数量不同，因为传感器代码将曝光时间以微秒转换为行/像素/时钟时间，这与微秒不完全匹配...





from pid import PID
distance_pid = PID(p=0.4, i=0)

# contant
blue_threshold = (14, 100, -9, 31, -16, 21)  # L A B
red_threshold  = (0, 45, 40, 127, -8, 45)
yellow_threshold = (58, 77, -22, 31, 16, 73)
green_threshold = (43, 23, -37, -6, -14, 17)
ROW = 85  # 分析图像的第几行
width_const = 40  # 巡线判断阙值
edge_threshold = 5  # 巡线判断阙值
FOR_DELAY = 12500  # 三向路口直线延迟时间
SP_L = 30.8  # 直行时左轮速度
SP_R = 35  # 直行时右轮速度
DIS_CONST = 30  # 距离判断阙值
alpha_value = 0.2  # 低通滤波器的alpha值

# variable
direction = 3
old_direction = 3
old_output = 0
old_error = 0
sp_l = 0
sp_r = 0
have_check = 0

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

def get_sparse_his(img, COL):
    global ROW
    line_data = []
    for i in range(80): # col
        line_data.append(0)
        for j in range(20): # row
            pixel = img.get_pixel(i+COL, j+ROW)
            # 检查像素是否为白色(1)或黑色(0)
            if pixel == (0,0,0):  # 黑色
                line_data[i] += 1
    return line_data

def low_pass_filter(input_data, alpha):
    global old_output
    output_data = alpha * input_data + (1 - alpha) * old_output
    old_output = output_data
    return output_data

# 使用低通滤波器平滑输出数据
def get_error_filtered(his, alpha):
    global old_error, old_output, direction
    if direction == 3:
        for index, value in enumerate(his):
            if value > edge_threshold:
                old_error = width_const - index
                old_output = old_error  # 将old_error作为输出的一部分
                return low_pass_filter(old_error, alpha)
    elif direction == 1:
        for index, value in reversed(list(enumerate(his))):
            if value > edge_threshold:
                old_error = width_const - index
                old_output = old_error  # 将old_error作为输出的一部分
                return low_pass_filter(old_error, alpha)
    return old_error


templates1 = ['/left1.pgm']
templates2 = ['/forward1.pgm']
templates3 = ['/right1.pgm']
templates4 = ['/obstacle1.pgm']



def track(img):
    global old_direction, direction, old_error
    img_blue = img.copy().binary([blue_threshold])
    img_blue.dilate(1)
    error = 0
    if direction == 1:
        his = get_sparse_his(img_blue, 0)
        if old_direction != direction:
            old_error = 30
        old_direction = direction
        error = get_error_filtered(his, alpha_value)
    elif direction == 3:
        his = get_sparse_his(img_blue, 79)
        error = get_error_filtered(his, alpha_value)

#    print('==', error, '==')
#    print(direction)
    distance_output = distance_pid.get_pid(error, 1)

    return SP_L - distance_output, SP_R + distance_output


def check_arrow(img):
    global direction
#    print('In State 1')

    img_grey = img.copy()
    img_grey = img_grey.to_grayscale()

    for t in templates1:  # 如果与模板匹配
        template = image.Image(t) # template获取图片
        r = img_grey.find_template(template, 0.90, step=4, search=SEARCH_EX) # 进行相关设置,可以设置roi缩小区域
        if r: # 如果有目标
            img.draw_rectangle(r) # 画矩形，框出匹配的目标
            direction = 3
            print("left")
            return 2
    for t in templates2:
        template = image.Image(t)
        r = img_grey.find_template(template, 0.90, step=4, search=SEARCH_EX) #, roi=(10, 0, 60, 60))
        if r:
            img.draw_rectangle(r)
            direction = 2
            print("forward")
            return 2
#    for t in templates3:
#        template = image.Image(t)
#        r = img_grey.find_template(template, 0.90, step=4, search=SEARCH_EX) #, roi=(10, 0, 60, 60))
#        if r:
#            img.draw_rectangle(r)
#            direction = 3
#            print("right")
#            return 2

    return 1

def traffic_light(img):
    global direction
    print('In State 2')

    img_r = img.copy()
    img_r = img_r.binary([red_threshold])
    for c in img_r.find_circles(threshold = 5000, x_margin = 10, y_margin = 10, r_margin = 10,
                r_min = 2, r_max = 100, r_step = 2):
        area = (c.x()-c.r(), c.y()-c.r(), 2*c.r(), 2*c.r())
        img.draw_rectangle(area, color = (255, 255, 255))
        direction = 0

    img_y = img.copy()
    img_y = img_y.binary([yellow_threshold])
    for c in img_y.find_circles(threshold = 5000, x_margin = 10, y_margin = 10, r_margin = 10,
                r_min = 2, r_max = 100, r_step = 2):
        area = (c.x()-c.r(), c.y()-c.r(), 2*c.r(), 2*c.r())
        img.draw_rectangle(area, color = (255, 255, 255))
        direction = 0

    img_g = img.copy()
    img_g = img_g.binary([green_threshold])
    for c in img_g.find_circles(threshold = 5000, x_margin = 10, y_margin = 10, r_margin = 10,
                r_min = 2, r_max = 100, r_step = 2):
        area = (c.x()-c.r(), c.y()-c.r(), 2*c.r(), 2*c.r())
        img.draw_rectangle(area, color = (255, 255, 255))
        direction = 3
        return 3

    return 2

def pedestrian(img):
    global direction
#    print('In State 3')

    # 轮询读取uart，距离 cm
    if uart.any():
        distance_bytes = uart.read(1)  # 读取一个字节的数据
        distance = int.from_bytes(distance_bytes, 'big')  # 将字节转换为整数
        print(distance)
        if distance < DIS_CONST:
            direction = 0
        else:
            direction = 3

    img_grey = img.copy()
    img_grey = img_grey.to_grayscale()
    for t in templates1:
        template = image.Image(t)
        r = img_grey.find_template(template, 0.90, step=4, search=SEARCH_EX) #, roi=(10, 0, 60, 60))
        if r:
            img.draw_rectangle(r)
            direction = 3
            print("left")
            return 4

    return 3

def state_4(img):
    global direction
    print('In State 4')





    return 5

def state_5(img):
    global direction
    print('In State 5')
    return 6

def state_6(img):
    global direction
    print('In State 6')
    return 6

# 状态转移字典
state_functions = {
    1: check_arrow,
    2: traffic_light,
    3: pedestrian,
    4: state_4,
    5: state_5,
    6: state_6,
}

# 初始状态
current_state = 4

# 运行状态机
while True:
    clock.tick()

    img = sensor.snapshot()
#    img = img.to_grayscale()

    sp_l, sp_r = track(img)
    current_state = state_functions[current_state](img)


    if direction == 2:
        send_sp(SP_L, SP_R)
        pyb.delay(FOR_DELAY)
        direction = 3
        old_error = -30
    elif direction == 0:
        send_sp(0, 0)
    else:
        send_sp(sp_l, sp_r)
