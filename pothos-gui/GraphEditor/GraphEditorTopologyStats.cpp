// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphEditor/GraphEditor.hpp"
#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

class TopologyStatsDialog : public QDialog
{
    Q_OBJECT
public:

    TopologyStatsDialog(EvalEngine *evalEngine, QWidget *parent):
        QDialog(parent),
        _evalEngine(evalEngine)
    {
        //create layouts
        this->setWindowTitle(tr("Topology stats viewer"));
    }

private:
    EvalEngine *_evalEngine;
};

void GraphEditor::handleShowTopologyStatsDialog(void)
{
    if (not this->isVisible()) return;

    //create the dialog
    auto dialog = new TopologyStatsDialog(_evalEngine, this);
    dialog->show();
    dialog->adjustSize();
    dialog->setWindowState(Qt::WindowMaximized);
    dialog->exec();
    delete dialog;
}

#include "GraphEditorTopologyStats.moc"
