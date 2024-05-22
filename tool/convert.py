import sys
import os
import shutil
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QLabel, QLineEdit, QPushButton, QTextEdit, QMessageBox, QFileDialog
from PIL import Image

class FileConverter(QWidget):
    def __init__(self):
        super().__init__()
        self.resize(900, 200)
        self.initUI()

    def initUI(self):
        self.setWindowTitle('文件转换器')

        layout = QVBoxLayout()

        self.folder_label = QLabel('选择文件夹：')
        layout.addWidget(self.folder_label)

        # 设置默认目录为 F:\Study\TDPS\project\data\arrow
        self.default_folder = r'F:\Study\TDPS\project\data\arrow'
        self.folder_input = QLineEdit(self.default_folder)
        layout.addWidget(self.folder_input)

        self.browse_button = QPushButton('浏览')
        self.browse_button.clicked.connect(self.browseFolder)
        layout.addWidget(self.browse_button)

        self.convert_button = QPushButton('转换文件')
        self.convert_button.clicked.connect(self.convertFiles)
        layout.addWidget(self.convert_button)

        self.output_label = QLabel('文件列表：')
        layout.addWidget(self.output_label)

        self.output_text = QTextEdit()
        layout.addWidget(self.output_text)

        self.setLayout(layout)

    def browseFolder(self):
        folder_path = QFileDialog.getExistingDirectory(self, '选择文件夹', self.default_folder)
        if folder_path:
            self.folder_input.setText(folder_path)

    def convertFiles(self):
        folder_path = self.folder_input.text()
        if not folder_path:
            QMessageBox.warning(self, '警告', '请输入文件夹路径')
            return

        if not os.path.isdir(folder_path):
            QMessageBox.warning(self, '警告', '文件夹路径不存在')
            return

        bmp_files = [f for f in os.listdir(folder_path) if f.lower().endswith('.bmp')]
        if not bmp_files:
            QMessageBox.information(self, '提示', '文件夹中没有 .bmp 文件')
            return

        try:
            pgm_files = []
            for bmp_file in bmp_files:
                bmp_path = os.path.join(folder_path, bmp_file)
                pgm_file = os.path.splitext(bmp_file)[0] + '.pgm'
                pgm_path = os.path.join(folder_path, pgm_file)

                # # 使用PIL库打开.bmp文件，并转换为.pgm格式
                # with Image.open(bmp_path) as img:
                #     img_convert = img.convert('L')
                #     img_convert.save(pgm_path)

                # 打开BMP文件
                img = Image.open(bmp_path)
                # 转换为灰度图
                img_gray = img.convert('L')
                # 保存为PGM文件
                img_gray.save(pgm_path)

                pgm_files.append("/" + pgm_file)

            self.output_text.setText(str(pgm_files))
            QMessageBox.information(self, '提示', '文件转换完成')
        except Exception as e:
            QMessageBox.critical(self, '错误', f'文件转换失败：{str(e)}')


def bmp_to_pgm(input_file, output_file):
    # 打开BMP文件
    img = Image.open(input_file)
    # 转换为灰度图
    img_gray = img.convert('L')
    # 保存为PGM文件
    img_gray.save(output_file)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    converter = FileConverter()
    converter.show()
    sys.exit(app.exec_())
