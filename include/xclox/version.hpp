/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_VERSION_HPP
#define XCLOX_VERSION_HPP

#define XCLOX_VERSION_MAJOR 1
#define XCLOX_VERSION_MINOR 0
#define XCLOX_VERSION_PATCH 0

#define XCLOX_VERSION_CODE (XCLOX_VERSION_MAJOR * 10000 + XCLOX_VERSION_MINOR * 100 + XCLOX_VERSION_PATCH)

#define VALUE(v) #v
#define STRING(v) VALUE(v)

#define XCLOX_VERSION_STRING STRING(XCLOX_VERSION_MAJOR) "." STRING(XCLOX_VERSION_MINOR) "." STRING(XCLOX_VERSION_PATCH)

#endif // XCLOX_VERSION_HPP
