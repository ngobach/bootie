cmake_minimum_required(VERSION 3.31.6)
set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)

if(NOT APPLE)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-static -s")
endif()

include(FetchContent)

FetchContent_Declare(
  battery-embed
  GIT_REPOSITORY https://github.com/batterycenter/embed.git
  GIT_TAG        v1.2.19
)
FetchContent_MakeAvailable(battery-embed)

FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
)
FetchContent_MakeAvailable(pugixml)

project(
  bootie
  VERSION 0.1.0
  LANGUAGES CXX
)

set(
  SRCS src/bootie.cpp src/nativeio.cpp
)

if(NOT APPLE)
set(CMAKE_CXX_LINKER_DEPFILE_SUPPORTED FALSE)
endif()

set(CMAKE_CXX_STANDARD 20)

add_executable(bootie ${SRCS})
target_link_libraries(bootie PRIVATE argparse pugixml)
b_embed(bootie resources/gptdisk-64-sectors.raw)
