// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MainWindow.hpp"
#include "PothosGuiUtils.hpp"
#include <Pothos/Init.hpp>
#include <Pothos/Remote.hpp>
#include <QMessageBox>
#include <QApplication>
#include <stdexcept>
#include <cstdlib> //EXIT_FAILURE
#include <memory>

int main(int argc, char **argv)
{
    //create the entry point to the GUI
    QApplication app(argc, argv);
    app.setOrganizationName("PothosWare");
    app.setApplicationName("Pothos");

    //setup the application icon
    {
        app.setWindowIcon(QIcon(makeIconPath("PothosGui.png")));
    }

    //perform library initialization with graphical error message on failure
    Pothos::RemoteServer server;
    try
    {
        Pothos::init();

        //try to talk to the server on localhost, if not there, spawn a custom one
        //make a server and node that is temporary with this process
        try
        {
            Pothos::RemoteClient client("tcp://localhost");
        }
        catch (const Pothos::RemoteClientError &)
        {
            server = Pothos::RemoteServer("tcp://localhost:"+Pothos::RemoteServer::getLocatorPort());
            //TODO make server background so it does not close with process
            Pothos::RemoteClient client("tcp://localhost"); //now it should connect to the new server
        }
    }
    catch (const Pothos::Exception &ex)
    {
        QMessageBox msgBox(QMessageBox::Critical, "Pothos Initialization Error", QString::fromStdString(ex.displayText()));
        msgBox.exec();
        return EXIT_FAILURE;
    }

    //create the main window for the GUI
    std::unique_ptr<QWidget> mainWindow(new PothosGuiMainWindow(nullptr));
    mainWindow->show();

    //begin application execution
    return app.exec();
}
