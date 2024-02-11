/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#include "tools/server.hpp"
#include "tools/tracer.hpp"
#include "xclox/ntp/client.hpp"

using namespace xclox::ntp;
using namespace std::chrono;

TEST_SUITE("Client")
{
    auto isClientPacket = [](const Packet& packet) {
        return packet.version() == 4 && packet.mode() == 3 && Timestamp(system_clock::now()) - Timestamp(packet.transmitTimestamp()) < seconds(2);
    };
    auto isServerPacket = [](const Packet& packet) {
        return (packet.version() == 3 || packet.version() == 4) && packet.mode() == 4 && Timestamp(system_clock::now()) - Timestamp(packet.transmitTimestamp()) < seconds(2);
    };
    auto stringify = [](const asio::ip::udp::endpoint& endpoint) {
        std::stringstream ss;
        ss << endpoint;
        return ss.str();
    };
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
        Tracer<std::string, std::string, Client::Status, Packet, steady_clock::duration> clientTracer;
        Server server1, server2, server3, server4, server5;
    };

    TEST_CASE_FIXTURE(Context, "query" * doctest::timeout(5))
    {
        Client client(clientTracer.callable());
        client.query("x.y");
        client.query("255.255.255.255");
        client.query("time.windows.com");
        CHECK(clientTracer.wait(3, seconds(5)) == 3);
        CHECK(clientTracer.find([&](const std::string& name, const std::string& address, Client::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == "x.y" && address == "" && status == Client::Status::ResolveError && packet.isNull() && rtt == seconds(0);
        }) == 1);
        CHECK(clientTracer.find([&](const std::string& name, const std::string& address, Client::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == "255.255.255.255" && address == "255.255.255.255:123" && status == Client::Status::SendError && isClientPacket(packet) && rtt < seconds(1);
        }) == 1);
        CHECK(clientTracer.find([&](const std::string& name, const std::string& address, Client::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == "time.windows.com" && !address.empty() && asio::ip::make_address(address.substr(0, address.size() - 4)).is_v4() && status == Client::Status::Succeeded && isServerPacket(packet) && rtt < seconds(5);
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "reset callback" * doctest::timeout(2))
    {
        Client client(clientTracer.callable());
        client.query("x.y");
        CHECK(clientTracer.wait(1) == 1);
        client.setCallback({});
        client.query("x.y");
        CHECK(clientTracer.wait(2) == 1);
    }

    TEST_CASE_FIXTURE(Context, "query concurrently" * doctest::timeout(1))
    {
        Client client(clientTracer.callable());
        const std::string& host1 = stringify(server1.endpoint());
        const std::string& host2 = stringify(server2.endpoint());
        const std::string& host3 = stringify(server3.endpoint());
        const int QueryCount = 99;
        server1.loop(QueryCount);
        server2.loop(QueryCount);
        server3.loop(QueryCount);
        asio::thread_pool pool;
        for (int i = 0; i < QueryCount; ++i) {
            asio::post(pool, [&] { client.query(host1); });
            asio::post(pool, [&] { client.query(host2); });
            asio::post(pool, [&] { client.query(host3); });
        }
        pool.join();
        CHECK(serverTracer1.wait(QueryCount * 2) == QueryCount * 2);
        CHECK(serverTracer2.wait(QueryCount * 2) == QueryCount * 2);
        CHECK(serverTracer3.wait(QueryCount * 2) == QueryCount * 2);
        CHECK(clientTracer.wait(QueryCount * 3) == QueryCount * 3);
        CHECK(clientTracer.find([&](const std::string& name, const std::string& address, Client::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == host1 && address == host1 && status == Client::Status::Succeeded && isClientPacket(packet) && rtt < seconds(1);
        }) == QueryCount);
        CHECK(clientTracer.find([&](const std::string& name, const std::string& address, Client::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == host2 && address == host2 && status == Client::Status::Succeeded && isClientPacket(packet) && rtt < seconds(1);
        }) == QueryCount);
        CHECK(clientTracer.find([&](const std::string& name, const std::string& address, Client::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == host3 && address == host3 && status == Client::Status::Succeeded && isClientPacket(packet) && rtt < seconds(1);
        }) == QueryCount);
    }

    TEST_CASE_FIXTURE(Context, "cancel queries" * doctest::timeout(1))
    {
        const std::string& host = stringify(server1.endpoint());
        Client client(clientTracer.callable());
        client.cancel();
        server1.replay(nullptr, 0, milliseconds(100));
        client.query(host);
        CHECK(serverTracer1.wait(1) == 1);
        client.cancel();
        client.query("255.255.255.255");
        CHECK(clientTracer.wait(2) == 2);
        CHECK(clientTracer.find([&](const std::string& name, const std::string& address, Client::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == host && address == "" && status == Client::Status::Cancelled && packet.isNull() && rtt == seconds(0);
        }) == 1);
        CHECK(clientTracer.find([&](const std::string& name, const std::string& address, Client::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == "255.255.255.255" && address == "255.255.255.255:123" && status == Client::Status::SendError && isClientPacket(packet) && rtt < seconds(1);
        }) == 1);
        client.cancel();
        client.cancel();
        server1.replay();
        client.query(host);
        CHECK(clientTracer.wait(3) == 3);
        CHECK(clientTracer.find([&](const std::string& name, const std::string& address, Client::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == host && address == host && status == Client::Status::Succeeded && isClientPacket(packet) && rtt < seconds(1);
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "cancel queries concurrently" * doctest::timeout(5))
    {
        std::vector<Server*> serverList { &server1, &server2, &server3, &server4, &server5 };
        const size_t ServerCount = serverList.size();
        const size_t CancelCount = 99;
        Client client(clientTracer.callable());
        asio::thread_pool pool;
        for (size_t j = 0; j < CancelCount; ++j) {
            for (size_t i = 0; i < ServerCount; ++i) {
                serverList.at(i)->replay();
                client.query(stringify(serverList.at(i)->endpoint()), milliseconds(i));
                asio::post(pool, [&] { client.cancel(); });
            }
            CHECK(clientTracer.wait(ServerCount) == ServerCount);
            clientTracer.reset();
        }
    }

    TEST_CASE_FIXTURE(Context, "wait all queries upon destruction" * doctest::timeout(11))
    {
        server1.replay(nullptr, 0, milliseconds(50));
        const std::string& host = stringify(server1.endpoint());
        Client(clientTracer.callable()).query(host);
        CHECK(clientTracer.wait() == 1);
        CHECK(clientTracer.find([&](const std::string& name, const std::string& address, Client::Status status, const Packet& packet, const steady_clock::duration& rtt) {
            return name == host && address == host && status == Query::Status::Succeeded && isClientPacket(packet) && rtt < milliseconds(100);
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "time out" * doctest::timeout(1))
    {
        const std::string& host = stringify(server1.endpoint());
        for (int i = 0; i < 3; ++i) {
            const auto& start = steady_clock::now();
            server1.receive();
            Client client(clientTracer.callable());
            client.query(host, milliseconds(i * 100));
            CHECK(clientTracer.wait() == 1);
            CHECK(clientTracer.find([&](const std::string& name, const std::string& address, Query::Status status, const Packet& packet, const steady_clock::duration& rtt) {
                return name == host && address == "" && status == Query::Status::TimeoutError && packet.isNull() && rtt == seconds(0);
            }) == 1);
            CHECK(abs(duration_cast<milliseconds>(steady_clock::now() - start).count() - i * 100) < 50);
            clientTracer.reset();
        }
    }
} // TEST_SUITE
