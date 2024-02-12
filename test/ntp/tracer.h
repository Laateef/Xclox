/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#include "tools/tracer.hpp"

#include "tools/helper.hpp"

using namespace std::chrono;

TEST_SUITE("Tracer")
{
    TEST_CASE("counter is initialized to zero on construction")
    {
        Tracer<> t;
        CHECK(t.counter() == 0);
    }

    TEST_CASE("counter increases on each call")
    {
        Tracer<> t;
        t.callable()();
        CHECK(t.counter() == 1);
        t.callable()();
        CHECK(t.counter() == 2);
    }

    TEST_CASE("counter is not shared among instances")
    {
        Tracer<> t1;
        Tracer<> t2;
        t1.callable()();
        CHECK(t1.counter() == 1);
        CHECK(t2.counter() == 0);
    }

    TEST_CASE("wait returns as soon as it finds a match")
    {
        const auto& start = steady_clock::now();
        SUBCASE("immediately")
        {
            Tracer<> t;
            t.callable()();
            CHECK(t.wait() == 1);
            CHECK(compare(start, milliseconds(1)));
        }
        SUBCASE("after a while")
        {
            Tracer<> t;
            std::thread([&] {
                std::this_thread::sleep_for(milliseconds(100));
                t.callable()();
            }).detach();
            CHECK(t.wait() == 1);
            CHECK(compare(start, milliseconds(100)));
        }
    }

    TEST_CASE("wait gives up after 1 second")
    {
        const auto& start = steady_clock::now();
        Tracer<> t;
        std::thread([&] {
            std::this_thread::sleep_for(milliseconds(1100));
            t.callable()();
        }).detach();
        CHECK(t.wait() == 0);
        CHECK(compare(start, milliseconds(1000)));
        CHECK(t.wait() == 1);
        CHECK(compare(start, milliseconds(1100)));
    }

    TEST_CASE("wait for multiple calls")
    {
        const auto& start = steady_clock::now();
        Tracer<> t;
        std::thread([&] {
            std::this_thread::sleep_for(milliseconds(100));
            t.callable()();
            std::this_thread::sleep_for(milliseconds(100));
            t.callable()();
            std::this_thread::sleep_for(milliseconds(100));
            t.callable()();
        }).detach();
        CHECK(t.wait(3) == 3);
        CHECK(compare(start, milliseconds(300)));
    }

    TEST_CASE("wait returns as soon as it finds bigger counts")
    {
        const auto& start = steady_clock::now();
        Tracer<> t;
        std::thread([&] {
            t.callable()();
            t.callable()();
            t.callable()();
        }).detach();
        std::this_thread::sleep_for(milliseconds(100));
        CHECK(t.wait(2) == 3);
        CHECK(compare(start, milliseconds(100)));
    }

    TEST_CASE("wait up to a given duration")
    {
        const auto& start = steady_clock::now();
        Tracer<> t;
        t.callable()();
        CHECK(t.wait(1, seconds(0)) == 1);
        CHECK(compare(start, milliseconds(1)));
        std::thread([&] {
            std::this_thread::sleep_for(milliseconds(200));
            t.callable()();
            std::this_thread::sleep_for(milliseconds(300));
            t.callable()();
        }).detach();
        CHECK(t.wait(2, milliseconds(50)) == 1);
        CHECK(compare(start, milliseconds(50)));
        CHECK(t.wait(3, milliseconds(200)) == 2);
        CHECK(compare(start, milliseconds(250)));
        CHECK(t.wait(3, milliseconds(300)) == 3);
        CHECK(compare(start, milliseconds(500)));
    }

    TEST_CASE("parameterizable callbacks")
    {
        Tracer<int> t1;
        t1.callable()(1);
        CHECK(t1.counter() == 1);
        Tracer<float, int> t2;
        t2.callable()(1.2f, 5);
        CHECK(t2.counter() == 1);
        Tracer<std::string, double, int> t3;
        t3.callable()("abc", 2.5, 9);
        CHECK(t3.counter() == 1);
    }

    TEST_CASE("search call history")
    {
        Tracer<bool> t;
        CHECK(t.find(true) == 0);
        CHECK(t.find(false) == 0);
        t.callable()(true);
        CHECK(t.find(true) == 1);
        CHECK(t.find(false) == 0);
        t.callable()(false);
        CHECK(t.find(true) == 1);
        CHECK(t.find(false) == 1);
    }

    TEST_CASE("count specific calls")
    {
        Tracer<int, std::string> t;
        CHECK(t.counter() == 0);
        t.callable()(1, "abc");
        t.callable()(2, "def");
        CHECK(t.counter() == 2);
        CHECK(t.find(1, "abc") == 1);
        CHECK(t.find(2, "def") == 1);
        t.callable()(1, "abc");
        t.callable()(2, "def");
        CHECK(t.counter() == 4);
        CHECK(t.find(1, "abc") == 2);
        CHECK(t.find(2, "def") == 2);
    }

    TEST_CASE("search using a custom finder")
    {
        Tracer<int, std::string, bool> t;
        t.callable()(1, "abc", false);
        t.callable()(2, "abc", true);
        t.callable()(3, "def", false);
        t.callable()(3, "def", false);
        CHECK(t.find([](int i, const std::string&, bool) { return i == 0; }) == 0);
        CHECK(t.find([](int, const std::string& s, bool) { return s == ""; }) == 0);
        CHECK(t.find([](int i, const std::string&, bool) { return i == 1; }) == 1);
        CHECK(t.find([](int, const std::string& s, bool) { return s == "abc"; }) == 2);
        CHECK(t.find([](int i, const std::string& s, bool) { return i == 3 && s == "def"; }) == 2);
        CHECK(t.find([](int, const std::string&, bool b) { return b == false; }) == 3);
        CHECK(t.find([](int i, const std::string&, bool) { return i == 1 || i == 2 || i == 3; }) == 4);
    }

    TEST_CASE("concurrent calls")
    {
        const size_t count = 9999;
        const size_t multiplier = 9;
        Tracer<> t;
        for (size_t i = 0; i < multiplier; ++i) {
            std::thread([&] {
                for (size_t i = 0; i < count; ++i) {
                    t.callable()();
                }
            }).detach();
        }
        CHECK(t.wait(count * multiplier, seconds(3)) == count * multiplier);
    }

    TEST_CASE("resetable")
    {
        Tracer<bool> t;
        t.callable()(true);
        t.callable()(false);
        CHECK(t.counter() == 2);
        CHECK(t.find(true) == 1);
        CHECK(t.find(false) == 1);
        t.reset();
        CHECK(t.counter() == 0);
        CHECK(t.find(true) == 0);
        CHECK(t.find(false) == 0);
    }
} // TEST_SUITE
