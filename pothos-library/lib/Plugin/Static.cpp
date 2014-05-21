// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin/Static.hpp>
#include <Pothos/Exception.hpp>
#include <Poco/Logger.h>

void Pothos::Detail::safeInit(const std::string &clientAbi, const std::string &name, InitFcn init)
{
    if (clientAbi != Pothos::System::getAbiVersion())
    {
        poco_error_f3(Poco::Logger::get("Pothos.StaticBlock.safeInit"),
            "ABI mismatch when loading %s\n"
            "Client expected ABI %s but got %s from the library.",
            name, clientAbi, Pothos::System::getAbiVersion());
        return;
    }
    std::string error_msg;
    try
    {
        init();
        return;
    }
    catch(const Pothos::Exception &ex)
    {
        error_msg = ex.displayText();
    }
    catch(const std::exception &ex)
    {
        error_msg = ex.what();
    }
    catch(...)
    {
        error_msg = "unknown";
    }
    poco_error_f2(Poco::Logger::get("Pothos.StaticBlock.safeInit"),
        "loading %s, exception %s", name, error_msg);
}
