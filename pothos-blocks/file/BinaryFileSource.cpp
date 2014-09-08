// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif //_MSC_VER
#include <stdio.h>
#include <cerrno>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#include <Poco/Logger.h>

/***********************************************************************
 * |PothosDoc Binary File Source
 *
 * Read data from a file and write it to an output stream on port 0.
 *
 * |category /Sources
 * |category /File IO
 * |keywords source binary file
 *
 * |param path[File Path] The path to the input file.
 * |default ""
 * |widget FileEntry(mode=open)
 *
 * |factory /blocks/binary_file_source()
 * |setter setFilePath(path)
 **********************************************************************/
class BinaryFileSource : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new BinaryFileSource();
    }

    BinaryFileSource(void):
        _fd(-1)
    {
        this->setupOutput(0, "byte");
        this->registerCall(this, POTHOS_FCN_TUPLE(BinaryFileSource, setFilePath));
    }

    void setFilePath(const std::string &path)
    {
        _path = path;
        //file was open -> close old fd, and open this new path
        if (_fd != -1)
        {
            this->deactivate();
            this->activate();
        }
    }

    void activate(void)
    {
        _fd = open(_path.c_str(), O_RDONLY | O_BINARY);
        if (_fd < 0)
        {
            poco_error_f4(Poco::Logger::get("BinaryFileSource"), "open(%s) returned %d -- %s(%d)", _path, _fd, std::string(strerror(errno)), errno);
        }
    }

    void deactivate(void)
    {
        close(_fd);
        _fd = -1;
    }

    void work(void)
    {
        #ifdef _MSC_VER
        //TODO use windows API to have timeout
        #else
        //setup timeval for timeout
        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = this->workInfo().maxTimeoutNs/1000; //ns->us

        //setup rset for timeout
        fd_set rset;
        FD_ZERO(&rset);
        FD_SET(_fd, &rset);

        //call select with timeout
        if (::select(_fd+1, &rset, NULL, NULL, &tv) <= 0) return this->yield();
        #endif

        auto out0 = this->output(0);
        auto ptr = out0->buffer().as<void *>();
        auto r = read(_fd, ptr, out0->elements());
        if (r >= 0) out0->produce(size_t(r));
        else
        {
            poco_error_f3(Poco::Logger::get("BinaryFileSource"), "read() returned %d -- %s(%d)", int(r), std::string(strerror(errno)), errno);
        }
    }

private:
    int _fd;
    std::string _path;
};

static Pothos::BlockRegistry registerBinaryFileSource(
    "/blocks/binary_file_source", &BinaryFileSource::make);
