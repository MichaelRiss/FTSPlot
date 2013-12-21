project(O1Plot)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)
find_package(OpenGL REQUIRED)

set(QT_USE_QTMAIN true)
set(QT_USE_QTOPENGL true)
INCLUDE(${QT_USE_FILE})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})

foreach(item ${O1PlotLib_SRCS})
  STRING(REGEX REPLACE "^(.+)$" "../\\1" value ${item})
  set(O1PlotBenchLib_SRCS ${O1PlotBenchLib_SRCS} ${value})
endforeach(item)
set(O1PlotBenchLib_SRCS ${O1PlotBenchLib_SRCS} benchmarkHelper.cpp)

foreach(item ${O1PlotBenchLib_SRCS})
  STRING(REPLACE "cpp" "h" value ${item})
  set(O1PlotBenchLib_HDRS ${O1PlotBenchLib_HDRS} ${value})
endforeach(item)

set(O1PlotBench_SRCS O1PlotBenchMainWindow.cpp O1PlotBench.cpp)
set(O1PlotBench_UI_SRCS O1PlotBenchMainWindow.ui)

qt4_wrap_ui(O1PlotBench_UI_HDRS ${O1PlotBench_UI_SRCS})
qt4_automoc(${O1PlotBench_SRCS})
qt4_wrap_cpp(O1PlotBenchLib_MOC_SRCS ${O1PlotBenchLib_HDRS} OPTIONS "-DBENCHMARK")

# Set some Win32 Specific Settings
IF(WIN32)
SET(GUI_TYPE WIN32)
ENDIF(WIN32)
# Set some Apple MacOS Specific settings
IF (APPLE)
SET(GUI_TYPE MACOSX_BUNDLE)
ENDIF (APPLE)

add_library(O1PlotBenchLib ${O1PlotBenchLib_SRCS} ${O1PlotBenchLib_MOC_SRCS} ${O1PlotBench_UI_HDRS})
add_executable(O1PlotBench ${GUI_TYPE} ${O1PlotBench_SRCS} ${O1PlotBench_UI_HDRS})
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
set_property(TARGET O1PlotBench PROPERTY COMPILE_DEFINITIONS ${DEFINES})
set_property(TARGET O1PlotBenchLib PROPERTY COMPILE_DEFINITIONS ${DEFINES})

target_link_libraries(O1PlotBench O1PlotBenchLib ${QT_LIBRARIES} ${OPENGL_LIBRARIES} -lrt)
