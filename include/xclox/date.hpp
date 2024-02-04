/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_DATE_HPP
#define XCLOX_DATE_HPP

#include "internal.hpp"

namespace xclox {

namespace internal {

    inline Days ymdToDays(int year, int month, int day)
    {
        // Math from http://howardhinnant.github.io/date_algorithms.html
        auto const y = static_cast<int>(year) - (month <= 2) + (year < 1); // if the year is negative; i.e. before common era, add one year.
        auto const m = static_cast<unsigned>(month);
        auto const d = static_cast<unsigned>(day);
        auto const era = (y >= 0 ? y : y - 399) / 400;
        auto const yoe = static_cast<unsigned>(y - era * 400); // [0, 399]
        auto const doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1; // [0, 365]
        auto const doe = yoe * 365 + yoe / 4 - yoe / 100 + doy; // [0, 146096]

        return Days { era * 146097 + static_cast<long>(doe) - 719468 };
    }

    inline void daysToYmd(Days dys, int* year, int* month, int* day)
    {
        // Math from http://howardhinnant.github.io/date_algorithms.html
        auto const z = dys.count() + 719468;
        auto const era = (z >= 0 ? z : z - 146096) / 146097;
        auto const doe = static_cast<unsigned long>(z - era * 146097); // [0, 146096]
        auto const yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365; // [0, 399]
        auto const y = static_cast<Days::rep>(yoe) + era * 400;
        auto const doy = doe - (365 * yoe + yoe / 4 - yoe / 100); // [0, 365]
        auto const mp = (5 * doy + 2) / 153; // [0, 11]
        auto const m = mp < 10 ? mp + 3 : mp - 9; // [1, 12]
        auto const d = doy - (153 * mp + 2) / 5 + 1; // [1, 31]
        if (year) {
            *year = y + (m <= 2);
            *year = *year - (*year < 1); // if the year is negative; i.e. before common era, subtract one year.;
        }
        if (month)
            *month = static_cast<int>(m);
        if (day)
            *day = static_cast<int>(d);
    }

} // namespace internal

/**
 * @class Date
 *
 * Date is an immutable class representing a date without a time zone in the ISO-8601 calendar system, such as "2017-12-15".
 *
 * The ISO-8601 calendar system is the modern civil calendar system used today in most of the world. It is equivalent to the proleptic Gregorian calendar system.
 *
 * Date describes the date as a triple (year, month, day). There is no year 0, and dates with a year 0 are considered invalid.
 * Negative years indicate years before the common era (BCE). So, year -1 represents year 1 BCE, year -2 represents year 2 BCE, and so on.
 *
 * A default-constructed Date object is invalid (calling isValid() on it returns false).
 * Date objects can be created by explicitly giving the number of years, months and days.
 * Also, it can be created from the number of days since the epoch "1970-01-01".
 *
 * The year, month, and day of the date can be accessed through the methods year(), month(), and day(), respectively.
 * Other date fields, such as day-of-year, day-of-week, and week-of-year, can also be accessed through dayOfYear(), dayOfWeek(), and weekOfYear(), respectively.
 *
 * Date defines two durations: #Days and #Weeks, and two enumerations: #Weekday and #Month.
 *
 * Date provides methods for manipulating dates. Years, months, and days can be added to a date (through addYears(), addMonths(), and addDays(), respectively) or subtracted from it (through subtractYears(), subtractMonths(), and subtractDays(), respectively).
 *
 * It also provides operators for comparing dates. Date A is considered earlier than Date B if A is smaller than B, and so on.
 * The methods daysBetween() and weeksBetween() returns how many days and weeks are between two dates, respectively.
 *
 * Also, Date provides the methods fromJulianDay() and toJulianDay() to convert a date from and to a Julian Day Number (JDN).
 * The toString() method can be used to get a textual representation of the date formatted according to a given format string.
 *
 * @see The unit tests in @ref date.h for further details.
 */
class Date {
public:
    /**
     * @name Durations
     * @{
     */

    using Days = internal::Days; ///< Day duration.
    using Weeks = std::chrono::duration<long, std::ratio_multiply<std::ratio<7>, Days::period>>; ///< Week duration.

    /// @}

    /**
     * @name Enumerations
     * @{
     */

