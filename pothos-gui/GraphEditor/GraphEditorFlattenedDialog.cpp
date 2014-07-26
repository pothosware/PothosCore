// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphEditor/GraphEditor.hpp"
#include "TopologyEngine/TopologyEngine.hpp"
#include <Pothos/Exception.hpp>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>
#include <Poco/Process.h>
#include <Poco/Environment.h>
#include <Poco/TemporaryFile.h>
#include <QDialog>
#include <QFile>
#include <QImage>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QMessageBox>

void GraphEditor::handleShowFlattenedDialog(void)
{
    if (not this->isVisible()) return;

    std::string errorMsg;
    try
    {
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
        os << _topologyEngine->getTopology().toDotMarkup();
        os.close();
        outPipe.close();

        //check for errors
        if (ph.wait() != 0 or not QFile(QString::fromStdString(tempFile)).exists())
        {
            Poco::PipeInputStream es(errPipe);
            std::string errMsg;
            es >> errMsg;
            throw Pothos::Exception("PothosGui.GraphEditor.showFlattenedGraphDialog()", "png failed: " + errMsg);
        }

        //create the image from file
        QImage image(QString::fromStdString(tempFile), "png");

        //create the dialog
        auto dialog = new QDialog(this);
        dialog->setWindowTitle(tr("Flattened graph"));
        dialog->setMinimumSize(800, 600);
        auto layout = new QVBoxLayout(dialog);
        dialog->setLayout(layout);
        auto scroll = new QScrollArea(dialog);
        layout->addWidget(scroll);
        auto label = new QLabel(scroll);
        scroll->setWidget(label);
        scroll->setWidgetResizable(true);
        label->setPixmap(QPixmap::fromImage(image));
        dialog->exec();
        delete dialog;
    }
    catch (const Pothos::Exception &ex)
    {
        errorMsg = ex.displayText();
    }
    catch (const Poco::Exception &ex)
    {
        errorMsg = ex.displayText();
    }

    if (not errorMsg.empty())
    {
        QMessageBox msgBox(QMessageBox::Critical, "Error rendering flattened graph", QString::fromStdString(errorMsg));
        msgBox.exec();
    }
}
