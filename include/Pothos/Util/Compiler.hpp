///
/// \file Util/Compiler.hpp
///
/// Compiler utilities for creating API control of various compilers.
///
/// \copyright
/// Copyright (c) 2014-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <vector>
#include <string>
#include <memory>

namespace Pothos {
namespace Util {

/*!
 * Compilation arguments.
 */
class POTHOS_API CompilerArgs
{
public:

    //! Create an empty compiler args
    CompilerArgs(void);

    //! Create a new args with default Pothos development libraries + includes
    static CompilerArgs defaultDevEnv(void);

    //! A list of source file paths
    std::vector<std::string> sources;

    //! A list of include paths
    std::vector<std::string> includes;

    //! A list of linkable libraries
    std::vector<std::string> libraries;

    //! A list of compiler flags
    std::vector<std::string> flags;
};

/*!
 * A simple compiler interface for creating modules
 */
class POTHOS_API Compiler
{
public:
    typedef std::shared_ptr<Compiler> Sptr;

    //! Compiler destructor
    virtual ~Compiler(void);

    /*!
     * Create a compiler instance given the name of the compiler.
     * Plugins for custom Compilers should be located in
     * the plugin registry: /util/compiler/[name]
     * \throws Exception if the factory function fails.
     * \param name the name of a compilier factory in the plugin tree
     * \return a new shared pointer to a compiler interface
     */
    static Sptr make(const std::string &name = "");

    /*!
     * Test if this compiler instance is usable.
     */
    virtual bool test(void) = 0;

    /*!
     * Compile a set of C++ sources into a runtime loadable module.
     * \throws Exception with message when a compilation fails
     * \param args the compiler arguments (flags and sources)
     * \return the path to the output binary/loadable module
     */
    virtual std::string compileCppModule(const CompilerArgs &args) = 0;

    /*!
     * Get an absolute path to a temporary file.
     * The file will be automatically removed
     * when the Compiler object is destroyed.
     * \param ext the file extension with dot (ex: ".obj")
     * \return the full path to a temporary file
     */
    const std::string &createTempFile(const std::string &ext = "");

private:
    std::vector<std::string> _tempFiles;
};

} //namespace Util
} //namespace Pothos
