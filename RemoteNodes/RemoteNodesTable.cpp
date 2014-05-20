// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include <QTableWidget>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QToolButton>
#include <QSignalMapper>
#include <QTimer>
#include <QToolTip>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <Pothos/Remote.hpp>
#include <Poco/DateTimeFormatter.h>
#include <map>
#include <iostream>
#include <functional> //std::bind

/***********************************************************************
 * Custom line editor for node URI entry
 **********************************************************************/
class NodeUriQLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    NodeUriQLineEdit(QWidget *parent):
        QLineEdit(parent)
    {
        this->setPlaceholderText("Click to enter a new Node URI");

        connect(
            this, SIGNAL(returnPressed(void)),
            this, SLOT(handleReturnPressed(void)));
    }

    QSize minimumSizeHint(void) const
    {
        auto size = QLineEdit::minimumSizeHint();
        size.setWidth(50);
        return size;
    }

public slots:
    void handleReturnPressed(void)
    {
        if (this->text() == getDefaultUri()) return;
        if (this->text().isEmpty()) return;
        emit handleUriEntered(this->text());
        this->setText("");
    }

    void handleErrorMessage(const QString &errMsg)
    {
        QToolTip::showText(this->mapToGlobal(QPoint()), "<font color=\"red\">"+errMsg+"</font>");
    }

signals:
    void handleUriEntered(const QString &);

private:
    void focusInEvent(QFocusEvent *)
    {
        if (this->text().isEmpty())
        {
            this->setText(getDefaultUri());
        }
    }

    void focusOutEvent(QFocusEvent *)
    {
        if (this->text() == getDefaultUri())
        {
            this->setText("");
        }
    }

    static QString getDefaultUri(void)
    {
        return "tcp://";
    }
};

/***********************************************************************
 * helper to disable cell editing for RO cells
 **********************************************************************/
static void disableEdit(QTableWidgetItem *i)
{
    if (i == nullptr) return;
    auto flags = i->flags();
    flags &= ~Qt::ItemIsEditable;
    i->setFlags(flags);
}

/***********************************************************************
 * helper to make tool buttons for add/remove nodes
 **********************************************************************/
static QToolButton *makeToolButton(QWidget *parent, const QString &theme)
{
    auto tb = new QToolButton(parent);
    tb->setCursor(Qt::PointingHandCursor);
    tb->setFocusPolicy(Qt::NoFocus);
    tb->setIcon(makeIconFromTheme(theme));
    tb->setStyleSheet("background: transparent; border: none;");
    return tb;
}

/***********************************************************************
 * node table implementation
 **********************************************************************/
class RemoteNodesQTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    RemoteNodesQTableWidget(QWidget *parent):
        QTableWidget(parent),
        _removeMapper(new QSignalMapper(this)),
        _timer(new QTimer(this)),
        _watcher(new QFutureWatcher<std::vector<Pothos::RemoteNode>>(this))
    {
        this->setColumnCount(nCols);
        size_t col = 0;
        this->setHorizontalHeaderItem(col++, new QTableWidgetItem(tr("Action")));
        this->setHorizontalHeaderItem(col++, new QTableWidgetItem(tr("URI")));
        this->setHorizontalHeaderItem(col++, new QTableWidgetItem(tr("Name")));
        this->setHorizontalHeaderItem(col++, new QTableWidgetItem(tr("Last Access")));
        this->reloadTable();

        connect(
            _removeMapper, SIGNAL(mapped(const QString &)),
            this, SLOT(handleRemove(const QString &)));
        connect(
            _timer, SIGNAL(timeout(void)),
            this, SLOT(handleUpdateStatus(void)));
        connect(
            _watcher, SIGNAL(finished(void)),
            this, SLOT(handleNodeQueryComplete(void)));
        connect(
            this, SIGNAL(cellClicked(int, int)),
            this, SLOT(handleCellClicked(int, int)));
        _timer->start(5000/*ms*/);
    }

signals:
    void handleErrorMessage(const QString &);
    void nodeInfoRequest(const Pothos::RemoteNode &);

