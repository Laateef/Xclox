/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_INTERNAL_HPP
#define XCLOX_INTERNAL_HPP

#include <chrono>
#include <iomanip>
#include <sstream>
#include <array>
#include <algorithm>

namespace xclox {

namespace internal {

    using Days = std::chrono::duration<long, std::ratio_multiply<std::ratio<24>, std::chrono::hours::period>>;

    inline int countIdenticalCharsFrom(size_t pos, const std::string& str)
    {
        size_t idx = pos + 1;
        while (idx < str.size() && str[idx] == str[pos]) {
            ++idx;
        }
        return static_cast<int>(idx - pos);
    }

    inline int readIntAndAdvancePos(const std::string& str, size_t& pos, size_t maxDigitCount)
    {
        std::string intStr;
        while (intStr.size() < maxDigitCount && pos < str.size() && std::isdigit(str[pos])) {
            intStr += str[pos];
            ++pos;
        }
        return std::stoi(intStr);
    }

    inline std::string getShortWeekdayName(int index)
    {
        static const std::string weekdayNameArray[] = {
            "Mon",
            "Tue",
            "Wed",
            "Thu",
            "Fri",
            "Sat",
            "Sun"
        };
        return weekdayNameArray[index - 1];
    }

    inline std::string getLongWeekdayName(int index)
    {
        static const std::string weekdayNameArray[] = {
            "Monday",
            "Tuesday",
            "Wednesday",
            "Thursday",
            "Friday",
            "Saturday",
            "Sunday"
        };
        return weekdayNameArray[index - 1];
    }

    inline const std::array<std::string, 12>& getShortMonthNameArray()
    {
        static const std::array<std::string, 12> monthNameArray = {
            "Jan",
            "Feb",
            "Mar",
            "Apr",
            "May",
            "Jun",
            "Jul",
            "Aug",
            "Sep",
            "Oct",
            "Nov",
            "Dec"
        };
        return monthNameArray;
    }

    inline std::string getShortMonthName(int month)
    {
        return getShortMonthNameArray()[month - 1];
    }

    inline int getShortMonthNumber(const std::string& month)
    {
        return static_cast<int>(std::distance(getShortMonthNameArray().cbegin(), std::find(getShortMonthNameArray().cbegin(), getShortMonthNameArray().cend(), month)) + 1);
    }

    inline const std::array<std::string, 12>& getLongMonthNameArray()
    {
        static const std::array<std::string, 12> monthNameArray = {
            "January",
            "February",
            "March",
            "April",
            "May",
            "June",
            "July",
            "August",
            "September",
            "October",
            "November",
            "December"
        };
        return monthNameArray;
    }

    inline std::string getLongMonthName(int month)
    {
        return getLongMonthNameArray()[month - 1];
    }

    inline int getLongMonthNumber(const std::string& month)
    {
        return static_cast<int>(std::distance(getLongMonthNameArray().cbegin(), std::find(getLongMonthNameArray().cbegin(), getLongMonthNameArray().cend(), month)) + 1);
    }

} // namespace internal

} // namespace xclox

#endif // XCLOX_INTERNAL_HPP
