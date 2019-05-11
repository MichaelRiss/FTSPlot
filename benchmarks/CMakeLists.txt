project(FTSPlot)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)
find_package(OpenGL REQUIRED)

set(QT_USE_QTMAIN true)
set(QT_USE_QTOPENGL true)
INCLUDE(${QT_USE_FILE})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})

foreach(item ${FTSPlotLib_SRCS})
  STRING(REGEX REPLACE "^(.+)$" "../\\1" value ${item})
  set(FTSPlotBenchLib_SRCS ${FTSPlotBenchLib_SRCS} ${value})
endforeach(item)
set(FTSPlotBenchLib_SRCS ${FTSPlotBenchLib_SRCS} benchmarkHelper.cpp)

foreach(item ${FTSPlotBenchLib_SRCS})
  STRING(REPLACE "cpp" "h" value ${item})
  set(FTSPlotBenchLib_HDRS ${FTSPlotBenchLib_HDRS} ${value})
endforeach(item)

set(FTSPlotBench_SRCS FTSPlotBenchMainWindow.cpp FTSPlotBench.cpp)
set(FTSPlotBench_UI_SRCS FTSPlotBenchMainWindow.ui)

qt4_wrap_ui(FTSPlotBench_UI_HDRS ${FTSPlotBench_UI_SRCS})
qt4_automoc(${FTSPlotBench_SRCS})
qt4_wrap_cpp(FTSPlotBenchLib_MOC_SRCS ${FTSPlotBenchLib_HDRS} OPTIONS "-DBENCHMARK")

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

add_library(FTSPlotBenchLib ${FTSPlotBenchLib_SRCS} ${FTSPlotBenchLib_MOC_SRCS} ${FTSPlotBench_UI_HDRS})
add_executable(FTSPlotBench ${GUI_TYPE} ${FTSPlotBench_SRCS} ${FTSPlotBench_UI_HDRS})
set(DEFINES "BENCHMARK")
set( DEFINES ${DEFINES} BENCHREPETITIONS=${BENCHREPETITIONS} )
set( DEFINES ${DEFINES} BLOCKFACTOR=${BLOCKFACTOR} )
set( DEFINES ${DEFINES} BRANCHFACTOR=${BRANCHFACTOR} )
set( DEFINES ${DEFINES} LINUX_DROP_CACHES_VALUE="${LINUX_DROP_CACHES_VALUE}" )
if( LINUX_DISABLE_FILESYSTEMCACHE )
  set( DEFINES ${DEFINES} "LINUX_DISABLE_FILESYSTEMCACHE" )
endif( LINUX_DISABLE_FILESYSTEMCACHE )
if( COUNT_TILES )
  set( DEFINES ${DEFINES} "COUNT_TILES" )
endif ( COUNT_TILES )
set_property(TARGET FTSPlotBench PROPERTY COMPILE_DEFINITIONS ${DEFINES})
set_property(TARGET FTSPlotBenchLib PROPERTY COMPILE_DEFINITIONS ${DEFINES})

target_link_libraries(FTSPlotBench FTSPlotBenchLib ${QT_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_LIBRARIES} -lrt)
