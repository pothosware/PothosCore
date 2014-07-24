// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //make icon theme
#include "AffinitySupport/AffinityZonesDock.hpp"
#include "PropertiesPanel/BlockPropertiesPanel.hpp"
#include "PropertiesPanel/BlockPropertyEditWidget.hpp"
#include "GraphObjects/GraphObject.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "GraphEditor/GraphDraw.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QTimer>
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
    _formLayout(nullptr),
    _block(block)
{
    auto blockDesc = block->getBlockDesc();

    //master layout for this widget
    auto layout = new QVBoxLayout(this);

    //create a scroller and a form layout
    auto scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setWidget(new QWidget(scroll));
    _formLayout = new QFormLayout(scroll);
    scroll->widget()->setLayout(_formLayout);
    layout->addWidget(scroll);

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
        connect(_idLineEdit, SIGNAL(returnPressed(void)), this, SLOT(handleCommitButton(void)));
    }

    //properties
    for (const auto &prop : _block->getProperties())
    {
        _propIdToOriginal[prop.getKey()] = _block->getPropertyValue(prop.getKey());
        auto paramDesc = _block->getParamDesc(prop.getKey());
        assert(paramDesc);

        //create editable widget
        auto editWidget = new BlockPropertyEditWidget(paramDesc, this);
        connect(editWidget, SIGNAL(valueChanged(void)), this, SLOT(handleEditWidgetChanged(void)));
        connect(editWidget, SIGNAL(commitRequested(void)), this, SLOT(handleCommitButton(void)));
        _propIdToEditWidget[prop.getKey()] = editWidget;

        //create labels
        _propIdToFormLabel[prop.getKey()] = new QLabel(this);
        _propIdToErrorLabel[prop.getKey()] = new QLabel(this);
        editWidget->setToolTip(this->getParamDocString(_block->getParamDesc(prop.getKey())));

        //layout stuff
        auto editLayout = new QVBoxLayout();
        editLayout->addWidget(editWidget);
        editLayout->addWidget(_propIdToErrorLabel[prop.getKey()]);
        _formLayout->addRow(_propIdToFormLabel[prop.getKey()], editLayout);
    }

    //affinity zone
    {
        _affinityZoneOriginal = _block->getAffinityZone();
        auto dock = dynamic_cast<AffinityZonesDock *>(getObjectMap()["affinityZonesDock"]);
        assert(dock != nullptr);
        _affinityZoneBox = dock->makeComboBox(this);
        _formLayout->addRow(_affinityZoneLabel, _affinityZoneBox);
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
    if (blockDesc->isArray("docs"))
    {
        QString output;
        output += QString("<h1>%1</h1>").arg(QString::fromStdString(blockDesc->get("name").convert<std::string>()));
        output += QString("<p>%1</p>").arg(QString::fromStdString(block->getBlockDescPath()));
        output += "<p>";
        for (const auto &lineObj : *blockDesc->getArray("docs"))
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
            for (const auto &prop : _block->getProperties())
            {
                output += this->getParamDocString(_block->getParamDesc(prop.getKey()));
            }
        }

        //enumerate slots
        if (not block->getSlotPorts().empty())
        {
            output += QString("<h2>%1</h2>").arg(tr("Slots"));
            output += "<ul>";
            for (const auto &port : block->getSlotPorts())
            {
                output += QString("<li>%1(...)</li>").arg(port.getName());
            }
            output += "</ul>";
        }

        //enumerate signals
        if (not block->getSignalPorts().empty())
        {
            output += QString("<h2>%1</h2>").arg(tr("Signals"));
            output += "<ul>";
            for (const auto &port : block->getSignalPorts())
            {
                output += QString("<li>%1(...)</li>").arg(port.getName());
            }
            output += "</ul>";
        }

        auto text = new QLabel(output, this);
        text->setStyleSheet("QLabel{background:white;margin:1px;}");
        text->setWordWrap(true);
        _formLayout->addRow(text);
    }

    //buttons
    {
        auto buttonLayout = new QHBoxLayout();
        layout->addLayout(buttonLayout);
        auto commitButton = new QPushButton(makeIconFromTheme("dialog-ok-apply"), tr("Commit"), this);
        connect(commitButton, SIGNAL(pressed(void)), this, SLOT(handleCommitButton(void)));
        buttonLayout->addWidget(commitButton);
        auto cancelButton = new QPushButton(makeIconFromTheme("dialog-cancel"), tr("Cancel"), this);
        connect(cancelButton, SIGNAL(pressed(void)), this, SLOT(handleCancelButton(void)));
        buttonLayout->addWidget(cancelButton);
    }

    //update timer
    _updateTimer->setSingleShot(true);
    _updateTimer->setInterval(UPDATE_TIMER_MS);
    connect(_updateTimer, SIGNAL(timeout(void)), this, SLOT(handleUpdateTimerExpired(void)));

    //connect state change to the graph editor
    auto draw = dynamic_cast<GraphDraw *>(_block->parent());
    auto editor = draw->getGraphEditor();
    connect(this, SIGNAL(stateChanged(const GraphState &)), editor, SLOT(handleStateChange(const GraphState &)));
    connect(_block, SIGNAL(destroyed(QObject*)), this, SLOT(handleBlockDestroyed(QObject*)));

    this->updateAllForms();
    _ignoreChanges = false;
}

