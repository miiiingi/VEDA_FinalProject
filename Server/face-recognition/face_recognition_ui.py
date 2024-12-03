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
from PySide6.QtWidgets import (QApplication, QGridLayout, QHBoxLayout, QLabel,
    QLineEdit, QMainWindow, QMenuBar, QPushButton,
    QSizePolicy, QStatusBar, QVBoxLayout, QWidget)

from QLed import QLed

class Ui_FaceRecognitionWindow(object):
    def setupUi(self, FaceRecognitionWindow):
        if not FaceRecognitionWindow.objectName():
            FaceRecognitionWindow.setObjectName(u"FaceRecognitionWindow")
        FaceRecognitionWindow.resize(640, 480)
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

        self.gridLayout = QGridLayout()
        self.gridLayout.setObjectName(u"gridLayout")
        self.status_led2_frame = QLed(self.centralwidget)
        self.status_led2_frame.setObjectName(u"status_led2_frame")
        self.status_led2_frame.setMinimumSize(QSize(80, 80))
        self.status_led2_frame.setOnColour(1)
        self.status_led2_frame.setShape(1)

        self.gridLayout.addWidget(self.status_led2_frame, 1, 0, 1, 1)

        self.status_led3_frame = QLed(self.centralwidget)
        self.status_led3_frame.setObjectName(u"status_led3_frame")
        self.status_led3_frame.setMinimumSize(QSize(80, 80))

        self.gridLayout.addWidget(self.status_led3_frame, 1, 1, 1, 1)

        self.status_led4_frame = QLed(self.centralwidget)
        self.status_led4_frame.setObjectName(u"status_led4_frame")
        self.status_led4_frame.setMinimumSize(QSize(80, 80))

        self.gridLayout.addWidget(self.status_led4_frame, 1, 2, 1, 1)

        self.status_led7_frame = QLed(self.centralwidget)
        self.status_led7_frame.setObjectName(u"status_led7_frame")
        self.status_led7_frame.setMinimumSize(QSize(80, 80))

        self.gridLayout.addWidget(self.status_led7_frame, 2, 2, 1, 1)

        self.status_led9_frame = QLed(self.centralwidget)
        self.status_led9_frame.setObjectName(u"status_led9_frame")
        self.status_led9_frame.setMinimumSize(QSize(80, 80))

        self.gridLayout.addWidget(self.status_led9_frame, 3, 1, 1, 1)

        self.status_led10_frame = QLed(self.centralwidget)
        self.status_led10_frame.setObjectName(u"status_led10_frame")
        self.status_led10_frame.setMinimumSize(QSize(80, 80))

        self.gridLayout.addWidget(self.status_led10_frame, 3, 2, 1, 1)

        self.start_button = QPushButton(self.centralwidget)
        self.start_button.setObjectName(u"start_button")

        self.gridLayout.addWidget(self.start_button, 4, 0, 1, 1)

        self.stop_button = QPushButton(self.centralwidget)
        self.stop_button.setObjectName(u"stop_button")

        self.gridLayout.addWidget(self.stop_button, 4, 1, 1, 1)

        self.lineEdit = QLineEdit(self.centralwidget)
        self.lineEdit.setObjectName(u"lineEdit")
        self.lineEdit.setEnabled(False)
        sizePolicy = QSizePolicy(QSizePolicy.Policy.Preferred, QSizePolicy.Policy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.lineEdit.sizePolicy().hasHeightForWidth())
        self.lineEdit.setSizePolicy(sizePolicy)
        self.lineEdit.setAlignment(Qt.AlignCenter)

        self.gridLayout.addWidget(self.lineEdit, 0, 1, 1, 1)

        self.lineEdit_2 = QLineEdit(self.centralwidget)
        self.lineEdit_2.setObjectName(u"lineEdit_2")
        self.lineEdit_2.setEnabled(False)
        sizePolicy.setHeightForWidth(self.lineEdit_2.sizePolicy().hasHeightForWidth())
        self.lineEdit_2.setSizePolicy(sizePolicy)
        self.lineEdit_2.setAlignment(Qt.AlignCenter)

        self.gridLayout.addWidget(self.lineEdit_2, 0, 0, 1, 1)

        self.lineEdit_3 = QLineEdit(self.centralwidget)
        self.lineEdit_3.setObjectName(u"lineEdit_3")
        self.lineEdit_3.setEnabled(False)
        sizePolicy.setHeightForWidth(self.lineEdit_3.sizePolicy().hasHeightForWidth())
        self.lineEdit_3.setSizePolicy(sizePolicy)
        self.lineEdit_3.setAlignment(Qt.AlignCenter)

        self.gridLayout.addWidget(self.lineEdit_3, 0, 2, 1, 1)

        self.lineEdit_4 = QLineEdit(self.centralwidget)
        self.lineEdit_4.setObjectName(u"lineEdit_4")
        self.lineEdit_4.setEnabled(False)
        sizePolicy.setHeightForWidth(self.lineEdit_4.sizePolicy().hasHeightForWidth())
        self.lineEdit_4.setSizePolicy(sizePolicy)
        self.lineEdit_4.setAlignment(Qt.AlignCenter)

        self.gridLayout.addWidget(self.lineEdit_4, 2, 1, 1, 1)


        self.horizontalLayout.addLayout(self.gridLayout)

        FaceRecognitionWindow.setCentralWidget(self.centralwidget)
        self.menubar = QMenuBar(FaceRecognitionWindow)
        self.menubar.setObjectName(u"menubar")
        self.menubar.setGeometry(QRect(0, 0, 640, 24))
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
        self.status_led2_frame.setStyleSheet(QCoreApplication.translate("FaceRecognitionWindow", u"background-color: #e0e0e0; border: 2px solid #999999; border-radius: 50%;", None))
        self.status_led3_frame.setStyleSheet(QCoreApplication.translate("FaceRecognitionWindow", u"background-color: #e0e0e0; border: 2px solid #999999; border-radius: 50%;", None))
        self.status_led4_frame.setStyleSheet(QCoreApplication.translate("FaceRecognitionWindow", u"background-color: #e0e0e0; border: 2px solid #999999; border-radius: 50%;", None))
        self.status_led7_frame.setStyleSheet(QCoreApplication.translate("FaceRecognitionWindow", u"background-color: #e0e0e0; border: 2px solid #999999; border-radius: 50%;", None))
        self.status_led9_frame.setStyleSheet(QCoreApplication.translate("FaceRecognitionWindow", u"background-color: #e0e0e0; border: 2px solid #999999; border-radius: 50%;", None))
        self.status_led10_frame.setStyleSheet(QCoreApplication.translate("FaceRecognitionWindow", u"background-color: #e0e0e0; border: 2px solid #999999; border-radius: 50%;", None))
        self.start_button.setText(QCoreApplication.translate("FaceRecognitionWindow", u"Start", None))
        self.stop_button.setText(QCoreApplication.translate("FaceRecognitionWindow", u"Stop", None))
        self.lineEdit.setText(QCoreApplication.translate("FaceRecognitionWindow", u"\ubb38 \uac1c\ubc29", None))
        self.lineEdit_2.setText(QCoreApplication.translate("FaceRecognitionWindow", u"\ucd08\uc778\uc885", None))
        self.lineEdit_3.setText(QCoreApplication.translate("FaceRecognitionWindow", u"LED", None))
        self.lineEdit_4.setText(QCoreApplication.translate("FaceRecognitionWindow", u"\ubb38 \uc7a0\uae08", None))
    # retranslateUi

