// qt
#include <QtCore/QCoreApplication>
#include <QtCore>
#include <QFileInfo>

// std/stl
#include <iostream>
using namespace std;

// vmm
//#include "string_utils.h"
#include "config_handler.h"
#include "configuration_module.h"
#include "socket_handler.h"


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
    bool dbg = false;
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
                    dbg = argv[++optin];
        else if (cmd == "-c" || cmd == "--send-config")
                    sendConfigOnly = true;
        else if (cmd == "-r" || cmd == "--run-DAQ")
                    runDAQOnly = true;
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
    conf_handler.LoadConfig(inputConfigFile);

    cout << "______________" << endl;
    cout << "Testing Socket" << endl;

    SocketHandler server;
    server.loadIPList(conf_handler.getIPList()).ping();
    server.setName("FEC");
    SocketHandler client;
    client.loadIPList(conf_handler.getIPList()).ping();
    client.setName("DAQ");
    server.BindSocket(QHostAddress::LocalHost, 1235);
    client.BindSocket(QHostAddress::LocalHost, 1234);
    server.TestUDP();

    Configuration *configModule = new Configuration();
    // pass the configuration
    configModule->LoadConfig(conf_handler).LoadSocket(server);
    configModule->SendConfig();
    

  //  delete testSocket;

    return app.exec(); 
}
