
#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

// Qt
#include <QObject>
#include <QUdpSocket>
#include <QStringList>

// vmm
#include "vmmsocket.h"

// std/stl
#include <string>


/////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------------- //
//  SocketHandler
// ----------------------------------------------------------------------- //
/////////////////////////////////////////////////////////////////////////////

class SocketHandler : public QObject
{
    Q_OBJECT;

    public :
        explicit SocketHandler(QObject *parent = 0);
        virtual ~SocketHandler(){};
        SocketHandler& setDebug(bool dbg) { m_dbg = dbg; return *this; }
        bool dbg() { return m_dbg; }

        // Load in the list of IP's
        SocketHandler& loadIPList(const QString& iplist);
        QStringList& idList() { return m_idlist; }
        QStringList& ipList() { return m_iplist; }
        SocketHandler& ping();
        bool pinged() { return m_pinged; }

        // update global command counter
        void updateCommandCounter() { n_globalCommandCounter++; }
        quint32 commandCounter() { return n_globalCommandCounter; }

        // add sockets
        void addSocket(std::string name = "",
            quint16 bindingPort = 0,
            QAbstractSocket::BindMode mode = QAbstractSocket::DefaultForPlatform);

        // send data
        void SendDatagram(const QByteArray& datagram, const QString& ip,
                            const quint16& destinationPort,
                            const QString& whichSocket = "",
                            const QString& callingFn = "");

        // retrieve sockets
        VMMSocket& fecSocket()    { return *m_fecSocket; }
        VMMSocket& vmmappSocket() { return *m_vmmappSocket; }
        VMMSocket& daqSocket()    { return *m_daqSocket; }

        // Print
        void Print();

    private :
        bool m_dbg;
        bool m_pinged;
        quint32 n_globalCommandCounter;
        VMMSocket *m_fecSocket;
        VMMSocket *m_vmmappSocket;
        VMMSocket *m_daqSocket;

        QStringList m_idlist;
        QStringList m_iplist;

        // retrieve socket by name
        VMMSocket& getSocket(std::string whichSocket="");

}; // class SocketHandler


#endif
