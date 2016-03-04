
#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

#include <QObject>
#include <QUdpSocket>
#include <QStringList>

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

        void BindSocket(const QHostAddress & address, quint16 port = 0,
                    QAbstractSocket::BindMode mode =
                                QAbstractSocket::DefaultForPlatform);
        void TestUDP();
        void setName(std::string n = "") { name = n; }
        std::string getName() { return name; }

        // Load in the list of IP's
        SocketHandler& loadIPList(const QString& iplist);
        SocketHandler& ping();

    signals :

    public slots :
        void readyRead();

    private :
        bool m_dbg;
        bool m_pinged;
        std::string name;
        QUdpSocket *socket;

        QStringList m_idlist;
        QStringList m_iplist;

}; // class SocketHandler


#endif
