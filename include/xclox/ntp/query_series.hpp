/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_COMPOUND_QUERY_HPP
#define XCLOX_COMPOUND_QUERY_HPP

#include "query_single.hpp"

namespace xclox {

namespace ntp {

    /**
     * @class QuerySeries
     *
     * QuerySeries is an ephemeral class representing a series of NTP queries.
     *
     * @see The unit tests in @ref query_series.h for further details.
     */
    class QuerySeries {
    public:
        /**
         * @name Aliases
         * @{
         */

        using Callback = QuerySingle::Callback; ///< Type of query callback.
        using DefaultTimeout = internal::DefaultTimeout<QuerySingle, 5000>; ///< Type of query timeout milliseconds holder.

        /// @}

        /// Constructs a NTP query series on the given context that targets the given endpoints one at a time until success and runs within the given timeout duration.
        explicit QuerySeries(asio::io_context& io, const asio::ip::udp::resolver::results_type& endpoints, const std::chrono::milliseconds& timeout)
            : m_io(io)
            , m_endpoints(endpoints)
            , m_timer(io, timeout)
        {
        }

        /**
         * Starts querying the given endpoints one at a time until success or all endpoints are queried.
         * @param io a run context on which the operations of the query are executed.
         * @param endpoints a server address list to be queried.
         * @param callback a callable to report the result of the query to the caller.
         * @param timeout a time duration after which the query is cancelled if it is not completed.
         * @return a weak reference to the query that helps in tracing it.
         */
        static std::weak_ptr<QuerySeries> start(asio::io_context& io, const asio::ip::udp::resolver::results_type& endpoints, Callback callback, const std::chrono::milliseconds& timeout = std::chrono::milliseconds(DefaultTimeout::ms))
        {
            if (!callback || endpoints.empty()) {
                return {};
            }
            auto query = std::make_shared<QuerySeries>(io, endpoints, timeout);
            query->m_timer.async_wait([query](const asio::error_code& error) {
                if (error != asio::error::operation_aborted) {
                    query->m_timer.expires_at(std::chrono::steady_clock::time_point::min());
                    if (auto currentQuery = query->m_subquery.lock())
                        currentQuery->cancel();
                }
            });
            query->m_callback = [query, callback](const asio::ip::udp::endpoint& endpoint, const asio::error_code& error, const Packet& packet, const std::chrono::steady_clock::duration& rtt) {
                if (error && error != asio::error::operation_aborted && query->index(endpoint) < query->m_endpoints.size() - 1) {
                    query->m_subquery = QuerySingle::start(query->m_io, std::next(query->m_endpoints.cbegin(), query->index(endpoint) + 1)->endpoint(), query->m_callback);
                } else {
                    query->m_timer.cancel();
                    query->m_callback = {};
                    callback(
                        endpoint,
                        query->m_timer.expiry() == std::chrono::steady_clock::time_point::max() ? asio::error::operation_aborted : (query->m_timer.expiry() == std::chrono::steady_clock::time_point::min() ? asio::error::timed_out : error),
                        packet,
                        rtt);
                }
            };
            query->m_subquery = QuerySingle::start(io, endpoints.cbegin()->endpoint(), query->m_callback);
            return query;
        }

        /// Cancels the query reporting asio::error::operation_aborted to the caller.
        void cancel()
        {
            m_timer.expires_at(std::chrono::steady_clock::time_point::max());
            if (auto query = m_subquery.lock())
                query->cancel();
        }

    private:
        size_t index(const asio::ip::udp::endpoint& endpoint) const
        {
            return std::distance(m_endpoints.cbegin(), std::find_if(m_endpoints.cbegin(), m_endpoints.cend(), [&endpoint](const asio::ip::basic_resolver_entry<asio::ip::udp>& entry) { return entry.endpoint() == endpoint; }));
        }

        asio::io_context& m_io;
        asio::ip::udp::resolver::results_type m_endpoints;
        asio::steady_timer m_timer;
        Callback m_callback;
        std::weak_ptr<QuerySingle> m_subquery;
    };

} // namespace ntp

} // namespace xclox

#endif // XCLOX_COMPOUND_QUERY_HPP
