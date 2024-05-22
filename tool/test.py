import sys
import os
from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QLineEdit, QPushButton, QVBoxLayout, QMessageBox, QFileDialog

class ConverterApp(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("BMP to PGM Converter")
        self.setGeometry(100, 100, 400, 200)

        default_path = r"F:\Study\TDPS\project\data\arrow"  # 设置默认路径

        self.folder_label = QLabel("请选择文件夹：")
        self.folder_input = QLineEdit(default_path)  # 设置默认路径
        self.browse_button = QPushButton("浏览")
        self.browse_button.clicked.connect(self.browse_folder)

        self.convert_button = QPushButton("转换")
        self.convert_button.clicked.connect(self.convert_bmp_to_pgm)

        self.output_label = QLabel("转换后的文件列表：")
        self.output_text = QLabel()

        layout = QVBoxLayout()
        layout.addWidget(self.folder_label)
        layout.addWidget(self.folder_input)
        layout.addWidget(self.browse_button)
        layout.addWidget(self.convert_button)
        layout.addWidget(self.output_label)
        layout.addWidget(self.output_text)

        self.setLayout(layout)

    def browse_folder(self):
        folder_path = QFileDialog.getExistingDirectory(self, "选择文件夹")
        if folder_path:
            self.folder_input.setText(folder_path)

    def convert_bmp_to_pgm(self):
        folder_path = self.folder_input.text()
        if not os.path.isdir(folder_path):
            QMessageBox.critical(self, "错误", "文件夹路径无效！")
            return

        bmp_files = [f for f in os.listdir(folder_path) if f.lower().endswith('.bmp')]
        if not bmp_files:
            QMessageBox.information(self, "提示", "文件夹中没有 BMP 文件！")
            return

        pgm_files = []
        for bmp_file in bmp_files:
            bmp_path = os.path.join(folder_path, bmp_file)
            pgm_file = os.path.splitext(bmp_file)[0] + '.pgm'
            pgm_path = os.path.join(folder_path, pgm_file)

            pgm_files.append(pgm_path)

        self.output_text.setText(str(pgm_files))

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = ConverterApp()
    window.show()
    sys.exit(app.exec_())
