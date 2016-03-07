// vmm
#include "run_module.h"

// std/stl
#include <iostream>
using namespace std;

//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  RunModule
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
RunModule::RunModule(QObject *parent) :
    QObject(parent),
    m_dbg(true),
    m_writeNtuple(true),
    m_socketHandler(0),
    m_configHandler(0)
{

}
// ------------------------------------------------------------------------ //
RunModule& RunModule::LoadConfig(ConfigHandler& config)
{
    m_configHandler = &config;
    if(!m_configHandler) {
        cout << "RunModule::LoadConfig    ERROR ConfigHandler instance null" << endl;
        exit(1);
    }
    return *this;
}
// ------------------------------------------------------------------------ //
RunModule& RunModule::LoadSocket(SocketHandler& socket)
{
    m_socketHandler = &socket;

    if(m_socketHandler) {
        cout << "------------------------------------------------------" << endl;
        cout << "RunModule::LoadSocket    SocketHandler instance loaded" << endl;
        m_socketHandler->Print();
        cout << "------------------------------------------------------" << endl;
    }
    else {
        cout << "RunModule::LoadSocket    ERROR SocketHandler instance null" << endl;
        exit(1);
    }

    return *this;
}
// ------------------------------------------------------------------------ //
void RunModule::prepareRun()
{
    if(dbg()) cout << "RunModule::prepareRun    Preparing for DAQ" << endl;

    //configure the FE DAQ configuration
    setTriggerAcqConstants();
}
// ------------------------------------------------------------------------ //
void RunModule::setTriggerAcqConstants()
{
    if(dbg()) cout << "RunModule::setTriggerAcqConstants" << endl;

    bool ok;
    QByteArray datagram;

    // send T/DAQ constants to VMMAPP port
    int send_to_port = config().commSettings().vmmapp_port;

    for(const auto& ip : socket().ipList()) {
        // UPDATE COUNTER, ETC... SHOULD NOW BE DONE
        // SOLELY IN SOCKETHANDLER TO WHICH WE PASS
        // THE DATAGRAMS

        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0); // rewind

        ///////////////////////////
        // header info
        ///////////////////////////
        QString cmd, cmdType, cmdLength, msbCounter;
        cmd         = "AA";
        cmdType     = "AA";
        cmdLength   = "FFFF";
        msbCounter  = "0x80000000"; 
        out << (quint32)(socket().commandCounter() + msbCounter.toUInt(&ok,16))
            << (quint32) 0
            << (quint32) config().getHDMIChannelMap()
            << (quint8)  cmd.toUInt(&ok,16)
            << (quint8)  cmdType.toUInt(&ok,16)
            << (quint16) cmdLength.toUInt(&ok, 16);

        ///////////////////////////
        // trigger constants
        ///////////////////////////
        out << (quint32) 0
            //trigger period
            << (quint32) 2
            << (quint32) config().daqSettings().trigger_period.toInt(&ok,16)
            //pulser delay
            << (quint32) 4
            << (quint32) config().daqSettings().tp_delay
            //acq. sync
            << (quint32) 5
            << (quint32) config().daqSettings().acq_sync
            //acq. window
            << (quint32) 6
            << (quint32) config().daqSettings().acq_window;

        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                "RunModule::setTriggerAcqConstants"); 

    } // ip



    socket().fecSocket().closeAndDisconnect();
    


}
