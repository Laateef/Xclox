/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_SERVER_HPP
#define XCLOX_SERVER_HPP

#define ASIO_NO_DEPRECATED
#include <asio.hpp>

#include <thread>

class Server {
public:
    using Callback = std::function<void(const asio::ip::udp::endpoint&, const asio::error_code&, const uint8_t*, size_t)>;

    Server(uint16_t port, Callback callable)
        : m_port(port)
        , m_callback(callable)
        , m_guard(asio::make_work_guard(m_io))
        , m_timer(m_io)
        , m_size()
        , m_thread(std::thread([this]() { m_io.run(); }))
    {
    }

    ~Server()
    {
        m_guard.reset();
        m_timer.cancel();
        close();
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    asio::ip::udp::endpoint endpoint() const
    {
        return asio::ip::udp::endpoint(asio::ip::address_v4().loopback(), m_port);
    }

    void send(const asio::ip::udp::endpoint& destination, const uint8_t* data, size_t size)
    {
        reset();
        startSending(destination, data, size);
    }

    void receive(const std::chrono::milliseconds& timeout = std::chrono::seconds(1))
    {
        reset();
        watch(timeout);
        startReceiving();
    }

    void replay(const uint8_t* data = nullptr, size_t size = 0, const std::chrono::milliseconds& delay = std::chrono::seconds(0))
    {
        reset();
        startLooping(1, data, size, delay);
    }

    void loop(size_t count, const uint8_t* data = nullptr, size_t size = 0)
    {
        reset();
        startLooping(count, data, size);
    }

private:
    void startReceiving(std::function<void()> postCallback = {})
    {
        m_socket->async_receive_from(
            asio::buffer(m_buffer),
            m_endpoint,
            [this, postCallback](const asio::error_code& error, size_t bytes) {
                m_size = bytes;
                if (error && m_timer.expiry() == std::chrono::steady_clock::time_point::min()) {
                    m_callback(asio::ip::udp::endpoint(), error, nullptr, 0);
                    return;
                }
                m_timer.cancel();
                m_callback(m_endpoint, error, bytes ? m_buffer.data() : nullptr, bytes);
                if (postCallback) {
                    postCallback();
                }
            });
    }

    void startSending(const asio::ip::udp::endpoint& destination, const uint8_t* data, size_t size, std::function<void()> postCallback = {})
    {
        m_socket->async_send_to(
            asio::buffer(data, size),
            destination,
            [this, destination, data, postCallback](const asio::error_code& error, size_t bytes) {
                m_callback(destination, error, data, bytes);
                if (postCallback) {
                    postCallback();
                }
            });
    }

    void startLooping(size_t count, const uint8_t* data = nullptr, size_t size = 0, const std::chrono::milliseconds& delay = std::chrono::seconds(0))
    {
        startReceiving([this, count, data, size, delay] {
            std::this_thread::sleep_for(delay);
            startSending(m_endpoint, data ? data : m_buffer.data(), data ? size : m_size, [this, count] {
                if (count > 1) {
                    startLooping(count - 1);
                }
            });
        });
    }

    void watch(const std::chrono::milliseconds& timeout)
    {
        m_timer.expires_after(timeout);
        m_timer.async_wait([this](const asio::error_code& error) {
            if (error != asio::error::operation_aborted) {
                m_timer.expires_at(std::chrono::steady_clock::time_point::min());
                close();
            } });
    }

    void close()
    {
        if (m_socket && m_socket->is_open()) {
            m_socket->close();
        }
    }

    void reset()
    {
        m_timer.cancel();
        close();
        m_socket = std::unique_ptr<asio::ip::udp::socket>(new asio::ip::udp::socket(m_io, asio::ip::udp::endpoint(asio::ip::address_v4::loopback(), m_port)));
    }

    uint16_t m_port;
    Callback m_callback;
    asio::io_context m_io;
    asio::executor_work_guard<asio::io_context::executor_type> m_guard;
    asio::steady_timer m_timer;
    std::unique_ptr<asio::ip::udp::socket> m_socket;
    asio::ip::udp::endpoint m_endpoint;
    std::array<uint8_t, 99> m_buffer;
    size_t m_size;
    std::thread m_thread;
};

#endif // XCLOX_SERVER_HPP
