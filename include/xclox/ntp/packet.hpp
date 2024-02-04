/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_PACKET_HPP
#define XCLOX_PACKET_HPP

#include "coder.hpp"
#include "timestamp.hpp"

#include <algorithm>
#include <array>
#include <memory>

namespace xclox {

namespace ntp {

    namespace internal {
        using DataType = std::array<uint8_t, 48>;

        inline bool isZeroed(const DataType& data)
        {
            return !std::any_of(data.cbegin(), data.cend(), [](int x) { return x; });
        }

        inline std::shared_ptr<const DataType> pointerize(const DataType& data)
        {
            return isZeroed(data) ? nullptr : std::make_shared<const DataType>(data);
        }
    }

    /**
     * @class Packet
     *
     * Packet is an immutable raw NTP packet.
     *
     * Packet internally holds only the required NTP fields (48 bytes).
     *
     * A packet is null if all its data is zeros, and this can be checked with isNull().
     * Packet is serializable to raw data via toData().
     * Packet objects are comparable to each other in terms of equality through operators.
     *
     * Delay and offset calculations can be carried out via delay() and offset(), respectively.
     * The calculations are correct only if:
     *   - The client clock is consistent across the departure and arrival of the NTP packet.
     *   - The client clock is within 68 years of the server; otherwise, the returned offset is ambiguous and cannot be resolved correctly to a real timestamp.
     *
     * @see The unit tests in @ref packet.h for further details.
     */
    class Packet {
    public:
        using DataType = internal::DataType; ///< Type of packet's underlying data.

        /// Constructs a NTP packet from the given values.
        explicit Packet(uint8_t leap, uint8_t version, uint8_t mode, uint8_t stratum, int8_t poll, int8_t precision, uint32_t rootDelay, uint32_t rootDispersion, uint32_t referenceID, uint64_t referenceTimestamp, uint64_t originTimestamp, uint64_t receiveTimestamp, uint64_t transmitTimestamp)
        {
            DataType d;
            Coder::serialize<uint8_t>(static_cast<uint8_t>(leap << 6 | version << 3 | mode), &d[0]);
            Coder::serialize<uint8_t>(stratum, &d[1]);
            Coder::serialize<uint8_t>(static_cast<uint8_t>(poll), &d[2]);
            Coder::serialize<uint8_t>(static_cast<uint8_t>(precision), &d[3]);
            Coder::serialize<uint32_t>(rootDelay, &d[4]);
            Coder::serialize<uint32_t>(rootDispersion, &d[8]);
            Coder::serialize<uint32_t>(referenceID, &d[12]);
            Coder::serialize<uint64_t>(referenceTimestamp, &d[16]);
            Coder::serialize<uint64_t>(originTimestamp, &d[24]);
            Coder::serialize<uint64_t>(receiveTimestamp, &d[32]);
            Coder::serialize<uint64_t>(transmitTimestamp, &d[40]);
            m_data = internal::pointerize(d);
        }

        /// Constructs a NTP packet from the given raw data buffer.
        explicit Packet(const DataType& data)
            : m_data(internal::pointerize(data))
        {
        }

        /// Constructs a null NTP packet that has no underlying data. @see isNull()
        Packet() = default;

        /// Default destructor.
        ~Packet() = default;

        /// Copy constructor.
        Packet(const Packet&) = default;

        /// Move constructor.
        Packet(Packet&&) = default;

        /// Copy assignment operator.
        Packet& operator=(const Packet&) = delete;

        /// Move assignment operator.
        Packet& operator=(Packet&&) = delete;

        /// Equality operator.
        bool operator==(const Packet& other) const
        {
            return m_data && other.m_data ? *m_data == *other.m_data : isNull() && other.isNull();
        }

        /// Inequality operator.
        bool operator!=(const Packet& other) const
        {
            return !operator==(other);
        }

        /// Returns a raw data representation of the underlying packet.
        DataType data() const
        {
            return m_data ? *m_data : DataType {};
        }

        /// Returns whether the underlying data is all zeros.
        bool isNull() const
        {
            return !m_data || internal::isZeroed(*m_data);
        }

        /**
         * Returns an integer warning of an impending leap second to be inserted or deleted in the last minute of the current month.
         *
         *    Value | Meaning
         *    ----- | -------------------------------------
         *    0     | no warning
         *    1     | last minute of the day has 61 seconds
         *    2     | last minute of the day has 59 seconds
         *    3     | unknown (clock unsynchronized)
         */
        uint8_t leap() const
        {
            return static_cast<uint8_t>(m_data ? m_data->at(0) >> 6 : 0);
        }

        /// Returns an unsigned integer representing the NTP version number.
        uint8_t version() const
        {
            return static_cast<uint8_t>(m_data ? m_data->at(0) >> 3 & 7 : 0);
        }

