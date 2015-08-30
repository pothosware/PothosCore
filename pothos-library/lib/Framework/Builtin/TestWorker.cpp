// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <chrono>
#include <thread>
#include <iostream>

struct MyWorker0 : Pothos::Block
{
    MyWorker0(void)
    {
        this->setupOutput(0, "float32");
        once = false;
    }

    void work(void)
    {
        if (once) return;
        once = true;
        std::cout << "hello from worker0\n";
        std::cout << "output ptr " << this->output(0)->buffer().address << std::endl;
        std::cout << "output elements " << this->output(0)->elements() << std::endl;
        std::cout << "output dtype size " << this->output(0)->dtype().size() << std::endl;
        this->output(0)->produce(10);
    }

    bool once;
};

struct MyWorker1 : Pothos::Block
{
    MyWorker1(void)
    {
        this->setupInput(0, "float32");
        this->input(0)->setReserve(1);
    }

    void work(void)
    {
        std::cout << "hello from worker1\n";
        std::cout << "input ptr " << this->input(0)->buffer().address << std::endl;
        std::cout << "input elements " << this->input(0)->elements() << std::endl;
        std::cout << "input dtype size " << this->input(0)->dtype().size() << std::endl;
        this->input(0)->consume(this->input(0)->elements());
    }
};

struct AddFloat32 : Pothos::Block
{
    AddFloat32(void)
    {
        this->setupInput(0, "float32");
        this->setupOutput(0, "float32");
    }

    void work(void)
    {
        auto out = outputs()[0]->buffer().as<float *>();
        auto in0 = inputs()[0]->buffer().as<const float *>();
        const size_t elems = this->workInfo().minElements;

        for (size_t i = 1; i < inputs().size(); i++)
        {
            auto in = inputs()[i]->buffer().as<const float *>();
            for (size_t n = 0; n < elems; n++)
            {
                out[n] = in0[n] + in[n];
            }
            in0 = out; //setup for next loop
            inputs()[i]->consume(elems);
        }

        inputs()[0]->consume(elems);
        outputs()[0]->produce(elems);
    }
};

POTHOS_TEST_BLOCK("/framework/tests", test_workers)
{
    auto w0 = new MyWorker0();
    auto w1 = std::shared_ptr<MyWorker1>(new MyWorker1());

    {
        Pothos::Topology t;
        t.connect(w0, 0, w1, 0);
        t.commit();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    delete w0;
    //delete w1;
}

struct ActivateThrower : Pothos::Block
{
    ActivateThrower(void)
    {
        this->setupInput(0, "float32");
        this->input(0)->setReserve(1);
    }

    void activate(void)
    {
        std::cerr << "activate is going to throw!" << std::endl;
        throw Pothos::Exception("ActivateThrower!");
    }

    void work(void)
    {
        std::cerr << "this work should never be called" << std::endl;
    }
};

POTHOS_TEST_BLOCK("/framework/tests", test_activate_throw)
{
    auto w0 = std::shared_ptr<MyWorker0>(new MyWorker0());
    auto w1 = std::shared_ptr<ActivateThrower>(new ActivateThrower());

    {
        Pothos::Topology t;
        t.connect(w0, 0, w1, 0);
        POTHOS_TEST_THROWS(t.commit(), Pothos::TopologyConnectError);
    }
}
