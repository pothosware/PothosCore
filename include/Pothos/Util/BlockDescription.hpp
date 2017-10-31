///
/// \file Util/BlockDescription.hpp
///
/// Utility class for parsing block description markup.
/// https://github.com/pothosware/pothos/wiki/BlockDescriptionMarkup
///
/// \copyright
/// Copyright (c) 2016-2017 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <iosfwd>
#include <string>
#include <vector>
#include <memory>
#include <cstddef>

namespace Pothos {
namespace Util {

/*!
 * Parse multiple sources for block description markup
 * and manages the resulting JSON descriptions.
 */
class POTHOS_API BlockDescriptionParser
{
public:

    //! Create an empty block description parser
    BlockDescriptionParser(void);

    /*!
     * Feed the parser a source file with embedded markup.
     * \param is an input stream like an open file
     * \throws SyntaxException if the parsing failed
     */
    void feedStream(std::istream &is);

    /*!
     * Feed the parser a source file with embedded markup.
     * \param filePath the path to a source file
     * \throws SyntaxException if the parsing failed
     * \throws FileException for file access errors
     */
    void feedFilePath(const std::string &filePath);

    /*!
     * Get all factories that were discovered when parsing.
     */
    std::vector<std::string> listFactories(void) const;

    /*!
     * Get a JSON array containing all block descriptions.
     * \return a string containing a JSON array
     */
    std::string getJSONArray(const size_t indent = 0) const;

    /*!
     * Get a JSON object for a single block description.
     * \param factoryPath a discovered factory path
     * \param indent indentation level for formatting
     * \return a string containing a JSON object
     */
    std::string getJSONObject(const std::string &factoryPath, const size_t indent = 0) const;

private:
    struct Impl;
    std::shared_ptr<Impl> _impl;
};

} //namespace Util
} //namespace Pothos
