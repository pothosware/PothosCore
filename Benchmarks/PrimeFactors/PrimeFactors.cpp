// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <queue>

#include <Theron/Theron.h>

#include "../Common/Timer.h"


static const Theron::uint64_t PRIME_FACTOR_1 = 20483ULL;
static const Theron::uint64_t PRIME_FACTOR_2 = 29303ULL;


THERON_FORCEINLINE static void Factorize(Theron::uint64_t n, Theron::uint64_t *const factors)
{
    Theron::uint64_t count(0);

    if (n <= 3)
    {
        factors[count++] = n;
    }
    else
    {
        Theron::uint64_t d(2);
        while (d < n)
        {
            if ((n % d) == 0)
            {
                factors[count++] = d;
                n /= d;
            }
            else
            {
                d = (d == 2) ? 3 : (d + 2);
            }
        }

        factors[count++] = d;
    }
}


struct Buffer
{
    static const Theron::uint32_t MAX_ENTRIES = 8;

    Theron::uint64_t mData[MAX_ENTRIES];
};


THERON_DECLARE_REGISTERED_MESSAGE(Buffer *);
THERON_DEFINE_REGISTERED_MESSAGE(Buffer *);


struct Query
{
    inline Query() : mProcessed(false)
    {
    }

    THERON_FORCEINLINE void Process(Buffer &buffer)
    {
        Factorize(mInteger, buffer.mData);

        mProcessed = true;
        mFactor0 = buffer.mData[0];
        mFactor1 = buffer.mData[1];
    }

    Theron::Address mClient;
    bool mProcessed;
    Theron::uint64_t mInteger;
    Theron::uint64_t mFactor0;
    Theron::uint64_t mFactor1;
};


THERON_DECLARE_REGISTERED_MESSAGE(Query);
THERON_DEFINE_REGISTERED_MESSAGE(Query);


class Server : public Theron::Actor
{
public:

    struct Request
    {
    };

    inline explicit Server(Theron::Framework &framework, const int numBuffers) : Theron::Actor(framework)
    {
        RegisterHandler(this, &Server::RequestHandler);
        RegisterHandler(this, &Server::BufferHandler);

        mBuffers = new Buffer[numBuffers];
        for (int index(0); index < numBuffers; ++index)
        {
            mBufferQueue.push(mBuffers + index);
        }
    }

    inline ~Server()
    {
        delete [] mBuffers;
    }

private:

    inline void RequestHandler(const Request &/*request*/, const Theron::Address from)
    {
        if (mBufferQueue.empty())
        {
            mRequestQueue.push(from);
        }
        else
        {
            Send(mBufferQueue.front(), from);
            mBufferQueue.pop();
        }
    }

    inline void BufferHandler(Buffer *const &buffer, const Theron::Address /*from*/)
    {
        if (mRequestQueue.empty())
        {
            mBufferQueue.push(buffer);
        }
        else
        {
            Send(buffer, mRequestQueue.front());
            mRequestQueue.pop();
        }
    }

    Buffer *mBuffers;
    std::queue<Buffer *> mBufferQueue;
    std::queue<Theron::Address> mRequestQueue;
};


THERON_DECLARE_REGISTERED_MESSAGE(Server::Request);
THERON_DEFINE_REGISTERED_MESSAGE(Server::Request);


class Worker : public Theron::Actor
{
public:

    inline Worker(Theron::Framework &framework, const Theron::Address &dispatcher, const Theron::Address &server) :
      Theron::Actor(framework),
      mDispatcher(dispatcher),
      mServer(server)
    {
        RegisterHandler(this, &Worker::QueryHandler);
        RegisterHandler(this, &Worker::BufferHandler);
    }

private:

    inline void QueryHandler(const Query &query, const Theron::Address /*from*/)
    {
        Send(Server::Request(), mServer);
        mQuery = query;
    }

    inline void BufferHandler(Buffer *const &buffer, const Theron::Address /*from*/)
    {
        mQuery.Process(*buffer);
        Send(mQuery, mDispatcher);
        Send(buffer, mServer);
    }

    const Theron::Address mDispatcher;
    const Theron::Address mServer;
    Query mQuery;
};


class Dispatcher : public Theron::Actor
{
public:

    inline Dispatcher(Theron::Framework &framework, const Theron::Address &server, const int workerCount) :
      Theron::Actor(framework)
    {
        for (int i = 0; i < workerCount; ++i)
        {
            mWorkers.push_back(new Worker(framework, GetAddress(), server));
            mFreeQueue.push(mWorkers.back()->GetAddress());
        }

        RegisterHandler(this, &Dispatcher::Handler);
    }

    inline ~Dispatcher()
    {
        const int workerCount(static_cast<int>(mWorkers.size()));
        for (int i = 0; i < workerCount; ++i)
        {
            delete mWorkers[i];
        }
    }

private:

    inline void Handler(const Query &query, const Theron::Address from)
    {
        if (query.mProcessed)
        {
            Send(query, query.mClient);
            mFreeQueue.push(from);
        }
        else
        {
            mWorkQueue.push(query);
        }

        if (!mWorkQueue.empty() && !mFreeQueue.empty())
        {
            Send(mWorkQueue.front(), mFreeQueue.front());

            mFreeQueue.pop();
            mWorkQueue.pop();
        }
    }

    std::vector<Worker *> mWorkers;             // Pointers to the owned workers.
    std::queue<Theron::Address> mFreeQueue;     // Queue of available workers.
    std::queue<Query> mWorkQueue;               // Queue of unprocessed queries.
};


int main(int argc, char *argv[])
{
    const int numQueries = (argc > 1 && atoi(argv[1]) > 0) ? atoi(argv[1]) : 1000000;
    const int numThreads = (argc > 2 && atoi(argv[2]) > 0) ? atoi(argv[2]) : 16;
    const int numWorkers = (argc > 3 && atoi(argv[3]) > 0) ? atoi(argv[3]) : 16;
    const int numBuffers = (argc > 4 && atoi(argv[4]) > 0) ? atoi(argv[4]) : 8;

    printf("Using numQueries = %d (use first command line argument to change)\n", numQueries);
    printf("Using numThreads = %d (use second command line argument to change)\n", numThreads);
    printf("Using numWorkers = %d (use third command line argument to change)\n", numWorkers);
    printf("Using numBuffers = %d (use fourth command line argument to change)\n", numBuffers);

    // The reported time includes the startup and cleanup cost.
    Timer timer;
    timer.Start();

    {
        Theron::Framework framework(numThreads);
        Server server(framework, numBuffers);
        Dispatcher dispatcher(framework, server.GetAddress(), numWorkers);
        Theron::Receiver receiver;

        Query query;
        query.mClient = receiver.GetAddress();
        query.mProcessed = false;
        query.mInteger = PRIME_FACTOR_1 * PRIME_FACTOR_2;
        query.mFactor0 = 0;
        query.mFactor1 = 0;

        int queryCount(0);
        int resultCount(0);

        // Throttle the queries so as not to swamp the queues.
        while (resultCount < numQueries)
        {
            while (queryCount < resultCount + numWorkers * 2 && queryCount < numQueries)
            {
                framework.Send(query, receiver.GetAddress(), dispatcher.GetAddress());
                ++queryCount;
            }

            resultCount += static_cast<int>(receiver.Wait(16));
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

