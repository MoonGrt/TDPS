import sensor, image, time

THRESHOLD_LAB = (80, 100, -7, 11, -12, 7)
THRESHOLD_GRAY = (0, 61)

BIN = 0
GRAY = 1

sensor.reset()                      # Reset and initialize the sensor.
if GRAY:
    sensor.set_pixformat(sensor.GRAYSCALE) # Set pixel format to RGB565 (or GRAYSCALE)
    EXPOSURE_TIME_SCALE = 1         # 更改此值以调整曝光。试试10.0 / 0.1 /等。
else:
    sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
    EXPOSURE_TIME_SCALE = 2.2
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)

# 打印出初始曝光时间以进行比较。
print("Initial exposure == %d" % sensor.get_exposure_us())

sensor.skip_frames(time = 2000)     # Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.

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

while(True):
    clock.tick()                    # Update the FPS clock.

    if BIN:
        if GRAY:
            img = sensor.snapshot().binary([THRESHOLD_GRAY])  # 拍摄图像并应用二值化阈值
        else:
            img = sensor.snapshot().binary([THRESHOLD_LAB])  # 拍摄图像并应用二值化阈值
    else:
        img = sensor.snapshot()         # Take a picture and return the image.




#    print(clock.fps())              # Note: OpenMV Cam runs about half as fast when connected
                                    # to the IDE. The FPS should increase once disconnected.

