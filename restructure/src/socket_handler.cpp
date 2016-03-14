
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
    m_dryrun(false),
    m_skipProcessing(true),
    n_globalCommandCounter(0),
    m_fecSocket(0),
    m_vmmappSocket(0),
    m_daqSocket(0)
{
}
// ---------------------------------------------------------------------- //
void SocketHandler::setDryRun()
{
    cout << "------------------------------------------------------" << endl;
    cout << " SocketHandler configured for dry run (will not send  " << endl;
    cout << " anything over the network)" << endl;
    cout << "------------------------------------------------------" << endl;
    m_dryrun = true;
}

// ---------------------------------------------------------------------- //
SocketHandler& SocketHandler::loadIPList(const QString& ipstring)
{
    m_iplist.clear();
    m_iplist << ipstring.split(",");
    cout << "Loaded " << m_iplist.size() << " IP addresses." << endl;
    return *this;
}
// ---------------------------------------------------------------------- //
bool SocketHandler::ping()
{
    if(m_dbg) {
        cout << "INFO Pinging IP addresses" << endl;
    }
    if(m_iplist.size()==0) {
        cout << "ERROR There are no IP addresses loaded. Please use" << endl;
        cout << "ERROR SocketHandler::loadIPList." << endl;
    }
    for(const auto& ip : m_iplist) {
        #ifdef __linux__
        int status_code = QProcess::execute("ping", QStringList()<<"-c1"<<ip);
        #elif __APPLE__
        int status_code = QProcess::execute("ping", QStringList()<<"-t1"<<ip);
        #endif

        //////////////////////////////////
        if(!dryrun()) {
            if(status_code == 0)
                m_pinged = true;
            else {
                m_pinged = false;
                cout << "ERROR Unable to successfully ping the IP "
                        << ip.toStdString() << endl;
            } 
        }
        else {
            m_pinged = true;
        }
        //////////////////////////////////
    }
    return m_pinged;
}
// ---------------------------------------------------------------------- //
void SocketHandler::updateCommandCounter()
{
    n_globalCommandCounter++;
    emit commandCounterUpdated();
}
// ---------------------------------------------------------------------- //
void SocketHandler::addSocket(std::string name, quint16 bindingPort,
                                    QAbstractSocket::BindMode mode)
{
    QString lname = QString::fromStdString(name).toLower();
    if(lname=="fec") {
        m_fecSocket = new VMMSocket();
        m_fecSocket->setDebug(dbg());
        m_fecSocket->setName(name);
        m_fecSocket->setBindingPort(bindingPort);
        if(!dryrun())
            m_fecSocket->bindSocket(bindingPort, mode);

        cout << "----------------------------------------------------" << endl;
        cout << " SocketHandler::addSocket    VMMSocket added:" << endl;
        m_fecSocket->Print();
        cout << "----------------------------------------------------" << endl;

    } // fec
    else if(lname=="daq") {
        m_daqSocket = new VMMSocket();
        m_daqSocket->setDebug(dbg());
        m_daqSocket->setName(name);
        m_daqSocket->setBindingPort(bindingPort);
        if(!dryrun())
            m_daqSocket->bindSocket(bindingPort, mode);
        
        cout << "----------------------------------------------------" << endl;
        cout << " SocketHandler::addSocket    VMMSocket added:" << endl;
        m_daqSocket->Print();
        cout << "----------------------------------------------------" << endl;
    } //daq
    else if(lname=="vmmapp") {
        m_vmmappSocket = new VMMSocket();
        m_vmmappSocket->setDebug(dbg());
        m_vmmappSocket->setName(name);
        m_vmmappSocket->setBindingPort(bindingPort);
        if(!dryrun())
            m_vmmappSocket->bindSocket(bindingPort, mode);

        cout << "----------------------------------------------------" << endl;
        cout << " SocketHandler::addSocket    VMMSocket added:" << endl;
        m_vmmappSocket->Print();
        cout << "----------------------------------------------------" << endl;
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

    // get the requested VMMSocket
    VMMSocket& socket = getSocket(whichSocket.toStdString());

    //CHECK STATUS ENUM
    #warning TODO add STATUS ENUM AND CHECK (FSM)
    if(!pinged()) {
        cout << "SocketHandler::SendDatagram    " << fn
             << "ERROR Boards are not in pinged OK state" << endl;
        exit(1);
    }

    // make sure the socket is connected (bound) to the correct port
    if(!dryrun()) {
        if(!socket.checkAndReconnect(callingFn.toStdString()))
            exit(1);
    }

    // now send the data
    bool ok;
    if(dbg()) cout << "SocketHandler::SendDatagram    " << fn
                   << (dryrun() ? "[dry run]" : "")
                   << "  Data from socket '" << socket.getName()
                   << "' sent to (IP,port) = (" 
                   << ip.toStdString() << ", " << destPort << ") :"
                   << endl << "                               "
                   << datagram.toHex().toStdString() << endl;
    if(!dryrun())
        socket.writeDatagram(datagram, QHostAddress(ip), destPort);

    #warning does closing socket here prevent reply processing???
    //socket.closeAndDisconnect();
}
// ---------------------------------------------------------------------- //
bool SocketHandler::waitForReadyRead(std::string name, int msec)
{
    bool status = false;
    if(dryrun() || m_skipProcessing) status = true;
    else {
        VMMSocket& vmmsocket = getSocket(name);
        status = vmmsocket.socket().waitForReadyRead(msec);
    }
    return status;
}
// ---------------------------------------------------------------------- //
void SocketHandler::processReply(std::string name, const QString& ip_to_check,
                    quint32 cmd_delay)
{
    quint32 count = commandCounter();
    cout << "HANDLER : delay = " << cmd_delay << "  global : " << count << endl; 

    if(dryrun() || m_skipProcessing) {
        cout << " SocketHandler::processReply    NOT PROCESSING REPLIES! " << endl;
        return;
    }

    VMMSocket& socket = getSocket(name);
    socket.processReply(ip_to_check, cmd_delay, count);
}
// ---------------------------------------------------------------------- //
void SocketHandler::closeAndDisconnect(std::string name, std::string callingFn)
{
    if(dryrun())
        return;

    VMMSocket& vmmsocket = getSocket(name);
    vmmsocket.closeAndDisconnect(callingFn);
}
// ---------------------------------------------------------------------- //
QByteArray SocketHandler::buffer(std::string name)
{
    VMMSocket& vmmsocket = getSocket(name);
    return vmmsocket.buffer(); 
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
    if(!m_fecSocket && !m_daqSocket && !m_vmmappSocket) {
        cout << "-----------------------------------------" << endl;
        cout << "SocketHandler currently holds no sockets" << endl;
        cout << "-----------------------------------------" << endl;
    }
    if(m_fecSocket)
        fecSocket().Print();
    if(m_daqSocket)
        daqSocket().Print();
    if(m_vmmappSocket)
        vmmappSocket().Print();
}
