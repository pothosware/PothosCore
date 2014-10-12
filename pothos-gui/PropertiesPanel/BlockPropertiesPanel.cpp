// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //get object map
#include "AffinitySupport/AffinityZonesDock.hpp"
#include "BlockPropertiesPanel.hpp"
#include "BlockPropertyEditWidget.hpp"
#include "GraphObjects/GraphObject.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QTabWidget>
#include <QLabel>
#include <QPainter>
#include <QTimer>
#include <sstream>
#include <cassert>

static const long UPDATE_TIMER_MS = 500;

BlockPropertiesPanel::BlockPropertiesPanel(GraphBlock *block, QWidget *parent):
    QWidget(parent),
    _ignoreChanges(true),
    _idLabel(new QLabel(this)),
    _idLineEdit(new QLineEdit(this)),
    _affinityZoneLabel(new QLabel(this)),
    _affinityZoneBox(nullptr),
    _blockErrorLabel(new QLabel(this)),
    _updateTimer(new QTimer(this)),
    _jsonBlockDesc(nullptr),
    _formLayout(nullptr),
    _block(block)
{
    auto blockDesc = block->getBlockDesc();

    //master layout for this widget
    _formLayout = new QFormLayout(this);

    //title
    {
        auto label = new QLabel(QString("<h1>%1</h1>").arg(_block->getTitle().toHtmlEscaped()), this);
        label->setAlignment(Qt::AlignCenter);
        _formLayout->addRow(label);
    }

    //errors
    {
        _formLayout->addRow(_blockErrorLabel);
    }

    //id
    {
        _idOriginal = _block->getId();
        _formLayout->addRow(_idLabel, _idLineEdit);
        connect(_idLineEdit, SIGNAL(textEdited(const QString &)), this, SLOT(handleEditWidgetChanged(const QString &)));
        connect(_idLineEdit, SIGNAL(returnPressed(void)), this, SLOT(handleCommit(void)));
    }

    //create optional properties tabs
    auto propertiesTabs = new QTabWidget(this);
    _formLayout->addRow(propertiesTabs);
    for (const auto &propKey : _block->getProperties())
    {
        const auto tabName = _block->getParamDesc(propKey)->optValue<std::string>("tab", "");
        if (_paramLayouts.count(tabName) != 0) continue;
        auto tab = new QWidget(propertiesTabs);
        _paramLayouts[tabName] = new QFormLayout(tab);
        propertiesTabs->addTab(tab, tabName.empty()? tr("Default") : QString::fromStdString(tabName));
    }

    //only one default tab? fall back to no tab widget
    if (_paramLayouts.size() == 1 and _paramLayouts.count("") == 1)
    {
        _paramLayouts[""] = _formLayout;
        delete propertiesTabs;
    }

    //properties
    for (const auto &propKey : _block->getProperties())
    {
        _propIdToOriginal[propKey] = _block->getPropertyValue(propKey);
        auto paramDesc = _block->getParamDesc(propKey);

        //create editable widget
        auto editWidget = new BlockPropertyEditWidget(paramDesc, this);
        connect(editWidget, SIGNAL(valueChanged(void)), this, SLOT(handleEditWidgetChanged(void)));
        connect(editWidget, SIGNAL(commitRequested(void)), this, SLOT(handleCommit(void)));
        _propIdToEditWidget[propKey] = editWidget;

        //create labels
        _propIdToFormLabel[propKey] = new QLabel(this);
        _propIdToErrorLabel[propKey] = new QLabel(this);
        editWidget->setToolTip(this->getParamDocString(propKey));

        //layout stuff
        auto editLayout = new QVBoxLayout();
        editLayout->addWidget(editWidget);
        editLayout->addWidget(_propIdToErrorLabel[propKey]);
        auto layout = _paramLayouts.at(paramDesc->optValue<std::string>("tab", ""));
        layout->addRow(_propIdToFormLabel[propKey], editLayout);
    }

    //affinity zone
    {
        _affinityZoneOriginal = _block->getAffinityZone();
        auto dock = dynamic_cast<AffinityZonesDock *>(getObjectMap()["affinityZonesDock"]);
        assert(dock != nullptr);
        _affinityZoneBox = dock->makeComboBox(this);
        if (_block->isGraphWidget())
        {
            _affinityZoneLabel->hide();
            _affinityZoneBox->hide();
        }
        else _formLayout->addRow(_affinityZoneLabel, _affinityZoneBox);
        connect(_affinityZoneBox, SIGNAL(activated(const QString &)), this, SLOT(handleEditWidgetChanged(const QString &)));
    }

    //draw the block's preview onto a mini pixmap
    //this is cool, maybe useful, but its big, where can we put it?
    /*
    {
        const auto bounds = _block->getBoundingRect();
        QPixmap pixmap(bounds.size().toSize()+QSize(2,2));
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.translate(-bounds.topLeft()+QPoint(1,1));
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        _block->render(painter);
        painter.end();
        auto label = new QLabel(this);
        label->setPixmap(pixmap);
        _formLayout->addRow(label);
        _formLayout->setAlignment(label, Qt::AlignHCenter);
    }
    */

    //block level description
    auto docTabs = new QTabWidget(this);
    connect(docTabs, SIGNAL(currentChanged(int)), this, SLOT(handleDocTabChanged(int)));
    _formLayout->addRow(docTabs);
    {
        QString output;
        output += QString("<h1>%1</h1>").arg(QString::fromStdString(blockDesc->get("name").convert<std::string>()));
        output += QString("<p>%1</p>").arg(QString::fromStdString(block->getBlockDescPath()));
        output += "<p>";
        if (blockDesc->isArray("docs")) for (const auto &lineObj : *blockDesc->getArray("docs"))
        {
            const auto line = lineObj.extract<std::string>();
            if (line.empty()) output += "<p /><p>";
            else output += QString::fromStdString(line)+"\n";
        }
        output += "</p>";

        //enumerate properties
        if (not _block->getProperties().empty())
        {
            output += QString("<h2>%1</h2>").arg(tr("Properties"));
            for (const auto &propKey : _block->getProperties())
            {
                output += this->getParamDocString(propKey);
            }
        }

        //enumerate slots
        if (not block->getSlotPorts().empty())
        {
            output += QString("<h2>%1</h2>").arg(tr("Slots"));
            output += "<ul>";
            for (const auto &portKey : block->getSlotPorts())
            {
                output += QString("<li>%1(...)</li>").arg(portKey);
            }
            output += "</ul>";
        }

        //enumerate signals
        if (not block->getSignalPorts().empty())
        {
            output += QString("<h2>%1</h2>").arg(tr("Signals"));
            output += "<ul>";
            for (const auto &portKey : block->getSignalPorts())
            {
                output += QString("<li>%1(...)</li>").arg(portKey);
            }
            output += "</ul>";
        }

        auto text = new QLabel(output, docTabs);
        text->setStyleSheet("QLabel{background:white;margin:1px;}");
        text->setWordWrap(true);
        docTabs->addTab(text, tr("Documentation"));
    }

    //block desc JSON (filled in by event handler)
    {
        _jsonBlockDesc = new QLabel(docTabs);
        docTabs->addTab(_jsonBlockDesc, tr("JSON description"));
    }

    //update timer
    _updateTimer->setSingleShot(true);
    _updateTimer->setInterval(UPDATE_TIMER_MS);
    connect(_updateTimer, SIGNAL(timeout(void)), this, SLOT(handleUpdateTimerExpired(void)));

    connect(_block, SIGNAL(destroyed(QObject*)), this, SLOT(handleBlockDestroyed(QObject*)));
    this->updateAllForms();
    _ignoreChanges = false;
}

