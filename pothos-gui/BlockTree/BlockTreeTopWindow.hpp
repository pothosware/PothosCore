// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <QWidget>
#include <Poco/JSON/Object.h>

class QPushButton;
class BlockTreeWidget;

//! Top level widget for the block tree
class BlockTreeTopWindow : public QWidget
{
    Q_OBJECT
public:
    BlockTreeTopWindow(QWidget *parent);

signals:
    void addBlockEvent(const Poco::JSON::Object::Ptr &);

private slots:
    void handleAdd(void);

    void handleBlockDescEvent(const Poco::JSON::Object::Ptr &blockDesc, bool add);

private:
    QPushButton *_addButton;
    Poco::JSON::Object::Ptr _blockDesc;
    BlockTreeWidget *_blockTree;
};
