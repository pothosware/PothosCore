// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


#include <stdio.h>
#include <string>

#include <Theron/Theron.h>


// Register our message types. Registering message types is an optional optimization.
// It avoids a dependency on dynamic_cast, which relies on built-in C++ RTTI.
THERON_REGISTER_MESSAGE(std::string);


// A simple actor that sends back string messages it receives.
class Replier : public Theron::Actor
{
public:

    inline Replier(Theron::Framework &framework) : Theron::Actor(framework)
    {
        RegisterHandler(this, &Replier::StringHandler);
    }

private:

    inline void StringHandler(const std::string &message, const Theron::Address from)
    {
        printf("Received message '%s'\n", message.c_str());
        Send(message, from);
    }
};


int main()
{
    Theron::Framework framework;
    Theron::Receiver receiver;
    Replier replier(framework);

    // Send the actor a message and wait for the reply, to check it's working.
    if (!framework.Send(
        std::string("Hello"),
        receiver.GetAddress(),
        replier.GetAddress()))
    {
        printf("ERROR: Failed to send message to replier\n");
    }

    receiver.Wait();
}

