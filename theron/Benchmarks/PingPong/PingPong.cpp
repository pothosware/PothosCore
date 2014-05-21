// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


//
// This benchmarks measures the latency of responding to messages in Theron.
// Latency refers to the delay, or elapsed time, between sending a message
// to an actor and receiving a response. Even if Theron is capabale of high throughput
// (handling millions of messages per second across a number of actors), the latency
// of handling individual messages is an independent concern and equally important for
// specialized applications where fast responses are important.
//
// The ping-pong benchmark is a standard microbenchmark commonly used to measure the
// message processing speed of concurrent systems such as computer networks.
// * Create two actors, called Ping and Pong.
// * Ping is set up to send any non-zero integer messages it receives to Pong, decremented by one.
// * Pong is set up to send any non-zero integer messages it receives to Ping, decremented by one.
// * On receipt of a zero integer message, Ping and Pong send a signal message to the client code indicating completion.
// * Processing is initiated by sending a non-zero integer message to Ping.
//
// The work done by the benchmark consists of sending n messages between Ping and Pong, where
// n is the initial value of the integer message initially sent to Ping. The latency of the
// message sending is calculated as the total execution time divided by the number of messages n.
//


#include <stdio.h>
#include <stdlib.h>

#include <Theron/Theron.h>

#include "../Common/Timer.h"


class PingPong : public Theron::Actor
{
public:

    struct StartMessage
    {
        inline StartMessage(const Theron::Address &caller, const Theron::Address &partner) :
          mCaller(caller),
          mPartner(partner)
        {
        }

        Theron::Address mCaller;
        Theron::Address mPartner;
    };

    inline PingPong(Theron::Framework &framework) : Theron::Actor(framework)
    {
        RegisterHandler(this, &PingPong::Start);
    }

private:

    inline void Start(const StartMessage &message, const Theron::Address /*from*/)
    {
        mCaller = message.mCaller;
        mPartner = message.mPartner;

        DeregisterHandler(this, &PingPong::Start);
        RegisterHandler(this, &PingPong::Receive);
    }

    inline void Receive(const int &message, const Theron::Address /*from*/)
    {
        if (message > 0)
        {
            Send(message - 1, mPartner);
        }
        else
        {
            Send(message, mCaller);
        }
    }

    Theron::Address mCaller;
    Theron::Address mPartner;
};


// Register the message types so that registered names are used instead of dynamic_cast.
THERON_DECLARE_REGISTERED_MESSAGE(int);
THERON_DECLARE_REGISTERED_MESSAGE(PingPong::StartMessage);

THERON_DEFINE_REGISTERED_MESSAGE(int);
THERON_DEFINE_REGISTERED_MESSAGE(PingPong::StartMessage);


int main(int argc, char *argv[])
{
    const int numMessages = (argc > 1 && atoi(argv[1]) > 0) ? atoi(argv[1]) : 50000000;
    const int numThreads = (argc > 2 && atoi(argv[2]) > 0) ? atoi(argv[2]) : 16;

    printf("Using numMessages = %d (use first command line argument to change)\n", numMessages);
    printf("Using numThreads = %d (use second command line argument to change)\n", numThreads);
    printf("Starting %d message sends between ping and pong...\n", numMessages);

    Theron::Framework framework(numThreads);
    Theron::Receiver receiver;

    PingPong ping(framework);
    PingPong pong(framework);

    // Start Ping and Pong, sending each the address of the other and the address of the receiver.
    const PingPong::StartMessage pingStart(receiver.GetAddress(), pong.GetAddress());
    framework.Send(pingStart, receiver.GetAddress(), ping.GetAddress());
    const PingPong::StartMessage pongStart(receiver.GetAddress(), ping.GetAddress());
    framework.Send(pongStart, receiver.GetAddress(), pong.GetAddress());

    Timer timer;
    timer.Start();

    // Send the initial integer count to Ping.
    framework.Send(numMessages, receiver.GetAddress(), ping.GetAddress());

    // Wait to hear back from either Ping or Pong when the count reaches zero.
    receiver.Wait();
    timer.Stop();

    // The number of full cycles is half the number of messages.
    printf("Completed %d message response cycles in %.1f seconds\n", numMessages / 2, timer.Seconds());
    printf("Average response time is %.10f seconds\n", timer.Seconds() / (numMessages / 2));

#if THERON_ENABLE_DEFAULTALLOCATOR_CHECKS
    Theron::IAllocator *const allocator(Theron::AllocatorManager::GetAllocator());
    const int allocationCount(static_cast<Theron::DefaultAllocator *>(allocator)->GetAllocationCount());
    const int peakBytesAllocated(static_cast<Theron::DefaultAllocator *>(allocator)->GetPeakBytesAllocated());
    printf("Total number of allocations: %d calls\n", allocationCount);
    printf("Peak memory usage in bytes: %d bytes\n", peakBytesAllocated);
#endif // THERON_ENABLE_DEFAULTALLOCATOR_CHECKS

}

