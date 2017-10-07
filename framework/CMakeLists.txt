message ("-----------------------------------------")
message ("Building the hic test high-level software")
message ("-----------------------------------------")

if (UNIX)
    find_program(CMAKE_C_COMPILER NAMES $ENV{CC} gcc PATHS ENV PATH NO_DEFAULT_PATH)
    find_program(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)
endif (UNIX)

project (FRAMEWORK)

cmake_minimum_required (VERSION 3.4.3 FATAL_ERROR)
message ("   Cmake version : ${CMAKE_VERSION}")

#### setup output directories

set (INSTALL_PATH ${CMAKE_SOURCE_DIR})
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

#### compiler flags

if (APPLE)
    set (CMAKE_MACOSX_RPATH 1)
endif (APPLE)
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED YES)

if (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_CXX_FLAGS "-O2 -g -Wall -Wextra -Wshadow -fPIC -Woverloaded-virtual -Wno-unknown-pragmas -m64 -mcmodel=large")
    set (COMPILEFLAGS -O2 -fPIC -g -Wall -Wextra -Wshadow -Woverloaded-virtual -Wno-unknown-pragmas -m64 -mcmodel=large)
    set (LINKFLAGS -rdynamic -lusb-1.0 )
    else()
    set (CMAKE_CXX_FLAGS "-O2 -g -Wall -Wextra -Wshadow -fPIC -Woverloaded-virtual -Wno-unknown-pragmas -m64")
    set (COMPILEFLAGS -O2 -fPIC -g -Wall -Wextra -Wshadow -Woverloaded-virtual -Wno-unknown-pragmas -m64)
    set (LINKFLAGS -lusb-1.0 )
    set (LINKFLAGS)
endif()

message ("-- Flags:")
message ("    C++ compiler flags : ${CMAKE_CXX_FLAGS}")
message ("    Compile flags : ${COMPILEFLAGS}")
message ("    Link flags : ${LINKFLAGS}")

#### package(s) and include directories

list (APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package (ROOT REQUIRED COMPONENTS Gpad Graf Hist MathCore Minuit RIO Postscript)
include (${ROOT_USE_FILE})

find_package (Threads REQUIRED)

find_path (LIBUSB_INCLUDE_DIR
    NAMES libusb.h
    PATH_SUFFIXES "include" "libusb" "libusb-1.0")
find_library (LIBUSB_LIBRARY
    NAMES usb-1.0
    PATH_SUFFIXES "lib" "lib32" "lib64")

message ("-- libusb include dir : ${LIBUSB_INCLUDE_DIR}")
message ("-- libusb library : ${LIBUSB_LIBRARY}")

include_directories ("${PROJECT_SOURCE_DIR}/src/common")
include_directories ("${PROJECT_SOURCE_DIR}/src/mosaic")
include_directories ("${PROJECT_SOURCE_DIR}/src/manager")
include_directories ("${LIBUSB_INCLUDE_DIR}")
include_directories (/usr/local/include /usr/include)

get_property (inc_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
message ("-- Include directories = ${inc_dirs}")

link_directories (/usr/local/lib )

#### build and install MOSAIC library

add_subdirectory(src/common)
add_subdirectory(src/mosaic)
add_subdirectory(src/manager)

#### build and install the executables

foreach (EXECNAME
    mosaic
    fifo
    digitalscan
    roottest
#    threshold
#    scantest
#    dacscan
#    noiseocc_ext
#    noiseocc
#    poweron
#    pulselength
#    source
#    temperature
#    template
)
    add_executable (test_${EXECNAME} exe/main_${EXECNAME}.cpp)
    target_link_libraries (test_${EXECNAME} LINK_PUBLIC COMMON MOSAIC MANAGER ${LIBUSB_LIBRARY} ${ROOT_LIBRARIES})
    add_dependencies(test_${EXECNAME} COMMON MOSAIC MANAGER)
    install (TARGETS test_${EXECNAME} DESTINATION ${CMAKE_SOURCE_DIR}/bin)
endforeach (EXECNAME)

