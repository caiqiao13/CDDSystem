cmake_minimum_required(VERSION 3.12)
project(detector_test)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 11)
# 设置源文件编码为 UTF-8
if(MSVC)
    add_compile_options("/utf-8")
    add_compile_options("/permissive-")
    add_compile_options("/Zc:__cplusplus")
else()
    add_compile_options(-finput-charset=UTF-8)
    add_compile_options(-fexec-charset=UTF-8)
endif()

set(CMAKE_AUTOMOC ON)  
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

############### include and build ####################
include_directories("./include")
set(CMAKE_AUTOUIC_SEARCH_PATHS "ui")
file(GLOB_RECURSE SOURCEFILES "include/*.h" "src/*.cpp" "resources/*.qrc" "ui/*.ui")	#遍历获取source/路径下所有的*.cpp和*.c文件列表
add_executable(
    ${PROJECT_NAME}
    # WIN32  # 隐藏窗口
    ${SOURCEFILES}
)


##########  Config Qt6  ##########
find_package(Qt6 COMPONENTS Core Gui Widgets Sql REQUIRED)
target_link_libraries(
    ${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
)

 
# ##########  Config OpenCV  ##########
set(OpenCV_PATH D:/study/opencv/OpenCV_MingW-main/OpenCV_MingW-main/OpenCV4.11_mingw13.1.0)
include_directories(${OpenCV_PATH}/include)
link_directories(${OpenCV_PATH}/x64/mingw/lib)
file(GLOB OpenCV_LIBS ${OpenCV_PATH}/x64/mingw/lib/libopencv_*.a)
target_link_libraries(${PROJECT_NAME}
        ${OpenCV_LIBS} 
)

