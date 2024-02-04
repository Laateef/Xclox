/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "version.h"
#include "time.h"
#include "date.h"
#include "datetime.h"
#include "ntp/timestamp.h"
#include "ntp/coder.h"
#include "ntp/packet.h"
#include "ntp/tracer.h"
#include "ntp/server.h"
#include "ntp/query_single.h"
#include "ntp/query_series.h"
#include "ntp/query.h"
#include "ntp/client.h"

// Help doctest with printing custom data types
namespace std {
template <class R, class P>
ostream& operator<<(ostream& os, const std::chrono::duration<R, P>& d)
{
    return os << std::chrono::steady_clock::duration(d).count();
}
template <class C>
ostream& operator<<(ostream& os, const std::chrono::time_point<C>& tp)
{
    return os << tp.time_since_epoch();
}
ostream& operator<<(ostream& os, const Timestamp& timestamp)
{
    return os << timestamp.value();
}
template <class T, size_t S>
ostream& operator<<(ostream& os, const std::array<T, S>& data)
{
    os << "0x" << setfill('0');
    for (const auto b : data) {
        os << setw(2) << hex << static_cast<int>(b);
    }
    return os;
}
ostream& operator<<(ostream& os, const Packet& packet)
{
    return os << packet.data();
}
} // namespace std
