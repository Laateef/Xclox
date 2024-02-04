/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#include "xclox/version.hpp"

TEST_CASE("version")
{
    int major = XCLOX_VERSION_MAJOR;
    int minor = XCLOX_VERSION_MINOR;
    int patch = XCLOX_VERSION_PATCH;

    CHECK(XCLOX_VERSION_CODE == major * 10000 + minor * 100 + patch);
    CHECK(std::strcmp(XCLOX_VERSION_STRING, (std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch)).c_str()) == 0);
}
