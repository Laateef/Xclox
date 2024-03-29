/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_DATE_TIME_HPP
#define XCLOX_DATE_TIME_HPP

#include "date.hpp"
#include "time.hpp"

#include <cmath>

namespace xclox {

/**
 * @class DateTime
 *
 * DateTime is an immutable class representing a datetime without a time zone in the ISO-8601 calendar system, such as "2017-12-31, 22:34:55 UTC".
 *
 * The ISO-8601 calendar system is the modern civil calendar system used today in most of the world. It is equivalent to the proleptic Gregorian calendar system.
 * DateTime describes the datetime as a Date part (year, month, day) and a Time part (hour, minute, second, subsecond).
 *
 * Internally, DateTime describes a datetime as a combination of the number of days since the epoch "1970-01-01 00:00:00.000 UTC" plus the number of nanoseconds since midnight.
 *
 * Default-constructed DateTime objects are invalid (calling isValid() on them returns false) and are set to "0000-00-00 00:00:00".
 * DateTime objects can be created by giving a Date object and a Time object. It can also be created by giving only a Date object; in this case, the time part is considered to be at midnight.
 * Also, a DateTime object can be created from a formatted string through fromString() or from a Julian day through fromJulianDay().
 * The method current() returns the current datetime obtained from the system clock.
 *
 * The datetime fields can be accessed through the methods year(), month(), day(), hour(), minute(), second(), millisecond(), microsecond(), and nanosecond().
 * Other fields, such as day-of-year, day-of-week, and week-of-year, can also be accessed through dayOfYear(), dayOfWeek(), and weekOfYear(), respectively.
 *
 * DateTime provides methods for manipulating datetimes. Years, months, days, hours, minutes, seconds, milliseconds, microseconds, and nanoseconds can be added to a datetime (through addYears(), addMonths(), addDays(), addHours(), addMinutes(), addSeconds(), addMilliseconds(), addMicroseconds(), and addNanoseconds(), respectively).
 * or subtracted from it (through subtractYears(), subtractMonths(), subtractDays(), subtractHours(), subtractMinutes(), subtractSeconds(), subtractMilliseconds(), subtractMicroseconds(), and subtractNanoseconds(), respectively).
 *
 * It also has operators for comparison. DateTime A is considered earlier than DateTime B if A is smaller than B, and so on.
 *
 * The methods weeksBetween(), daysBetween(), hoursBetween(), minutesBetween(), secondsBetween(), millisecondsBetween(), and microsecondsBetween() returns how many weeks, days, hours, minutes, seconds, milliseconds, and microseconds, respectively, are between two DateTime objects.
 *
 * The toString() method can be used to get a textual representation of the datetime formatted according to a given format string.
 *
 * @see The unit tests in @ref datetime.h for further details.
 */
class DateTime {
    using Duration = std::chrono::nanoseconds;

public:
    /**
     * @name Durations
     * @{
     */

    using Nanoseconds = Time::Nanoseconds; ///< Nanosecond duration.
    using Microseconds = Time::Microseconds; ///< Microsecond duration.
    using Milliseconds = Time::Milliseconds; ///< Millisecond duration.
    using Seconds = Time::Seconds; ///< Second duration.
    using Minutes = Time::Minutes; ///< Minute duration.
    using Hours = Time::Hours; ///< Hour duration.
    using Days = Date::Days; ///< Day duration.
    using Weeks = Date::Weeks; ///< Week duration.

    /// @}

    /**
     * @name Enumerations
     * @{
     */

    using Weekday = Date::Weekday; ///< Weekday enumeration.
    using Month = Date::Month; ///< Month enumeration.

    /// @}

    /**
     * @name Constructors and Destructors
     * @{
     */

    /// Constructs an invalid DateTime object with every field set to zero. @see isValid()
    DateTime() = default;

    /// Copy-constructs a DateTime object from \p other.
    DateTime(const DateTime& other) = default;

    /// Move-constructs a DateTime object from \p other.
    DateTime(DateTime&& other) = default;

    /**
     * Constructs a DateTime object from \p duration since the epoch "1970-01-01 00:00:00 UTC".
     * The constructed datetime has whatever precision it is given, down to nanoseconds.
     */
    explicit DateTime(const Duration& duration)
    {
        const auto& subDay = duration % Days(1);
        const auto& floatingDay = Days(duration.count() < 0 && subDay.count() != 0 ? 1 : 0);
        m_date = Date(std::chrono::duration_cast<Days>(duration - floatingDay));
        m_time = Time(subDay + floatingDay);
    }

