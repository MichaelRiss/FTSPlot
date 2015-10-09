project(FTSPlot)
cmake_minimum_required( VERSION 3.0.2 )
find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL)
find_package(OpenGL REQUIRED)


set(CMAKE_INCLUDE_CURRENT_DIR ON)


option( LINUX_DISABLE_FILESYSTEMCACHE "When benchmarking on Linux empty the filesystem cache before each test run. You need to be root or have write access to /proc/sys/vm/drop_caches to use this feature." )
option( COUNT_TILES "Output the number of vertexes, lines and quads generated." )
set( BLOCKFACTOR "20" CACHE STRING "Performancetuning parameter, must be a multiple of 4: Blockfilesize will be 2^BLOCKFACTOR bytes." )
set( BRANCHFACTOR "4" CACHE STRING "Performancetuning parameter, must be a multiple of 4: Trees will branch with 2^BRANCHFACTOR." )
set( BENCHREPETITIONS "1000" CACHE STRING "Number of repetitions when doing benchmarks." )
set( LINUX_DROP_CACHES_VALUE "3" CACHE STRING "Value that should be written into /proc/sys/vm/drop_caches, 1 = only free page cache, 3 = free page cache, directory entries and inodes." )

set(FTSPlotLib_SRCS ResizingTableView.cpp HideNotifyWidget.cpp ColorDialogDelegate.cpp Interval.cpp IntervalListLoader.cpp IntervalEditor.cpp 
    EventEditorLoader.cpp EventEditor.cpp YScaleBar.cpp FTSPlotWidget.cpp XScaleBar.cpp TimeSeriesPlot.cpp 
    GL_Layer.cpp TimeSeriesPlotLoader.cpp ColorDialogEditor.cpp FTSPlotModulesModel.cpp)


set(FTSPlot_SRCS FTSPlotMain.cpp mainwindow.cpp)

set(FTSPrepLib_SRCS FTSPrep.cpp FTSPrepGUI.cpp Pow2SpinBox.cpp)

set(FTSPrep_SRCS FTSPrepMain.cpp)

set(FTSPlotLib_UI_SRCS FileOpenFailed.ui EventEditorGUI.ui IntervalEditorGUI.ui)
set(FTSPlot_UI_SRCS FTSPlotMainWindow.ui MenuBox.ui)
set(FTSPrep_UI_SRCS FTSPrepGUI.ui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
qt5_wrap_ui(FTSPlotLib_UI_HDRS ${FTSPlotLib_UI_SRCS})
qt5_wrap_ui(FTSPlot_UI_HDRS ${FTSPlot_UI_SRCS})
qt5_wrap_ui(FTSPrep_UI_HDRS ${FTSPrep_UI_SRCS})


# Set some Win32 Specific Settings
IF(WIN32)
SET(GUI_TYPE WIN32)
ENDIF(WIN32)
# Set some Apple MacOS Specific settings
IF (APPLE)
SET(GUI_TYPE MACOSX_BUNDLE)
ENDIF (APPLE)

add_executable(FTSPlot ${GUI_TYPE} ${FTSPlot_SRCS} ${FTSPlot_MOC_SRCS} ${FTSPlot_UI_HDRS})
add_executable(FTSPrep ${GUI_TYPE} ${FTSPrep_SRCS} ${FTSPrep_MOC_SRCS})
add_library(FTSPlotLib ${FTSPlotLib_SRCS} ${FTSPlotLib_MOC_SRCS} ${FTSPlotLib_UI_HDRS} ${FTSPrepLib_SRCS} ${FTSPrepLib_MOC_SRCS} ${FTSPrep_UI_HDRS})
qt5_use_modules(FTSPlotLib Widgets OpenGL)

set_target_properties(FTSPlotLib PROPERTIES OUTPUT_NAME FTSPlot)
set( DEFINES ${DEFINES} BLOCKFACTOR=${BLOCKFACTOR} )
set( DEFINES ${DEFINES} BRANCHFACTOR=${BRANCHFACTOR} )
if( COUNT_TILES )
  set( DEFINES ${DEFINES} "COUNT_TILES" )
endif ( COUNT_TILES )
set_property(TARGET FTSPlotLib PROPERTY COMPILE_DEFINITIONS ${DEFINES})
set_property(TARGET FTSPlot PROPERTY COMPILE_DEFINITIONS ${DEFINES})

target_link_libraries(FTSPlot FTSPlotLib ${OPENGL_LIBRARIES})
target_link_libraries(FTSPrep FTSPlotLib)
target_link_libraries(FTSPlotLib ${OPENGL_LIBRARIES})

add_subdirectory(tests)
if( UNIX AND NOT APPLE) # MacOS X does not support clock_gettime()
add_subdirectory(benchmarks)
endif( UNIX AND NOT APPLE)
