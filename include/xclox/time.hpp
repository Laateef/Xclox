/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_TIME_HPP
#define XCLOX_TIME_HPP

#include "internal.hpp"

namespace xclox {

/**
 * @class Time
 *
 * Time is an immutable time class representing a time without a time zone in the ISO-8601 calendar system, such as "09:55:02".
 *
 * Time is represented by nanosecond precision. For example, the value "09:55:02.123456789" can be stored in a Time object.
 * Time uses the 24-hour clock format. Internally, it describes the time as the number of nanoseconds since midnight.
 * Time provides methods for comparing times, manipulating a time by adding or subtracting durations, and converting the whole time into minutes, seconds, or any finer duration.
 *
 * A Time object can be created by giving the number of hours, minutes, seconds, milliseconds, etc. explicitly.
 * Also, it can be created from **std::time_t**, **std::tm**, or **std::chrono::system_clock::time_point** objects.
 *
 * A default-constructed time object is invalid (calling isValid() on it returns false).
 * The static method current() can be used to get the current time from the system's clock.
 * Note that the accuracy depends on the accuracy of the underlying operating system; not all systems provide 1-millisecond accuracy.
 *
 * The methods hour(), minute(), second(), millisecond(), microsecond(), and nanosecond() provide access to the number of hours, minutes, seconds, milliseconds, microseconds, and nanoseconds of the time, respectively.
 *
 * Time supports the C++11 chrono library's way of manipulating time, i.e., time constructors and methods can take duration parameters such as **std::chrono::hours(1)**, **std::chrono::minutes(1)**, etc.
 * For convenience, Time defines aliases for these durations, which may be used instead of chrono's ones.
 * Time durations are #Hours, #Minutes, #Seconds, #Milliseconds, #Microseconds, and #Nanoseconds, which correspond to **std:chrono::hours**, **std:chrono::minutes**, **std:chrono::seconds**, **std:chrono::milliseconds**, **std:chrono::microseconds**, and **std:chrono::nanoseconds**, respectively.
 *
 * The methods toHours(), toMinutes(), toSeconds(), toMilliseconds(), toMicroseconds(), and toNanoseconds() return the whole time passed since midnight as a number of hours, minutes, seconds, milliseconds, microseconds, or nanoseconds, respectively.
 *
 * The toString() method can be used to get a textual representation of the time formatted according to a given format string.
 *
 * Time provides the comparison operators to compare two Time objects. Time A is considered earlier than Time B if A is smaller than B, and so on.
 * Time also provides methods to compute the difference between two times in hours, minutes, etc.
 *
 * @see The unit tests in @ref time.h for further details.
 */
class Time {
    using Duration = std::chrono::nanoseconds;
    using Days = internal::Days;

public:
    /**
     * @name Durations
     * @{
     */

    using Nanoseconds = std::chrono::nanoseconds; ///< Nanosecond duration.
    using Microseconds = std::chrono::microseconds; ///< Microsecond duration.
    using Milliseconds = std::chrono::milliseconds; ///< Millisecond duration.
    using Seconds = std::chrono::seconds; ///< Second duration.
    using Minutes = std::chrono::minutes; ///< Minute duration.
    using Hours = std::chrono::hours; ///< Hour duration.

    /// @}

    /**
     * @name Constructors and Destructors
     * @{
     */

    /// Constructs an invalid Time object with every field is set to zero. @see isValid()
    Time()
        : m_duration(Hours(24))
    {
    }

    /// Copy-constructs a Time object from \p other.
    Time(const Time& other) = default;

    /// Move-constructs a Time object from \p other.
    Time(Time&& other) = default;

    /// Constructs a Time object from the standard library **std::time_t** object \p scalarStdTime.
    explicit Time(std::time_t scalarStdTime)
        : m_duration(Seconds(scalarStdTime))
    {
    }

    /// Constructs a Time object from the standard library **std::tm** object \p brokenStdTime.
    explicit Time(const std::tm& brokenStdTime)
        : m_duration(Hours(brokenStdTime.tm_hour) + Minutes(brokenStdTime.tm_min) + Seconds(brokenStdTime.tm_sec))
    {
    }

    /**
     * Constructs a Time object from time \p duration elapsed since midnight ("00:00:00"). For example:
     *
     * @code
     *    Time myTime(Time::Hours(2) + Time::Minutes(55));
     * @endcode
     *
     * @param duration can be **Time::Duration** or **std::chrono::duration**.
     */
    explicit Time(const Duration& duration)
        : m_duration(duration)
    {
    }

