/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#include "xclox/datetime.hpp"

using namespace xclox;
using namespace std::chrono;

TEST_SUITE("DateTime")
{
    constexpr seconds NtpDeltaSeconds { 2208988800 };
    constexpr seconds UnixRolloverSeconds { 2147483647 };

    auto compare = [](const DateTime& dt, int year, int month, int day, int hour, int minute, int second, int milli, int micro, int nano) {
        return dt.year() == year && dt.month() == month && dt.day() == day
            && dt.hour() == hour && dt.minute() == minute && dt.second() == second
            && dt.millisecond() == milli && dt.microsecond() == micro && dt.nanosecond() == nano;
    };

    TEST_CASE("constructible")
    {
        SUBCASE("default")
        {
            DateTime dt;

            CHECK(compare(dt, 0, 0, 0, 0, 0, 0, 0, 0, 0));
        }
        SUBCASE("values")
        {
            SUBCASE("before unix rollover")
            {
                DateTime dt1(Date(2038, 1, 19), Time(3, 14, 6, Time::Nanoseconds(999999999)));
                DateTime dt2(UnixRolloverSeconds - seconds(1) + nanoseconds(999999999));
                DateTime dt3(system_clock::time_point(UnixRolloverSeconds - seconds(1) + microseconds(999999)));

                CHECK(compare(dt1, 2038, 1, 19, 3, 14, 6, 999, 999999, 999999999));
                CHECK(compare(dt2, 2038, 1, 19, 3, 14, 6, 999, 999999, 999999999));
                CHECK(compare(dt3, 2038, 1, 19, 3, 14, 6, 999, 999999, 999999000));
            }
            SUBCASE("unix rollover")
            {
                DateTime dt1(Date(2038, 1, 19), Time(3, 14, 7));
                DateTime dt2(UnixRolloverSeconds);
                DateTime dt3(system_clock::time_point { UnixRolloverSeconds });

                CHECK(compare(dt1, 2038, 1, 19, 3, 14, 7, 0, 0, 0));
                CHECK(compare(dt2, 2038, 1, 19, 3, 14, 7, 0, 0, 0));
                CHECK(compare(dt3, 2038, 1, 19, 3, 14, 7, 0, 0, 0));
            }
            SUBCASE("after unix rollover")
            {
                DateTime dt1(Date(2038, 1, 19), Time(3, 14, 8, Time::Milliseconds(500)));
                DateTime dt2(UnixRolloverSeconds + milliseconds(1500));
                DateTime dt3(system_clock::time_point(UnixRolloverSeconds + milliseconds(1500)));

                CHECK(compare(dt1, 2038, 1, 19, 3, 14, 8, 500, 500000, 500000000));
                CHECK(compare(dt2, 2038, 1, 19, 3, 14, 8, 500, 500000, 500000000));
                CHECK(compare(dt3, 2038, 1, 19, 3, 14, 8, 500, 500000, 500000000));
            }
            SUBCASE("before unix epoch")
            {
                DateTime dt1(Date(1969, 12, 31), Time(23, 59, 59, Time::Nanoseconds(123456789)));
                DateTime dt2(nanoseconds(-876543211));
                DateTime dt3(system_clock::time_point(microseconds(-876544)));

                CHECK(compare(dt1, 1969, 12, 31, 23, 59, 59, 123, 123456, 123456789));
                CHECK(compare(dt2, 1969, 12, 31, 23, 59, 59, 123, 123456, 123456789));
                CHECK(compare(dt3, 1969, 12, 31, 23, 59, 59, 123, 123456, 123456000));
            }
            SUBCASE("unix epoch")
            {
                DateTime dt1(Date(1970, 1, 1), Time(0, 0, 0));
                DateTime dt2(seconds(0));
                DateTime dt3(system_clock::time_point(seconds(0)));

                CHECK(compare(dt1, 1970, 1, 1, 0, 0, 0, 0, 0, 0));
                CHECK(compare(dt2, 1970, 1, 1, 0, 0, 0, 0, 0, 0));
                CHECK(compare(dt3, 1970, 1, 1, 0, 0, 0, 0, 0, 0));
            }
            SUBCASE("after unix epoch")
            {
                DateTime dt1(Date(1970, 1, 1), Time(0, 0, 0, Time::Nanoseconds(1)));
                DateTime dt2(nanoseconds(1));
                DateTime dt3(system_clock::time_point(microseconds(1)));

                CHECK(compare(dt1, 1970, 1, 1, 0, 0, 0, 0, 0, 1));
                CHECK(compare(dt2, 1970, 1, 1, 0, 0, 0, 0, 0, 1));
                CHECK(compare(dt3, 1970, 1, 1, 0, 0, 0, 0, 1, 1000));
            }
            SUBCASE("before ntp epoch")
            {
                DateTime dt1(Date(1899, 12, 31), Time(23, 59, 59, Time::Nanoseconds(999999999)));
                DateTime dt2(-NtpDeltaSeconds - nanoseconds(1));
                DateTime dt3(system_clock::time_point(-NtpDeltaSeconds - microseconds(1)));

                CHECK(compare(dt1, 1899, 12, 31, 23, 59, 59, 999, 999999, 999999999));
                CHECK(compare(dt2, 1899, 12, 31, 23, 59, 59, 999, 999999, 999999999));
                CHECK(compare(dt3, 1899, 12, 31, 23, 59, 59, 999, 999999, 999999000));
            }
            SUBCASE("ntp epoch")
            {
                DateTime dt1(Date(1900, 1, 1), Time(0, 0, 0));
                DateTime dt2(-NtpDeltaSeconds);
                DateTime dt3(system_clock::time_point(-NtpDeltaSeconds));

                CHECK(compare(dt1, 1900, 1, 1, 0, 0, 0, 0, 0, 0));
                CHECK(compare(dt2, 1900, 1, 1, 0, 0, 0, 0, 0, 0));
                CHECK(compare(dt3, 1900, 1, 1, 0, 0, 0, 0, 0, 0));
            }
            SUBCASE("after ntp epoch")
            {
                DateTime dt1(Date(1900, 1, 1), Time(0, 0, 0, DateTime::Nanoseconds(1)));
                DateTime dt2(-NtpDeltaSeconds + nanoseconds(1));
                DateTime dt3(system_clock::time_point(-NtpDeltaSeconds + microseconds(1)));

                CHECK(compare(dt1, 1900, 1, 1, 0, 0, 0, 0, 0, 1));
                CHECK(compare(dt2, 1900, 1, 1, 0, 0, 0, 0, 0, 1));
                CHECK(compare(dt3, 1900, 1, 1, 0, 0, 0, 0, 1, 1000));
            }
        }
    }

    TEST_CASE("validatable")
    {
        CHECK(DateTime().isValid() == false);
        CHECK(DateTime(Date(), Time()).isValid() == false);
        CHECK(DateTime(Date(2001, 2, 3), Time(Time::Hours(-1))).isValid() == false);
        CHECK(DateTime(Date(0, 1, 2), Time(Time::Hours(1))).isValid() == false);
        CHECK(DateTime(Date(2006, 5, 4), Time(3, 2, 1)).isValid() == true);
        CHECK(DateTime(microseconds(-1)).isValid() == true);
        CHECK(DateTime(system_clock::time_point(microseconds(-1))).isValid() == true);
        //
        // additional tests.
        //
        CHECK(DateTime(system_clock::now().time_since_epoch()).isValid() == true);
        CHECK(DateTime(system_clock::now()).isValid() == true);
        CHECK(DateTime::current().isValid() == true);
        CHECK(DateTime::epoch().isValid() == true);
    }

    TEST_CASE("comparable")
    {
        CHECK(DateTime(Date(2012, 3, 27), Time(8, 55, 21, Time::Nanoseconds(123456789))) < DateTime(Date(2017, 3, 27), Time(8, 55, 21, Time::Nanoseconds(123456789))));
        CHECK(DateTime(Date(2012, 3, 27), Time(8, 55, 21, Time::Nanoseconds(123456789))) <= DateTime(Date(2017, 3, 27), Time(8, 55, 21, Time::Nanoseconds(123456789))));
        CHECK(DateTime(Date(2012, 3, 27), Time(8, 55, 21, Time::Nanoseconds(123456789))) <= DateTime(Date(2012, 3, 27), Time(8, 55, 21, Time::Nanoseconds(123456789))));
        CHECK(DateTime(Date(2012, 9, 27), Time(8, 55, 21, Time::Nanoseconds(123456789))) > DateTime(Date(2012, 3, 27), Time(8, 55, 21, Time::Nanoseconds(123456789))));
        CHECK(DateTime(Date(2012, 3, 27), Time(9, 55, 21, Time::Nanoseconds(123456789))) >= DateTime(Date(2012, 3, 27), Time(8, 55, 21, Time::Nanoseconds(123456789))));
        CHECK(DateTime(Date(2015, 3, 27), Time(1, 55, 21, Time::Nanoseconds(123456789))) >= DateTime(Date(2015, 3, 27), Time(1, 55, 21, Time::Nanoseconds(123456789))));
        CHECK(DateTime(Date(2012, 3, 27), Time(8, 55, 21, Time::Nanoseconds(123456789))) == DateTime(Date(2012, 3, 27), Time(8, 55, 21, Time::Nanoseconds(123456789))));
        CHECK(DateTime(Date(2017, 12, 6), Time(16, 32, 4, Time::Nanoseconds(987654321))) != DateTime(Date(2012, 3, 27), Time(8, 55, 21, Time::Nanoseconds(123456789))));
    }

    TEST_CASE("copyable")
    {
        DateTime dt1;
        DateTime dt2(Date(2001, 2, 3), Time(4, 5, 6));
        DateTime dt3(Date(2006, 5, 4), Time(3, 2, 1));

        SUBCASE("construction")
        {
            DateTime dt4(dt1);
            DateTime dt5(dt2);
            DateTime dt6(std::move(dt3));

            CHECK(dt1 == dt4);
            CHECK(dt2 == dt5);
            CHECK(dt3 == dt6);
        }
        SUBCASE("assignment")
        {
            DateTime dt4 = dt1;
            DateTime dt5 = dt2;
            DateTime dt6 = std::move(dt3);

            CHECK(dt1 == dt4);
            CHECK(dt2 == dt5);
            CHECK(dt3 == dt6);
        }
    }

    TEST_CASE("current datetime")
    {
        // DateTime dt = DateTime::current();
        // std::time_t tTime = std::time(nullptr);
        // std::tm* tmTime = std::gmtime(&tTime);

        // CHECK(dt.year() == tmTime->tm_year + 1900);
        // CHECK(dt.month() == tmTime->tm_mon + 1);
        // CHECK(dt.day() == tmTime->tm_mday);
        // CHECK(dt.hour() == tmTime->tm_hour);
        // CHECK(dt.minute() == tmTime->tm_min);
        // CHECK(dt.second() == tmTime->tm_sec);
    }

    TEST_CASE("epoch")
    {
        CHECK(DateTime::epoch() == DateTime(Date(1970, 1, 1), Time(0, 0, 0)));
    }

    TEST_CASE("addition & subtraction")
    {
        SUBCASE("add duration")
        {
            // Adding time durations which don't affect the date part.
            CHECK(DateTime(Date(2045, 3, 27), Time(16, 2, 3, 4)).addDuration(DateTime::Minutes(10)) == DateTime(Date(2045, 3, 27), Time(16, 12, 3, 4)));
            CHECK(DateTime(Date(2045, 3, 27), Time(1, 2, 3, 4)).addDuration(DateTime::Hours(3)) == DateTime(Date(2045, 3, 27), Time(4, 2, 3, 4)));
            // Adding time durations which affect the date part.
            CHECK(DateTime(Date(2045, 3, 27), Time(16, 2, 3, 4)).addDuration(DateTime::Hours(10)) == DateTime(Date(2045, 3, 28), Time(2, 2, 3, 4)));
            // Adding date durations which only affect the date part.
            CHECK(DateTime(Date(2045, 3, 27), Time(1, 2, 3, 4)).addDuration(DateTime::Days(3)) == DateTime(Date(2045, 3, 30), Time(1, 2, 3, 4)));
            // Adding a negative duration
            CHECK(DateTime(Date(2045, 3, 27), Time(1, 2, 3, 4)).addDuration(-DateTime::Days(3)) == DateTime(Date(2045, 3, 24), Time(1, 2, 3, 4)));
        }
        SUBCASE("subtract duration")
        {
            // Subtracting time durations which don't affect the date part.
            CHECK(DateTime(Date(2045, 3, 27), Time(16, 2, 3, 4)).subtractDuration(DateTime::Minutes(10)) == DateTime(Date(2045, 3, 27), Time(15, 52, 3, 4)));
            CHECK(DateTime(Date(2045, 3, 27), Time(7, 2, 3, 4)).subtractDuration(DateTime::Hours(3)) == DateTime(Date(2045, 3, 27), Time(4, 2, 3, 4)));
            // Subtracting time durations which affect the date part.
            CHECK(DateTime(Date(2045, 3, 27), Time(6, 2, 3, 4)).subtractDuration(DateTime::Hours(10)) == DateTime(Date(2045, 3, 26), Time(20, 2, 3, 4)));
            // Subtracting date durations which only affect the date part.
            CHECK(DateTime(Date(2045, 3, 27), Time(1, 2, 3, 4)).subtractDuration(DateTime::Days(3)) == DateTime(Date(2045, 3, 24), Time(1, 2, 3, 4)));
            // Subtracting a negative duration
            CHECK(DateTime(Date(2045, 3, 27), Time(1, 2, 3, 4)).subtractDuration(-DateTime::Days(3)) == DateTime(Date(2045, 3, 30), Time(1, 2, 3, 4)));
        }
        SUBCASE("operators")
        {
            CHECK((DateTime(Date(2017, 3, 27), Time(22, 19, 53, 4)) - DateTime(Date(2017, 3, 26), Time(22, 12, 53, 4))) == DateTime::Days(1) + DateTime::Minutes(7));
            CHECK((DateTime(Date(2017, 12, 29), Time(23, 12, 53, 4)) + DateTime::Hours(35)) == DateTime(Date(2017, 12, 31), Time(10, 12, 53, 4)));
            CHECK((DateTime(Date(2017, 12, 31), Time(10, 12, 53, 4)) - DateTime::Hours(35)) == DateTime(Date(2017, 12, 29), Time(23, 12, 53, 4)));
        }
    }

    TEST_CASE("formatting")
    {
        CHECK(DateTime().toString("d/M/yyyy, hh:mm:ss.fffffffff") == "");
        CHECK(DateTime(Date(1999, 5, 18), Time(23, 55, 57, Time::Nanoseconds(123456789))).toString("d/M/yyyy, hh:mm:ss.fffffffff") == "18/5/1999, 23:55:57.123456789");
        CHECK(DateTime(Date(1969, 12, 31), Time(23, 59, 59, 1)).toString("yyyy-MM-dd hh:mm:ss.fff") == "1969-12-31 23:59:59.001");
        //
        // additional tests.
        //
        const auto& format = "yyyy-MM-dd hh:mm:ss.fffffffff";
        // NTP epoch
        CHECK(DateTime(-NtpDeltaSeconds - seconds(1)).toString(format) == "1899-12-31 23:59:59.000000000");
        CHECK(DateTime(-NtpDeltaSeconds - nanoseconds(1)).toString(format) == "1899-12-31 23:59:59.999999999");
        CHECK(DateTime(-NtpDeltaSeconds).toString(format) == "1900-01-01 00:00:00.000000000");
        CHECK(DateTime(-NtpDeltaSeconds + nanoseconds(1)).toString(format) == "1900-01-01 00:00:00.000000001");
        CHECK(DateTime(-NtpDeltaSeconds + seconds(1)).toString(format) == "1900-01-01 00:00:01.000000000");
        // UNIX epoch
        CHECK(DateTime(-seconds(1)).toString(format) == "1969-12-31 23:59:59.000000000");
        CHECK(DateTime(-nanoseconds(1)).toString(format) == "1969-12-31 23:59:59.999999999");
        CHECK(DateTime(seconds(0)).toString(format) == "1970-01-01 00:00:00.000000000");
        CHECK(DateTime(nanoseconds(1)).toString(format) == "1970-01-01 00:00:00.000000001");
        CHECK(DateTime(seconds(1)).toString(format) == "1970-01-01 00:00:01.000000000");
        // UNIX rollover
        CHECK(DateTime(UnixRolloverSeconds - seconds(1)).toString(format) == "2038-01-19 03:14:06.000000000");
        CHECK(DateTime(UnixRolloverSeconds - nanoseconds(1)).toString(format) == "2038-01-19 03:14:06.999999999");
        CHECK(DateTime(UnixRolloverSeconds).toString(format) == "2038-01-19 03:14:07.000000000");
        CHECK(DateTime(UnixRolloverSeconds + nanoseconds(1)).toString(format) == "2038-01-19 03:14:07.000000001");
        CHECK(DateTime(UnixRolloverSeconds + seconds(1)).toString(format) == "2038-01-19 03:14:08.000000000");
    }

    TEST_CASE("parsing")
    {
        CHECK(DateTime::fromString("18/5/1999, 23:55:57.123456789", "d/M/yyyy, hh:mm:ss.fffffffff") == DateTime(Date(1999, 5, 18), Time(23, 55, 57, Time::Nanoseconds(123456789))));
    }

    TEST_CASE("Julian day conversion")
    {
        SUBCASE("to")
        {
            CHECK(DateTime(Date(-4714, 11, 24), Time(12, 0, 0)).toJulianDay() == doctest::Approx(0).epsilon(0.0000000001));
            CHECK(DateTime(Date(-4714, 11, 26), Time(0, 0, 0)).toJulianDay() == doctest::Approx(1.5).epsilon(0.0000000001));
            CHECK(DateTime(Date(2017, 12, 31), Time(00, 9, 35)).toJulianDay() == doctest::Approx(2458118.506655093).epsilon(0.0000000001));
        }
        SUBCASE("from")
        {
            CHECK(DateTime::fromJulianDay(0) == DateTime(Date(-4714, 11, 24), Time(12, 0, 0)));
            CHECK(DateTime::fromJulianDay(1.5) == DateTime(Date(-4714, 11, 26), Time(0, 0, 0)));
            CHECK(DateTime::fromJulianDay(2458118.506655093) == DateTime(Date(2017, 12, 31), Time(0, 9, 35)));
        }
    }

    TEST_CASE("diffing")
    {
        CHECK(DateTime::nanosecondsBetween(DateTime(Date(2017, 1, 15), Time(12, 45, 36, DateTime::Nanoseconds(123001013))), DateTime(Date(2017, 1, 15), Time(12, 45, 37))) == 876998987);
        CHECK(DateTime::microsecondsBetween(DateTime(Date(2017, 1, 15), Time(12, 45, 36, DateTime::Microseconds(123001))), DateTime(Date(2017, 1, 15), Time(12, 45, 37))) == 876999);
        CHECK(DateTime::millisecondsBetween(DateTime(Date(2017, 1, 15), Time(12, 45, 36, 123)), DateTime(Date(2017, 1, 15), Time(12, 45, 37))) == 877);
        CHECK(DateTime::secondsBetween(DateTime(Date(2002, 1, 1), Time(15, 45, 36)), DateTime(Date(2002, 1, 2), Time(15, 2, 37))) == 83821);
        CHECK(DateTime::minutesBetween(DateTime(Date(2000, 1, 2), Time(23, 45, 36)), DateTime(Date(2000, 1, 2), Time(12, 2, 36))) == 703);
        CHECK(DateTime::hoursBetween(DateTime(Date(1998, 1, 1), Time(23, 45, 36)), DateTime(Date(1998, 1, 2), Time(12, 2, 36))) == 12);
        CHECK(DateTime::daysBetween(DateTime(Date(1970, 1, 1), Time(23, 2, 36)), DateTime(Date(1971, 1, 1), Time(23, 2, 36))) == 365);
    }

    TEST_CASE("serialization & deserialization")
    {
        DateTime dt;
        std::stringstream ss;
        ss << DateTime(Date(2014, 11, 9), Time(2, 44, 21, 987));
        ss >> dt;
        CHECK(dt == DateTime(Date(2014, 11, 9), Time(2, 44, 21, 987)));
    }
} // TEST_SUITE
