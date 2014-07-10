// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>

POTHOS_TEST_BLOCK("/framework/tests", test_thread_pool)
{
    Pothos::ThreadPool tp0;
    POTHOS_TEST_TRUE(not tp0);

    Pothos::ThreadPool tp1(Pothos::ThreadPoolArgs(2/*threads*/));
    POTHOS_TEST_TRUE(tp1);

    Pothos::ThreadPoolArgs args2;
    args2.affinityMode = "FAIL";
    POTHOS_TEST_THROWS(Pothos::ThreadPool tp2(args2), Pothos::ThreadPoolError);

    Pothos::ThreadPoolArgs args3;
    args3.yieldMode = "FAIL";
    POTHOS_TEST_THROWS(Pothos::ThreadPool tp3(args3), Pothos::ThreadPoolError);

    Pothos::ThreadPoolArgs args4;
    args4.priority = -1e6;
    POTHOS_TEST_THROWS(Pothos::ThreadPool tp4(args4), Pothos::ThreadPoolError);
}
