cmake_minimum_required(VERSION 3.10)
project(CmdGame CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(CmdGame main.cpp)

if(MSVC)
    # ソースと文字列をUTF-8として扱う
    target_compile_options(CmdGame PRIVATE /utf-8)
endif()
