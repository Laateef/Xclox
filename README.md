# Xclox

[![build](https://github.com/laateef/xclox/workflows/.github/workflows/ci.yml/badge.svg)](https://github.com/laateef/xclox/actions)
[![license](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

A header-only cross-platform date and time library for C++.
It offers a high-level approach to creating, manipulating, and formatting times, dates, and datetimes.
It provides a flexible and advanced NTP client for querying internet time in an asynchronous fashion based on the ASIO library.
It has been thoroughly tested, as its development has been test-driven (TDD).

## Usage

The following example shows only the basic functionalities of the library. For further details, please see the documentation.

### Example

```cpp
#include <iostream>

#include <xclox.hpp>

using namespace xclox;

int main(int argc, char** argv)
{
    Time t(1, 2, 3);
    Date d(2003, 2, 1);
    DateTime dt(d, t);

    std::cout << " DateTime: " << dt
              << "\n\t Time: " << d
              << "\n\t Date: " << t
              << "\n\n Epoch: " << DateTime::epoch()
              << "\n\n Now: " << DateTime::current().toString("yyyy-MM-dd hh:mm:ss.f")
              << "\n\n Tomorrow: " << DateTime(std::chrono::system_clock::now()) + DateTime::Days(1);

    ntp::Client client([&](
                           const std::string& name,
                           const std::string& address,
                           ntp::Client::Status status,
                           const ntp::Packet& packet,
                           const std::chrono::steady_clock::duration& rtt) {
        const auto& destinationTime = std::chrono::system_clock::now();
        std::cout << "\n\n NTP Client: "
                  << "\n\t Server Name: " << name
                  << "\n\t Resolved Address: " << address
                  << "\n\t Query Status: " << (status == ntp::Client::Status::Succeeded ? "Succeeded" : "Failed")
                  << "\n\t Packet: "
                  << "\n\t\t Valid: " << std::boolalpha << !packet.isNull()
                  << "\n\t\t Reference Timestamp: " << packet.referenceTimestamp()
                  << "\n\t\t Origin Timestamp: " << packet.originTimestamp()
                  << "\n\t\t Receive Timestamp: " << packet.originTimestamp()
                  << "\n\t\t Transmit Timestamp: " << packet.originTimestamp()
                  << "\n\t Destination Time: " << DateTime(destinationTime)
                  << "\n\t Round-trip Time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(rtt).count() << "ns"
                  << "\n\t Time Offset: " << std::chrono::duration_cast<std::chrono::nanoseconds>(packet.offset(destinationTime)).count() << "ns"
                  << "\n\t Adjusted Time: " << DateTime(destinationTime + packet.offset(destinationTime));
    });

    client.query("pool.ntp.org");

    return 0;
}
```

### Output

```
 DateTime: 2003-02-01T01:02:03.000
         Time: 2003-02-01
         Date: 01:02:03.000

 Epoch: 1970-01-01T00:00:00.000

 Now: 2024-01-29 03:16:56.7

 Tomorrow: 2024-01-30T03:16:56.770

 NTP Client:
         Server Name: pool.ntp.org
         Resolved Address: 162.159.200.1:123
         Query Status: Succeeded
         Packet:
                 Valid: true
                 Reference Timestamp: 16816888586391356887
                 Origin Timestamp: 16816888685208543774
                 Receive Timestamp: 16816888685208543774
                 Transmit Timestamp: 16816888685208543774
         Destination Time: 2024-01-29T03:16:56.916
         Round-trip Time: 83525300ns
         Time Offset: 360550500ns
         Adjusted Time: 2024-01-29T03:16:57.276
```
