message ("-----------------------------------------")
message ("        Building MOSAIC library")
message ("-----------------------------------------")

project (MOSAIC)

cmake_minimum_required (VERSION 3.4.3 FATAL_ERROR)

#### source files

file (GLOB MOSAIC_SOURCES *.cpp)
file (GLOB MOSAIC_HEADERS *.h)

#### the library

add_library (MOSAIC STATIC ${MOSAIC_SOURCES} ${MOSAIC_HEADERS})
target_link_libraries (MOSAIC LINK_PUBLIC COMMON ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(MOSAIC COMMON)

install (TARGETS MOSAIC LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/lib
                        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/lib)
