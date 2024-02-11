/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_QUERY_HPP
#define XCLOX_QUERY_HPP

#include "query_series.hpp"

namespace xclox {

namespace ntp {

    namespace internal {

        std::string getHost(const std::string& server)
        {
            return server.find(':') == std::string::npos ? server : server.substr(0, server.find(':'));
        }

        std::string getPort(const std::string& server)
        {
            return server.find(':') == std::string::npos ? "123" : server.substr(server.find(':') + 1);
        }

        auto stringify = [](const asio::ip::udp::endpoint& endpoint) {
            std::stringstream ss;
            ss << endpoint;
            return ss.str();
        };

    } // namespace internal

    /**
     * @class Query
     *
     * Query is an ephemeral class representing a NTP query from start to end.
     *
     * @see The unit tests in @ref query.h for further details.
     */
    class Query : public std::enable_shared_from_this<Query> {
    public:
        /**
         * @name Enumerations & Aliases
         * @{
         */

        /**
         * @enum Status
         * Type of query status.
         */
        enum class Status : uint8_t {
            ResolveError = 1, ///< The server domain name is not resolved.
            SendError = 2, ///< The client packet is not sent to the server.
            ReceiveError = 4, ///< The server packet is not received by the client.
            TimeoutError = 8, ///< The query timed out while waiting for the server's packet.
            Cancelled = 16, ///< The client cancelled the query.
            Succeeded = 32 ///< The client received the server's packet successfully.
        };

        using Callback = std::function<void(const std::string&, const std::string&, Status, const Packet&, const std::chrono::steady_clock::duration&)>; ///< Type of query callback.
        using DefaultTimeout = internal::DefaultTimeout<QuerySingle, 5000>; ///< Type of query timeout milliseconds holder.

        /// @}

        /// Constructs a NTP query that targets \p server and uses \p callback for reporting back its result.
        explicit Query(const std::string& server, Callback callback)
            : m_server(server)
            , m_callback(callback)
            , m_timer(m_io)
            , m_resolver(m_io)
            , m_finalized(false)
        {
        }

        /**
         * Starts querying all resolved addresses of \p server one at a time until success.
         * @param pool a thread pool on which the operations of the query are executed.
         * @param server a server domain name or address to be resolved for querying.
         * @param callback a callable to report the result of the query to the caller.
         * @param timeout a time duration after which the query is cancelled if it is not completed.
         * @return a weak reference to the query that helps in tracing it.
         */
        static std::weak_ptr<Query> start(asio::thread_pool& pool, const std::string& server, Callback callback, const std::chrono::milliseconds& timeout = std::chrono::milliseconds(DefaultTimeout::ms))
        {
            if (!callback) {
                return {};
            }
            auto query = std::make_shared<Query>(server, callback);
            query->m_timer.expires_after(timeout);
            query->m_timer.async_wait([self = std::weak_ptr<Query>(query->shared_from_this())](const asio::error_code& error) {
                if (error != asio::error::operation_aborted) {
                    if (auto query = self.lock()) {
                        query->m_io.stop();
                        query->finalize(query->m_server, "", Status::TimeoutError, Packet(), std::chrono::seconds(0));
                    }
                }
            });
            query->m_resolver.async_resolve(
                internal::getHost(server),
                internal::getPort(server),
                [query = std::weak_ptr<Query>(query->shared_from_this())](
                    const asio::error_code& error,
                    const asio::ip::udp::resolver::results_type& endpoints) {
                    auto self = query.lock();
                    if (!self) {
                        return;
                    }
                    if (error) {
                        self->m_timer.cancel();
                        self->finalize(self->m_server, "", Status::ResolveError, Packet(), std::chrono::seconds(0));
                        return;
                    }
                    self->m_subquery = QuerySeries::start(
                        self->m_io,
                        endpoints,
                        [query = std::weak_ptr<Query>(self->shared_from_this())](
                            const asio::ip::udp::endpoint& endpoint,
                            const asio::error_code& error,
                            const Packet& packet,
                            const std::chrono::steady_clock::duration& rtt) {
                            auto self = query.lock();
                            if (!self) {
                                return;
                            }
                            self->m_timer.cancel();
                            self->finalize(
                                self->m_server,
                                internal::stringify(endpoint),
                                error ? (packet.isNull() ? Status::ReceiveError : Status::SendError) : Status::Succeeded,
                                packet,
                                rtt);
                        });
                });
            asio::post(pool, [self = query] {
                self->m_io.run();
            });
            return query;
        }

        /// Cancels the query reporting Query::Status::Cancelled to the caller.
        void cancel()
        {
            asio::dispatch(m_io, [query = std::weak_ptr<Query>(shared_from_this())] {
                auto self = query.lock();
                if (self && !self->m_io.stopped()) {
                    self->m_io.stop();
                    self->finalize(self->m_server, "", Status::Cancelled, Packet(), std::chrono::seconds(0));
                }
            });
        }

    private:
        void finalize(const std::string& name, const std::string& address, Status status, const Packet& packet, const std::chrono::steady_clock::duration& rtt)
        {
            if (!m_finalized) {
                m_callback(name, address, status, packet, rtt);
                m_finalized = true;
            }
        }

        std::string m_server;
        Callback m_callback;
        asio::io_context m_io;
        asio::steady_timer m_timer;
        asio::ip::udp::resolver m_resolver;
        std::weak_ptr<QuerySeries> m_subquery;
        bool m_finalized;
    };

} // namespace ntp

} // namespace xclox

#endif // XCLOX_QUERY_HPP
