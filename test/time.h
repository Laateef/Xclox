/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#include "xclox/time.hpp"

using namespace xclox;
using namespace std::chrono;

TEST_SUITE("Time")
{
    TEST_CASE("constructible")
    {
        SUBCASE("default")
        {
            Time t;

            CHECK(t.hour() == 0);
            CHECK(t.minute() == 0);
            CHECK(t.second() == 0);
            CHECK(t.millisecond() == 0);
            CHECK(t.microsecond() == 0);
            CHECK(t.nanosecond() == 0);
        }
        SUBCASE("hour, minute, second")
        {
            Time t(13, 44, 2);

            CHECK(t.hour() == 13);
            CHECK(t.minute() == 44);
            CHECK(t.second() == 2);
            CHECK(t.millisecond() == 0);
            CHECK(t.microsecond() == 0);
            CHECK(t.nanosecond() == 0);
        }
        SUBCASE("time values with a subsecond duration")
        {
            Time t(13, 44, 2, Time::Nanoseconds(781945521));

            CHECK(t.hour() == 13);
            CHECK(t.minute() == 44);
            CHECK(t.second() == 2);
            CHECK(t.millisecond() == 781);
            CHECK(t.microsecond() == 781945);
            CHECK(t.nanosecond() == 781945521);
        }
        SUBCASE("aliased time duration")
        {
            CHECK(Time(Time::Hours(23)) == Time(23, 0, 0));
            CHECK(Time(Time::Minutes(178)) == Time(2, 58, 0, 0));
            CHECK(Time(Time::Seconds(7199)) == Time(1, 59, 59, 0));
            CHECK(Time(Time::Milliseconds(7198943)) == Time(1, 59, 58, 943));
            CHECK(Time(Time::Microseconds(74362675869)) == Time(20, 39, 22, Time::Microseconds(675869)));
            CHECK(Time(Time::Nanoseconds(8974362675546)) == Time(2, 29, 34, Time::Nanoseconds(362675546)));
            CHECK(Time(Time::Hours(16) + Time::Minutes(18) + Time::Seconds(55) + Time::Milliseconds(178) + Time::Microseconds(221) + Time::Nanoseconds(759)) == Time(16, 18, 55, Time::Nanoseconds(178221759)));
        }
        SUBCASE("system time duration")
        {
            const auto& duration = system_clock::now().time_since_epoch() % hours(24);
            Time t(duration);

            CHECK(t.hour() == duration_cast<hours>(duration).count());
            CHECK(t.minute() == duration_cast<minutes>(duration % hours(1)).count());
            CHECK(t.second() == duration_cast<seconds>(duration % minutes(1)).count());
            CHECK(t.millisecond() == duration_cast<milliseconds>(duration % seconds(1)).count());
            CHECK(t.microsecond() == duration_cast<microseconds>(duration % seconds(1)).count());
            CHECK(t.nanosecond() == duration_cast<nanoseconds>(duration % seconds(1)).count());
        }
        SUBCASE("system time point")
        {
            const auto& now = system_clock::now();
            Time t(now);

            CHECK(t.hour() == duration_cast<hours>(now.time_since_epoch() % hours(24)).count());
            CHECK(t.minute() == duration_cast<minutes>(now.time_since_epoch() % hours(1)).count());
            CHECK(t.second() == duration_cast<seconds>(now.time_since_epoch() % minutes(1)).count());
            CHECK(t.millisecond() == duration_cast<milliseconds>(now.time_since_epoch() % seconds(1)).count());
            CHECK(t.microsecond() == duration_cast<microseconds>(now.time_since_epoch() % seconds(1)).count());
            CHECK(t.nanosecond() == duration_cast<nanoseconds>(now.time_since_epoch() % seconds(1)).count());
        }
    }

    TEST_CASE("validatable")
    {
        CHECK(Time().isValid() == false);
        CHECK(Time(Time::Hours(-1)).isValid() == false);
        CHECK(Time(Time::Hours(23)).isValid() == true);
        CHECK(Time(Time::Hours(24)).isValid() == false);
        CHECK(Time(Time::Hours(25)).isValid() == false);
        // additional tests.
        CHECK(Time(0, 0, 0).isValid() == true);
        CHECK(Time(1, 2, 3).isValid() == true);
        CHECK(Time(-1, 0, 3).isValid() == false);
        CHECK(Time(30, 20, 10).isValid() == false);
        CHECK(Time(system_clock::now().time_since_epoch()).isValid() == false);
        CHECK(Time(system_clock::now()).isValid() == true);
        CHECK(Time::current().isValid() == true);
        CHECK(Time::midnight().isValid() == true);
    }

    TEST_CASE("comparable")
    {
        CHECK(Time(7, 9, 2, Time::Nanoseconds(675869233)) < Time(7, 45, 22, Time::Nanoseconds(536969233)));
        CHECK(Time(7, 9, 2, Time::Nanoseconds(536969435)) <= Time(7, 9, 2, Time::Nanoseconds(536969435)));
        CHECK(Time(8, 9, 2, Time::Nanoseconds(675869676)) > Time(7, 45, 22, Time::Nanoseconds(536969212)));
        CHECK(Time(7, 46, 2, Time::Nanoseconds(675869112)) >= Time(7, 45, 22, Time::Nanoseconds(536969112)));
        CHECK(Time(15, 4, 12, Time::Nanoseconds(554969231)) == Time(15, 4, 12, Time::Nanoseconds(554969231)));
        CHECK(Time(7, 9, 2, Time::Nanoseconds(675869123)) != Time(4, 45, 22, Time::Nanoseconds(536969321)));
    }

    TEST_CASE("copyable")
    {
        Time t1;
        Time t2(1, 2, 3);
        Time t3(3, 2, 1);

        SUBCASE("construction")
        {
            Time t4(t1);
            Time t5(t2);
            Time t6(std::move(t3));
            CHECK(t1 == t4);
            CHECK(t2 == t5);
            CHECK(t3 == t6);
        }
        SUBCASE("assignment")
        {
            Time t4 = t1;
            Time t5 = t2;
            Time t6 = std::move(t3);

            CHECK(t1 == t4);
            CHECK(t2 == t5);
            CHECK(t3 == t6);
        }
    }

    TEST_CASE("current time")
    {
        CHECK(abs(duration_cast<milliseconds>(Time::current() - Time(system_clock::now())).count()) < 100);
    }

    TEST_CASE("midnight")
    {
        CHECK((Time::midnight() - Time(0, 0, 0)).count() == 0);
    }

    TEST_CASE("addition & subtraction")
    {
        SUBCASE("hours")
        {
            CHECK(Time(Time::Hours(7)).addHours(2) == Time(9, 0, 0, 0));
            CHECK(Time(Time::Hours(9)).subtractHours(2) == Time(7, 0, 0, 0));
        }
        SUBCASE("minutes")
        {
            CHECK(Time(Time::Minutes(178)).addMinutes(2) == Time(3, 0, 0, 0));
            CHECK(Time(Time::Minutes(180)).subtractMinutes(2) == Time(2, 58, 0, 0));
        }
        SUBCASE("seconds")
        {
            CHECK(Time(Time::Seconds(55)).addSeconds(9) == Time(0, 1, 4, 0));
            CHECK(Time(Time::Seconds(64)).subtractSeconds(9) == Time(0, 0, 55, 0));
        }
        SUBCASE("milliseconds")
        {
            CHECK(Time(Time::Milliseconds(555)).addMilliseconds(445) == Time(0, 0, 1, 0));
            CHECK(Time(Time::Milliseconds(1000)).subtractMilliseconds(445) == Time(0, 0, 0, 555));
        }
        SUBCASE("microseconds")
        {
            CHECK(Time(Time::Microseconds(555)).addMicroseconds(445) == Time(0, 0, 0, Time::Microseconds(1000)));
            CHECK(Time(Time::Microseconds(1000)).subtractMicroseconds(445) == Time(0, 0, 0, Time::Microseconds(555)));
        }
        SUBCASE("nanoseconds")
        {
            CHECK(Time(Time::Nanoseconds(8974362675556)).addNanoseconds(445) == Time(2, 29, 34, Time::Nanoseconds(362676001)));
            CHECK(Time(Time::Nanoseconds(8974362676001)).subtractNanoseconds(445) == Time(2, 29, 34, Time::Nanoseconds(362675556)));
        }
        SUBCASE("operators")
        {
            CHECK(Time(11, 23, 11) - Time(10, 23, 11) == Time::Hours(1));
            CHECK(Time(11, 23, 11) - Time::Hours(10) == Time(1, 23, 11));
            CHECK(Time(1, 23, 11) + Time::Hours(10) == Time(11, 23, 11));
        }
    }

    TEST_CASE("diffing")
    {
        CHECK(Time::hoursBetween(Time(10, 23, 25), Time(11, 23, 29)) == 1);
        CHECK(Time::minutesBetween(Time(11, 23, 11), Time(11, 53, 11)) == 30);
        CHECK(Time::secondsBetween(Time(9, 23, 55), Time(9, 23, 35)) == -20);
        CHECK(Time::millisecondsBetween(Time(7, 23, 11, 850), Time(7, 23, 12, 900)) == 1050);
        CHECK(Time::microsecondsBetween(Time(13, 23, 20, Time::Microseconds(789500)), Time(13, 23, 20, Time::Microseconds(789400))) == -100);
        CHECK(Time::nanosecondsBetween(Time(18, 56, 5, Time::Nanoseconds(789500235)), Time(18, 56, 5, Time::Nanoseconds(789500135))) == -100);
    }

    TEST_CASE("formatting")
    {
        SUBCASE("empty format")
        {
            CHECK(Time(1, 2, 3).toString("") == "");
        }
        SUBCASE("invalid time")
        {
            CHECK(Time().toString("H:m:s") == "");
            CHECK(Time(0, 0, -1).toString("H:m:s") == "");
            CHECK(Time(Time::Hours(24)).toString("HH:mm:ss") == "");
        }
        SUBCASE("12 hours")
        {
            CHECK(Time(23, 45, 2).toString("H:m:s") == "11:45:2");
            CHECK(Time(0, 45, 2).toString("H:m:s") == "12:45:2");
            CHECK(Time(3, 45, 2).toString("HH:m:s") == "03:45:2");
        }
        SUBCASE("meridiem label small letters")
        {
            CHECK(Time(3, 45, 2).toString("HH:mm:ss a") == "03:45:02 am");
            CHECK(Time(13, 45, 2).toString("HH:mm:ss a") == "01:45:02 pm");
            CHECK(Time(Time::Hours(0)).toString("HH:mm:ss a") == "12:00:00 am");
            CHECK(Time(Time::Hours(12)).toString("HH:mm:ss a") == "12:00:00 pm");
        }
        SUBCASE("meridiem label capital letters")
        {
            CHECK(Time(3, 45, 2).toString("HH:mm:ss A") == "03:45:02 AM");
            CHECK(Time(13, 45, 2).toString("HH:mm:ss A") == "01:45:02 PM");
            CHECK(Time(Time::Hours(0)).toString("HH:mm:ss A") == "12:00:00 AM");
            CHECK(Time(Time::Hours(12)).toString("HH:mm:ss A") == "12:00:00 PM");
        }
        SUBCASE("reserve unparsable expressions")
        {
            CHECK(Time(3, 45, 2).toString("hhmmss") == "034502");
            CHECK(Time(21, 52, 41).toString("hhmmss ieee") == "215241 ieee");
        }
        SUBCASE("24 hours")
        {
            CHECK(Time(0, 0, 0).toString("h:m:s") == "0:0:0");
            CHECK(Time(0, 0, 0).toString("hh:mm:ss") == "00:00:00");
            CHECK(Time(22, 45, 2).toString("h:m:s") == "22:45:2");
            CHECK(Time(3, 45, 2).toString("hh:m:s") == "03:45:2");
        }
        SUBCASE("minutes")
        {
            CHECK(Time(Time::Hours(22) + Time::Minutes(5)).toString("h:m") == "22:5");
            CHECK(Time(Time::Hours(22) + Time::Minutes(5)).toString("h:mm") == "22:05");
        }
        SUBCASE("seconds")
        {
            CHECK(Time(Time::Minutes(55) + Time::Seconds(7)).toString("m:s") == "55:7");
            CHECK(Time(Time::Minutes(55) + Time::Seconds(7)).toString("m:ss") == "55:07");
        }
        SUBCASE("fraction")
        {
            Time t(Time::Hours(7) + Time::Minutes(9) + Time::Seconds(2) + Time::Milliseconds(675) + Time::Microseconds(869) + Time::Nanoseconds(93));

            CHECK(t.toString("hh:mm:ss.f") == "07:09:02.6");
            CHECK(t.toString("hh:mm:ss.ff") == "07:09:02.67");
            CHECK(t.toString("hh:mm:ss.fff") == "07:09:02.675");
            CHECK(t.toString("hh:mm:ss.ffff") == "07:09:02.6758");
            CHECK(t.toString("hh:mm:ss.fffff") == "07:09:02.67586");
            CHECK(t.toString("hh:mm:ss.ffffff") == "07:09:02.675869");
            CHECK(t.toString("hh:mm:ss.fffffff") == "07:09:02.6758690");
            CHECK(t.toString("hh:mm:ss.ffffffff") == "07:09:02.67586909");
            CHECK(t.toString("hh:mm:ss.fffffffff") == "07:09:02.675869093");
        }
        SUBCASE("fraction - zero milliseconds")
        {
            Time t(Time::Hours(7) + Time::Minutes(9) + Time::Seconds(2) + Time::Microseconds(869) + Time::Nanoseconds(93));

            CHECK(t.toString("hh:mm:ss.f") == "07:09:02.0");
            CHECK(t.toString("hh:mm:ss.ff") == "07:09:02.00");
            CHECK(t.toString("hh:mm:ss.fff") == "07:09:02.000");
            CHECK(t.toString("hh:mm:ss.ffff") == "07:09:02.0008");
            CHECK(t.toString("hh:mm:ss.fffff") == "07:09:02.00086");
            CHECK(t.toString("hh:mm:ss.ffffff") == "07:09:02.000869");
            CHECK(t.toString("hh:mm:ss.fffffff") == "07:09:02.0008690");
            CHECK(t.toString("hh:mm:ss.ffffffff") == "07:09:02.00086909");
            CHECK(t.toString("hh:mm:ss.fffffffff") == "07:09:02.000869093");
        }
        SUBCASE("fraction - zero microseconds")
        {
            Time t(Time::Hours(7) + Time::Minutes(9) + Time::Seconds(2) + Time::Milliseconds(675) + Time::Nanoseconds(44));

            CHECK(t.toString("h:m:s") == "7:9:2");
            CHECK(t.toString("hh:mm:ss") == "07:09:02");
            CHECK(t.toString("hh:mm:ss.f") == "07:09:02.6");
            CHECK(t.toString("hh:mm:ss.ff") == "07:09:02.67");
            CHECK(t.toString("hh:mm:ss.fff") == "07:09:02.675");
            CHECK(t.toString("hh:mm:ss.ffff") == "07:09:02.6750");
            CHECK(t.toString("hh:mm:ss.fffff") == "07:09:02.67500");
            CHECK(t.toString("hh:mm:ss.ffffff") == "07:09:02.675000");
            CHECK(t.toString("hh:mm:ss.fffffff") == "07:09:02.6750000");
            CHECK(t.toString("hh:mm:ss.ffffffff") == "07:09:02.67500004");
            CHECK(t.toString("hh:mm:ss.fffffffff") == "07:09:02.675000044");
        }
        SUBCASE("fraction - zero nanoseconds")
        {
            Time t(Time::Hours(7) + Time::Minutes(9) + Time::Seconds(2) + Time::Milliseconds(675) + Time::Microseconds(869));

            CHECK(t.toString("h:m:s") == "7:9:2");
            CHECK(t.toString("hh:mm:ss") == "07:09:02");
            CHECK(t.toString("hh:mm:ss.f") == "07:09:02.6");
            CHECK(t.toString("hh:mm:ss.ff") == "07:09:02.67");
            CHECK(t.toString("hh:mm:ss.fff") == "07:09:02.675");
            CHECK(t.toString("hh:mm:ss.ffff") == "07:09:02.6758");
            CHECK(t.toString("hh:mm:ss.fffff") == "07:09:02.67586");
            CHECK(t.toString("hh:mm:ss.ffffff") == "07:09:02.675869");
            CHECK(t.toString("hh:mm:ss.fffffff") == "07:09:02.6758690");
            CHECK(t.toString("hh:mm:ss.ffffffff") == "07:09:02.67586900");
            CHECK(t.toString("hh:mm:ss.fffffffff") == "07:09:02.675869000");
            CHECK(t.toString("hh:mm:ss.fff fff fff") == "07:09:02.675 675 675");
        }
    }

    TEST_CASE("parsing")
    {
        CHECK(Time::fromString("9", "h") == Time(Time::Hours(9)));
        CHECK(Time::fromString("01", "hh") == Time(Time::Hours(1)));
        CHECK(Time::fromString("9", "H") == Time(Time::Hours(9)));
        CHECK(Time::fromString("12", "H") == Time(Time::Hours(12)));
        CHECK(Time::fromString("01", "HH") == Time(Time::Hours(1)));
        CHECK(Time::fromString("01 pm", "HH a") == Time(Time::Hours(13)));
        CHECK(Time::fromString("01 PM", "HH A") == Time(Time::Hours(13)));
        CHECK(Time::fromString("3", "m") == Time(Time::Minutes(3)));
        CHECK(Time::fromString("03", "mm") == Time(Time::Minutes(3)));
        CHECK(Time::fromString("37", "s") == Time(Time::Seconds(37)));
        CHECK(Time::fromString("06", "ss") == Time(Time::Seconds(6)));
        CHECK(Time::fromString("1", "f") == Time(Time::Milliseconds(100)));
        CHECK(Time::fromString("12", "ff") == Time(Time::Milliseconds(120)));
        CHECK(Time::fromString("123", "fff") == Time(Time::Milliseconds(123)));
        CHECK(Time::fromString("1234", "ffff") == Time(Time::Microseconds(123400)));
        CHECK(Time::fromString("12345", "fffff") == Time(Time::Microseconds(123450)));
        CHECK(Time::fromString("123456", "ffffff") == Time(Time::Microseconds(123456)));
        CHECK(Time::fromString("1234567", "fffffff") == Time(Time::Nanoseconds(123456700)));
        CHECK(Time::fromString("12345678", "ffffffff") == Time(Time::Nanoseconds(123456780)));
        CHECK(Time::fromString("123456789", "fffffffff") == Time(Time::Nanoseconds(123456789)));
        CHECK(Time::fromString("14:32:09.123456789", "hh:mm:ss.fffffffff") == Time(14, 32, 9, Time::Nanoseconds(123456789)));
        CHECK(Time::fromString("143209", "hhmmss") == Time(14, 32, 9));
        CHECK(Time::fromString("ieee 143209", "ieee hhmmss") == Time(14, 32, 9));
    }

    TEST_CASE("conversion")
    {
        SUBCASE("nanoseconds")
        {
            CHECK(Time(23, 56, 33, Time::Nanoseconds(978432162)).toNanosecondsSinceMidnight() == 86193978432162);
        }
        SUBCASE("microseconds")
        {
            CHECK(Time(23, 56, 33, Time::Nanoseconds(978432162)).toMicrosecondsSinceMidnight() == 86193978432);
        }
        SUBCASE("milliseconds")
        {
            CHECK(Time(23, 56, 33, Time::Nanoseconds(978432162)).toMillisecondsSinceMidnight() == 86193978);
        }
        SUBCASE("seconds")
        {
            CHECK(Time(23, 56, 33, Time::Nanoseconds(978432162)).toSecondsSinceMidnight() == 86193);
        }
        SUBCASE("minutes")
        {
            CHECK(Time(23, 56, 33, Time::Nanoseconds(978432162)).toMinutesSinceMidnight() == 1436);
        }
        SUBCASE("hours")
        {
            CHECK(Time(23, 56, 33, Time::Nanoseconds(978432162)).toHoursSinceMidnight() == 23);
        }
        SUBCASE("broken")
        {
            Time t(14, 32, 9);
            std::tm tmTime = t.toBrokenStdTime();
            CHECK(tmTime.tm_hour == t.hour());
            CHECK(tmTime.tm_min == t.minute());
            CHECK(tmTime.tm_sec == t.second());
        }
        SUBCASE("scalar")
        {
            Time t(14, 32, 9);
            std::time_t tTime = t.toScalarStdTime();
            CHECK(tTime == t.toSecondsSinceMidnight());
        }
    }

    TEST_CASE("serialization & deserialization")
    {
        Time t;
        std::stringstream ss;
        ss << Time(14, 32, 9);
        ss >> t;
        CHECK(t == Time(14, 32, 9));
    }
} // TEST_SUITE
