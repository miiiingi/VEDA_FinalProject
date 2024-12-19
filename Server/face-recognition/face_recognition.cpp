#include "face_recognition.h"
#include <QDateTime>
#include <QPixmap>
#include <QFile>
#include <QMessageBox>
#include <QCloseEvent>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <vector>

#include <dlib/image_processing/frontal_face_detector.h>  // 얼굴 검출기
#include <dlib/image_processing.h>
#include <dlib/image_io.h>
#include <dlib/dnn.h>


FaceRecognitionWorker::FaceRecognitionWorker(QObject *parent) : QObject(parent), running(false) {}

void FaceRecognitionWorker::loadModels() {
    faceDetector = get_frontal_face_detector();
    deserialize("../asset/dlib_face_recognition_resnet_model_v1.dat") >> faceRecognitionModel;
    deserialize("../asset/shape_predictor_68_face_landmarks.dat") >> shapePredictor;
}

std::vector<dlib::matrix<dlib::rgb_pixel>> FaceRecognitionWorker::getFaceEncoding(const std::string& imagePath, frontal_face_detector& faceDetector,
                                        shape_predictor& shapePredictor, anet_type& faceRecognitionModel) {
        dlib::matrix<dlib::rgb_pixel> image;
        load_image(image, imagePath);
        auto faces = faceDetector(image);
        if (faces.size() == 0) {
            return {};
        } else {
                auto shape = shapePredictor(image, faces[0]);
                matrix<rgb_pixel> face_chip;
                extract_image_chip(image, get_face_chip_details(shape,150,0.25), face_chip);
                return {face_chip}; 
        }
}
std::vector<dlib::rectangle> FaceRecognitionWorker::getOpenCVFaceEncoding(const cv::Mat& frame, frontal_face_detector& faceDetector,
                                        shape_predictor& shapePredictor, anet_type& faceRecognitionModel) {
    cv::Mat resizedFrame;
    cv::resize(frame, resizedFrame, cv::Size(), 0.25, 0.25);
    cv::cvtColor(resizedFrame, resizedFrame, cv::COLOR_BGR2RGB);
    dlib::matrix<dlib::rgb_pixel> dlibImage(resizedFrame.rows, resizedFrame.cols);
    for (int r = 0; r < resizedFrame.rows; ++r) {
        for (int c = 0; c < resizedFrame.cols; ++c) {
            dlibImage(r, c) = dlib::rgb_pixel(resizedFrame.at<cv::Vec3b>(r, c)[0], 
                                               resizedFrame.at<cv::Vec3b>(r, c)[1], 
                                               resizedFrame.at<cv::Vec3b>(r, c)[2]);
        }
    }

    return faceDetector(dlibImage);
}



void FaceRecognitionWorker::startRecognition() {
    running = true;
    cv::VideoCapture videoCapture(0);
    if (!videoCapture.isOpened()) {
        emit errorOccurred("Failed to open camera.");
        return;
    }
    std::vector<std::vector<matrix<float,0,1>>> knownFaceEncodings;
    std::cout << dlib::cuda::get_num_devices() << " CUDA devices detected." << std::endl;

    loadModels();

    std::vector<std::string> knownFaceNames = { "junsup", "jihwan", "mingi" };
    
    // Load known face encodings
    std::vector<dlib::matrix<dlib::rgb_pixel>> junsupImage = getFaceEncoding("../asset/junsup.png", faceDetector, shapePredictor, faceRecognitionModel);
    std::vector<dlib::matrix<dlib::rgb_pixel>> jihwanImage = getFaceEncoding("../asset/jihwan.png", faceDetector, shapePredictor, faceRecognitionModel);
    std::vector<dlib::matrix<dlib::rgb_pixel>> mingiImage = getFaceEncoding("../asset/mingi.png", faceDetector, shapePredictor, faceRecognitionModel);
    knownFaceEncodings.push_back(faceRecognitionModel(junsupImage));
    knownFaceEncodings.push_back(faceRecognitionModel(jihwanImage));
    knownFaceEncodings.push_back(faceRecognitionModel(mingiImage));
    // std::vector<matrix<float,0,1>> face_descriptors = faceRecognitionModel(knownFaceEncodings[0]);
    // knownFaceEncodings.push_back(getFaceEncoding(":/asset/jihwan.png", faceDetector, shapePredictor, faceRecognitionModel));
    // knownFaceEncodings.push_back(getFaceEncoding(":/asset/mingi.png", faceDetector, shapePredictor, faceRecognitionModel));

    // Print face descriptors for debugging
    // for (const auto& descriptor : face_descriptors) {
    //     std::cout << "Face descriptor: " << trans(descriptor) << std::endl;
    // }

    bool process_this_frame = true;

    while (running) {
        cv::Mat frame;
        if (!videoCapture.read(frame)) {
            emit errorOccurred("Failed to read frame.");
            break;
        }
        if (process_this_frame){
            std::vector<dlib::rectangle> faces = getOpenCVFaceEncoding(frame, faceDetector, shapePredictor, faceRecognitionModel);
            std::vector<> faceNames;
            std::vector<> faceLocations;
            for(auto face: faces){
                std::vector<> faceLocation;
                std::cout << "Face found at x: " << face.left() << " y: " << face.top() << std::endl;
                faceLocation.push_back(face.top());
                faceLocation.push_back(face.right());
                faceLocation.push_back(face.bottom());
                faceLocation.push_back(face.left());

                faceLocations.push_back(faceLocation);
                auto shape = shapePredictor(image, faces[0]);
                matrix<rgb_pixel> face_chip;
                extract_image_chip(image, get_face_chip_details(shape,150,0.25), face_chip);

            }

            cv::resize(frame, frame, cv::Size(320, 240));
            emit frameUpdated(frame);

        }
    }
    videoCapture.release();
}

void FaceRecognitionWorker::stopRecognition() {
    running = false;
}

void FaceRecognitionWorker::processFrame(cv::Mat &frame) {
    // cv::Mat resizedFrame;
    // cv::resize(frame, resizedFrame, cv::Size(), 0.25, 0.25);
    // cv::cvtColor(resizedFrame, resizedFrame, cv::COLOR_BGR2RGB);
    // dlib::matrix<dlib::rgb_pixel> dlibImage(resizedFrame.rows, resizedFrame.cols);
    // for (int r = 0; r < resizedFrame.rows; ++r) {
    //     for (int c = 0; c < resizedFrame.cols; ++c) {
    //         dlibImage(r, c) = dlib::rgb_pixel(resizedFrame.at<cv::Vec3b>(r, c)[0], 
    //                                            resizedFrame.at<cv::Vec3b>(r, c)[1], 
    //                                            resizedFrame.at<cv::Vec3b>(r, c)[2]);
    //     }
    // }

    // std::vector<dlib::rectangle> faces = faceDetector(dlibImage);
    // for (auto &face : faces) {
    //     dlib::full_object_detection shape = shapePredictor(dlibImage, face);
    //     faceDescriptor = faceRecognitionModel->compute_face_descriptor(dlibImage, shape)
    //     cv::rectangle(frame, cv::Rect(face.left(), face.top(), face.width(), face.height()), cv::Scalar(0, 255, 0), 2);
    // }
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


