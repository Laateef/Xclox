/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#include "xclox/date.hpp"

using namespace xclox;
using namespace std::chrono;

TEST_SUITE("Date")
{
    constexpr seconds NtpDeltaSeconds { 2208988800 };
    constexpr seconds UnixRolloverSeconds { 2147483647 };

    TEST_CASE("constructible")
    {
        SUBCASE("default")
        {
            Date d;

            CHECK(d.year() == 0);
            CHECK(d.month() == 0);
            CHECK(d.day() == 0);
        }
        SUBCASE("year, month, day")
        {
            Date d1(2012, 3, 27);
            CHECK(d1.year() == 2012);
            CHECK(d1.month() == 3);
            CHECK(d1.day() == 27);

            Date d2(-4714, 11, 24);
            CHECK(d2.year() == -4714);
            CHECK(d2.month() == 11);
            CHECK(d2.day() == 24);
        }
        SUBCASE("number of days since epoch")
        {
            Date d1(-duration_cast<Date::Days>(NtpDeltaSeconds));
            CHECK(d1.year() == 1900);
            CHECK(d1.month() == 1);
            CHECK(d1.day() == 1);

            Date d2(Date::Days(0));
            CHECK(d2.year() == 1970);
            CHECK(d2.month() == 1);
            CHECK(d2.day() == 1);

            Date d3(duration_cast<Date::Days>(UnixRolloverSeconds));
            CHECK(d3.year() == 2038);
            CHECK(d3.month() == 1);
            CHECK(d3.day() == 19);
        }
    }

    TEST_CASE("validatable")
    {
        CHECK(Date().isValid() == false);
        CHECK(Date(0, 0, 0).isValid() == false);
        CHECK(Date(0, 1, 1).isValid() == false);
        CHECK(Date(2017, -1, 1).isValid() == false);
        CHECK(Date(2018, 1, -1).isValid() == false);
        CHECK(Date(2018, -1, -1).isValid() == false);
        CHECK(Date(1, 1, 1).isValid() == true);
        //
        // additional tests.
        //
        // for (long jd = 0; jd <= 9999999; ++jd) {
        //     Date d = Date::fromJulianDay(jd);
        //     CHECK(d == Date(-4714, 11, 24).addDays(jd));
        //     CHECK(d.isValid());
        // }
        // for (int y = -9999; y <= 9999; ++y) {
        //     for (char m = 0; m <= 13; ++m)
        //         for (char d = 0; d <= 32; ++d) {
        //             if (y != 0 && m >= 1 && m <= 12 && d >= 1 && d <= Date::daysInMonthOfYear(y, m))
        //                 CHECK(Date(y, m, d).isValid());
        //             else
        //                 CHECK_FALSE(Date(y, m, d).isValid());
        //         }
        // }
    }

    TEST_CASE("comparable")
    {
        CHECK(Date(2012, 3, 27) == Date(2012, 3, 27));
        CHECK(Date(2012, 3, 1) != Date(2012, 3, 2));
        CHECK(Date(2012, 3, 1) < Date(2012, 3, 2));
        CHECK(Date(2012, 3, 2) <= Date(2012, 3, 2));
        CHECK(Date(2012, 3, 2) <= Date(2012, 3, 3));
        CHECK(Date(2012, 3, 3) > Date(2012, 3, 2));
        CHECK(Date(2012, 3, 3) >= Date(2012, 3, 3));
        CHECK(Date(2012, 3, 4) >= Date(2012, 3, 3));
    }

    TEST_CASE("copyable")
    {
        Date d1;
        Date d2(2001, 2, 3);
        Date d3(2003, 2, 1);

        SUBCASE("construction")
        {
            Date d4(d1);
            Date d5(d2);
            Date d6(std::move(d3));

            CHECK(d1 == d4);
            CHECK(d2 == d5);
            CHECK(d3 == d6);
        }
        SUBCASE("assignment")
        {
            Date d4 = d1;
            Date d5 = d2;
            Date d6 = std::move(d3);

            CHECK(d1 == d4);
            CHECK(d2 == d5);
            CHECK(d3 == d6);
        }
    }

    TEST_CASE("current date")
    {
        CHECK(Date::current() == Date(duration_cast<Date::Days>(system_clock::now().time_since_epoch())));
    }

    TEST_CASE("epoch")
    {
        CHECK(Date::epoch() == Date(1970, 1, 1));
    }

    TEST_CASE("addition & subtraction")
    {
        SUBCASE("day")
        {
            CHECK(Date(2045, 3, 27).addDays(5) == Date(2045, 4, 1));
            CHECK(Date(2045, 4, 1).subtractDays(5) == Date(2045, 3, 27));
            //
            // additional tests.
            //
            CHECK(Date(2018, 1, 1).addDays(0) == Date(2018, 1, 1));
            CHECK(Date(2018, 1, 1).subtractDays(0) == Date(2018, 1, 1));
            CHECK(Date(2018, 1, 2).addDays(-1) == Date(2018, 1, 1));
            CHECK(Date(2018, 1, 1).subtractDays(-1) == Date(2018, 1, 2));
            // long dys = 0;
            // for (int y = 1970; y <= 9999; ++y) {
            //     for (char m = 1; m <= 12; ++m)
            //         for (char d = 1; d <= Date::daysInMonthOfYear(y, m); ++d) {
            //             CHECK(Date(y, m, d) == Date(1970, 1, 1).addDays(dys++));
            //         }
            // }
            // dys = 1;
            // for (int y = 1969; y >= -9999; --y) {
            //     if (y == 0) {
            //         continue;
            //     }
            //     for (char m = 12; m >= 1; --m)
            //         for (char d = Date::daysInMonthOfYear(y, m); d >= 1; --d)
            //             CHECK(Date(y, m, d) == Date(1970, 1, 1).subtractDays(dys++));
            // }
        }
        SUBCASE("month")
        {
            // when the sum of augend and addend is smaller than 12.
            CHECK(Date(2012, 3, 27).addMonths(5) == Date(2012, 8, 27));
            // when the sum of augend and addend is bigger than 12.
            CHECK(Date(2012, 8, 27).addMonths(10) == Date(2013, 6, 27));
            // when the result of addition is 12. due to the representation of month is 1...12 rather than 0...11.
            CHECK(Date(2012, 8, 27).addMonths(4) == Date(2012, 12, 27));
            // when the addend is negative.
            CHECK(Date(2012, 3, 27).addMonths(-5) == Date(2011, 10, 27));
            // when the minuend is bigger than the subtrahend.
            CHECK(Date(2013, 6, 27).subtractMonths(5) == Date(2013, 1, 27));
            // when the minuend is smaller than the subtrahend.
            CHECK(Date(2013, 1, 27).subtractMonths(10) == Date(2012, 3, 27));
            // when the result of subtraction is 12. due to the representation of month is 1...12 rather than 0...11.
            CHECK(Date(2013, 1, 27).subtractMonths(1) == Date(2012, 12, 27));
            // when the subtrahend is negative.
            CHECK(Date(2011, 10, 27).subtractMonths(-5) == Date(2012, 3, 27));
            // when the ending day/month combination does not exist in the resulting month/year, returns the latest valid date.
            CHECK(Date(2013, 1, 31).addMonths(1) == Date(2013, 2, 28));
            CHECK(Date(2013, 2, 28).addMonths(1) == Date(2013, 3, 28));
            CHECK(Date(2012, 3, 31).subtractMonths(1) == Date(2012, 2, 29));
            //
            // additional tests.
            //
            CHECK(Date(2018, 1, 1).addMonths(0) == Date(2018, 1, 1));
            CHECK(Date(2018, 1, 1).subtractMonths(0) == Date(2018, 1, 1));
        }
        SUBCASE("year")
        {
            CHECK(Date(1966, 11, 2).addYears(40) == Date(2006, 11, 2));
            CHECK(Date(2006, 11, 2).subtractYears(40) == Date(1966, 11, 2));
            //
            // additional tests.
            //
            CHECK(Date(2018, 1, 1).addYears(0) == Date(2018, 1, 1));
            CHECK(Date(2018, 1, 1).subtractYears(0) == Date(2018, 1, 1));
            CHECK(Date(2018, 1, 1).addYears(-1) == Date(2017, 1, 1));
            CHECK(Date(2018, 1, 1).subtractYears(-1) == Date(2019, 1, 1));
        }
    }

    TEST_CASE("leap year")
    {
        CHECK(Date(2012, 1, 1).isLeapYear());
        CHECK_FALSE(Date::isLeapYear(2011));
        CHECK_FALSE(Date::isLeapYear(100));
        CHECK(Date::isLeapYear(2800));
        CHECK(Date::isLeapYear(-1));
        CHECK_FALSE(Date::isLeapYear(0)); // no year 0, should return false.
        //
        // additional tests.
        //
        CHECK(Date::isLeapYear(-4801));
        CHECK_FALSE(Date::isLeapYear(-4800));
        CHECK(Date::isLeapYear(-4445));
        CHECK_FALSE(Date::isLeapYear(-4444));
        CHECK_FALSE(Date::isLeapYear(-6));
        CHECK(Date::isLeapYear(-5));
        CHECK_FALSE(Date::isLeapYear(-4));
        CHECK_FALSE(Date::isLeapYear(-3));
        CHECK_FALSE(Date::isLeapYear(-2));
        CHECK_FALSE(Date::isLeapYear(1));
        CHECK_FALSE(Date::isLeapYear(2));
        CHECK_FALSE(Date::isLeapYear(3));
        CHECK(Date::isLeapYear(4));
        CHECK_FALSE(Date::isLeapYear(7));
        CHECK(Date::isLeapYear(8));
        CHECK(Date::isLeapYear(400));
        CHECK_FALSE(Date::isLeapYear(700));
        CHECK_FALSE(Date::isLeapYear(1500));
        CHECK(Date::isLeapYear(1600));
        CHECK_FALSE(Date::isLeapYear(1700));
        CHECK_FALSE(Date::isLeapYear(1800));
        CHECK_FALSE(Date::isLeapYear(1900));
        CHECK(Date::isLeapYear(2000));
        CHECK_FALSE(Date::isLeapYear(2100));
        CHECK_FALSE(Date::isLeapYear(2200));
        CHECK_FALSE(Date::isLeapYear(2300));
        CHECK(Date::isLeapYear(2400));
        CHECK_FALSE(Date::isLeapYear(2500));
        CHECK_FALSE(Date::isLeapYear(2600));
        CHECK_FALSE(Date::isLeapYear(2700));
    }

    TEST_CASE("extraction of individual components")
    {
        int year, month, day;

        Date(1989, 3, 12).getYearMonthDay(&year, &month, &day);
        Date(1989, 3, 12).getYearMonthDay(&year, &month, nullptr);
        Date(1989, 3, 12).getYearMonthDay(&year, nullptr, nullptr);
        Date(1989, 3, 12).getYearMonthDay(nullptr, nullptr, nullptr);
        Date(1989, 3, 12).getYearMonthDay(0, &month, &day);
        Date(1989, 3, 12).getYearMonthDay(0, 0, &day);
        Date(1989, 3, 12).getYearMonthDay(0, 0, 0);

        CHECK(year == 1989);
        CHECK(month == 3);
        CHECK(day == 12);
    }

    TEST_CASE("calculation")
    {
        SUBCASE("day of week")
        {
            CHECK(Date(1970, 1, 1).dayOfWeek() == static_cast<int>(Date::Weekday::Thursday));
            CHECK(Date(2001, 1, 1).dayOfWeek() == 1);
            CHECK(Date(2002, 1, 1).dayOfWeek() == 2);
            CHECK(Date(2003, 1, 1).dayOfWeek() == 3);
            CHECK(Date(2004, 1, 1).dayOfWeek() == 4);
            CHECK(Date(2010, 1, 1).dayOfWeek() == 5);
            CHECK(Date(2005, 1, 1).dayOfWeek() == 6);
            CHECK(Date(2006, 1, 1).dayOfWeek() == 7);
        }
        SUBCASE("day of year")
        {
            CHECK(Date(1970, 1, 1).dayOfYear() == 1);
            CHECK(Date(2017, 12, 2).dayOfYear() == 336);
            CHECK(Date(2064, 2, 29).dayOfYear() == 60);
        }
        SUBCASE("days in month")
        {
            CHECK(Date::daysInMonthOfYear(1970, 1) == 31);
            CHECK(Date(1970, 1, 1).daysInMonth() == 31);
            CHECK(Date(1970, 2, 1).daysInMonth() == 28);
            CHECK(Date(2012, 2, 1).daysInMonth() == 29);
            CHECK(Date(2055, 3, 1).daysInMonth() == 31);
            CHECK(Date(2013, 4, 1).daysInMonth() == 30);
            CHECK(Date(2025, 5, 1).daysInMonth() == 31);
            CHECK(Date(2036, 6, 1).daysInMonth() == 30);
            CHECK(Date(2057, 7, 1).daysInMonth() == 31);
            CHECK(Date(2088, 8, 1).daysInMonth() == 31);
            CHECK(Date(2009, 9, 1).daysInMonth() == 30);
            CHECK(Date(2001, 10, 1).daysInMonth() == 31);
            CHECK(Date(2023, 11, 1).daysInMonth() == 30);
            CHECK(Date(2023, 12, 1).daysInMonth() == 31);
        }
        SUBCASE("days in year")
        {
            CHECK(Date(1970, 1, 1).daysInYear() == 365);
            CHECK(Date(2012, 2, 2).daysInYear() == 366);
        }
        SUBCASE("number of days between two dates")
        {
            CHECK(Date::daysBetween(Date(1970, 1, 1), Date(1971, 1, 1)) == 365);
            CHECK(Date::daysBetween(Date(2012, 1, 1), Date(2016, 1, 1)) == 1461);
            CHECK(Date::daysBetween(Date(-1, 1, 1), Date(1, 1, 1)) == 366); // year -1 is a leap year.
            CHECK(Date::weeksBetween(Date(1970, 1, 8), Date(1970, 1, 1)) == -1);
            CHECK(Date::weeksBetween(Date(1970, 1, 1), Date(1971, 1, 1)) == 52);
        }
        SUBCASE("week of year")
        {
            int weekYear;

            // when the week is in the middle of the year
            CHECK(Date(2017, 12, 3).weekOfYear(&weekYear) == 48);
            CHECK(weekYear == 2017);

            // when the week is at the start of year
            CHECK(Date(2002, 12, 31).weekOfYear(&weekYear) == 1);
            CHECK(weekYear == 2003);

            // when the week is at the end of year
            CHECK(Date(2000, 1, 1).weekOfYear(&weekYear) == 52);
            CHECK(weekYear == 1999);

            // when the week is at the end of 53-week-year
            CHECK(Date(2010, 1, 1).weekOfYear(&weekYear) == 53);
            CHECK(weekYear == 2009);
        }
    }

    TEST_CASE("conversion")
    {
        SUBCASE("to Julian day")
        {
            CHECK(Date(-4714, 11, 24).toJulianDay() == 0);
            CHECK(Date(-4714, 11, 25).toJulianDay() == 1);
            CHECK(Date(1970, 1, 1).toJulianDay() == 2440588);
            CHECK(Date(2000, 1, 1).toJulianDay() == 2451545);
            CHECK(Date(2017, 12, 4).toJulianDay() == 2458092);
            //
            // additional tests.
            //
            // long jd = 0;
            // for (char d = 24; d <= Date::daysInMonthOfYear(-4714, 11); ++d)
            //     CHECK(Date(-4714, 11, d).toJulianDay() == jd++);
            // for (char d = 1; d <= Date::daysInMonthOfYear(-4714, 12); ++d)
            //     CHECK(Date(-4714, 12, d).toJulianDay() == jd++);
            // for (long y = -4713; y <= 9999; ++y) {
            //     if (y == 0) {
            //         continue;
            //     }
            //     for (char m = 1; m <= 12; ++m)
            //         for (char d = 1; d <= Date::daysInMonthOfYear(y, m); ++d)
            //             CHECK(Date(y, m, d).toJulianDay() == jd++);
            // }
        }
        SUBCASE("from Julian day")
        {
            CHECK(Date::fromJulianDay(0) == Date(-4714, 11, 24));
            CHECK(Date::fromJulianDay(1) == Date(-4714, 11, 25));
            CHECK(Date::fromJulianDay(2440588) == Date(1970, 1, 1));
            CHECK(Date::fromJulianDay(2451545) == Date(2000, 1, 1));
            CHECK(Date::fromJulianDay(2458092) == Date(2017, 12, 4));
            //
            // additional tests.
            //
            // for (long jd = 0; jd <= 9999999; ++jd) {
            //     Date d = Date::fromJulianDay(jd);
            //     CHECK(d == Date(-4714, 11, 24).addDays(jd));
            //     CHECK(d.isValid());
            // }
        }
        SUBCASE("to days since epoch")
        {
            CHECK(Date(1970, 1, 1).toDaysSinceEpoch() == 0);
            CHECK(Date(1971, 1, 1).toDaysSinceEpoch() == 365);
        }
    }

    TEST_CASE("formatting")
    {
        SUBCASE("year")
        {
            CHECK(Date(572, 4, 22).toString("y") == "572");
            CHECK(Date(1999, 4, 13).toString("yy") == "99");
            CHECK(Date(1901, 6, 11).toString("yy") == "01");
            CHECK(Date(1999, 7, 4).toString("yyyy") == "1999");
            CHECK(Date(-795, 7, 23).toString("yyyy") == "0795");
            CHECK(Date(-1795, 7, 23).toString("#yyyy") == "-1795");
            CHECK(Date(1795, 7, 23).toString("#yyyy") == "+1795");
            CHECK(Date(-1795, 7, 23).toString("yyyy E") == "1795 BCE");
            CHECK(Date(1795, 7, 23).toString("yyyy E") == "1795 CE");
            CHECK(Date(1795, 7, 23).toString("yy . yy") == "95 . 95");
        }
        SUBCASE("month")
        {
            CHECK(Date(572, 4, 22).toString("M") == "4");
            CHECK(Date(1999, 5, 13).toString("MM") == "05");
            CHECK(Date(1999, 11, 13).toString("MM") == "11");
            CHECK(Date(1901, 6, 11).toString("MMM") == "Jun");
            CHECK(Date(1999, 7, 30).toString("MMMM") == "July");
        }
        SUBCASE("day")
        {
            CHECK(Date(572, 4, 22).toString("d") == "22");
            CHECK(Date(1999, 4, 3).toString("dd") == "03");
            CHECK(Date(1901, 6, 11).toString("dd") == "11");
            CHECK(Date(1999, 7, 4).toString("ddd") == "Sun");
            CHECK(Date(2017, 12, 15).toString("dddd") == "Friday");
        }
        SUBCASE("date")
        {
            // unparsable expressions are reserved.
            CHECK(Date(2017, 12, 19).toString("yyyyMMdd ieee") == "20171219 ieee");
            // invalid dates return an empty string.
            CHECK(Date().toString("yyyy-MM-dd E") == "");
            //
            // additional tests.
            //
            CHECK(Date(0, 0, 0).toString("") == "");
            CHECK(Date(0, 0, 0).toString("yyyy-MM-dd E") == "");
            CHECK(Date(0, -1, 1).toString("yyyy-MM-dd E") == "");
            CHECK(Date(0, 1, -1).toString("yyyy-MM-dd E") == "");
            CHECK(Date(0, -1, -1).toString("yyyy-MM-dd E") == "");
            CHECK(Date(572, 4, 22).toString("y.M.d") == "572.4.22");
            CHECK(Date(2017, 4, 3).toString("yy.MM.dd") == "17.04.03");
            CHECK(Date(2007, 5, 11).toString("yy.MMM.dd") == "07.May.11");
            CHECK(Date(2017, 12, 10).toString("ddd dd.MM.yyyy") == "Sun 10.12.2017");
            CHECK(Date(2017, 12, 15).toString("dddd dd MMMM yyyy") == "Friday 15 December 2017");
            CHECK(Date(2017, 12, 16).toString("yyyy-MM-dd E") == "2017-12-16 CE");
            CHECK(Date(-2017, 12, 16).toString("#yyyy.MM.dd") == "-2017.12.16");
            CHECK(Date(2017, 12, 19).toString("yyyyMMdd") == "20171219");
        }
    }

    TEST_CASE("parsing")
    {
        SUBCASE("year")
        {
            CHECK(Date::fromString("572", "y") == Date(572, 1, 1));
            CHECK(Date::fromString("12", "yy") == Date(2012, 1, 1));
            CHECK(Date::fromString("1999", "yyyy") == Date(1999, 1, 1));
            CHECK(Date::fromString("-572", "#y") == Date(-572, 1, 1));
            CHECK(Date::fromString("+572", "#y") == Date(572, 1, 1));
            CHECK(Date::fromString("-1999", "#yyyy") == Date(-1999, 1, 1));
            CHECK(Date::fromString("+1999", "#yyyy") == Date(1999, 1, 1));
            CHECK(Date::fromString("572 CE", "y E") == Date(572, 1, 1));
            CHECK(Date::fromString("572 BCE", "y E") == Date(-572, 1, 1));
            CHECK(Date::fromString("1999 CE", "yyyy E") == Date(1999, 1, 1));
            CHECK(Date::fromString("1999 BCE", "yyyy E") == Date(-1999, 1, 1));
        }
        SUBCASE("month")
        {
            CHECK(Date::fromString("1", "M") == Date(1, 1, 1));
            CHECK(Date::fromString("02", "MM") == Date(1, 2, 1));
            CHECK(Date::fromString("Aug", "MMM") == Date(1, 8, 1));
            CHECK(Date::fromString("September", "MMMM") == Date(1, 9, 1));
            CHECK(Date::fromString("January, 2009", "MMMM, yyyy") == Date(2009, 1, 1));
            CHECK(Date::fromString("December, 2011", "MMMM, yyyy") == Date(2011, 12, 1));
        }
        SUBCASE("day")
        {
            CHECK(Date::fromString("1", "d") == Date(1, 1, 1));
            CHECK(Date::fromString("02", "dd") == Date(1, 1, 2));
            CHECK(Date::fromString("Thu, 22.05.17", "ddd, dd.MM.yy") == Date(2017, 5, 22));
            CHECK(Date::fromString("Thursday, 01.12.1989", "dddd, dd.MM.yyyy") == Date(1989, 12, 1));
        }
        SUBCASE("date")
        {
            CHECK(Date::fromString("572.4.22", "y.M.d") == Date(572, 4, 22));
            CHECK(Date::fromString("17.04.03", "yy.MM.dd") == Date(2017, 4, 3));
            CHECK(Date::fromString("07.May.11", "yy.MMM.dd") == Date(2007, 5, 11));
            CHECK(Date::fromString("Sun 10.12.2017", "ddd dd.MM.yyyy") == Date(2017, 12, 10));
            CHECK(Date::fromString("Friday 15 December 2017", "dddd dd MMMM yyyy") == Date(2017, 12, 15));
            CHECK(Date::fromString("2017-12-16 CE", "yyyy-MM-dd E") == Date(2017, 12, 16));
            CHECK(Date::fromString("-2017.12.16", "#yyyy.MM.dd") == Date(-2017, 12, 16));
            CHECK(Date::fromString("20171219", "yyyyMMdd") == Date(2017, 12, 19));
            CHECK(Date::fromString("ieee 20171219", "ieee yyyyMMdd") == Date(2017, 12, 19));
        }
    }

    TEST_CASE("serialization & deserialization")
    {
        Date d;
        std::stringstream ss;
        ss << Date(2014, 11, 9);
        ss >> d;
        CHECK(d == Date(2014, 11, 9));
    }
} // TEST_SUITE
