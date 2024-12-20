#include "face_recognition.h"
#include <QApplication>
#include <QMainWindow>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    FaceRecognitionWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
