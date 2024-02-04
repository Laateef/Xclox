/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_SIMPLE_QUERY_HPP
#define XCLOX_SIMPLE_QUERY_HPP

#include "packet.hpp"

#define ASIO_NO_DEPRECATED
#include <asio.hpp>

#include <memory>

namespace xclox {

namespace ntp {

    namespace internal {

        /// @cond Doxygen_Suppress
        template <typename T, int N>
        struct DefaultTimeout {
            static const int ms = N;
        };
        template <typename T, int N>
        const int DefaultTimeout<T, N>::ms;
        /// @endcond
    }

    /**
     * @class QuerySingle
     *
     * QuerySingle is an ephemeral class representing a single NTP query.
     *
     * @see The unit tests in @ref query_single.h for further details.
     */
    class QuerySingle {
    public:
        /**
         * @name Aliases
         * @{
         */

        using Callback = std::function<void(const asio::ip::udp::endpoint&, const asio::error_code&, const Packet&, const std::chrono::steady_clock::duration&)>; ///< Type of query callback.
        using DefaultTimeout = internal::DefaultTimeout<QuerySingle, 3000>; ///< Type of query timeout milliseconds holder.

        /// @}

        /// Constructs a single NTP query on the given context that runs within the given timeout duration.
        explicit QuerySingle(asio::io_context& io, const std::chrono::milliseconds& timeout)
            : m_timer(io, timeout)
            , m_socket(io, asio::ip::udp::v4())
        {
        }

        /**
         * Starts querying the given endpoint.
         * @param io a run context on which the operations of the query are executed.
         * @param server a server address to be queried.
         * @param callback a callable to report the result of the query to the caller.
         * @param timeout a time duration after which the query is cancelled if it is not completed.
         * @return a weak reference to the query that helps in tracing it.
         */
        static std::weak_ptr<QuerySingle> start(asio::io_context& io, const asio::ip::udp::endpoint& server, Callback callback, const std::chrono::milliseconds& timeout = std::chrono::milliseconds(DefaultTimeout::ms))
        {
            if (!callback) {
                return {};
            }
            auto query = std::make_shared<QuerySingle>(io, timeout);
            query->m_timer.async_wait([query](const asio::error_code& error) {
                if (error != asio::error::operation_aborted) {
                    query->m_timer.expires_at(std::chrono::steady_clock::time_point::min());
                    query->m_socket.close();
                }
            });
            Packet packet(0, 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, Timestamp(std::chrono::system_clock::now()).value());
            const auto& time = std::chrono::steady_clock::now();
            query->m_socket.async_send_to(
                asio::buffer(packet.data()),
                server,
                [query, server, callback, packet, time](const asio::error_code& error, std::size_t) {
                    if (error) {
                        query->m_timer.cancel();
                        callback(server, error, packet, std::chrono::steady_clock::now() - time);
                        return;
                    }
                    query->m_socket.async_receive_from(
                        asio::buffer(query->m_buffer),
                        query->m_endpoint,
                        [query, server, callback, time](const asio::error_code& error, std::size_t size) {
                            query->m_timer.cancel();
                            if (error || size != query->m_buffer.size() || query->m_timer.expiry() == std::chrono::steady_clock::time_point::max()) {
                                callback(server,
                                    query->m_timer.expiry() == std::chrono::steady_clock::time_point::max() ? asio::error::operation_aborted : (query->m_timer.expiry() == std::chrono::steady_clock::time_point::min() ? asio::error::timed_out : (error ? error : asio::error::message_size)),
                                    Packet(),
                                    std::chrono::steady_clock::now() - time);
                            } else {
                                callback(server, error, Packet(query->m_buffer), std::chrono::steady_clock::now() - time);
                            }
                        });
                });
            return query;
        }

        /// Cancels the query reporting asio::error::operation_aborted to the caller.
        void cancel()
        {
            m_timer.expires_at(std::chrono::steady_clock::time_point::max());
            m_socket.close();
        }

    private:
        void close()
        {
            if (m_socket.is_open()) {
                asio::error_code ec;
                m_socket.close(ec);
            }
        }

        asio::steady_timer m_timer;
        asio::ip::udp::socket m_socket;
        asio::ip::udp::endpoint m_endpoint;
        Packet::DataType m_buffer;
    };

} // namespace ntp

} // namespace xclox

#endif // XCLOX_SIMPLE_QUERY_HPP
