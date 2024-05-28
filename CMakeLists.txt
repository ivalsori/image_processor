cmake_minimum_required(VERSION 3.27)
project(image_processor)

set(CMAKE_CXX_STANDARD 20)

add_executable(
    image_processor
    image_processor.cpp
        Image.cpp Image.h Filter.cpp Filter.h ImageRedactor.cpp ImageRedactor.h BMPio.cpp BMPio.h ImageException.cpp ImageException.h)
