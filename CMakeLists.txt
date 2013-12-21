project(O1Plot)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)
find_package(OpenGL REQUIRED)

set(QT_USE_QTMAIN true)
set(QT_USE_QTOPENGL true)
INCLUDE(${QT_USE_FILE})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})

option( LINUX_DISABLE_FILESYSTEMCACHE "When benchmarking on Linux empty the filesystem cache before each test run. You need to be root or have write access to /proc/sys/vm/drop_caches to use this feature." )
option( COUNT_TILES "Output the number of vertexes, lines and quads generated." )
set( BLOCKFACTOR "20" CACHE STRING "Performancetuning parameter, must be a multiple of 4: Blockfilesize will be 2^BLOCKFACTOR bytes." )
set( BRANCHFACTOR "4" CACHE STRING "Performancetuning parameter, must be a multiple of 4: Trees will branch with 2^BRANCHFACTOR." )
set( BENCHREPETITIONS "1000" CACHE STRING "Number of repetitions when doing benchmarks." )
set( LINUX_DROP_CACHES_VALUE "3" CACHE STRING "Value that should be written into /proc/sys/vm/drop_caches, 1 = only free page cache, 3 = free page cache, directory entries and inodes." )

set(O1PlotLib_SRCS ResizingTableView.cpp HideNotifyWidget.cpp ColorDialogDelegate.cpp Interval.cpp IntervalListLoader.cpp IntervalEditor.cpp 
    EventEditorLoader.cpp EventEditor.cpp YScaleBar.cpp SimpleViewWidget.cpp XScaleBar.cpp TimeSeriesPlot.cpp 
    GL_Layer.cpp TimeSeriesPlotLoader.cpp ColorDialogEditor.cpp SimpleViewModulesModel.cpp)
foreach(item ${O1PlotLib_SRCS})
  STRING(REPLACE "cpp" "h" value ${item})
  set(O1PlotLib_HDRS ${O1PlotLib_HDRS} ${value})
endforeach(item)

set(O1Plot_SRCS O1PlotMain.cpp mainwindow.cpp)

set(O1PrepLib_SRCS O1Prep.cpp O1PrepGUI.cpp Pow2SpinBox.cpp)
foreach(item ${O1PrepLib_SRCS})
  STRING(REPLACE "cpp" "h" value ${item})
  set(O1PrepLib_HDRS ${O1PrepLib_HDRS} ${value})
endforeach(item)
set(O1Prep_SRCS O1PrepMain.cpp)

set(O1PlotLib_UI_SRCS FileOpenFailed.ui EventEditorGUI.ui IntervalEditorGUI.ui)
set(O1Plot_UI_SRCS O1PlotMainWindow.ui MenuBox.ui)

set(O1Prep_UI_SRCS O1PrepGUI.ui)


qt4_wrap_ui(O1PlotLib_UI_HDRS ${O1PlotLib_UI_SRCS})
qt4_wrap_ui(O1Plot_UI_HDRS ${O1Plot_UI_SRCS})
qt4_wrap_ui(O1Prep_UI_HDRS ${O1Prep_UI_SRCS})

qt4_wrap_cpp(O1PlotLib_MOC_SRCS ${O1PlotLib_HDRS})
qt4_wrap_cpp(O1Plot_MOC_SRCS mainwindow.h)
qt4_wrap_cpp(O1PrepLib_MOC_SRCS ${O1PrepLib_HDRS})
qt4_wrap_cpp(O1Prep_MOC_SRCS ${O1Prep_HDRS})

# Set some Win32 Specific Settings
IF(WIN32)
SET(GUI_TYPE WIN32)
ENDIF(WIN32)
# Set some Apple MacOS Specific settings
IF (APPLE)
SET(GUI_TYPE MACOSX_BUNDLE)
ENDIF (APPLE)

add_executable(O1Plot ${GUI_TYPE} ${O1Plot_SRCS} ${O1Plot_MOC_SRCS} ${O1Plot_UI_HDRS})
add_executable(O1Prep ${GUI_TYPE} ${O1Prep_SRCS} ${O1Prep_MOC_SRCS})
add_library(O1PlotLib ${O1PlotLib_SRCS} ${O1PlotLib_MOC_SRCS} ${O1PlotLib_UI_HDRS} ${O1PrepLib_SRCS} ${O1PrepLib_MOC_SRCS} ${O1Prep_UI_HDRS})

set_target_properties(O1PlotLib PROPERTIES OUTPUT_NAME O1Plot)
set( DEFINES ${DEFINES} BLOCKFACTOR=${BLOCKFACTOR} )
set( DEFINES ${DEFINES} BRANCHFACTOR=${BRANCHFACTOR} )
if( COUNT_TILES )
  set( DEFINES ${DEFINES} "COUNT_TILES" )
endif ( COUNT_TILES )
set_property(TARGET O1PlotLib PROPERTY COMPILE_DEFINITIONS ${DEFINES})
set_property(TARGET O1Plot PROPERTY COMPILE_DEFINITIONS ${DEFINES})

target_link_libraries(O1Plot O1PlotLib ${QT_LIBRARIES} ${OPENGL_LIBRARIES})
target_link_libraries(O1Prep O1PlotLib ${QT_LIBRARIES})
target_link_libraries(O1PlotLib ${QT_LIBRARIES} ${OPENGL_LIBRARIES})

add_subdirectory(tests)
if( UNIX AND NOT APPLE) # MacOS X does not support clock_gettime()
add_subdirectory(benchmarks)
endif( UNIX AND NOT APPLE)
