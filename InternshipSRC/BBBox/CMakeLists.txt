cmake_minimum_required(VERSION 2.6)

SET(TARGET BBoxGTGUI)

PROJECT(${TARGET})


## allow more human readable "if then else" constructs
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)


##################################
## Set build_type if not set
IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF()


####################################################
## Find some packages 

FIND_PACKAGE(VideoFeatureBridge REQUIRED)

FIND_PACKAGE(Qt4  4.2.0 COMPONENTS QtCore QtGui REQUIRED)
IF(NOT QT4_FOUND)
  MESSAGE(SEND_ERROR "Failed to find Qt 4.3 or greater.")
ENDIF(NOT QT4_FOUND)


#REM: clang detection
# clang is only supported by CMake version >= 2.8.2
# For CMake version < 2.8.2, when clang compiler is used : 
# CMAKE_COMPILER_IS_GNUCC is true
# CMAKE_CXX_COMPILER_ID is GNU

# CHECK_SYMBOL_EXISTS is only available in CMake >= 2.8
# thus we can not do CHECK_SYMBOL_EXISTS(__clang__ "" CLANG_COMPILER)

IF(CMAKE_COMPILER_IS_GNUCC)
  # With GCC, always set flags to have most warnings
  ADD_DEFINITIONS(-Wall -Wextra)

  string(REGEX MATCH ".*clang.*" USE_CLANG ${CMAKE_CXX_COMPILER})

  IF(NOT USE_CLANG)

    #-Wunused-but-set-variable is available with gcc 4.5.1 on Fedora (14)
    # but not available with gcc 4.5.2 on Ubuntu (11.04)
    # Thus we can not test with IF(${GCC_VERSION} VERSION_GREATER 4.5.0)
    #EXEC_PROGRAM( ${CMAKE_CXX_COMPILER} ARGS "-dumpversion" OUTPUT_VARIABLE GCC_VERSION )
    #IF(${GCC_VERSION} VERSION_GREATER 4.5.0)
    #  #-Wunused-but-set-variable seems only in gcc version 4.5.1 and above.
    #  ADD_DEFINITIONS(-Wunused-but-set-variable)      
    #ENDIF()
    INCLUDE(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG(-Wunused-but-set-variable HAS_WARNING_UNUSED_BUT_SET_VARIABLE)
    if (HAS_WARNING_UNUSED_BUT_SET_VARIABLE)
      ADD_DEFINITIONS(-Wunused-but-set-variable)
      #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-but-set-variable")
    ENDIF()

  ENDIF()

ENDIF(CMAKE_COMPILER_IS_GNUCC)


ADD_DEFINITIONS(-D__STDC_CONSTANT_MACROS) ## For FFMPEG & C++ !!!!

SET(HDRS ImageArea.hpp MainWindow.hpp AABBox.hpp )

SET(SRCS ImageArea.cpp main.cpp MainWindow.cpp )

SET(MOC_HDRS ImageArea.hpp MainWindow.hpp )

INCLUDE(${QT_USE_FILE}) #we do it here, to have QT_LIBRARIES defined

INCLUDE_DIRECTORIES(${VIDEOFEATUREBRIDGE_INCLUDE_DIRS})

QT4_WRAP_CPP(MOC_SOURCES ${MOC_HDRS})


ADD_EXECUTABLE(${TARGET} ${SRCS} ${HDRS} ${MOC_SOURCES})

TARGET_LINK_LIBRARIES(${TARGET} ${VIDEOFEATUREBRIDGE_LIBRARIES} ${QT_LIBRARIES})
