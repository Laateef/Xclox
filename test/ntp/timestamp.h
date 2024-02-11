/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#include "xclox/ntp/timestamp.hpp"

#include "tools/helper.hpp"

using namespace xclox::ntp;
using namespace std::chrono;

TEST_SUITE("Timestamp")
{
    constexpr uint32_t ShortFormMax { std::numeric_limits<uint32_t>::max() };
    constexpr uint32_t ShortFormMid { ShortFormMax / 2 + 1 };
    constexpr uint64_t ShortFormLen { ShortFormMax + 1ull };
    constexpr uint64_t LongFormMax { std::numeric_limits<uint64_t>::max() };

    TEST_CASE("constant values")
    {
        CHECK(ShortFormMid == 0x80000000);
        CHECK(ShortFormLen == 0x100000000);
    }

    TEST_CASE("default constructible to zero")
    {
        Timestamp t;
        CHECK(t.value() == 0);
    }

    TEST_CASE("constructible from a raw timestamp value")
    {
        Timestamp t1(0);
        CHECK(t1.value() == 0);

        Timestamp t2(LongFormMax);
        CHECK(t2.value() == LongFormMax);
    }

    TEST_CASE("constructible from seconds and a fraction")
    {
        Timestamp t1(0, 0);
        CHECK(t1.value() == 0);

        Timestamp t2(0, ShortFormMax);
        CHECK(t2.value() == ShortFormMax);

        Timestamp t3(ShortFormMax, ShortFormMax);
        CHECK(t3.value() == LongFormMax);
    }

    TEST_CASE("constructible from a duration")
    {
        Timestamp t1(seconds(0));
        CHECK(t1.value() == 0);

        Timestamp t2(seconds(1));
        CHECK(t2.value() == ShortFormLen);

        Timestamp t3(milliseconds(500));
        CHECK(t3.value() == ShortFormMid);

        Timestamp t4(seconds { ShortFormLen });
        CHECK(t4.value() == 0);

        Timestamp t5(seconds { ShortFormLen + 1 });
        CHECK(t5.value() == ShortFormLen);
    }

    TEST_CASE("constructible from a time point")
    {
        Timestamp t1(system_clock::time_point {});
        CHECK(t1.seconds() == xclox::ntp::internal::EpochDeltaSeconds.count());
        CHECK(t1.fraction() == 0);

        Timestamp t2(system_clock::time_point { milliseconds(500) - xclox::ntp::internal::EpochDeltaSeconds });
        CHECK(t2.seconds() == 0);
        CHECK(t2.fraction() == ShortFormMid);

        Timestamp t3(system_clock::time_point { seconds(ShortFormLen) - xclox::ntp::internal::EpochDeltaSeconds + milliseconds(500) });
        CHECK(t3.seconds() == 0);
        CHECK(t3.fraction() == ShortFormMid);
    }

    TEST_CASE("breakable down into seconds and a fraction")
    {
        Timestamp t1(0);
        CHECK(t1.seconds() == 0);
        CHECK(t1.fraction() == 0);

        Timestamp t2(ShortFormMax);
        CHECK(t2.seconds() == 0);
        CHECK(t2.fraction() == ShortFormMax);

        Timestamp t3(ShortFormLen);
        CHECK(t3.seconds() == 1);
        CHECK(t3.fraction() == 0);

        Timestamp t4(LongFormMax);
        CHECK(t4.seconds() == ShortFormMax);
        CHECK(t4.fraction() == ShortFormMax);
    }

    TEST_CASE("convertible into duration")
    {
        Timestamp t1(0);
        CHECK(t1.duration() == seconds(0));

        Timestamp t2(ShortFormLen);
        CHECK(t2.duration() == seconds(1));

        Timestamp t3(ShortFormMid);
        CHECK(t3.duration() == microseconds(500000));

        Timestamp t4(LongFormMax);
        CHECK(t4.duration() == seconds(ShortFormLen) - system_clock::duration(1));
    }

    TEST_CASE("retains duration with system clock precision ± 1 unit")
    {
        auto currentDuration = system_clock::duration(0);
        const auto& endDuration = currentDuration + milliseconds(1);
        while (currentDuration < endDuration) {
            CHECK(abs((currentDuration - Timestamp(currentDuration).duration()).count()) <= 1);
            currentDuration = currentDuration + system_clock::duration(1);
        }
    }

    TEST_CASE("comparable")
    {
        CHECK(Timestamp() == Timestamp());
        CHECK(Timestamp(1) == Timestamp(1));
        CHECK(Timestamp(1, 1) == Timestamp(1, 1));
        CHECK_FALSE(Timestamp(1) == Timestamp(1, 1));
        CHECK(Timestamp(system_clock::duration(1)) == Timestamp(system_clock::duration(1)));
        CHECK(Timestamp() != Timestamp(1));
        CHECK(Timestamp(1) != Timestamp(1, 1));
        CHECK(Timestamp(system_clock::duration(0)) != Timestamp(system_clock::duration(1)));
        CHECK_FALSE(Timestamp(system_clock::duration(1)) != Timestamp(system_clock::duration(1)));
    }

    TEST_CASE("subtractable")
    {
        CHECK(Timestamp() - Timestamp() == system_clock::duration(0));
        CHECK(Timestamp(ShortFormLen) - Timestamp(0) == seconds(1));
        CHECK(Timestamp(0) - Timestamp(ShortFormLen) == -seconds(1));
        CHECK(Timestamp(LongFormMax) - Timestamp(LongFormMax) == system_clock::duration(0));
    }
} // TEST_SUITE
