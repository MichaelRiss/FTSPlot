project(FTSPlottest)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)
find_package(OpenGL REQUIRED)

set(QT_USE_QTMAIN true)
set(QT_USE_QTOPENGL true)
set(QT_USE_QTTEST true)
INCLUDE(${QT_USE_FILE})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})



set(runTests_SRCS IntervalsTest.cpp EventsTest.cpp YRangeTest.cpp XRangeTest.cpp ReorderModulesTest.cpp EditModuleTest.cpp ModifyColorTest.cpp ShowHideTest.cpp runTests.cpp SimpleViewTest.cpp TimeSeriesPlotTest.cpp EventEditorTest.cpp IntervalEditorTest.cpp ListModulesTest.cpp)
set(SimpleViewTest_SRCS SimpleViewTest.cpp)
set(TimeSeriesPlotTest_SRCS TimeSeriesPlotTest.cpp)
set(EventEditorTest_SRCS EventEditorTest.cpp)
set(IntervalEditorTest_SRCS IntervalEditorTest.cpp)
set(ListModulesTest_SRCS ListModulesTest.cpp)

qt4_automoc(${runTests_SRCS})
qt4_automoc(${SimpleViewTest_SRCS})
qt4_automoc(${TimeSeriesPlotTest_SRCS})
qt4_automoc(${EventEditorTest_SRCS})
qt4_automoc(${IntervalEditorTest_SRCS})
qt4_automoc(${ListModulesTest_SRCS})


# Set some Win32 Specific Settings
IF(WIN32)
SET(GUI_TYPE WIN32)
ENDIF(WIN32)
# Set some Apple MacOS Specific settings
IF (APPLE)
SET(GUI_TYPE MACOSX_BUNDLE)
ENDIF (APPLE)
IF (UNIX)
find_package(X11 REQUIRED)
ENDIF (UNIX)

ADD_EXECUTABLE(runTests ${runTests_SRCS})
set( DEFINES ${DEFINES} BLOCKFACTOR=${BLOCKFACTOR} )
set( DEFINES ${DEFINES} BRANCHFACTOR=${BRANCHFACTOR} )
set_property(TARGET runTests PROPERTY COMPILE_DEFINITIONS ${DEFINES})

TARGET_LINK_LIBRARIES(runTests FTSPlotLib ${QT_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_LIBRARIES})
