/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_CLIENT_HPP
#define XCLOX_CLIENT_HPP

#include "query.hpp"

#include <list>

namespace xclox {

namespace ntp {

    /**
     * @class Client
     *
     * Client is an asynchronous multi-query NTP client.
     *
     * Typically, a Client object is constructed by passing a callable of type Client::Callback to its constructor.
     * User NTP query requests can be placed via query().
     * Once the query is finished, the registered callable is called back with the following details:
     *   - NTP server name as it was provided by the caller
     *   - IP address if the server name is successfully resolved or an empty string otherwise
     *   - Client::Status flag indicating the final status of the query
     *   - Packet object representing the server's reply on success or a null packet otherwise
     *   - Elapsed time since sending the packet to the server
     *
     * A default-constructed Client ignores any queries made on it if there is no registered callback.
     * So, before issuing any query requests on such a Client, a callback has to be registered via setCallback().
     *
     * Client first tries to resolve the server name; if resolving fails, Client::Status::ResolveError is reported.
     * Otherwise, Client starts querying the resolved addresses one at a time until success or all addresses are queried.
     *
     * Client awaits all pending queries until completion upon destruction.
     * If you need to destruct a Client object as soon as possible, use cancel() to cancel all queries.
     *
     * By default, each placed query has a total timeout period of 5 seconds to complete.
     * If a query timed out, it is cancelled and reported to the caller with Client::Status::TimeoutError.
     *
     * @see The unit tests in @ref client.h for further details.
     */
    class Client {
    public:
        /**
         * @name Aliases
         * @{
         */

        using Callback = Query::Callback; ///< Type of query callback.
        using Status = Query::Status; ///< Type of query status.
        using DefaultTimeout = Query::DefaultTimeout; ///< Type of query timeout holder.

        /// @}

        /**
         * Default constructor.
         * @param callable is for reporting the result of each placed query back to the caller.
         */
        explicit Client(Callback callable)
            : m_callable(callable)
        {
        }

        /// Default destructor.
        ~Client()
        {
            m_pool.join();
        }

        /**
         * Place a NTP query [thread-safe].
         * @param server is a domain name or an IP address, optionally along with a custom port number in the form "host[:port]". The default port is "123".
         * @param timeout is the total time after which the query is cancelled if it is not completed.
         */
        void query(const std::string& server, const std::chrono::milliseconds& timeout = std::chrono::milliseconds(DefaultTimeout::ms))
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            purgeQueryList();
            m_queryList.push_back(Query::start(m_pool, server, m_callable, timeout));
        }

        /// Register a callable for reporting the result of the query back to the caller.
        void setCallback(Callback callable)
        {
            m_callable = std::move(callable);
        }

        /// Cancel all current queries [thread-safe].
        void cancel()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto query : m_queryList) {
                if (auto shared = query.lock()) {
                    shared->cancel();
                }
            }
            purgeQueryList();
        }

    private:
        void purgeQueryList()
        {
            m_queryList.remove_if(std::mem_fn(&std::weak_ptr<Query>::expired));
        }

        Callback m_callable;
        asio::thread_pool m_pool;
        std::mutex m_mutex;
        std::list<std::weak_ptr<Query>> m_queryList;
    };

} // namespace ntp

} // namespace xclox

#endif // XCLOX_CLIENT_HPP
