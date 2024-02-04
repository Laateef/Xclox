#
# Copyright (c) 2024 Abdullatif Kalla.
#
# This source code is licensed under the MIT license found in the
# LICENSE.txt file in the root directory of this source tree.
#

# Fetch doctest
include(FetchContent)
FetchContent_Declare(doctest DOWNLOAD_NO_EXTRACT true URL "https://github.com/doctest/doctest/releases/download/v2.4.11/doctest.h")
FetchContent_MakeAvailable(doctest)

add_executable(tests main.cpp)
target_include_directories(tests SYSTEM PRIVATE ${doctest_SOURCE_DIR} ${asio_SOURCE_DIR}/include PRIVATE ../include)
target_compile_options(tests PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wall -Wextra -pedantic -Werror>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

enable_testing()
add_test(tests tests)
