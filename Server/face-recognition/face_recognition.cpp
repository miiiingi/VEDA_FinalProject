#include "face_recognition.h"
#include <QDateTime>
#include <QPixmap>
#include <QFile>
#include <QMessageBox>
#include <QCloseEvent>

FaceRecognitionWindow::FaceRecognitionWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FaceRecognitionWindow)
{
    ui->setupUi(this);
    setupIcons();

    // 초기 설정
    ui->stop_button->setEnabled(false);
    ui->video_label->setText("Camera Off");
    ui->video_label_2->setText("Logging View");
}

FaceRecognitionWindow::~FaceRecognitionWindow()
{
    delete ui;
}

void FaceRecognitionWindow::setupIcons()
{
    // 아이콘 설정
    QString idleDoorImagePath = ":/asset/icon_door_idle.png";
    QString idleBellImagePath = ":/asset/icon_bell_idle.png";
    QString idleSirenImagePath = ":/asset/icon_siren_idle.png";

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
    ui->start_button->setEnabled(false);
    ui->stop_button->setEnabled(true);
    updateLog("시스템 시작");
}

void FaceRecognitionWindow::on_stop_button_clicked()
{
    ui->start_button->setEnabled(true);
    ui->stop_button->setEnabled(false);
    updateLog("시스템 정지");
}

void FaceRecognitionWindow::on_bell_button_clicked()
{
    updateLog("벨 알림 발생");
}

void FaceRecognitionWindow::on_door_button_clicked()
{
    updateLog("문 열림 감지");
}

void FaceRecognitionWindow::on_siren_button_clicked()
{
    updateLog("사이렌 작동");
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
        event->accept();
    } else {
        event->ignore();
    }
}


