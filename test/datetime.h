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
        SUBCASE("era of year as a positive(+) or negative(-) sign")
        {
            CHECK(DateTime(Date(-1, 2, 3), Time(4, 5, 6)).toString("#") == "-");
            CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("#") == "+");
        }
        SUBCASE("era of year as CE or BCE")
        {
            CHECK(DateTime(Date(-1, 2, 3), Time(4, 5, 6)).toString("E") == "BCE");
            CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("E") == "CE");
        }
        SUBCASE("year as one digit or more (1, 9999+)")
        {
            CHECK(DateTime(Date(-1, 2, 3), Time(4, 5, 6)).toString("y") == "1");
            CHECK(DateTime(Date(11, 2, 3), Time(4, 5, 6)).toString("y") == "11");
        }
        SUBCASE("year of era as two digits (00, 99)")
        {
            CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("yy") == "01");
            CHECK(DateTime(Date(-1, 2, 3), Time(4, 5, 6)).toString("yy") == "01");
            CHECK(DateTime(Date(123, 2, 3), Time(4, 5, 6)).toString("yy") == "23");
            CHECK(DateTime(Date(-1234, 2, 3), Time(4, 5, 6)).toString("yy") == "34");
        }
        SUBCASE("year as four digits (0000, 9999)")
        {
            CHECK(DateTime(Date(-1, 2, 3), Time(4, 5, 6)).toString("yyyy") == "0001");
            CHECK(DateTime(Date(12345, 2, 3), Time(4, 5, 6)).toString("yyyy") == "2345");
        }
        SUBCASE("month of year as one digit or more (1, 12)")
        {
            CHECK(DateTime(Date(1, 1, 3), Time(4, 5, 6)).toString("M") == "1");
        }
        SUBCASE("month of year as two digits (01, 12)")
        {
            CHECK(DateTime(Date(1, 1, 3), Time(4, 5, 6)).toString("MM") == "01");
        }
        SUBCASE("month of year as short name (e.g. Feb)")
        {
            CHECK(DateTime(Date(1, 1, 3), Time(4, 5, 6)).toString("MMM") == "Jan");
        }
        SUBCASE("month of year as short name (e.g. February)")
        {
            CHECK(DateTime(Date(1, 1, 3), Time(4, 5, 6)).toString("MMMM") == "January");
        }
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
        SUBCASE("one-digit hour (0, 23)")
        {
            CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("h") == "4");
        }
        SUBCASE("two-digit hour (00, 23)")
        {
            CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("hh") == "04");
        }
        SUBCASE("one-digit minute (0, 59)")
        {
            CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("m") == "5");
        }
        SUBCASE("two-digit minute (00, 59)")
        {
            CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("mm") == "05");
        }
        SUBCASE("one-digit second (0, 59)")
        {
            CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("s") == "6");
        }
        SUBCASE("two-digit second (00, 59)")
        {
            CHECK(DateTime(Date(1, 2, 3), Time(4, 5, 6)).toString("ss") == "06");
        }
        SUBCASE("subsecond")
        {
            DateTime dt1(Date(1, 2, 3), Time(4, 5, 6, 0));
            DateTime dt2(Date(1, 2, 3), Time(4, 5, 6, DateTime::Nanoseconds(987654321)));

            SUBCASE("one-digit subsecond (0, 9)")
            {
                CHECK(dt1.toString("f") == "0");
                CHECK(dt2.toString("f") == "9");
            }
            SUBCASE("two-digit subsecond (00, 99)")
            {
                CHECK(dt1.toString("ff") == "00");
                CHECK(dt2.toString("ff") == "98");
            }
            SUBCASE("three-digit subsecond (000, 999)")
            {
                CHECK(dt1.toString("fff") == "000");
                CHECK(dt2.toString("fff") == "987");
            }
            SUBCASE("four-digit subsecond (0000, 9999)")
            {
                CHECK(dt1.toString("ffff") == "0000");
                CHECK(dt2.toString("ffff") == "9876");
            }
            SUBCASE("five-digit subsecond (00000, 99999)")
            {
                CHECK(dt1.toString("fffff") == "00000");
                CHECK(dt2.toString("fffff") == "98765");
            }
            SUBCASE("six-digit subsecond (000000, 999999)")
            {
                CHECK(dt1.toString("ffffff") == "000000");
                CHECK(dt2.toString("ffffff") == "987654");
            }
            SUBCASE("seven-digit subsecond (0000000, 9999999)")
            {
                CHECK(dt1.toString("fffffff") == "0000000");
                CHECK(dt2.toString("fffffff") == "9876543");
            }
            SUBCASE("eight-digit subsecond (00000000, 99999999)")
            {
                CHECK(dt1.toString("ffffffff") == "00000000");
                CHECK(dt2.toString("ffffffff") == "98765432");
            }
            SUBCASE("nine-digit subsecond (000000000, 999999999)")
            {
                CHECK(dt1.toString("fffffffff") == "000000000");
                CHECK(dt2.toString("fffffffff") == "987654321");
            }
        }
        SUBCASE("before/after noon indicator (i.e. am or pm)")
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
            SUBCASE("AM / PM)")
            {
                CHECK(dt1.toString("A") == "AM");
                CHECK(dt2.toString("A") == "AM");
                CHECK(dt3.toString("A") == "PM");
                CHECK(dt4.toString("A") == "PM");
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
