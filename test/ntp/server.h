/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#include "tools/server.hpp"

#include "tools/tracer.hpp"

#include "tools/helper.hpp"

using namespace std::chrono;

TEST_SUITE("Server")
{
    struct Context {
        Context()
            : server(32101, tracer.callable())
            , socket(io, asio::ip::udp::endpoint(asio::ip::address_v4::loopback(), 0))
        {
        }

        Tracer<asio::ip::udp::endpoint, asio::error_code, const uint8_t*, size_t> tracer;
        Server server;
        asio::io_context io;
        asio::ip::udp::socket socket;
    };

    TEST_CASE("parameterizable on port number")
    {
        Server server1(12345, {});
        CHECK(server1.endpoint() == asio::ip::udp::endpoint(asio::ip::address_v4::loopback(), 12345));
        Server server2(54321, {});
        CHECK(server2.endpoint() == asio::ip::udp::endpoint(asio::ip::address_v4::loopback(), 54321));
    }

    TEST_CASE_FIXTURE(Context, "receive zero bytes")
    {
        server.receive();
        uint8_t* sendData = nullptr;
        CHECK(socket.send_to(asio::buffer(sendData, 0), server.endpoint()) == 0);
        CHECK(tracer.wait() == 1);
        CHECK(tracer.find([&](const asio::ip::udp::endpoint& endpoint,
                              const asio::error_code& error,
                              const uint8_t* data,
                              size_t size) {
            return endpoint.port() == socket.local_endpoint().port() && !error && data == nullptr && size == 0;
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "receive multiple bytes")
    {
        server.receive();
        std::array<uint8_t, 99> sendData;
        for (uint8_t i = 0; i < sendData.size(); ++i) {
            sendData[i] = i;
        }
        CHECK(socket.send_to(asio::buffer(sendData), server.endpoint()) == sendData.size());
        CHECK(tracer.wait() == 1);
        CHECK(tracer.find([&](const asio::ip::udp::endpoint& endpoint,
                              const asio::error_code& error,
                              const uint8_t* data,
                              size_t size) {
            return socket.local_endpoint().port() == endpoint.port() && !error && std::memcmp(data, sendData.data(), size) == 0 && size == sendData.size();
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "receive times out")
    {
        for (size_t i = 1; i < 5; ++i) {
            const auto& timeout = milliseconds(i * 100);
            const auto& testCaseStart = steady_clock::now();
            server.receive(timeout);
            CHECK(tracer.wait(1, milliseconds(timeout + milliseconds(50))) == 1);
            CHECK(steady_clock::now() - testCaseStart < timeout + milliseconds(100));
            CHECK(tracer.find([&](const asio::ip::udp::endpoint& endpoint,
                                  const asio::error_code& error,
                                  const uint8_t* data,
                                  size_t size) {
                return endpoint == asio::ip::udp::endpoint() && error == asio::error::operation_aborted && data == nullptr && size == 0;
            }) == 1);
            tracer.reset();
        }
    }

    TEST_CASE_FIXTURE(Context, "cancel operations upon destruction")
    {
        auto s = std::unique_ptr<Server>(new Server(uint16_t {}, tracer.callable()));
        s->receive();
        s.reset();
        CHECK(tracer.wait() == 1);
        CHECK(tracer.find([&](const asio::ip::udp::endpoint& endpoint,
                              const asio::error_code& error,
                              const uint8_t* data,
                              size_t size) {
            return endpoint == asio::ip::udp::endpoint() && error == asio::error::operation_aborted && data == nullptr && size == 0;
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "send zero bytes")
    {
        uint8_t recvData = 255;
        asio::ip::udp::endpoint senderEndpoint;
        asio::error_code recvError { asio::error::would_block };
        size_t recvSize {};
        socket.async_receive_from(
            asio::buffer(&recvData, 1),
            senderEndpoint,
            [&](const asio::error_code& error, size_t size) {
                recvError = error;
                recvSize = size;
            });
        uint8_t* sendData = nullptr;
        std::thread([&] { io.run(); }).detach();
        server.send(socket.local_endpoint(), sendData, 0);
        CHECK(tracer.wait() == 1);
        CHECK(tracer.find([&](const asio::ip::udp::endpoint& endpoint,
                              const asio::error_code& error,
                              const uint8_t* data,
                              size_t size) {
            return endpoint == socket.local_endpoint() && !error && data == nullptr && size == 0;
        }) == 1);
        CHECK(senderEndpoint.port() == server.endpoint().port());
        CHECK_FALSE(recvError);
        CHECK(recvSize == 0);
        CHECK(recvData == 255);
    }

    TEST_CASE_FIXTURE(Context, "send fails")
    {
        uint8_t sendData = 1;
        server.send(broadcastEndpoint, &sendData, 1);
        CHECK(tracer.wait() == 1);
        CHECK(tracer.find([&](const asio::ip::udp::endpoint& endpoint,
                              const asio::error_code& error,
                              const uint8_t* data,
                              size_t size) {
            return endpoint == broadcastEndpoint && error == asio::error::access_denied && data == &sendData && size == 0;
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "send multiple bytes")
    {
        std::array<uint8_t, 99> recvData;
        asio::ip::udp::endpoint senderEndpoint;
        asio::error_code recvError { asio::error::would_block };
        size_t recvSize {};
        socket.async_receive_from(
            asio::buffer(recvData),
            senderEndpoint,
            [&](const asio::error_code& error, std::size_t size) {
                recvError = error;
                recvSize = size;
            });
        std::thread([&] { io.run(); }).detach();
        std::array<uint8_t, 99> sendData;
        for (uint8_t i = 0; i < sendData.size(); ++i) {
            sendData[i] = i;
        }
        server.send(socket.local_endpoint(), sendData.data(), sendData.size());
        CHECK(tracer.wait() == 1);
        CHECK(tracer.find([&](const asio::ip::udp::endpoint& endpoint,
                              const asio::error_code& error,
                              const uint8_t* data,
                              size_t size) {
            return endpoint == socket.local_endpoint() && !error && std::memcmp(data, sendData.data(), size) == 0 && size == sendData.size();
        }) == 1);
        CHECK(senderEndpoint.port() == server.endpoint().port());
        CHECK_FALSE(recvError);
        CHECK(recvSize == sendData.size());
        CHECK(recvData == sendData);
    }

    TEST_CASE_FIXTURE(Context, "cancel pending operation" * doctest::timeout(0.1))
    {
        std::array<uint8_t, 99> recvData;
        asio::ip::udp::endpoint senderEndpoint;
        socket.async_receive_from(
            asio::buffer(recvData),
            senderEndpoint,
            [&](const asio::error_code&, std::size_t) {});
        std::thread([&] { io.run(); }).detach();
        server.receive();
        std::array<uint8_t, 99> sendData;
        sendData.fill(1);
        server.send(socket.local_endpoint(), sendData.data(), sendData.size());
        CHECK(tracer.wait(2) == 2);
        CHECK(tracer.find([&](const asio::ip::udp::endpoint& endpoint,
                              const asio::error_code& error,
                              const uint8_t* data,
                              size_t size) {
            return endpoint == asio::ip::udp::endpoint() && error == asio::error::operation_aborted && data == nullptr && size == 0;
        }) == 1);
        CHECK(tracer.find([&](const asio::ip::udp::endpoint& endpoint,
                              const asio::error_code& error,
                              const uint8_t* data,
                              size_t size) {
            return endpoint == socket.local_endpoint() && !error && std::memcmp(data, sendData.data(), size) == 0 && size == sendData.size();
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "replay incoming data")
    {
        std::array<uint8_t, 64> recvData {};
        std::array<uint8_t, 64> sendData;
        for (uint8_t i = 0; i < sendData.size(); ++i) {
            sendData[i] = i;
        }
        size_t ByteCount = sendData.size();
        server.replay();
        socket.async_send_to(
            asio::buffer(sendData),
            server.endpoint(), [ByteCount](const asio::error_code& error, std::size_t size) {
                CHECK_FALSE(error);
                CHECK(size == ByteCount);
            });
        asio::ip::udp::endpoint senderEndpoint;
        socket.async_receive_from(
            asio::buffer(recvData),
            senderEndpoint, [ByteCount](const asio::error_code& error, std::size_t size) {
                CHECK_FALSE(error);
                CHECK(size == ByteCount);
            });
        CHECK(io.run() == 2);
        CHECK(senderEndpoint == server.endpoint());
        CHECK(recvData == sendData);
        CHECK(tracer.wait(2) == 2);
        CHECK(tracer.find([&](const asio::ip::udp::endpoint& endpoint,
                              const asio::error_code& error,
                              const uint8_t* data,
                              size_t size) {
            return endpoint == socket.local_endpoint() && !error && std::memcmp(data, sendData.data(), size) == 0 && size == sendData.size();
        }) == 2);
    }

    TEST_CASE_FIXTURE(Context, "replay custom data with 100ms delay")
    {
        steady_clock::time_point recvTime;
        steady_clock::time_point sendTime;
        std::array<uint8_t, 99> recvData {};
        std::array<uint8_t, 64> sendData {};
        std::array<uint8_t, 99> custData;
        for (uint8_t i = 0; i < custData.size(); ++i) {
            custData[i] = i;
        }
        size_t CustByteCount = custData.size();
        size_t SendByteCount = sendData.size();
        server.replay(custData.data(), custData.size(), milliseconds(100));
        asio::ip::udp::endpoint senderEndpoint;
        socket.async_receive_from(
            asio::buffer(recvData),
            senderEndpoint, [CustByteCount, &recvTime](const asio::error_code& error, std::size_t size) {
                recvTime = steady_clock::now();
                CHECK_FALSE(error);
                CHECK(size == CustByteCount);
            });
        socket.async_send_to(
            asio::buffer(sendData),
            server.endpoint(), [SendByteCount, &sendTime](const asio::error_code& error, std::size_t size) {
                sendTime = steady_clock::now();
                CHECK_FALSE(error);
                CHECK(size == SendByteCount);
            });
        CHECK(io.run() == 2);
        CHECK(compare(recvTime - sendTime, milliseconds(100)));
        CHECK(senderEndpoint.port() == server.endpoint().port());
        CHECK(recvData == custData);
        CHECK(tracer.wait(2) == 2);
        CHECK(tracer.find([&](const asio::ip::udp::endpoint& endpoint,
                              const asio::error_code& error,
                              const uint8_t* data,
                              size_t size) {
            return endpoint == socket.local_endpoint() && !error && std::memcmp(data, sendData.data(), sendData.size()) == 0 && size == sendData.size();
        }) == 1);
        CHECK(tracer.find([&](const asio::ip::udp::endpoint& endpoint,
                              const asio::error_code& error,
                              const uint8_t* data,
                              size_t size) {
            return endpoint == socket.local_endpoint() && !error && std::memcmp(data, custData.data(), custData.size()) == 0 && size == custData.size();
        }) == 1);
    }

    TEST_CASE_FIXTURE(Context, "replay resets upon each call")
    {
        const size_t Count = 9;
        for (size_t i = 0; i < Count; ++i) {
            server.replay();
        }
        CHECK(tracer.wait(Count - 1) == Count - 1);
        CHECK(tracer.find([&](const asio::ip::udp::endpoint& endpoint,
                              const asio::error_code& error,
                              const uint8_t* data,
                              size_t size) {
            return endpoint == asio::ip::udp::endpoint() && error == asio::error::operation_aborted && data == nullptr && size == 0;
        }) == Count - 1);
        std::array<uint8_t, 9> recvData;
        asio::ip::udp::endpoint senderEndpoint;
        socket.async_receive_from(asio::buffer(recvData), senderEndpoint, [](const asio::error_code&, std::size_t) {});
        std::array<uint8_t, 9> sendData;
        sendData.fill(9);
        socket.async_send_to(asio::buffer(sendData), server.endpoint(), [](const asio::error_code&, std::size_t) {});
        CHECK(io.run() == 2);
        CHECK(senderEndpoint == server.endpoint());
        CHECK(recvData == sendData);
        CHECK(tracer.wait(Count + 1) == Count + 1);
        CHECK(tracer.find([&](const asio::ip::udp::endpoint& endpoint,
                              const asio::error_code& error,
                              const uint8_t* data,
                              size_t size) {
            return endpoint == socket.local_endpoint() && !error && std::memcmp(data, sendData.data(), sendData.size()) == 0 && size == sendData.size();
        }) == 2);
    }

    TEST_CASE_FIXTURE(Context, "loop")
    {
        const size_t ByteCount = 9;
        const size_t QueryCount = 99;
        const size_t HandlerCount = QueryCount * 2;
        std::array<std::array<uint8_t, ByteCount>, QueryCount> recvArrayList {};
        std::array<asio::ip::udp::endpoint, QueryCount> senderEndpointArrayList {};
        std::array<uint8_t, ByteCount> sendArray;
        for (uint8_t i = 0; i < sendArray.size(); ++i) {
            sendArray[i] = i;
        }
        server.loop(QueryCount);
        for (size_t i = 0; i < QueryCount; ++i) {
            auto s = std::make_shared<asio::ip::udp::socket>(io, asio::ip::udp::endpoint(asio::ip::address_v4::loopback(), 0));
            auto counterHandler = [ByteCount, s](const asio::error_code& error, std::size_t size) {
                CHECK_FALSE(error);
                CHECK(size == ByteCount);
            };
            s->async_send_to(asio::buffer(sendArray), server.endpoint(), counterHandler);
            s->async_receive_from(asio::buffer(recvArrayList[i]), senderEndpointArrayList[i], counterHandler);
        }
        CHECK(io.run() == HandlerCount);
        CHECK(tracer.wait(HandlerCount) == HandlerCount);
        CHECK(tracer.find([&](const asio::ip::udp::endpoint& endpoint,
                              const asio::error_code& error,
                              const uint8_t* data,
                              size_t size) {
            return endpoint.address() == server.endpoint().address() && !error && std::memcmp(data, sendArray.data(), sendArray.size()) == 0 && size == sendArray.size();
        }) == HandlerCount);
    }
} // TEST_SUITE
