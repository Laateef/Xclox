#
# Copyright (c) 2024 Abdullatif Kalla.
#
# This source code is licensed under the MIT license found in the
# LICENSE.txt file in the root directory of this source tree.
#

cmake_minimum_required(VERSION 3.13)

project(Xclox VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate version.hpp
configure_file(
  ${PROJECT_SOURCE_DIR}/version.in
  ${PROJECT_SOURCE_DIR}/include/xclox/version.hpp
)

# Fetch ASIO
include(FetchContent)
FetchContent_Declare(asio DOWNLOAD_EXTRACT_TIMESTAMP true URL "https://sourceforge.net/projects/asio/files/asio/1.28.1%20%28Stable%29/asio-1.28.1.zip/download")
FetchContent_MakeAvailable(asio)

file(GLOB_RECURSE header_files LIST_DIRECTORIES false "${PROJECT_SOURCE_DIR}/include/*.hpp")

add_library(xclox INTERFACE)
target_sources(xclox INTERFACE ${header_files})
target_include_directories(xclox SYSTEM INTERFACE ${asio_SOURCE_DIR}/include include)

option(XCLOX_BUILD_TESTS "build tests" ON)
if (XCLOX_BUILD_TESTS)
    add_subdirectory(test)
endif()

add_subdirectory(demo)
