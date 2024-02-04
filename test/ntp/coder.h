/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#include "xclox/ntp/coder.hpp"

using namespace xclox::ntp;

TEST_SUITE("Coder")
{
    TEST_CASE("deserialize a byte")
    {
        uint8_t data[] = { 0 };
        CHECK(Coder::deserialize<uint8_t>(data) == 0);
        data[0] = 0xFF;
        CHECK(Coder::deserialize<uint8_t>(data) == 0xFF);
    }

    TEST_CASE("deserialize a word")
    {
        uint8_t data1[] { 0, 0xFF };
        CHECK(Coder::deserialize<uint16_t>(data1) == 0xFF);
        uint8_t data2[] { 0x12, 0x34 };
        CHECK(Coder::deserialize<uint16_t>(data2) == 0x1234);
    }

    TEST_CASE("deserialize a double word")
    {
        uint8_t data[] { 0x12, 0x34, 0x56, 0x78 };
        CHECK(Coder::deserialize<uint32_t>(data) == 0x12345678);
    }

    TEST_CASE("deserialize a quad word")
    {
        uint8_t data[] { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
        CHECK(Coder::deserialize<uint64_t>(data) == 0x0123456789ABCDEF);
    }

    TEST_CASE("serialize a byte")
    {
        uint8_t data[] { 1 };
        Coder::serialize<uint8_t>(0, data);
        CHECK(data[0] == 0);
        Coder::serialize<uint8_t>(0xFF, data);
        CHECK(data[0] == 0xFF);
    }

    TEST_CASE("serialize a word")
    {
        uint8_t data[] { 1, 1 };
        Coder::serialize<uint16_t>(0, data);
        CHECK(data[0] == 0);
        CHECK(data[1] == 0);
        Coder::serialize<uint16_t>(0x1234, data);
        CHECK(data[0] == 0x12);
        CHECK(data[1] == 0x34);
    }

    TEST_CASE("serialize a double word")
    {
        uint8_t data[] { 1, 1, 1, 1 };
        Coder::serialize<uint32_t>(0, data);
        CHECK(data[0] == 0);
        CHECK(data[1] == 0);
        CHECK(data[2] == 0);
        CHECK(data[3] == 0);
        Coder::serialize<uint32_t>(0x12345678, data);
        CHECK(data[0] == 0x12);
        CHECK(data[1] == 0x34);
        CHECK(data[2] == 0x56);
        CHECK(data[3] == 0x78);
    }

    TEST_CASE("serialize a quad word")
    {
        uint8_t data[] { 1, 1, 1, 1, 1, 1, 1, 1 };
        Coder::serialize<uint64_t>(0, data);
        CHECK(data[0] == 0);
        CHECK(data[1] == 0);
        CHECK(data[2] == 0);
        CHECK(data[3] == 0);
        CHECK(data[4] == 0);
        CHECK(data[5] == 0);
        CHECK(data[6] == 0);
        CHECK(data[7] == 0);
        Coder::serialize<uint64_t>(0x0123456789ABCDEF, data);
        CHECK(data[0] == 0x01);
        CHECK(data[1] == 0x23);
        CHECK(data[2] == 0x45);
        CHECK(data[3] == 0x67);
        CHECK(data[4] == 0x89);
        CHECK(data[5] == 0xAB);
        CHECK(data[6] == 0xCD);
        CHECK(data[7] == 0xEF);
    }

    TEST_CASE("serialize & deserialize over the range of uint16_t")
    {
        uint8_t data[] { 1, 1 };
        for (uint16_t i = 0; i < std::numeric_limits<uint16_t>::max(); ++i) {
            Coder::serialize<uint16_t>(i, data);
            CHECK(Coder::deserialize<uint16_t>(data) == i);
        }
    }
} // TEST_SUITE
