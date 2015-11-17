// qt
#include <QtCore/QCoreApplication>
#include <QFileInfo> // FileExists
#include <QtCore>

// vmmrun
#include "run_module.h"

// std/stl
#include <iostream>
using namespace std;

void help()
{
    qDebug() << " ---------------------------------------------------------------- ";
    qDebug() << " vmmrun usage : ";
    qDebug() << "   $ ./vmmrun -i (--input-xml) <path-to-xml-file> [options]";
    qDebug() << " options :";
    qDebug() << "    -i / --input-xml   : (required) input configuration xml";
    qDebug() << "    -o / --output-name : (optional) provide an output filename";
    qDebug() << "                         to be used instead of the one defined";
    qDebug() << "                         in the input-xml file";
    qDebug() << "    --test-mode        : (optional) toggle test mode on";
    qDebug() << "    -d / --dbg         : (optional) toggle debug mode on";
    qDebug() << " example :";
    qDebug() << "   $ ./vmmrun -i config.xml -o testrun.txt --test-mode"; 
    qDebug() << " ---------------------------------------------------------------- ";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QString inputXMLFile("");
    bool dbg = false;
    bool testMode = false;
    bool useCustomName = false;
    QString customName("");

    // ---------------------------------------
    // take in command line
    int optin(1);
    while(optin < argc) {
        string cmd = argv[optin];
        if(cmd == "-i" || cmd == "--input-xml") { inputXMLFile = argv[++optin]; }
        else if(cmd == "-o" || cmd == "--output-name") { useCustomName = true; customName = argv[++optin]; } 
        else if(cmd == "--test-mode") { testMode = true; }
        else if(cmd == "-d" || cmd == "--dbg") { dbg = true; }
        else if(cmd == "-h" || cmd == "--help") { help(); exit(0); }
        else {
            qDebug() << "Incorrect command-line options! Printing expected usage below.";
            help();
            exit(1);
        }
        optin++;
    } // while optin
    if(inputXMLFile == "") {
        qDebug() << "You did not provide an input file. Use the -i or --input-xml command line option to specify this.";
        help();
        exit(1);
    }
    else {
        QFileInfo checkFile(inputXMLFile);
        if(!(checkFile.exists() && checkFile.isFile())) {
            qDebug() << "The provided input file (" << inputXMLFile << ") is not found. Please check that th epath, etc... are correct.";
            help();
            exit(1);
        }
    }

    // ---------------------------------------
    // begin DAQ
    RunDAQ daq;
    // if the user provided an output name, set this
    if(useCustomName) {
        if(customName == "") { qDebug() << "User-provided output name is \"\". Aborting."; help(); abort(); }
        daq.SetOutputFileName(customName);
    }
    daq.SetDebugMode(dbg);
    daq.SetTestMode(testMode);
    // process the input file and grab the daq configuration
    daq.ReadRFile(inputXMLFile);
    // set up the acquisition mode
    daq.PrepareRun();

    QObject::connect(&daq, SIGNAL(ExitLoop()), &app, SLOT(quit()));
    QMetaObject::invokeMethod(&daq, "Run", Qt::QueuedConnection); //RunDAQ::Run() called as soon as application enters main event loop

    return app.exec();
}
