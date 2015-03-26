// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QString>
#include <QStringList>
#include <map>

/*!
 * Global variables for top level graph editor.
 * This is a light-weight copyable class used to
 * pass around global variable and eval information.
 */
class GraphGlobals
{
public:
    GraphGlobals(void);

    //! clear all globals
    void clear(void);

    //! Set a global variable and its expression -- overwrite existing
    void setGlobalExpression(const QString &name, const QString &expression);

    //! Get a global variable's expression
    const QString &getGlobalExpression(const QString &name) const;

    //! Get a list of all globals by name (order they were added)
    const QStringList &listGlobals(void) const;

    //! Set a type string for the evaluated variable.
    void setGlobalTypeStr(const QString &name, const QString &typeStr);

    //! Get the type string for an evaluated variable.
    const QString &getGlobalTypeStr(const QString &name) const;

    //! Set a value string for the evaluated variable.
    void setGlobalValueStr(const QString &name, const QString &valueStr);

    //! Get the value string for an evaluated variable.
    const QString &getGlobalValueStr(const QString &name) const;

    //! Set an error message for the evaluated variable.
    void setGlobalErrorMsg(const QString &name, const QString &errorMsg);

    //! Get the error message for an evaluated variable.
    const QString &getGlobalErrorMsg(const QString &name) const;

private:
    QStringList _names;
    std::map<QString, QString> _variableExprs;
    std::map<QString, QString> _evalTypeStrs;
    std::map<QString, QString> _evalValueStrs;
    std::map<QString, QString> _evalErrorMsgs;
};

bool operator==(const GraphGlobals &lhs, const GraphGlobals &rhs);
