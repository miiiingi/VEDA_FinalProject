import os
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
from PySide6.QtWidgets import (QApplication, QVBoxLayout, QLabel, QHBoxLayout, QPushButton, QWidget, QMainWindow)
from PySide6.QtGui import QImage, QPixmap, QMovie
from PySide6.QtCore import Qt, QThread, Signal, QTimer, QObject, Slot, QMutex, QFile, QIODevice
from face_recognition_ui import Ui_FaceRecognitionWindow

class TCPServer(QObject):
    emit_tcp_signal = Signal(bytearray)
    def __init__(self, result_queue, parent=None, host='0.0.0.0', port=5100):
        super().__init__()
        self.server_socket = None
        self.host = host
        self.port = port
        self.client_sockets = []
        self.client_lock = QMutex()
        self.running = True
        self.result_queue = result_queue

    def setup_socket(self):
        try:
            self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.server_socket.bind((self.host, self.port))
            self.server_socket.listen(5)
            self.server_socket.settimeout(1)  # 타임아웃 설정

        except Exception as e:
            print(f"Socket setup error: {e}")
            self.close_resources()
            raise

    def receive_data(self, client_socket):
        BUFFER_SIZE=1024
        while self.running:
            try:
                data = client_socket.recv(BUFFER_SIZE)
                if not data:
                    print("From Client Socket, Not Received Data")
                    break
                    
                byte_array = bytearray(data)
                self.emit_tcp_signal.emit(byte_array)
            except:
                break

    def send_data(self, client_socket):
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

    def run(self):
        try:
            self.setup_socket()
            print(f"TCP Server waiting for connections on {self.host}:{self.port}")
            while self.running:
                try:
                    client_socket, client_addr = self.server_socket.accept()
                    print(f"Client connected from {client_addr[0]}")
                    self.client_lock.lock()
                    self.client_sockets.append(client_socket)
                    self.client_lock.unlock()
                    
                    receive_worker = QThread()
                    send_worker = QThread()
                    
                    receive_worker.run = lambda: self.receive_data(client_socket)
                    send_worker.run = lambda: self.send_data(client_socket)
                    
                    receive_worker.start()
                    send_worker.start()
                    
                except socket.timeout:
                    # 타임아웃은 무시하고 계속 대기
                    continue

                except Exception as e:
                    print(f"Connection error: {e}")
                    break

        except Exception as e:
            print(f"Server run error: {e}")
        finally:
            self.close_resources()

    def close_resources(self):
        self.running = False
        self.client_lock.lock()
        for sock in self.client_sockets:
            try:
                sock.shutdown(socket.SHUT_RDWR)
                sock.close()
            except Exception as e:
                print(f"Error closing client socket: {e}")
        self.client_sockets.clear()
        self.client_lock.unlock()
        
        if self.server_socket:
            try:
                self.server_socket.close()
            except Exception as e:
                print(f"Error closing server socket: {e}")

    def __del__(self):
        self.close_resources()

    def stop(self):
        self.running = False
        self.client_lock.lock()
        for sock in self.client_sockets:
            sock.close()
        self.client_lock.unlock()
        self.server_socket.close()

class FaceRecognitionThread(QObject):
    image_update_signal = Signal(np.ndarray)
    def __init__(self, result_queue):
        super().__init__()
        self.result_queue = result_queue
        self.running = True
        self.video_capture = None

    @Slot()
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

                    face_names = []
                    for face_encoding in face_encodings:
                        matches = face_recognition.compare_faces(known_face_encodings, face_encoding)
                        name = "Unknown"

                        if True in matches:
                            first_match_index = matches.index(True)
                            name = known_face_names[first_match_index]

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

                frame = cv2.resize(frame, (320, 240))
                self.image_update_signal.emit(frame)
                
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    self.running = False
                    break

        except Exception as e:
            print(f"Face recognition initialization error: {e}")
        finally:
            video_capture.release()
            cv2.destroyAllWindows()

    def stop(self):
        self.running = False