    /**
     * @enum Weekday
     * Type of weekday.
     */
    enum class Weekday {
        Monday = 1,
        Tuesday = 2,
        Wednesday = 3,
        Thursday = 4,
        Friday = 5,
        Saturday = 6,
        Sunday = 7
    };

    /**
     * @enum Month
     * Type of month.
     */
    enum class Month {
        January = 1,
        February = 2,
        March = 3,
        April = 4,
        May = 5,
        June = 6,
        July = 7,
        August = 8,
        September = 9,
        October = 10,
        November = 11,
        December = 12
    };

    /// @}

    /**
     * @name Constructors and Destructors
     * @{
     */

    /// Constructs an invalid Date object with every field is set to zero. @see isValid()
    Date()
        : m_year(0)
        , m_month(0)
        , m_day(0)
    {
    }

    /// Copy-constructs a Date object from \p other.
    Date(const Date& other) = default;

    /// Move-constructs a Date object from \p other.
    Date(Date&& other) = default;

    /// Constructs a Date object from \p days elapsed since the epoch "1970-01-01".
    explicit Date(const Days& days)
    {
        internal::daysToYmd(days, &m_year, &m_month, &m_day);
    }

    /// Constructs a Date object from the given \p year, \p month and \p day.
    explicit Date(int year, int month, int day)
        : m_year(year)
        , m_month(month)
        , m_day(day)
    {
    }

    /// Default destructor.
    ~Date() = default;

    /// @}

    /**
     * @name Assignment Operators
     * @{
     */

    /// Copy assignment operator.
    Date& operator=(const Date& other) = default;

    /// Move assignment operator.
    Date& operator=(Date&& other) = default;

    /// @}

    /**
     * @name Comparison Operators
     * @{
     */

    /// Returns whether this date is earlier than \p other.
    bool operator<(const Date& other) const
    {
        return this->year() < other.year() || (this->year() == other.year() && this->month() < other.month()) || (this->year() == other.year() && this->month() == other.month() && this->day() < other.day());
    }

    /// Returns whether this date is earlier than \p other or equal to it.
    bool operator<=(const Date& other) const
    {
        return this->operator<(other) || this->operator==(other);
    }

    /// Returns whether this date is later than \p other.
    bool operator>(const Date& other) const
    {
        return this->year() > other.year() || (this->year() == other.year() && this->month() > other.month()) || (this->year() == other.year() && this->month() == other.month() && this->day() > other.day());
    }

    /// Returns whether this date is later than \p other or equal to it.
    bool operator>=(const Date& other) const
    {
        return this->operator>(other) || this->operator==(other);
    }

    /// Returns whether this date is equal to \p other.
    bool operator==(const Date& other) const
    {
        return this->year() == other.year() && this->month() == other.month() && this->day() == other.day();
    }

    /// Returns whether this date is different from \p other.
    bool operator!=(const Date& other) const
    {
        return this->year() != other.year() || this->month() != other.month() || this->day() != other.day();
    }

    /// @}

    /**
     * @name Querying Methods
     * @{
     */

    /**
     * Returns whether this date object represents a valid date.
     * A valid date contains a non-zero year, a valid month (between 1 and 12), and a valid day of month (between 1 and 31).
     *
     * @code
     *    Date d; // same as Date(0, 0, 0)
     *    bool state1 = d.isValid(); // returns false
     *    bool state2 = Date(1999, -1, 1).isValid(); // returns false
     *    bool state3 = Date(1999, 1, 0).isValid(); // returns false
     *    d = Date(1970, 1, 1);
     *    bool state4 = d.isValid(); // returns true
     * @endcode
     */
    bool isValid() const
    {
        return m_year != 0 && (m_month > 0 && m_month < 13) && (m_day > 0 && m_day < (daysInMonthOfYear(m_year, m_month) + 1));
    }

    /// Set the year, month, and day of this date in the parameters \p year, \p month, and \p day, respectively.
    void getYearMonthDay(int* year, int* month, int* day) const
    {
        if (year)
            *year = m_year;
        if (month)
            *month = m_month;
        if (day)
            *day = m_day;
    }

    /// Returns the day of the month of this date as a number between 1 and 31.
    int day() const
    {
        return m_day;
    }

