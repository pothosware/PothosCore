// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QString>
#include <QSettings>
#include <QMap>
#include <QIcon>

class QAction;
class QMenu;
class QSplashScreen;

//! global settings
QSettings &getSettings(void);

//! global widget maps
QMap<QString, QAction *> &getActionMap(void);
QMap<QString, QMenu *> &getMenuMap(void);
QMap<QString, QObject *> &getObjectMap(void);

//! icon utils
QString makeIconPath(const QString &name);
QIcon makeIconFromTheme(const QString &name);

//! status message
void postStatusMessage(const QString &msg);

//! get global splash screen
QSplashScreen *getSplashScreen(void);
