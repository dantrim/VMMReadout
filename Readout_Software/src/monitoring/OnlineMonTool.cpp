
#include "OnlineMonTool.h"

//qt
#include <QtNetwork>
#include <qlocalserver.h>
#include <qlocalsocket.h>
#include <QByteArray>
#include <QDataStream>

//std/stl
#include <iostream>
using std::cout;
using std::endl;


//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  OnlineMonTool
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
OnlineMonTool::OnlineMonTool(QObject *parent) :
    QObject(parent),
    m_server(NULL),
    m_server_name("vmm-mon-server")
{
}

// ------------------------------------------------------------------------ //
void OnlineMonTool::closeMonitoringServer()
{
    QLocalServer::removeServer(QString::fromStdString(m_server_name));
    if(m_server) {
        delete m_server;
    }
}
// ------------------------------------------------------------------------ //
bool OnlineMonTool::initializeServer()
{
    bool ok = true;

    QLocalServer::removeServer(QString::fromStdString(m_server_name));
    if(m_server) {
        delete m_server;
    }

    m_server = new QLocalServer(this);
    if(!m_server->listen(QString::fromStdString(m_server_name))) {
        cout << "OnlineMonTool::initializeServer    Unable to start the server: " << m_server_name << endl;
        cout << "OnlineMonTool::initializeServer    Server error string: " << m_server->errorString().toStdString() << endl;
        ok = false;
    }

    if(ok) {
        #warning TEST THAT WE DO NOT NEED THIS CONNECTION
        cout << "DONT NEED THIS CONNECTION -- DAQ SENDS TO MON REGARDLESS" << endl;
        connect(m_server, SIGNAL(newConnection()), this, SLOT(sendTest()));
    }

    return ok;

}
// ------------------------------------------------------------------------ //
void OnlineMonTool::writeToBuffer(std::string buff)
{
    if(m_string_buff.size()==3) m_string_buff.clear();
    test = buff;
    //m_string_buff.push_back(buff);
    
}
// ------------------------------------------------------------------------ //
void OnlineMonTool::sendTest()
{
    //std::string dummy = "Danny TZ2 0 0 X 43 1 20 60 150 100 100 120";
    //m_string_buff.clear();
    //m_string_buff.push_back(dummy);
   // if(m_string_buff.size()){    
    //    for(int i = 0; i < (int)m_string_buff.size(); i++) {
            QString msg = QString(QString::fromStdString(test));
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out << (quint16)0;
            out << msg;
            out.device()->seek(0);
            out << (quint16)(block.size() - sizeof(quint16));

            QLocalSocket *clientConnection = m_server->nextPendingConnection();
            connect(clientConnection, SIGNAL(disconnected()),
                                clientConnection, SLOT(deleteLater()));

            clientConnection->write(block);
            clientConnection->flush();
            //clientConnection->disconnectFromServer();
            clientConnection->abort();
       // }

   // }

}
// ------------------------------------------------------------------------ //
void OnlineMonTool::sendToMonitoring(std::string event_string)
{
    //m_eventBuffer->clear();
    m_string_buff.clear();
    std::cout << "OMT: " << event_string << std::endl;
    //std::string dummy = "Danny TZ2 0 0 X 43 1 20 60 150 100 100 120";
    //m_string_buff.clear();
    //m_string_buff.push_back(dummy);
    m_string_buff.push_back(event_string);
    if(m_string_buff.size()){    
    QString msg = QString(QString::fromStdString(m_string_buff.back()));

    QDataStream out(m_eventBuffer, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    //out << (quint16)0;
    out << QString(QString::fromStdString(event_string));
//    out << msg;

    //out.device()->seek(0);
    //out << (quint16)(m_eventBuffer->size() - sizeof(quint16));

    QLocalSocket *clientConnection = m_server->nextPendingConnection();
    connect(clientConnection, SIGNAL(disconnected()),
                clientConnection, SLOT(deleteLater()));

    clientConnection->write(*m_eventBuffer);
    clientConnection->flush();
    clientConnection->disconnectFromServer();
    }

}
