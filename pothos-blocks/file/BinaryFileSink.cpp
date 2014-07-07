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

#include <Poco/Logger.h>

/***********************************************************************
 * |PothosDoc Binary File Sink
 *
 * Read streaming data from port 0 and write the contents to a file.
 *
 * |category /Sinks
 * |category /File IO
 * |keywords sink binary file
 *
 * |param path[File Path] The path to the output file.
 * |default ""
 *
 * |factory /blocks/binary_file_sink()
 * |setter setFilePath(path)
 **********************************************************************/
class BinaryFileSink : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new BinaryFileSink();
    }

    BinaryFileSink(void):
        _fd(-1)
    {
        this->setupInput(0, "byte");
        this->registerCall(POTHOS_FCN_TUPLE(BinaryFileSink, setFilePath));
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
        _fd = open(_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
    }

    void deactivate(void)
    {
        close(_fd);
        _fd = -1;
    }

    void work(void)
    {
        auto in0 = this->input(0);
        auto buff = in0->buffer();
        if (not buff) return;
        auto ptr = buff.as<const void *>();
        auto r = write(_fd, ptr, buff.length);
        if (r >= 0) in0->consume(size_t(r));
        else
        {
            poco_error_f3(Poco::Logger::get("BinaryFileSink"), "write() returned %d -- %s(%d)", int(r), std::string(strerror(errno)), errno);
        }
    }

private:
    int _fd;
    std::string _path;
};

static Pothos::BlockRegistry registerBinaryFileSink(
    "/blocks/binary_file_sink", &BinaryFileSink::make);
