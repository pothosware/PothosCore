// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/System/Logger.hpp>
#include <Pothos/Plugin/Static.hpp> //static block
#include <Poco/Logger.h>
#include <Poco/Format.h>
#include <Poco/Environment.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/NullChannel.h>
#include <Poco/SimpleFileChannel.h>
#include <Poco/FormattingChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/Net/RemoteSyslogChannel.h>
#include <Poco/Net/RemoteSyslogListener.h>
#include <Poco/Net/DatagramSocket.h>
#include <Poco/SingletonHolder.h>
#include <Poco/AutoPtr.h>
#include <iostream>
#include <memory>
#include <mutex>

/***********************************************************************
 * mutex for protecting logger methods
 **********************************************************************/
static std::mutex &getSetupLoggerMutex(void)
{
    static Poco::SingletonHolder<std::mutex> sh;
    return *sh.get();
}

/***********************************************************************
 * InterceptStream custom streambuf to redirect to logger
 **********************************************************************/
class InterceptStream : public std::streambuf
{
public:
    InterceptStream(std::ostream& stream, const std::string &source):
      _logger(Poco::Logger::get(source)),
      _orgstream(stream),
      _newstream(nullptr)
    {
        //Swap the the old buffer in ostream with this buffer.
        _orgbuf=_orgstream.rdbuf(this);
        //Create a new ostream that we set the old buffer in
        std::unique_ptr<std::ostream> os(new std::ostream(_orgbuf));
        _newstream.swap(os);
    }

    ~InterceptStream(void)
    {
        _orgstream.rdbuf(_orgbuf);//Restore old buffer
    }
protected:
    virtual std::streamsize xsputn(const char *msg, std::streamsize count)
    {
        //Output to new stream with old buffer (to e.g. screen [std::cout])
        //_newstream->write(msg, count);
        //Output to logger
        _outbuf.append(msg, count);
        return count;
    }

    int_type overflow(int_type c)
    {
        if(not traits_type::eq_int_type(c, traits_type::eof()))
        {
            char_type const t = traits_type::to_char_type(c);
            this->xsputn(&t, 1);
        }
        return not traits_type::eof();
    }

    int sync(void)
    {
        if (not _outbuf.empty() and _outbuf.back() == '\n')
        {
            poco_information(_logger, _outbuf);
            _outbuf.clear();
        }
        return 0;
    }

private:
    Poco::Logger &_logger;
    std::string _outbuf;
    std::streambuf*    _orgbuf;
    std::ostream&      _orgstream;
    std::unique_ptr<std::ostream>  _newstream;
};

/***********************************************************************
 * Public System Logger API implementation
 **********************************************************************/
struct RemoteSyslogListenerCloser
{
    RemoteSyslogListenerCloser(Poco::Net::RemoteSyslogListener *listener):
        listener(listener)
    {
        return;
    }
    ~RemoteSyslogListenerCloser(void)
    {
        if (listener) listener->close();
        listener = nullptr;
    }
    Poco::AutoPtr<Poco::Net::RemoteSyslogListener> listener;
};

std::string Pothos::System::Logger::startSyslogListener(void)
{
    std::unique_lock<std::mutex> lock(getSetupLoggerMutex());
    static Poco::AutoPtr<Poco::Net::RemoteSyslogListener> listener;
    static std::shared_ptr<RemoteSyslogListenerCloser> closer;

    if (not listener)
    {
        //find an available udp port
        Poco::Net::DatagramSocket sock;
        sock.bind(Poco::Net::SocketAddress("0.0.0.0:0"));
        const auto port = sock.address().port();
        sock.close();

        //create a new listener and feed it the root channel
        listener = new Poco::Net::RemoteSyslogListener(port);
        listener->addChannel(Poco::Logger::get("").getChannel());
        listener->open();
        closer.reset(new RemoteSyslogListenerCloser(listener));
    }

    //return the port number of the log service
    return listener->getProperty(Poco::Net::RemoteSyslogListener::PROP_PORT);
}

void Pothos::System::Logger::startSyslogForwarding(const std::string &addr)
{
    std::unique_lock<std::mutex> lock(getSetupLoggerMutex());
    Poco::AutoPtr<Poco::Channel> channel(new Poco::Net::RemoteSyslogChannel(addr, ""/*empty name*/));
    Poco::Logger::get("").setChannel(channel);
    Poco::Logger::get("").setLevel("trace"); //lowest level -> forward everything
}

void Pothos::System::Logger::forwardStdIoToLogging(const std::string &source)
{
    std::unique_lock<std::mutex> lock(getSetupLoggerMutex());
    static std::shared_ptr<InterceptStream> clogRedirected;
    clogRedirected.reset(new InterceptStream(std::clog, source));
    static std::shared_ptr<InterceptStream> coutRedirected;
    coutRedirected.reset(new InterceptStream(std::cout, source));
    static std::shared_ptr<InterceptStream> cerrRedirected;
    cerrRedirected.reset(new InterceptStream(std::cerr, source));
}

static void __setupDefaultLogging(void)
{
    const std::string logLevel = Poco::Environment::get("POTHOS_LOG_LEVEL", "notice");
    const std::string logChannel = Poco::Environment::get("POTHOS_LOG_CHANNEL", "color");
    const std::string logFile = Poco::Environment::get("POTHOS_LOG_FILE", "pothos.log");

    //set the logging level at the chosen root of logging inheritance
    Poco::Logger::get("").setLevel(logLevel);

    //create and set the channel with the type string specified
    Poco::AutoPtr<Poco::Channel> channel;
    if (logChannel == "null") channel = new Poco::NullChannel();
    else if (logChannel == "console") channel = new Poco::ConsoleChannel();
    else if (logChannel == "color") channel = new Poco::ColorConsoleChannel();
    else if (logChannel == "file" and not logFile.empty())
    {
        channel = new Poco::SimpleFileChannel();
        channel->setProperty("path", logFile);
    }
    if (channel.isNull()) return;

    //setup formatting
    Poco::AutoPtr<Poco::Formatter> formatter(new Poco::PatternFormatter());
    formatter->setProperty("pattern", "%Y-%m-%d %H:%M:%S %s: %t");
    Poco::AutoPtr<Poco::Channel> formattingChannel(new Poco::FormattingChannel(formatter, channel));
    Poco::Logger::get("").setChannel(formattingChannel);
}

void Pothos::System::Logger::setupDefaultLogging(void)
{
    std::unique_lock<std::mutex> lock(getSetupLoggerMutex());
    __setupDefaultLogging();
}

pothos_static_block(pothosLoggingInit)
{
    __setupDefaultLogging();
}

#include <Pothos/Managed.hpp>

static auto managedSystemLogger = Pothos::ManagedClass()
    .registerClass<Pothos::System::Logger>()
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::System::Logger, startSyslogListener))
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::System::Logger, startSyslogForwarding))
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::System::Logger, forwardStdIoToLogging))
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::System::Logger, setupDefaultLogging))
    .commit("Pothos/System/Logger");
