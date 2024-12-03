# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'face_recognition.ui'
##
## Created by: Qt User Interface Compiler version 6.8.0
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QBrush, QColor, QConicalGradient, QCursor,
    QFont, QFontDatabase, QGradient, QIcon,
    QImage, QKeySequence, QLinearGradient, QPainter,
    QPalette, QPixmap, QRadialGradient, QTransform)
from PySide6.QtWidgets import (QApplication, QHBoxLayout, QLabel, QMainWindow,
    QMenuBar, QPushButton, QSizePolicy, QSpacerItem,
    QStatusBar, QVBoxLayout, QWidget)

class Ui_FaceRecognitionWindow(object):
    def setupUi(self, FaceRecognitionWindow):
        if not FaceRecognitionWindow.objectName():
            FaceRecognitionWindow.setObjectName(u"FaceRecognitionWindow")
        FaceRecognitionWindow.resize(974, 862)
        FaceRecognitionWindow.setAutoFillBackground(False)
        self.centralwidget = QWidget(FaceRecognitionWindow)
        self.centralwidget.setObjectName(u"centralwidget")
        self.horizontalLayout = QHBoxLayout(self.centralwidget)
        self.horizontalLayout.setObjectName(u"horizontalLayout")
        self.videoLayout = QVBoxLayout()
        self.videoLayout.setObjectName(u"videoLayout")
        self.video_label = QLabel(self.centralwidget)
        self.video_label.setObjectName(u"video_label")
        self.video_label.setMinimumSize(QSize(320, 240))
        self.video_label.setAlignment(Qt.AlignCenter)

        self.videoLayout.addWidget(self.video_label)


        self.horizontalLayout.addLayout(self.videoLayout)

        self.statusLayout = QVBoxLayout()
        self.statusLayout.setSpacing(10)
        self.statusLayout.setObjectName(u"statusLayout")
        self.doorbell_layout = QVBoxLayout()
        self.doorbell_layout.setObjectName(u"doorbell_layout")
        self.status_doorbell_frame = QLabel(self.centralwidget)
        self.status_doorbell_frame.setObjectName(u"status_doorbell_frame")
        self.status_doorbell_frame.setMinimumSize(QSize(80, 80))
        self.status_doorbell_frame.setStyleSheet(u"background-color: #e0e0e0; border: 2px solid #999999; border-radius: 50%;")
        self.status_doorbell_frame.setAlignment(Qt.AlignCenter)

        self.doorbell_layout.addWidget(self.status_doorbell_frame)

        self.status_doorbell_label = QLabel(self.centralwidget)
        self.status_doorbell_label.setObjectName(u"status_doorbell_label")
        self.status_doorbell_label.setAlignment(Qt.AlignCenter)

        self.doorbell_layout.addWidget(self.status_doorbell_label)


        self.statusLayout.addLayout(self.doorbell_layout)

        self.buzzer_layout = QVBoxLayout()
        self.buzzer_layout.setObjectName(u"buzzer_layout")
        self.status_buzzer_frame = QLabel(self.centralwidget)
        self.status_buzzer_frame.setObjectName(u"status_buzzer_frame")
        self.status_buzzer_frame.setMinimumSize(QSize(80, 80))
        self.status_buzzer_frame.setStyleSheet(u"background-color: #e0e0e0; border: 2px solid #999999; border-radius: 50%;")
        self.status_buzzer_frame.setAlignment(Qt.AlignCenter)

        self.buzzer_layout.addWidget(self.status_buzzer_frame)

        self.status_buzzer_label = QLabel(self.centralwidget)
        self.status_buzzer_label.setObjectName(u"status_buzzer_label")
        self.status_buzzer_label.setAlignment(Qt.AlignCenter)

        self.buzzer_layout.addWidget(self.status_buzzer_label)


        self.statusLayout.addLayout(self.buzzer_layout)

        self.led1_layout = QVBoxLayout()
        self.led1_layout.setObjectName(u"led1_layout")
        self.status_led1_frame = QLabel(self.centralwidget)
        self.status_led1_frame.setObjectName(u"status_led1_frame")
        self.status_led1_frame.setMinimumSize(QSize(80, 80))
        self.status_led1_frame.setStyleSheet(u"background-color: #e0e0e0; border: 2px solid #999999; border-radius: 50%;")
        self.status_led1_frame.setAlignment(Qt.AlignCenter)

        self.led1_layout.addWidget(self.status_led1_frame)

        self.status_led1_label = QLabel(self.centralwidget)
        self.status_led1_label.setObjectName(u"status_led1_label")
        self.status_led1_label.setAlignment(Qt.AlignCenter)

        self.led1_layout.addWidget(self.status_led1_label)


        self.statusLayout.addLayout(self.led1_layout)

        self.led2_layout = QVBoxLayout()
        self.led2_layout.setObjectName(u"led2_layout")
        self.status_led2_frame = QLabel(self.centralwidget)
        self.status_led2_frame.setObjectName(u"status_led2_frame")
        self.status_led2_frame.setMinimumSize(QSize(80, 80))
        self.status_led2_frame.setStyleSheet(u"background-color: #e0e0e0; border: 2px solid #999999; border-radius: 50%;")
        self.status_led2_frame.setAlignment(Qt.AlignCenter)

        self.led2_layout.addWidget(self.status_led2_frame)

        self.status_led2_label = QLabel(self.centralwidget)
        self.status_led2_label.setObjectName(u"status_led2_label")
        self.status_led2_label.setAlignment(Qt.AlignCenter)

        self.led2_layout.addWidget(self.status_led2_label)


        self.statusLayout.addLayout(self.led2_layout)

        self.led3_layout = QVBoxLayout()
        self.led3_layout.setObjectName(u"led3_layout")
        self.status_led3_frame = QLabel(self.centralwidget)
        self.status_led3_frame.setObjectName(u"status_led3_frame")
        self.status_led3_frame.setMinimumSize(QSize(80, 80))
        self.status_led3_frame.setStyleSheet(u"background-color: #e0e0e0; border: 2px solid #999999; border-radius: 50%;")
        self.status_led3_frame.setAlignment(Qt.AlignCenter)

        self.led3_layout.addWidget(self.status_led3_frame)

        self.status_led3_label = QLabel(self.centralwidget)
        self.status_led3_label.setObjectName(u"status_led3_label")
        self.status_led3_label.setAlignment(Qt.AlignCenter)

        self.led3_layout.addWidget(self.status_led3_label)


        self.statusLayout.addLayout(self.led3_layout)

        self.led4_layout = QVBoxLayout()
        self.led4_layout.setObjectName(u"led4_layout")
        self.status_led4_frame = QLabel(self.centralwidget)
        self.status_led4_frame.setObjectName(u"status_led4_frame")
        self.status_led4_frame.setMinimumSize(QSize(80, 80))
        self.status_led4_frame.setStyleSheet(u"background-color: #e0e0e0; border: 2px solid #999999; border-radius: 50%;")
        self.status_led4_frame.setAlignment(Qt.AlignCenter)

        self.led4_layout.addWidget(self.status_led4_frame)

        self.status_led4_label = QLabel(self.centralwidget)
        self.status_led4_label.setObjectName(u"status_led4_label")
        self.status_led4_label.setAlignment(Qt.AlignCenter)

        self.led4_layout.addWidget(self.status_led4_label)


        self.statusLayout.addLayout(self.led4_layout)

        self.buttonLayout = QVBoxLayout()
        self.buttonLayout.setObjectName(u"buttonLayout")
        self.start_button = QPushButton(self.centralwidget)
        self.start_button.setObjectName(u"start_button")

        self.buttonLayout.addWidget(self.start_button)

        self.stop_button = QPushButton(self.centralwidget)
        self.stop_button.setObjectName(u"stop_button")

        self.buttonLayout.addWidget(self.stop_button)


        self.statusLayout.addLayout(self.buttonLayout)

        self.verticalSpacer = QSpacerItem(20, 40, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding)

        self.statusLayout.addItem(self.verticalSpacer)


        self.horizontalLayout.addLayout(self.statusLayout)

        FaceRecognitionWindow.setCentralWidget(self.centralwidget)
        self.menubar = QMenuBar(FaceRecognitionWindow)
        self.menubar.setObjectName(u"menubar")
        self.menubar.setGeometry(QRect(0, 0, 974, 37))
        FaceRecognitionWindow.setMenuBar(self.menubar)
        self.statusbar = QStatusBar(FaceRecognitionWindow)
        self.statusbar.setObjectName(u"statusbar")
        FaceRecognitionWindow.setStatusBar(self.statusbar)

        self.retranslateUi(FaceRecognitionWindow)

        QMetaObject.connectSlotsByName(FaceRecognitionWindow)
    # setupUi

    def retranslateUi(self, FaceRecognitionWindow):
        FaceRecognitionWindow.setWindowTitle(QCoreApplication.translate("FaceRecognitionWindow", u"Face Recognition", None))
        self.video_label.setText("")
        self.status_doorbell_frame.setText("")
        self.status_doorbell_label.setText(QCoreApplication.translate("FaceRecognitionWindow", u"\ucd08\uc778\uc885", None))
        self.status_buzzer_frame.setText("")
        self.status_buzzer_label.setText(QCoreApplication.translate("FaceRecognitionWindow", u"\ubd80\uc800", None))
        self.status_led1_frame.setText("")
        self.status_led1_label.setText(QCoreApplication.translate("FaceRecognitionWindow", u"LED1", None))
        self.status_led2_frame.setText("")
        self.status_led2_label.setText(QCoreApplication.translate("FaceRecognitionWindow", u"LED2", None))
        self.status_led3_frame.setText("")
        self.status_led3_label.setText(QCoreApplication.translate("FaceRecognitionWindow", u"LED3", None))
        self.status_led4_frame.setText("")
        self.status_led4_label.setText(QCoreApplication.translate("FaceRecognitionWindow", u"LED4", None))
        self.start_button.setText(QCoreApplication.translate("FaceRecognitionWindow", u"Start Recognition", None))
        self.stop_button.setText(QCoreApplication.translate("FaceRecognitionWindow", u"Stop Recognition", None))
    # retranslateUi

