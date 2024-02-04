/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_TIMESTAMP_HPP
#define XCLOX_TIMESTAMP_HPP

#include <chrono>

namespace xclox {

namespace ntp {

    namespace internal {
        constexpr std::chrono::seconds EpochDeltaSeconds { 0x83AA7E80 };
    }

    /**
     * @class Timestamp
     *
     * Timestamp is an immutable class representing a NTP timestamp.
     *
     * A NTP timestamp is a 64-bit, unsigned fixed-point number in seconds relative to the prime epoch "1900-01-01 00:00:00".
     * It includes a 32-bit unsigned seconds field spanning 136 years and a 32-bit fraction field resolving 232 picoseconds.
     * Timestamp handles the fractional part at the resolution of the operating system clock.
     *
     * Era 0 includes dates from the prime epoch "1900-01-01 00:00:00" to "2036-02-07 06:28:15".
     * The base date for era 1 is established when the timestamp field wraps around, that is, "2036-02-07 06:28:16" corresponds to "1900-01-01 00:00:00".
     *
     * NTP timestamps are unsigned values, and operations on them produce results in the same or adjacent eras.
     * The only arithmetic operation permitted on NTP timestamps is subtraction.
     * It yields a time duration ranging from 136 years in the past to 136 years in the future.
     *
     * Timestamp objects are comparable to each other in terms of equality through operators.
     *
     * Default-constructed Timestamp objects have a value of zero, which is a special case representing unknown or unsynchronized time.
     * Timestamp objects can also be constructed from integer and fractional parts, a system time duration, or a system time point.
     * duration() can be used to convert the timestamp to a system time duration since the prime epoch.
     *
     * @see The unit tests in @ref timestamp.h for further details.
     */
    class Timestamp {
    public:
        /// @param value is a raw NTP timestamp in long format with the first 32 bits being the seconds and the other 32 bits being the fraction of a second.
        explicit Timestamp(uint64_t value = 0)
            : m_data(value)
        {
        }

        /**
         * @param seconds is the number of seconds since the prime epoch.
         * @param fraction is the fraction of a second.
         */
        explicit Timestamp(uint32_t seconds, uint32_t fraction)
            : m_data((static_cast<uint64_t>(seconds) << 32) | fraction)
        {
        }

        /// @param duration is a system time duration since the prime epoch.
        explicit Timestamp(std::chrono::system_clock::duration duration)
            : m_data(durationToValue(duration))
        {
        }

        /// @param timePoint is a system time point.
        explicit Timestamp(std::chrono::system_clock::time_point timePoint)
            : m_data(durationToValue(timePoint.time_since_epoch() + internal::EpochDeltaSeconds))
        {
        }

        /// Returns the number of seconds of the NTP timestamp.
        uint32_t seconds() const
        {
            return static_cast<uint32_t>(m_data >> 32);
        }

        /// Returns the fraction of a second of the NTP timestamp.
        uint32_t fraction() const
        {
            return static_cast<uint32_t>(m_data);
        }

        /// Returns the NTP timestamp in long format.
        uint64_t value() const
        {
            return m_data;
        }

        /// Returns the NTP timestamp as a system time duration.
        std::chrono::system_clock::duration duration() const
        {
            return std::chrono::seconds(seconds()) + std::chrono::system_clock::duration(static_cast<int>(DurationRange * fraction() / FractionRange));
        }

        /// Equality operator.
        bool operator==(const Timestamp& other) const
        {
            return m_data == other.m_data;
        }

        /// Inequality operator.
        bool operator!=(const Timestamp& other) const
        {
            return !operator==(other);
        }

        /// Returns the result of subtracting \p other from this timestamp as a system time duration.
        std::chrono::system_clock::duration operator-(const Timestamp& other) const
        {
            return duration() - other.duration();
        }

    private:
        static constexpr auto FractionRange { std::numeric_limits<uint32_t>::max() + 1.0 };
        static constexpr auto DurationRange { static_cast<double>(std::chrono::system_clock::period::den) };
        static constexpr uint64_t durationToValue(const std::chrono::system_clock::duration& duration)
        {
            return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(duration).count()) << 32
                | static_cast<uint32_t>(FractionRange * static_cast<double>((duration % std::chrono::seconds(1)).count()) / DurationRange);
        }

        uint64_t m_data;
    };

} // namespace ntp

} // namespace xclox

#endif // XCLOX_TIMESTAMP_HPP
