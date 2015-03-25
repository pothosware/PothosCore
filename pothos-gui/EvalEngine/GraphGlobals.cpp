// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphGlobals.hpp"

GraphGlobals::GraphGlobals(void)
{
    return;
}

void GraphGlobals::setGlobalExpression(const QString &name, const QString &expression)
{
    _variableExprs[name] = expression;
    _evalTypeStrs[name] = "";
    _evalValueStrs[name] = "";
    _evalErrorMsgs[name] = "";
    _names.append(name);
}

const QString &GraphGlobals::getGlobalExpression(const QString &name) const
{
    return _variableExprs.at(name);
}

const QStringList &GraphGlobals::getGlobals(void) const
{
    return _names;
}

void GraphGlobals::setGlobalTypeStr(const QString &name, const QString &typeStr)
{
    _evalTypeStrs[name] = typeStr;
}

const QString &GraphGlobals::getGlobalTypeStr(const QString &name) const
{
    return _evalTypeStrs.at(name);
}

void GraphGlobals::setGlobalValueStr(const QString &name, const QString &valueStr)
{
    _evalValueStrs[name] = valueStr;
}

const QString &GraphGlobals::getGlobalValueStr(const QString &name) const
{
    return _evalValueStrs.at(name);
}

void GraphGlobals::setGlobalErrorMsg(const QString &name, const QString &errorMsg)
{
    _evalErrorMsgs[name] = errorMsg;
}

const QString &GraphGlobals::getGlobalErrorMsg(const QString &name) const
{
    return _evalErrorMsgs.at(name);
}

bool operator==(const GraphGlobals &lhs, const GraphGlobals &rhs)
{
    if (lhs.getGlobals() != rhs.getGlobals()) return false;
    for (const auto &name : lhs.getGlobals())
    {
        if (lhs.getGlobalExpression(name) != rhs.getGlobalExpression(name)) return false;
    }
    return true;
}
