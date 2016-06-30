///
/// \file Util/EvalEnvironment.hpp
///
/// Expression evaluation utilities.
///
/// \copyright
/// Copyright (c) 2014-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Object.hpp>
#include <string>
#include <vector>
#include <memory>

namespace Pothos {
namespace Util {

/*!
 * The evaluation environment can evaluate and inspect expressions.
 * Expressions are considered to be valid C++ expressions involving
 * bools, integers, strings, floats, stl classes...
 */
class POTHOS_API EvalEnvironment
{
public:

    /*!
     * Create a new empty eval environment in a shared ptr.
     * This is a convenience factory for EvalEnvironment.
     */
    static std::shared_ptr<EvalEnvironment> make(void);

    /*!
     * Create an empty eval environment.
     * The environment can evaluate expressions with eval() and can
     * be extended with new constants using the registerConstant calls.
     */
    EvalEnvironment(void);

    /*!
     * Try to evaluate an expression in this environment.
     * \param expression a string expression
     * \throws Pothos::Exception with the evaluation error message
     * \return a proxy object representing the evaluated expression
     */
    Pothos::Object eval(const std::string &expression);

    /*!
     * Register a constant that will be used in other expressions.
     * For an expression like 2*x, then "x" would be a constant.
     */
    void registerConstantExpr(const std::string &key, const std::string &expr);

    /*!
     * Register a constant that will be used in other expressions.
     */
    void registerConstantObj(const std::string &key, const Pothos::Object &obj);

    /*!
     * Unregister a previously registered constant.
     */
    void unregisterConstant(const std::string &key);

private:
    struct Impl; std::shared_ptr<Impl> _impl;

    /*!
     * Split an expression given the tokenizer -- deals with quotes, nesting, escapes.
     * The null tokenizer means split the expression into top level container objects.
     */
    static std::vector<std::string> splitExpr(const std::string &expr, const char tokenizer = '\0');

    Pothos::Object _evalList(const std::string &expr);
    Pothos::Object _evalMap(const std::string &expr);
};

} //namespace Util
} //namespace Pothos
