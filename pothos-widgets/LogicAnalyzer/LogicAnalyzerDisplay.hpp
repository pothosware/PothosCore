// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Framework.hpp>
#include <QWidget>

class LogicAnalyzerDisplay : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    LogicAnalyzerDisplay(void);

    ~LogicAnalyzerDisplay(void);

    QWidget *widget(void)
    {
        return this;
    }

    //! set the plotter's title
    void setTitle(const QString &title);
};

