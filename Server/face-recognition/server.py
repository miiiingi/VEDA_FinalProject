import sys
import socket
import signal
import threading
import queue
import time
from collections import Counter
import face_recognition
import cv2
import numpy as np
from PyQt6.QtWidgets import (QApplication, QMainWindow, QVBoxLayout, QLabel, QHBoxLayout, 
                           QPushButton, QWidget)
from PyQt6.QtGui import QImage, QPixmap
from PyQt6.QtCore import Qt, QThread, pyqtSignal, QTimer

class TCPServer(threading.Thread):
    def __init__(self, result_queue, host='0.0.0.0', port=5100):
        super().__init__()
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.bind((host, port))
        self.server_socket.listen(5)
        self.client_sockets = []
        self.client_lock = threading.Lock()
        self.running = True
        self.result_queue = result_queue

    def handle_client(self, client_socket, client_addr):
        print(f"Client connected from {client_addr[0]}")
        try:
            while self.running:
                try:
                    start_time = time.time()
                    counts = Counter()
                    while time.time() - start_time < 1:
                        try:
                            name = self.result_queue.get_nowait()
                            result = "0" if name == "Unknown" else "1"
                            counts[result] += 1
                        except queue.Empty:
                            time.sleep(0.1)
                    
                    if counts:
                        most_common = counts.most_common(1)[0][0]
                        client_socket.send(most_common.encode())
                    else:
                        client_socket.send(b"0")
                except:
                    break
        finally:
            with self.client_lock:
                if client_socket in self.client_sockets:
                    self.client_sockets.remove(client_socket)
            client_socket.close()

    def run(self):
        print("TCP Server waiting for connections...")
        while self.running:
            try:
                client_socket, client_addr = self.server_socket.accept()
                with self.client_lock:
                    self.client_sockets.append(client_socket)
                client_thread = threading.Thread(
                    target=self.handle_client,
                    args=(client_socket, client_addr)
                )
                client_thread.daemon = True
                client_thread.start()
            except:
                break

    def stop(self):
        self.running = False
        with self.client_lock:
            for sock in self.client_sockets:
                sock.close()
        self.server_socket.close()

class FaceRecognitionThread(QThread):
    image_update_signal = pyqtSignal(np.ndarray)
    access_status_signal = pyqtSignal(str)

    def __init__(self, result_queue):
        super().__init__()
        self.result_queue = result_queue
        self.running = True

    def run(self):
        video_capture = cv2.VideoCapture(0)
        try:
            mingi_image = face_recognition.load_image_file("mingi.jpg")
            mingi_face_encoding = face_recognition.face_encodings(mingi_image)[0]

            known_face_encodings = [
                mingi_face_encoding
            ]
            known_face_names = [
                "mingi"
            ]

            process_this_frame = True

            while self.running:
                ret, frame = video_capture.read()
                if not ret:
                    break

                if process_this_frame:
                    small_frame = cv2.resize(frame, (0, 0), fx=0.25, fy=0.25)
                    rgb_small_frame = small_frame[:, :, ::-1]

                    face_locations = face_recognition.face_locations(rgb_small_frame)
                    face_encodings = face_recognition.face_encodings(rgb_small_frame, face_locations)

                    if not face_encodings:
                        self.result_queue.put("Unknown")
                        self.access_status_signal.emit("0")

                    face_names = []
                    for face_encoding in face_encodings:
                        matches = face_recognition.compare_faces(known_face_encodings, face_encoding)
                        name = "Unknown"

                        if True in matches:
                            first_match_index = matches.index(True)
                            name = known_face_names[first_match_index]
                            self.access_status_signal.emit("1")
                        else:
                            self.access_status_signal.emit("0")

                        face_names.append(name)
                        self.result_queue.put(name)

                    process_this_frame = not process_this_frame

                    for (top, right, bottom, left), name in zip(face_locations, face_names):
                        top *= 4
                        right *= 4
                        bottom *= 4
                        left *= 4

                        cv2.rectangle(frame, (left, top), (right, bottom), (0, 0, 255), 2)
                        cv2.rectangle(frame, (left, bottom - 35), (right, bottom), (0, 0, 255), cv2.FILLED)
                        font = cv2.FONT_HERSHEY_DUPLEX
                        cv2.putText(frame, name, (left + 6, bottom - 6), font, 0.6, (255, 255, 255), 1)

                self.image_update_signal.emit(frame)

        finally:
            video_capture.release()

    def stop(self):
        self.running = False

class FaceRecognitionApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()
        self.result_queue = queue.Queue()
        self.face_recognition_thread = FaceRecognitionThread(self.result_queue)
        self.face_recognition_thread.image_update_signal.connect(self.update_frame)
        self.face_recognition_thread.access_status_signal.connect(self.update_access_status)
        self.tcp_server = TCPServer(self.result_queue)
        
        self.status_timer = QTimer()
        self.status_timer.timeout.connect(self.reset_status)
    def initUI(self):
        self.setWindowTitle('Face Recognition')
        self.setGeometry(100, 100, 1000, 600)
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        layout = QHBoxLayout()

        # 왼쪽 비디오 영역
        video_layout = QVBoxLayout()
        self.video_label = QLabel()
        self.video_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.video_label.setMinimumSize(800, 600)
        video_layout.addWidget(self.video_label)
        layout.addLayout(video_layout)

        # 오른쪽 상태 영역 (전체 세로 레이아웃)
        status_layout = QVBoxLayout()
        status_layout.setSpacing(10)  # 위젯 사이 간격 조정

        # 상태 라벨들 생성
        status_labels = [
            ('초인종', 'status_doorbell', '#FF0000'),
            ('부저', 'status_buzzer', '#00FF00'),
            ('LED1', 'status_led1', '#FFFF00'),
            ('LED2', 'status_led2', '#0000FF'),
            ('LED3', 'status_led3', '#FFFFFF'),
            ('LED4', 'status_led4', '#FFFFFF')
        ]

        for label_text, attr_name, active_color in status_labels:
            # 각 라벨에 대한 컨테이너 생성
            label_container = QVBoxLayout()
            
            # 프레임 생성 (기본 회색, 원형)
            frame = QLabel()
            frame.setStyleSheet("""
                QLabel {
                    background-color: #e0e0e0;
                    border: 2px solid #999999;
                    border-radius: 50%;
                    min-width: 80px;
                    min-height: 80px;
                }
            """)

            # 상태 텍스트 생성
            status_label = QLabel(label_text)
            status_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
            status_label.setStyleSheet("""
                QLabel {
                    font-size: 12px;
                    font-weight: bold;
                    color: #333333;
                }
            """)

            # 컨테이너에 프레임과 라벨 추가
            label_container.addWidget(frame, alignment=Qt.AlignmentFlag.AlignCenter)
            label_container.addWidget(status_label, alignment=Qt.AlignmentFlag.AlignCenter)
            
            # 메인 상태 레이아웃에 컨테이너 추가
            status_layout.addLayout(label_container)

            # 동적 속성 할당
            setattr(self, f'{attr_name}_frame', frame)
            setattr(self, f'{attr_name}_label', status_label)

        # 버튼들 추가
        button_layout = QVBoxLayout()
        self.start_button = QPushButton('Start Recognition')
        self.stop_button = QPushButton('Stop Recognition')
        
        button_layout.addWidget(self.start_button)
        button_layout.addWidget(self.stop_button)
        
        # 메인 상태 레이아웃에 버튼 레이아웃 추가
        status_layout.addLayout(button_layout)

        # 남은 공간 채우기
        status_layout.addStretch(1)

        # 레이아웃에 추가
        layout.addLayout(status_layout)
        central_widget.setLayout(layout)

    def change_led_status(self, led_name, is_on=True):
        frame = getattr(self, f'{led_name}_frame', None)
        if frame:
            if is_on:
                # 해당 LED의 활성 색상으로 변경
                color_map = {
                    'status_doorbell': '#FF0000',
                    'status_buzzer': '#00FF00',
                    'status_led1': '#FFFF00',
                    'status_led2': '#0000FF',
                    'status_led3': '#FFFFFF',
                    'status_led4': '#FFFFFF'
                }
                frame.setStyleSheet(f"""
                    QLabel {{
                        background-color: {color_map.get(led_name, '#e0e0e0')};
                        border: 2px solid #999999;
                        border-radius: 50%;
                        min-width: 80px;
                        min-height: 80px;
                    }}
                """)
            else:
                # LED 끄기 (회색으로 변경)
                frame.setStyleSheet("""
                    QLabel {
                        background-color: #e0e0e0;
                        border: 2px solid #999999;
                        border-radius: 50%;
                        min-width: 80px;
                        min-height: 80px;
                    }
                """)
    def update_access_status(self, status):
        if status == "1":
            self.status_frame.setStyleSheet("""
                QLabel {
                    background-color: #4CAF50;
                    border: 2px solid #45a049;
                    border-radius: 50px;
                }
            """)
            self.status_text.setText("접근 허가")
            self.status_text.setStyleSheet("QLabel { color: #4CAF50; }")
        else:
            self.status_frame.setStyleSheet("""
                QLabel {
                    background-color: #f44336;
                    border: 2px solid #da190b;
                    border-radius: 50px;
                }
            """)
            self.status_text.setText("접근 거부")
            self.status_text.setStyleSheet("QLabel { color: #f44336; }")
        
        self.status_timer.start(3000)

    def reset_status(self):
        self.status_frame.setStyleSheet("""
            QLabel {
                background-color: #e0e0e0;
                border: 2px solid #999999;
                border-radius: 50px;
            }
        """)
        self.status_text.setText("대기중")
        self.status_text.setStyleSheet("QLabel { color: #333333; }")
        self.status_timer.stop()

    def start_recognition(self):
        self.face_recognition_thread.start()
        self.tcp_server.start()

    def stop_recognition(self):
        self.face_recognition_thread.stop()
        self.tcp_server.stop()

    def update_frame(self, frame):
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        h, w, ch = rgb_frame.shape
        bytes_per_line = ch * w
        qt_image = QImage(rgb_frame.data, w, h, bytes_per_line, QImage.Format.Format_RGB888)
        pixmap = QPixmap.fromImage(qt_image)
        self.video_label.setPixmap(pixmap.scaled(
            self.video_label.size(),
            Qt.AspectRatioMode.KeepAspectRatio,
            Qt.TransformationMode.SmoothTransformation
        ))

    def closeEvent(self, event):
        self.face_recognition_thread.stop()
        self.tcp_server.stop()
        event.accept()

def main():
    app = QApplication(sys.argv)
    face_recognition_app = FaceRecognitionApp()
    face_recognition_app.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    main()
