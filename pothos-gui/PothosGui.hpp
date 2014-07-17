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
QWidget *makeBlockTree(QWidget *parent);
QWidget *makePropertiesPanel(QWidget *parent);
QWidget *makeMessageWindow(QWidget *parent);
QObject *makeBlockCache(QObject *parent);
QWidget *makeAffinityPanel(QWidget *parent);

//--- remote node factories --//
QWidget *makeRemoteNodesWindow(QWidget *parent);
QWidget *makeRemoteNodesTable(QWidget *parent);
QWidget *makePluginRegistryTree(QWidget *parent);
QWidget *makePluginModuleTree(QWidget *parent);
QWidget *makeSystemInfoTree(QWidget *parent);

//-- graph editor factories --//
QWidget *makeGraphEditorTabs(QWidget *parent);
QWidget *makeGraphEditor(QWidget *parent);
QWidget *makeGraphPage(QWidget *parent);
QWidget *makeGraphDraw(QWidget *parent);