    /// Constructs a DateTime object from the standard library's chrono time point, \p timePoint.
    explicit DateTime(const std::chrono::system_clock::time_point& timePoint)
        : DateTime(timePoint.time_since_epoch())
    {
    }

    /// Constructs a DateTime object from \p date, leaving the time part at midnight ("00:00:00").
    explicit DateTime(const Date& date)
        : m_date(date)
        , m_time(Nanoseconds::zero())
    {
    }

    /// Constructs a DateTime object from \p date and \p time.
    explicit DateTime(const Date& date, const Time& time)
        : m_date(date)
        , m_time(time)
    {
    }

    /// Default destructor.
    ~DateTime() = default;

    /// @}

    /**
     * @name Assignment Operators
     * @{
     */

    /// Copy assignment operator.
    DateTime& operator=(const DateTime& other) = default;

    /// Move assignment operator.
    DateTime& operator=(DateTime&& other) = default;

    /// @}

    /**
     * @name Comparison Operators
     * @{
     */

    /// Returns whether this datetime is earlier than \p other.
    bool operator<(const DateTime& other) const
    {
        return (this->m_date < other.m_date) || (this->m_date == other.m_date && this->m_time < other.m_time);
    }

    /// Returns whether this datetime is earlier than \p other or equal to it.
    bool operator<=(const DateTime& other) const
    {
        return (this->m_date <= other.m_date) || (this->m_date == other.m_date && this->m_time <= other.m_time);
    }

    /// Returns whether this datetime is later than \p other.
    bool operator>(const DateTime& other) const
    {
        return (this->m_date > other.m_date) || (this->m_date == other.m_date && this->m_time > other.m_time);
    }

    /// Returns whether this datetime is later than \p other or equal to it.
    bool operator>=(const DateTime& other) const
    {
        return (this->m_date >= other.m_date) || (this->m_date == other.m_date && this->m_time >= other.m_time);
    }

    /// Returns whether this datetime is equal to \p other.
    bool operator==(const DateTime& other) const
    {
        return this->m_date == other.m_date && this->m_time == other.m_time;
    }

    /// Returns whether this datetime is different from \p other.
    bool operator!=(const DateTime& other) const
    {
        return this->m_date != other.m_date || this->m_time != other.m_time;
    }

    /// @}

    /**
     * @name Addition/Subtraction Operators
     * @{
     */

    /// Returns the result of subtracting \p other from this datetime as #Nanoseconds duration.
    Nanoseconds operator-(const DateTime& other) const
    {
        return this->toStdDurationSinceEpoch() - other.toStdDurationSinceEpoch();
    }

    /// Returns the result of subtracting \p duration from this datetime as a new DateTime object.
    DateTime operator-(const Duration& duration) const
    {
        return subtractDuration(duration);
    }

    /// Returns the result of adding \p duration to this datetime as a new DateTime object.
    DateTime operator+(const Duration& duration) const
    {
        return addDuration(duration);
    }
    /// @}

    /**
     * @name Querying Methods
     * @{
     */

    /// Returns whether this datetime object represents a valid datetime. A DateTime object is valid if both the date and time parts are valid. For more information, see Date#isValid() and Time#isValid().
    bool isValid() const
    {
        return m_date.isValid() && m_time.isValid();
    }

    /// Returns the date part of this datetime.
    Date date() const
    {
        return m_date;
    }

    /// Returns the time part of this datetime.
    Time time() const
    {
        return m_time;
    }

    /// Returns the nanosecond of second (0, 999999999).
    long nanosecond() const
    {
        return m_time.nanosecond();
    }

    /// Returns the microsecond of second (0, 999999).
    long microsecond() const
    {
        return m_time.microsecond();
    }

    /// Returns the millisecond of second (0, 999).
    int millisecond() const
    {
        return m_time.millisecond();
    }

    /// Returns the second of minute (0, 59).
    int second() const
    {
        return m_time.second();
    }

    /// Returns the minute of hour (0, 59).
    int minute() const
    {
        return m_time.minute();
    }

    /// Returns the hour of day (0, 23).
    int hour() const
    {
        return m_time.hour();
    }

    /// Returns the day of month (1, 31).
    int day() const
    {
        return m_date.day();
    }

    /// Returns the month of year (1, 12), which corresponds to the enumeration #Month.
    int month() const
    {
        return m_date.month();
    }

