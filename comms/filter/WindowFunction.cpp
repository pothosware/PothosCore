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

#include <spuce/filters/design_window.h>
using namespace spuce;

////////////////////////////////////////////////////////////////////////
//Window function support:
//https://en.wikipedia.org/wiki/Window_function
// Window design is from spuce library
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
    std::string _win_type;
    int _length;
    double _atten;
    double _power;
    std::vector<double> _window;
    void reload(void);
};

static double rectangular(const size_t, const size_t)
{
    return 1.0;
}

WindowFunction::WindowFunction(void):
    _win_type("hann"),
    _length(21),
    _atten(60),
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
        _atten = args->getElement<double>(0); // This is beta for kaiser
    }
  	else if (name == "chebyshev")
		{
        if (args->size() != 1) throw Pothos::InvalidArgumentException("WindowFunction("+type+")", "expects format: chebyshev(atten)");
				_atten = args->getElement<double>(0);
		}
    else if (args->size() != 0) throw Pothos::InvalidArgumentException("WindowFunction("+type+")", name + " takes no arguments");

    //bind window function
		_win_type = name;
		_window = design_window(name, _length, _atten);

		if (_window.size() == 0)
			throw Pothos::InvalidArgumentException("WindowFunction::setType("+type+")", "unknown window name");
    this->reload();
}

void WindowFunction::setSize(const size_t length)
{
    if (length == _window.size()) return;
		_length = length;
		_window = design_window(_win_type, _length, _atten);
    this->reload();
}

void WindowFunction::reload(void)
{
    _power = 0.0;
    for (size_t n = 0; n < _window.size(); n++)
    {
        _power += _window[n]*_window[n];
    }
    _power = std::sqrt(_power/_length);
}

#include <Pothos/Managed.hpp>

static auto managedWindowFunction = Pothos::ManagedClass()
    .registerConstructor<WindowFunction>()
    .registerMethod(POTHOS_FCN_TUPLE(WindowFunction, setType))
    .registerMethod(POTHOS_FCN_TUPLE(WindowFunction, setSize))
    .registerMethod(POTHOS_FCN_TUPLE(WindowFunction, power))
    .registerMethod(POTHOS_FCN_TUPLE(WindowFunction, window))
    .commit("Pothos/Comms/WindowFunction");
