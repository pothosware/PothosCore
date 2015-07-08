// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //makeIconFromTheme
#include "GraphEditor/GraphEditor.hpp"
#include "EvalEngine/EvalEngine.hpp"
#include <QDialog>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QFuture>
#include <QFutureWatcher>
#include <QScrollArea>
#include <QTreeWidget>
#include <QtConcurrent/QtConcurrent>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>

class TopologyStatsDialog : public QDialog
{
    Q_OBJECT
public:

    TopologyStatsDialog(EvalEngine *evalEngine, QWidget *parent):
        QDialog(parent),
        _evalEngine(evalEngine),
        _topLayout(new QVBoxLayout(this)),
        _manualReloadButton(new QPushButton(makeIconFromTheme("view-refresh"), tr("Manual Reload"), this)),
        _autoReloadButton(new QPushButton(makeIconFromTheme("view-refresh"), tr("Automatic Reload"), this)),
        _statsScroller(new QScrollArea(this)),
        _statsTree(new QTreeWidget(this)),
        _timer(new QTimer(this)),
        _watcher(new QFutureWatcher<std::string>(this))
    {
        //create layouts
        this->setWindowTitle(tr("Topology stats viewer"));
        auto formsLayout = new QHBoxLayout();
        _topLayout->addLayout(formsLayout);
        formsLayout->addWidget(_manualReloadButton);
        formsLayout->addWidget(_autoReloadButton);
        _topLayout->addWidget(_statsScroller);

        //setup the refresh buttons
        _autoReloadButton->setCheckable(true);

        //setup the JSON stats tree
        _statsTree->setHeaderLabels(QStringList(tr("Block Stats")));
        _statsScroller->setWidget(_statsTree);
        _statsScroller->setWidgetResizable(true);

        //connect the signals
        connect(_manualReloadButton, SIGNAL(pressed(void)), this, SLOT(handleManualReload(void)));
        connect(_autoReloadButton, SIGNAL(clicked(bool)), this, SLOT(handleAutomaticReload(bool)));
        connect(_timer, SIGNAL(timeout(void)), this, SLOT(handleManualReload(void)));
        connect(_watcher, SIGNAL(finished(void)), this, SLOT(handleWatcherDone(void)));

        //initialize
        this->handleManualReload();
    }

private slots:
    void handleManualReload(void)
    {
        _watcher->setFuture(QtConcurrent::run(std::bind(&EvalEngine::getTopologyJSONStats, _evalEngine)));
    }

    void handleAutomaticReload(const bool enb)
    {
        if (enb) _timer->start(1000);
        else _timer->stop();
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
            Poco::JSON::Parser p; p.parse(jsonStats);
            auto topObj = p.getHandler()->asVar().extract<Poco::JSON::Object::Ptr>();
            std::vector<std::string> names; topObj->getNames(names);
            for (const auto &name : names)
            {
                const auto dataObj = topObj->getObject(name);

                auto &item = _statsItems[name];
                if (item == nullptr)
                {
                    const auto title = dataObj->getValue<std::string>("blockName");
                    item = new QTreeWidgetItem(QStringList(QString::fromStdString(title)));
                    _statsTree->addTopLevelItem(item);
                }

                auto &label = _statsLabels[name];
                if (label == nullptr)
                {
                    label = new QLabel(_statsTree);
                    label->setStyleSheet("QLabel{background:white;margin:1px;}");
                    label->setWordWrap(true);
                    label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
                    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
                    auto subItem = new QTreeWidgetItem(item);
                    _statsTree->setItemWidget(subItem, 0, label);
                }

                std::stringstream ss; dataObj->stringify(ss, 4);
                label->setText(QString::fromStdString(ss.str()).replace("\\/", "/"));
            }
        }
    }

private:
    EvalEngine *_evalEngine;
    QVBoxLayout *_topLayout;
    QPushButton *_manualReloadButton;
    QPushButton *_autoReloadButton;
    QScrollArea *_statsScroller;
    QTreeWidget *_statsTree;
    QTimer *_timer;
    QFutureWatcher<std::string> *_watcher;
    std::map<std::string, QTreeWidgetItem *> _statsItems;
    std::map<std::string, QLabel *> _statsLabels;
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
