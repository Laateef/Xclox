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
        CHECK(abs(duration_cast<milliseconds>(DateTime::current().toStdTimePoint() - system_clock::now()).count()) < 100);
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
        SUBCASE("empty format")
        {
            CHECK(DateTime(Date(2024, 2, 15), Time(2, 3, 1, 4)).toString("") == "");
        }
        SUBCASE("era of year")
        {
            SUBCASE("positive(+) or negative(-) sign")
            {
                CHECK(DateTime(Date(-1, 2, 3), Time(4, 5, 6)).toString("#") == "-");
                CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("#") == "+");
            }
            SUBCASE("CE / BCE")
            {
                CHECK(DateTime(Date(-1, 2, 3), Time(4, 5, 6)).toString("E") == "BCE");
                CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("E") == "CE");
            }
        }
        SUBCASE("year")
        {
            SUBCASE("minimum number of digits (1, 9999)")
            {
                CHECK(DateTime(Date(-1, 2, 3), Time(4, 5, 6)).toString("y") == "1");
                CHECK(DateTime(Date(11, 2, 3), Time(4, 5, 6)).toString("y") == "11");
            }
            SUBCASE("two digits (00, 99)")
            {
                CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("yy") == "01");
                CHECK(DateTime(Date(-1, 2, 3), Time(4, 5, 6)).toString("yy") == "01");
                CHECK(DateTime(Date(123, 2, 3), Time(4, 5, 6)).toString("yy") == "23");
                CHECK(DateTime(Date(-1234, 2, 3), Time(4, 5, 6)).toString("yy") == "34");
            }
            SUBCASE("four digits (0000, 9999)")
            {
                CHECK(DateTime(Date(-1, 2, 3), Time(4, 5, 6)).toString("yyyy") == "0001");
                CHECK(DateTime(Date(12345, 2, 3), Time(4, 5, 6)).toString("yyyy") == "2345");
            }
        }
        SUBCASE("month")
        {
            SUBCASE("minimum number of digits (1, 12)")
            {
                CHECK(DateTime(Date(1, 1, 3), Time(4, 5, 6)).toString("M") == "1");
            }
            SUBCASE("two digits (01, 12)")
            {
                CHECK(DateTime(Date(1, 1, 3), Time(4, 5, 6)).toString("MM") == "01");
            }
            SUBCASE("short name (e.g. Feb)")
            {
                CHECK(DateTime(Date(1, 1, 3), Time(4, 5, 6)).toString("MMM") == "Jan");
            }
            SUBCASE("long name (e.g. February)")
            {
                CHECK(DateTime(Date(1, 1, 3), Time(4, 5, 6)).toString("MMMM") == "January");
            }
        }
        SUBCASE("day")
        {
            SUBCASE("day of month as one digit or more (1, 31)")
            {
                CHECK(DateTime(Date(1, 1, 1), Time(4, 5, 6)).toString("d") == "1");
            }
            SUBCASE("day of month as two digits (00, 31)")
            {
                CHECK(DateTime(Date(1, 1, 1), Time(4, 5, 6)).toString("dd") == "01");
            }
            SUBCASE("day of week as short name (e.g. Fri)")
            {
                CHECK(DateTime(Date(2024, 2, 18), Time(4, 5, 6)).toString("ddd") == "Sun");
            }
            SUBCASE("day of week as long name (e.g. Friday)")
            {
                CHECK(DateTime(Date(2024, 2, 18), Time(4, 5, 6)).toString("dddd") == "Sunday");
            }
        }
        SUBCASE("hour")
        {
            SUBCASE("24-hour clock")
            {
                SUBCASE("minimum number of digits (0, 23)")
                {
                    CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("h") == "4");
                }
                SUBCASE("two digits (00, 23)")
                {
                    CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("hh") == "04");
                }
            }
            SUBCASE("12-hour clock")
            {
                SUBCASE("minimum number of digits (0, 23)")
                {
                    CHECK(DateTime(Date(1, 2, 3), Time(0, 5, 6)).toString("H") == "12");
                    CHECK(DateTime(Date(1, 2, 3), Time(12, 5, 6)).toString("H") == "12");
                    CHECK(DateTime(Date(1, 2, 3), Time(13, 5, 6)).toString("H") == "1");
                }
                SUBCASE("two digits (00, 23)")
                {
                    CHECK(DateTime(Date(1, 2, 3), Time(14, 5, 6)).toString("HH") == "02");
                }
            }
            SUBCASE("before/after noon indicator")
            {
                Date d(1, 2, 3);
                DateTime dt1(d, Time(0, 0, 0));
                DateTime dt2(d, Time(11, 59, 59));
                DateTime dt3(d, Time(12, 0, 0));
                DateTime dt4(d, Time(23, 59, 59));

                SUBCASE("am / pm")
                {
                    CHECK(dt1.toString("a") == "am");
                    CHECK(dt2.toString("a") == "am");
                    CHECK(dt3.toString("a") == "pm");
                    CHECK(dt4.toString("a") == "pm");
                }
                SUBCASE("AM / PM")
                {
                    CHECK(dt1.toString("A") == "AM");
                    CHECK(dt2.toString("A") == "AM");
                    CHECK(dt3.toString("A") == "PM");
                    CHECK(dt4.toString("A") == "PM");
                }
            }
        }
        SUBCASE("minute")
        {
            SUBCASE("minimum number of digits (0, 59)")
            {
                CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("m") == "5");
            }
            SUBCASE("two digits (00, 59)")
            {
                CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("mm") == "05");
            }
        }
        SUBCASE("second")
        {
            SUBCASE("minimum number of digits (0, 59)")
            {
                CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("s") == "6");
            }
            SUBCASE("two digits (00, 59)")
            {
                CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("ss") == "06");
            }
        }
        SUBCASE("subsecond")
        {
            DateTime dt1(Date(1, 2, 3), Time(4, 5, 6, 0));
            DateTime dt2(Date(1, 2, 3), Time(4, 5, 6, DateTime::Nanoseconds(123456780)));

            SUBCASE("one-digit subsecond (0, 9)")
            {
                CHECK(dt1.toString("f") == "0");
                CHECK(dt2.toString("f") == "1");
            }
            SUBCASE("nine-digit subsecond (000000000, 999999999)")
            {
                CHECK(dt1.toString("fffffffff") == "000000000");
                CHECK(dt2.toString("fffffffff") == "123456780");
            }
        }
        SUBCASE("unrecognized flags are preserved")
        {
            const auto& text = "- GNU'S NOT UNIX!";
            CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6, DateTime::Nanoseconds(987654321))).toString(text) == text);
        }
        SUBCASE("combinations of format specifiers")
        {
            DateTime dt(Date(2024, 2, 18), Time(21, 46, 7, DateTime::Nanoseconds(987654321)));
            CHECK_EQ(dt.toString(" # E y-yy-yyyy M-MM-MMM-MMMM d-dd-ddd-dddd h-hh-m-mm-s-ss f-ff-fff-ffff-fffff-ffffff-fffffff-ffffffff-fffffffff a A "),
                " + CE 2024-24-2024 2-02-Feb-February 18-18-Sun-Sunday 21-21-46-46-7-07 9-98-987-9876-98765-987654-9876543-98765432-987654321 pm PM ");
        }
        SUBCASE("flags with unrecognized length are preserved")
        {
            DateTime dt(Date(2024, 2, 18), Time(21, 46, 7, DateTime::Nanoseconds(987654321)));
            CHECK(dt.toString(" yyy yyyyy ddddd MMMMM hhh mmm sss ffffffffff ") == " yyy yyyyy ddddd MMMMM hhh mmm sss ffffffffff ");
        }
        SUBCASE("default format")
        {
            CHECK(DateTime(Date(2024, 2, 18), Time(21, 46, 7, DateTime::Nanoseconds(987654321))).toString() == "2024-02-18 21:46:07");
        }
        SUBCASE("invalid date or time")
        {
            CHECK(DateTime().toString("d/M/yyyy, hh:mm:ss.fffffffff") == "");
            CHECK(DateTime(Date(0, 0, 0), Time(-4, -5, 66)).toString("#y-M-d h:m:s.f") == "");
            CHECK(DateTime(Date(1, -2, -3), Time(4, 5, 6)).toString("E yy-MMM-ddd h-mm-ss A") == "");
            CHECK(DateTime(Date(0, 2, 3), Time(4, 5, 6, -9)).toString("@ yyyy-MM-dd hh:mm:ss.fff a") == "");
        }
        //
        // additional tests.
        //
        // ISO format
        CHECK(DateTime(Date(2024, 2, 18), Time(21, 46, 7, DateTime::Nanoseconds(987654321))).toString("yyyy-MM-ddThh:mm:ss") == "2024-02-18T21:46:07");
        // Web format
        CHECK(DateTime(Date(2024, 2, 18), Time(21, 46, 7, DateTime::Nanoseconds(987654321))).toString("ddd, dd MMM yyyy hh:mm:ss") == "Sun, 18 Feb 2024 21:46:07");
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
        SUBCASE("year")
        {
            SUBCASE("minimum number of digits (1, 9999)")
            {
                SUBCASE("single digit")
                {
                    CHECK(DateTime::fromString("1", "y") == DateTime(Date(1, 1, 1), Time(0, 0, 0)));
                }
                SUBCASE("multiple digits")
                {
                    CHECK(DateTime::fromString("123", "y") == DateTime(Date(123, 1, 1), Time(0, 0, 0)));
                }
                SUBCASE("limited to four digits")
                {
                    CHECK(DateTime::fromString("123456789", "y") == DateTime(Date(1234, 1, 1), Time(0, 0, 0)));
                }
            }
            SUBCASE("year of era as two digits (00, 99)")
            {
                CHECK(DateTime::fromString("987654", "yy") == DateTime(Date(98, 1, 1), Time(0, 0, 0)));
            }
            SUBCASE("year as four digits (0000, 9999)")
            {
                CHECK(DateTime::fromString("123456789", "yyyy") == DateTime(Date(1234, 1, 1), Time(0, 0, 0)));
            }
        }
        SUBCASE("era of year")
        {
            SUBCASE("positive(+) or negative(-) sign")
            {
                CHECK(DateTime::fromString("-123", "#y") == DateTime(Date(-123, 1, 1), Time(0, 0, 0)));
                CHECK(DateTime::fromString("+123", "#y") == DateTime(Date(123, 1, 1), Time(0, 0, 0)));
                SUBCASE("located after the year")
                {
                    CHECK(DateTime::fromString("19-", "y#") == DateTime(Date(-19, 1, 1), Time(0, 0, 0)));
                }
            }
            SUBCASE("CE / BCE")
            {
                CHECK(DateTime::fromString("1BCE", "yE") == DateTime(Date(-1, 1, 1), Time(0, 0, 0)));
                CHECK(DateTime::fromString("987CE", "yE") == DateTime(Date(987, 1, 1), Time(0, 0, 0)));
                SUBCASE("located before the year")
                {
                    CHECK(DateTime::fromString("BCE1", "Ey") == DateTime(Date(-1, 1, 1), Time(0, 0, 0)));
                    CHECK(DateTime::fromString("CE987", "Ey") == DateTime(Date(987, 1, 1), Time(0, 0, 0)));
                }
            }
        }
        SUBCASE("unrecognized characters are skipped according to the format")
        {
            CHECK(DateTime::fromString("(1-1999 2-08) [!]{@$%^&*}", "(1-yyyy 2-MM) [!]{@$%^&*}") == DateTime(Date(1999, 8, 1), Time(0, 0, 0)));
        }
        SUBCASE("month")
        {
            SUBCASE("minimum number of digits (1, 12)")
            {
                CHECK(DateTime::fromString("12345", "yyyyM") == DateTime(Date(1234, 5, 1), Time(0, 0, 0)));
                SUBCASE("greedy when there is one more digit")
                {
                    CHECK(DateTime::fromString("19991123", "yyyyM") == DateTime(Date(1999, 11, 1), Time(0, 0, 0)));
                }
            }
            SUBCASE("two digits (01, 12)")
            {
                CHECK(DateTime::fromString("990123", "yyMM") == DateTime(Date(99, 1, 1), Time(0, 0, 0)));
            }
            SUBCASE("short name (e.g. Feb)")
            {
                CHECK(internal::getShortMonthNumber("abc") == 13);
                CHECK(DateTime::fromString("1999 Sep", "yyyy MMM") == DateTime(Date(1999, 9, 1), Time(0, 0, 0)));
                SUBCASE("case insensitive")
                {
                    CHECK(DateTime::fromString("1999 sEp", "yyyy MMM") == DateTime(Date(1999, 9, 1), Time(0, 0, 0)));
                }
                SUBCASE("the second pattern takes effect in case of multiple occurences")
                {
                    CHECK(DateTime::fromString("Oct, dec - 1999", "MMM, MMM - y") == DateTime(Date(1999, 12, 1), Time(0, 0, 0)));
                }
            }
            SUBCASE("long name (e.g. February)")
            {
                CHECK(internal::getLongMonthNumber("abcdef") == 13);
                CHECK(DateTime::fromString("1999 December", "yyyy MMMM") == DateTime(Date(1999, 12, 1), Time(0, 0, 0)));
                SUBCASE("case insensitive")
                {
                    CHECK(DateTime::fromString("1999 decEmbeR", "yyyy MMMM") == DateTime(Date(1999, 12, 1), Time(0, 0, 0)));
                }
                SUBCASE("followed by an interfering expression")
                {
                    CHECK(DateTime::fromString("deCEmber BCE 9", "MMMM E y") == DateTime(Date(-9, 12, 1), Time(0, 0, 0)));
                }
            }
        }
        SUBCASE("day")
        {
            SUBCASE("day of month")
            {
                CHECK(DateTime::fromString("1999-9-9", "yyyy-M-d") == DateTime(Date(1999, 9, 9), Time(0, 0, 0)));
            }
            SUBCASE("day of week as short name (e.g. Fri)")
            {
                CHECK(internal::search(internal::getShortWeekdayNameArray(), "mon") == 0);
                CHECK(internal::search(internal::getShortWeekdayNameArray(), "abc") == 7);
                CHECK(DateTime::fromString("Sun, 2024-2-25", "ddd, yyyy-M-d") == DateTime(Date(2024, 2, 25), Time(0, 0, 0)));
            }
            SUBCASE("day of week as long name (e.g. Friday)")
            {
                CHECK(internal::search(internal::getLongWeekdayNameArray(), "MONDAY") == 0);
                CHECK(internal::search(internal::getLongWeekdayNameArray(), "a") == 7);
                CHECK(DateTime::fromString("Sunday, 2024-2-25", "dddd, yyyy-M-d") == DateTime(Date(2024, 2, 25), Time(0, 0, 0)));
            }
        }
        SUBCASE("hour")
        {
            SUBCASE("24-hour clock")
            {
                CHECK(DateTime::fromString("2024-2-25 1", "yyyy-M-d h") == DateTime(Date(2024, 2, 25), Time(1, 0, 0)));
            }
            SUBCASE("12-hour clock")
            {
                CHECK(DateTime::fromString("1 12:00:00 am", "y H:m:s a") == DateTime(Date(1, 1, 1), Time(0, 0, 0)));
                CHECK(DateTime::fromString("1 12:59:59 AM", "y H:m:s A") == DateTime(Date(1, 1, 1), Time(0, 59, 59)));
                CHECK(DateTime::fromString("1 11:00:00 pm", "y HH:mm:ss a") == DateTime(Date(1, 1, 1), Time(23, 0, 0)));
                CHECK(DateTime::fromString("1 11:59:59 PM", "y HH:mm:ss A") == DateTime(Date(1, 1, 1), Time(23, 59, 59)));
            }
        }
        SUBCASE("minute")
        {
            CHECK(DateTime::fromString("2024-2-25 1", "yyyy-M-d m") == DateTime(Date(2024, 2, 25), Time(0, 1, 0)));
        }
        SUBCASE("second")
        {
            CHECK(DateTime::fromString("2024-2-25 1", "yyyy-M-d s") == DateTime(Date(2024, 2, 25), Time(0, 0, 1)));
        }
        SUBCASE("subsecond")
        {
            SUBCASE("single digit (0, 9)")
            {
                CHECK(DateTime::fromString("2024-2-25 1.5", "yyyy-M-d s.f") == DateTime(Date(2024, 2, 25), Time(0, 0, 1, 500)));
            }
            SUBCASE("multiple digits (000000000, 999999999)")
            {
                CHECK(DateTime::fromString("1 1.123456780", "y s.fffffffff") == DateTime(Date(1, 1, 1), Time(0, 0, 1, Time::Nanoseconds(123456780))));
            }
        }
        SUBCASE("flags with unrecognized length are ignored")
        {
            CHECK(DateTime::fromString("1999-08-07 01:02:03", "##yyyy-MM-dd hh:mm:ss").isValid() == false);
            CHECK(DateTime::fromString("1999-08-07 01:02:03", "yyyyy-MM-dd hh:mm:ss").isValid() == false);
            CHECK(DateTime::fromString("1999-08-07 01:02:03", "yyyy-MMMMMM-dd hh:mm:ss").isValid() == false);
            CHECK(DateTime::fromString("1999-08-07 01:02:03", "yyyy-MM-ddddd hh:mm:ss").isValid() == false);
            CHECK(DateTime::fromString("1999-08-07 01:02:03", "yyyy-MM-dd hhh:mm:ss").isValid() == false);
            CHECK(DateTime::fromString("1999-08-07 01:02:03", "yyyy-MM-dd hh:mmm:ss").isValid() == false);
            CHECK(DateTime::fromString("1999-08-07 01:02:03", "yyyy-MM-dd hh:mm:sss").isValid() == false);
            CHECK(DateTime::fromString("1999-08-07 01:02:03.123456789", "yyyy-MM-dd hh:mm:ss.ffffffffff").isValid() == false);
            CHECK(DateTime::fromString("1999-08-07 01:02:03.0 CE", "yyyy-MM-dd hh:mm:ss.f EE").isValid() == false);
            CHECK(DateTime::fromString("1999-08-07 01:02:03.0 am", "yyyy-MM-dd hh:mm:ss.f aa").isValid() == false);
            CHECK(DateTime::fromString("1999-08-07 01:02:03.0 AM", "yyyy-MM-dd hh:mm:ss.f AA").isValid() == false);
        }
        SUBCASE("missing data yields invalid objects")
        {
            CHECK(DateTime::fromString("abcde", "y").isValid() == false);
            CHECK(DateTime::fromString("1", "y M").isValid() == false);
            CHECK(DateTime::fromString("1", "y MMM").isValid() == false);
            CHECK(DateTime::fromString("1 abc", "y MMM").isValid() == false);
            CHECK(DateTime::fromString("1", "y d").isValid() == false);
            CHECK(DateTime::fromString("1", "y ddd").isValid() == false);
            CHECK(DateTime::fromString("1 abc", "y ddd").isValid() == false);
            CHECK(DateTime::fromString("1", "y h").isValid() == false);
            CHECK(DateTime::fromString("1", "y H").isValid() == false);
            CHECK(DateTime::fromString("1", "y m").isValid() == false);
            CHECK(DateTime::fromString("1", "y s").isValid() == false);
            CHECK(DateTime::fromString("1", "y f").isValid() == false);
            CHECK(DateTime::fromString("1", "y a").isValid() == false);
            CHECK(DateTime::fromString("1", "y A").isValid() == false);
            CHECK(DateTime::fromString("1", "y E").isValid() == false);
            CHECK(DateTime::fromString("1", "y #").isValid() == false);
        }
        SUBCASE("unrecognized identical characters are skipped, so whitespaces may be used as placeholders")
        {
            CHECK(DateTime::fromString("y:1999 / M:08 / d:07 6", " :yyyy /  :MM /  :dd h") == DateTime(Date(1999, 8, 7), Time(6, 0, 0)));
        }
        SUBCASE("default format")
        {
            CHECK(DateTime::fromString("2024-02-02 01:33:06") == DateTime(Date(2024, 2, 2), Time(1, 33, 6)));
        }
        SUBCASE("empty format") // doc
        {
            CHECK(DateTime::fromString("2024-02-02 01:33:06", "").isValid() == false);
        }
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
