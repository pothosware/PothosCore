// Copyright (c) 2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphEditor/GraphPage.hpp"
#include "GraphEditor/GraphDraw.hpp"

GraphPage::GraphPage(QWidget *parent):
    QScrollArea(parent)
{
    //create draw with parent so draw can connect to parent's signals
    //the draw will be reparented by scrollarea automatically by set widget
    this->setWidget(new GraphDraw(parent));
}
