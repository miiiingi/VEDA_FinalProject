#include "face_recognition.h"
#include <QDateTime>
#include <QPixmap>
#include <QFile>
#include <QMessageBox>
#include <QCloseEvent>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include <QElapsedTimer>
#include <chrono>

#include <dlib/image_processing/frontal_face_detector.h>  // 얼굴 검출기
#include <dlib/image_processing.h>
#include <dlib/image_io.h>
#include <dlib/dnn.h>
#include <dlib/opencv.h>


FaceRecognitionWorker::FaceRecognitionWorker(QObject *parent) : QObject(parent), running(false) {}

void FaceRecognitionWorker::loadModels() {
    faceDetector = get_frontal_face_detector();
    deserialize("../asset/dlib_face_recognition_resnet_model_v1.dat") >> faceRecognitionModel;
    deserialize("../asset/shape_predictor_68_face_landmarks.dat") >> shapePredictor;
}

std::vector<dlib::rectangle> FaceRecognitionWorker::getOpenCVFaceEncoding(const cv::Mat& frame, frontal_face_detector& faceDetector,
                                        shape_predictor& shapePredictor, anet_type& faceRecognitionModel) {
    cv::Mat resizedFrame;
    cv::resize(frame, resizedFrame, cv::Size(), 0.25, 0.25);
    cv::cvtColor(resizedFrame, resizedFrame, cv::COLOR_BGR2RGB);
    dlib::cv_image<dlib::rgb_pixel> cimg(resizedFrame);

    return faceDetector(cimg);
}

void FaceRecognitionWorker::startRecognition() {
    running = true;
    cv::VideoCapture videoCapture(0);
    if (!videoCapture.isOpened()) {
        emit errorOccurred("Failed to open camera.");
        return;
    }
    std::cout << dlib::cuda::get_num_devices() << " CUDA devices detected." << std::endl;
    loadModels();
    int frameCount = 0;
    
    // QElapsedTimer 대신 std::chrono 사용 (Qt가 없는 경우를 위한 대체 방법)
    auto timer_start = std::chrono::steady_clock::now();
    
    while (running) {
        cv::Mat frame;
        if (!videoCapture.read(frame)) {
            emit errorOccurred("Failed to read frame.");
            break;
        }
        if (process_this_frame){
            std::vector<dlib::rectangle> faces = getOpenCVFaceEncoding(frame, faceDetector, shapePredictor, faceRecognitionModel);
            for(auto face: faces){
                int left = face.left() * 4;
                int top = face.top() * 4;
                int right = face.right() * 4;
                int bottom = face.bottom() * 4;
                std::cout << "Face found at x: " << left << " y: " << top << std::endl;
                // Draw rectangle around the face
                cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), cv::Scalar(0, 0, 255), 4); // Increased thickness
                cv::rectangle(frame, cv::Point(left, bottom - 35), cv::Point(right, bottom), cv::Scalar(0, 0, 255), cv::FILLED);
                // Add text label
                std::string name = "mingi"; // Placeholder for name
                int baseline = 0;
                cv::putText(frame, name, cv::Point(left + 6, bottom - 6), cv::FONT_HERSHEY_DUPLEX, 1.2, cv::Scalar(255, 255, 255), 2); // Increased font size and thickness
            }
            cv::resize(frame, frame, cv::Size(320, 240));
            emit frameUpdated(frame);
        }
        frameCount++;
        
        // FPS 계산을 위한 시간 측정
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - timer_start).count();
        
        if (elapsed >= 1000) { // 1초가 지났다면
            double fps = frameCount / (elapsed / 1000.0);
            std::cout << "FPS: " << fps << std::endl;
            frameCount = 0;
            timer_start = current_time;
        }
    }
    videoCapture.release();
}

void FaceRecognitionWorker::stopRecognition() {
    running = false;
}

FaceRecognitionWindow::FaceRecognitionWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FaceRecognitionWindow)
    , worker(new FaceRecognitionWorker)
{
    qRegisterMetaType<cv::Mat>("cv::Mat");
    ui->setupUi(this);
    setupIcons();
    setupConnections();
    worker->moveToThread(&workerThread);

    // 초기 설정
    ui->stop_button->setEnabled(false);
    ui->video_label->setText("Camera Off");
    ui->video_label_2->setText("Logging View");
}

