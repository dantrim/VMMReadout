// qt
#include <QtCore/QCoreApplication>
#include <QtCore>
#include <QFileInfo>

// std/stl
#include <iostream>
using namespace std;

// vmm
#include "config_handler.h"
#include "configuration_module.h"
#include "run_module.h"
#include "socket_handler.h"
#include "data_handler.h"


void help()
{
    cout<<" ------------------------------------------------------ "<<endl;
    cout<<"   vmmexec usage :" << endl;
    cout<<" $ ./vmmexec -i (--input) <path-to-config-xml> [options]"<<endl;
    cout<<" options :"<<endl;
    cout <<"    -i (--input)            : (required) input config xml"<<endl;
    cout <<"    -c (--send-config)      : only send the configuration"<<endl;
    cout <<"                              (do not begin DAQ)"<<endl;
    cout <<"                              [default: false]" <<endl;
    cout <<"    -r (--run-DAQ)          : only begin DAQ"<<endl;
    cout <<"                              (configuration sending   "<<endl;
    cout <<"                               required!) "<< endl;
    cout <<"                              [default: false]" <<endl;
    cout <<"    --no-write              : do not write ROOT ntuple"<<endl;
    cout <<"                              if running DAQ." << endl;
    cout <<"                              [by default the ntuple is written]" << endl;
    cout <<"    --dry-run               : do not send anything through" << endl;
    cout <<"                              the network " << endl;
    cout <<"                              [default: false]" << endl;
    cout <<"    -d (--dbg)              : set verbose mode on"<<endl;
    cout<<" ------------------------------------------------------ "<<endl;
}


int main(int argc, char *argv[])
{

    // QT
    QCoreApplication app(argc, argv);

    // ---------------------------------------- //
    //  input cmd-options
    QString inputConfigFile("");
    bool sendConfigOnly = false;
    bool runDAQOnly = false; 
    bool writeNtuple = true;
    int dbg = false;
    bool dryrun = false;
    bool config_ = false;

    int optin = 1;
    while(optin < argc)
    {
        string cmd = argv[optin];
        if      (cmd == "-i" || cmd == "--input") {
                    inputConfigFile = argv[++optin];
                    config_ = true;
        }
        else if (cmd == "-d" || cmd == "--dbg")
                    dbg = atoi(argv[++optin]);
        else if (cmd == "-c" || cmd == "--send-config")
                    sendConfigOnly = true;
        else if (cmd == "-r" || cmd == "--run-DAQ")
                    runDAQOnly = true;
        else if (cmd == "--no-write")
                    writeNtuple = false;
        else if (cmd == "--dry-run")
                    dryrun = true;
        else {
            cout << "Incorrect command line options." << endl;
            help();
            exit(1);
        }
        optin++;
    }
    if(sendConfigOnly && runDAQOnly) {
        cout << "You have provided both the '-c' ('--send-config') " << endl;
        cout << "option AND the '-r' ('--run-DAQ') option. If you " << endl;
        cout << "are going to use one of these options, you may only " << endl;
        cout << "provide one of them." << endl;
        exit(1);
    }
    if(!config_) {
        cout << "Input config required." << endl;
        help();
        exit(1);
    }
    else if(inputConfigFile == "") {
        cout << "Provided an empty file name" << endl;
        exit(1);
    }
    else {
        QFileInfo checkConfig(inputConfigFile);
        if(!(checkConfig.exists() && checkConfig.isFile())) {
            cout << "The provided input config file (" << inputConfigFile.toStdString()
                 << ") is not found." << endl;
            exit(1);
        }
    }
    // ---------------------------------------- //

    ConfigHandler conf_handler;
    conf_handler.setDebug(dbg);
    conf_handler.LoadConfig(inputConfigFile);

    cout << "______________" << endl;
    cout << "Testing Socket" << endl;

    SocketHandler socketHandler;
    socketHandler.setDebug(dbg);
    socketHandler.loadIPList(conf_handler.getIPList());
    int pingOK = socketHandler.ping();
    if(!pingOK)
        return 1;

    if(dryrun)
        socketHandler.setDryRun();
    socketHandler.addSocket("FEC", 1235);
    socketHandler.addSocket("DAQ", 1234);
    //socketHandler.addSocket("VMMAPP", 1236); // we only ever send to this, never binding

    
    Configuration configModule;
    configModule.setDebug(dbg);
    

    RunModule runModule;
    runModule.setDebug(dbg).setDoWrite(writeNtuple);
 //   runModule.LoadConfig(conf_handler).LoadSocket(socketHandler);
 //   runModule.initializeDataHandler();

 //   socketHandler.fecSocket().TestUDP();
 //   return app.exec();


    if(!runDAQOnly) {
        // pass the configuration
        configModule.LoadConfig(conf_handler).LoadSocket(socketHandler);
        //configModule.SendConfig();

    }

    if(!sendConfigOnly) {
        // pass the configuration
        runModule.LoadConfig(conf_handler).LoadSocket(socketHandler);
        runModule.initializeDataHandler();
        runModule.setupOutputFiles(conf_handler.daqSettings(),
            "/Users/dantrim/workarea/NSW/VMMReadout/restructure/build",
            "testing.txt");
        socketHandler.fecSocket().TestUDP();

        // pass the DAQ configuration
   //     runModule.prepareRun();
   //     runModule.ACQon();
    }

    if(sendConfigOnly) {
        app.quit();
        return 0;
    }
    else {

        // exit the event loop when DAQ is finished
        QObject::connect(&runModule, SIGNAL(EndRun()), &app, SLOT(quit()));

        // set "Run" method to execute as soon as this application
        // enters into the main event loop
        QMetaObject::invokeMethod(&runModule, "Run", Qt::QueuedConnection);

        // begin the event loop
        return app.exec();
    }

}
