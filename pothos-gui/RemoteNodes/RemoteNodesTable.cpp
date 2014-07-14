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
#include <Pothos/Proxy.hpp>
#include <Pothos/System.hpp>
#include <Poco/DateTimeFormatter.h>
#include <map>
#include <iostream>
#include <functional> //std::bind

/***********************************************************************
 * global query for remote nodes known to system
 **********************************************************************/
QStringList getRemoteNodeUris(void)
{
    auto uris = getSettings().value("RemoteNodesTable/uris").toStringList();
    uris.push_back("tcp://localhost");

    //sanitize duplicates
    QStringList noDups;
    for (const auto &uri : uris)
    {
        if (std::find(noDups.begin(), noDups.end(), uri) == noDups.end()) noDups.push_back(uri);
    }
    return noDups;
}

static void setRemoteNodeUris(const QStringList &uris)
{
    getSettings().setValue("RemoteNodesTable/uris", uris);
}

/***********************************************************************
 * represent information about a remote node
 **********************************************************************/
struct NodeInfo
{
    NodeInfo(void):
        isOnline(false),
        accessCount(0)
    {}
    QString uri;
    Poco::Timestamp lastAccess;
    bool isOnline;
    size_t accessCount;
    QString nodeName;
    void update(void)
    {
        try
        {
            Pothos::RemoteClient client(this->uri.toStdString());
            if (nodeName.isEmpty())
            {
                auto env = client.makeEnvironment("managed");
                auto nodeInfo = env->findProxy("Pothos/System/NodeInfo").call<Pothos::System::NodeInfo>("get");
                this->nodeName = QString::fromStdString(nodeInfo.nodeName);
            }
            this->lastAccess = Poco::Timestamp();
            this->accessCount++;
            this->isOnline = true;
        }
        catch(const Pothos::RemoteClientError &)
        {
            this->isOnline = false;
        }
    }
};

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
        _watcher(new QFutureWatcher<std::vector<NodeInfo>>(this))
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
    void nodeInfoRequest(const std::string &);

private slots:

    void handleCellClicked(const int row, const int col)
    {
        if (col < 1) return;
        for (const auto &entry : _uriToRow)
        {
            if (entry.second != size_t(row)) continue;
            auto &info = _uriToInfo.at(entry.first);
            info.update();
            if (info.isOnline) emit nodeInfoRequest(info.uri.toStdString());
            else emit handleErrorMessage(tr("node %1 is offline").arg(info.uri));
        }
    }

    void handleRemove(const QString &uri)
    {
        try
        {
            auto uris = getRemoteNodeUris();
            uris.erase(std::find(uris.begin(), uris.end(), uri));
            setRemoteNodeUris(uris);
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
            auto uris = getRemoteNodeUris();
            if (std::find(uris.begin(), uris.end(), uri) != uris.end())
            {
                emit handleErrorMessage(tr("%1 already exists").arg(uri));
                return;
            }
            uris.push_back(uri);
            setRemoteNodeUris(uris);
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
            auto uris = getRemoteNodeUris();
            bool changed = false;
            if (_uriToRow.size() != size_t(uris.size())) changed = true;
            for (int i = 0; i < uris.size(); i++)
            {
                if (_uriToRow.find(uris[i]) == _uriToRow.end()) changed = true;
            }
            if (changed) return this->reloadTable();
        }

        std::vector<NodeInfo> nodes;
        for (const auto &entry : _uriToInfo)
        {
            nodes.push_back(entry.second);
        }
        this->reloadRows(nodes); //initial load, future will fill in the rest
        _watcher->setFuture(QtConcurrent::run(std::bind(&RemoteNodesQTableWidget::peformNodeComms, nodes)));
    }

private:

    static std::vector<NodeInfo> peformNodeComms(std::vector<NodeInfo> nodes)
    {
        for (size_t i = 0; i < nodes.size(); i++) nodes[i].update();
        return nodes;
    }

    void reloadRows(const std::vector<NodeInfo> &nodes);
    void reloadTable(void);
    QSignalMapper *_removeMapper;
    QTimer *_timer;
    QFutureWatcher<std::vector<NodeInfo>> *_watcher;
    std::map<QString, size_t> _uriToRow;
    std::map<QString, NodeInfo> _uriToInfo;
    static const size_t nCols = 4;
};

void RemoteNodesQTableWidget::reloadRows(const std::vector<NodeInfo> &nodeInfos)
{
    for (size_t i = 0; i < nodeInfos.size(); i++)
    {
        const auto &info = nodeInfos[i];
        if (_uriToRow.find(info.uri) == _uriToRow.end()) continue;
        const size_t row = _uriToRow[info.uri];
        _uriToInfo[info.uri] = info;

        //gather information
        auto ldt = Poco::LocalDateTime(Poco::DateTime(info.lastAccess));
        auto timeStr = Poco::DateTimeFormatter::format(ldt, "%h:%M:%S %A - %b %e %Y");
        auto accessTimeStr = QString((info.accessCount == 0)? tr("Never") : QString::fromStdString(timeStr));
        QIcon statusIcon = makeIconFromTheme(
            info.isOnline?"network-transmit-receive":"network-offline");

        //load columns
        size_t col = 1;
        this->setItem(row, col++, new QTableWidgetItem(statusIcon, info.uri));
        this->setItem(row, col++, new QTableWidgetItem(info.nodeName));
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
    _uriToRow.clear();

    //query keys and set dimensions
    auto uris = getRemoteNodeUris();
    this->setRowCount(uris.size()+1);
    size_t row = 0;

    //create new entry row
    auto addButton = makeToolButton(this, "list-add");
    this->setCellWidget(row, 0, addButton);
    auto lineEdit = new NodeUriQLineEdit(this);
    this->setCellWidget(row, 1, lineEdit);
    this->setSpan(row, 1, 1, nCols-1);
    row++;

    //enumerate the available nodes
    for (int i = 0; i < uris.size(); i++)
    {
        const auto &uri = uris[i];
        _uriToInfo[uri].uri = uri;
        _uriToRow[uri] = row;
        auto removeButton = makeToolButton(this, "list-remove");
        this->setCellWidget(row, 0, removeButton);
        connect(removeButton, SIGNAL(clicked(void)), _removeMapper, SLOT(map()));
        _removeMapper->setMapping(removeButton, uri);

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