QString BlockPropertiesPanel::getParamDocString(const QString &propKey)
{
    const auto paramDesc = _block->getParamDesc(propKey);
    assert(paramDesc);
    QString unitsStr;
    if (paramDesc->has("units")) unitsStr = QString(" (%1)")
        .arg(QString::fromStdString(paramDesc->getValue<std::string>("units")).toHtmlEscaped());
    QString output;
    output += QString("<h3>%1%2</h3>").arg(_block->getPropertyName(propKey).toHtmlEscaped()).arg(unitsStr);
    if (paramDesc->isArray("desc")) for (const auto &lineObj : *paramDesc->getArray("desc"))
    {
        const auto line = lineObj.extract<std::string>();
        if (line.empty()) output += "<p /><p>";
        else output += QString::fromStdString(line)+"\n";
    }
    else output += QString("<p>%1</p>").arg(tr("Undocumented"));
    return output;
}

void BlockPropertiesPanel::handleBlockDestroyed(QObject *)
{
    this->deleteLater();
}

void BlockPropertiesPanel::handleEditWidgetChanged(void)
{
    if (_ignoreChanges) return;

    //dump editor id to block
    _block->setId(_idLineEdit->text());

    //dump the affinity zone to block
    _block->setAffinityZone(_affinityZoneBox->itemData(_affinityZoneBox->currentIndex()).toString());

    //dump all values from edit widgets into the block's property values
    for (const auto &propKey : _block->getProperties())
    {
        auto newValue = _propIdToEditWidget[propKey]->value();
        newValue.replace("\n", ""); //cannot handle multi-line values
        _block->setPropertyValue(propKey, newValue);
    }

    this->updateAllForms(); //quick update for labels
    _updateTimer->start(UPDATE_TIMER_MS); //schedule new eval
}

void BlockPropertiesPanel::handleUpdateTimerExpired(void)
{
    _block->update();
    this->updateAllForms();
}

