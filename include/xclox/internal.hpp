/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_INTERNAL_HPP
#define XCLOX_INTERNAL_HPP

#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <iomanip>
#include <sstream>
#include <unordered_map>

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
            intStr += str[pos++];
        }
        return std::stoi(intStr);
    }

    inline const std::array<std::string, 7>& getShortWeekdayNameArray()
    {
        static const std::array<std::string, 7> weekdayNameArray = {
            "Mon",
            "Tue",
            "Wed",
            "Thu",
            "Fri",
            "Sat",
            "Sun"
        };
        return weekdayNameArray;
    }

    inline std::string getShortWeekdayName(int day)
    {
        return getShortWeekdayNameArray()[day - 1];
    }

    inline const std::array<std::string, 7>& getLongWeekdayNameArray()
    {
        static const std::array<std::string, 7> weekdayNameArray = {
            "Monday",
            "Tuesday",
            "Wednesday",
            "Thursday",
            "Friday",
            "Saturday",
            "Sunday"
        };
        return weekdayNameArray;
    }

    inline std::string getLongWeekdayName(int day)
    {
        return getLongWeekdayNameArray()[day - 1];
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

    template <size_t S>
    inline const size_t search(const std::array<std::string, S>& keywordArray, const std::string& input)
    {
        return std::distance(keywordArray.cbegin(), std::find_if(keywordArray.cbegin(), keywordArray.cend(), [&](const std::string& keyword) {
            return std::search(
                       input.cbegin(), input.cend(),
                       keyword.cbegin(), keyword.cend(),
                       [](char c1, char c2) {
                           return std::tolower(c1) == std::tolower(c2);
                       })
                != input.cend();
        }));
    }

    inline bool isPattern(char flag, size_t count = 0)
    {
        static const std::unordered_map<char, size_t> patternMap {
            { '#', 1 },
            { 'E', 1 },
            { 'y', (1 | 1 << 1 | 1 << 3) },
            { 'M', (1 | 1 << 1 | 1 << 2 | 1 << 3) },
            { 'd', (1 | 1 << 1 | 1 << 2 | 1 << 3) },
            { 'h', (1 | 1 << 1) },
            { 'H', (1 | 1 << 1) },
            { 'm', (1 | 1 << 1) },
            { 's', (1 | 1 << 1) },
            { 'f', (1 | 1 << 1 | 1 << 2 | 1 << 3 | 1 << 4 | 1 << 5 | 1 << 6 | 1 << 7 | 1 << 8) },
            { 'a', 1 },
            { 'A', 1 }
        };
        const auto iter = patternMap.find(flag);
        return iter != patternMap.cend() && (count == 0 || iter->second & (1 << count - 1));
    }

} // namespace internal

} // namespace xclox

#endif // XCLOX_INTERNAL_HPP
