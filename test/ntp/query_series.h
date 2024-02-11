/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#include "xclox/ntp/query_series.hpp"

#include "tools/server.hpp"
#include "tools/tracer.hpp"

#include "tools/helper.hpp"

using namespace xclox::ntp;
using namespace std::chrono;

TEST_SUITE("QuerySeries")
{
    struct Context {
        Context()
            : server1(32101, serverTracer1.callable())
            , server2(32102, serverTracer2.callable())
            , server3(32103, serverTracer3.callable())
        {
        }
        Tracer<asio::ip::udp::endpoint, asio::error_code, const uint8_t*, size_t> serverTracer1, serverTracer2, serverTracer3;
        Tracer<asio::ip::udp::endpoint, asio::error_code, Packet, steady_clock::duration> queryTracer;
        Server server1, server2, server3;
        asio::io_context io;
    };

    TEST_CASE_FIXTURE(Context, "no callback or endpoints")
    {
        QuerySeries::start(io, asio::ip::udp::resolver::results_type(), {});
        QuerySeries::start(io, asio::ip::udp::resolver::results_type::create(server1.endpoint(), "", ""), {});
        QuerySeries::start(io, asio::ip::udp::resolver::results_type(), queryTracer.callable());
        CHECK(io.run() == 0);
    }

    TEST_CASE_FIXTURE(Context, "single query fails")
    {
        QuerySeries::start(io, asio::ip::udp::resolver::results_type::create(broadcastEndpoint, "", ""), queryTracer.callable());
        io.run();
        CHECK(queryTracer.counter() == 1);
        CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
            return endpoint == broadcastEndpoint && error == asio::error::access_denied && isClientPacket(packet) && compare(rtt, milliseconds(1));
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "single query succeeds")
    {
        server1.replay();
        QuerySeries::start(io, asio::ip::udp::resolver::results_type::create(server1.endpoint(), "", ""), queryTracer.callable());
        io.run();
        CHECK(queryTracer.counter() == 1);
        CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
            return endpoint == server1.endpoint() && !error && isClientPacket(packet) && compare(rtt, milliseconds(1));
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "two queries fail")
    {
        const auto& localEndpoint = asio::ip::udp::endpoint(asio::ip::make_address("0.0.0.0"), 1234);
        std::vector<asio::ip::udp::endpoint> endpointList { localEndpoint, broadcastEndpoint };
        QuerySeries::start(io, asio::ip::udp::resolver::results_type::create(endpointList.begin(), endpointList.end(), "", ""), queryTracer.callable());
        io.run();
        CHECK(queryTracer.counter() == 1);
        CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
            return endpoint == broadcastEndpoint && error == asio::error::access_denied && isClientPacket(packet) && compare(rtt, milliseconds(1));
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "first query fails, second query times out, third query succeeds")
    {
        uint8_t data { 1 };
        server1.replay(&data, 0);
        server2.receive();
        server3.replay();
        std::vector<asio::ip::udp::endpoint> endpointList { server1.endpoint(), server2.endpoint(), server3.endpoint() };
        QuerySeries::start(io, asio::ip::udp::resolver::results_type::create(endpointList.begin(), endpointList.end(), "", ""), queryTracer.callable());
        io.run();
        CHECK(queryTracer.counter() == 1);
        CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
            return endpoint == server3.endpoint() && !error && isClientPacket(packet) && compare(rtt, milliseconds(1));
        }) == 1);
        CHECK(serverTracer1.wait(2) == 2);
        CHECK(serverTracer2.wait(1) == 1);
        CHECK(serverTracer3.wait(2) == 2);
    }

    TEST_CASE_FIXTURE(Context, "traceable")
    {
        SUBCASE("single-target")
        {
            auto query = QuerySeries::start(io, asio::ip::udp::resolver::results_type::create(broadcastEndpoint, "", ""), queryTracer.callable());
            CHECK_FALSE(query.expired());
            io.run();
            CHECK(queryTracer.counter() == 1);
            CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
                return endpoint == broadcastEndpoint && error == asio::error::access_denied && !packet.isNull() && compare(rtt, milliseconds(1));
            }) == 1);
            CHECK(query.expired());
        }
        SUBCASE("multi-target")
        {
            server1.replay();
            std::vector<asio::ip::udp::endpoint> endpointList { server1.endpoint(), broadcastEndpoint };
            auto query = QuerySeries::start(io, asio::ip::udp::resolver::results_type::create(endpointList.begin(), endpointList.end(), "", ""), queryTracer.callable());
            CHECK_FALSE(query.expired());
            io.run();
            CHECK(serverTracer1.wait(2) == 2);
            CHECK(queryTracer.counter() == 1);
            CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
                return endpoint == server1.endpoint() && !error && !packet.isNull() && compare(rtt, milliseconds(1));
            }) == 1);
            CHECK(query.expired());
        }
        SUBCASE("3-target")
        {
            const auto& someEndpoint = asio::ip::udp::endpoint(asio::ip::make_address("254.254.254.254"), 1234);
            server1.replay();
            std::vector<asio::ip::udp::endpoint> endpointList { broadcastEndpoint, server1.endpoint(), someEndpoint };
            auto query = QuerySeries::start(io, asio::ip::udp::resolver::results_type::create(endpointList.begin(), endpointList.end(), "", ""), queryTracer.callable());
            CHECK_FALSE(query.expired());
            io.run();
            CHECK(serverTracer1.wait(2) == 2);
            CHECK(queryTracer.counter() == 1);
            CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
                return endpoint == server1.endpoint() && !error && !packet.isNull() && compare(rtt, milliseconds(1));
            }) == 1);
            CHECK(query.expired());
        }
    }

    TEST_CASE_FIXTURE(Context, "cancellable")
    {
        SUBCASE("before running a query")
        {
            auto query = QuerySeries::start(io, asio::ip::udp::resolver::results_type::create(broadcastEndpoint, "", ""), queryTracer.callable());
            query.lock()->cancel();
            io.run();
            CHECK(queryTracer.wait() == 1);
            CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
                return endpoint == broadcastEndpoint && error == asio::error::operation_aborted && !packet.isNull() && compare(rtt, milliseconds(1));
            }) == 1);
            CHECK(query.expired());
        }
        SUBCASE("during the first query")
        {
            server1.receive();
            auto query = QuerySeries::start(io, asio::ip::udp::resolver::results_type::create(server1.endpoint(), "", ""), queryTracer.callable());
            std::thread([&] {
                io.run();
            }).detach();
            CHECK(serverTracer1.wait() == 1);
            query.lock()->cancel();
            CHECK(queryTracer.wait() == 1);
            CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
                return endpoint == server1.endpoint() && error == asio::error::operation_aborted && packet.isNull() && compare(rtt, milliseconds(1));
            }) == 1);
            CHECK(query.expired());
        }
        SUBCASE("during the second query")
        {
            uint8_t data {};
            server1.replay(&data, 1);
            server2.receive();
            server3.replay();
            std::vector<asio::ip::udp::endpoint> endpointList { server1.endpoint(), server2.endpoint(), server3.endpoint() };
            auto query = QuerySeries::start(io, asio::ip::udp::resolver::results_type::create(endpointList.begin(), endpointList.end(), "", ""), queryTracer.callable());
            std::thread([&] {
                io.run();
            }).detach();
            CHECK(serverTracer1.wait(2) == 2);
            CHECK(serverTracer2.wait() == 1);
            query.lock()->cancel();
            CHECK(queryTracer.wait() == 1);
            CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
                return endpoint == server2.endpoint() && error == asio::error::operation_aborted && packet.isNull() && compare(rtt, milliseconds(1));
            }) == 1);
            CHECK(serverTracer3.counter() == 0);
            CHECK(query.expired());
        }
        SUBCASE("multiple cancellations")
        {
            auto query = QuerySeries::start(io, asio::ip::udp::resolver::results_type::create(broadcastEndpoint, "", ""), queryTracer.callable());
            auto handle = query.lock();
            handle->cancel();
            io.run();
            handle->cancel();
            CHECK(io.run() == 0);
        }
    }

    TEST_CASE_FIXTURE(Context, "custom timeout")
    {
        for (int i = 0; i < 3; ++i) {
            server1.receive();
            const auto& start = steady_clock::now();
            auto query = QuerySeries::start(io, asio::ip::udp::resolver::results_type::create(server1.endpoint(), "", ""), queryTracer.callable(), milliseconds(i * 100));
            io.run();
            io.restart();
            CHECK(compare(start, milliseconds(i * 100)));
            CHECK(query.expired());
            CHECK(queryTracer.counter() == 1);
            CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
                return endpoint == server1.endpoint() && error == asio::error::timed_out && packet.isNull() && compare(rtt, milliseconds(i * 100));
            }) == 1);
            queryTracer.reset();
        }
    }

    TEST_CASE_FIXTURE(Context, "default timeout")
    {
        server1.receive();
        server2.receive(milliseconds(QuerySingle::DefaultTimeout::ms + 100));
        const auto& start = steady_clock::now();
        std::vector<asio::ip::udp::endpoint> endpointList { server1.endpoint(), server2.endpoint() };
        auto query = QuerySeries::start(io, asio::ip::udp::resolver::results_type::create(endpointList.begin(), endpointList.end(), "", ""), queryTracer.callable());
        io.run();
        CHECK(compare(start, milliseconds(QuerySeries::DefaultTimeout::ms)));
        CHECK(query.expired());
        CHECK(queryTracer.counter() == 1);
        CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
            return endpoint == server2.endpoint() && error == asio::error::timed_out && packet.isNull() && compare(rtt, milliseconds(QuerySeries::DefaultTimeout::ms - QuerySingle::DefaultTimeout::ms));
        }) == 1);
    }
} // TEST_SUITE
