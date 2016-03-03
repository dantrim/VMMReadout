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


void help()
{
    cout<<" ------------------------------------------------------ "<<endl;
    cout<<"   vmmexec usage :" << endl;
    cout<<" $ ./vmmexec -i (--input) <path-to-config-xml> [options]"<<endl;
    cout<<" options :"<<endl;
    cout <<"    -i (--input)            : (required) input config xml"<<endl;
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
        else {
            cout << "Incorrect command line options." << endl;
            help();
            exit(1);
        }
        optin++;
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
    

    return app.exec(); 
}