    /// Returns the year as a number. There is no year 0. Negative numbers indicate years before 1 CE; that is, year -1 is year 1 BCE, year -2 is year 2 BCE, and so on.
    int year() const
    {
        return m_date.year();
    }

    /// Set the year, month, and day in the parameters \p year, \p month, and \p day, respectively.
    void getYearMonthDay(int* year, int* month, int* day) const
    {
        return m_date.getYearMonthDay(year, month, day);
    }

    /// Returns the weekday as a number between 1 and 7, which corresponds to the enumeration #Weekday.
    int dayOfWeek() const
    {
        return m_date.dayOfWeek();
    }

    /// Returns the day of the year as a number between 1 and 365 (1 to 366 on leap years).
    int dayOfYear() const
    {
        return m_date.dayOfYear();
    }

    /// Returns the number of days in the current month. It ranges between 28 and 31.
    int daysInMonth() const
    {
        return m_date.daysInMonth();
    }

    /// Returns the number of days in the current year. It is either 365 or 366.
    int daysInYear() const
    {
        return m_date.daysInYear();
    }

    /// Returns whether the year of this datetime is a leap year. For more information, see Date#isLeapYear(int).
    bool isLeapYear() const
    {
        return m_date.isLeapYear();
    }

    /**
     * Returns the week number of the year of this datetime, and optionally stores the year in \p weekYear.
     * The week of the year ranges between 1 and 53. Most years have 52 weeks, but some have 53.
     * According to ISO-8601, weeks start on Monday, and the first Thursday of a year is always in week 1 of that year.
     * \p weekYear is not always the same as year(). For example:
     *   - The date of "1 January 2000" falls in week 52 of the year 1999.
     *   - The date of "31 December 2002" falls in week 1 of the year 2003.
     *   - The date of "1 January 2010" falls in week 53 of the year 2009.
     */
    int weekOfYear(int* weekYear = nullptr) const
    {
        return m_date.weekOfYear(weekYear);
    }

    /**
     * Returns the name of the weekday of this datetime.
     * The short and long names of the weekdays are named according to the following convention:
     *
     *    Weekday No. | Short Name | Long Name
     *    ----------- | ---------- | ---------
     *    1           | Mon        | Monday
     *    2           | Tue        | Tuesday
     *    3           | Wed        | Wednesday
     *    4           | Thu        | Thursday
     *    5           | Fri        | Friday
     *    6           | Sat        | Saturday
     *    7           | Sun        | Sunday
     *
     * @param shortName If true, returns the short name of the weekday, such as "Sat". If false (by default), returns the long name, such as "Saturday".
     */
    std::string dayOfWeekName(bool shortName = false) const
    {
        return m_date.dayOfWeekName(shortName);
    }

    /**
     * Returns the name of the month of this datetime.
     * The short and long names of the months are named according to the following convention:
     *
     *    Month No. | Short Name | Long Name
     *    --------- | ---------- | ---------
     *    1         | Jan        | January
     *    2         | Feb        | February
     *    3         | Mar        | March
     *    4         | Apr        | April
     *    5         | May        | May
     *    6         | Jun        | June
     *    7         | Jul        | July
     *    8         | Aug        | August
     *    9         | Sep        | September
     *    10        | Oct        | October
     *    11        | Nov        | November
     *    12        | Dec        | December
     *
     * @param shortName If true, returns the short name of the month, such as "Jan". If false (by default), returns the long name, such as "January".
     */
    std::string monthName(bool shortName = false) const
    {
        return m_date.monthName(shortName);
    }

    /// @}

    /**
     * @name Addition/Subtraction Methods
     * @{
     */

    /// Returns a new DateTime object representing this datetime with \p nanoseconds added to it.
    DateTime addNanoseconds(int nanoseconds) const
    {
        return addDuration(Nanoseconds(nanoseconds));
    }

    /// Returns a new DateTime object representing this datetime with \p nanoseconds subtracted from it.
    DateTime subtractNanoseconds(int nanoseconds) const
    {
        return subtractDuration(Nanoseconds(nanoseconds));
    }

    /// Returns a new DateTime object representing this datetime with \p microseconds added to it.
    DateTime addMicroseconds(int microseconds) const
    {
        return addDuration(Microseconds(microseconds));
    }

    /// Returns a new DateTime object representing this datetime with \p microseconds subtracted from it.
    DateTime subtractMicroseconds(int microseconds) const
    {
        return subtractDuration(Microseconds(microseconds));
    }

    /// Returns a new DateTime object representing this datetime with \p milliseconds added to it.
    DateTime addMilliseconds(int milliseconds) const
    {
        return addDuration(Milliseconds(milliseconds));
    }

