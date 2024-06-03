from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QHBoxLayout, QLabel, QLineEdit, QPushButton, QTextEdit, QFileDialog

class FileConverter(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle('文件转换器')

        layout = QVBoxLayout()

        self.folder_label = QLabel('选择文件夹：')
        layout.addWidget(self.folder_label)

        # 设置默认目录为 F:\Study\TDPS\project\data\arrow
        self.default_folder = r'F:\Study\TDPS\project\data\arrow'
        self.folder_input = QLineEdit(self.default_folder)

        self.browse_button = QPushButton('浏览')
        self.browse_button.clicked.connect(self.browseFolder)

        self.convert_button = QPushButton('转换文件')
        self.convert_button.clicked.connect(self.convertFiles)

        # Create a horizontal layout for the input box and buttons
        h_layout = QHBoxLayout()
        h_layout.addWidget(self.folder_input)
        h_layout.addWidget(self.browse_button)
        h_layout.addWidget(self.convert_button)

        # Add the horizontal layout to the main layout
        layout.addLayout(h_layout)

        self.output_label = QLabel('文件列表：')
        layout.addWidget(self.output_label)

        self.output_text = QTextEdit()
        layout.addWidget(self.output_text)

        self.setLayout(layout)

    def browseFolder(self):
        folder = QFileDialog.getExistingDirectory(self, '选择文件夹', self.default_folder)
        if folder:
            self.folder_input.setText(folder)

    def convertFiles(self):
        # Placeholder for file conversion logic
        folder = self.folder_input.text()
        self.output_text.append(f'Converting files in folder: {folder}')

if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)
    ex = FileConverter()
    ex.show()
    sys.exit(app.exec_())
