/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_HPP
#define XCLOX_HPP

#include "xclox/version.hpp"
#include "xclox/datetime.hpp"
#include "xclox/ntp/client.hpp"

/** @mainpage Documentation
 *
 * @section Introduction
 *
 * **Xclox** is a header-only cross-platform date and time library for C++.
 * It offers a high-level approach to creating, manipulating, and formatting times, dates, and datetimes.
 * It provides a flexible and advanced NTP client for querying internet time in an asynchronous fashion based on the ASIO library.
 * It has been thoroughly tested, as its development has been test-driven (TDD).
 *
 * @section Demonstration
 *
 * The following example shows only the basic functionalities of the library.
 * For further details, please see the full pages of the particular classes and their unit tests.
 *
 * xclox::Time, xclox::Date, xclox::DateTime, xclox::ntp::Client.
 *
 * @subsection Example
 * @include demo.cpp
 *
 * @subsection Output
 * @include demo.log
 *
 * @section Installation
 *
 * The library is header-only, so to link it to your project, just add the path of its include directory.
 * Please note that the NTP client depends on the Boost.ASIO networking library.
 * So, to use it, you need to build the unit tests (and optionally run them), as this installs ASIO automatically.
 * @code
 *    cd xclox_src_dir
 *    cmake -B build
 *    cmake --build build
 *    ctest --test-dir build\test
 * @endcode
 */

#endif // XCLOX_HPP
