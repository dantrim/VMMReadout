// vmm
#include "data_handler.h"
#include "vmmsocket.h"

// qt
#include <QByteArray>
#include <QBitArray>
#include <QUdpSocket>
#include <QMap>
#include <QList>

using namespace std;


//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  DataHandler
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
DataHandler::DataHandler(QObject *parent) :
    QObject(parent),
    m_dbg(false),
    m_daqSocket(0)
{
}
// ------------------------------------------------------------------------ //
DataHandler& DataHandler::LoadDAQSocket(VMMSocket& vmmsocket)
{
    if(!m_daqSocket) {
        m_daqSocket = &vmmsocket;
        connect(&m_daqSocket->socket(), SIGNAL(readyRead()),
                                                this, SLOT(readEvent()));
    }
    else {
        cout << "DataHandler::LoadDAQSocket    WARNING VMMSocket instance is "
             << "already active (non-null)! Will keep the first." << endl;
        return *this;
    }

    if(m_daqSocket) {
        cout << "------------------------------------------------------" << endl;
        cout << " DataHandler::LoadSocket    VMMSocket loaded" << endl;
        m_daqSocket->Print();
        cout << "------------------------------------------------------" << endl;
    }
    else {
        cout << "DataHandler::LoadSocket    ERROR VMMSocket instance null" << endl;
        exit(1);
    }
    return *this;
}

// ------------------------------------------------------------------------ //
void DataHandler::readEvent()
{
    if(dbg()) cout << "DataHandler::readEvent    Receiving event packet..." << endl;

    QHostAddress vmmip;
    QByteArray datagram;
    datagram.clear();

    QMap<QString, QList<QByteArray> > datamap; // [IP : packets received]
    datamap.clear();

    while(daqSocket().socket().hasPendingDatagrams()){
        datagram.resize(daqSocket().socket().pendingDatagramSize());
        daqSocket().socket().readDatagram(datagram.data(), datagram.size(), &vmmip);

     //   ////////////////////////
     //   // dummy testing
     //   datagram.resize(daqSocket().socket().pendingDatagramSize());
     //   daqSocket().socket().readDatagram(datagram.data(), datagram.size(), &vmmip);
     //   cout << "DAQ socket receiving data" << endl;
     //   cout << "MESSAGE from IP : " << vmmip.toString().toStdString() << endl;
     //   cout << "MESSAGE         : " << datagram.toStdString() << endl;

     //   //////////////////////
        if(dbg()) cout << "DataHandler::readEvent    DAQ datagram read: "
                       << datagram.toHex()
                       << " from VMM with IP: " << vmmip.toString().toStdString()
                       << endl;

        QString ip = vmmip.toString();
        QList<QByteArray> arr = datamap.value(ip); // datamap[ip] = arr 
        arr.append(datagram);
    } // while loop

    QMap<QString, QList<QByteArray> >::iterator it;
    for(it = datamap.begin(); it!=datamap.end(); it++) {
        if(dbg()) cout << "DataHandler::readEvent    "
                       << "Reading data from address: " << it.key().toStdString()
                       << endl;
        QList<QByteArray> arr = datamap.value(it.key());
        QList<QByteArray>::iterator bit;
        for(bit = arr.begin(); bit != arr.end(); bit++) {
            if(dbg()) cout << "DataHandler::readEvent    "
                           << " > " << (*ba).toHex().toStdString()
                           << endl;
        } // bit

    } // it

    return;
}




//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  Misc Methods
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
quint32 DataHandler::ValueToReplaceHEX32(QString hex, int bitToChange,
                                                bool newBitValue)
{
    bool ok;

    // initialize a 32 bit word = 0
    QBitArray commandToSend(32,false);
    QString bin, tmp;

    // convert input hex word to binary
    bin = tmp.number(hex.toUInt(&ok,16),2); 

    // copy old word
    for(int i = 0; i < bin.size(); i++) {
        QString bit = bin.at(i);
        commandToSend.setBit(32-bin.size()+i, bit.toUInt(&ok,10));
    } // i

    // now change the bit
    commandToSend.setBit(31-bitToChange, newBitValue);
    QByteArray byteArr = bitsToBytes(commandToSend);
    quint32 tmp32 = byteArr.toHex().toUInt(&ok,16);
 //   QString yep;
 //   yep.setNum(tmp32, 2);
 //   cout << "YEP  : " << yep.toStdString() << endl;
 //   cout << "FINAL : " << QBitArrayToString(commandToSend).toStdString() << endl; 
    return tmp32; 
}
// ------------------------------------------------------------------------ //
QByteArray DataHandler::bitsToBytes(QBitArray bits)
{
    QByteArray outbytes;
    outbytes.resize(bits.count()/8);
    outbytes.fill(0);

    for(int b = 0; b < bits.count(); ++b)
        outbytes[b/8] = ( outbytes.at(b/8) | ( (bits[b]?1:0) << (7-(b%8))));
    return outbytes;
}
// ------------------------------------------------------------------------ //
QBitArray DataHandler::bytesToBits(QByteArray bytes)
{
    QBitArray outbits;
    outbits.resize(bytes.count()*8);

    for(int i = 0; i < bytes.count(); ++i) {
        for(int b = 0; b < 8; ++b) {
            outbits.setBit( i*8+b, bytes.at(i)&(1<<(7-b)) );
        } // b
    } // i
    return outbits;
} 
// ------------------------------------------------------------------------ //
QString DataHandler::QBitArrayToString(const QBitArray& array)
{
    uint value = 0;
    for(uint i = 0; i < array.size(); i++)
    {
        value <<= 1;
        value += (int)array.at(i);
    }

    QString str;
    str.setNum(value, 10);
    
    return str;
}

