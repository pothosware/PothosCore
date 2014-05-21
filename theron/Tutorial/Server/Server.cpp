// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


/*
The Client and Server examples show how to perform distributed computing with Theron.
*/


#include "../Common/Utils.h"

#include <Theron/Defines.h>

// NOTE: Must include xs.h before standard headers to avoid warnings in MS headers!
#if THERON_XS
#include <xs/xs.h>
#endif // THERON_XS

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <Theron/Theron.h>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable:4996)  // function or variable may be unsafe.
#endif //_MSC_VER


// Simple message type that can be copied with memcpy so safely sent over the network.
struct TextMessage
{
    explicit TextMessage(const char *const text)
    {
        mText[0] = '\0';
        strcpy(mText, text);
    }

    char mText[256];
};


// In order to be sent over the network, message types must be registered.
THERON_REGISTER_MESSAGE(TextMessage);


class Printer : public Theron::Actor
{
public:

    Printer(Theron::Framework &framework, const char *const name) :
      Theron::Actor(framework, name)
    {
        RegisterHandler(this, &Printer::Handler);
    }

private:

    void Handler(const TextMessage &message, const Theron::Address /*from*/)
    {
        printf("%s\n", message.mText);
        if (strcmp(message.mText, "exit") == 0)
        {
            // Signal we're done.
            Send(0, Theron::Address("receiver"));
        }
    }
};


int main(int argc, char *argv[])
{
    char buffer[256] = { '\0' };

    if (argc < 3)
    {
        printf("Usage: Server <server IP address> <client IP address>\n");
        printf("Use local IP address as server address, and IP address of remote host as client address.\n");

        if (GetLocalIPAddress(buffer))
        {
            printf("Local (server) IP address is %s.\n", buffer);
        }

        return 1;
    }

    printf("Connecting; start remote client.\n");

    // Create a local endpoint.
    sprintf(buffer, "tcp://%s:5555", argv[1]);
    Theron::EndPoint endPoint("server", buffer);

    // Connect to the remote endpoint.
    sprintf(buffer, "tcp://%s:5556", argv[2]);
    if (!endPoint.Connect(buffer))
    {
        printf("ERROR: Connection failed - check networking is enabled.\n");
        return 1;
    }

    // The framework and receiver are tied to the endpoint.
    Theron::Receiver receiver(endPoint, "receiver");
    Theron::Framework framework(endPoint);

    // The unique name of the actor allows the client to send it messages remotely.
    Printer printer(framework, "printer");

    receiver.Wait();
}


#ifdef _MSC_VER
#pragma warning(pop)
#endif //_MSC_VER

