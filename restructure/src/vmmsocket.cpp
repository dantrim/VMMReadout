
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
bool VMMSocket::hasPendingDatagrams()
{
    return m_socket->hasPendingDatagrams();
}
// ----------------------------------------------------------------------- //
quint64 VMMSocket::pendingDatagramSize()
{
    return m_socket->pendingDatagramSize();
}
// ----------------------------------------------------------------------- //
quint64 VMMSocket::readDatagram(char* data, quint64 maxSize,
                    QHostAddress* address, quint16* port)
{
    return m_socket->readDatagram(data, maxSize, address, port);
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
    if(dbg()) cout << getName() << " socket receiving data..." << endl;

    if     (getName()=="fec" || getName()=="FEC")
        emit dataReady();
    else if(getName()=="DAQ" || getName()=="daq")
        emit dataReady();
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
void VMMSocket::processReply(const QString &ip_to_check, quint32 cmd_delay,
                                    quint32 globalCount)
{
    if(dbg()) cout << "VMMSocket::processReply    "
                   << "Processing datagram replies for IP: "
                   << ip_to_check.toStdString() << endl; 

    bool ok;
    QString datagram_hex;
    unsigned int cmd_cnt_to_check = globalCount - cmd_delay; 
    QHostAddress vmmIP;

    QStringList replies; 
    replies.clear();

    while(socket().hasPendingDatagrams()) {
        buffer().resize(socket().pendingDatagramSize());
        socket().readDatagram(buffer().data(), buffer().size(), &vmmIP);

        if(dbg()) cout << "VMMSocket::processReply    "
                       << "Received datagram (hex): "
                       << buffer().toHex().toStdString()
                       << " from VMM with IP: " << vmmIP.toString().toStdString()
                       << " and message size is " << buffer().size() << endl;

        datagram_hex.clear();
        datagram_hex = buffer().mid(0,4).toHex();
        quint32 received = datagram_hex.toUInt(&ok,16);
        if(received != cmd_cnt_to_check) {
            cout << "VMMSocket::processReply    Command number received, "
                 << received << ", does not match internal command counter" << endl;
            cout << "VMMSocket::processReply    expected, " << cmd_cnt_to_check
                 << endl;
            exit(1);
        }

        // fill our list of VMM replies
        replies.append(vmmIP.toString());
    }//while

    if(dbg()) {
        for(const auto& ip : replies) {
            cout << "VMMSocket::processReply    VMM with IP ["
                 << ip.toStdString() << "] sent a reply to command "
                 << cmd_cnt_to_check << endl;
        } // ip
    }

    if(replies.size()>0) {
        for(const auto& ip : replies) {
            // unexpected ip has replied
            if(ip != ip_to_check) {
                cout << "VMMSocket::processReply    "
                     << "VMM with IP [" << ip.toStdString()
                     << "] has sent a reply at command " << cmd_cnt_to_check
                     << " to a command not sent to it! Out of sync." << endl;
                exit(1);
            } // unexpected ip
        } // ip
    } // replies > 0

    if(!replies.contains(ip_to_check)) {
        cout << "VMMSocket::processReply    VMM with IP: "
             << ip_to_check.toStdString() << " did not acknowledge command # "
             << cmd_cnt_to_check << endl;
        exit(1);
    }

}
// ----------------------------------------------------------------------- //
void VMMSocket::Print()
{
    stringstream ss;
    ss << "   VMMSocket    Name          : " << getName() << endl;
    ss << "   VMMSocket    Bound to port : " << getBindingPort() << endl;
    ss << "   VMMSocket    Status        : " << m_socket->state() << endl;
    cout << ss.str() << endl;
}