    /// Returns a new DateTime object representing this datetime with \p milliseconds subtracted from it.
    DateTime subtractMilliseconds(int milliseconds) const
    {
        return subtractDuration(Milliseconds(milliseconds));
    }

    /// Returns a new DateTime object representing this datetime with \p seconds added to it.
    DateTime addSeconds(int seconds) const
    {
        return addDuration(Seconds(seconds));
    }

    /// Returns a new DateTime object representing this datetime with \p seconds subtracted from it.
    DateTime subtractSeconds(int seconds) const
    {
        return subtractDuration(Seconds(seconds));
    }

    /// Returns a new DateTime object representing this datetime with \p minutes added to it.
    DateTime addMinutes(int minutes) const
    {
        return addDuration(Minutes(minutes));
    }

    /// Returns a new DateTime object representing this datetime with \p minutes subtracted from it.
    DateTime subtractMinutes(int minutes) const
    {
        return subtractDuration(Minutes(minutes));
    }

    /// Returns a new DateTime object representing this datetime with \p hours added to it.
    DateTime addHours(int hours) const
    {
        return addDuration(Hours(hours));
    }

    /// Returns a new DateTime object representing this datetime with \p hours subtracted from it.
    DateTime subtractHours(int hours) const
    {
        return subtractDuration(Hours(hours));
    }

    /// Returns a new DateTime object representing this datetime with \p days added to it.
    DateTime addDays(int days) const
    {
        return DateTime(m_date.addDays(days), m_time);
    }

    /// Returns a new DateTime object representing this datetime with \p days subtracted from it.
    DateTime subtractDays(int days) const
    {
        return DateTime(m_date.subtractDays(days), m_time);
    }

    /// Returns a new DateTime object representing this datetime with \p months added to it. See Date#addMonths() for more information about how the operation is done.
    DateTime addMonths(int months) const
    {
        return DateTime(m_date.subtractMonths(months), m_time);
    }

    /// Returns a new DateTime object representing this datetime with \p months subtracted from it. See Date#subtractMonths() for more information about how the operation is done.
    DateTime subtractMonths(int months) const
    {
        return DateTime(m_date.subtractMonths(months), m_time);
    }

    /// Returns a new DateTime object representing this datetime with \p years added to it.
    DateTime addYears(int years) const
    {
        return DateTime(m_date.addYears(years), m_time);
    }

    /// Returns a new DateTime object representing this datetime with \p years subtracted from it.
    DateTime subtractYears(int years) const
    {
        return DateTime(m_date.subtractYears(years), m_time);
    }

    /// Returns a new DateTime object representing this datetime with \p duration added to it.
    DateTime addDuration(const Duration& duration) const
    {
        if (duration.count() < 0)
            return subtractDuration(-duration);

        const Duration totalDuration = Nanoseconds(m_time.toNanosecondsSinceMidnight()) + duration;
        return DateTime(m_date.addDays(std::chrono::duration_cast<Days>(totalDuration).count()), Time(totalDuration % Days(1)));
    }

    /// Returns a new DateTime object representing this datetime with \p duration subtracted from it.
    DateTime subtractDuration(const Duration& duration) const
    {
        if (duration.count() < 0)
            return addDuration(-duration);

        return DateTime(m_date.subtractDays(std::abs(std::chrono::duration_cast<Days>(Nanoseconds(m_time.toNanosecondsSinceMidnight()) - duration - Days(1)).count())), Time((Days(1) + Nanoseconds(m_time.toNanosecondsSinceMidnight()) - (duration % Days(1))) % Days(1)));
    }

    /// @}

    /**
     * @name Conversion Methods
     * @{
     */

    /// Returns the number of elapsed nanoseconds since "1970-01-01 00:00:00.000 UTC", not counting leap seconds.
    long long toNanosecondsSinceEpoch() const
    {
        return (std::chrono::duration_cast<Nanoseconds>(m_date.toStdDurationSinceEpoch() + m_time.toStdDurationSinceMidnight())).count();
    }

    /// Returns the number of elapsed microseconds since "1970-01-01 00:00:00.000 UTC", not counting leap seconds.
    long long toMicrosecondsSinceEpoch() const
    {
        return (std::chrono::duration_cast<Microseconds>(m_date.toStdDurationSinceEpoch() + m_time.toStdDurationSinceMidnight())).count();
    }