    /// Constructs a Time object from the given system time point \p timePoint.
    explicit Time(const std::chrono::system_clock::time_point& timePoint)
        : m_duration(timePoint.time_since_epoch() % Days(1))
    {
    }

    /// Constructs a Time object from the given \p hours, \p minutes and \p seconds.
    explicit Time(int hours, int minutes, int seconds)
        : m_duration(Hours(hours) + Minutes(minutes) + Seconds(seconds))
    {
    }

    /// Constructs a Time object from the given \p hours, \p minutes, \p seconds and \p milliseconds.
    explicit Time(int hours, int minutes, int seconds, int milliseconds)
        : m_duration(Hours(hours) + Minutes(minutes) + Seconds(seconds) + Milliseconds(milliseconds))
    {
    }

    /**
     * Constructs a Time object from the given \p hours, \p minutes, \p seconds, and \p subseconds.
     * The \p subseconds parameter can be any fine duration of Time such as **Time::Microseconds(54)**, or any fine duration of chrono, such as **std::chrono::nanoseconds(435223543)**.
     */
    explicit Time(int hours, int minutes, int seconds, const Duration& subseconds)
        : m_duration(Hours(hours) + Minutes(minutes) + Seconds(seconds) + subseconds)
    {
    }

    /**
     * Constructs a Time object from the given \p hours, \p minutes, \p seconds, and \p subseconds.
     * All parameters can be **Time::Duration** or **std::chrono::duration**. For example:
     *
     * @code
     *    Time myTime(Time::Hours(2), Time::Minutes(55), Time::Seconds(10), Time::Nanoseconds(435223543));
     *    Time sameTime(std::chrono::hours(2), std::chrono::::minutes(55), std::chrono::::seconds(10), std::chrono::::nanoseconds(435223543));
     * @endcode
     */
    explicit Time(Hours hours, Minutes minutes, Seconds seconds, const Duration& subseconds)
        : m_duration(hours + minutes + seconds + subseconds)
    {
    }

    /// Default destructor.
    ~Time() = default;

    /// @}

    /**
     * @name Assignment Operators
     * @{
     */

    /// Copy assignment operator.
    Time& operator=(const Time& other) = default;

    /// Move assignment operator.
    Time& operator=(Time&& other) = default;

    /// @}

    /**
     * @name Comparison Operators
     * The following operators compare this time to \p other.
     * For example, if this time is smaller than \p other, it means that it is earlier than \p other.
     * @{
     */

    /// Returns whether this time is earlier than \p other.
    bool operator<(const Time& other) const
    {
        return this->m_duration < other.m_duration;
    }

    /// Returns whether this time is earlier than \p other or equal to it.
    bool operator<=(const Time& other) const
    {
        return this->m_duration <= other.m_duration;
    }

    /// Returns whether this time is later than \p other.
    bool operator>(const Time& other) const
    {
        return this->m_duration > other.m_duration;
    }

    /// Returns whether this time is later than \p other or equal to it.
    bool operator>=(const Time& other) const
    {
        return this->m_duration >= other.m_duration;
    }

    /// Returns whether this time is equal to \p other.
    bool operator==(const Time& other) const
    {
        return this->m_duration == other.m_duration;
    }

    /// Returns whether this time is different from \p other.
    bool operator!=(const Time& other) const
    {
        return this->m_duration != other.m_duration;
    }

    /// @}

    /**
     * @name Addition/Subtraction Operators
     * @{
     */

    /// Returns the result of adding \p duration to this time as a new Time object.
    Time operator+(const Duration& duration) const
    {
        return Time(this->m_duration + duration);
    }

    /// Returns the result of subtracting \p duration from this time as a new Time object.
    Time operator-(const Duration& duration) const
    {
        return Time(this->m_duration - duration);
    }

    /// Returns the result of subtracting \p other from this time as a **Time::Nanoseconds** duration.
    Nanoseconds operator-(const Time& other) const
    {
        return Nanoseconds(this->m_duration - other.m_duration);
    }

    /// @}

    /**
     * @name Querying Methods
     * @{
     */