    /// Returns the month of the year of this date as a number between 1 and 12, which corresponds to the enumeration #Month.
    int month() const
    {
        return m_month;
    }

    /**
     * Returns the year of this date as a number.
     * There is no year 0. Negative numbers indicate years before 1 BCE; for example, year -1 is year 1 BCE, and so on.
     */
    int year() const
    {
        return m_year;
    }

    /// Returns the weekday of this date as a number between 1 and 7, which corresponds to the enumeration #Weekday.
    int dayOfWeek() const
    {
        return (toDaysSinceEpoch() % 7 + 3) % 7 + 1;
    }

    /// Returns the day of year of this date as a number between 1 and 365 (1 to 366 on leap years).
    int dayOfYear() const
    {
        return toDaysSinceEpoch() - internal::ymdToDays(year(), 1, 1).count() + 1;
    }

    /// Returns the number of days in the month of this date. It ranges between 28 and 31.
    int daysInMonth() const
    {
        return daysInMonthOfYear(year(), month());
    }

    /// Returns the number of days in the year of this date. It is either 365 or 366.
    int daysInYear() const
    {
        return (isLeapYear() ? 366 : 365);
    }

    /// Returns whether the year of this date is a leap year. @see isLeapYear(int)
    bool isLeapYear() const
    {
        return isLeapYear(year());
    }

    /**
     * Returns the week of the year of this date, and optionally stores the year in weekYear.
     * The week of the year ranges between 1 and 53. Most years have 52 weeks, but some have 53.
     * According to ISO-8601, weeks start on Monday, and the first Thursday of a year is always in week 1 of that year.
     * weekYear is not always the same as year(). For example:
     *   - The date of "1 January 2000" falls in week 52 of the year 1999.
     *   - The date of "31 December 2002" falls in week 1 of the year 2003.
     *   - The date of "1 January 2010" falls in week 53 of the year 2009.
     */
    int weekOfYear(int* weekYear = nullptr) const
    {
        static auto getFirstWeekDate = [](int year) {
            Date d(year, 1, 1);
            return d.addDays((11 - d.dayOfWeek()) % 7 - 3);
        };
        int y = year();
        Date currentDate = *this;
        Date firstWeekDate = getFirstWeekDate(y);
        if (currentDate < firstWeekDate) {
            // If the given date is earlier than the start of the first week of the year that contains it, then the date belongs to the last week of the previous year.
            --y;
            firstWeekDate = getFirstWeekDate(y);
        } else {
            Date nextYearFirstWeekDate = getFirstWeekDate(y + 1);
            if (currentDate >= nextYearFirstWeekDate) {
                // If the given date is on or after the start of the first week of the next year, then the date belongs to the first week of the next year.
                ++y;
                firstWeekDate = nextYearFirstWeekDate;
            }
        }
        int week = daysBetween(firstWeekDate, currentDate) / 7 + 1;
        if (weekYear)
            *weekYear = y;
        return week;
    }

    /**
     * Returns the name of the weekday of this date.
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
        return shortName ? internal::getShortWeekdayName(dayOfWeek()) : internal::getLongWeekdayName(dayOfWeek());
    }

    /**
     * Returns the name of the month of this date.
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
        return shortName ? internal::getShortMonthName(month()) : internal::getLongMonthName(month());
    }

    /// @}

    /**
     * @name Addition/Subtraction Methods
     * @{
     */

    /// Returns the result of adding \p days to this date as a new Date object.
    Date addDays(int days) const
    {
        int y, m, d;
        internal::daysToYmd(Days(internal::ymdToDays(m_year, m_month, m_day) + Days(days)), &y, &m, &d);
        return Date(y, m, d);
    }

    /// Returns the result of subtracting \p days from this date as a new Date object.
    Date subtractDays(int days) const
    {
        int y, m, d;
        internal::daysToYmd(Days(internal::ymdToDays(m_year, m_month, m_day) - Days(days)), &y, &m, &d);
        return Date(y, m, d);
    }