    /// Returns the number of elapsed milliseconds since "1970-01-01 00:00:00.000 UTC", not counting leap seconds.
    long long toMillisecondsSinceEpoch() const
    {
        return (std::chrono::duration_cast<Milliseconds>(m_date.toStdDurationSinceEpoch() + m_time.toStdDurationSinceMidnight())).count();
    }

    /// Returns the number of elapsed seconds since "1970-01-01 00:00:00.000 UTC, not counting leap seconds.
    long long toSecondsSinceEpoch() const
    {
        return (std::chrono::duration_cast<Seconds>(m_date.toStdDurationSinceEpoch() + m_time.toStdDurationSinceMidnight())).count();
    }

    /// Returns the number of elapsed minutes since "1970-01-01 00:00:00.000 UTC, not counting leap seconds.
    long toMinutesSinceEpoch() const
    {
        return (std::chrono::duration_cast<Minutes>(m_date.toStdDurationSinceEpoch() + m_time.toStdDurationSinceMidnight())).count();
    }

    /// Returns the number of elapsed hours since "1970-01-01 00:00:00.000 UTC, not counting leap seconds.
    long toHoursSinceEpoch() const
    {
        return (std::chrono::duration_cast<Hours>(m_date.toStdDurationSinceEpoch() + m_time.toStdDurationSinceMidnight())).count();
    }

    /// Returns the number of elapsed days since "1970-01-01 00:00:00.000 UTC", not counting leap seconds.
    long toDaysSinceEpoch() const
    {
        return m_date.toDaysSinceEpoch();
    }

    /// Returns a **std::chrono::microseconds** duration since "1970-01-01 00:00:00.000 UTC", not counting leap seconds.
    Microseconds toStdDurationSinceEpoch() const
    {
        return std::chrono::duration_cast<Microseconds>(m_date.toStdDurationSinceEpoch() + m_time.toStdDurationSinceMidnight());
    }

    /// Returns a **std::chrono::system_clock::time_point** representation of this datetime.
    std::chrono::system_clock::time_point toStdTimePoint() const
    {
        return std::chrono::system_clock::time_point(toStdDurationSinceEpoch());
    }

    /// Returns a **std::tm** representation of this datetime.
    std::tm toBrokenStdTime() const
    {
        int y, m, d;
        getYearMonthDay(&y, &m, &d);
        std::tm cTime = { 0 };
        cTime.tm_year = y - 1970;
        cTime.tm_year = m;
        cTime.tm_year = d;
        cTime.tm_hour = hour();
        cTime.tm_min = minute();
        cTime.tm_sec = second();

        return cTime;
    }

    /// Returns a **std::time_t** representation of this datetime.
    std::time_t toScalarStdTime() const
    {
        return std::time_t(toSecondsSinceEpoch());
    }

    /**
     * Returns the corresponding Julian Day Number (JDN) of this datetime as a double, where the integral part represents the day count and the fractional part represents the time since midday Universal Time (UT).
     * The JDN is the consecutive numbering of days and fractions since noon Universal Time (UT) on 1 January  4713 BCE in the proleptic Julian calendar, which occurs on 24 November 4714 BCE in the proleptic Gregorian calendar.
     * That is, the JDN 0 corresponds to "12:00:00 UTC, 24 November 4714 BCE", the JDN 1.5 represents "00:00:00 UTC, 26 November 4714 BCE", and the JDN of today at "00:09:35 UTC, 31 December 2017 CE" is 2458118.506655093.
     * **Note** that the date to be converted is considered Gregorian.
     * Also, the current Gregorian rules are extended backwards and forwards.
     * There is no year 0. The first year before the common era (i.e., year 1 BCE) is year -1, year -2 is year 2 BCE, and so on.
     */
    double toJulianDay() const
    {
        return m_date.toDaysSinceEpoch() + 2440587.5 + static_cast<double>(m_time.toNanosecondsSinceMidnight()) / static_cast<double>(Nanoseconds(Days(1)).count());
    }

