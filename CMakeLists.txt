cmake_minimum_required(VERSION 3.7)
project(QAmiensRCode2)

set(PROJECT_NAME QAmiensRCode)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES BitTampon.cpp QAmiensRCode.cpp QAmiensRCode_main.cpp QAmiensRSegment.cpp)
set(HEADER_FILES BitTampon.hpp QAmiensRCode.hpp QAmiensRSegment.hpp)
set(OTHER_FILES QAmiensRCode.xml)

#sfml
set(SFML_DIR "D:\\Documents\\C++\\Librairies\\SFML-2.4.1_DW232")
set(SFML_ROOT ${SFML_DIR})
set(CMAKE_MODULE_PATH "${SFML_DIR}/cmake/Modules" ${CMAKE_MODULE_PATH})
find_package(SFML 2 COMPONENTS system window graphics audio REQUIRED)

include_directories(${SFML_INCLUDE_DIR})
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${OTHER_FILES})
target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES})

