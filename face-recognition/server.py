import socket
import threading
import face_recognition
import cv2
import numpy as np
import queue
import time
from collections import Counter

# 전역 큐를 사용하여 얼굴 인식 결과를 TCP 서버로 전달
result_queue = queue.Queue()

class TCPServer(threading.Thread):
    def __init__(self, host='0.0.0.0', port=5100):
        super().__init__()
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.bind((host, port))
        self.server_socket.listen(5)
        self.client_sockets = []
        self.client_lock = threading.Lock()
        self.running = True
        
    def handle_client(self, client_socket, client_addr):
        print(f"Client connected from {client_addr[0]}")
        try:
            while self.running:
                try:
                    # 1초 동안 데이터를 모아 빈도를 계산
                    start_time = time.time()
                    counts = Counter()
                    while time.time() - start_time < 1:
                        try:
                            name = result_queue.get_nowait()
                            # Unknown은 "0", 다른 이름은 "1"로 처리
                            result = "0" if name == "Unknown" else "1"
                            counts[result] += 1
                        except queue.Empty:
                            time.sleep(0.1)
                    
                    if counts:
                        # 가장 빈도가 높은 값을 전송
                        most_common = counts.most_common(1)[0][0]
                        client_socket.send(most_common.encode())
                        print(f"Sent: {most_common}")
                    else:
                        # 데이터가 없으면 "0"을 전송
                        client_socket.send(b"0")
                except:
                    break
                    
        except Exception as e:
            print(f"Error handling client: {e}")
        finally:
            with self.client_lock:
                if client_socket in self.client_sockets:
                    self.client_sockets.remove(client_socket)
            client_socket.close()
            print(f"Client disconnected from {client_addr[0]}")
    
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

class FaceRecognition(threading.Thread):
    def __init__(self):
        super().__init__()
        self.running = True
        
    def run(self):
        video_capture = cv2.VideoCapture(0)
        try:
            # 샘플 이미지 로드 및 인코딩
            obama_image = face_recognition.load_image_file("obama.jpg")
            obama_face_encoding = face_recognition.face_encodings(obama_image)[0]
            
            biden_image = face_recognition.load_image_file("biden.jpg")
            biden_face_encoding = face_recognition.face_encodings(biden_image)[0]
            
            mingi_image = face_recognition.load_image_file("mingi.jpg")
            mingi_face_encoding = face_recognition.face_encodings(mingi_image)[0]
            
            known_face_encodings = [
                obama_face_encoding,
                biden_face_encoding,
                mingi_face_encoding
            ]
            known_face_names = [
                "Barack Obama",
                "Joe Biden",
                "mingi"
            ]
            
            process_this_frame = True
            
            while self.running:
                ret, frame = video_capture.read()
                
                if process_this_frame:
                    # 프레임 크기 조정
                    small_frame = cv2.resize(frame, (0, 0), fx=0.25, fy=0.25)
                    rgb_small_frame = small_frame[:, :, ::-1]
                    
                    # 얼굴 위치와 인코딩 찾기
                    face_locations = face_recognition.face_locations(rgb_small_frame)
                    face_encodings = face_recognition.face_encodings(rgb_small_frame, face_locations)
                    
                    # 얼굴이 하나도 없으면 "Unknown" 전송
                    if not face_encodings:
                        result_queue.put("Unknown")
                    
                    face_names = []
                    for face_encoding in face_encodings:
                        # 알려진 얼굴과 비교
                        matches = face_recognition.compare_faces(known_face_encodings, face_encoding)
                        name = "Unknown"
                        
                        # 가장 잘 매칭되는 얼굴 찾기
                        if True in matches:
                            first_match_index = matches.index(True)
                            name = known_face_names[first_match_index]
                            
                        face_names.append(name)
                        # 인식된 이름을 큐에 추가
                        result_queue.put(name)
                
                process_this_frame = not process_this_frame
                
                # 결과 화면에 표시
                for (top, right, bottom, left), name in zip(face_locations, face_names):
                    # 좌표 원래 크기로 변환
                    top *= 4
                    right *= 4
                    bottom *= 4
                    left *= 4
                    
                    # 얼굴 주변에 박스 그리기
                    cv2.rectangle(frame, (left, top), (right, bottom), (0, 0, 255), 2)
                    
                    # 이름 표시를 위한 박스
                    cv2.rectangle(frame, (left, bottom - 35), (right, bottom), (0, 0, 255), cv2.FILLED)
                    font = cv2.FONT_HERSHEY_DUPLEX
                    cv2.putText(frame, name, (left + 6, bottom - 6), font, 0.6, (255, 255, 255), 1)
                
                # 결과 화면 표시
                cv2.imshow('Face Recognition', frame)
                
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    self.running = False
                    break
                
        finally:
            video_capture.release()
            cv2.destroyAllWindows()
    
    def stop(self):
        self.running = False

def main():
    tcp_server = TCPServer()
    face_recognition_thread = FaceRecognition()
    
    tcp_server.start()
    face_recognition_thread.start()
    
    try:
        while face_recognition_thread.is_alive():
            face_recognition_thread.join(0.1)
    except KeyboardInterrupt:
        print("\nShutting down...")
    finally:
        face_recognition_thread.stop()
        tcp_server.stop()
        face_recognition_thread.join()
        tcp_server.join()

if __name__ == "__main__":
    main()

