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
import datetime
import pandas as pd
import base64
from PySide6.QtWidgets import (QApplication, QVBoxLayout, QLabel, QHBoxLayout, QPushButton, QWidget, QMainWindow)
from PySide6.QtGui import QImage, QPixmap, QMovie
from PySide6.QtCore import Qt, QThread, Signal, QTimer, QObject, Slot, QMutex, QFile, QIODevice
from face_recognition_ui import Ui_FaceRecognitionWindow

class TCPServer(QObject):
    emit_tcp_signal = Signal(bytearray)
    def __init__(self, result_queue, logListWidget, parent=None, host='0.0.0.0', port=5100):
        super().__init__()
        self.logListWidget = logListWidget 
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
            self.logListWidget.addItem(f"TCP Server waiting for connections on {self.host}:{self.port}")
            while self.running:
                try:
                    client_socket, client_addr = self.server_socket.accept()
                    print(f"Client connected from {client_addr[0]}")
                    self.logListWidget.addItem(f"Client connected from {client_addr[0]}")
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
    cropped_image_update_signal = Signal(np.ndarray)
    image_cropping_signal = Signal(np.ndarray)
    def __init__(self, result_queue, cropping):
        super().__init__()
        self.result_queue = result_queue
        self.running = True
        self.video_capture = None
        self.cropping = cropping

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

                    if self.cropping:
                        cropped_face = frame[top:bottom, left:right]
                        self.cropped_image_update_signal.emit(cropped_face)
                        self.cropping = False

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
        self.wrong_passwd_count = 0
        self.cropping = False
        self.log_dataframe = pd.DataFrame()
        self.logListWidget.itemClicked.connect(self.show_log_image)
        self.setup_bell_icon()
        self.setup_door_icon()
        self.setup_siren_icon()

        self.result_queue = queue.Queue()
        self.face_recognition_thread = QThread()
        self.face_recognition_worker = FaceRecognitionThread(self.result_queue, self.cropping)
        self.face_recognition_worker.moveToThread(self.face_recognition_thread)
        self.face_recognition_thread.started.connect(self.face_recognition_worker.run)
        self.face_recognition_worker.image_update_signal.connect(self.update_frame)
        self.face_recognition_worker.image_cropping_signal.connect(self.receive_cropping_signal)
        self.face_recognition_worker.cropped_image_update_signal.connect(self.display_cropped_image)

        self.tcp_thread = QThread()
        self.tcp_server = TCPServer(result_queue=self.result_queue, logListWidget = self.logListWidget)
        self.tcp_server.moveToThread(self.tcp_thread)
        self.tcp_thread.started.connect(self.tcp_server.run)
        self.tcp_server.emit_tcp_signal.connect(self.function_mapping)

        # 버튼 연결
        self.start_button.clicked.connect(self.start_recognition)
        self.stop_button.clicked.connect(self.stop_recognition)
    
    def frame_to_base64(self, frame):
        if frame is None:
            return None
        _, buffer = cv2.imencode('.jpg', frame)
        return base64.b64encode(buffer).decode('utf-8')

    def receive_cropping_signal(self, boolean):
        self.face_recognition_worker.cropping = boolean

    def display_cropped_image(self, image):
        base64_method = {
            'log_message': self.log_message,
            'image': self.frame_to_base64(image)
        }

        # 파일이 존재하면 기존 데이터 로드
        if os.path.isfile('logs/result.csv'):
            self.log_dataframe = pd.read_csv('logs/result.csv')
        else:
            # 존재하지 않을 경우 빈 데이터프레임 생성
            self.log_dataframe = pd.DataFrame(columns=['log_message', 'image'])
        
        # 새 행을 데이터프레임으로 변환
        new_row = pd.DataFrame([base64_method])

        # 기존 데이터프레임과 새 행 결합
        self.log_dataframe = pd.concat([self.log_dataframe, new_row], ignore_index=True)

        os.makedirs('logs/', exist_ok = True)

        if self.log_dataframe is not None:
            self.log_dataframe.to_csv('logs/result.csv', index=False)
    
    def show_log_image(self, item):
        index = self.logListWidget.row(item)
        data_frame = pd.read_csv('logs/result.csv', index_col = False)
        selected_item_text = self.logListWidget.item(index).text()
        print(f"show log: {selected_item_text}")
        matching_row = data_frame[data_frame['log_message'] == selected_item_text]
        if not matching_row.empty:
            # base64 인코딩된 이미지 디코딩
            base64_image = matching_row['image'].values[0]
            decoded_image = base64.b64decode(base64_image)
            
            # 이미지를 numpy 배열로 변환
            nparr = np.frombuffer(decoded_image, np.uint8)
            image = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

            # OpenCV 이미지를 QPixmap으로 변환
            height, width, channel = image.shape
            bytes_per_line = 3 * width
            q_image = QImage(image.data, width, height, bytes_per_line, QImage.Format_RGB888).rgbSwapped()
            pixmap = QPixmap.fromImage(q_image)
            
            # 비디오 라벨에 이미지 표시
            self.video_label_2.setPixmap(pixmap.scaled(self.video_label_2.size(), Qt.KeepAspectRatio, Qt.SmoothTransformation))

    def setup_bell_icon(self):
        self.active_bell_image_path = 'asset/icon_bell_active.gif'
        self.idle_bell_image_path = 'asset/icon_bell_idle.png'

        self.bell_button = self.findChild(QPushButton, 'bell_button')

        # 파일 존재 여부 확인
        if os.path.exists(self.idle_bell_image_path):
            # LED 대신 GIF 설정
            idle_pixmap = QPixmap(self.idle_bell_image_path)
            scaled_pixmap = idle_pixmap.scaled(
                self.icon_bell.size(), 
                Qt.KeepAspectRatio, 
                Qt.SmoothTransformation
            )
            self.icon_bell.setPixmap(scaled_pixmap)
        else:
            print(f"jpg 파일을 찾을 수 없습니다: {self.active_bell_image_path}")
        
        # 활성화 상태 GIF 준비
        if os.path.exists(self.active_bell_image_path):
            self.icon_bell_movie = QMovie(self.active_bell_image_path)
            self.icon_bell_movie.setScaledSize(self.icon_bell.size())
        else:
            print(f"GIF 파일을 찾을 수 없습니다: {self.active_bell_image_path}")
            self.icon_bell_movie = None
        
        self.bell_button.clicked.connect(self.start_bell_animation)

    def setup_door_icon(self):
        self.active_door_image_path = 'asset/icon_door_active.gif'
        self.idle_door_image_path = 'asset/icon_door_idle.png'

        self.door_button = self.findChild(QPushButton, 'door_button')

        # 파일 존재 여부 확인
        if os.path.exists(self.idle_door_image_path):
            # LED 대신 GIF 설정
            idle_pixmap = QPixmap(self.idle_door_image_path)
            scaled_pixmap = idle_pixmap.scaled(
                self.icon_door.size(), 
                Qt.KeepAspectRatio, 
                Qt.SmoothTransformation
            )
            self.icon_door.setPixmap(scaled_pixmap)
        else:
            print(f"jpg 파일을 찾을 수 없습니다: {self.active_door_image_path}")
        
        # 활성화 상태 GIF 준비
        if os.path.exists(self.active_door_image_path):
            self.icon_door_movie = QMovie(self.active_door_image_path)
            self.icon_door_movie.setScaledSize(self.icon_door.size())
        else:
            print(f"GIF 파일을 찾을 수 없습니다: {self.active_door_image_path}")
            self.icon_door_movie = None
        
        self.door_button.clicked.connect(self.start_door_animation)

    def setup_siren_icon(self):
        self.active_siren_red_image_path = 'asset/icon_siren_active.gif'
        self.active_siren_blue_image_path = 'asset/icon_siren_blue.gif'
        self.active_siren_green_image_path = 'asset/icon_siren_green.gif'
        self.idle_siren_image_path = 'asset/icon_siren_idle.png'

        self.siren_button = self.findChild(QPushButton, 'siren_button')

        # 파일 존재 여부 확인
        if os.path.exists(self.idle_siren_image_path):
            # LED 대신 GIF 설정
            idle_pixmap = QPixmap(self.idle_siren_image_path)
            scaled_pixmap = idle_pixmap.scaled(
                self.icon_siren.size(), 
                Qt.KeepAspectRatio, 
                Qt.SmoothTransformation
            )
            self.icon_siren.setPixmap(scaled_pixmap)
        else:
            print(f"jpg 파일을 찾을 수 없습니다: {self.active_siren_red_image_path}")
        
        # 활성화 상태 GIF 준비
        if os.path.exists(self.active_siren_red_image_path):
            self.icon_siren_red_movie = QMovie(self.active_siren_red_image_path)
            self.icon_siren_red_movie.setScaledSize(self.icon_siren.size())
        else:
            print(f"GIF 파일을 찾을 수 없습니다: {self.active_siren_red_image_path}")
            self.icon_siren_red_movie = None

        # 활성화 상태 GIF 준비
        if os.path.exists(self.active_siren_blue_image_path):
            self.icon_siren_blue_movie = QMovie(self.active_siren_blue_image_path)
            self.icon_siren_blue_movie.setScaledSize(self.icon_siren.size())
        else:
            print(f"GIF 파일을 찾을 수 없습니다: {self.active_siren_blue_image_path}")
            self.icon_siren_blue_movie = None

        # 활성화 상태 GIF 준비
        if os.path.exists(self.active_siren_green_image_path):
            self.icon_siren_green_movie = QMovie(self.active_siren_green_image_path)
            self.icon_siren_green_movie.setScaledSize(self.icon_siren.size())
        else:
            print(f"GIF 파일을 찾을 수 없습니다: {self.active_siren_green_image_path}")
            self.icon_siren_green_movie = None

        self.siren_button.clicked.connect(self.start_red_siren_animation)

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
    
    def function_mapping(self, tcp_received_signal):
        self.log_message = ""
        received = int(chr(tcp_received_signal[0]))
        current_time = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        try:
            if received == 0:
                self.start_bell_animation()
                self.start_red_siren_animation()
                message = "미등록 출입자가 출입을 시도하였습니다."
            elif received == 1:
                self.wrong_passwd_count += 1
                self.start_bell_animation()
                if(self.wrong_passwd_count >= 3):
                    message = "출입자가 3번이상 비밀번호를 틀렸습니다."
                    self.start_red_siren_animation()
                    self.wrong_passwd_count = 0
                else:
                    message = "출입자가 입력한 비밀번호가 틀렸습니다."
                    self.start_blue_siren_animation()
            elif received == 2:
                self.start_bell_animation()
                self.start_red_siren_animation()
                message = "미등록 출입자가 출입을 시도하였습니다."
            elif received == 3:
                self.start_bell_animation()
                self.start_door_animation()
                self.start_green_siren_animation()
                message = "출입문이 개방되었습니다."
            elif received == 4:
                self.start_bell_animation()
                self.start_door_animation()
                self.start_green_siren_animation()
                message = "출입문이 자동 잠금되었습니다."
            elif received == 5:
                self.start_bell_animation()
                message = "미등록 출입자가 벨을 호출하였습니다."
        except Exception as e:
            print(f"TCP Received Error: {e}")
        
        self.log_message = f"[{received}][{current_time}][{message}]"
        self.logListWidget.addItem(self.log_message)
        self.face_recognition_worker.image_cropping_signal.emit(True)
        

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
                lambda frame: self.stop_bell_animation_at_last_frame(frame, total_frames)
            )

    def stop_bell_animation_at_last_frame(self, current_frame, total_frames):
        if current_frame == total_frames - 1:
            self.icon_bell_movie.stop()
            
            # 이전 연결 해제 (메모리 누수 방지)
            try:
                self.icon_bell_movie.frameChanged.disconnect()
            except TypeError:
                pass
            
            # idle 상태로 되돌리기
            self.reset_bell_to_idle()

    def reset_bell_to_idle(self):
        # idle 이미지로 되돌리기
        idle_pixmap = QPixmap(self.idle_bell_image_path)
        scaled_pixmap = idle_pixmap.scaled(
            self.icon_bell.size(), 
            Qt.KeepAspectRatio, 
            Qt.SmoothTransformation
        )
        self.icon_bell.setPixmap(scaled_pixmap)

    def start_door_animation(self):
        if self.icon_door_movie:
            # 현재 프레임 수 확인
            total_frames = self.icon_door_movie.frameCount()
            
            # 첫 프레임으로 리셋
            self.icon_door_movie.jumpToFrame(0)
            
            # GIF 애니메이션 시작
            self.icon_door.setMovie(self.icon_door_movie)
            self.icon_door_movie.start()
            
            # 마지막 프레임에 도달하면 정지 및 idle 상태로 전환
            self.icon_door_movie.frameChanged.connect(
                lambda frame: self.stop_door_animation_at_last_frame(frame, total_frames)
            )

    def stop_door_animation_at_last_frame(self, current_frame, total_frames):
        if current_frame == total_frames - 1:
            self.icon_door_movie.stop()
            
            # 이전 연결 해제 (메모리 누수 방지)
            try:
                self.icon_door_movie.frameChanged.disconnect()
            except TypeError:
                pass
            
            # idle 상태로 되돌리기
            self.reset_door_to_idle()

    def reset_door_to_idle(self):
        # idle 이미지로 되돌리기
        idle_pixmap = QPixmap(self.idle_door_image_path)
        scaled_pixmap = idle_pixmap.scaled(
            self.icon_door.size(), 
            Qt.KeepAspectRatio, 
            Qt.SmoothTransformation
        )
        self.icon_door.setPixmap(scaled_pixmap)

    def start_red_siren_animation(self):
        if self.icon_siren_red_movie:
            # 현재 프레임 수 확인
            total_frames = self.icon_siren_red_movie.frameCount()
            
            # 첫 프레임으로 리셋
            self.icon_siren_red_movie.jumpToFrame(0)
            
            # GIF 애니메이션 시작
            self.icon_siren.setMovie(self.icon_siren_red_movie)
            self.icon_siren_red_movie.start()
            
            # 마지막 프레임에 도달하면 정지 및 idle 상태로 전환
            self.icon_siren_red_movie.frameChanged.connect(
                lambda frame: self.stop_red_siren_animation_at_last_frame(frame, total_frames)
            )

    def stop_red_siren_animation_at_last_frame(self, current_frame, total_frames):
        if current_frame == total_frames - 1:
            self.icon_siren_red_movie.stop()
            
            # 이전 연결 해제 (메모리 누수 방지)
            try:
                self.icon_siren_red_movie.frameChanged.disconnect()
            except TypeError:
                pass
            
            # idle 상태로 되돌리기
            self.reset_siren_to_idle()

    def start_blue_siren_animation(self):
        if self.icon_siren_blue_movie:
            # 현재 프레임 수 확인
            total_frames = self.icon_siren_blue_movie.frameCount()
            
            # 첫 프레임으로 리셋
            self.icon_siren_blue_movie.jumpToFrame(0)
            
            # GIF 애니메이션 시작
            self.icon_siren.setMovie(self.icon_siren_blue_movie)
            self.icon_siren_blue_movie.start()
            
            # 마지막 프레임에 도달하면 정지 및 idle 상태로 전환
            self.icon_siren_blue_movie.frameChanged.connect(
                lambda frame: self.stop_blue_siren_animation_at_last_frame(frame, total_frames)
            )

    def stop_blue_siren_animation_at_last_frame(self, current_frame, total_frames):
        if current_frame == total_frames - 1:
            self.icon_siren_blue_movie.stop()
            
            # 이전 연결 해제 (메모리 누수 방지)
            try:
                self.icon_siren_blue_movie.frameChanged.disconnect()
            except TypeError:
                pass
            
            # idle 상태로 되돌리기
            self.reset_siren_to_idle()

    def start_green_siren_animation(self):
        if self.icon_siren_green_movie:
            # 현재 프레임 수 확인
            total_frames = self.icon_siren_green_movie.frameCount()
            
            # 첫 프레임으로 리셋
            self.icon_siren_green_movie.jumpToFrame(0)
            
            # GIF 애니메이션 시작
            self.icon_siren.setMovie(self.icon_siren_green_movie)
            self.icon_siren_green_movie.start()
            
            # 마지막 프레임에 도달하면 정지 및 idle 상태로 전환
            self.icon_siren_green_movie.frameChanged.connect(
                lambda frame: self.stop_green_siren_animation_at_last_frame(frame, total_frames)
            )

    def stop_green_siren_animation_at_last_frame(self, current_frame, total_frames):
        if current_frame == total_frames - 1:
            self.icon_siren_green_movie.stop()
            
            # 이전 연결 해제 (메모리 누수 방지)
            try:
                self.icon_siren_green_movie.frameChanged.disconnect()
            except TypeError:
                pass
            
            # idle 상태로 되돌리기
            self.reset_siren_to_idle()

    def reset_siren_to_idle(self):
        # idle 이미지로 되돌리기
        idle_pixmap = QPixmap(self.idle_siren_image_path)
        scaled_pixmap = idle_pixmap.scaled(
            self.icon_siren.size(), 
            Qt.KeepAspectRatio, 
            Qt.SmoothTransformation
        )
        self.icon_siren.setPixmap(scaled_pixmap)

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
