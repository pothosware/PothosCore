// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"
#include <Pothos/Testing.hpp>
#include <Pothos/Plugin/Loader.hpp>
#include <Pothos/System/Paths.hpp>
#include <Poco/Process.h>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>
#include <Poco/String.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <future>
#include <cctype>
#include <algorithm> //max

struct SelfTestResults
{
    std::vector<std::string> testsPassed;
    std::vector<std::string> testsFailed;
};

static std::string collectVerbose(const Poco::Pipe &pipe)
{
    size_t maxLen = 0;
    Poco::PipeInputStream is(pipe);
    std::vector<std::string> lines;
    try
    {
        std::string line;
        while (is.good())
        {
            std::getline(is, line);
            while (not line.empty() and std::isspace(line.back()))
            {
                line.pop_back();
            }
            if (line.empty()) continue;
            maxLen = std::max(maxLen, line.length());
            lines.push_back(line);
        }
    }
    catch (...){}
    std::ostringstream ss;
    ss << " +-" << std::string(maxLen, '-') << "-+" << std::endl;
    for (const auto &line : lines)
    {
        const size_t padLen = maxLen-line.length();
        ss << " | " << line << std::string(padLen, ' ') << " |" << std::endl;
    }
    ss << " +-" << std::string(maxLen, '-') << "-+" << std::endl;
    return ss.str();
}

static bool spawnSelfTestOneProcess(const std::string &path)
{
    std::cout << "Testing " << path << "... " << std::flush;
    const int success = 200;

    //create args
    Poco::Process::Args args;
    args.push_back("--self-test1");
    args.push_back(path);
    args.push_back("--success-code");
    args.push_back(std::to_string(success));

    //launch
    Poco::Process::Env env;
    Poco::Pipe outPipe; //no fwd stdio
    Poco::ProcessHandle ph(Poco::Process::launch(
        Pothos::System::getPothosUtilExecutablePath(),
        args, nullptr, &outPipe, &outPipe, env));

    std::future<std::string> verboseFuture(std::async(std::launch::async, &collectVerbose, outPipe));
    const bool ok = (ph.wait() == success);
    std::cout << ((ok)? "success!" : "FAIL!") << std::endl;

    outPipe.close();
    verboseFuture.wait();
    if (not ok) std::cout << verboseFuture.get();

    return ok;
}

static void runPluginSelfTestsR(const Pothos::PluginPath &path, SelfTestResults &results)
{
    //run the test found at path
    if (not Pothos::PluginRegistry::empty(path))
    {
        auto plugin = Pothos::PluginRegistry::get(path);
        if (plugin.getObject().type() == typeid(std::shared_ptr<Pothos::TestingBase>))
        {
            if (spawnSelfTestOneProcess(path.toString()))
            {
                results.testsPassed.push_back(path.toString());
            }
            else
            {
                results.testsFailed.push_back(path.toString());
            }
        }
    }
    //iterate on the subtree stuff
    auto nodes = Pothos::PluginRegistry::list(path);
    for (auto it = nodes.begin(); it != nodes.end(); it++)
    {
        runPluginSelfTestsR(path.join(*it), results);
    }
}

void PothosUtilBase::selfTestOne(const std::string &, const std::string &path)
{
    Pothos::ScopedInit init;

    auto plugin = Pothos::PluginRegistry::get(path);
    auto test = plugin.getObject().extract<std::shared_ptr<Pothos::TestingBase>>();
    std::cout << "Testing " << plugin.getPath().toString() << "..." << std::endl;
    try
    {
        test->runTests();
        std::cout << "success!" << std::endl;
    }
    catch(...)
    {
        throw;
    }
}

void PothosUtilBase::selfTests(const std::string &, const std::string &path)
{
    Pothos::ScopedInit init;

    SelfTestResults results;
    runPluginSelfTestsR(path.empty()? "/" : path, results);
    std::cout << std::endl;

    const size_t totalTests = results.testsPassed.size() + results.testsFailed.size();
    if (results.testsFailed.empty())
    {
        std::cout << "All " << totalTests << " tests passed!" << std::endl;
    }
    else
    {
        std::cout << "Failed " << results.testsFailed.size() << " out of " << totalTests << " tests" << std::endl;
        for (auto it = results.testsFailed.begin(); it != results.testsFailed.end(); it++)
        {
            std::cout << "  FAIL: " << *it << std::endl;
        }
        std::cout << std::endl;
        throw Pothos::Exception("Failures occurred in self test suite.");
    }
    std::cout << std::endl;
}