void BlockPropertiesPanel::handleCancel(void)
{
    _updateTimer->stop();

    //reset values in block to original setting
    _block->setId(_idOriginal);
    _block->setAffinityZone(_affinityZoneOriginal);
    for (const auto &propKey : _block->getProperties())
    {
        _block->setPropertyValue(propKey, _propIdToOriginal[propKey]);
    }
    _block->update(); //update after change reversion

    //an edit widget return press signal may have us here,
    //and not the commit button, so make sure panel is deleted
    this->deleteLater();
}

void BlockPropertiesPanel::handleCommit(void)
{
    _updateTimer->stop();

    //were there changes?
    std::vector<QString> propertiesModified;
    for (const auto &propKey : _block->getProperties())
    {
        if (_block->getPropertyValue(propKey) != _propIdToOriginal[propKey])
        {
            propertiesModified.push_back(_block->getPropertyName(propKey));
        }
    }

    //was the ID changed?
    if (_idOriginal != _block->getId()) propertiesModified.push_back(tr("ID"));

    //was the affinity zone changed?
    if (_affinityZoneOriginal != _block->getAffinityZone()) propertiesModified.push_back(tr("Affinity Zone"));

    if (propertiesModified.empty()) return this->handleCancel();

    //emit a new graph state event
    auto desc = (propertiesModified.size() == 1)? propertiesModified.front() : tr("properties");
    emit this->stateChanged(GraphState("document-properties", tr("Edit %1 %2").arg(_block->getId()).arg(desc)));

    //an edit widget return press signal may have us here,
    //and not the commit button, so make sure panel is deleted
    this->deleteLater();
}

void BlockPropertiesPanel::handleDocTabChanged(int index)
{
    if (_jsonBlockDesc == nullptr) return;
    _jsonBlockDesc->setText("");
    if (index != 1) return;
    std::stringstream ss; _block->getBlockDesc()->stringify(ss, 4);
    _jsonBlockDesc->setStyleSheet("QLabel{background:white;margin:1px;}");
    _jsonBlockDesc->setWordWrap(true);
    _jsonBlockDesc->setText(QString::fromStdString(ss.str()));
}

void BlockPropertiesPanel::updateAllForms(void)
{
    //block id
    {
        _idLineEdit->setText(_block->getId());
        _idLabel->setText(QString("<b>%1%2</b>")
            .arg(tr("ID"))
            .arg((_idOriginal != _block->getId())?"*":""));
    }

    //affinity zone
    {
        for (int i = 0; i < _affinityZoneBox->count(); i++)
        {
            if (_affinityZoneBox->itemData(i).toString() == _block->getAffinityZone())
            {
                _affinityZoneBox->setCurrentIndex(i);
            }
        }
        _affinityZoneLabel->setText(QString("<b>%1%2</b>")
            .arg(tr("Affinity Zone"))
            .arg((_affinityZoneOriginal != _block->getAffinityZone())?"*":""));
    }

    //update block errors
    {
        const auto &errors = _block->getBlockErrorMsgs();
        _blockErrorLabel->setVisible(not errors.isEmpty());
        _blockErrorLabel->setWordWrap(true);
        QString errorList;
        for (const auto &errMsg : errors)
        {
            errorList += QString("<li><i>%1</i></li>").arg(errMsg.toHtmlEscaped());
        }
        if (errors.size() > 1) errorList = QString("<ul>%1</ul>").arg(errorList);
        _blockErrorLabel->setText(QString(
            "<p><span style='color:red;'>%1</span></p>")
            .arg(errorList));
    }

    for (const auto &key : _block->getProperties()) this->updatePropForms(key);
}

void BlockPropertiesPanel::updatePropForms(const QString &propKey)
{
    auto paramDesc = _block->getParamDesc(propKey);
    auto editWidget = _propIdToEditWidget[propKey];
    auto errorLabel = _propIdToErrorLabel[propKey];
    auto formLabel = _propIdToFormLabel[propKey];

    //create label string
    bool propChanged = (_propIdToOriginal[propKey] != _block->getPropertyValue(propKey));
    auto label = QString("<span style='color:%1;'><b>%2%3</b></span>")
        .arg(_block->getPropertyErrorMsg(propKey).isEmpty()?"black":"red")
        .arg(_block->getPropertyName(propKey))
        .arg(propChanged?"*":"");
    if (paramDesc->has("units")) label += QString("<br /><i>%1</i>")
        .arg(QString::fromStdString(paramDesc->getValue<std::string>("units")));
    formLabel->setText(label);

    //error label
    QString errorMsg = _block->getPropertyErrorMsg(propKey);
    errorLabel->setVisible(not errorMsg.isEmpty());
    errorLabel->setText(QString("<span style='color:red;'><p><i>%1</i></p></span>").arg(errorMsg.toHtmlEscaped()));
    errorLabel->setWordWrap(true);

    //set the editor's value and type string colors
    editWidget->setValue(_block->getPropertyValue(propKey));
    editWidget->setColors(_block->getPropertyTypeStr(propKey));
}
