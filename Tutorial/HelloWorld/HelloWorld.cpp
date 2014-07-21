// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


#include <stdio.h>
#include <string>

#include <Theron/Theron.h>


// Actor type that prints strings.
// Derives from Theron::Actor.
class Printer : public Theron::Actor
{
public:

    // Constructor, passes the framework to the baseclass.
    Printer(Theron::Framework &framework) : Theron::Actor(framework)
    {
        // Register the message handler.
        RegisterHandler(this, &Printer::Print);
    }

private:

    // Handler for messages of type std::string.
    void Print(const std::string &message, const Theron::Address from)
    {
        // Print the string.
        printf("%s\n", message.c_str());

        // Send a dummy message back for synchronization.
        Send(0, from);
    }
};


int main()
{
    // Construct a framework and instantiate a Printer within it.
    Theron::Framework framework;
    Printer printer(framework);

    // Construct a receiver to receive the reply message.
    Theron::Receiver receiver;

    // Send a string message to the Printer.
    // We pass the address of the receiver as the 'from' address.
    if (!framework.Send(
        std::string("Hello world!"),
        receiver.GetAddress(),
        printer.GetAddress()))
    {
        printf("ERROR: Failed to send message\n");
    }

    // Synchronize with the dummy message sent in reply to make sure we're done.
    receiver.Wait();
}

