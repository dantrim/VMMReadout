
// vmm
#include "vmmsocket.h"

// Qt
#include <QByteArray>

// std/stl
#include <sstream>
#include <iostream>
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------------- //
//  VMMSocket
// ----------------------------------------------------------------------- //
/////////////////////////////////////////////////////////////////////////////

VMMSocket::VMMSocket(QObject* parent) :
    QObject(parent),
    m_dbg(false),
    m_name(""),
    m_bindingPort(0),
    m_socket(0)
{
    m_socket = new QUdpSocket(this);
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

}
// ----------------------------------------------------------------------- //
void VMMSocket::bindSocket(quint16 port, QAbstractSocket::BindMode mode)
{
    #warning TODO check status and if already bound
    bool bind;
    if(m_socket) {
        bind = m_socket->bind(port, mode);
    }
    else {
        cout << "socket named " << getName() << " is null!" << endl;
        exit(1);
    }
    if(!bind) {
        cout << "unable to bind socket named " << getName() << endl;
        exit(1);
    }
    else {
        cout << "socket named " << getName() << " successfully "
                  << "bound to port " << port << endl;
    }
}
// ----------------------------------------------------------------------- //
void VMMSocket::TestUDP()
{
    checkAndReconnect("VMMSocket::TestUDP");

    QByteArray data;
    data.append("hello from udp");
    data.append("  from socket named: ");
    data.append(QString::fromStdString(getName()));
    m_socket->writeDatagram(data, QHostAddress::LocalHost, 1234);

    closeAndDisconnect();
}
// ----------------------------------------------------------------------- //
void VMMSocket::readyRead()
{
    QByteArray buffer;
    buffer.resize(m_socket->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;
    m_socket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
    cout << "Socket named: " << getName() << " receiving data" << endl;
    cout << "Message from  : " << sender.toString().toStdString() << endl;
    cout << "Message port  : " << senderPort << endl;
    cout << "Message       : " << buffer.toStdString() << endl;
}
// ----------------------------------------------------------------------- //
quint64 VMMSocket::writeDatagram(const QByteArray& datagram,
            const QHostAddress& host, quint16 port)
{
    return m_socket->writeDatagram(datagram, host, port);
}
// ----------------------------------------------------------------------- //
bool VMMSocket::checkAndReconnect(std::string callingFn)
{
    bool status = true;
    if(m_socket->state() == QAbstractSocket::UnconnectedState) {
        status = false;
        string fn = "";
        if(callingFn!="") fn = "(" + callingFn + ") ";

        if(dbg()) cout << "VMMSocket::checkAndReconnect    " << fn
                        << "About to rebind socket named " << getName()
                        << " to port " << getBindingPort() << endl; 
        bool bnd = m_socket->bind(getBindingPort(), QUdpSocket::ShareAddress);
        if(!bnd) {
            status = false;
            cout << "VMMSocket::checkAndReconnect    " << fn
                 << "ERROR Unable to re-bind socket named " << getName()
                 << " to port " << getBindingPort() << endl;
            closeAndDisconnect(callingFn);
        }
        else {
            status = true;
            if(dbg()) cout << "VMMSocket::checkAndReconnect    " << fn
                           << "Socket named " << getName() << " successfully "
                           << "rebound to port " << getBindingPort() << endl;
        }
    }
    return status;
}
// ----------------------------------------------------------------------- //
void VMMSocket::closeAndDisconnect(std::string callingFn)
{
    string fn = "";
    if(callingFn!="") fn = "(" + callingFn + ") ";
    if(dbg()) cout << "VMMSocket::closeAndDisconnect    " << fn
                   << "Closing and disconnecting from host the socket "
                   << "named " << getName() << " (bound on port "
                   << getBindingPort() << ")" << endl;
    m_socket->close();
    m_socket->disconnectFromHost();
}
// ----------------------------------------------------------------------- //
void VMMSocket::Print()
{
    stringstream ss;
    ss << "   VMMSocket    Name          : " << getName() << endl;
    ss << "   VMMSocket    Bound to port : " << m_socket->localPort() << endl;
    cout << ss.str() << endl;
}
