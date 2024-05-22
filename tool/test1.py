
from PIL import Image

def bmp_to_pgm(input_file, output_file):
    # 打开BMP文件
    img = Image.open(input_file)
    
    # 转换为灰度图
    img_gray = img.convert('L')
    
    # 保存为PGM文件
    img_gray.save(output_file)

# 调用函数进行转换
bmp_to_pgm('F:\Study\TDPS\project\data\\arrow\left\zuo3.bmp', '.\output\zuo3.pgm')
