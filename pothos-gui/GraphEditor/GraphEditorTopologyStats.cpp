// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //makeIconFromTheme
#include "GraphEditor/GraphEditor.hpp"
#include "EvalEngine/EvalEngine.hpp"
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QFuture>
#include <QFutureWatcher>
#include <QScrollArea>
#include <QtConcurrent/QtConcurrent>

class TopologyStatsDialog : public QDialog
{
    Q_OBJECT
public:

    TopologyStatsDialog(EvalEngine *evalEngine, QWidget *parent):
        QDialog(parent),
        _evalEngine(evalEngine),
        _topLayout(new QVBoxLayout(this)),
        _refreshButton(new QPushButton(makeIconFromTheme("view-refresh"), tr("Reload"), this)),
        _statsScroller(new QScrollArea(this)),
        _statsLabel(new QLabel(this)),
        _watcher(new QFutureWatcher<std::string>(this))
    {
        //create layouts
        this->setWindowTitle(tr("Topology stats viewer"));
        _topLayout->addWidget(_refreshButton);
        _topLayout->addWidget(_statsScroller);

        //setup the refresh button
        connect(_refreshButton, SIGNAL(pressed(void)), this, SLOT(handleClicked(void)));
        connect(_watcher, SIGNAL(finished(void)), this, SLOT(handleWatcherDone(void)));

        //setup the JSON stats label
        _statsLabel->setStyleSheet("QLabel{background:white;margin:1px;}");
        _statsLabel->setWordWrap(true);
        _statsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        _statsLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        _statsScroller->setWidget(_statsLabel);
        _statsScroller->setWidgetResizable(true);

        //initialize
        this->handleClicked();
    }

private slots:
    void handleClicked(void)
    {
        _watcher->setFuture(QtConcurrent::run(std::bind(&EvalEngine::getTopologyJSONStats, _evalEngine)));
    }

    void handleWatcherDone(void)
    {
        const auto jsonStats = _watcher->result();
        if (jsonStats.empty())
        {
            QMessageBox msgBox(QMessageBox::Critical, tr("Topology stats error"), tr("empty markup - is the topology active?"));
            msgBox.exec();
        }
        else
        {
            _statsLabel->setText(QString::fromStdString(jsonStats));
        }
    }

private:
    EvalEngine *_evalEngine;
    QVBoxLayout *_topLayout;
    QPushButton *_refreshButton;
    QScrollArea *_statsScroller;
    QLabel *_statsLabel;
    QFutureWatcher<std::string> *_watcher;
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
