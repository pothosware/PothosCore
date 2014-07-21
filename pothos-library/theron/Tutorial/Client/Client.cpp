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


int main(int argc, char *argv[])
{
    char buffer[256] = { '\0' };

    if (argc < 3)
    {
        printf("Usage: Client <client IP address> <server IP address>\n");
        printf("Use local IP address as client address, and IP address of remote host as server address.\n");

        if (GetLocalIPAddress(buffer))
        {
            printf("Local (client) IP address is %s.\n", buffer);
        }

        return 1;
    }

    printf("Connecting; start remote server.\n");

    // Create a local endpoint.
    sprintf(buffer, "tcp://%s:5556", argv[1]);
    Theron::EndPoint endPoint("client", buffer);

    // Connect to the remote endpoint.
    sprintf(buffer, "tcp://%s:5555", argv[2]);
    if (!endPoint.Connect(buffer))
    {
        printf("ERROR: Connection failed - check networking is enabled.\n");
        return 1;
    }

    // The framework is tied to the endpoint.
    Theron::Framework framework(endPoint);

    // Send messages to the server in a loop until the user enters 'exit'.
    // Note that if the server hasn't started listening yet it may miss the first messages!
    printf("Enter lines of text (max 256 chars per line). Type 'exit' to end.\n");

    while (strcmp(buffer, "exit") != 0)
    {
        // Send the text in a messages to the remote 'printer' actor using its unique name.
        scanf("%s", buffer);
        framework.Send(
            TextMessage(buffer),
            Theron::Address(),
            Theron::Address("printer"));
    }
}


#ifdef _MSC_VER
#pragma warning(pop)
#endif //_MSC_VER

