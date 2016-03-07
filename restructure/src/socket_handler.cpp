
// vmm
#include "socket_handler.h"

// Qt
#include <QProcess>
#include <QByteArray>

// std/stl
#include <iostream>
using namespace std;

////////////////////////////////////////////////////////////////////////////
// ---------------------------------------------------------------------- //
//  SocketHandler
// ---------------------------------------------------------------------- //
////////////////////////////////////////////////////////////////////////////

SocketHandler::SocketHandler(QObject* parent) :
    QObject(parent),
    m_dbg(false),
    m_pinged(false),
    n_globalCommandCounter(0),
    m_fecSocket(0),
    m_vmmappSocket(0),
    m_daqSocket(0)
{

    //Socket = new QUdpSocket();
    //// use SocketHandler readyRead
    ////socket->bind(QHostAddress::LocalHost, 1234);
    //Connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

}

// ---------------------------------------------------------------------- //
SocketHandler& SocketHandler::loadIPList(const QString& ipstring)
{
    m_iplist << ipstring.split(",");
    cout << "Loaded " << m_iplist.size() << " IP addresses." << endl;
    return *this;
}
// ---------------------------------------------------------------------- //
SocketHandler& SocketHandler::ping()
{
    if(m_dbg) {
        cout << "INFO Pinging IP addresses" << endl;
    }
    if(m_iplist.size()==0) {
        cout << "ERROR There are no IP addresses loaded. Please use" << endl;
        cout << "ERROR SocketHandler::loadIPList." << endl;
        exit(1);
    }
    for(const auto& ip : m_iplist) {
        #ifdef __linux__
        int status_code = QProcess::execute("ping", QStringList()<<"-c1"<<ip);
        #elif __APPLE__
        int status_code = QProcess::execute("ping", QStringList()<<"-t1"<<ip);
        #endif
        if(status_code == 0)
            m_pinged = true;
        else {
            m_pinged = false;
            cout << "ERROR Unable to successfully ping the IP "
                    << ip.toStdString() << endl;
            cout << "ERROR >>> Exiting." << endl;
            exit(1);
        } 
    }
    return *this;
}
// ---------------------------------------------------------------------- //
void SocketHandler::addSocket(std::string name, quint16 bindingPort,
                                    QAbstractSocket::BindMode mode)
{
    using namespace std;

    QString lname = QString::fromStdString(name).toLower();
    if(lname=="fec") {
        m_fecSocket = new VMMSocket();
        m_fecSocket->setDebug(dbg());
        m_fecSocket->setName(name);
        m_fecSocket->setBindingPort(bindingPort);
        m_fecSocket->bindSocket(bindingPort, mode);

        std::cout << "----------------------------------------------------" << std::endl;
        std::cout << "FEC VMMSocket [" << m_fecSocket->getName() << "] added" << std::endl;
        m_fecSocket->Print();
        std::cout << "----------------------------------------------------" << std::endl;

    } // fec
    else if(lname=="daq") {
        m_daqSocket = new VMMSocket();
        m_daqSocket->setDebug(dbg());
        m_daqSocket->setName(name);
        m_daqSocket->setBindingPort(bindingPort);
        m_daqSocket->bindSocket(bindingPort, mode);
        
        std::cout << "----------------------------------------------------" << std::endl;
        std::cout << "DAQ VMMSocket [" << m_daqSocket->getName() << "] added" << std::endl;
        m_daqSocket->Print();
        std::cout << "----------------------------------------------------" << std::endl;
    } //daq
    else if(lname=="vmmapp") {
        m_vmmappSocket = new VMMSocket();
        m_vmmappSocket->setDebug(dbg());
        m_vmmappSocket->setName(name);
        m_vmmappSocket->setBindingPort(bindingPort);
        m_vmmappSocket->bindSocket(bindingPort, mode);

        std::cout << "----------------------------------------------------" << std::endl;
        std::cout << "VMMAPP VMMSocket [" << m_vmmappSocket->getName() << "] added" << std::endl;
        m_vmmappSocket->Print();
        std::cout << "----------------------------------------------------" << std::endl;
    } //vmmapp
    else {
        cout << "ERROR addSocket    Currently can only add the fec, daq, or vmmapp "
             << "sockets named 'fec', 'daq', and 'vmmapp', respectively. You have "
             << "attempted to add a socket named: " << name << endl;
        exit(1);
    }
}
// ---------------------------------------------------------------------- //
void SocketHandler::SendDatagram(const QByteArray& datagram, const QString& ip,
            const quint16& destPort, const QString& whichSocket, const QString& callingFn)
{
    std::string fn = "";
    if(callingFn!="") fn = "(" + callingFn.toStdString() + ") ";

    VMMSocket& socket = getSocket(whichSocket.toStdString());

    //CHECK STATUS ENUM
    #warning TODO add STATUS ENUM AND CHECK (FSM)
    if(!pinged()) {
        cout << "SocketHandler::SendDatagram    " << fn
             << "ERROR Boards are not in pinged OK state" << endl;
        exit(1);
    }
    if(!socket.checkAndReconnect(callingFn.toStdString())) exit(1);

    if(dbg()) cout << "SocketHandler::SendDatagram    " << fn
                   << "  IP  : " << ip.toStdString()
                   << "  Data: " << datagram.toHex().toStdString() << endl;
    socket.writeDatagram(datagram, QHostAddress(ip), destPort);

    socket.closeAndDisconnect();
    

}
// ---------------------------------------------------------------------- //
VMMSocket& SocketHandler::getSocket(std::string whichSocket)
{
    if(whichSocket=="") {
        cout << "SocketHandler::getSocket    ERROR This method must be passed "
             << "a string containing the name of the desired socket" << endl;
        exit(1);
    }
    QString lname = QString::fromStdString(whichSocket).toLower();
    if(lname=="fec") {
        if(m_fecSocket) return *m_fecSocket;
        else {
            cout << "SocketHandler::getSocket    Requested socket (fec) is null" << endl;
            exit(1);
        }
    }
    else if(lname=="daq") {
        if(m_daqSocket) return *m_daqSocket;
        else {
            cout << "SocketHandler::getSocket    Requested socket (daq) is null" << endl;
            exit(1);
        }
    }
    else if(lname=="vmmapp") {
        if(m_vmmappSocket) return *m_vmmappSocket;
        else {
            cout << "SocketHandler::getSocket    Requested socket (vmmapp) is null" << endl;
            exit(1);
        }
    }
    else {
        cout << "SocketHandler::getSocket    ERROR Currently can only retrieve the"
             << " fec, daq, or vmmapp sockets. You have attempted to retrieve"
             << " a socket named: " << whichSocket << endl;
        exit(1);
    }
}
// ---------------------------------------------------------------------- //
void SocketHandler::Print()
{
    if(m_fecSocket)
        fecSocket().Print();
    if(m_daqSocket)
        daqSocket().Print();
    if(m_vmmappSocket)
        vmmappSocket().Print();
}
