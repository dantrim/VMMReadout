
#ifndef ONLINEMONTOOL_H
#define ONLINEMONTOOL_H

//std/stl
#include <string>
#include <vector>

//nsw
//#include "map_handler.h"
//#include "element.h"

// qt
#include <QObject>
#include <QLocalServer>
class QLocalSocket;
class QByteArray;

class OnlineMonTool : public QObject
{

    Q_OBJECT

    public :
        explicit OnlineMonTool(QObject *parent = 0);
        virtual ~OnlineMonTool(){};

        bool initializeServer();

        void sendToMonitoring(std::string event_string);

        void closeMonitoringServer();
        QLocalServer& server() { return *m_server; }
       void writeToBuffer(std::string buff);

    private :
        QLocalServer *m_server;
        std::string m_server_name;
        std::vector<std::string> m_string_buff;
        QByteArray* m_eventBuffer;
        std::string test;

    public slots :
        void sendTest();
        
        

}; // class

#endif