QString BlockPropertiesPanel::getParamDocString(const Poco::JSON::Object::Ptr &paramDesc)
{
    assert(paramDesc);
    QString output;
    output += QString("<h3>%1</h3>").arg(QString::fromStdString(paramDesc->getValue<std::string>("name")));
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
    for (const auto &prop : _block->getProperties())
    {
        QString newValue = _propIdToEditWidget[prop.getKey()]->value();
        _block->setPropertyValue(prop.getKey(), newValue);
    }

    this->updateAllForms(); //quick update for labels
    _updateTimer->start(UPDATE_TIMER_MS); //schedule new eval
}

void BlockPropertiesPanel::handleUpdateTimerExpired(void)
{
    _block->update();
    this->updateAllForms();
}

void BlockPropertiesPanel::handleCancelButton(void)
{
    _updateTimer->stop();

    //empty state causes reset to the last known point
    emit this->stateChanged(GraphState());

    this->deleteLater();
}

void BlockPropertiesPanel::handleCommitButton(void)
{
    _updateTimer->stop();

    //were there changes?
    std::vector<QString> propertiesModified;
    for (const auto &prop : _block->getProperties())
    {
        if (_block->getPropertyValue(prop.getKey()) != _propIdToOriginal[prop.getKey()]) propertiesModified.push_back(prop.getName());
    }

    //was the ID changed?
    if (_idOriginal != _block->getId()) propertiesModified.push_back(tr("ID"));

    //was the affinity zone changed?
    if (_affinityZoneOriginal != _block->getAffinityZone()) propertiesModified.push_back(tr("Affinity Zone"));

    if (propertiesModified.empty()) return this->handleCancelButton();

    //emit a new graph state event
    auto desc = (propertiesModified.size() == 1)? propertiesModified.front() : tr("properties");
    emit this->stateChanged(GraphState("document-properties", tr("Edit %1 %2").arg(_block->getId()).arg(desc)));

    //done with this panel
    this->deleteLater();
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
        _blockErrorLabel->setVisible(not _block->getBlockErrorMsg().isEmpty());
        _blockErrorLabel->setWordWrap(true);
        _blockErrorLabel->setText(QString(
            "<p><span style='color:red;'><i>%1</i></span></p>")
            .arg(_block->getBlockErrorMsg().toHtmlEscaped()));
    }

    for (const auto &prop : _block->getProperties())
    {
        this->updatePropForms(prop);
    }
}

void BlockPropertiesPanel::updatePropForms(const GraphBlockProp &prop)
{
    auto paramDesc = _block->getParamDesc(prop.getKey());
    auto editWidget = _propIdToEditWidget[prop.getKey()];
    auto errorLabel = _propIdToErrorLabel[prop.getKey()];
    auto formLabel = _propIdToFormLabel[prop.getKey()];

    //create label string
    bool propChanged = (_propIdToOriginal[prop.getKey()] == _block->getPropertyValue(prop.getKey()));
    auto label = QString("<span style='color:%1;'><b>%2%3</b></span>")
        .arg(_block->getPropertyErrorMsg(prop.getKey()).isEmpty()?"black":"red")
        .arg(prop.getName())
        .arg(propChanged?"":"*");
    if (paramDesc->has("units")) label += QString("<br /><i>%1</i>")
        .arg(QString::fromStdString(paramDesc->getValue<std::string>("units")));
    formLabel->setText(label);

    //error label
    QString errorMsg = _block->getPropertyErrorMsg(prop.getKey());
    errorLabel->setVisible(not errorMsg.isEmpty());
    errorLabel->setText(QString("<span style='color:red;'><p><i>%1</i></p></span>").arg(errorMsg.toHtmlEscaped()));
    errorLabel->setWordWrap(true);

    //set the editor's value and type string colors
    editWidget->setValue(_block->getPropertyValue(prop.getKey()));
    editWidget->setColors(_block->getPropertyTypeStr(prop.getKey()));
}
