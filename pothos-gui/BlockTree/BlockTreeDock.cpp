// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <BlockTree/BlockTreeDock.hpp>
#include <BlockTree/BlockTreeTopWindow.hpp>
#include <QAction>

BlockTreeDock::BlockTreeDock(QWidget *parent):
    QDockWidget(parent),
    _blockTreeWindow(new BlockTreeTopWindow(this))
{
    this->setObjectName("BlockTreeDock");
    this->setWindowTitle(tr("Block Tree"));
    this->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    this->setWidget(_blockTreeWindow);

    connect(_blockTreeWindow, SIGNAL(addBlockEvent(const Poco::JSON::Object::Ptr &)),
        this, SIGNAL(addBlockEvent(const Poco::JSON::Object::Ptr &)));
}

void BlockTreeDock::activateFind(void)
{
    this->show();
    this->raise();
    _blockTreeWindow->activateFind();
}