class FaceRecognitionApp(QMainWindow, Ui_FaceRecognitionWindow):
    def __init__(self):
        super().__init__()
        self.setupUi(self)

        self.gif_path = 'asset/icon_bell_active.gif'
        self.idle_image_path = 'asset/icon_bell_idle.png'

        self.test_button = self.findChild(QPushButton, 'test_button')

        # 파일 존재 여부 확인
        if os.path.exists(self.idle_image_path):
            # LED 대신 GIF 설정
            idle_pixmap = QPixmap(self.idle_image_path)
            scaled_pixmap = idle_pixmap.scaled(
                self.icon_bell.size(), 
                Qt.KeepAspectRatio, 
                Qt.SmoothTransformation
            )
            self.icon_bell.setPixmap(scaled_pixmap)
        else:
            print(f"jpg 파일을 찾을 수 없습니다: {self.gif_path}")
        
        # 활성화 상태 GIF 준비
        if os.path.exists(self.gif_path):
            self.icon_bell_movie = QMovie(self.gif_path)
            self.icon_bell_movie.setScaledSize(self.icon_bell.size())
        else:
            print(f"GIF 파일을 찾을 수 없습니다: {self.gif_path}")
            self.icon_bell_movie = None
        
        self.test_button.clicked.connect(self.start_bell_animation)

        # Frameless 속성 설정
        self.result_queue = queue.Queue()
        self.face_recognition_thread = QThread()
        self.face_recognition_worker = FaceRecognitionThread(self.result_queue)
        self.face_recognition_worker.moveToThread(self.face_recognition_thread)
        self.face_recognition_thread.started.connect(self.face_recognition_worker.run)
        self.face_recognition_worker.image_update_signal.connect(self.update_frame)

        self.tcp_thread = QThread()
        self.tcp_server = TCPServer(result_queue=self.result_queue)
        self.tcp_server.moveToThread(self.tcp_thread)
        self.tcp_thread.started.connect(self.tcp_server.run)
        self.tcp_server.emit_tcp_signal.connect(self.change_led_status)

        # 버튼 연결
        self.start_button.clicked.connect(self.start_recognition)
        self.stop_button.clicked.connect(self.stop_recognition)

    def start_bell_animation(self):
        if self.icon_bell_movie:
            # 현재 프레임 수 확인
            total_frames = self.icon_bell_movie.frameCount()
            
            # 첫 프레임으로 리셋
            self.icon_bell_movie.jumpToFrame(0)
            
            # GIF 애니메이션 시작
            self.icon_bell.setMovie(self.icon_bell_movie)
            self.icon_bell_movie.start()
            
            # 마지막 프레임에 도달하면 정지 및 idle 상태로 전환
            self.icon_bell_movie.frameChanged.connect(
                lambda frame: self.stop_animation_at_last_frame(frame, total_frames)
            )

    def stop_animation_at_last_frame(self, current_frame, total_frames):
        if current_frame == total_frames - 1:
            self.icon_bell_movie.stop()
            
            # 이전 연결 해제 (메모리 누수 방지)
            try:
                self.icon_bell_movie.frameChanged.disconnect()
            except TypeError:
                pass
            
            # idle 상태로 되돌리기
            self.reset_to_idle()

    def reset_to_idle(self):
        # idle 이미지로 되돌리기
        idle_pixmap = QPixmap(self.idle_image_path)
        scaled_pixmap = idle_pixmap.scaled(
            self.icon_bell.size(), 
            Qt.KeepAspectRatio, 
            Qt.SmoothTransformation
        )
        self.icon_bell.setPixmap(scaled_pixmap)

    def start_recognition(self):
        if not self.face_recognition_thread.isRunning():
            self.face_recognition_thread.start()
        if not self.tcp_thread.isRunning():
            self.tcp_thread.start()
        self.start_button.setEnabled(False)
        self.stop_button.setEnabled(True)

    def stop_recognition(self):
        if self.face_recognition_thread and self.face_recognition_thread.is_alive():
            self.face_recognition_thread.stop()
            self.face_recognition_thread.quit()
            self.face_recognition_thread.wait()
            self.face_recognition_thread.join(timeout=2)

        if self.tcp_server and self.tcp_server.is_alive():
            self.tcp_server.close_resources()
            self.tcp_server.join(timeout=2)
            self.start_button.setEnabled(True)
            self.stop_button.setEnabled(False)

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

    # 클래스 메서드로 변경
    def convert_signal(self, tcp_received_signal):
        signal_map = {
            1: 'status_doorbell',
            2: 'status_buzzer',
            3: 'status_led1',
            4: 'status_led2',
            5: 'status_led3',
            6: 'status_led4'
        }
        return signal_map.get(tcp_received_signal, 'status_off')
    
    def change_led_status(self, tcp_received_signal):
        print(f"tcp received signal: {int(chr(tcp_received_signal[0]))}")
        converted_signal = self.convert_signal(int(chr(tcp_received_signal[0])))
        print(f"converted signal: {converted_signal}")
        is_on = converted_signal != 'status_off'
        
        frame = getattr(self, f'{converted_signal}_frame', None)
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
                        background-color: {color_map.get(converted_signal, '#e0e0e0')};
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

    def closeEvent(self, event):
        pass
        # 사용자가 윈도우를 닫을 때 호출되는 메서드
        # reply = QMessageBox.question(
        #     self,
        #     '종료 확인',
        #     '애플리케이션을 종료하시겠습니까?',
        #     QMessageBox.Yes | QMessageBox.No,
        #     QMessageBox.No
        # )

        # if reply == QMessageBox.Yes:
        #     try:
        #         # 스레드 정지
        #         if self.face_recognition_thread and self.face_recognition_thread.is_alive():
        #             self.face_recognition_thread.stop()
        #             self.face_recognition_thread.join(timeout=2)

        #         if self.tcp_server and self.tcp_server.is_alive():
        #             self.tcp_server.close_resources()
        #             self.tcp_server.join(timeout=2)

        #     except Exception as e:
        #         print(f"Error during application closure: {e}")
            
        #     event.accept()
        # else:
            # event.ignore()

def main():
    app = QApplication(sys.argv)
    face_recognition_app = FaceRecognitionApp()
    face_recognition_app.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    main()
