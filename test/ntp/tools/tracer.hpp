/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_TRACER_HPP
#define XCLOX_TRACER_HPP

#include <chrono>
#include <functional>
#include <thread>

namespace cpp11 {

template <size_t...>
struct IndexSequence {
};

template <size_t N, size_t... Next>
struct IndexSequenceHelper : public IndexSequenceHelper<N - 1U, N - 1U, Next...> {
};

template <size_t... Next>
struct IndexSequenceHelper<0U, Next...> {
    using type = IndexSequence<Next...>;
};

template <size_t N>
using MakeIndexSequence = typename IndexSequenceHelper<N>::type;

template <typename Function, typename Tuple, size_t... I>
bool apply(Function&& f, Tuple&& t, IndexSequence<I...>)
{
    return std::forward<Function>(f)(std::get<I>(std::forward<Tuple>(t))...);
}

template <typename Function, typename Tuple>
bool apply(Function&& f, Tuple&& t)
{
    return apply(std::forward<Function>(f), std::forward<Tuple>(t), MakeIndexSequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value> {});
}

} // namespace cpp11

template <typename... T>
class Tracer {
public:
    using CallType = std::function<void(const T&...)>;
    using PackType = std::tuple<T...>;

    Tracer()
    {
        m_callable = [this](const T&... args) {
            std::lock_guard<std::mutex> guard(m_mutex);
            m_callList.emplace_back(std::make_tuple(args...));
        };
    }

    Tracer(const Tracer<T...>&) = delete;
    Tracer(Tracer<T...>&&) = delete;
    ~Tracer() = default;

    Tracer<T...>& operator=(const Tracer<T...>&) = delete;
    Tracer<T...>& operator=(Tracer<T...>&&) = delete;

    CallType callable() const
    {
        return m_callable;
    }

    size_t counter() const
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        return m_callList.size();
    }

    size_t wait(size_t count = 1, std::chrono::milliseconds timeout = std::chrono::seconds(1)) const
    {
        const auto& end = std::chrono::steady_clock::now() + timeout;
        while (counter() < count && std::chrono::steady_clock::now() < end) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return counter();
    }

    size_t find(const T&... args) const
    {
        return static_cast<size_t>(std::count(m_callList.cbegin(), m_callList.cend(), std::make_tuple(args...)));
    }

    size_t find(std::function<bool(T...)> finder) const
    {
        auto wrapper = [&](const PackType& pack) { return cpp11::apply(finder, pack); };
        return std::count_if(m_callList.cbegin(), m_callList.cend(), wrapper);
    }

    void reset()
    {
        m_callList.clear();
    }

private:
    std::vector<PackType> m_callList;
    mutable std::mutex m_mutex;
    CallType m_callable;
};

#endif // XCLOX_TRACER_HPP
