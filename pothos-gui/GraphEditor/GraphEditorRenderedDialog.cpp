// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphEditor/GraphEditor.hpp"
#include "EvalEngine/EvalEngine.hpp"
#include <Pothos/Framework/Topology.hpp>
#include <Pothos/Exception.hpp>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>
#include <Poco/Process.h>
#include <Poco/Environment.h>
#include <Poco/TemporaryFile.h>
#include <Poco/JSON/Object.h>
#include <QComboBox>
#include <QDialog>
#include <QFile>
#include <QPixmap>
#include <QLabel>
#include <QScrollArea>
#include <QTabWidget>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <sstream>

struct ImageResult
{
    QPixmap pixmap;
    std::string errorMsg;
};

static ImageResult dotMarkupToImage(const std::string &markup)
{
    ImageResult result;
    if (markup.empty())
    {
        result.errorMsg = "empty markup - is the topology active?";
        return result;
    }

    //temp file
    auto tempFile = Poco::TemporaryFile::tempName();
    Poco::TemporaryFile::registerForDeletion(tempFile);

    //create args
    Poco::Process::Args args;
    args.push_back("-Tpng"); //yes png
    args.push_back("-o"); //output to file
    args.push_back(tempFile);

    //launch
    Poco::Pipe inPipe, outPipe, errPipe;
    Poco::Process::Env env;
    Poco::ProcessHandle ph(Poco::Process::launch(
        Poco::Environment::get("DOT_EXECUTABLE", "dot"),
        args, &inPipe, &outPipe, &errPipe, env));

    //write the markup into dot
    Poco::PipeOutputStream os(inPipe);
    os << markup;
    os.close();
    outPipe.close();

    //check for errors
    if (ph.wait() != 0 or not QFile(QString::fromStdString(tempFile)).exists())
    {
        Poco::PipeInputStream es(errPipe);
        std::string errMsg;
        es >> errMsg;
        result.errorMsg = "png failed: " + errMsg;
        return result;
    }

    //create the image from file
    result.pixmap = QPixmap(QString::fromStdString(tempFile), "png");
    return result;
}

static ImageResult dotMarkupToImageSafe(const std::string &markup)
{
    ImageResult result;
    try
    {
        result = dotMarkupToImage(markup);
    }
    catch (const Poco::Exception &ex)
    {
        result.errorMsg = "png failed: " + ex.displayText();
    }
    return result;
}

class RenderedGraphDialog : public QDialog
{
    Q_OBJECT
public:
    RenderedGraphDialog(EvalEngine *evalEngine, QWidget *parent):
        QDialog(parent),
        _evalEngine(evalEngine),
        _topLayout(new QVBoxLayout(this)),
        _modeOptions(new QComboBox(this)),
        _portOptions(new QComboBox(this)),
        _watcher(new QFutureWatcher<ImageResult>(this)),
        _currentView(nullptr)
    {
        //create layouts
        this->setWindowTitle(tr("Rendered topology viewer"));
        auto formsLayout = new QHBoxLayout();
        _topLayout->addLayout(formsLayout);

        //create entry forms
        auto modeOptionsLayout = new QFormLayout();
        formsLayout->addLayout(modeOptionsLayout);
        _modeOptions->addItem(tr("Top level"), QString("top"));
        _modeOptions->addItem(tr("Flattened"), QString("flat"));
        _modeOptions->addItem(tr("Rendered"), QString("rendered"));
        modeOptionsLayout->addRow(tr("Display mode"), _modeOptions);

        auto portOptionsLayout = new QFormLayout();
        formsLayout->addLayout(portOptionsLayout);
        _portOptions->addItem(tr("Connected"), QString("connected"));
        _portOptions->addItem(tr("All ports"), QString("all"));
        portOptionsLayout->addRow(tr("Show ports"), _portOptions);

        //connect widget changed events
        connect(_modeOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(handleChange(int)));
        connect(_portOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(handleChange(int)));
        connect(_watcher, SIGNAL(finished(void)), this, SLOT(handleWatcherDone(void)));

        //initialize
        QPixmap pixmap(parent->size());
        pixmap.fill(Qt::white);
        installNewView(pixmap);
        this->handleChange(0);
    }

private slots:
    void handleChange(int)
    {
        Poco::JSON::Object::Ptr configObj(new Poco::JSON::Object());
        configObj->set("mode", _modeOptions->itemData(_modeOptions->currentIndex()).toString().toStdString());
        configObj->set("port", _portOptions->itemData(_portOptions->currentIndex()).toString().toStdString());
        std::stringstream ss;
        configObj->stringify(ss);

        const auto markup = _evalEngine->getTopologyDotMarkup(ss.str());
        _watcher->setFuture(QtConcurrent::run(std::bind(&dotMarkupToImageSafe, markup)));
    }

    void handleWatcherDone(void)
    {
        auto imageResult = _watcher->result();
        if (imageResult.errorMsg.empty())
        {
            installNewView(imageResult.pixmap);
        }
        else
        {
            QMessageBox msgBox(QMessageBox::Critical, tr("Topology render error"), QString::fromStdString(imageResult.errorMsg));
            msgBox.exec();
        }
    }

    void installNewView(const QPixmap &pixmap)
    {
        delete _currentView; //delete the previous view
        _currentView = new QWidget(this);
        auto layout = new QVBoxLayout(_currentView);
        auto scroll = new QScrollArea(_currentView);
        layout->addWidget(scroll);
        auto label = new QLabel(scroll);
        scroll->setWidget(label);
        scroll->setWidgetResizable(true);
        label->setPixmap(pixmap);
        _topLayout->addWidget(_currentView, 1);
    }

private:
    EvalEngine *_evalEngine;
    QVBoxLayout *_topLayout;
    QComboBox *_modeOptions;
    QComboBox *_portOptions;
    QFutureWatcher<ImageResult> *_watcher;
    QWidget *_currentView;
};

void GraphEditor::handleShowRenderedGraphDialog(void)
{
    if (not this->isVisible()) return;

    //create the dialog
    auto dialog = new RenderedGraphDialog(_evalEngine, this);
    dialog->show();
    dialog->adjustSize();
    dialog->setWindowState(Qt::WindowMaximized);
    dialog->exec();
    delete dialog;
}

#include "GraphEditorRenderedDialog.moc"