private slots:

    void handleCellClicked(const int row, const int col)
    {
        if (col < 1) return;
        for (const auto &entry : _keyToRow)
        {
            if (entry.second != size_t(row)) continue;
            const auto &node = _keyToNode.at(entry.first);
            if (node.isOnline()) emit nodeInfoRequest(node);
            else emit handleErrorMessage(tr("node %1 is offline").arg(QString::fromStdString(node.getName())));
        }
    }

    void handleRemove(const QString &key)
    {
        try
        {
            Pothos::RemoteNode::fromKey(key.toStdString()).removeFromRegistry();
            this->reloadTable();
        }
        catch(const Pothos::Exception &ex)
        {
            emit handleErrorMessage(QString::fromStdString(ex.displayText()));
        }
    }

    void handleAdd(const QString &uri)
    {
        try
        {
            Pothos::RemoteNode(uri.toStdString()).addToRegistry();
            this->reloadTable();
        }
        catch(const Pothos::Exception &ex)
        {
            emit handleErrorMessage(QString::fromStdString(ex.displayText()));
        }
    }

    void handleNodeQueryComplete(void)
    {
        this->reloadRows(_watcher->result());
    }

    void handleUpdateStatus(void)
    {
        //did the keys change?
        {
            auto keys = Pothos::RemoteNode::listRegistryKeys();
            bool changed = false;
            if (_keyToRow.size() != keys.size()) changed = true;
            for (size_t i = 0; i < keys.size(); i++)
            {
                if (_keyToRow.find(keys[i]) == _keyToRow.end()) changed = true;
            }
            if (changed) return this->reloadTable();
        }

        std::vector<Pothos::RemoteNode> nodes;
        for (const auto &entry : _keyToNode)
        {
            nodes.push_back(entry.second);
        }
        this->reloadRows(nodes); //initial load, future will fill in the rest
        _watcher->setFuture(QtConcurrent::run(std::bind(&RemoteNodesQTableWidget::peformNodeComms, nodes)));
    }

private:

    static std::vector<Pothos::RemoteNode> peformNodeComms(std::vector<Pothos::RemoteNode> nodes)
    {
        for (size_t i = 0; i < nodes.size(); i++)
        {
            try{nodes[i].communicate();}catch(const Pothos::RemoteNodeError &){}
        }
        return nodes;
    }

    void reloadRows(const std::vector<Pothos::RemoteNode> &nodes);
    void reloadTable(void);
    QSignalMapper *_removeMapper;
    QTimer *_timer;
    QFutureWatcher<std::vector<Pothos::RemoteNode>> *_watcher;
    std::map<std::string, size_t> _keyToRow;
    std::map<std::string, Pothos::RemoteNode> _keyToNode;
    static const size_t nCols = 4;
};

void RemoteNodesQTableWidget::reloadRows(const std::vector<Pothos::RemoteNode> &nodes)
{
    for (size_t i = 0; i < nodes.size(); i++)
    {
        const auto &node = nodes[i];
        if (_keyToRow.find(node.getKey()) == _keyToRow.end()) continue;
        const size_t row = _keyToRow[node.getKey()];
        _keyToNode[node.getKey()] = node;

        //gather information
        auto t = Poco::Timestamp::fromEpochTime(node.getLastAccess());
        auto ldt = Poco::LocalDateTime(Poco::DateTime(t));
        auto timeStr = Poco::DateTimeFormatter::format(ldt, "%h:%M:%S %A - %b %e %Y");
        auto accessTimeStr = QString((t == 0)? tr("Never") : QString::fromStdString(timeStr));
        QIcon statusIcon = makeIconFromTheme(
            node.isOnline()?"network-transmit-receive":"network-offline");

        //load columns
        size_t col = 1;
        this->setItem(row, col++, new QTableWidgetItem(statusIcon, QString::fromStdString(node.getUri())));
        this->setItem(row, col++, new QTableWidgetItem(QString::fromStdString(node.getName())));
        this->setItem(row, col++, new QTableWidgetItem(accessTimeStr));
        for (size_t c = 0; c < nCols; c++) disableEdit(this->item(row, c));
    }
    this->resizeColumnsToContents();
}

void RemoteNodesQTableWidget::reloadTable(void)
{
    //clear stuff for table reload
    this->clearContents();
    this->clearSpans();
    _keyToRow.clear();

    //query keys and set dimensions
    auto keys = Pothos::RemoteNode::listRegistryKeys();
    this->setRowCount(keys.size()+1);
    size_t row = 0;

    //create new entry row
    auto addButton = makeToolButton(this, "list-add");
    this->setCellWidget(row, 0, addButton);
    auto lineEdit = new NodeUriQLineEdit(this);
    this->setCellWidget(row, 1, lineEdit);
    this->setSpan(row, 1, 1, nCols-1);
    row++;

    //enumerate the available nodes
    for (size_t i = 0; i < keys.size(); i++)
    {
        const auto &key = keys[i];
        _keyToNode[key] = Pothos::RemoteNode::fromKey(key);
        _keyToRow[key] = row;
        auto removeButton = makeToolButton(this, "list-remove");
        this->setCellWidget(row, 0, removeButton);
        connect(removeButton, SIGNAL(clicked(void)), _removeMapper, SLOT(map()));
        _removeMapper->setMapping(removeButton, QString::fromStdString(key));

        row++;
    }

    //connect for new entries
    connect(
        addButton, SIGNAL(clicked(void)),
        lineEdit, SLOT(handleReturnPressed(void)));
    connect(
        lineEdit, SIGNAL(handleUriEntered(const QString &)),
        this, SLOT(handleAdd(const QString &)));
    connect(
        this, SIGNAL(handleErrorMessage(const QString &)),
        lineEdit, SLOT(handleErrorMessage(const QString &)));

    this->handleUpdateStatus(); //fills in status
}

QWidget *makeRemoteNodesTable(QWidget *parent)
{
    return new RemoteNodesQTableWidget(parent);
}

#include "RemoteNodesTable.moc"
