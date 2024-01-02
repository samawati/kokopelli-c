cmake_minimum_required(VERSION 2.82)

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project(frep)

#set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing -fopenmp -g -Wall -Wmissing-prototypes -Wstrict-prototypes -fPIC -g -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gdwarf-4")

add_subdirectory(libfab)

file(GLOB SRC_FILES src/*.c)

add_executable(frep
	main.cpp

    ${SRC_FILES}
    
    third-party/imgui/imgui.cpp
    third-party/imgui/imgui_draw.cpp
    third-party/imgui/imgui_tables.cpp
    third-party/imgui/imgui_impl_glfw.cpp
    third-party/imgui/imgui_impl_opengl3.cpp
    third-party/imgui/imgui_widgets.cpp
    third-party/imgui-color-text-edit/TextEditor.cpp)



target_include_directories(frep PRIVATE
	src
    libfab
    third-party/imgui
    third-party/imgui-color-text-edit)
	
target_link_libraries(frep fab lua5.1 -lpthread )
target_link_libraries(frep glfw GLEW GL)
