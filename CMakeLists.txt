#C:\Qt\6.11.1\msvc2022_64\bin\windeployqt.exe "C:\Users\Joshua Davies\OneDrive - Saint Kentigern\Comp Sci\Transcriber\build\Debug\MyProject.exe"
cmake_minimum_required(VERSION 3.16)

project(MyProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH "C:/Qt/6.11.1/msvc2022_64")

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(MyProject
    transcriber.cpp
    globals.cpp
    audioinput.cpp
    fft.cpp
    beatfinder.cpp
    bpm.cpp
    mainwindow.cpp
    freqsender.cpp

)

target_include_directories(MyProject PRIVATE
    ${CMAKE_SOURCE_DIR}/libs
)

find_library(FFTW3_LIBRARY
    NAMES libfftw3-3
    PATHS ${CMAKE_SOURCE_DIR}/libs
)

find_library(PORTAUDIO_LIBRARY
    NAMES portaudio
    PATHS ${CMAKE_SOURCE_DIR}/libs
)

set_target_properties(MyProject PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${CMAKE_SOURCE_DIR}/libs"
)

target_precompile_headers(MyProject PRIVATE pch/pch.h)

target_link_libraries(MyProject PRIVATE
    Qt6::Widgets
    ${FFTW3_LIBRARY}
    ${PORTAUDIO_LIBRARY}
)