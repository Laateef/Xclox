/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_HELPER_COMMON
#define XCLOX_HELPER_COMMON

bool compare(const std::chrono::steady_clock::duration& actualDuration, const std::chrono::milliseconds& referenceDuration)
{
    const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(actualDuration) - referenceDuration;
#ifdef __APPLE__
    return std::chrono::milliseconds(-25) < diff && diff < std::chrono::milliseconds(275);
#else
    return std::chrono::milliseconds(-25) < diff && diff < std::chrono::milliseconds(75);
#endif
}

bool compare(const std::chrono::steady_clock::time_point& start, const std::chrono::milliseconds& duration)
{
    return compare(std::chrono::steady_clock::now() - start, duration);
}

namespace std {
template <class R, class P>
ostream& operator<<(ostream& os, const chrono::duration<R, P>& d)
{
    return os << steady_clock::duration(d).count();
}
template <class C>
ostream& operator<<(ostream& os, const chrono::time_point<C>& tp)
{
    return os << tp.time_since_epoch();
}
template <class T, size_t S>
ostream& operator<<(ostream& os, const array<T, S>& data)
{
    os << "0x" << setfill('0');
    for (const auto b : data) {
        os << setw(2) << hex << static_cast<int>(b);
    }
    return os;
}
} // namespace std

#endif // XCLOX_HELPER_COMMON

#ifdef ASIO_HPP
#ifndef XCLOX_HELPER_ASIO
#define XCLOX_HELPER_ASIO

const auto& broadcastEndpoint = asio::ip::udp::endpoint(asio::ip::make_address("255.255.255.255"), 2345);

std::string stringify(const asio::ip::udp::endpoint& endpoint)
{
    std::stringstream ss;
    ss << endpoint;
    return ss.str();
}

#endif // XCLOX_HELPER_ASIO
#endif // ASIO_HPP

#ifdef XCLOX_TIMESTAMP_HPP
#ifndef XCLOX_HELPER_TIMESTAMP
#define XCLOX_HELPER_TIMESTAMP

namespace std {
ostream& operator<<(ostream& os, const xclox::ntp::Timestamp& timestamp)
{
    return os << timestamp.value();
}
}

#endif // XCLOX_HELPER_TIMESTAMP
#endif // XCLOX_TIMESTAMP_HPP

#ifdef XCLOX_PACKET_HPP
#ifndef XCLOX_HELPER_PACKET
#define XCLOX_HELPER_PACKET

bool isClientPacket(const xclox::ntp::Packet& packet)
{
    return packet.version() == 4 && packet.mode() == 3 && xclox::ntp::Timestamp(std::chrono::system_clock::now()) - xclox::ntp::Timestamp(packet.transmitTimestamp()) < std::chrono::seconds(2);
}

bool isServerPacket(const xclox::ntp::Packet& packet)
{
    return (packet.version() == 3 || packet.version() == 4) && packet.mode() == 4 && xclox::ntp::Timestamp(std::chrono::system_clock::now()) - xclox::ntp::Timestamp(packet.transmitTimestamp()) < std::chrono::seconds(2);
}

namespace std {
ostream& operator<<(ostream& os, const xclox::ntp::Packet& packet)
{
    return os << packet.data();
}
}

#endif // XCLOX_HELPER_PACKET
#endif // XCLOX_PACKET_HPP
