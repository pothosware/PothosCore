// Copyright (c) 2013-2018 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"
#include <Pothos/System.hpp>
#include <iostream>

void PothosUtilBase::printSystemInfo(const std::string &, const std::string &)
{
    std::cout << "Lib Version: " << Pothos::System::getLibVersion() << std::endl;
    std::cout << "API Version: " << Pothos::System::getApiVersion() << std::endl;
    std::cout << "ABI Version: " << Pothos::System::getAbiVersion() << std::endl;
    std::cout << "Root Path: " << Pothos::System::getRootPath() << std::endl;
    std::cout << "Data Path: " << Pothos::System::getDataPath() << std::endl;
    std::cout << "User Data: " << Pothos::System::getUserDataPath() << std::endl;
    std::cout << "User Config: " << Pothos::System::getUserConfigPath() << std::endl;
    std::cout << "Runtime Library: " << Pothos::System::getPothosRuntimeLibraryPath() << std::endl;
    std::cout << "Util Executable: " << Pothos::System::getPothosUtilExecutablePath() << std::endl;
    std::cout << "Dev Include Path: " << Pothos::System::getPothosDevIncludePath() << std::endl;
    std::cout << "Dev Library Path: " << Pothos::System::getPothosDevLibraryPath() << std::endl;
}
