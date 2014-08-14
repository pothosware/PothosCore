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
#include <Poco/AutoPtr.h>
#include <iostream>
#include <memory>

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
        poco_information(_logger, std::string(msg, count));
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
        return 0;
    }

private:
    Poco::Logger &_logger;
    std::streambuf*    _orgbuf;
    std::ostream&      _orgstream;
    std::unique_ptr<std::ostream>  _newstream;
};

/***********************************************************************
 * Public System Logger API implementation
 **********************************************************************/
std::string Pothos::System::startSyslogListener(void)
{
    //find an available udp port
    Poco::Net::DatagramSocket sock;
    sock.bind(Poco::Net::SocketAddress("0.0.0.0:0"));
    const auto port = sock.address().port();
    sock.close();

    //create a new listener and feed it the root channel
    auto listener = new Poco::Net::RemoteSyslogListener(port);
    listener->addChannel(Poco::Logger::get("").getChannel());
    listener->open();

    //return the port number of the log service
    return listener->getProperty(Poco::Net::RemoteSyslogListener::PROP_PORT);
}

void Pothos::System::startSyslogForwarding(const std::string &addr)
{
    Poco::AutoPtr<Poco::Channel> channel(new Poco::Net::RemoteSyslogChannel(addr, ""/*empty name*/));
    Poco::Logger::get("").setChannel(channel);
    Poco::Logger::get("").setLevel("trace"); //lowest level -> forward everything
}

void Pothos::System::forwardStdIoToLogging(const std::string &source)
{
    static std::shared_ptr<InterceptStream> clogRedirected;
    clogRedirected.reset(new InterceptStream(std::clog, source));
}

void Pothos::System::setupDefaultLogging(void)
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

pothos_static_block(pothosLoggingInit)
{
    Pothos::System::setupDefaultLogging();
}

#include <Pothos/Managed.hpp>
struct DummyLoggerClass{};

static auto managedLogger = Pothos::ManagedClass()
    .registerClass<DummyLoggerClass>()
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::System, startSyslogListener))
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::System, startSyslogForwarding))
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::System, forwardStdIoToLogging))
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::System, setupDefaultLogging))
    .commit("Pothos/System/Logger");
