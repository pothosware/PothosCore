// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Config.hpp>
#include <Pothos/Exception.hpp>
#include <Poco/String.h>
#include <Poco/RegularExpression.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>
#include <cmath>
#include <complex>
#include <vector>
#include <string>
#include <functional>
#include <cassert>

////////////////////////////////////////////////////////////////////////
//Window function support:
//https://en.wikipedia.org/wiki/Window_function
////////////////////////////////////////////////////////////////////////
class WindowFunction
{
public:

    WindowFunction(void);

    //! Set the window function type (use common window function names)
    void setType(const std::string &name);

    /*!
     * Get the power of the window function
     */
    double power(void) const
    {
        return _power;
    }

    /*!
     * Get the window values (only update if length changed).
     */
    const std::vector<double> &window(void) const
    {
        return _window;
    }

    /*!
     * Set to a new window size.
     */
    void setSize(const size_t length);

private:
    std::function<double(const size_t, const size_t)> _calc;
    double _power;
    std::vector<double> _window;
    void reload(void);
};

static double rectangular(const size_t, const size_t)
{
    return 1.0;
}

static double hann(const size_t n, const size_t M)
{
    return 0.5 - 0.5*std::cos((2.0*M_PI*n)/(M-1));
}

static double hamming(const size_t n, const size_t M)
{
    return 0.54 - 0.46*std::cos((2.0*M_PI*n)/(M-1));
}

static double blackman(const size_t n, const size_t M)
{
    return 0.42 - 0.5*std::cos((2.0*M_PI*n)/M) + 0.08*std::cos((4.0*M_PI*n)/M);
}

static double bartlett(const size_t n, const size_t M)
{
    return (2.0/(M-1.0)) * (((M-1.0)/2.0) - std::abs(n - ((M-1.0)/2.0)));
}

static double flattop(const size_t n, const size_t M)
{
    static const double a0 = 1.0, a1 = 1.93, a2 = 1.29, a3 = 0.388, a4=0.028;
    return a0
        -a1*std::cos((2.0*M_PI*n)/(M-1))
        +a2*std::cos((4.0*M_PI*n)/(M-1))
        -a3*std::cos((6.0*M_PI*n)/(M-1))
        +a4*std::cos((8.0*M_PI*n)/(M-1));
}

//http://www.labbookpages.co.uk/audio/firWindowing.html#code
static double modZeroBessel(double x)
{
    int i;

    double x_2 = x/2;
    double num = 1;
    double fact = 1;
    double result = 1;

    for (i=1 ; i<20 ; i++) {
        num *= x_2 * x_2;
        fact *= i;
        result += num / (fact * fact);
    //		printf("%f %f %f\n", num, fact, result);
    }

    return result;
}

//http://www.labbookpages.co.uk/audio/firWindowing.html#code
static double kaiser(const size_t n, const size_t M, double beta)
{
    double m_2 = (double)(M-1) / 2.0;
    double denom = modZeroBessel(beta);					// Denominator of Kaiser function

    double val = ((n) - m_2) / m_2;
    val = 1 - (val * val);
    return modZeroBessel(beta * std::sqrt(val)) / denom;
}

WindowFunction::WindowFunction(void):
    _calc(&hann),
    _power(1.0f)
{
    return;
}

void WindowFunction::setType(const std::string &type)
{
    //parse the input
    Poco::RegularExpression::MatchVec matches;
    Poco::RegularExpression("^\\s*(\\w+)\\s*(\\((.*)\\))?\\s*$").match(type, 0, matches);
    if (matches.empty()) throw Pothos::InvalidArgumentException("WindowFunction("+type+")", "cant parse window type");

    //parse the args
    Poco::JSON::Array::Ptr args(new Poco::JSON::Array());
    if (matches.size() > 3)
    {
        auto argsStr = type.substr(matches[3].offset, matches[3].length);
        Poco::JSON::Parser p; p.parse("["+argsStr+"]");
        args = p.getHandler()->asVar().extract<Poco::JSON::Array::Ptr>();
    }

    //check input
    auto name = Poco::toLower(type.substr(matches[1].offset, matches[1].length));
    if (name == "kaiser")
    {
        if (args->size() != 1) throw Pothos::InvalidArgumentException("WindowFunction("+type+")", "expects format: kaiser(beta)");
    }
    else if (args->size() != 0) throw Pothos::InvalidArgumentException("WindowFunction("+type+")", name + " takes no arguments");

    //bind window function
    if (name == "rectangular") _calc = &rectangular;
    else if (name == "hann") _calc = &hann;
    else if (name == "hamming") _calc = &hamming;
    else if (name == "blackman") _calc = &blackman;
    else if (name == "bartlett") _calc = &bartlett;
    else if (name == "flattop") _calc = &flattop;
    else if (name == "kaiser") _calc = std::bind(&kaiser, std::placeholders::_1, std::placeholders::_2, args->getElement<double>(0));
    else throw Pothos::InvalidArgumentException("WindowFunction::setType("+type+")", "unknown window name");
    this->reload();
}

void WindowFunction::setSize(const size_t length)
{
    if (length == _window.size()) return;
    _window.resize(length);
    this->reload();
}

void WindowFunction::reload(void)
{
    _power = 0.0;
    const auto length = _window.size();
    for (size_t n = 0; n < length; n++)
    {
        _window[n] = _calc(n, length);
        _power += _window[n]*_window[n];
    }
    _power = std::sqrt(_power/length);
}

#include <Pothos/Managed.hpp>

static auto managedWindowFunction = Pothos::ManagedClass()
    .registerConstructor<WindowFunction>()
    .registerMethod(POTHOS_FCN_TUPLE(WindowFunction, setType))
    .registerMethod(POTHOS_FCN_TUPLE(WindowFunction, setSize))
    .registerMethod(POTHOS_FCN_TUPLE(WindowFunction, power))
    .registerMethod(POTHOS_FCN_TUPLE(WindowFunction, window))
    .commit("Pothos/Comms/WindowFunction");
