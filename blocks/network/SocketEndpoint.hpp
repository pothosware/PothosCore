//
// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/BufferChunk.hpp>
#include <chrono>
#include <cstdint>

static const uint16_t PothosPacketTypeMessage = uint16_t('M');
static const uint16_t PothosPacketTypeLabel = uint16_t('L');
static const uint16_t PothosPacketTypeBuffer = uint16_t('B');
static const uint16_t PothosPacketTypeDType = uint16_t('D');
static const uint16_t PothosPacketTypeHeader = uint16_t('H');
static const uint16_t PothosPacketTypePayload = uint16_t('P');

class PothosPacketSocketEndpoint
{
public:

    /*!
     * Create a new socket endpoint.
     * For the URI scheme, the protocol can be udp or tcp.
     * Do not specify the port for automatic port selection on BIND.
     * \param uri the socket parameters proto://host:port
     * \param opt the socket mode BIND or CONNECT
     */
    PothosPacketSocketEndpoint(const std::string &uri, const std::string &opt);

    /*!
     * Destruct a socket endpoint.
     */
    ~PothosPacketSocketEndpoint(void);

    /*!
     * Get the actual port of the socket.
     * Use this to discover the port of an endpoint,
     * with an unspecified port number in the bind URL.
     */
    std::string getActualPort(void) const;

    /*!
     * Perform the communication initialization handshake.
     */
    void openComms(const std::chrono::high_resolution_clock::duration &timeout = std::chrono::milliseconds(100));

    /*!
     * Perform the communication shutdown handshake.
     */
    void closeComms(const std::chrono::high_resolution_clock::duration &timeout = std::chrono::milliseconds(100));

    /*!
     * Is the endpoint ready for communication?
     */
    bool isReady(void);

    /*!
     * Receive data from the remote endpoint.
     */
    void recv(uint16_t &type, Pothos::BufferChunk &buffer, const std::chrono::high_resolution_clock::duration &timeout = std::chrono::milliseconds(100));

    /*!
     * Send data to the remote endpoint.
     */
    void send(const uint16_t type, const void *buff, const size_t numBytes, const bool more = false);

private:
    struct Impl; Impl *_impl;
};
