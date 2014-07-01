// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <string>

//! throw ExprEvalError when evaluation fails
class ExprEvalError : public std::runtime_error
{
public:
    ExprEvalError(const std::string &what):
        std::runtime_error(what){}
};

/*!
 * The evaluation environment can evaluate and inspect expressions.
 * Expressions are considered to be valid C++ expressions involving
 * bools, integers, strings, floats, stl classes...
 */
class ExprEvalEnvironment
{
public:

    //! Make an evaluation environment that can eval on a remote client
    static ExprEvalEnvironment *make(Pothos::RemoteClient &client);

    /*!
     * Try to evaluate an expression in this environment.
     * \param expression a string expression
     * \throws ExprEvalError with the evaluation error message
     * \return a proxy object representing the evaluated expression
     */
    virtual Pothos::Proxy tryEval(const std::string &expression) = 0;

    /*!
     * Register a constant that will be used in other expressions.
     * For an expression like 2*x, then "x" would be a constant.
     */
    virtual void registerConstant(const std::string &key, const std::string &expression) = 0;
};
