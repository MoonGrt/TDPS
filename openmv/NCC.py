import time, sensor, image
from image import SEARCH_EX, SEARCH_DS
#从imgae模块引入SEARCH_EX和SEARCH_DS。使用from import仅仅引入SEARCH_EX,
#SEARCH_DS两个需要的部分，而不把image模块全部引入。

sensor.reset() #初始化传感器（摄像头）
# 设置传感器
sensor.set_contrast(1)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.LCD) #分辨率，用LCD屏幕的话需要设为LCD。SEARCH_EX 最大用 QQVGA
sensor.set_pixformat(sensor.RGB565) #照片模式，灰度图像方式
sensor.skip_frames(time = 200) #延时跳过一些帧，等待感光元件变稳定。
sensor.set_auto_gain(False) # 颜色跟踪必须关闭自动增益
sensor.set_auto_whitebal(False) #关闭白平衡。
sensor.set_auto_exposure(False)# 设置曝光,需要更改

#sensor.set_windowing(((640-80)//2, (480-60)//2, 80, 60)) #子分辨率。可设置windowing窗口来减少搜索图片

templates1 = ['/zuo10.pgm']

clock = time.clock() #跟踪FPS帧率

#运行模板匹配
while (True):
    clock.tick() # 追踪两个snapshots()之间经过的毫秒数.
    img = sensor.snapshot().lens_corr(strength = 1.8, zoom = 1.0) #去畸变
    img = img.to_grayscale()

    #初始化计数常量
    n1 = 0
    n2 = 0
    n3 = 0

    for t in templates1:  #如果与模板匹配
        template = image.Image(t) #template获取图片
        r = img.find_template(template, 0.70, step=4, search=SEARCH_EX) #进行相关设置,可以设置roi缩小区域
        if r: #如果有目标
            img.draw_rectangle(r) #画矩形，框出匹配的目标
            n1 = n1 + 1

    print(clock.fps())
