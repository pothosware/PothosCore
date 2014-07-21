// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


//
// This benchmark implements a variant on the "thread-ring" benchmark intended to
// measure the raw speed of message passing. In the traditional benchmark, a single
// "token" message is sent around a ring of connected actors. The token is an integer
// counter, and the counter is decremented with every hop. When the token value reaches
// zero the identity of the actor holding the token is printed out.
//
// Note that the processing within each actor is trivial and consists only of decrementing
// the value of the token it received in a message, and forwarding it to the next actor
// in the ring in another message. To form the ring, each actor is provided with the address
// of the next actor in the ring on construction.
//
// Unlike the standard benchmark, this variant creates 503 tokens instead of just one.
// Each of the actors in the ring is passed one token initially. As before, the tokens
// are passed around the ring and decremented with each hop until they reach zero. The
// waiting main program terminates when it has received all of the replies, indicating
// all the tokens have reached zero. The total number of "hops" to be performed is split
// equally between the 503 tokens, so that between them they perform the intended number
// of hops, but in parallel.
//
// This benchmark is a more severe (and arguably more meaningful) measure of the raw
// performance of an Actor Model implementation, since it involves parallel message
// passing and so contention over the shared components of the message passing mechanism.
// It's execution times show a lot of variability, presumably because the execution order
// (the order in which the actors are processed) is non-deterministic and can vary from
// one run to the next.
//


#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <Theron/Theron.h>

#include "../Common/Timer.h"


class Member : public Theron::Actor
{
public:

    inline Member(Theron::Framework &framework) : Theron::Actor(framework)
    {
        RegisterHandler(this, &Member::InitHandler);
    }

private:

    inline void InitHandler(const Theron::Address &next, const Theron::Address from)
    {
        mNext = next;
        mCaller = from;

        RegisterHandler(this, &Member::TokenHandler);
        DeregisterHandler(this, &Member::InitHandler);
    }

    inline void TokenHandler(const int &token, const Theron::Address /*from*/)
    {
        if (token > 0)
        {
            Send(token - 1, mNext);
        }
        else
        {
            Send(token, mCaller);
        }
    }

    Theron::Address mNext;
    Theron::Address mCaller;
};


// Register the message types so that registered names are used instead of dynamic_cast.
THERON_DECLARE_REGISTERED_MESSAGE(int);
THERON_DEFINE_REGISTERED_MESSAGE(int);

THERON_DECLARE_REGISTERED_MESSAGE(Theron::Address);
THERON_DEFINE_REGISTERED_MESSAGE(Theron::Address);


int main(int argc, char *argv[])
{
    const int numHops = (argc > 1 && atoi(argv[1]) > 0) ? atoi(argv[1]) : 50000000;
    const int numThreads = (argc > 2 && atoi(argv[2]) > 0) ? atoi(argv[2]) : 16;
    const int numActors = (argc > 3 && atoi(argv[3]) > 0) ? atoi(argv[3]) : 503;
    const int tokenValue((numHops + numActors - 1) / numActors);

    printf("Using numHops = %d (use first command line argument to change)\n", numHops);
    printf("Using numThreads = %d (use second command line argument to change)\n", numThreads);
    printf("Using numActors = %d (use third command line argument to change)\n", numActors);
    printf("Starting %d tokens with initial value %d in a ring of %d actors...\n", numActors, tokenValue, numActors);

    // The reported time includes the startup and cleanup cost.
    Timer timer;
    timer.Start();

    {
        Theron::Framework framework(numThreads);
        std::vector<Member *> members(numActors);
        Theron::Receiver receiver;

        // Create the member actors.
        for (int index = 0; index < numActors; ++index)
        {
            members[index] = new Member(framework);
        }

        // Initialize the actors by passing each one the address of the next actor in the ring.
        for (int index(numActors - 1), nextIndex(0); index >= 0; nextIndex = index--)
        {
            framework.Send(members[nextIndex]->GetAddress(), receiver.GetAddress(), members[index]->GetAddress());
        }

        // Send exactly one token to each actor, all with the same initial value, rounding up if required.
        for (int index = 0; index < numActors; ++index)
        {
            framework.Send(tokenValue, receiver.GetAddress(), members[index]->GetAddress());
        }

        // Wait for all signal messages, indicating the tokens have all reached zero.
        int outstandingCount(numActors);
        while (outstandingCount)
        {
            outstandingCount -= receiver.Wait(outstandingCount);
        }

        // Destroy the member actors.
        for (int index = 0; index < numActors; ++index)
        {
            delete members[index];
        }
    }

    timer.Stop();

    printf("Processed in %.1f seconds\n", timer.Seconds());

#if THERON_ENABLE_DEFAULTALLOCATOR_CHECKS
    Theron::IAllocator *const allocator(Theron::AllocatorManager::GetAllocator());
    const int allocationCount(static_cast<Theron::DefaultAllocator *>(allocator)->GetAllocationCount());
    const int peakBytesAllocated(static_cast<Theron::DefaultAllocator *>(allocator)->GetPeakBytesAllocated());
    printf("Total number of allocations: %d calls\n", allocationCount);
    printf("Peak memory usage in bytes: %d bytes\n", peakBytesAllocated);
#endif // THERON_ENABLE_DEFAULTALLOCATOR_CHECKS

}

