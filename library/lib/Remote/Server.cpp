// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Remote.hpp>
#include <Pothos/System/Paths.hpp>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>
#include <Poco/Process.h>
#include <Poco/StringTokenizer.h>
#include <Poco/URI.h>
#include <Poco/String.h>
#include <Poco/Message.h>
#include <Poco/Net/RemoteSyslogChannel.h>
#include <thread>
#include <cassert>

std::string Pothos::RemoteServer::getLocatorPort(void)
{
    return "16415";
}

struct Pothos::RemoteServer::Impl
{
    Impl(const Poco::ProcessHandle &ph):
        ph(ph)
    {
        return;
    }

    ~Impl(void)
    {
        client = RemoteClient(); //reset
        Poco::Process::kill(ph);
        outPipe.close();
        errPipe.close();
        if (outThread.joinable()) outThread.join();
        if (errThread.joinable()) errThread.join();
    }

    Poco::ProcessHandle ph;
    std::string actualPort;
    RemoteClient client;
    std::string uriStr;

    //members used for syslog forwarding
    Poco::Pipe outPipe, errPipe;
    std::thread outThread, errThread;
};

Pothos::RemoteServer::RemoteServer(void)
{
    assert(not *this);
}

Pothos::RemoteServer::RemoteServer(const std::string &uriStr, const bool closePipes)
{
    //validate the URI first
    if (not uriStr.empty()) POTHOS_EXCEPTION_TRY
    {
        Poco::URI uri(uriStr);
        if (uri.getScheme() != "tcp") throw InvalidArgumentException("unsupported URI scheme");
    }
    POTHOS_EXCEPTION_CATCH(const Exception &ex)
    {
        throw RemoteServerError("Pothos::RemoteServer("+uriStr+")", ex);
    }

    //create args
    Poco::Process::Args args;
    args.push_back("--require-active");
    args.push_back("--proxy-server");
    args.push_back(uriStr);

    //launch
    Poco::Pipe outPipe, errPipe;
    Poco::Process::Env env;
    Poco::ProcessHandle ph(Poco::Process::launch(
        System::getPothosUtilExecutablePath(),
        args, nullptr, &outPipe, &errPipe, env));
    _impl.reset(new Impl(ph));
    _impl->uriStr = uriStr;

    //read back the port from the server
    Poco::PipeInputStream is(outPipe);
    while (is.good() and not is.eof())
    {
        std::string line; std::getline(is, line);
        if (line.empty()) continue;
        const Poco::StringTokenizer tok(line, " ");
        if (tok.count() >= 2 and tok[0] == "Port:")
        {
            _impl->actualPort = tok[1];
            break;
        }
    }
    const bool readPortError = _impl->actualPort.empty();

    //close pipes to not overfill and backup
    if (closePipes or readPortError)
    {
        outPipe.close();
        errPipe.close();
    }
    //save our pipes to be used with syslog
    else
    {
        _impl->outPipe = outPipe;
        _impl->errPipe = errPipe;
    }

    //An error spawning the process above could have caused a problem,
    //which can be detected when the parsing of the port fails.
    if (readPortError)
    {
        _impl.reset(); //kills process

        //collect error message
        Poco::PipeInputStream ies(errPipe);
        const std::string errMsg = std::string(
            std::istreambuf_iterator<char>(ies),
            std::istreambuf_iterator<char>());

        throw Pothos::RemoteServerError("Pothos::RemoteServer("+uriStr+")", errMsg);
    }

    //Try to connect to the server.
    //Store an open connection within this server wrapper.
    {
        Poco::URI uri(uriStr);
        uri.setPort(std::stoul(this->getActualPort()));
        _impl->client = RemoteClient(uri.toString());
    }
}

const std::string &Pothos::RemoteServer::getUri(void) const
{
    assert(_impl);
    return _impl->uriStr;
}

Pothos::RemoteServer::operator bool(void) const
{
    return bool(_impl);
}

std::string Pothos::RemoteServer::getActualPort(void) const
{
    assert(_impl);
    return _impl->actualPort;
}

/***********************************************************************
 * syslog forwarding implementation
 **********************************************************************/

//! Guess the priority using keywords in the front of the message.
static Poco::Message::Priority guessPrio(const std::string &msg)
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

//! Worker thread operation to read the pipe and forward to the syslog channel
static void pipeToSyslogWorker(Poco::AutoPtr<Poco::Channel> channel, const Poco::Pipe &pipe, const std::string &source)
{
    Poco::PipeInputStream is(pipe);
    try
    {
        std::string line;
        while (is.good())
        {
            std::getline(is, line);
            if (line.empty()) continue;
            channel->log(Poco::Message(source, line, guessPrio(line)));
        }
    }
    catch (...){}
}

//! Create the syslog channel and forwarding threads
void Pothos::RemoteServer::startSyslogForwarding(const std::string &addr, const std::string &source)
{
    assert(_impl);
    Poco::AutoPtr<Poco::Channel> channel(new Poco::Net::RemoteSyslogChannel(addr, ""/*empty name*/));
    _impl->outThread = std::thread(std::bind(&pipeToSyslogWorker, channel, _impl->outPipe, source));
    _impl->errThread = std::thread(std::bind(&pipeToSyslogWorker, channel, _impl->errPipe, source));
}

#include <Pothos/Managed.hpp>

static auto managedRemoteServer = Pothos::ManagedClass()
    .registerConstructor<Pothos::RemoteServer>()
    .registerConstructor<Pothos::RemoteServer, std::string>()
    .registerConstructor<Pothos::RemoteServer, std::string, bool>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::RemoteServer, getActualPort))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::RemoteServer, startSyslogForwarding))
    .commit("Pothos/RemoteServer");
