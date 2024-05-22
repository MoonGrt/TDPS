import sensor, image, time, machine
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

# 如果要重新打开自动曝光，请执行以下操作：sensor.set_auto_exposure(True)
# 请注意，相机传感器将根据需要更改曝光时间。

# 执行：sensor.set_auto_exposure(False)，只是禁用曝光值更新，但不会更改相机传感器确定的曝光值。



from pid import PID
dissatnce_pid = PID(p=0.4, i=0)

blue_threshold = (25, 100, -12, 41, -20, 16)  # L A B
reg_threshold  = (91, 100, -16, 15, -7, 25)
ROI = (0,60,160,60)

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

def get_sparse_his(img):
    line_data = []
    for i in range(80): # col
        line_data.append(0)
        for j in range(20): # row
            pixel = img.get_pixel(i+79, j+85)
            # 检查像素是否为白色(1)或黑色(0)
            if pixel == (0,0,0):  # 黑色
                line_data[i] += 1
    return line_data

old_error = 0
width_const = 40
def get_error(his):
    global old_error
    for index, value in enumerate(his):
        if value > 5:
            old_error = width_const -index
            return width_const - index
    if old_error > 0:
        return 40
    elif old_error < 0:
        return -40
    else:
        return 0

sp_l = 0
sp_r = 0

templates1 = ['/left1.pgm']
templates2 = ['/right1.pgm']
templates3 = ['/forward1.pgm']

while(True):
    clock.tick()

    img = sensor.snapshot()
    img_grey = img.copy()
    img_grey = img_grey.to_grayscale()

    #初始化计数常量
    n1 = 0
    n2 = 0
    n3 = 0

    for t in templates1:  #如果与模板匹配
        template = image.Image(t) #template获取图片
        r = img_grey.find_template(template, 0.90, step=4, search=SEARCH_EX) #进行相关设置,可以设置roi缩小区域
        if r: #如果有目标
            img.draw_rectangle(r) #画矩形，框出匹配的目标
#            print("left")

    for t in templates2:
        template = image.Image(t)
        r = img_grey.find_template(template, 0.90, step=4, search=SEARCH_EX) #, roi=(10, 0, 60, 60))
        if r:
            img.draw_rectangle(r)
#            print("right")

    for t in templates3:
        template = image.Image(t)
        r = img_grey.find_template(template, 0.70, step=4, search=SEARCH_EX) #, roi=(10, 0, 60, 60))
        if r:
            img.draw_rectangle(r)
#            print("forward")



#    img_r = img.copy()
#    img_r = img_r.binary([reg_threshold])
#    img_b = img.copy()
#    img_b = img_b.binary([blue_threshold])
#    img_g = img.copy()
#    img_g = img_g.binary([blue_threshold])
#    img_y = img.copy()
#    img_y = img_y.binary([blue_threshold])
#    img_grey = img.copy()
#    img_grey = img_grey.binary([blue_threshold])


    img_blue = img.binary([blue_threshold])
    his = get_sparse_his(img_blue)
#    print(his)
    error = get_error(his)
    print('==', error, '==')
    distance_output = dissatnce_pid.get_pid(error, 1)
    print(distance_output)

    sp_l = 30.5 - distance_output
    sp_r = 35 + distance_output
    print(sp_l, sp_r)
    send_sp(sp_l, sp_r)

    print(clock.fps())
