
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
    name("")
{

    socket = new QUdpSocket();
    // use SocketHandler readyRead
    //socket->bind(QHostAddress::LocalHost, 1234);
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

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
void SocketHandler::BindSocket(const QHostAddress & address, quint16 port,
        QAbstractSocket::BindMode mode)
{
    // TODO : add function for checking if alreayd bound!
    // TODO : ADD STATUS ENUM
    socket->bind(address, port);
}


// ---------------------------------------------------------------------- //
void SocketHandler::TestUDP()
{
    QByteArray data;
    data.append("hello from udp");
    data.append("  from socket named: ");
    data.append(QString::fromStdString(getName()));
    socket->writeDatagram(data, QHostAddress::LocalHost, 1234);

}

// ---------------------------------------------------------------------- //
void SocketHandler::readyRead()
{
    QByteArray buffer;
    buffer.resize(socket->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;
    socket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
    cout << "Socket named: " << name << " is receiving a data packet" << endl;
    cout << "Message from : " << sender.toString().toStdString() << endl;
    cout << "Message port : " << senderPort << endl;
    cout << "Message      : " << buffer.toStdString() << endl;
}
