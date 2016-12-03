cmake_minimum_required(VERSION 3.7)
project(QAmiensRCode2)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES BitBuffer.cpp QAmiensRCode.cpp QAmiensRCode_main.cpp QrSegment.cpp)
set(HEADER_FILES BitBuffer.hpp QAmiensRCode.hpp QrSegment.hpp)
set(OTHER_FILES QAmiensRCode.xml)
add_executable(QAmiensRCode2 ${SOURCE_FILES} ${HEADER_FILES} ${OTHER_FILES})