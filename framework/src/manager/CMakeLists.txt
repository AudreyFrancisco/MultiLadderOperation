message ("-----------------------------------------")
message ("        Building framework library")
message ("-----------------------------------------")

project (MANAGER)

cmake_minimum_required (VERSION 3.4.3 FATAL_ERROR)

#### source files

file (GLOB MANAGER_SOURCES *.cpp)
file (GLOB MANAGER_HEADERS *.h)

#### the library

add_library (MANAGER STATIC ${MANAGER_SOURCES} ${MANAGER_HEADERS} )
target_link_libraries (MANAGER LINK_PUBLIC COMMON MOSAIC ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(MANAGER COMMON MOSAIC)


install (TARGETS MANAGER LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/lib
                         ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/lib)