    /**
     * Returns whether this time object represents a valid time.
     * A valid time is a fraction of a day; the time to be valid must be between 00:00:00 and 23:59:59. Thus, negative times or times containing 24 hours or more are considered invalid.
     *
     * @code
     *    Time t1; // t1.isValid(); returns false.
     *    Time t2(0, 0, 0); // t2.isValid(); returns true.
     *    Time t3(22, 1, 55); // t3.isValid(); returns true.
     *    Time t4(-1, 0, 0); // t4.isValid(); returns false.
     *    Time t5(Time::Hours(24)); // t5.isValid(); returns false.
     *    Time t6(Time::Hours(-1)); // t6.isValid(); returns false.
     *    Time t7(Time::Seconds(1)); // t7.isValid(); returns true.
     *    Time t8(std::chrono::system_clock::now()); // t8.isValid(); returns true.
     * @endcode
     */
    bool isValid() const
    {
        return m_duration.count() >= 0 && m_duration < Days(1);
    }

    /// Returns the nanosecond of second (0, 999999999).
    long nanosecond() const
    {
        return static_cast<long>(std::chrono::duration_cast<Nanoseconds>(m_duration % Seconds(1)).count());
    }

    /// Returns the microsecond of second (0, 999999).
    long microsecond() const
    {
        return static_cast<long>(std::chrono::duration_cast<Microseconds>(m_duration % Seconds(1)).count());
    }

    /// Returns the millisecond of second (0, 999).
    int millisecond() const
    {
        return static_cast<int>(std::chrono::duration_cast<Milliseconds>(m_duration % Seconds(1)).count());
    }

    /// Returns the second of minute (0, 59).
    int second() const
    {
        return static_cast<int>(std::chrono::duration_cast<Seconds>(m_duration % Minutes(1)).count());
    }

    /// Returns the minute of hour (0, 59).
    int minute() const
    {
        return std::chrono::duration_cast<Minutes>(m_duration % Hours(1)).count();
    }

    /// Returns the hour of day (0, 23).
    int hour() const
    {
        return std::chrono::duration_cast<Hours>(m_duration % Days(1)).count();
    }

    /// @}

    /**
     * @name Addition/Subtraction Methods
     * @{
     */

    /// Returns the result of adding \p nanoseconds to this time as a new Time object.
    Time addNanoseconds(int nanoseconds) const
    {
        return Time(m_duration + Nanoseconds(nanoseconds));
    }

    /// Returns the result of subtracting \p nanoseconds from this time as a new Time object.
    Time subtractNanoseconds(int nanoseconds) const
    {
        return Time(m_duration - Nanoseconds(nanoseconds));
    }

    /// Returns the result of adding \p microseconds to this time as a new Time object.
    Time addMicroseconds(int microseconds) const
    {
        return Time(m_duration + Microseconds(microseconds));
    }

    /// Returns the result of subtracting \p microseconds from this time as a new Time object.
    Time subtractMicroseconds(int microseconds) const
    {
        return Time(m_duration - Microseconds(microseconds));
    }

    /// Returns the result of adding \p milliseconds to this time as a new Time object.
    Time addMilliseconds(int milliseconds) const
    {
        return Time(m_duration + Milliseconds(milliseconds));
    }

    /// Returns the result of subtracting \p milliseconds from this time as a new Time object.
    Time subtractMilliseconds(int milliseconds) const
    {
        return Time(m_duration - Milliseconds(milliseconds));
    }

    /// Returns the result of adding \p seconds to this time as a new Time object.
    Time addSeconds(int seconds) const
    {
        return Time(m_duration + Seconds(seconds));
    }

    /// Returns the result of subtracting \p seconds from this time as a new Time object.
    Time subtractSeconds(int seconds) const
    {
        return Time(m_duration - Seconds(seconds));
    }

    /// Returns the result of adding \p minutes to this time as a new Time object.
    Time addMinutes(int minutes) const
    {
        return Time(m_duration + Minutes(minutes));
    }

    /// Returns the result of subtracting \p minutes from this time as a new Time object.
    Time subtractMinutes(int minutes) const
    {
        return Time(m_duration - Minutes(minutes));
    }

    /// Returns the result of adding \p hours to this time as a new Time object.
    Time addHours(int hours) const
    {
        return Time(m_duration + Hours(hours));
    }

    /// Returns the result of subtracting \p hours from this time as a new Time object.
    Time subtractHours(int hours) const
    {
        return Time(m_duration - Hours(hours));
    }

    /// Returns the result of adding \p duration to this time as a new Time object.
    Time addDuration(const Duration& duration) const
    {
        return Time(m_duration + duration);
    }

