/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#include "xclox/ntp/query.hpp"

#include "tools/server.hpp"
#include "tools/tracer.hpp"

#include "tools/helper.hpp"

using namespace xclox::ntp;
using namespace std::chrono;

TEST_SUITE("Query")
{
    struct Context {
        Context()
            : server1(32101, serverTracer1.callable())
            , server2(32102, serverTracer2.callable())
            , server3(32103, serverTracer3.callable())
            , server4(32104, serverTracer4.callable())
            , server5(32105, serverTracer5.callable())
        {
        }
        Tracer<asio::ip::udp::endpoint, asio::error_code, const uint8_t*, size_t> serverTracer1, serverTracer2, serverTracer3, serverTracer4, serverTracer5;
        Tracer<std::string, std::string, Query::Status, Packet, steady_clock::duration> queryTracer;
        Server server1, server2, server3, server4, server5;
        asio::thread_pool pool;
    };

    TEST_CASE_FIXTURE(Context, "non-existing domain" * doctest::timeout(1))
    {
        const std::string& host = "x.y";
        Query::start(pool, host, queryTracer.callable());
        CHECK(queryTracer.wait() == 1);
        CHECK(queryTracer.find([&](const std::string& name, const std::string& address, Query::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == host && address.empty() && status == Query::Status::ResolveError && packet.isNull() && rtt == seconds(0);
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "non-existing server" * doctest::timeout(1))
    {
        const std::string& host = "255.255.255.255";
        Query::start(pool, host, queryTracer.callable());
        CHECK(queryTracer.wait() == 1);
        CHECK(queryTracer.find([&](const std::string& name, const std::string& address, Query::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == host && address == host + ":123" && status == Query::Status::SendError && !packet.isNull() && rtt > nanoseconds(0) && compare(rtt, milliseconds(1));
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "custom port" * doctest::timeout(1))
    {
        SUBCASE("name")
        {
            const std::string& host = "255.255.255.255:ntp";
            Query::start(pool, host, queryTracer.callable());
            CHECK(queryTracer.wait() == 1);
            CHECK(queryTracer.find([&](const std::string& name, const std::string& address, Query::Status status, const Packet& packet, const steady_clock::duration& rtt) {
                return name == host && address == "255.255.255.255:123" && status == Query::Status::SendError && !packet.isNull() && rtt > nanoseconds(0) && compare(rtt, milliseconds(1));
            }) == 1);
        }
        SUBCASE("number")
        {
            const std::string& host = "255.255.255.255:123";
            Query::start(pool, host, queryTracer.callable());
            CHECK(queryTracer.wait() == 1);
            CHECK(queryTracer.find([&](const std::string& name, const std::string& address, Query::Status status, const Packet& packet, const steady_clock::duration& rtt) {
                return name == host && address == host && status == Query::Status::SendError && !packet.isNull() && rtt > nanoseconds(0) && compare(rtt, milliseconds(1));
            }) == 1);
        }
    }

    TEST_CASE_FIXTURE(Context, "bogus server" * doctest::timeout(1))
    {
        uint8_t data {};
        server1.replay(&data, 1);
        const std::string& host = stringify(server1.endpoint());
        Query::start(pool, host, queryTracer.callable());
        CHECK(queryTracer.wait() == 1);
        CHECK(queryTracer.find([&](const std::string& name, const std::string& address, Query::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == host && address == host && status == Query::Status::ReceiveError && packet.isNull() && rtt > nanoseconds(0) && compare(rtt, milliseconds(1));
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "success" * doctest::timeout(1))
    {
        server1.replay(nullptr, 0, milliseconds(100));
        const std::string& host = stringify(server1.endpoint());
        Query::start(pool, host, queryTracer.callable());
        CHECK(queryTracer.wait() == 1);
        CHECK(queryTracer.find([&](const std::string& name, const std::string& address, Query::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == host && address == host && status == Query::Status::Succeeded && !packet.isNull() && compare(rtt, milliseconds(100));
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "non-blocking" * doctest::timeout(1))
    {
        server1.replay(nullptr, 0, milliseconds(200));
        const std::string& host = stringify(server1.endpoint());
        const auto& start = steady_clock::now();
        Query::start(pool, host, queryTracer.callable());
        CHECK(compare(start, milliseconds(1)));
        CHECK(queryTracer.wait() == 1);
        CHECK(queryTracer.find([&](const std::string& name, const std::string& address, Query::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == host && address == host && status == Query::Status::Succeeded && !packet.isNull() && compare(rtt, milliseconds(200));
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "traceable" * doctest::timeout(1))
    {
        const std::string host = "x.y";
        auto query = Query::start(pool, host, queryTracer.callable());
        CHECK_FALSE(query.expired());
        CHECK(queryTracer.wait() == 1);
        CHECK(query.expired());
    }

    TEST_CASE_FIXTURE(Context, "no callback" * doctest::timeout(1))
    {
        auto query = Query::start(pool, "254.254.254.254:1234", {});
        CHECK(query.expired());
    }

    TEST_CASE_FIXTURE(Context, "timeout - lookup" * doctest::timeout(11))
    {
        const auto& start = steady_clock::now();
        const std::string& host = "1234567890";
        const auto& timeoutMs = 100;
        auto query = Query::start(pool, host, queryTracer.callable(), milliseconds(timeoutMs));
        CHECK_FALSE(query.expired());
        CHECK(queryTracer.wait() == 1);
        CHECK(queryTracer.find([&](const std::string& name, const std::string& address, Query::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == host && address == "" && status == Query::Status::TimeoutError && packet.isNull() && rtt == seconds(0);
        }) == 1);
        CHECK(query.expired());
        CHECK(compare(start, milliseconds(timeoutMs)));
    }

    TEST_CASE_FIXTURE(Context, "timeout - query" * doctest::timeout(1))
    {
        for (int i = 0; i < 3; ++i) {
            server1.receive();
            const auto& start = steady_clock::now();
            const std::string& host = stringify(server1.endpoint());
            auto query = Query::start(pool, host, queryTracer.callable(), milliseconds(i * 100));
            CHECK_FALSE(query.expired());
            CHECK(queryTracer.wait() == 1);
            CHECK(queryTracer.find([&](const std::string& name, const std::string& address, Query::Status status, const Packet& packet, const steady_clock::duration& rtt) {
                return name == host && address == "" && status == Query::Status::TimeoutError && packet.isNull() && rtt == seconds(0);
            }) == 1);
            CHECK(compare(start, milliseconds(i * 100)));
            CHECK(query.expired());
            queryTracer.reset();
        }
    }

    TEST_CASE_FIXTURE(Context, "cancellable during lookup" * doctest::timeout(11))
    {
        const auto& start = steady_clock::now();
        const std::string& host = "1234567890";
        auto query = Query::start(pool, host, queryTracer.callable());
        CHECK_FALSE(query.expired());
        query.lock()->cancel();
        CHECK(queryTracer.wait() == 1);
        CHECK(queryTracer.find([&](const std::string& name, const std::string& address, Query::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == host && address == "" && status == Query::Status::Cancelled && packet.isNull() && rtt == seconds(0);
        }) == 1);
        CHECK(query.expired());
        CHECK(compare(start, milliseconds(1)));
    }

    TEST_CASE_FIXTURE(Context, "cancellable during query - multiple times" * doctest::timeout(1))
    {
        const auto& start = steady_clock::now();
        server1.replay(nullptr, 0, milliseconds(300));
        const std::string& host = stringify(server1.endpoint());
        auto query = Query::start(pool, host, queryTracer.callable());
        CHECK(serverTracer1.wait() == 1);
        CHECK_FALSE(query.expired());
        for (int i = 0; i < 10; ++i) {
            asio::post(pool, [query] {
                if (auto handle = query.lock()) {
                    std::this_thread::sleep_for(milliseconds(10));
                    handle->cancel();
                }
            });
        }
        CHECK(queryTracer.wait() == 1);
        CHECK(queryTracer.find([&](const std::string& name, const std::string& address, Query::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == host && address == "" && status == Query::Status::Cancelled && packet.isNull() && rtt == seconds(0);
        }) == 1);
        // The query lasts a bit before it expires on Linux
        std::this_thread::sleep_for(milliseconds(100));
        CHECK(query.expired());
        CHECK(serverTracer1.counter() == 1);
        CHECK(serverTracer1.wait(2) == 2);
        CHECK(compare(start, milliseconds(300)));
    }

    TEST_CASE_FIXTURE(Context, "cancellable concurrently" * doctest::timeout(1))
    {
        std::vector<Server*> serverList { &server1, &server2, &server3, &server4, &server5 };
        const size_t QueryCount = serverList.size();
        for (size_t j = 0; j < QueryCount; ++j) {
            for (size_t i = 0; i < QueryCount; ++i) {
                serverList.at(i)->replay(nullptr, 0, milliseconds(1));
                auto query = Query::start(pool, stringify(serverList.at(i)->endpoint()), queryTracer.callable(), milliseconds(0));
                asio::post(pool, [query] {
                    if (auto shared = query.lock()) {
                        shared->cancel();
                    }
                });
            }
            CHECK(queryTracer.wait(QueryCount) == QueryCount);
            queryTracer.reset();
        }
    }

    TEST_CASE_FIXTURE(Context, "domain name" * doctest::timeout(1))
    {
        const std::string& host = "time.windows.com";
        Query::start(pool, host, queryTracer.callable());
        CHECK(queryTracer.wait() == 1);
        CHECK(queryTracer.find([&](const std::string& name, const std::string& address, Query::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == host && !address.empty() && asio::ip::make_address(address.substr(0, address.size() - 4)).is_v4() && status == Query::Status::Succeeded && isServerPacket(packet) && rtt < seconds(1);
        }) == 1);
    }
} // TEST_SUITE
