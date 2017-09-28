message ("-----------------------------------------")
message ("        Building COMMON library")
message ("-----------------------------------------")

project (COMMON)

cmake_minimum_required (VERSION 3.4.3 FATAL_ERROR)

#### source files

file (GLOB COMMON_SOURCES *.cpp)
file (GLOB COMMON_HEADERS *.h)

#### the library

add_library (COMMON STATIC ${COMMON_SOURCES} ${COMMON_HEADERS} )
target_link_libraries (COMMON LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
install (TARGETS COMMON LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/lib
                        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/lib)
