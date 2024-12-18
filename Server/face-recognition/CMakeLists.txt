cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(FaceRecognition VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Gui Widgets Network Multimedia MultimediaWidgets REQUIRED)

find_package(OpenCV 4.5.4 REQUIRED COMPONENTS
	core
	highgui
	imgproc
	dnn
)

set(SOURCES
    main.cpp
    face_recognition.cpp
    face_recognition.qrc
)

set(HEADERS
    face_recognition.h
)

set(UI_FILES
    face_recognition.ui
)
# .qrc 파일 추가
set(RESOURCE_FILES
    face_recognition.qrc
)

# 리소스를 빌드 시스템에 등록
qt5_add_resources(RESOURCES ${RESOURCE_FILES})

# UI 파일 처리 (qt5_wrap_ui)
qt5_wrap_ui(UI_HEADERS ${UI_FILES})

# Meta-Object Compiler(MOC) 헤더 처리
qt5_wrap_cpp(MOC_HEADERS ${HEADERS})

add_executable(FaceRecognition
    ${SOURCES} ${MOC_HEADERS} ${UI_HEADERS} ${RESOURCES}
)

target_link_libraries(FaceRecognition PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Qt5::Multimedia
    Qt5::MultimediaWidgets
)

target_include_directories(FaceRecognition PRIVATE ${CMAKE_BINARY_DIR})
