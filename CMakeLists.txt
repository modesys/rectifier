cmake_minimum_required (VERSION 3.1)
project(Rectifier)

set (OpenCV_DIR /home/emanuele/opencv/build)

find_package( OpenCV REQUIRED )
find_package( Boost COMPONENTS system thread filesystem REQUIRED)
#find_package( Qt5 REQUIRED COMPONENTS Sql )

###
### make sure we use c++11
###
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

###
###boost include stuff (required for all libcam)
###
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

###
### QT stuff (required if you want a GUI)
###
set(CMAKE_AUTOMOC ON)  #This will autogenerate qt gui stuff
set(CMAKE_AUTORCC ON)  #this needs to be on so cmake will know what to do with QT resource Files

find_package(Qt5Widgets)
find_package(Qt5PrintSupport)
#find all the qt UI stuff
file(GLOB UI
    "ui/*.ui"
)
#make them into headers
qt5_wrap_ui (UI_HDRS  ${UI})

###
### add all your non QT sources
###

# find all non ui sources
file(GLOB SRCS
    "src/*.h"
    "src/*.cpp"
    "src/*.hpp"
)

# find all ui related sources
file(GLOB UI_SRCS
    "ui/*.h"
    "ui/*.cpp"
    "ui/*.hpp"
)

###
### Add executables
###

add_executable(Rectifier main/main.cpp ui/res/res.qrc ${SRCS} ${UI_HDRS} ${UI_SRCS})
target_link_libraries (Rectifier libCam Qt5::Widgets  ${Boost_LIBRARIES} ${OpenCV_LIBS}  Qt5::PrintSupport)

###
### Add Library
###

add_library(Rectifier_lib SHARED ${SRCS} ${UI_HDRS})
target_include_directories (Rectifier_lib PUBLIC "src/" "ui/")
target_link_libraries (Rectifier_lib libCam Qt5::Widgets ${Boost_LIBRARIES} ${OpenCV_LIBS})