    /**
     * Returns the datetime as a string formatted according to the format string \p format.
     * The format string may contain the following patterns:
     *
     *    Pattern     | Meaning
     *    ----------- | -----------------------------------------------------
     *    #           | era of year as a positive sign(+) or negative sign(-)
     *    E           | era of year as CE or BCE
     *    y           | year as one digit or more (1, 9999+)
     *    yy          | year of era as two digits (00, 99)
     *    yyyy        | year as four digits (0000, 9999)
     *    M           | month of year as one digit or more (1, 12)
     *    MM          | month of year as two digits (01, 12)
     *    MMM         | month of year as short name (e.g. "Feb")
     *    MMMM        | month of year as long name (e.g. "February")
     *    d           | day of month as one digit or more (1, 31)
     *    dd          | day of month as two digits (00, 31)
     *    ddd         | day of week as short name (e.g. "Fri")
     *    dddd        | day of week as long name (e.g. "Friday")
     *    h           | one-digit hour (0, 23)
     *    hh          | two-digit hour (00, 23)
     *    H           | one-digit hour (1, 12)
     *    HH          | two-digit hour (01, 12)
     *    m           | one-digit minute (0, 59)
     *    mm          | two-digit minute (00, 59)
     *    s           | one-digit second (0, 59)
     *    ss          | two-digit second (00, 59)
     *    f           | one-digit subsecond (0, 9)
     *    ff          | two-digit subsecond (00, 99)
     *    fff         | three-digit subsecond (000, 999)
     *    ffff        | four-digit subsecond (0000, 9999)
     *    fffff       | five-digit subsecond (00000, 99999)
     *    ffffff      | six-digit subsecond (000000, 999999)
     *    fffffff     | seven-digit subsecond (0000000, 9999999)
     *    ffffffff    | eight-digit subsecond (00000000, 99999999)
     *    fffffffff   | nine-digit subsecond (000000000, 999999999)
     *    a           | before/after noon indicator(i.e. am or pm)
     *    A           | before/after noon indicator(i.e. AM or PM)
     *
     * Any character in the format string not listed above will be inserted as-is into the output string.
     * If the datetime is invalid, an empty string will be returned.
     * @see dayOfWeekName(), monthName()
     */
    std::string toString(const std::string& format = "yyyy-MM-dd hh:mm:ss") const
    {
        if (!isValid() || format.empty())
            return std::string();
        std::stringstream output;
        for (size_t pos = 0; pos < format.size(); ++pos) {
            const auto count = internal::countIdenticalCharsFrom(pos, format);
            output << (internal::isPattern(format.at(pos), count) ? stringify(format.at(pos), count) : format.substr(pos, count));
            pos += count - 1;
        }
        return output.str();
    }

    /// @}

    /**
     * Returns a DateTime object set to the current datetime obtained from the system clock.
     * **Note** that the returned datetime is not the current local datetime but rather the current system datetime in Coordinated Universal Time (UTC).
     */
    static DateTime current()
    {
        return DateTime(std::chrono::system_clock::now());
    }

    /// Returns a DateTime object set to the epoch "1970-1-1T00:00:00".
    static DateTime epoch()
    {
        return DateTime(Date::epoch(), Time::midnight());
    }

    /**
     * Returns a DateTime object from the string \p datetime formatted according to the format string \p format.
     * The format patterns are the same patterns used in the method toString(). @see toString()
     */
    static DateTime fromString(const std::string& datetime, const std::string& format = "yyyy-MM-dd hh:mm:ss")
    {
        int sign = 1;
        int y = 0;
        int M = 1;
        int d = 1;
        int h = 0;
        int m = 0;
        int s = 0;
        int f = 0;
        size_t dtPos = 0;
        for (size_t fmtPos = 0; fmtPos < format.size(); ++fmtPos) {
            const auto count = internal::countIdenticalCharsFrom(fmtPos, format);
            if (internal::isPattern(format[fmtPos])) {
                if (!internal::isPattern(format[fmtPos], count)) {
                    return DateTime();
                }
            }
            if (format[fmtPos] == 'y') {
                y = parse(format[fmtPos], count, datetime, dtPos);
            } else if (format[fmtPos] == '#' || format[fmtPos] == 'E') {
                sign = parse(format[fmtPos], count, datetime, dtPos);
            } else if (format[fmtPos] == 'M') {
                M = parse(format[fmtPos], count, datetime, dtPos);
            } else if (format[fmtPos] == 'd') {
                if (count == 3) {
                    parse(format[fmtPos], count, datetime, dtPos);
                } else {
                    d = parse(format[fmtPos], count, datetime, dtPos);
                }
            } else if (format[fmtPos] == 'h' || format[fmtPos] == 'H') {
                h = parse(format[fmtPos], count, datetime, dtPos);
            } else if (format[fmtPos] == 'a' || format[fmtPos] == 'A') {
                int clock = parse(format[fmtPos], count, datetime, dtPos);
                h += clock == -12 && h >= 12 || clock == 12 && h < 12 ? clock : 0;
            } else if (format[fmtPos] == 'm') {
                m = parse(format[fmtPos], count, datetime, dtPos);
            } else if (format[fmtPos] == 's') {
                s = parse(format[fmtPos], count, datetime, dtPos);
            } else if (format[fmtPos] == 'f') {
                f = parse(format[fmtPos], count, datetime, dtPos) * std::pow(10, 9 - count);
            } else {
                dtPos += count;
            }
            if (dtPos == std::string::npos) {
                return DateTime();
            }
            fmtPos += count - 1;
        }
        return DateTime(Date(sign * y, M, d), Time(h, m, s, Nanoseconds(f)));
    }

