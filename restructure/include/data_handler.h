
#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

// qt
#include <QObject>
#include <QFile>
class QByteArray;
class QBitArray;
class QUdpSocket;

// std/stl
#include <iostream>

// vmm
class VMMSocket;


//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  DataHandler
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
class DataHandler : public QObject
{
    Q_OBJECT

    public :
        explicit DataHandler(QObject *parent = 0);
        virtual ~DataHandler(){};

        DataHandler& setDebug(bool dbg) { m_dbg = dbg; return *this; }
        bool dbg() { return m_dbg; }
        DataHandler& LoadDAQSocket(VMMSocket& socket); 

        /////////////////////////////////////////
        // general methods
        // not necessarily class specific
        /////////////////////////////////////////
        static quint32 ValueToReplaceHEX32(QString datagramHex, int bitToChange,
                                                bool newBitValue);
        static QByteArray bitsToBytes(QBitArray bits);
        static QBitArray bytesToBits(QByteArray bytes); 
        static QString QBitArrayToString(const QBitArray &arr);

        /////////////////////////////////////////
        // data handling
        /////////////////////////////////////////
        VMMSocket& daqSocket() { return *m_daqSocket; }



    private :
        bool m_dbg;
        VMMSocket *m_daqSocket;

        QFile m_daqFile;

        

    signals :

    public slots :
        void readEvent();

    private slots :


}; // class DataHandler

#endif
