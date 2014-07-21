// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


/*
* The Computer Language Benchmarks Game
* http://shootout.alioth.debian.org/
*/

//
// This is a performance benchmark. It implements a simple benchmark commonly used to
// measure the raw speed of message passing. A single "token" message is sent around
// a ring of connected actors. The token is an integer counter, and the counter is decremented
// with every hop. When the token value reaches zero the identity of the actor holding the token
// is printed out. Here's the official description from
// http://shootout.alioth.debian.org/u64q/performance.php?test=threadring
//
// - create 503 linked threads (named 1 to 503)
// - thread 503 should be linked to thread 1, forming an unbroken ring
// - pass a token to thread 1
// - pass the token from thread to thread N times
// - print the name of the last thread (1 to 503) to take the token
//
// Note that the processing within each actor is trivial and consists only of decrementing
// the value of the token it received in a message, and forwarding it to the next actor
// in the ring in another message. To form the ring, each actor is provided with the address
// of the next actor in the ring on construction.
//


#include <stdio.h>
#include <stdlib.h>

#include <Theron/Theron.h>

#include "../Common/Timer.h"


static const int NUM_ACTORS = 503;


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
THERON_DECLARE_REGISTERED_MESSAGE(Theron::Address);

THERON_DEFINE_REGISTERED_MESSAGE(int);
THERON_DEFINE_REGISTERED_MESSAGE(Theron::Address);


struct AddressCatcher
{
    inline void Catch(const int &/*message*/, const Theron::Address from) { mAddress = from; }
    Theron::Address mAddress;
};


int main(int argc, char *argv[])
{
    AddressCatcher catcher;

    const int numHops = (argc > 1 && atoi(argv[1]) > 0) ? atoi(argv[1]) : 50000000;
    const int numThreads = (argc > 2 && atoi(argv[2]) > 0) ? atoi(argv[2]) : 16;

    printf("Using numHops = %d (use first command line argument to change)\n", numHops);
    printf("Using numThreads = %d (use second command line argument to change)\n", numThreads);
    printf("Starting one token in a ring of %d actors...\n", NUM_ACTORS);

    // The reported time includes the startup and cleanup cost.
    Timer timer;
    timer.Start();

    {
        Theron::Framework framework(numThreads);
        Member *members[NUM_ACTORS];

        Theron::Receiver receiver;
        receiver.RegisterHandler(&catcher, &AddressCatcher::Catch);

        // Create NUM_ACTORS member actors for the ring.
        for (int index = 0; index < NUM_ACTORS; ++index)
        {
            members[index] = new Member(framework);
        }

        // Initialize the actors by passing each one the address of the next actor in the ring.
        for (int index(NUM_ACTORS - 1), nextIndex(0); index >= 0; nextIndex = index--)
        {
            framework.Send(members[nextIndex]->GetAddress(), receiver.GetAddress(), members[index]->GetAddress());
        }

        // Start the processing by sending the token to the first actor.
        framework.Send(numHops, receiver.GetAddress(), members[0]->GetAddress());

        // Wait for the signal message indicating the tokens has reached zero.
        receiver.Wait();

        // Destroy the member actors.
        for (int index = 0; index < NUM_ACTORS; ++index)
        {
            delete members[index];
        }
    }

    timer.Stop();

    printf("Processed in %.1f seconds\n", timer.Seconds());
    printf("Token stopped at entity '%d'\n", catcher.mAddress.AsInteger());

#if THERON_ENABLE_DEFAULTALLOCATOR_CHECKS
    Theron::IAllocator *const allocator(Theron::AllocatorManager::GetAllocator());
    const int allocationCount(static_cast<Theron::DefaultAllocator *>(allocator)->GetAllocationCount());
    const int peakBytesAllocated(static_cast<Theron::DefaultAllocator *>(allocator)->GetPeakBytesAllocated());
    printf("Total number of allocations: %d calls\n", allocationCount);
    printf("Peak memory usage in bytes: %d bytes\n", peakBytesAllocated);
#endif // THERON_ENABLE_DEFAULTALLOCATOR_CHECKS

}

