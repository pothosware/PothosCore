// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "RemoteProxyDatagram.hpp"
#include <Pothos/Exception.hpp>
#include <Poco/ByteOrder.h>
#include <streambuf>
#include <iostream>
#include <cstdint>
#include <algorithm> //min/max
#include <cstring> //memcpy

/***********************************************************************
 * Header structure and constants
 **********************************************************************/
#define POTHOS_PACKET_WORD32(str) \
    (uint32_t(str[0]) << 24) | \
    (uint32_t(str[1]) << 16) | \
    (uint32_t(str[2]) << 8) | \
    (uint32_t(str[3]) << 0)

static const uint32_t PothosRPCHeaderWord = POTHOS_PACKET_WORD32("PRPC");
static const uint32_t PothosRPCTrailerWord = POTHOS_PACKET_WORD32("CPRP");

struct PothosRPCHeader
{
    uint32_t headerWord;
    uint32_t payloadBytes;
};

struct PothosRPCTrailer
{
    uint32_t trailerWord;
};

/***********************************************************************
 * Serialization streambuf
 **********************************************************************/
class PRPCDatagramObuf : public std::streambuf
{
public:
    PRPCDatagramObuf(std::ostream &os, const Pothos::Object &data):
        _bytesWritten(0),
        _payloadData(1024)
    {
        //serialize to a temporary buffer
        std::ostream oser(this);
        data.serialize(oser);

        //load the header and trailer
        PothosRPCHeader header;
        header.headerWord = Poco::ByteOrder::toNetwork(PothosRPCHeaderWord);
        header.payloadBytes = Poco::ByteOrder::toNetwork(uint32_t(_bytesWritten));

        PothosRPCTrailer trailer;
        trailer.trailerWord = Poco::ByteOrder::toNetwork(PothosRPCTrailerWord);

        //write to the output stream
        os.write((const char *)&header, sizeof(header));
        os.write(_payloadData.data(), _bytesWritten);
        os.write((const char *)&trailer, sizeof(trailer));
        os.flush();
    }

    void ensureSize(const std::streamsize count)
    {
        //resize temporary buffer to hold the msg
        while (size_t(_bytesWritten + count) > _payloadData.size())
        {
            _payloadData.resize(_payloadData.size()*2);
        }
    }

    int_type overflow(int_type c)
    {
        if (traits_type::eq_int_type(c, traits_type::eof())) return traits_type::eof();
        const char_type t = traits_type::to_char_type(c);
        this->ensureSize(1);
        _payloadData[_bytesWritten++] = t;
        return c;
    }

    std::streamsize xsputn(const char *s, std::streamsize count)
    {
        this->ensureSize(count);

        //append the message to the payload
        std::memcpy(_payloadData.data()+_bytesWritten, s, count);
        _bytesWritten += count;

        return count;
    }

private:
    std::streamsize _bytesWritten;
    std::vector<char_type> _payloadData;
};

/***********************************************************************
 * Deserialization streambuf
 **********************************************************************/
class PRPCDatagramIbuf : public std::streambuf
{
public:
    PRPCDatagramIbuf(std::istream &is, Pothos::Object &data):
        _bytesRead(0)
    {
        //read the header
        PothosRPCHeader header;
        is.read((char *)&header, sizeof(header));
        if (is.eof()) throw Pothos::IOException("recvDatagram()", "stream end");
        if (not is) throw Pothos::IOException("recvDatagram()", "stream error");

        //parse the header
        if (Poco::ByteOrder::fromNetwork(header.headerWord) != PothosRPCHeaderWord)
        {
            throw Pothos::IOException("recvDatagram()", "headerWord fail");
        }
        _payloadData.resize(Poco::ByteOrder::fromNetwork(header.payloadBytes));

        //read the payload
        is.read(_payloadData.data(), _payloadData.size());
        if (is.eof()) throw Pothos::IOException("recvDatagram()", "stream end");
        if (not is) throw Pothos::IOException("recvDatagram()", "stream error");

        //read the trailer
        PothosRPCTrailer trailer;
        is.read((char *)&trailer, sizeof(trailer));
        if (is.eof()) throw Pothos::IOException("recvDatagram()", "stream end");
        if (not is) throw Pothos::IOException("recvDatagram()", "stream error");

        //parse the trailer
        if (Poco::ByteOrder::fromNetwork(trailer.trailerWord) != PothosRPCTrailerWord)
        {
            throw Pothos::IOException("recvDatagram()", "trailerWord fail");
        }

        //deserialize from temporary buffer
        std::istream iser(this);
        data.deserialize(iser);
    }

    int_type underflow(void)
    {
        if (this->showmanyc() == 0) return traits_type::eof();
        const char c = _payloadData[_bytesRead];
        return traits_type::to_int_type(c);
    }

    int_type uflow(void)
    {
        if (this->showmanyc() == 0) return traits_type::eof();
        const char c = _payloadData[_bytesRead++];
        return traits_type::to_int_type(c);
    }

    std::streamsize showmanyc(void)
    {
        return _payloadData.size()-_bytesRead;
    }

    int_type pbackfail(int_type c)
    {
        if (traits_type::eq_int_type(c, traits_type::eof())) return traits_type::eof();
        if (_bytesRead == 0) return traits_type::eof(); //still at start
        const char_type t = traits_type::to_char_type(c);
        if (t != _payloadData[_bytesRead-1]) return traits_type::eof(); //wrong character
        _bytesRead--;
        return c;
    }

    std::streamsize xsgetn(char *s, std::streamsize n)
    {
        const auto available = this->showmanyc();
        if (available == 0) return traits_type::eof();
        n = std::min<std::streamsize>(n, available);
        std::memcpy(s, _payloadData.data()+_bytesRead, n);
        _bytesRead += n;
        return n;
    }

private:
    std::streamsize _bytesRead;
    std::vector<char_type> _payloadData;
};

/***********************************************************************
 * Wrapper calls for datagram interface
 **********************************************************************/
void sendDatagram(std::ostream &os, const Pothos::ObjectKwargs &reqArgs)
{
    Pothos::Object request(reqArgs);
    PRPCDatagramObuf(os, request);
}

Pothos::ObjectKwargs recvDatagram(std::istream &is)
{
    Pothos::Object reply;
    PRPCDatagramIbuf(is, reply);
    return reply.extract<Pothos::ObjectKwargs>();
}
