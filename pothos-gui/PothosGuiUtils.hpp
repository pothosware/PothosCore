// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QString>
#include <QSettings>
#include <QMap>
#include <QIcon>
#include <QColor>
#include <string>

class QAction;
class QMenu;

//! global settings
QSettings &getSettings(void);

//! global widget maps
QMap<QString, QAction *> &getActionMap(void);
QMap<QString, QMenu *> &getMenuMap(void);
QMap<QString, QObject *> &getObjectMap(void);

//! icon utils
QString makeIconPath(const QString &name);
QIcon makeIconFromTheme(const QString &name);

//! Convert the type name to a displayable color
QColor typeStrToColor(const std::string &typeStr);
