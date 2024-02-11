/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#include "xclox/ntp/query_single.hpp"

#include "tools/server.hpp"
#include "tools/tracer.hpp"

#include "tools/helper.hpp"

using namespace xclox::ntp;
using namespace std::chrono;

TEST_SUITE("QuerySingle")
{
    struct Context {
        Context()
            : server(32101, serverTracer.callable())
        {
        }
        Tracer<asio::ip::udp::endpoint, asio::error_code, const uint8_t*, size_t> serverTracer;
        Tracer<asio::ip::udp::endpoint, asio::error_code, Packet, steady_clock::duration> queryTracer;
        Server server;
        asio::io_context io;
    };

    TEST_CASE_FIXTURE(Context, "no callback")
    {
        QuerySingle::start(io, server.endpoint(), {});
        CHECK(io.run() == 0);
    }

    TEST_CASE_FIXTURE(Context, "non-exising server")
    {
        QuerySingle::start(io, broadcastEndpoint, queryTracer.callable());
        io.run();
        CHECK(queryTracer.counter() == 1);
        CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
            return endpoint == broadcastEndpoint && error == asio::error::access_denied && isClientPacket(packet) && compare(rtt, milliseconds(1));
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "bogus server")
    {
        std::array<uint8_t, 9> sendData {};
        server.replay(sendData.data(), sendData.size());
        QuerySingle::start(io, server.endpoint(), queryTracer.callable());
        io.run();
        CHECK(serverTracer.wait(2) == 2);
        CHECK(serverTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const uint8_t* data, size_t size) {
            Packet::DataType buffer;
            std::memcpy(buffer.data(), data, size);
            return endpoint.address() == server.endpoint().address() && !error && isClientPacket(Packet(buffer)) && size == std::tuple_size<Packet::DataType> {};
        }) == 1);
        CHECK(serverTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const uint8_t* data, size_t size) {
            return endpoint.address() == server.endpoint().address() && !error && std::memcmp(data, sendData.data(), sendData.size()) == 0 && size == sendData.size();
        }) == 1);
        CHECK(queryTracer.counter() == 1);
        CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
            return endpoint == server.endpoint() && error == asio::error::message_size && packet.isNull() && compare(rtt, milliseconds(1));
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "valid server")
    {
        server.replay();
        QuerySingle::start(io, server.endpoint(), queryTracer.callable());
        io.run();
        CHECK(serverTracer.wait(2) == 2);
        const uint8_t* recvData = nullptr;
        size_t recvSize = 0;
        CHECK(serverTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const uint8_t* data, size_t size) {
            recvData = data;
            recvSize = size;
            return endpoint.address() == server.endpoint().address() && !error && size == std::tuple_size<Packet::DataType> {};
        }) == 2);
        CHECK(queryTracer.counter() == 1);
        CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
            return endpoint == server.endpoint() && !error && std::memcmp(packet.data().data(), recvData, recvSize) == 0 && isClientPacket(packet) && compare(rtt, milliseconds(1));
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "round-trip time")
    {
        SUBCASE("no delay")
        {
            QuerySingle::start(io, broadcastEndpoint, queryTracer.callable());
            io.run();
            CHECK(queryTracer.counter() == 1);
            CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
                return endpoint == broadcastEndpoint && error == asio::error::access_denied && isClientPacket(packet) && compare(rtt, milliseconds(1));
            }) == 1);
        }
        SUBCASE("receive error")
        {
            server.receive();
            QuerySingle::start(io, server.endpoint(), queryTracer.callable());
            std::this_thread::sleep_for(milliseconds(100));
            std::thread([&] {
                io.run();
            }).detach();
            CHECK(serverTracer.wait() == 1);
            asio::ip::udp::endpoint sender;
            CHECK(serverTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const uint8_t*, size_t) {
                sender = endpoint;
                return !error;
            }) == 1);
            std::this_thread::sleep_for(milliseconds(100));
            server.send(sender, nullptr, 0);
            CHECK(queryTracer.wait() == 1);
            CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
                return endpoint == server.endpoint() && error == asio::error::message_size && packet.isNull() && compare(rtt, milliseconds(200));
            }) == 1);
        }
        SUBCASE("successful query")
        {
            server.receive();
            QuerySingle::start(io, server.endpoint(), queryTracer.callable());
            std::this_thread::sleep_for(milliseconds(200));
            std::thread([&] {
                io.run();
            }).detach();
            CHECK(serverTracer.wait() == 1);
            asio::ip::udp::endpoint sender;
            Packet::DataType buffer;
            CHECK(serverTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const uint8_t* data, size_t size) {
                sender = endpoint;
                std::memcpy(buffer.data(), data, size);
                return !error;
            }) == 1);
            std::this_thread::sleep_for(milliseconds(200));
            server.send(sender, buffer.data(), buffer.size());
            CHECK(queryTracer.wait() == 1);
            CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
                return endpoint == server.endpoint() && !error && isClientPacket(packet) && compare(rtt, milliseconds(400));
            }) == 1);
        }
    }

    TEST_CASE_FIXTURE(Context, "custom timeout")
    {
        for (int i = 0; i < 3; ++i) {
            const auto& start = steady_clock::now();
            server.receive();
            QuerySingle::start(io, server.endpoint(), queryTracer.callable(), milliseconds(i * 100));
            io.run();
            CHECK(compare(start, milliseconds(i * 100)));
            CHECK(queryTracer.counter() == 1);
            CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
                return endpoint == server.endpoint() && error == asio::error::timed_out && packet.isNull() && compare(rtt, milliseconds(i * 100));
            }) == 1);
            CHECK(serverTracer.wait() == 1);
            queryTracer.reset();
            serverTracer.reset();
            io.restart();
        }
    }

    TEST_CASE_FIXTURE(Context, "default timeout")
    {
        const auto& start = steady_clock::now();
        server.receive();
        QuerySingle::start(io, server.endpoint(), queryTracer.callable());
        io.run();
        CHECK(compare(start, milliseconds(QuerySingle::DefaultTimeout::ms)));
        CHECK(queryTracer.counter() == 1);
        CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
            return endpoint == server.endpoint() && error == asio::error::timed_out && packet.isNull() && compare(rtt, milliseconds(QuerySingle::DefaultTimeout::ms));
        }) == 1);
        CHECK(serverTracer.counter() == 1);
    }

    TEST_CASE_FIXTURE(Context, "traceable")
    {
        SUBCASE("no query")
        {
            auto query = QuerySingle::start(io, broadcastEndpoint, {});
            CHECK(query.expired());
        }
        SUBCASE("unsuccessful query")
        {
            auto query = QuerySingle::start(io, broadcastEndpoint, queryTracer.callable());
            CHECK_FALSE(query.expired());
            io.run();
            CHECK(queryTracer.counter() == 1);
            CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet&, const steady_clock::duration&) {
                return endpoint == broadcastEndpoint && error == asio::error::access_denied;
            }) == 1);
            CHECK(query.expired());
        }
        SUBCASE("successful query")
        {
            server.replay();
            auto query = QuerySingle::start(io, server.endpoint(), queryTracer.callable());
            CHECK_FALSE(query.expired());
            io.run();
            CHECK(queryTracer.counter() == 1);
            CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet&, const steady_clock::duration&) {
                return endpoint == server.endpoint() && !error;
            }) == 1);
            CHECK(query.expired());
        }
    }

    TEST_CASE_FIXTURE(Context, "cancellable")
    {
        SUBCASE("on start")
        {
            auto query = QuerySingle::start(io, server.endpoint(), queryTracer.callable());
            query.lock()->cancel();
            io.run();
            CHECK(query.expired());
            CHECK(queryTracer.wait() == 1);
            CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
                return endpoint == server.endpoint() && error == asio::error::operation_aborted && packet.isNull() && compare(rtt, milliseconds(1));
            }) == 1);
            CHECK(serverTracer.counter() == 0);
        }
        SUBCASE("on receive")
        {
            server.receive();
            auto query = QuerySingle::start(io, server.endpoint(), queryTracer.callable());
            std::thread([&] {
                io.run();
            }).detach();
            CHECK(serverTracer.wait() == 1);
            query.lock()->cancel();
            CHECK(queryTracer.wait() == 1);
            CHECK(queryTracer.find([&](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const steady_clock::duration& rtt) {
                return endpoint == server.endpoint() && error == asio::error::operation_aborted && packet.isNull() && compare(rtt, milliseconds(1));
            }) == 1);
            CHECK(query.expired());
        }
        SUBCASE("on finish")
        {
            server.receive();
            auto query = QuerySingle::start(io, server.endpoint(), queryTracer.callable());
            auto handle = query.lock();
            std::thread([&] {
                io.run();
            }).detach();
            CHECK(serverTracer.wait() == 1);
            handle->cancel();
            CHECK(queryTracer.wait() == 1);
            handle->cancel();
            io.restart();
            CHECK(io.run() == 0);
        }
    } // TEST_CASE
} // TEST_SUITE