    /**
     * Returns the result of adding \p months to this date as a new Date object.
     * When the ending day/month combination does not exist in the resulting month/year, it returns the latest valid date. For example:
     *
     * @code
     *    Date d = Date(2013, 1, 31).addMonths(1); //d = Date(2013, 2, 28)
     * @endcode
     */
    Date addMonths(int months) const
    {
        if (months < 0)
            return subtractMonths(-months);

        const int totalMonths = m_month + months - 1;
        const int newYear = m_year + (totalMonths / 12);
        const int newMonth = (totalMonths % 12) + 1;
        const int newDaysInMonth = daysInMonthOfYear(newYear, newMonth);
        const int newDays = newDaysInMonth < m_day ? newDaysInMonth : m_day;

        return Date(newYear, newMonth, newDays);
    }

    /**
     * Returns the result of subtracting \p months from this date as a new Date object.
     * When the ending day/month combination does not exist in the resulting month/year, it returns the latest valid date. For example:
     *
     * @code
     *    Date d = Date(2012, 3, 31).subtractMonths(1); //d = Date(2012, 2, 29)
     * @endcode
     */
    Date subtractMonths(int months) const
    {
        if (months < 0)
            return addMonths(-months);

        const int newYear = m_year - (std::abs(m_month - months - 12) / 12);
        const int newMonth = ((11 + m_month - (months % 12)) % 12) + 1;
        const int newDaysInMonth = daysInMonthOfYear(newYear, newMonth);
        const int newDays = newDaysInMonth < m_day ? newDaysInMonth : m_day;

        return Date(newYear, newMonth, newDays);
    }

    /// Returns the result of adding \p years to this date as a new Date object.
    Date addYears(int years) const
    {
        const int newYear = m_year + years;
        return Date(newYear > 0 ? newYear : newYear - 1, m_month, m_day);
    }

    /// Returns the result of subtracting \p years from this date as a new Date object.
    Date subtractYears(int years) const
    {
        const int newYear = m_year - years;
        return Date(newYear > 0 ? newYear : newYear - 1, m_month, m_day);
    }

    /// @}

    /**
     * @name Conversion Methods
     * @{
     */

    /// Returns the number of elapsed days since the epoch "1970-01-01".
    long toDaysSinceEpoch() const
    {
        return internal::ymdToDays(year(), month(), day()).count();
    }

    /// Returns the elapsed time since the epoch "1970-01-01" as a #Days duration.
    Days toStdDurationSinceEpoch() const
    {
        return Days(toDaysSinceEpoch());
    }

    /**
     * Returns the corresponding Julian Day Number (JDN) of this date.
     * JDN is the consecutive numbering of days since the beginning of the Julian Period on 1 January 4713 BCE in the proleptic Julian calendar, which occurs on 24 November 4714 BCE in the proleptic Gregorian calender.
     * Note that the date to be converted is considered Gregorian. Also, the current Gregorian rules are extended backwards and forwards.
     * There is no year 0. The first year before the common era (i.e., year 1 BCE) is year -1, year -2 is year 2 BCE, and so on.
     */
    long toJulianDay() const
    {
        return toDaysSinceEpoch() + 2440588;
    }

