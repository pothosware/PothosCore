// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Poco/JSON/Object.h>
#include <QLineEdit>

/***********************************************************************
 * General purpose single line entry widget
 **********************************************************************/
class LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    LineEdit(QWidget *parent):
        QLineEdit(parent)
    {
        connect(this, SIGNAL(textEdited(const QString &)), this, SLOT(handleTextEdited(const QString &)));
        connect(this, SIGNAL(returnPressed(void)), this, SIGNAL(commitRequested(void)));
    }

public slots:
    QString value(void) const
    {
        return QLineEdit::text();
    }

    void setValue(const QString &s)
    {
        QLineEdit::setText(s);
    }

signals:
    void commitRequested(void);
    void widgetChanged(void);
    void entryChanged(void);

private slots:
    void handleTextEdited(const QString &)
    {
        emit this->entryChanged();
    }
};

/***********************************************************************
 * Factory function and registration
 **********************************************************************/
static QWidget *makeLineEdit(const Poco::JSON::Object::Ptr &, QWidget *parent)
{
    return new LineEdit(parent);
}

pothos_static_block(registerLineEdit)
{
    Pothos::PluginRegistry::add("/gui/EntryWidgets/LineEdit", Pothos::Callable(&makeLineEdit));
}

#include "LineEdit.moc"
