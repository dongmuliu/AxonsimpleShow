cmake_minimum_required(VERSION 2.8)




project(TestOPENNI2)
set(OPENNI_INCLUDE_DIR"E:/AXON_Release_20180814/Include")
set(OPENNI_LIB_DIR"E:/AXON_Release_20180814/Lib")
include_directories($ENV{OPENNI_INCLUDE_DIR})
link_directories($ENV{OPENNI_LIB_DIR})
#message($ENV{OPENNI2_INCLUDE})
#message($ENV{OPENNI2_REDIST})
find_package(OpenCV REQUIRED)

add_executable(${PROJECT_NAME} "Testmain.cpp")

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} OpenNI2 )