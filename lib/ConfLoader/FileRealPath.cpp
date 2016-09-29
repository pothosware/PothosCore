// Copyright (c) 2016-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Config.hpp>
#include <cstdio> //FILENAME_MAX
#include <cstdlib> //realpath/_fullpath
#include <string>

/*!
 * Resolve the symbolic link.
 * This functionality should be in Poco::File
 */
std::string Pothos_FileRealPath(const std::string &path)
{
    char buff[FILENAME_MAX];
    char *result(nullptr);

    #ifdef _MSC_VER
    result = _fullpath(buff, path.c_str(), sizeof(buff));
    #else
    result = realpath(path.c_str(), buff);
    #endif

    return (result != nullptr)?result:path;
}
