/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#include "xclox/ntp/packet.hpp"

#include "tools/helper.hpp"

#include <functional>
#include <map>

using namespace xclox::ntp;
using namespace std::chrono;

TEST_SUITE("Packet")
{
    Packet::DataType zeros {};

    Packet::DataType ones { []() {Packet::DataType d;d.fill(0xFF);return d; }() };

    Packet::DataType pattern {
        0xA3, // leap(2-bit), version(3-bit), mode(3-bit)
        0x02, // stratum
        0xFA, // poll
        0xEC, // precision
        0x98, 0x76, 0x54, 0x32, // root delay
        0xCB, 0xA9, 0x87, 0x65, // root dispersion
        0x23, 0x45, 0x67, 0x89, // reference ID
        0xBA, 0x98, 0x76, 0x54, 0x32, 0x10, 0xFE, 0xDC, // reference timestamp
        0xE9, 0x87, 0x65, 0x43, 0x21, 0x0F, 0xED, 0xCB, // origin timestamp
        0xE8, 0x76, 0x54, 0x32, 0x10, 0xFE, 0xDC, 0xBA, // receive timestamp
        0xE7, 0x65, 0x43, 0x21, 0x0F, 0xED, 0xCB, 0xA9 // transmit timestamp
    };

    const auto& ntpToSys = [](const system_clock::duration& duration) { return duration - xclox::ntp::internal::EpochDeltaSeconds; };

    TEST_CASE("default constructible to null")
    {
        Packet p;
        CHECK(p.data() == zeros);
    }

    TEST_CASE("constructible from raw data")
    {
        Packet p1(zeros);
        CHECK(p1.data() == zeros);

        Packet p2(ones);
        CHECK(p2.data() == ones);

        Packet p3(pattern);
        CHECK(p3.data() == pattern);
    }

    TEST_CASE("zeroed data is null")
    {
        Packet p1;
        CHECK(p1.isNull() == true);

        Packet p2(zeros);
        CHECK(p2.isNull() == true);

        Packet p3(ones);
        CHECK(p3.isNull() == false);

        for (size_t i = 0; i != std::tuple_size<Packet::DataType> {}; ++i) {
            Packet::DataType data {};
            data[i] = 1;
            Packet p4(data);
            CHECK(p4.isNull() == false);
        }
    }

    TEST_CASE("leap bits [0-1]")
    {
        for (uint8_t i = 0; i < 4; ++i) {
            Packet::DataType data {};
            data[0] = static_cast<uint8_t>(i << 6);
            Packet p(data);
            CHECK(p.leap() == i);
        }
    }

    TEST_CASE("version bits [2-4]")
    {
        for (uint8_t i = 0; i < 8; ++i) {
            Packet::DataType data {};
            data[0] = static_cast<uint8_t>(i << 3);
            Packet p(data);
            CHECK(p.version() == i);
        }
    }

    TEST_CASE("mode bits [5-7]")
    {
        for (uint8_t i = 0; i < 8; ++i) {
            Packet::DataType data {};
            data[0] = static_cast<uint8_t>(i);
            Packet p(data);
            CHECK(p.mode() == i);
        }
    }

    TEST_CASE("leap, version, and mode bits coexist")
    {
        for (uint8_t i = 0; i < 8; ++i) {
            Packet::DataType data {};
            data[0] = static_cast<uint8_t>(i << 6 | i << 3 | i);
            Packet p(data);
            if (i < 4) {
                CHECK(p.leap() == i);
            }
            CHECK(p.version() == i);
            CHECK(p.mode() == i);
        }
    }

    TEST_CASE("stratum bits [8-15]")
    {
        Packet::DataType data {};
        Packet p1(data);
        CHECK(p1.stratum() == 0);
        data[1] = 1;
        Packet p2(data);
        CHECK(p2.stratum() == 1);
        data[1] = 254;
        Packet p3(data);
        CHECK(p3.stratum() == 254);
    }

    TEST_CASE("poll bits [16-23]")
    {
        Packet::DataType data {};
        Packet p1(data);
        CHECK(p1.poll() == 0);
        data[2] = 1;
        Packet p2(data);
        CHECK(p2.poll() == 1);
        data[2] = static_cast<uint8_t>(-10);
        Packet p3(data);
        CHECK(p3.poll() == -10);
    }

    TEST_CASE("precision bits [24-31]")
    {
        Packet::DataType data {};
        Packet p1(data);
        CHECK(p1.precision() == 0);
        data[3] = 1;
        Packet p2(data);
        CHECK(p2.precision() == 1);
        data[3] = static_cast<uint8_t>(-20);
        Packet p3(data);
        CHECK(p3.precision() == -20);
    }

    TEST_CASE("32-bit values ( root delay [32-63], root dispersion [64-95], reference ID [96-127] )")
    {
        std::map<size_t, uint32_t (Packet::*)() const> offsetFunctionMap {
            { 4, &Packet::rootDelay },
            { 8, &Packet::rootDispersion },
            { 12, &Packet::referenceID }
        };
        for (const auto& pair : offsetFunctionMap) {
            Packet::DataType data {};
            Packet p1(data);
            CHECK((p1.*pair.second)() == 0);
            data[pair.first + 3] = 1;
            Packet p2(data);
            CHECK((p2.*pair.second)() == 1);
            data[pair.first + 0] = 0x01;
            data[pair.first + 1] = 0x23;
            data[pair.first + 2] = 0x45;
            data[pair.first + 3] = 0x67;
            Packet p3(data);
            CHECK((p3.*pair.second)() == 0x01234567);
        }
    }

    TEST_CASE("64-bit timestamps ( reference [128-191], origin [192-255], receive [256-319], transmit [320-383] )")
    {
        std::map<size_t, uint64_t (Packet::*)() const> offsetFunctionMap {
            { 16, &Packet::referenceTimestamp },
            { 24, &Packet::originTimestamp },
            { 32, &Packet::receiveTimestamp },
            { 40, &Packet::transmitTimestamp }
        };
        for (const auto& pair : offsetFunctionMap) {
            Packet::DataType data {};
            Packet p1(data);
            CHECK((p1.*pair.second)() == 0);
            data[pair.first + 7] = 1;
            Packet p2(data);
            CHECK((p2.*pair.second)() == 1);
            data[pair.first + 0] = 0x01;
            data[pair.first + 1] = 0x23;
            data[pair.first + 2] = 0x45;
            data[pair.first + 3] = 0x67;
            data[pair.first + 4] = 0x89;
            data[pair.first + 5] = 0xAB;
            data[pair.first + 6] = 0xCD;
            data[pair.first + 7] = 0xEF;
            Packet p3(data);
            CHECK((p3.*pair.second)() == 0x0123456789ABCDEF);
        }
    }

    TEST_CASE("constructible from values")
    {
        Packet p0(
            0, // leap
            0, // version
            0, // mode
            0, // stratum
            0, // poll
            0, // precision
            0, // rootDelay
            0, // rootDispersion
            0, // referenceID
            0, // referenceTimestamp
            0, // originTimestamp
            0, // receiveTimestamp
            0 // transmitTimestamp
        );
        CHECK(p0.data() == zeros);
        Packet p1(
            0xFF, // leap
            0xFF, // version
            0xFF, // mode
            0xFF, // stratum
            static_cast<int8_t>(0xFF), // poll
            static_cast<int8_t>(0xFF), // precision
            0xFFFFFFFF, // rootDelay
            0xFFFFFFFF, // rootDispersion
            0xFFFFFFFF, // referenceID
            0xFFFFFFFFFFFFFFFF, // referenceTimestamp
            0xFFFFFFFFFFFFFFFF, // originTimestamp
            0xFFFFFFFFFFFFFFFF, // receiveTimestamp
            0xFFFFFFFFFFFFFFFF // transmitTimestamp
        );
        CHECK(p1.data() == ones);
        Packet p2(
            0x02, // leap
            0x04, // version
            0x03, // mode
            0x02, // stratum
            static_cast<int8_t>(0xFA), // poll
            static_cast<int8_t>(0xEC), // precision
            0x98765432, // rootDelay
            0xCBA98765, // rootDispersion
            0x23456789, // referenceID
            0xBA9876543210FEDC, // referenceTimestamp
            0xE9876543210FEDCB, // originTimestamp
            0xE876543210FEDCBA, // receiveTimestamp
            0xE76543210FEDCBA9 // transmitTimestamp
        );
        CHECK(p2.data() == pattern);
    }

    TEST_CASE("comparable")
    {
        Packet p1;
        Packet p2(zeros);
        Packet p3(ones);
        Packet p4(pattern);
        CHECK(p1 == p1);
        CHECK(p1 == p2);
        CHECK_FALSE(p2 == p3);
        CHECK_FALSE(p3 == p4);
        CHECK(p3 == p3);
        CHECK(p4 == p4);
        CHECK_FALSE(p1 != p1);
        CHECK_FALSE(p1 != p2);
        CHECK(p2 != p3);
        CHECK(p3 != p4);
        CHECK_FALSE(p3 != p3);
        CHECK_FALSE(p4 != p4);
    }

    TEST_CASE("copyable")
    {
        Packet p1;
        Packet p2(zeros);
        Packet p3(ones);
        Packet p4(pattern);
        Packet p5(p1);
        Packet p6(p2);
        Packet p7(p3);
        Packet p8(p4);
        CHECK(p1 == p5);
        CHECK(p2 == p6);
        CHECK(p3 == p7);
        CHECK(p4 == p8);
    }

    TEST_CASE("movable")
    {
        Packet p1;
        Packet p2(zeros);
        Packet p3(ones);
        Packet p4(pattern);
        Packet p5(std::move(p1));
        Packet p6(std::move(p2));
        Packet p7(std::move(p3));
        Packet p8(std::move(p4));
        CHECK(p1.isNull());
        CHECK(p2.isNull());
        CHECK(p3.isNull());
        CHECK(p4.isNull());
        CHECK(p5 == Packet());
        CHECK(p6 == Packet(zeros));
        CHECK(p7 == Packet(ones));
        CHECK(p8 == Packet(pattern));
    }

    TEST_CASE("packet delay & offset")
    {
        SUBCASE("null packet")
        {
            Packet p;
            CHECK(p.delay(0) == system_clock::duration(0));
            CHECK(p.offset(0) == system_clock::duration(0));
        }
        SUBCASE("up-to-date clocks")
        {
            uint64_t origin = 0xE902661000000000; // 2023-11-17 22:22:08.00
            uint64_t receive = origin + 0x40000000; // 2023-11-17 22:22:08.25
            uint64_t transmit = origin + 0x80000000; // 2023-11-17 22:22:08.50
            uint64_t destination = origin + 0xC0000000; // 2023-11-17 22:22:08.75
            Packet p(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, origin, receive, transmit);
            CHECK(p.delay(destination) == milliseconds(500));
            CHECK(p.offset(destination) == system_clock::duration(0));
        }
        SUBCASE("zero latency")
        {
            uint64_t origin = 0xE902661000000000; // 2023-11-17 22:22:08.0
            uint64_t receive = origin; // 2023-11-17 22:22:08.0
            uint64_t transmit = origin + 0x80000000; // 2023-11-17 22:22:08.5
            uint64_t destination = transmit; // 2023-11-17 22:22:08.5
            Packet p(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, origin, receive, transmit);
            CHECK(p.delay(destination) == system_clock::duration(0));
            CHECK(p.offset(destination) == system_clock::duration(0));
        }
        SUBCASE("client clock is at NTP epoch")
        {
            uint64_t origin = 0; // 1900-01-01 00:00:00.0000
            uint64_t receive = 0xE902661010000000; // 2023-11-17 22:22:08.0625
            uint64_t transmit = receive + 0x10000000; // 2023-11-17 22:22:08.1250
            uint64_t destination = origin + 0x30000000; // 1900-01-01 00:00:00.1875
            Packet p(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, origin, receive, transmit);
            CHECK(p.delay(destination) == milliseconds(125));
            CHECK(p.offset(destination) == seconds(0xE9026610));
        }
        SUBCASE("client clock is at end of NTP era")
        {
            uint64_t origin = 0xFFFFFFFF00000000; // 2036-02-07 06:28:15.0000
            uint64_t receive = 0xE902661010000000; // 2023-11-17 22:22:08.0625
            uint64_t transmit = receive + 0x10000000; // 2023-11-17 22:22:08.1250
            uint64_t destination = origin + 0x30000000; // 2036-02-07 06:28:15.1875
            Packet p(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, origin, receive, transmit);
            CHECK(p.delay(destination) == milliseconds(125));
            CHECK(p.offset(destination) == -seconds(0x16FD99EF));
        }
        SUBCASE("client clock is at start of next NTP era")
        {
            uint64_t origin = 0; // 2036-02-07 06:28:16.0000
            uint64_t receive = 0xFFFFFFFF10000000; // 2036-02-07 06:28:15.0625
            uint64_t transmit = receive + 0x10000000; // 2036-02-07 06:28:15.1250
            uint64_t destination = origin + 0x40000000; // 2036-02-07 06:28:16.2500
            system_clock::time_point destinationTP(ntpToSys(seconds(0x100000000) + milliseconds(250))); // 2036-02-07 06:28:16.2500
            Packet p(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, origin, receive, transmit);
            CHECK(p.delay(destination) == microseconds(187500));
            // WRONG offset with a time stamp: 2036-02-07 06:28:15.218750
            CHECK(p.offset(destination) == seconds(0xFFFFFFFF) - microseconds(31250));
            // RIGHT offset with a time point: 2036-02-07 06:28:15.218750
            CHECK(p.offset(destinationTP) == -seconds(1) - microseconds(31250));
        }
        SUBCASE("server clock is at start of next NTP era")
        {
            uint64_t origin = 0xFFFFFFFF00000000; // 2036-02-07 06:28:15.0000
            uint64_t receive = 0x0000000010000000; // 2036-02-07 06:28:16.0625
            uint64_t transmit = receive + 0x10000000; // 2036-02-07 06:28:16.1250
            uint64_t destination = origin + 0x40000000; // 2036-02-07 06:28:15.2500
            system_clock::time_point destinationTP(ntpToSys(seconds(0xFFFFFFFF) + milliseconds(250))); // 2036-02-07 06:28:15.2500
            Packet p(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, origin, receive, transmit);
            CHECK(p.delay(destination) == microseconds(187500));
            // WRONG offset with a time stamp: 1900-01-01 00:00:00.218750
            CHECK(p.offset(destination) == -seconds(0xFFFFFFFF) - microseconds(31250));
            // RIGHT offset with a time point: 2036-02-07 06:28:16.218750
            CHECK(p.offset(destinationTP) == seconds(1) - microseconds(31250));
        }
        SUBCASE("client clock behind server clock by 68 years")
        {
            uint64_t origin = 0x8000000100000000; // 1968-01-20 03:14:09.0000
            uint64_t receive = 0x0000000010000000; // 2036-02-07 06:28:16.0625
            uint64_t transmit = receive + 0x10000000; // 2036-02-07 06:28:16.1250
            uint64_t destination = origin + 0x40000000; // 1968-01-20 03:14:09.2500
            system_clock::time_point destinationTP(ntpToSys(seconds(0x80000001) + milliseconds(250))); // 1968-01-20 03:14:09.2500
            Packet p(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, origin, receive, transmit);
            CHECK(p.delay(destination) == microseconds(187500));
            // WRONG offset with a time stamp: 1900-01-01 00:00:00.218750
            CHECK(p.offset(destination) == -seconds(0x80000001) - microseconds(31250));
            // RIGHT offset with a time point: 2036-02-07 06:28:16.218750
            CHECK(p.offset(destinationTP) == seconds(0x7FFFFFFF) - microseconds(31250));
        }
        SUBCASE("server clock behind client clock by 68 years")
        {
            uint64_t origin = 0x8000000000000000; // 2104-02-26 09:42:24.0000
            uint64_t receive = 0x0000000010000000; // 2036-02-07 06:28:16.0625
            uint64_t transmit = receive + 0x10000000; // 2036-02-07 06:28:16.1250
            uint64_t destination = origin + 0x40000000; // 2104-02-26 09:42:24.2500
            system_clock::time_point destinationTP(ntpToSys(seconds(0x80000000) + milliseconds(250))); // 2104-02-26 09:42:24.2500
            Packet p(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, origin, receive, transmit);
            CHECK(p.delay(destination) == microseconds(187500));
            // WRONG offset with a time stamp: 1900-01-01 00:00:00.218750
            CHECK(p.offset(destination) == -seconds(0x80000000) - microseconds(31250));
            // RIGHT offset with a time point: 2036-02-07 06:28:16.218750
            CHECK(p.offset(destinationTP) == -seconds(0x80000000) - microseconds(31250));
        }
    } // TEST_CASE
} // TEST_SUITE
