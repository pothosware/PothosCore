// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QWidget>
#include <QString>
#include <QSettings>
#include <QMap>
#include <QIcon>
#include <QStringList>

class QAction;
class QMenu;

//--- access to globals --//
QSettings &getSettings(void);
QMap<QString, QAction *> &getActionMap(void);
QMap<QString, QMenu *> &getMenuMap(void);
QMap<QString, QObject *> &getObjectMap(void);
QStringList getRemoteNodeUris(void);

//--- main factories --//
QString makeIconPath(const QString &name);
QIcon makeIconFromTheme(const QString &name);
QWidget *makeMainWindow(QWidget *parent);
QWidget *makePropertiesPanel(QWidget *parent);