    /// Returns the result of subtracting \p duration from this time as a new Time object.
    Time subtractDuration(const Duration& duration) const
    {
        return Time(m_duration - duration);
    }

    /// @}

    /**
     * @name Conversion Methods
     * @{
     */

    /// Returns the elapsed nanoseconds since midnight.
    long long toNanosecondsSinceMidnight() const
    {
        return std::chrono::duration_cast<Nanoseconds>(m_duration).count();
    }

    /// Returns the elapsed microseconds since midnight.
    long long toMicrosecondsSinceMidnight() const
    {
        return std::chrono::duration_cast<Microseconds>(m_duration).count();
    }

    /// Returns the elapsed milliseconds since midnight.
    long toMillisecondsSinceMidnight() const
    {
        return static_cast<long>(std::chrono::duration_cast<Milliseconds>(m_duration).count());
    }

    /// Returns the elapsed seconds since midnight.
    long toSecondsSinceMidnight() const
    {
        return static_cast<long>(std::chrono::duration_cast<Seconds>(m_duration).count());
    }

    /// Returns the elapsed minutes since midnight.
    int toMinutesSinceMidnight() const
    {
        return std::chrono::duration_cast<Minutes>(m_duration).count();
    }

    /// Returns the elapsed hours since midnight. If this time is invalid, the returned value may exceed 23. @see isValid().
    int toHoursSinceMidnight() const
    {
        return std::chrono::duration_cast<Hours>(m_duration).count();
    }

    /// Returns the elapsed time since midnight as a #Nanoseconds duration.
    Nanoseconds toStdDurationSinceMidnight() const
    {
        return m_duration;
    }

    /// Returns a **std::tm** representation of this time.
    std::tm toBrokenStdTime() const
    {
        std::tm cTime = { 0 };
        cTime.tm_hour = hour();
        cTime.tm_min = minute();
        cTime.tm_sec = second();
        return cTime;
    }

    /// Returns a **std::time_t** representation of this time.
    std::time_t toScalarStdTime() const
    {
        return std::time_t(toSecondsSinceMidnight());
    }

    /**
     * Returns the time as a string formatted according to the format string \p format.
     * The formatting string may contain the following patterns:
     *
     *    Pattern     | Meaning
     *    ----------- | -------------------------------------------
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
     * If this time is invalid, an empty string will be returned.
     */
    std::string toString(const std::string& format) const
    {
        if (!isValid())
            return std::string();

        std::stringstream output;

        for (size_t pos = 0; pos < format.size(); ++pos) {
            const int patternLength = internal::countIdenticalCharsFrom(pos, format);

            if (format[pos] == 'h') {
                output << std::setfill('0') << std::setw(patternLength) << hour();
                pos += patternLength - 1; // skip all identical characters except the last.
            } else if (format[pos] == 'H') {
                int hours12f = ((hour() == 0 || hour() == 12) ? 12 : hour() % 12);
                output << std::setfill('0') << std::setw(patternLength) << hours12f;
                pos += patternLength - 1;
            } else if (format[pos] == 'm') {
                output << std::setfill('0') << std::setw(patternLength) << minute();
                pos += patternLength - 1;
            } else if (format[pos] == 's') {
                output << std::setfill('0') << std::setw(patternLength) << second();
                pos += patternLength - 1;
            } else if (format[pos] == 'f') {
                std::string subseconds = std::to_string(std::chrono::duration_cast<Nanoseconds>(m_duration % Seconds(1)).count());
                std::string padddedSubsecondsString = subseconds.insert(0, 9 - subseconds.size(), '0');
                output << padddedSubsecondsString.substr(0, static_cast<size_t>(patternLength));
                pos += patternLength - 1;
            } else if (format[pos] == 'A') {
                output << (hour() >= 12 ? "PM" : "AM");
            } else if (format[pos] == 'a') {
                output << (hour() >= 12 ? "pm" : "am");
            } else {
                output << format[pos];
            }
        }

        return output.str();
    }

    /// @}

    /**
     * Returns a Time object set to the current time obtained from the system clock.
     * **Note** that the returned time is not the current local time, but rather the current system time, i.e., the current time in Coordinated Universal Time (UTC).
     */
    static Time current()
    {
        return Time(std::chrono::duration_cast<Nanoseconds>(std::chrono::system_clock::now().time_since_epoch() % Days(1)));
    }

