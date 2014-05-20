// Copyright (c) 2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include <QScrollArea>
#include <iostream>

class GraphPage : public QScrollArea
{
    Q_OBJECT
public:
    GraphPage(QWidget *parent):
        QScrollArea(parent)
    {
        //create draw with parent so draw can connect to parent's signals
        //the draw will be reparented by scrollarea automatically by set widget
        this->setWidget(makeGraphDraw(parent));
    }
};

QWidget *makeGraphPage(QWidget *parent)
{
    return new GraphPage(parent);
};

#include "GraphPage.moc"
