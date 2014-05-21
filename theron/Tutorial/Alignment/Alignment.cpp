// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


#include <stdio.h>

#include <Theron/Theron.h>


// We need to disable this irritating level-4 warning in Visual C++ builds.
#ifdef _MSC_VER
#pragma warning(disable:4324)
#endif // _MSC_VER


#define CACHE_ALIGNMENT 64


// A message type that, for some reason, must always be allocated on a cache-line.
struct THERON_PREALIGN(CACHE_ALIGNMENT) AlignedMessage
{
    AlignedMessage(const int value) : mValue(value) { }
    int mValue;

} THERON_POSTALIGN(CACHE_ALIGNMENT);


// Notify Theron of the alignment requirements of the message type.
THERON_ALIGN_MESSAGE(AlignedMessage, CACHE_ALIGNMENT);


// A simple actor that prints out the memory addresses of messages it receives.
// For the sake of the example we assume that the actor type also requires
// cache-line alignment. The same pair of macro's can be used here too.
class THERON_PREALIGN(CACHE_ALIGNMENT) AlignedActor : public Theron::Actor
{
public:

    inline AlignedActor(Theron::Framework &framework) : Theron::Actor(framework)
    {
        RegisterHandler(this, &AlignedActor::Handler);
    }

private:

    inline void Handler(const AlignedMessage &message, const Theron::Address from)
    {
        // Print out the memory address of the message copy to check its alignment.
        printf("Received message aligned to %d bytes at address 0x%p\n",
            CACHE_ALIGNMENT,
            &message);

        if (!THERON_ALIGNED(&message, CACHE_ALIGNMENT))
        {
            printf("ERROR: Received message isn't correctly aligned\n");
        }

        Send(message, from);
    }

} THERON_POSTALIGN(CACHE_ALIGNMENT);


int main()
{
    Theron::Framework framework;
    Theron::Receiver receiver;

    // Construct an instance of the aligned actor type on the stack.
    AlignedActor alignedActor(framework);

    // Print out its memory address to check that it's a multiple of 64 bytes as
    // required. This alignment of instances on the stack is performed by the compiler
    // and ensured by THERON_PREALIGN and THERON_POSTALIGN.
    printf("Constructed actor aligned to %d bytes at address 0x%p\n",
        CACHE_ALIGNMENT,
        &alignedActor);

    if (!THERON_ALIGNED(&alignedActor, CACHE_ALIGNMENT))
    {
        printf("ERROR: Constructed actor isn't correctly aligned\n");
    }

    // Construct an instance of the aligned message type on the stack.
    AlignedMessage alignedMessage(503);

    // Print out its memory address to check that it's a multiple of 64 bytes.
    printf("Constructed message aligned to %d bytes at address 0x%p\n",
        CACHE_ALIGNMENT,
        &alignedMessage);

    if (!THERON_ALIGNED(&alignedMessage, CACHE_ALIGNMENT))
    {
        printf("ERROR: Constructed message isn't correctly aligned\n");
    }

    // Send the aligned message to the address printer. The actor sees a different
    // copy of the message (because messages are copied when they are sent), yet
    // the copy should be correctly aligned as well. This second kind of alignment
    // is ensured by THERON_ALIGN_MESSAGE. Note however that it relies on the
    // allocator used by Theron supporting aligned allocation requests (and the
    // DefaultAllocator, used by default, does).
    if (!framework.Send(
        alignedMessage,
        receiver.GetAddress(),
        alignedActor.GetAddress()))
    {
        printf("ERROR: Failed to send message to address printer\n");
    }

    // Wait for the reply to be sure the message was handled.
    receiver.Wait();
}

