// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //get settings
#include "HostExplorer/HostSelectionTable.hpp"
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
#include <Poco/SingletonHolder.h>
#include <map>
#include <iostream>
#include <functional> //std::bind
#include <mutex>

//TODO do we need a mutex? isnt QSettings safe?
std::mutex &getMutex(void)
{
    static Poco::SingletonHolder<std::mutex> sh;
    return *sh.get();
}

/***********************************************************************
 * NodeInfo update implementation
 **********************************************************************/
void NodeInfo::update(void)
{
    //determine if the host is online and update access times
    try
    {
        Pothos::RemoteClient client(this->uri.toStdString());
        if (this->nodeName.isEmpty())
        {
            auto env = client.makeEnvironment("managed");
            auto nodeInfo = env->findProxy("Pothos/System/NodeInfo").call<Pothos::System::NodeInfo>("get");
            this->nodeName = QString::fromStdString(nodeInfo.nodeName);
            getSettings().setValue("HostExplorer/"+this->uri+"/nodeName", this->nodeName);
        }
        this->isOnline = true;
        this->lastAccess = Poco::Timestamp();
        getSettings().setValue("HostExplorer/"+this->uri+"/lastAccess", int(this->lastAccess.epochTime()));
    }
    //otherwise, fetch the information from the settings cache
    catch(const Pothos::RemoteClientError &)
    {
        this->isOnline = false;
        if (this->nodeName.isEmpty())
        {
            this->nodeName = getSettings().value("HostExplorer/"+this->uri+"/nodeName").toString();
        }
        this->lastAccess = Poco::Timestamp::fromEpochTime(std::time_t(getSettings().value("HostExplorer/"+this->uri+"/lastAccess").toInt()));
    }
}

/***********************************************************************
 * global query for hosts known to system
 **********************************************************************/
QStringList getHostUriList(void)
{
    std::lock_guard<std::mutex> lock(getMutex());

    auto uris = getSettings().value("HostExplorer/uris").toStringList();
    uris.push_back("tcp://localhost");

    //sanitize duplicates
    QStringList noDups;
    for (const auto &uri : uris)
    {
        if (std::find(noDups.begin(), noDups.end(), uri) == noDups.end()) noDups.push_back(uri);
    }
    return noDups;
}

static void setHostUriList(const QStringList &uris)
{
    std::lock_guard<std::mutex> lock(getMutex());

    getSettings().setValue("HostExplorer/uris", uris);
}

/***********************************************************************
 * Custom line editor for host URI entry
 **********************************************************************/
class HostUriQLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    HostUriQLineEdit(QWidget *parent):
        QLineEdit(parent)
    {
        this->setPlaceholderText(tr("Click to enter a new Host URI"));

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
HostSelectionTable::HostSelectionTable(QWidget *parent):
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

void HostSelectionTable::handleCellClicked(const int row, const int col)
{
    if (col < 1) return;
    for (const auto &entry : _uriToRow)
    {
        if (entry.second != size_t(row)) continue;
        auto &info = _uriToInfo.at(entry.first);
        info.update();
        if (info.isOnline) emit nodeInfoRequest(info.uri.toStdString());
        else emit handleErrorMessage(tr("Host %1 is offline").arg(info.uri));
    }
}

void HostSelectionTable::handleRemove(const QString &uri)
{
    try
    {
        auto uris = getHostUriList();
        uris.erase(std::find(uris.begin(), uris.end(), uri));
        setHostUriList(uris);
        this->reloadTable();
    }
    catch(const Pothos::Exception &ex)
    {
        emit handleErrorMessage(QString::fromStdString(ex.displayText()));
    }
}

void HostSelectionTable::handleAdd(const QString &uri)
{
    try
    {
        auto uris = getHostUriList();
        if (std::find(uris.begin(), uris.end(), uri) != uris.end())
        {
            emit handleErrorMessage(tr("%1 already exists").arg(uri));
            return;
        }
        uris.push_back(uri);
        setHostUriList(uris);
        this->reloadTable();
    }
    catch(const Pothos::Exception &ex)
    {
        emit handleErrorMessage(QString::fromStdString(ex.displayText()));
    }
}

void HostSelectionTable::handleNodeQueryComplete(void)
{
    this->reloadRows(_watcher->result());
}

void HostSelectionTable::handleUpdateStatus(void)
{
    //did the keys change?
    {
        auto uris = getHostUriList();
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
    _watcher->setFuture(QtConcurrent::run(std::bind(&HostSelectionTable::peformNodeComms, nodes)));
}

std::vector<NodeInfo> HostSelectionTable::peformNodeComms(std::vector<NodeInfo> nodes)
{
    for (size_t i = 0; i < nodes.size(); i++) nodes[i].update();
    return nodes;
}

void HostSelectionTable::reloadRows(const std::vector<NodeInfo> &nodeInfos)
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
        auto accessTimeStr = QString(info.neverAccessed()? tr("Never") : QString::fromStdString(timeStr));
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

void HostSelectionTable::reloadTable(void)
{
    //clear stuff for table reload
    this->clearContents();
    this->clearSpans();
    _uriToRow.clear();

    //query keys and set dimensions
    auto uris = getHostUriList();
    this->setRowCount(uris.size()+1);
    size_t row = 0;

    //create new entry row
    auto addButton = makeToolButton(this, "list-add");
    this->setCellWidget(row, 0, addButton);
    auto lineEdit = new HostUriQLineEdit(this);
    this->setCellWidget(row, 1, lineEdit);
    this->setSpan(row, 1, 1, nCols-1);
    row++;

    //enumerate the available hosts
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

#include "HostSelectionTable.moc"
