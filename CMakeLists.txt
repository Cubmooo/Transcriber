cmake_minimum_required(VERSION 3.20)
project(Transcriber)

find_package(portaudio CONFIG REQUIRED)
find_package(FFTW3 CONFIG REQUIRED)

add_executable(Transcriber main.cpp)

target_link_libraries(Transcriber PRIVATE
    portaudio
    FFTW3::fftw3
)