    /// Returns a DateTime object corresponding to the Julian day \p julianDay. See toJulianDay() for information about Julian Days.
    static DateTime fromJulianDay(double julianDay)
    {
        const long integer = static_cast<long>(julianDay);
        const double fraction = julianDay - integer;
        const long millisecondCount = static_cast<long>(static_cast<double>(Milliseconds(Days(1)).count()) * fraction);
        return DateTime(Date(Days(integer - 2440587))).subtractHours(12).addDuration(Milliseconds(millisecondCount));
    }

    /**
     * @name Calculation Methods
     * The following methods return the time difference between two datetimes: \p from and \p to. If \p to is earlier than (smaller than) \p from, then the difference is negative.
     * @{
     */

    /// Returns the number of nanoseconds between \p from and \p to.
    static long long nanosecondsBetween(const DateTime& from, const DateTime& to)
    {
        return std::abs(std::chrono::duration_cast<Nanoseconds>((from.m_date.toStdDurationSinceEpoch() + from.m_time.toStdDurationSinceMidnight()) - (to.m_date.toStdDurationSinceEpoch() + to.m_time.toStdDurationSinceMidnight())).count());
    }

    /// Returns the number of microseconds between \p from and \p to.
    static long long microsecondsBetween(const DateTime& from, const DateTime& to)
    {
        return std::abs(std::chrono::duration_cast<Microseconds>(from.toStdDurationSinceEpoch() - to.toStdDurationSinceEpoch()).count());
    }

    /// Returns the number of milliseconds between \p from and \p to.
    static long long millisecondsBetween(const DateTime& from, const DateTime& to)
    {
        return std::abs(std::chrono::duration_cast<Milliseconds>(from.toStdDurationSinceEpoch() - to.toStdDurationSinceEpoch()).count());
    }

    /// Returns the number of seconds between \p from and \p to.
    static long long secondsBetween(const DateTime& from, const DateTime& to)
    {
        return std::abs(std::chrono::duration_cast<Seconds>(from.toStdDurationSinceEpoch() - to.toStdDurationSinceEpoch()).count());
    }

    /// Returns the number of minutes between \p from and \p to.
    static long minutesBetween(const DateTime& from, const DateTime& to)
    {
        return std::abs(std::chrono::duration_cast<Minutes>(from.toStdDurationSinceEpoch() - to.toStdDurationSinceEpoch()).count());
    }

    /// Returns the number of hours between \p from and \p to.
    static long hoursBetween(const DateTime& from, const DateTime& to)
    {
        return std::abs(std::chrono::duration_cast<Hours>(from.toStdDurationSinceEpoch() - to.toStdDurationSinceEpoch()).count());
    }

    /// Returns the number of days between \p from and \p to.
    static long daysBetween(const DateTime& from, const DateTime& to)
    {
        return std::abs(std::chrono::duration_cast<Days>(from.toStdDurationSinceEpoch() - to.toStdDurationSinceEpoch()).count());
    }

    /// Returns the number of weeks between \p from and \p to.
    static long weeksBetween(const DateTime& from, const DateTime& to)
    {
        return std::abs(std::chrono::duration_cast<Weeks>(from.toStdDurationSinceEpoch() - to.toStdDurationSinceEpoch()).count());
    }

