#include <iostream>

#include "xclox.hpp"

int main(int argc, char** argv)
{
    // Library Version
    {
        std::cout << "\nCurrent Version: " << XCLOX_VERSION_STRING
                  << "\n\tMajor: " << XCLOX_VERSION_MAJOR
                  << "\n\tMinor: " << XCLOX_VERSION_MINOR
                  << "\n\tPatch: " << XCLOX_VERSION_PATCH
                  << std::endl;
    }

    // Time
    {
        xclox::Time t(1, 2, 3);

        assert(t.hour() == 1);
        assert(t.minute() == 2);
        assert(t.second() == 3);

        std::cout << "\nRandom Time: "
                  << "\n\t Default Format: " << t
                  << "\n\t Compact Format: " << t.toString("hhmmss")
                  << std::endl;
    }

    // Date
    {
        xclox::Date d(2003, 2, 1);

        assert(d.year() == 2003);
        assert(d.month() == 2);
        assert(d.day() == 1);

        std::cout << "\nRandom Date: "
                  << "\n\t Default Format: " << d
                  << "\n\t Compact Format: " << d.toString("yyyyMMdd")
                  << std::endl;
    }

    // DateTime
    {
        using namespace xclox;

        DateTime dt(Date(2006, 5, 4), Time(3, 2, 1));

        assert(dt.year() == 2006);
        assert(dt.month() == 5);
        assert(dt.day() == 4);
        assert(dt.hour() == 3);
        assert(dt.minute() == 2);
        assert(dt.second() == 1);

        std::cout << "\nRandom DateTime: "
                  << "\n\t Default Format: " << dt
                  << "\n\t Another Format: " << dt.toString("yyyy-MM-dd hh:mm:ss.f")
                  << std::endl;
    }

    // NTP Client
    {
        using namespace xclox::ntp;

        bool passed {};
        {
            Client client([&](
                              const std::string& name,
                              const std::string& address,
                              Client::Status status,
                              const Packet& packet,
                              const std::chrono::steady_clock::duration& rtt) {
                const auto& destinationTime = std::chrono::system_clock::now();

                assert(name == "pool.ntp.org");
                assert(address.empty() == false);
                assert(status == Client::Status::Succeeded);
                assert(packet.isNull() == false);
                assert(packet.offset(destinationTime) < std::chrono::seconds(1));
                assert(rtt > std::chrono::nanoseconds(1));
                assert(rtt < std::chrono::milliseconds(QuerySeries::DefaultTimeout::ms));

                std::cout << "\nCurrent NTP Time: "
                          << xclox::DateTime(destinationTime + packet.offset(destinationTime))
                          << std::endl;
                passed = true;
            });
            client.query("pool.ntp.org");
        }
        assert(passed);
    }

    return 0;
}
