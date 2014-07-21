// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


#include <stdio.h>
#include <string>

#include <Theron/Theron.h>

#include <Theron/Detail/Threading/SpinLock.h>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable:4324)  // structure was padded due to __declspec(align())
#endif //_MSC_VER


// A simple linear allocator implementing Theron::IAllocator.
// It allocates from a memory buffer, never freeing, until it runs out.
class LinearAllocator : public Theron::IAllocator
{
public:

    LinearAllocator(void *const buffer, const SizeType size) :
      mSpinLock(),
      mBuffer(static_cast<unsigned char *>(buffer)),
      mOffset(mBuffer),
      mEnd(mBuffer + size)
    {
    }

    virtual ~LinearAllocator()
    {
    }

    virtual void *Allocate(const SizeType size)
    {
        // Default 4-byte alignment.
        return AllocateAligned(size, 4);
    }

    virtual void *AllocateAligned(const SizeType size, const SizeType alignment)
    {
        unsigned char *allocation(0);

        mSpinLock.Lock();

        allocation = mOffset;
        THERON_ALIGN(allocation, alignment);

        // Buffer used up yet?
        if (allocation + size <= mEnd)
        {
            mOffset = allocation + size;
        }
        else
        {
            allocation = 0;
        }

        mSpinLock.Unlock();

        return static_cast<void *>(allocation);
    }

    virtual void Free(void *const /*memory*/)
    {
    }

    virtual void Free(void *const /*memory*/, const SizeType /*size*/)
    {
    }

    SizeType GetBytesAllocated()
    {
        return static_cast<SizeType>(mOffset - mBuffer);
    }    

private:

    LinearAllocator(const LinearAllocator &other);
    LinearAllocator &operator=(const LinearAllocator &other);

    Theron::Detail::SpinLock mSpinLock;     // Used to ensure thread-safe access.
    unsigned char *mBuffer;                 // Base address of referenced memory buffer.
    unsigned char *mOffset;                 // Current place within referenced memory buffer.
    unsigned char *mEnd;                    // End of referenced memory buffer (exclusive).
};


// A simple actor that sends back string messages it receives.
class Replier : public Theron::Actor
{
public:

    Replier(Theron::Framework &framework) : Theron::Actor(framework)
    {
        RegisterHandler(this, &Replier::StringHandler);
    }

private:

    void StringHandler(const std::string &message, const Theron::Address from)
    {
        Send(message, from);
    }
};


int main()
{
    // Construct a LinearAllocator around a memory buffer.
    // Note that the buffer needs to be quite big due to fixed memory overheads inside Theron.
    const unsigned int BUFFER_SIZE(1024 * 1024);
    unsigned char *const buffer = new unsigned char[BUFFER_SIZE];
    LinearAllocator allocator(buffer, BUFFER_SIZE);

    // Set the custom allocator for use by Theron.
    // Note that can only be done once, at start of day.
    Theron::AllocatorManager::SetAllocator(&allocator);

    // Construct a framework and an actor, send the actor a message and wait for the reply.
    // We do this in a local scope to ensure that all Theron objects are destructed before
    // we delete the buffer!
    {
        Theron::Framework framework;
        Theron::Receiver receiver;
        Replier replier(framework);

        if (!framework.Send(std::string("base"), receiver.GetAddress(), replier.GetAddress()))
        {
            printf("ERROR: Failed to send message to replier\n");
        }

        receiver.Wait();
    }

    printf("Allocated %d bytes\n", static_cast<int>(allocator.GetBytesAllocated()));

    delete [] buffer;
}


#ifdef _MSC_VER
#pragma warning(pop)
#endif //_MSC_VER

