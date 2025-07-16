cmake_minimum_required(VERSION 3.30)
project(minecraft)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic -Wshadow
        -fsanitize=address,undefined -g -fno-omit-frame-pointer
)

target_link_options(${PROJECT_NAME} PRIVATE
        -fsanitize=address,undefined
)

# === raylib setup ===
FetchContent_Declare(
        raylib
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/third_party/raylib"
)
FetchContent_MakeAvailable(raylib)

target_link_libraries(${PROJECT_NAME}
        PRIVATE raylib
        PRIVATE m pthread dl
)