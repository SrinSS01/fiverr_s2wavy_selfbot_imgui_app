cmake_minimum_required(VERSION 3.23)
project(SelfBot)

set(CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_STANDARD 23)
set(fonts_dir "${CMAKE_SOURCE_DIR}/fonts")
set(shared_lib_dir "${CMAKE_SOURCE_DIR}/lib")
set(output_dir "${CMAKE_BINARY_DIR}")
add_definitions(-DCURL_STATICLIB)
add_executable(
        ${PROJECT_NAME}
        src/main.cpp
        src/imgui.cpp
        src/imgui_demo.cpp
        src/imgui_draw.cpp
        src/imgui_impl_glfw.cpp
        src/imgui_impl_opengl3.cpp
        src/imgui_tables.cpp
        src/imgui_widgets.cpp
        src/imgui_stdlib.cpp
        src/LoginWindow.cpp
        src/LibCurl.cpp src/stb_image.cpp src/Api.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(
        ${PROJECT_NAME}
        ${CMAKE_SOURCE_DIR}/lib/libglfw3.a
        ${CMAKE_SOURCE_DIR}/lib/libjsoncpp.a
        ${CMAKE_SOURCE_DIR}/lib/libcurl-x64.dll
#        ${CMAKE_SOURCE_DIR}/lib/libcrypto.a
#        ${CMAKE_SOURCE_DIR}/lib/libpsl.a
#        ${CMAKE_SOURCE_DIR}/lib/libssh2.a
#        ${CMAKE_SOURCE_DIR}/lib/libssl.a
#        ${CMAKE_SOURCE_DIR}/lib/libz.a
#        ${CMAKE_SOURCE_DIR}/lib/libzstd.a
        opengl32
)
file(COPY ${fonts_dir} DESTINATION ${output_dir})
#file(COPY ${CMAKE_SOURCE_DIR}/lib/curl-ca-bundle.crt DESTINATION ${output_dir})
file(GLOB dlls_to_copy ${shared_lib_dir}/*.dll)
#file(COPY ${shared_lib_dir}/selfbot-backend.exe DESTINATION ${output_dir})
file(COPY ${dlls_to_copy} DESTINATION ${output_dir})