    /**
     * Returns this date as a string, formatted according to the format string \p format.
     * The format string may contain the following patterns:
     *
     *    Pattern | Meaning
     *    ------- | -----------------------------------------------------
     *    #       | era of year as a positive sign(+) or negative sign(-)
     *    E       | era of year as CE or BCE
     *    y       | year as one digit or more (1, 9999)
     *    yy      | year of era as two digits (00, 99)
     *    yyyy    | year as four digits (0000, 9999)
     *    M       | month of year as one digit or more (1, 12)
     *    MM      | month of year as two digits (01, 12)
     *    MMM     | month of year as short name (e.g. "Feb")
     *    MMMM    | month of year as long name (e.g. "February")
     *    d       | day of month as one digit or more (1, 31)
     *    dd      | day of month as two digits (00, 31)
     *    ddd     | day of week as short name (e.g. "Fri")
     *    dddd    | day of week as long name (e.g. "Friday")
     *
     * Any character in the format string not listed above will be inserted as-is into the output string.
     * If this date is invalid, an empty string will be returned.
     * @see dayOfWeekName(), monthName()
     */
    std::string toString(const std::string& format) const
    {
        if (!isValid())
            return std::string();

        std::stringstream output;

        for (size_t pos = 0; pos < format.size(); ++pos) {
            int y, m, d;
            getYearMonthDay(&y, &m, &d);
            y = std::abs(y);

            char currChar = format[pos];
            const int charCount = internal::countIdenticalCharsFrom(pos, format);

            if (currChar == '#') {
                output << (year() < 0 ? "-" : "+");
            } else if (currChar == 'y') {
                if (charCount == 1) {
                    output << y;
                } else if (charCount == 2) {
                    y = y - ((y / 100) * 100);
                    output << std::setfill('0') << std::setw(2) << y;
                } else if (charCount == 4) {
                    output << std::setfill('0') << std::setw(4) << y;
                }
                pos += charCount - 1; // skip all identical characters except the last.
            } else if (currChar == 'M') {
                if (charCount == 1) {
                    output << m;
                } else if (charCount == 2) {
                    output << std::setfill('0') << std::setw(2) << m;
                } else if (charCount == 3) {
                    output << monthName(true);
                } else if (charCount == 4) {
                    output << monthName(false);
                }
                pos += charCount - 1; // skip all identical characters except the last.
            } else if (currChar == 'd') {
                if (charCount == 1) {
                    output << d;
                } else if (charCount == 2) {
                    output << std::setfill('0') << std::setw(2) << d;
                } else if (charCount == 3) {
                    output << dayOfWeekName(true);
                } else if (charCount == 4) {
                    output << dayOfWeekName(false);
                }
                pos += charCount - 1; // skip all identical characters except the last.
            } else if (currChar == 'E') {
                output << (year() < 0 ? "BCE" : "CE");
            } else {
                output << currChar;
            }
        }

        return output.str();
    }

    /// @}

    /**
     * Returns a Date object set to the current date obtained from the system clock.
     * **Note** that the returned date is not the current local date but rather the current system date in Coordinated Universal Time (UTC).
     */
    static Date current()
    {
        return Date(std::chrono::duration_cast<Days>(std::chrono::system_clock::now().time_since_epoch()));
    }

    /// Returns a Date object set to the epoch "1970-01-01".
    static Date epoch()
    {
        return Date(Days(0));
    }

    /**
     * Returns a Date object from the date string \p date according to the format string \p format.
     * The format patterns are the same patterns used in the method toString(). @see toString()
     */
    static Date fromString(const std::string& date, const std::string& format)
    {
        int _year = 1, _month = 1, _day = 1;

        for (size_t fmtPos = 0, datPos = 0; fmtPos < format.size() && datPos < date.size(); ++fmtPos) {
            const size_t charCount = static_cast<size_t>(internal::countIdenticalCharsFrom(fmtPos, format));

            if (format[fmtPos] == '#') {
                if (date[datPos] == '+') {
                    _year = 1;
                    ++datPos;
                } else if (date[datPos] == '-') {
                    _year = -1;
                    ++datPos;
                }
            } else if (format[fmtPos] == 'y') {
                if (charCount == 1) {
                    _year = _year * internal::readIntAndAdvancePos(date, datPos, 4);
                } else if (charCount == 2) {
                    _year = _year * std::stoi(date.substr(datPos, charCount));
                    _year += 2000;
                    datPos += charCount;
                } else if (charCount == 4) {
                    _year = _year * std::stoi(date.substr(datPos, charCount));
                    datPos += charCount;
                }
                fmtPos += charCount - 1; // skip all identical characters except the last.
            } else if (format[fmtPos] == 'E') {
                if (date.substr(datPos, 2) == "CE") {
                    _year = std::abs(_year);
                    datPos += 2;
                } else if (date.substr(datPos, 3) == "BCE") {
                    _year = -std::abs(_year);
                    datPos += 3;
                }
            } else if (format[fmtPos] == 'M') {
                if (charCount == 1) {
                    _month = internal::readIntAndAdvancePos(date, datPos, 4);
                } else if (charCount == 2) {
                    _month = std::stoi(date.substr(datPos, charCount));
                    datPos += charCount;
                } else if (charCount == 3) {
                    _month = internal::getShortMonthNumber(date.substr(datPos, charCount));
                    datPos += charCount;
                } else if (charCount == 4) {
                    size_t newPos = datPos;
                    while (newPos < date.size() && std::isalpha(date[newPos]))
                        ++newPos;
                    _month = internal::getLongMonthNumber(date.substr(datPos, newPos - datPos));
                    datPos = newPos;
                }
                fmtPos += charCount - 1; // skip all identical characters except the last.
            } else if (format[fmtPos] == 'd') {
                if (charCount == 1) {
                    _day = internal::readIntAndAdvancePos(date, datPos, 2);
                } else if (charCount == 2) {
                    _day = std::stoi(date.substr(datPos, charCount));
                    datPos += charCount;
                } else if (charCount == 3) {
                    // lets the format string and the date string be in sync.
                    datPos += charCount;
                } else if (charCount == 4) {
                    while (datPos < date.size() && std::isalpha(date[datPos]))
                        ++datPos;
                }
                fmtPos += charCount - 1; // skip all identical characters except the last.
            } else {
                // not a pattern, skip it in the date string.
                ++datPos;
            }
        }

        return Date(_year, _month, _day);
    }

