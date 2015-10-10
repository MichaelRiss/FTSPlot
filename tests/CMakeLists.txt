project(FTSPlottest)
cmake_minimum_required( VERSION 3.0.2 )
find_package(Qt5 COMPONENTS Core Gui Widgets Test OpenGL)
find_package(OpenGL REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

set(runTests_SRCS IntervalsTest.cpp EventsTest.cpp YRangeTest.cpp XRangeTest.cpp ReorderModulesTest.cpp EditModuleTest.cpp ModifyColorTest.cpp ShowHideTest.cpp runTests.cpp FTSPlotWidgetTest.cpp TimeSeriesPlotTest.cpp EventEditorTest.cpp IntervalEditorTest.cpp ListModulesTest.cpp)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Set some Win32 Specific Settings
IF(WIN32)
SET(GUI_TYPE WIN32)
ENDIF(WIN32)
# Set some Apple MacOS Specific settings
IF (APPLE)
SET(GUI_TYPE MACOSX_BUNDLE)
ENDIF (APPLE)



ADD_EXECUTABLE(runTests ${runTests_SRCS})
qt5_use_modules(runTests Widgets OpenGL Test)
set( DEFINES ${DEFINES} BLOCKFACTOR=${BLOCKFACTOR} )
set( DEFINES ${DEFINES} BRANCHFACTOR=${BRANCHFACTOR} )
set_property(TARGET runTests PROPERTY COMPILE_DEFINITIONS ${DEFINES})

TARGET_LINK_LIBRARIES(runTests FTSPlotLib)
 