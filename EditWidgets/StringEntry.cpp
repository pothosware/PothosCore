// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Poco/JSON/Object.h>
#include <QLineEdit>

/***********************************************************************
 * Custom widget for string entry -- no quotes
 **********************************************************************/
class StringEntry : public QLineEdit
{
    Q_OBJECT
public:
    StringEntry(QWidget *parent):
        QLineEdit(parent)
    {
        connect(this, SIGNAL(textEdited(const QString &)), this, SLOT(handleTextEdited(const QString &)));
        connect(this, SIGNAL(returnPressed(void)), this, SIGNAL(commitRequested(void)));
    }

public slots:
    QString value(void) const
    {
        auto s = QLineEdit::text();
        return QString("\"%1\"").arg(s.replace("\"", "\\\"")); //escape
    }

    void setValue(const QString &s)
    {
        if (s.startsWith("\"") and s.endsWith("\""))
        {
            auto s0 = s.midRef(1, s.size()-2).toString();
            QLineEdit::setText(s0.replace("\\\"", "\"")); //unescape
        }
        else QLineEdit::setText(s);
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
static QWidget *makeStringEntry(const Poco::JSON::Object::Ptr &, QWidget *parent)
{
    return new StringEntry(parent);
}

pothos_static_block(registerStringEntry)
{
    Pothos::PluginRegistry::add("/gui/EntryWidgets/StringEntry", Pothos::Callable(&makeStringEntry));
}

#include "StringEntry.moc"
