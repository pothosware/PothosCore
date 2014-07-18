// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <QDockWidget>
#include <Poco/JSON/Object.h>

class BlockTreeTopWindow;

//! A top level dock widget with a block tree top window
class BlockTreeDock : public QDockWidget
{
    Q_OBJECT
public:
    BlockTreeDock(QWidget *parent);

signals:
    void addBlockEvent(const Poco::JSON::Object::Ptr &);

public slots:
    void activateFind(void);

private:
    BlockTreeTopWindow *_blockTreeWindow;
};
