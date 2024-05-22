#
#模板匹配简单拍照程序
#

import time, sensor, image
from image import SEARCH_EX, SEARCH_DS
#从imgae模块引入SEARCH_EX和SEARCH_DS。使用from import仅仅引入SEARCH_EX,
#SEARCH_DS两个需要的部分，而不把image模块全部引入。

sensor.reset() #初始化传感器（摄像头）
# 设置传感器
sensor.set_contrast(1)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.LCD) #分辨率，用LCD屏幕的话需要设为LCD。和模板匹配程序一样。
sensor.set_pixformat(sensor.GRAYSCALE) #照片模式，灰度图像方式
sensor.skip_frames(time = 200) #延时跳过一些帧，等待感光元件变稳定。
sensor.set_auto_gain(False) # 颜色跟踪必须关闭自动增益
sensor.set_auto_whitebal(False) #关闭白平衡。

clock = time.clock() # 跟踪FPS帧率

while(True):
    clock.tick()  # 追踪两个snapshots()之间经过的毫秒数.
    img = sensor.snapshot().lens_corr(strength = 1.8, zoom = 1.0)   #去畸变()
    print("FPS %f" % clock.fps()) # 注意: 当连接电脑后，OpenMV会变成一半的速度。当不连接电脑，帧率会增加。
