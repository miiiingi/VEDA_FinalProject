#ifndef FACE_RECOGNITION_H
#define FACE_RECOGNITION_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QCloseEvent>
#include <QKeyEvent>
#include "ui_face_recognition.h" // Qt Designer가 생성한 UI 헤더 포함

namespace Ui {
class FaceRecognitionWindow;
}

class FaceRecognitionWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FaceRecognitionWindow(QWidget *parent = nullptr);
    ~FaceRecognitionWindow();

private slots:
    void on_start_button_clicked();
    void on_stop_button_clicked();
    void on_bell_button_clicked();
    void on_door_button_clicked();
    void on_siren_button_clicked();

private:
    Ui::FaceRecognitionWindow *ui;

    void setupIcons();
    void updateLog(const QString &message);
    void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *event);
};

#endif // FACE_RECOGNITION_H