    /// Returns a Date object corresponding to the Julian day \p julianDay. @see toJulianDay()
    static Date fromJulianDay(long julianDay)
    {
        return Date(Days(julianDay - 2440588));
    }

    /**
     * @name Calculation Methods
     * The following methods return the time difference between two dates; \p from and \p to.
     * If \p to is earlier than (smaller than) \p from, then the difference is negative.
     * @{
     */

    /**
     * Returns the number of days between \p from and \p to. For example:
     *
     * @code
     *    int num = Date::daysBetween(Date(1999, 1, 1), Date(1999, 1, 3));  // num = 2
     * @endcode
     */
    static long daysBetween(const Date& from, const Date& to)
    {
        return to.toDaysSinceEpoch() - from.toDaysSinceEpoch();
    }

    /**
     * Returns the number of weeks between \p from and \p to. For example:
     *
     * @code
     *    int num = Date::weeksBetween(Date(1970, 1, 1), Date(1970, 1, 8));  // num = 1
     * @endcode
     */
    static long weeksBetween(const Date& from, const Date& to)
    {
        return daysBetween(from, to) / 7;
    }

    /// @}

    /**
     * Returns whether \p year is a leap year.
     * According to the ISO proleptic calendar system rules, a year is a leap year if it is divisible by four without remainder.
     * However, years divisible by 100 are not leap years, with the exception of years divisible by 400.
     * For example, 1904 is a leap year; it is divisible by 4.
     * 1900 was not a leap year, as it is divisible by 100.
     * However, 2000 was a leap year, as it is divisible by 400.
     */
    static bool isLeapYear(int year)
    {
        // no year 0 in the Gregorian calendar, the first year before the common era is -1 (year 1 BCE). So, -1, -5, -9 etc are leap years.
        if (year < 1)
            ++year;

        return (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0);
    }

    /// Returns the number of days in \p month of \p year. It ranges between 28 and 31.
    static int daysInMonthOfYear(int year, int month)
    {
        switch (month) {

        case 1:
            return 31;
        case 2:
            return (isLeapYear(year) ? 29 : 28);
        case 3:
            return 31;
        case 4:
            return 30;
        case 5:
            return 31;
        case 6:
            return 30;
        case 7:
            return 31;
        case 8:
            return 31;
        case 9:
            return 30;
        case 10:
            return 31;
        case 11:
            return 30;
        case 12:
            return 31;
        }

        return -1;
    }

private:
    int m_year;
    int m_month;
    int m_day;
};

/**
 * @name Input/Output Operators
 * @relates Date
 * @{
 */

/**
 * Writes date \p d to stream  \p os in ISO-8601 date format, "yyyy-MM-dd".
 * See toString() for information about the format patterns.
 */
std::ostream& operator<<(std::ostream& os, const Date& d)
{
    os << d.toString("yyyy-MM-dd");
    return os;
}

/**
 * Reads a date in ISO-8601 date format "yyyy-MM-dd" from stream \p is and stores it in date \p d.
 * See toString() for information about the format patterns.
 */
std::istream& operator>>(std::istream& is, Date& d)
{
    const int DateFormatWidth = 10;
    char result[DateFormatWidth];
    is.read(result, DateFormatWidth);
    d = Date::fromString(std::string(result, DateFormatWidth), "yyyy-MM-dd");

    return is;
}

/// @}

} // namespace xclox

#endif // XCLOX_DATE_HPP
