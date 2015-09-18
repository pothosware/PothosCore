// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/System/Logger.hpp>
#include <Pothos/Plugin/Static.hpp> //static block
#include <Poco/Logger.h>
#include <Poco/Format.h>
#include <Poco/Environment.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/Platform.h>
#if POCO_OS == POCO_OS_WINDOWS_NT
#include <Poco/WindowsConsoleChannel.h>
#endif
#include <Poco/NullChannel.h>
#include <Poco/SimpleFileChannel.h>
#include <Poco/FormattingChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/SplitterChannel.h>
#include <Poco/Net/RemoteSyslogChannel.h>
#include <Poco/Net/RemoteSyslogListener.h>
#include <Poco/Net/DatagramSocket.h>
#include <Poco/SingletonHolder.h>
#include <Poco/String.h>
#include <Poco/AutoPtr.h>
#include <iostream>
#include <chrono>
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
      _newstream(nullptr),
      _lastPut(std::chrono::high_resolution_clock::now()),
      _flush(false)
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
        //has old data? force flush on sync regardless of newline
        if ((std::chrono::high_resolution_clock::now() - _lastPut) > std::chrono::nanoseconds(long(0.5*1e9))) _flush = true;
        _lastPut = std::chrono::high_resolution_clock::now();

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
        if (not _outbuf.empty() and (_outbuf.back() == '\n' or _flush))
        {
            if (_outbuf.back() == '\n') _outbuf = _outbuf.substr(0, _outbuf.size()-1);
            if (not _outbuf.empty()) _logger.log(Poco::Message("", _outbuf, this->guessPrio(_outbuf)));
            _outbuf.clear();
            _flush = false;
        }
        return 0;
    }

    /*!
     * Guess the priority using keywords in the front of the message.
     */
    Poco::Message::Priority guessPrio(const std::string &msg)
    {
        const auto searchStr = Poco::toLower(msg.substr(0, 25));
        if (searchStr.find("fatal") != std::string::npos) return Poco::Message::PRIO_FATAL;
        if (searchStr.find("critical") != std::string::npos) return Poco::Message::PRIO_CRITICAL;
        if (searchStr.find("error") != std::string::npos) return Poco::Message::PRIO_ERROR;
        if (searchStr.find("warning") != std::string::npos) return Poco::Message::PRIO_WARNING;
        if (searchStr.find("debug") != std::string::npos) return Poco::Message::PRIO_DEBUG;
        if (searchStr.find("trace") != std::string::npos) return Poco::Message::PRIO_TRACE;
        return Poco::Message::PRIO_INFORMATION;
    }

private:
    Poco::Logger &_logger;
    std::string _outbuf;
    std::streambuf*    _orgbuf;
    std::ostream&      _orgstream;
    std::unique_ptr<std::ostream>  _newstream;
    std::chrono::high_resolution_clock::time_point _lastPut;
    bool _flush;
};

/***********************************************************************
 * Public System Logger API implementation
 **********************************************************************/
static Poco::AutoPtr<Poco::Net::RemoteSyslogListener> listener;

std::string Pothos::System::Logger::startSyslogListener(void)
{
    std::lock_guard<std::mutex> lock(getSetupLoggerMutex());

    if (not listener)
    {
        //find an available udp port
        Poco::Net::DatagramSocket sock;
        sock.bind(Poco::Net::SocketAddress("[::]:0"));
        const auto port = sock.address().port();
        sock.close();

        //create a new listener and feed it the root channel
        listener = new Poco::Net::RemoteSyslogListener(port);
        listener->addChannel(Poco::Logger::get("").getChannel());
        listener->open();
    }

    //return the port number of the log service
    return listener->getProperty(Poco::Net::RemoteSyslogListener::PROP_PORT);
}

void Pothos::System::Logger::stopSyslogListener(void)
{
    std::lock_guard<std::mutex> lock(getSetupLoggerMutex());
    if (not listener) return;
    listener->close();
    listener = nullptr;
}

void Pothos::System::Logger::startSyslogForwarding(const std::string &addr)
{
    std::lock_guard<std::mutex> lock(getSetupLoggerMutex());
    Poco::AutoPtr<Poco::Channel> channel(new Poco::Net::RemoteSyslogChannel(addr, ""/*empty name*/));
    Poco::Logger::get("").setChannel(channel);
    Poco::Logger::get("").setLevel("trace"); //lowest level -> forward everything
}

void Pothos::System::Logger::forwardStdIoToLogging(const std::string &source)
{
    std::lock_guard<std::mutex> lock(getSetupLoggerMutex());
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
    const std::string logFomat = Poco::Environment::get("POTHOS_LOG_FORMAT", "%Y-%m-%d %H:%M:%S %s: %t");
    const std::string logTimeZone = Poco::Environment::get("POTHOS_LOG_TIMEZONE", "local"); //local or UTC

    //set the logging level at the chosen root of logging inheritance
    Poco::Logger::get("").setLevel(logLevel);

    //create and set the channel with the type string specified
    Poco::AutoPtr<Poco::Channel> channel;
    if (logChannel == "null") channel = new Poco::NullChannel();
    #if POCO_OS == POCO_OS_WINDOWS_NT
    else if (logChannel == "console") channel = new Poco::WindowsConsoleChannel();
    else if (logChannel == "color") channel = new Poco::WindowsColorConsoleChannel();
    #else
    else if (logChannel == "console") channel = new Poco::ConsoleChannel();
    else if (logChannel == "color") channel = new Poco::ColorConsoleChannel();
    #endif
    else if (logChannel == "file" and not logFile.empty())
    {
        channel = new Poco::SimpleFileChannel();
        channel->setProperty("path", logFile);
    }
    if (channel.isNull()) return;

    //setup formatting
    Poco::AutoPtr<Poco::Formatter> formatter(new Poco::PatternFormatter());
    formatter->setProperty("pattern", logFomat);
    formatter->setProperty("times", logTimeZone);
    Poco::AutoPtr<Poco::Channel> formattingChannel(new Poco::FormattingChannel(formatter, channel));
    Poco::AutoPtr<Poco::SplitterChannel> splitterChannel(new Poco::SplitterChannel());
    splitterChannel->addChannel(formattingChannel);
    Poco::Logger::get("").setChannel(splitterChannel);
}

void Pothos::System::Logger::setupDefaultLogging(void)
{
    std::lock_guard<std::mutex> lock(getSetupLoggerMutex());
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
