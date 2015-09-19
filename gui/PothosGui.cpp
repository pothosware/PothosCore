// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MainWindow.hpp"
#include "PothosGuiUtils.hpp"
#include <Pothos/Init.hpp>
#include <Pothos/Remote.hpp>
#include <Pothos/System.hpp>
#include <Poco/Logger.h>
#include <QMessageBox>
#include <QApplication>
#include <QSplashScreen>
#include <stdexcept>
#include <cstdlib> //EXIT_FAILURE
#include <memory>

struct MyScopedSyslogListener
{
    MyScopedSyslogListener(void)
    {
        Pothos::System::Logger::startSyslogListener();
    }
    ~MyScopedSyslogListener(void)
    {
        Pothos::System::Logger::stopSyslogListener();
    }
};

int main(int argc, char **argv)
{
    MyScopedSyslogListener syslogListener;

    //did the user specified files on the command line?
    //stash the files so they are loaded into the editor
    //this replaces the currently stored file list
    QStringList files;
    for (int i = 1; i < argc; i++)
    {
        QString file(argv[i]);
        if (file.isEmpty()) continue;
        files.push_back(file);
    }
    if (not files.isEmpty()) getSettings().setValue("GraphEditorTabs/files", files);

    //create the entry point to the GUI
    QApplication app(argc, argv);
    app.setOrganizationName("PothosWare");
    app.setApplicationName("Pothos");

    //create splash screen
    getSplashScreen()->show();

    //setup the application icon
    app.setWindowIcon(QIcon(makeIconPath("PothosGui.png")));

    //perform library initialization with graphical error message on failure
    Pothos::RemoteServer server;
    try
    {
        postStatusMessage("Initializing Pothos plugins...");
        Pothos::init();

        //try to talk to the server on localhost, if not there, spawn a custom one
        //make a server and node that is temporary with this process
        postStatusMessage("Launching scratch process...");
        try
        {
            Pothos::RemoteClient client("tcp://[::1]");
        }
        catch (const Pothos::RemoteClientError &)
        {
            server = Pothos::RemoteServer("tcp://[::1]:"+Pothos::RemoteServer::getLocatorPort());
            //TODO make server background so it does not close with process
            Pothos::RemoteClient client("tcp://[::1]"); //now it should connect to the new server
        }
    }
    catch (const Pothos::Exception &ex)
    {
        QMessageBox msgBox(QMessageBox::Critical, "Pothos Initialization Error", QString::fromStdString(ex.displayText()));
        msgBox.exec();
        return EXIT_FAILURE;
    }

    POTHOS_EXCEPTION_TRY
    {
        //create the main window for the GUI
        std::unique_ptr<QWidget> mainWindow(new PothosGuiMainWindow(nullptr));
        mainWindow->show();
        getSplashScreen()->finish(mainWindow.get());
        getSettings().setParent(mainWindow.get());

        //begin application execution
        return app.exec();
    }
    POTHOS_EXCEPTION_CATCH (const Pothos::Exception &ex)
    {
        QMessageBox msgBox(QMessageBox::Critical, "PothosGui Application Error", QString::fromStdString(ex.displayText()));
        msgBox.exec();
        return EXIT_FAILURE;
    }
}