    /// @}

private:
    std::string stringify(char flag, size_t count) const
    {
        std::stringstream output;
        output << std::setfill('0') << std::setw(count);
        if (flag == '#') {
            output << (year() < 0 ? "-" : "+");
        } else if (flag == 'E') {
            output << (year() < 0 ? "BCE" : "CE");
        } else if (flag == 'y') {
            const int y = std::abs(year());
            if (count == 1) {
                output << y;
            } else if (count == 2) {
                output << y - (y / 100 * 100);
            } else if (count == 4) {
                output << y - (y / 10000 * 10000);
            }
        } else if (flag == 'M') {
            if (count == 1 || count == 2) {
                output << month();
            } else if (count == 3) {
                output << internal::getShortMonthName(month());
            } else if (count == 4) {
                output << internal::getLongMonthName(month());
            }
        } else if (flag == 'd') {
            if (count == 1 || count == 2) {
                output << day();
            } else if (count == 3) {
                output << internal::getShortWeekdayName(dayOfWeek());
            } else if (count == 4) {
                output << internal::getLongWeekdayName(dayOfWeek());
            }
        } else if (flag == 'h') {
            output << hour();
        } else if (flag == 'H') {
            int h = hour();
            output << (h == 0 ? 12 : h > 12 ? h - 12
                                            : h);
        } else if (flag == 'm') {
            output << minute();
        } else if (flag == 's') {
            output << second();
        } else if (flag == 'f') {
            output << nanosecond() / static_cast<long>(std::pow(10, 9 - count));
        } else if (flag == 'a') {
            output << (hour() < 12 ? "am" : "pm");
        } else if (flag == 'A') {
            output << (hour() < 12 ? "AM" : "PM");
        }
        return output.str();
    }

    static int parse(char flag, size_t count, std::string input, size_t& pos)
    {
        if (flag == 'y' || (flag == 'M' || flag == 'd' || flag == 'h' || flag == 'H' || flag == 'm' || flag == 's') && count <= 2 || flag == 'f') {
            if (pos < input.size() && std::isdigit(input[pos])) {
                return internal::readIntAndAdvancePos(input, pos, flag == 'f' ? count : (flag == 'y' && count == 1 || count == 4 ? 4 : 2));
            }
        } else if (flag == '#') {
            if (pos < input.size() && (input[pos] == '-' || input[pos] == '+')) {
                return (input[pos++] == '-' ? -1 : 1);
            }
        } else if (flag == 'E') {
            if (pos + 2 < input.size() && input.substr(pos, 3) == "BCE") {
                pos += 3;
                return -1;
            } else if (pos + 1 < input.size() && input.substr(pos, 2) == "CE") {
                pos += 2;
                return 1;
            }
        } else if ((flag == 'M' || flag == 'd') && count <= 4 && pos + 2 < input.size()) {
            const auto& line = count == 3 ? input.substr(pos, 3) : input.substr(pos);
            if (flag == 'M') {
                const auto& monthNameArray = count == 3 ? internal::getShortMonthNameArray() : internal::getLongMonthNameArray();
                size_t index = internal::search(monthNameArray, line);
                if (index < monthNameArray.size()) {
                    pos += monthNameArray[index].size();
                    return index + 1;
                }
            } else {
                const auto& weekdayNameArray = count == 3 ? internal::getShortWeekdayNameArray() : internal::getLongWeekdayNameArray();
                size_t index = internal::search(weekdayNameArray, line);
                if (index < weekdayNameArray.size()) {
                    pos += weekdayNameArray[index].size();
                    return index + 1;
                }
            }
        } else if ((flag == 'a' || flag == 'A') && pos + 1 < input.size()) {
            if (flag == 'a' && input.substr(pos, 2) == "am" || input.substr(pos, 2) == "AM") {
                return -12;
            } else if (flag == 'a' && input.substr(pos, 2) == "pm" || input.substr(pos, 2) == "PM") {
                return 12;
            }
        }
        pos = std::string::npos;
        return std::numeric_limits<int>::min();
    }

    Date m_date;
    Time m_time;
};

/**
 * @relates DateTime
 * @name Input/Output Operators
 * @{
 */

/// Writes \p dt to stream \p os in ISO-8601 date and time format "yyyy-MM-ddThh:mm:ss.fff". See toString() for information about the format patterns.
std::ostream& operator<<(std::ostream& os, const DateTime& dt)
{
    os << dt.toString("yyyy-MM-ddThh:mm:ss.fff");

    return os;
}

/// Reads a datetime in ISO-8601 date and time format "yyyy-MM-ddThh:mm:ss.fff" from stream \p is and stores it in \p dt. See toString() for information about the format patterns.
std::istream& operator>>(std::istream& is, DateTime& dt)
{
    const int DateTimeFormatWidth = 23;
    char result[DateTimeFormatWidth];
    is.read(result, DateTimeFormatWidth);
    dt = DateTime::fromString(std::string(result, DateTimeFormatWidth), "yyyy-MM-ddThh:mm:ss.fff");

    return is;
}

/// @}

}

#endif // XCLOX_DATE_TIME_HPP