        /**
         * Returns an unsigned integer representing the relationship between two NTP speakers.
         *
         *    Value | Meaning
         *    ----- | ------------------------
         *    0     | reserved
         *    1     | symmetric active
         *    2     | symmetric passive
         *    3     | client
         *    4     | server
         *    5     | broadcast
         *    6     | NTP control message
         *    7     | reserved for private use
         */
        uint8_t mode() const
        {
            return static_cast<uint8_t>(m_data ? m_data->at(0) & 7 : 0);
        }

        /**
         * Returns an unsigned integer representing the level of the server in the NTP hierarchy.
         *
         *    Value     | Meaning
         *    --------- | ---------------------------------------------------
         *    0         | unspecified or invalid
         *    1         | primary server (e.g., equipped with a GPS receiver)
         *    2..15     | secondary server (via NTP)
         *    16        | unsynchronized
         *    17..255   | reserved
         */
        uint8_t stratum() const
        {
            return static_cast<uint8_t>(m_data ? m_data->at(1) : 0);
        }

        /// Returns a signed integer representing the maximum interval between successive messages, in log2 seconds.
        int8_t poll() const
        {
            return m_data ? static_cast<int8_t>(m_data->at(2)) : 0;
        }

        /// Returns a signed integer representing the precision of the system clock, in log2 seconds.
        int8_t precision() const
        {
            return m_data ? static_cast<int8_t>(m_data->at(3)) : 0;
        }

        /// Returns the total round-trip delay to the reference clock, in NTP short format.
        uint32_t rootDelay() const
        {
            return m_data ? Coder::deserialize<uint32_t>(&m_data->at(4)) : 0;
        }

        /// Returns the total dispersion to the reference clock, in NTP short format.
        uint32_t rootDispersion() const
        {
            return m_data ? Coder::deserialize<uint32_t>(&m_data->at(8)) : 0;
        }

        /// Returns a 32-bit code identifying the particular server or reference clock.
        uint32_t referenceID() const
        {
            return m_data ? Coder::deserialize<uint32_t>(&m_data->at(12)) : 0;
        }

        /// Returns the server's time at which the system clock was last set or corrected.
        uint64_t referenceTimestamp() const
        {
            return m_data ? Coder::deserialize<uint64_t>(&m_data->at(16)) : 0;
        }

        /// Returns the client's time at which the packet departed to the server.
        uint64_t originTimestamp() const
        {
            return m_data ? Coder::deserialize<uint64_t>(&m_data->at(24)) : 0;
        }

        /// Returns the server's time at which the packet arrived from the client.
        uint64_t receiveTimestamp() const
        {
            return m_data ? Coder::deserialize<uint64_t>(&m_data->at(32)) : 0;
        }

        /// Returns the server's time at which the packet departed to the client.
        uint64_t transmitTimestamp() const
        {
            return m_data ? Coder::deserialize<uint64_t>(&m_data->at(40)) : 0;
        }

        /**
         * Returns the round-trip delay of the NTP packet passed from client to server and back again.
         * In some scenarios, it is possible for the delay computation to become negative and misleads the subsequent computations.
         * So, the returned value has to be clamped or checked before further processing.
         * @param destination the client's time at which the packet arrived from the server.
         */
        std::chrono::system_clock::duration delay(uint64_t destination) const
        {
            return (Timestamp(destination - originTimestamp())) - (Timestamp(transmitTimestamp() - receiveTimestamp()));
        }

        /**
         * Returns the time offset of the server relative to the client.
         * The time offset can range from 136 years in the past to 136 years in the future.
         * However, because timestamps can belong to different eras, ambiguous values may be returned.
         * So, this method is not intended to be used directly, as it works only with timestamps in the same era.
         * Instead, use the overload of this method, which takes a time point to get the correct offset for timestamps in different eras.
         * @param destination the client's time at which the packet arrived from the server.
         */
        std::chrono::system_clock::duration offset(uint64_t destination) const
        {
            return ((Timestamp(receiveTimestamp()) - Timestamp(originTimestamp())) + (Timestamp(transmitTimestamp()) - Timestamp(destination))) / 2;
        }

        /**
         * Returns the time offset of the server relative to the client.
         * The time offset can range from 68 years in the past to 68 years in the future.
         * So, the client clock must be set within 68 years of the server before the service is started.
         * This method can calculate the offset correctly for timestamps in the same or adjacent eras.
         * @param destination the client's time at which the packet arrived from the server.
         */
        std::chrono::system_clock::duration offset(const std::chrono::system_clock::time_point& destination) const
        {
            const auto& rawOffset = offset(Timestamp(destination.time_since_epoch() + internal::EpochDeltaSeconds).value());
            return std::chrono::seconds(static_cast<int32_t>(std::chrono::duration_cast<std::chrono::seconds>(rawOffset).count())) + rawOffset % std::chrono::seconds(1);
        }

    private:
        std::shared_ptr<const internal::DataType> m_data;
    };

} // namespace ntp

} // namespace xclox

#endif // XCLOX_PACKET_HPP
