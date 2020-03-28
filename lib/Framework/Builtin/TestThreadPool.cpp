// Copyright (c) 2014-2017 Josh Blum
//                    2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>

POTHOS_TEST_BLOCK("/framework/tests", test_thread_pool)
{
    Pothos::ThreadPool tp0;
    POTHOS_TEST_FALSE(tp0);

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

POTHOS_TEST_BLOCK("/framework/tests", test_thread_pool_args)
{
    Pothos::ThreadPoolArgs args("{\"affinity\":[0, 4], \"numThreads\":2}");
    const std::vector<size_t> expected = {0, 4};
    POTHOS_TEST_EQUALV(args.affinity, expected);
    POTHOS_TEST_EQUAL(args.numThreads, 2);
    POTHOS_TEST_EQUAL(args.priority, 0.0);
    POTHOS_TEST_EQUAL(args.affinityMode, "");
    POTHOS_TEST_EQUAL(args.yieldMode, "");
}
