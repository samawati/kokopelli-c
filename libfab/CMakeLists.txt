cmake_minimum_required(VERSION 2.82)

project(libfab)

set(CMAKE_BUILD_TYPE DEBUG)
#set(CMAKE_BUILD_TYPE RELEASE)

set(CMAKE_C_FLAGS "-g -fPIC -pedantic -std=gnu99")
set(CMAKE_C_FLAGS_RELEASE "-O3 -D '_STATIC_=static'")
set(CMAKE_C_FLAGS_DEBUG "-O0 -D '_STATIC_= '")

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
find_library(M_LIB m)

include_directories(.)

file(GLOB LIBFAB_SRC_FILES *.c)

add_library(fab STATIC
	${LIBFAB_SRC_FILES}
)

target_link_libraries(fab ${PNG_LIBRARY} ${M_LIB})
install(TARGETS fab DESTINATION ${PROJECT_SOURCE_DIR})
