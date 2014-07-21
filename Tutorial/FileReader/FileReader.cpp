// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


#include <stdio.h>
#include <vector>
#include <queue>

#include <Theron/Theron.h>


#ifdef _MSC_VER
#pragma warning(disable:4996) // 'fopen': This function or variable may be unsafe.
#endif // _MSC_VER


static const int MAX_FILES = 16;
static const int MAX_FILE_SIZE = 16384;


// A file read request: read the contents of a disk file into a memory buffer.
struct ReadRequest
{
public:

    explicit ReadRequest(
        const Theron::Address client = Theron::Address(),
        const char *const fileName = 0,
        unsigned char *const buffer = 0,
        const unsigned int bufferSize = 0) :
      mClient(client),
      mFileName(fileName),
      mProcessed(false),
      mBuffer(buffer),
      mBufferSize(bufferSize),
      mFileSize(0)
    {
    }

    void Process()
    {
        mProcessed = true;
        mFileSize = 0;

        // Try to open the file.
        FILE *const handle = fopen(mFileName, "rb");
        if (handle != 0)
        {
            // Read the file data, setting the actual size.
            mFileSize = (uint32_t) fread(
                mBuffer,
                sizeof(unsigned char),
                mBufferSize,
                handle);

            fclose(handle);
        }
    }

    Theron::Address mClient;            // Address of the requesting client.
    const char *mFileName;              // Name of the requested file.
    bool mProcessed;                    // Whether the file has been read.
    unsigned char *mBuffer;             // Buffer for file contents.
    unsigned int mBufferSize;           // Size of the buffer.
    unsigned int mFileSize;             // Size of the file in bytes.
};


// A stateless worker actor that processes work messages.
// Each worker can only process one work item at a time.
template <class WorkMessage>
class Worker : public Theron::Actor
{
public:

    // Constructor.
    Worker(Theron::Framework &framework) : Theron::Actor(framework)
    {
        RegisterHandler(this, &Worker::Handler);
    }

private:

    // Message handler for WorkMessage messages.
    void Handler(const WorkMessage &message, const Theron::Address from)
    {
        // The message parameter is const so we need to copy it to change it.
        WorkMessage result(message);
        result.Process();

        // Forward the processed message back to the sender.
        Send(result, from);
    }
};


// A dispatcher actor that processes work items.
// Internally the dispatcher creates and controls a pool of workers.
// It coordinates the workers to process the work items in parallel.
template <class WorkMessage>
class Dispatcher : public Theron::Actor
{
public:

    Dispatcher(Theron::Framework &framework, const int workerCount) : Theron::Actor(framework)
    {
        // Create the workers and add them to the free list.
        for (int i = 0; i < workerCount; ++i)
        {
            mWorkers.push_back(new WorkerType(framework));
            mFreeQueue.push(mWorkers.back()->GetAddress());
        }

        RegisterHandler(this, &Dispatcher::Handler);
    }

    ~Dispatcher()
    {
        // Destroy the workers.
        const int workerCount(static_cast<int>(mWorkers.size()));
        for (int i = 0; i < workerCount; ++i)
        {
            delete mWorkers[i];
        }
    }

private:

    typedef Worker<WorkMessage> WorkerType;

    // Handles work requests from clients.
    void Handler(const WorkMessage &message, const Theron::Address from)
    {
        // Has this work item been processed?
        if (message.mProcessed)
        {
            // Send the result back to the caller that requested it.
            Send(message, message.mClient);

            // Add the worker that sent the result to the free list.
            mFreeQueue.push(from);
        }
        else
        {
            // Add the unprocessed work message to the work list.
            mWorkQueue.push(message);
        }

        // Service the work queue.
        while (!mWorkQueue.empty() && !mFreeQueue.empty())
        {
            Send(mWorkQueue.front(), mFreeQueue.front());

            mFreeQueue.pop();
            mWorkQueue.pop();
        }
    }

    std::vector<WorkerType *> mWorkers;         // Pointers to the owned workers.
    std::queue<Theron::Address> mFreeQueue;     // Queue of available workers.
    std::queue<WorkMessage> mWorkQueue;         // Queue of unprocessed work messages.
};


int main(int argc, char *argv[])
{
    // Theron::Framework objects can be constructed with parameters.
    // We create a framework with n worker threads, where n is the number of
    // files we wish to be able to read concurrently. We also restrict the
    // worker threads (arbitrarily) to the first two processor cores.
    Theron::Framework::Parameters frameworkParams;
    frameworkParams.mThreadCount = MAX_FILES;
    frameworkParams.mProcessorMask = (1UL << 0) | (1UL << 1);
    Theron::Framework framework(frameworkParams);

    if (argc < 2)
    {
        printf("Expected up to 16 file name arguments.\n");
    }

    // Register a handler with a receiver to catch the result messages.
    Theron::Receiver receiver;
    Theron::Catcher<ReadRequest> resultCatcher;
    receiver.RegisterHandler(&resultCatcher, &Theron::Catcher<ReadRequest>::Push);

    // Create a dispatcher to process the work, with MAX_FILES workers.
    Dispatcher<ReadRequest> dispatcher(framework, MAX_FILES);

    // Send the work requests, one for each file name on the command line.
    for (int i = 0; i < MAX_FILES && i + 1 < argc; ++i)
    {
        unsigned char *const buffer = new unsigned char[MAX_FILE_SIZE];
        const ReadRequest message(
            receiver.GetAddress(),
            argv[i + 1],
            buffer,
            MAX_FILE_SIZE);

        framework.Send(message, receiver.GetAddress(), dispatcher.GetAddress());
    }

    // Wait for all results.
    for (int i = 1; i < argc; ++i)
    {
        receiver.Wait();
    }

    // Handle the results; we just print the sizes of the files.
    ReadRequest result;
    Theron::Address from;
    while (!resultCatcher.Empty())
    {
        resultCatcher.Pop(result, from);
        printf("Read %d bytes from file '%s'\n", result.mFileSize, result.mFileName);

        // Free the allocated buffer.
        delete [] result.mBuffer;
    }
}





