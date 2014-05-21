// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


#include <stdio.h>
#include <string>

#include <Theron/Theron.h>


static void DumpMessage(
    const void *const data,
    const Theron::uint32_t size,
    const Theron::Address from)
{
    printf("Unhandled %d byte message sent from address %d:\n",
        size,
        from.AsInteger());

    // Dump the message as hex data.
    if (data)
    {
        const char *const format("[%d] 0x%08x\n");

        const unsigned int *const begin(reinterpret_cast<const unsigned int *>(data));
        const unsigned int *const end(begin + size / sizeof(unsigned int));

        for (const unsigned int *word(begin); word != end; ++word)
        {
            printf(format, static_cast<int>(word - begin), *word);
        }
    }
}


// A simple actor that prints out strings it receives.
class Printer : public Theron::Actor
{
public:

    Printer(Theron::Framework &framework) : Theron::Actor(framework)
    {
        // Register the handler for string messages.
        RegisterHandler(this, &Printer::Print);

        // Register the default handler for all other kinds of messages.
        // If we don't register our own default handler then the 'default' default
        // handler will be used, which asserts on receiving an unhandled message.
        SetDefaultHandler(this, &Printer::DefaultHandler);
    }

private:

    void Print(const std::string &message, const Theron::Address from)
    {
        printf("%s\n", message.c_str());

        // Send the message back for synchronization.
        Send(message, from);
    }

    // Default handler which handles messages of unrecognized types.
    // This is a 'blind' handler which takes the unhandled message as raw data.
    void DefaultHandler(
        const void *const data,
        const Theron::uint32_t size,
        const Theron::Address from)
    {
        DumpMessage(data, size, from);
    }
};


// A handler object that reports any undelivered or unhandled messages.
class FallbackHandler
{
public:

    // Fallback handler which handles any messages not handled by an actor.
    // This is a 'blind' handler which takes the unhandled message as raw data.
    void Handle(
        const void *const data,
        const Theron::uint32_t size,
        const Theron::Address from)
    {
        DumpMessage(data, size, from);
    }
};


int main()
{
    Theron::Receiver receiver;
    Theron::Address printerAddress;

    // Create a framework and register a fallback handler with it.
    Theron::Framework framework;
    FallbackHandler fallbackHandler;
    framework.SetFallbackHandler(&fallbackHandler, &FallbackHandler::Handle);

    {
        // Create a printer actor within a local scope and remember its address.
        Printer printer(framework);
        printerAddress = printer.GetAddress();

        // Send the printer a message of a type which it doesn't handle.
        // This message reaches the printer but is handled by its default handler.
        framework.Send(103, receiver.GetAddress(), printerAddress);

        // Send the printer a string message to show that it actually works.
        framework.Send(std::string("hit"), receiver.GetAddress(), printerAddress);

        // Wait for the reply to the handled string message, for synchronization.
        receiver.Wait();
    }

    // Send a string message to the printer's address, which is now stale.
    // This message never reaches an actor so is handled by the fallback handler.
    framework.Send(std::string("miss"), receiver.GetAddress(), printerAddress);
}

