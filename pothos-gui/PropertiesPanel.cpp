// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include "GraphObjects/GraphObject.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <cassert>

class PropertiesPanelBlock : public QScrollArea
{
    Q_OBJECT
public:
    PropertiesPanelBlock(GraphBlock *block, QWidget *parent):
        QScrollArea(parent),
        _block(block)
    {
        this->setWidgetResizable(true);
        this->setWidget(new QWidget(this));
        auto blockDesc = block->getBlockDesc();
        auto layout = new QVBoxLayout();
        this->widget()->setLayout(layout);

        //title
        {
            auto label = new QLabel(QString("<h1>%1</h1>")
                .arg(_block->getTitle().toHtmlEscaped()), this);
            label->setAlignment(Qt::AlignCenter);
            layout->addWidget(label);
        }

        //id
        {
            auto propLayout = new QHBoxLayout();
            layout->addLayout(propLayout);

            auto label = new QLabel(QString("<b>%1</b>")
                .arg(tr("ID")), this);
            propLayout->addWidget(label);

            auto edit = new QLineEdit(this);
            edit->setText(block->getId());
            propLayout->addWidget(edit);
        }

        //properties
        for (const auto &prop : _block->getProperties())
        {
            auto paramDesc = _block->getParamDesc(prop.getKey());
            assert(paramDesc);

            auto propLayout = new QHBoxLayout();
            layout->addLayout(propLayout);

            auto label = new QLabel(QString("<b>%1</b>")
                .arg(prop.getName()), this);
            propLayout->addWidget(label);

            const auto value = _block->getPropertyValue(prop.getKey());
            if (paramDesc->isArray("options"))
            {
                auto combo = new QComboBox(this);
                //combo->setEditable(true);
                propLayout->addWidget(combo);
                for (const auto &optionObj : *paramDesc->getArray("options"))
                {
                    const auto option = optionObj.extract<Poco::JSON::Object::Ptr>();
                    combo->addItem(
                        QString::fromStdString(option->getValue<std::string>("name")),
                        QString::fromStdString(option->getValue<std::string>("value")));
                }
            }
            else
            {
                auto edit = new QLineEdit(this);
                propLayout->addWidget(edit);
                edit->setText(value);
            }
        }

        //block level description
        if (blockDesc->isArray("docs"))
        {
            auto text = new QTextEdit(this);
            layout->addWidget(text);

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
            output += QString("<h2>%1</h2>").arg(tr("Properties"));
            for (const auto &prop : _block->getProperties())
            {
                auto paramDesc = _block->getParamDesc(prop.getKey());
                assert(paramDesc);
                output += QString("<h3>%1</h3>").arg(prop.getName());
                if (paramDesc->isArray("desc")) for (const auto &lineObj : *paramDesc->getArray("desc"))
                {
                    const auto line = lineObj.extract<std::string>();
                    if (line.empty()) output += "<p /><p>";
                    else output += QString::fromStdString(line);
                }
                else output += QString("<p>%1</p>").arg(tr("Undocumented"));
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

            text->insertHtml(output);
        }

        //buttons
        {
            auto buttonLayout = new QHBoxLayout();
            layout->addLayout(buttonLayout);
            auto commitButton = new QPushButton(makeIconFromTheme("dialog-ok-apply"), "Commit", this);
            buttonLayout->addWidget(commitButton);
            auto cancelButton = new QPushButton(makeIconFromTheme("dialog-cancel"), "Cancel", this);
            buttonLayout->addWidget(cancelButton);
        }
    }

private:
    QPointer<GraphBlock> _block;
};

class PropertiesPanelTopWidget : public QStackedWidget
{
    Q_OBJECT
public:
    PropertiesPanelTopWidget(QWidget *parent):
        QStackedWidget(parent),
        _propertiesPanel(nullptr),
        _blockTree(makeBlockTree(this))
    {
        getObjectMap()["blockTree"] = _blockTree;
        this->addWidget(_blockTree);
    }

private slots:

    void handleGraphModifyProperties(GraphObject *obj)
    {
        auto block = dynamic_cast<GraphBlock *>(obj);
        if (block == nullptr) this->setCurrentWidget(_blockTree);
        else
        {
            //TODO connect panel delete to block delete
            delete _propertiesPanel;
            _propertiesPanel = new PropertiesPanelBlock(block, this);
            this->addWidget(_propertiesPanel);
            this->setCurrentWidget(_propertiesPanel);
        }
    }

private:
    QWidget *_propertiesPanel;
    QWidget *_blockTree;
};

QWidget *makePropertiesPanel(QWidget *parent)
{
    return new PropertiesPanelTopWidget(parent);
}

#include "PropertiesPanel.moc"
