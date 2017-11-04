// Copyright (c) 2017-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Util/RingDeque.hpp>
#include <string>
#include <iostream>

POTHOS_TEST_BLOCK("/util/tests", test_ring_deque)
{
    Pothos::Util::RingDeque<std::string> ring0;
    POTHOS_TEST_EQUAL(ring0.capacity(), 1);
    POTHOS_TEST_TRUE(ring0.empty());
    POTHOS_TEST_TRUE(not ring0.full());

    ring0.set_capacity(10);
    POTHOS_TEST_EQUAL(ring0.capacity(), 10);
    POTHOS_TEST_TRUE(ring0.empty());
    POTHOS_TEST_TRUE(not ring0.full());

    //fill with elements
    for (size_t i = 0; i < 10; i++)
    {
        POTHOS_TEST_EQUAL(ring0.size(), i);
        POTHOS_TEST_TRUE(not ring0.full());
        ring0.push_back(std::to_string(i));
        POTHOS_TEST_TRUE(not ring0.empty());
    }
    POTHOS_TEST_TRUE(ring0.full());
    POTHOS_TEST_EQUAL(ring0.size(), 10);

    //test indexing
    for (size_t i = 0; i < 10; i++)
    {
        POTHOS_TEST_EQUAL(ring0[i], std::to_string(i));
    }

    //pop in order
    for (size_t i = 0; i < 10; i++)
    {
        POTHOS_TEST_EQUAL(ring0.size(), 10-i);
        POTHOS_TEST_TRUE(not ring0.empty());
        POTHOS_TEST_EQUAL(ring0.front(), std::to_string(i));
        ring0.pop_front();
        POTHOS_TEST_TRUE(not ring0.full());
    }
    POTHOS_TEST_TRUE(ring0.empty());
    POTHOS_TEST_EQUAL(ring0.size(), 0);

    //fill with elements
    for (size_t i = 0; i < 10; i++)
    {
        POTHOS_TEST_TRUE(not ring0.full());
        ring0.push_back(std::to_string(i));
        POTHOS_TEST_TRUE(not ring0.empty());
    }
    POTHOS_TEST_TRUE(ring0.full());

    //pop in reverse
    for (int i = 9; i >= 0; i--)
    {
        POTHOS_TEST_EQUAL(ring0.size(), size_t(i+1));
        POTHOS_TEST_TRUE(not ring0.empty());
        POTHOS_TEST_EQUAL(ring0.back(), std::to_string(i));
        ring0.pop_back();
        POTHOS_TEST_TRUE(not ring0.full());
    }
    POTHOS_TEST_TRUE(ring0.empty());
    POTHOS_TEST_EQUAL(ring0.size(), 0);

    //fill with elements
    for (size_t i = 0; i < 10; i++)
    {
        POTHOS_TEST_EQUAL(ring0.size(), i);
        POTHOS_TEST_TRUE(not ring0.full());
        ring0.push_back(std::to_string(i));
        POTHOS_TEST_TRUE(not ring0.empty());
    }
    POTHOS_TEST_TRUE(ring0.full());
    POTHOS_TEST_EQUAL(ring0.size(), 10);

    //change capacity
    ring0.set_capacity(20);
    POTHOS_TEST_EQUAL(ring0.capacity(), 20);
    POTHOS_TEST_TRUE(not ring0.full());

    //test indexing
    for (size_t i = 0; i < 10; i++)
    {
        POTHOS_TEST_EQUAL(ring0[i], std::to_string(i));
    }

    ring0.clear();
    POTHOS_TEST_EQUAL(ring0.size(), 0);
    POTHOS_TEST_TRUE(ring0.empty());
    POTHOS_TEST_TRUE(not ring0.full());
}

POTHOS_TEST_BLOCK("/util/tests", test_ring_deque_copy)
{
    Pothos::Util::RingDeque<std::string> ring0(10);
    for (size_t i = 0; i < 10; i++)
    {
        ring0.push_back(std::to_string(i));
    }

    //test copy construct
    Pothos::Util::RingDeque<std::string> ring1(ring0);
    POTHOS_TEST_EQUAL(ring0.size(), ring1.size());
    for (size_t i = 0; i < 10; i++)
    {
        POTHOS_TEST_EQUAL(ring0[i], ring1[i]);
    }

    //test copy assignment
    Pothos::Util::RingDeque<std::string> ring2;
    ring2 = ring1;
    POTHOS_TEST_EQUAL(ring0.size(), ring2.size());
    for (size_t i = 0; i < 10; i++)
    {
        POTHOS_TEST_EQUAL(ring0[i], ring2[i]);
    }

    //test move construct
    Pothos::Util::RingDeque<std::string> ring3(std::move(ring2));
    POTHOS_TEST_EQUAL(ring2.size(), 0);
    POTHOS_TEST_EQUAL(ring0.size(), ring3.size());
    for (size_t i = 0; i < 10; i++)
    {
        POTHOS_TEST_EQUAL(ring0[i], ring3[i]);
    }

    //test move assignment
    Pothos::Util::RingDeque<std::string> ring4;
    ring4 = std::move(ring3);
    POTHOS_TEST_EQUAL(ring3.size(), 0);
    POTHOS_TEST_EQUAL(ring0.size(), ring4.size());
    for (size_t i = 0; i < 10; i++)
    {
        POTHOS_TEST_EQUAL(ring0[i], ring4[i]);
    }
}
