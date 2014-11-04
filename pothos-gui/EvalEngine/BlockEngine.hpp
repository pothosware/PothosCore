// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Proxy/Environment.hpp>
#include <QObject>
#include <QString>
#include <memory>

class BlockEngine : public QObject
{
    Q_OBJECT
public:

    BlockEngine(QObject *parent);

    ~BlockEngine(void);

private:
};
