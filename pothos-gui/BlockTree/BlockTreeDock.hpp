// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QDockWidget>
#include <Poco/JSON/Object.h>

class QPushButton;
class QLineEdit;
class BlockTreeWidget;

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

private slots:
    void handleAdd(void);

    void handleBlockDescEvent(const Poco::JSON::Object::Ptr &blockDesc, bool add);

private:
    QPushButton *_addButton;
    QLineEdit *_searchBox;
    Poco::JSON::Object::Ptr _blockDesc;
    BlockTreeWidget *_blockTree;
};
