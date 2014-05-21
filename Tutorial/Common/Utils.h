// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.
#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H


#include <Theron/Defines.h>

// This file uses WinSock for local IP address discovery.
#if THERON_XS && THERON_MSVC
#include <winsock2.h>
#endif

#include <string.h>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable:4996)  // function or variable may be unsafe.
#endif //_MSC_VER


inline bool GetLocalIPAddress(char *const result)
{
    if (result == 0)
    {
        return false;
    }

    // TODO: Currently only implemented in Visual Studio builds;
    // in theory it should be possible to support on other platforms too.
#if THERON_XS && THERON_MSVC

    // From an article 'Socket Programming gethostbyname' by Guy Lecky-Thompson.
    // Initialize WinSock.
    WSADATA wsa_Data;
    if (WSAStartup(0x101, &wsa_Data) != 0)
    {
        return false;
    }

    // Get the local host name.
    char hostName[255];
    if (gethostname(hostName, 255) != 0)
    {
        return false;
    }

    struct hostent *host_entry;
    host_entry = gethostbyname(hostName);

    char *localIP(0);
    localIP = inet_ntoa(*(struct in_addr *)*host_entry->h_addr_list);

    if (localIP == 0 || strlen(localIP) == 0)
    {
        return false;
    }

    result[0] = '\0';
    strcpy(result, localIP);

    // Shut down WinSock.
    if (WSACleanup() == 0)
    {
        return true;
    }

#endif

    return false;
}


#ifdef _MSC_VER
#pragma warning(pop)
#endif //_MSC_VER


#endif // COMMON_UTILS_H