FaceRecognitionWindow::~FaceRecognitionWindow()
{
    workerThread.quit();
    workerThread.wait();
    delete worker;
    delete ui;
}

void FaceRecognitionWindow::setupConnections() {
    connect(worker, &FaceRecognitionWorker::frameUpdated, this, &FaceRecognitionWindow::displayFrame);
    connect(worker, &FaceRecognitionWorker::errorOccurred, this, &FaceRecognitionWindow::showError);
    connect(&workerThread, &QThread::started, worker, &FaceRecognitionWorker::startRecognition);
    connect(&workerThread, &QThread::finished, worker, &FaceRecognitionWorker::stopRecognition);
}

void FaceRecognitionWindow::setupIcons()
{
    // 아이콘 설정
    QString idleDoorImagePath = "../asset/icon_door_idle.png";
    QString idleBellImagePath = "../asset/icon_bell_idle.png";
    QString idleSirenImagePath = "../asset/icon_siren_idle.png";

    // QPixmap으로 리소스 파일을 직접 로드하여 실패 여부 확인
    QPixmap idleDoorPixmap(idleDoorImagePath);
    if (idleDoorPixmap.isNull()) {
        QMessageBox::warning(this, "Error", "Idle door image not found!");
    } else {
        // 크기 조정 및 스무스하게 변환
        QPixmap scaledPixmap = idleDoorPixmap.scaled(
            ui->icon_door->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
        ui->icon_door->setPixmap(scaledPixmap);
    }
    // QPixmap으로 리소스 파일을 직접 로드하여 실패 여부 확인
    QPixmap idleBellPixmap(idleBellImagePath);
    if (idleBellPixmap.isNull()) {
        QMessageBox::warning(this, "Error", "Idle bell image not found!");
    } else {
        // 크기 조정 및 스무스하게 변환
        QPixmap scaledPixmap = idleBellPixmap.scaled(
            ui->icon_door->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
        ui->icon_bell->setPixmap(scaledPixmap);
    }
    // QPixmap으로 리소스 파일을 직접 로드하여 실패 여부 확인
    QPixmap idleSirenPixmap(idleSirenImagePath);
    if (idleSirenPixmap.isNull()) {
        QMessageBox::warning(this, "Error", "Idle siren image not found!");
    } else {
        // 크기 조정 및 스무스하게 변환
        QPixmap scaledPixmap = idleSirenPixmap.scaled(
            ui->icon_door->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
        ui->icon_siren->setPixmap(scaledPixmap);
    }
}

void FaceRecognitionWindow::on_start_button_clicked()
{
    workerThread.start();
    ui->start_button->setEnabled(false);
    ui->stop_button->setEnabled(true);
    updateLog("시스템 시작");
}

void FaceRecognitionWindow::displayFrame(const cv::Mat &frame) {
    cv::Mat rgbFrame;
    cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);
    QImage qImage(rgbFrame.data, rgbFrame.cols, rgbFrame.rows, rgbFrame.step, QImage::Format_RGB888);
    ui->video_label->setPixmap(QPixmap::fromImage(qImage));
}

void FaceRecognitionWindow::on_stop_button_clicked()
{
    ui->start_button->setEnabled(true);
    ui->stop_button->setEnabled(false);
    updateLog("시스템 정지");
}


void FaceRecognitionWindow::showError(const QString &error) {
    QMessageBox::critical(this, "Error", error);
}

void FaceRecognitionWindow::updateLog(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->logListWidget->addItem(timestamp + ": " + message);
}

void FaceRecognitionWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Q) {
        close();  // Q 키를 누르면 프로그램 종료
    } else {
        QMainWindow::keyPressEvent(event);  // 기본 이벤트 처리
    }
}

void FaceRecognitionWindow::closeEvent(QCloseEvent *event)
{
    // 종료 확인 메시지
    QMessageBox::StandardButton resBtn = QMessageBox::question(
        this, "Exit",
        tr("Are you sure you want to exit?\n"),
        QMessageBox::No | QMessageBox::Yes,
        QMessageBox::Yes);

    if (resBtn == QMessageBox::Yes) {
        worker->stopRecognition();
        workerThread.quit();
        workerThread.wait();
        event->accept();
        QCoreApplication::quit();
    } else {
        event->ignore();
    }
}


