#ifndef FACE_RECOGNITION_H
#define FACE_RECOGNITION_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QThread>
#include <opencv2/opencv.hpp>
#include <QListWidget>
#include <QCloseEvent>
#include <QKeyEvent>

#include <dlib/image_processing/frontal_face_detector.h>  // 얼굴 검출기
#include <dlib/image_processing.h>
#include <dlib/image_io.h>
#include <dlib/dnn.h>

#include "ui_face_recognition.h" // Qt Designer가 생성한 UI 헤더 포함

namespace Ui {
class FaceRecognitionWindow;
}

using namespace dlib;
using namespace std;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET> 
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

using anet_type = loss_metric<fc_no_bias<128,avg_pool_everything<
                            alevel0<
                            alevel1<
                            alevel2<
                            alevel3<
                            alevel4<
                            max_pool<3,3,2,2,relu<affine<con<32,7,7,2,2,
                            input_rgb_image_sized<150>
                            >>>>>>>>>>>>;


class FaceRecognitionWorker : public QObject {
    Q_OBJECT
public:
    explicit FaceRecognitionWorker(QObject *parent = nullptr);
    void loadModels();
    void startRecognition();
    void stopRecognition();

    std::vector<dlib::rectangle> getOpenCVFaceEncoding(const cv::Mat& frame, frontal_face_detector& faceDetector, shape_predictor& shapePredictor, anet_type& faceRecognitionModel);

signals:
    void frameUpdated(const cv::Mat &frame);
    void errorOccurred(const QString &error);

private:
    bool running;
    bool process_this_frame = true;
    frontal_face_detector faceDetector;
    shape_predictor shapePredictor;
    anet_type faceRecognitionModel;
};

class FaceRecognitionWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FaceRecognitionWindow(QWidget *parent = nullptr);
    ~FaceRecognitionWindow();

private slots:
    void on_start_button_clicked();
    void on_stop_button_clicked();
    void displayFrame(const cv::Mat &frame);
    void showError(const QString &error);

private:
    Ui::FaceRecognitionWindow *ui;
    QThread workerThread;
    FaceRecognitionWorker *worker;

    void setupIcons();
    void updateLog(const QString &message);
    void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *event);
    void setupConnections(); // 추가된 부분
};

#endif // FACE_RECOGNITION_H
