// Copyright (c) 2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QScrollArea>

class GraphPage : public QScrollArea
{
    Q_OBJECT
public:
    GraphPage(QWidget *parent);
};
