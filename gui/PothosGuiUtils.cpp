// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp"
#include <Pothos/System.hpp>
#include <Poco/Path.h>
#include <Poco/SingletonHolder.h>
#include <QSplashScreen>
#include <QApplication>
#include <QFormLayout>

QMap<QString, QAction *> &getActionMap(void)
{
    static Poco::SingletonHolder<QMap<QString, QAction *>> sh;
    return *sh.get();
}

QMap<QString, QMenu *> &getMenuMap(void)
{
    static Poco::SingletonHolder<QMap<QString, QMenu *>> sh;
    return *sh.get();
}

QMap<QString, QObject *> &getObjectMap(void)
{
    static Poco::SingletonHolder<QMap<QString, QObject *>> sh;
    return *sh.get();
}

QSettings &getSettings(void)
{
    static QSettings *settings = nullptr;
    if (settings == nullptr)
    {
        Poco::Path path(Pothos::System::getUserConfigPath());
        path.append("PothosGui.conf");
        const auto settingsPath = QString::fromStdString(path.toString());
        settings = new QSettings(settingsPath, QSettings::IniFormat);
    }
    return *settings;
}

QString makeIconPath(const QString &name)
{
    Poco::Path path(Pothos::System::getDataPath());
    path.append("icons");
    if (not name.isEmpty()) path.append(name.toStdString());
    return QString::fromStdString(path.toString());
}

QIcon makeIconFromTheme(const QString &name)
{
    return QIcon::fromTheme(name, QIcon(makeIconPath(name+".png")));
}

void postStatusMessage(const QString &msg)
{
    getSplashScreen()->showMessage(msg, Qt::AlignLeft | Qt::AlignBottom);
    QApplication::instance()->processEvents();
}

QSplashScreen *getSplashScreen(void)
{
    static QSplashScreen *splash = nullptr;
    if (splash == nullptr)
    {
        QPixmap pixmap(makeIconPath("PothosSplash.png"));
        splash = new QSplashScreen(pixmap);
    }
    return splash;
}

QFormLayout *makeFormLayout(QWidget *parent)
{
    auto layout = new QFormLayout(parent);
    layout->setRowWrapPolicy(QFormLayout::DontWrapRows);
    layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    layout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    layout->setLabelAlignment(Qt::AlignLeft);
    return layout;
}