    /// Returns a Time object set to midnight (i.e., "00:00:00").
    static Time midnight()
    {
        return Time(Nanoseconds::zero());
    }

    /**
     * Returns a Time object from the string \p time according to the format string \p format.
     * The format patterns are the same patterns used in the method toString(). @see toString()
     */
    static Time fromString(const std::string& time, const std::string& format)
    {
        int _hour = 0, _minute = 0, _second = 0;
        long _subsecond = 0;

        for (size_t fmtPos = 0, timPos = 0; fmtPos < format.size() && timPos < time.size(); ++fmtPos) {
            const int patternLength = internal::countIdenticalCharsFrom(fmtPos, format);

            if (format[fmtPos] == 'h' || format[fmtPos] == 'H') {
                _hour = internal::readIntAndAdvancePos(time, timPos, 2);
                fmtPos += patternLength - 1; // skip all identical characters except the last.
            } else if (format[fmtPos] == 'm') {
                _minute = internal::readIntAndAdvancePos(time, timPos, 2);
                fmtPos += patternLength - 1;
            } else if (format[fmtPos] == 's') {
                _second = internal::readIntAndAdvancePos(time, timPos, 2);
                fmtPos += patternLength - 1;
            } else if (format[fmtPos] == 'f') {
                std::string subsecondString = time.substr(timPos, static_cast<size_t>(patternLength));
                _subsecond = std::stoi(subsecondString.append(9 - subsecondString.size(), '0'));
                timPos += patternLength;
                fmtPos += patternLength - 1;
            } else if (format[fmtPos] == 'a' || format[fmtPos] == 'A') {
                if (time.substr(timPos, 2) == "pm" || time.substr(timPos, 2) == "PM") {
                    _hour = (_hour > 12 ? _hour : _hour + 12);
                    timPos += 2;
                }
            } else {
                ++timPos;
            }
        }

        return Time(Hours(_hour) + Minutes(_minute) + Seconds(_second) + Nanoseconds(_subsecond));
    }

    /**
     * @name Calculation Methods
     * The following methods return the time difference between two times: \p from and \p to.
     * If \p from is smaller than (earlier than) \p to, then the difference is negative.
     * @{
     */

    /// Returns the number of nanoseconds between \p from and \p to.
    static long long nanosecondsBetween(const Time& from, const Time& to)
    {
        return to.toNanosecondsSinceMidnight() - from.toNanosecondsSinceMidnight();
    }

    /// Returns the number of microseconds between \p from and \p to.
    static long long microsecondsBetween(const Time& from, const Time& to)
    {
        return to.toMicrosecondsSinceMidnight() - from.toMicrosecondsSinceMidnight();
    }

    /// Returns the number of milliseconds between \p from and \p to.
    static long millisecondsBetween(const Time& from, const Time& to)
    {
        return to.toMillisecondsSinceMidnight() - from.toMillisecondsSinceMidnight();
    }

    /// Returns the number of seconds between \p from and \p to.
    static long secondsBetween(const Time& from, const Time& to)
    {
        return to.toSecondsSinceMidnight() - from.toSecondsSinceMidnight();
    }

    /// Returns the number of minutes between \p from and \p to.
    static int minutesBetween(const Time& from, const Time& to)
    {
        return to.toMinutesSinceMidnight() - from.toMinutesSinceMidnight();
    }

    /// Returns the number of hours between \p from and \p to.
    static int hoursBetween(const Time& from, const Time& to)
    {
        return to.toHoursSinceMidnight() - from.toHoursSinceMidnight();
    }

    /// @}

private:
    Duration m_duration;
};

/**
 * @name Input/Output Operators
 * @relates Time
 * @{
 */

/// Writes time \p t to stream \p os in ISO-8601 time format "hh:mm:ss.fff". See toString() for information about the format patterns.
std::ostream& operator<<(std::ostream& os, const Time& t)
{
    os << t.toString("hh:mm:ss.fff");
    return os;
}

/// Reads a time in ISO-8601 time format "hh:mm:ss.fff" from stream \p is and stores it in time \p t. See toString() for information about the format patterns.
std::istream& operator>>(std::istream& is, Time& t)
{
    const int TimeFormatWidth = 12;
    char result[TimeFormatWidth];
    is.read(result, TimeFormatWidth);

    t = Time::fromString(std::string(result, TimeFormatWidth), "hh:mm:ss.fff");
    return is;
}

/// @}

} // namespace xclox

#endif // XCLOX_TIME_HPP
