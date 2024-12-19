cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(FaceRecognition VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Gui Widgets Network Multimedia MultimediaWidgets REQUIRED)

find_package(OpenCV 4.5.4 REQUIRED COMPONENTS
	core
	highgui
	imgproc
	dnn
)

find_package(dlib REQUIRED)

find_package(PNG REQUIRED)

set(CMAKE_BUILD_TYPE Release)  # 또는 Debug

set(SOURCES
    main.cpp
    face_recognition.cpp
)

set(HEADERS
    face_recognition.h
)

set(UI_FILES
    face_recognition.ui
)

set(CUDA_ARCHITECTURES "87") # 명시적으로 설정

# UI 파일 처리 (qt5_wrap_ui)
qt5_wrap_ui(UI_HEADERS ${UI_FILES})

# Meta-Object Compiler(MOC) 헤더 처리
qt5_wrap_cpp(MOC_HEADERS ${HEADERS})

add_executable(FaceRecognition
    ${SOURCES} ${MOC_HEADERS} ${UI_HEADERS}
)

target_link_libraries(FaceRecognition PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Qt5::Multimedia
    Qt5::MultimediaWidgets
    ${OpenCV_LIBS}
    dlib::dlib
    ${PNG_LIBRARIES}
)

target_include_directories(FaceRecognition PRIVATE
    ${CMAKE_BINARY_DIR}
    ${OpenCV_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
)

target_compile_definitions(FaceRecognition PRIVATE DLIB_PNG_SUPPORT)