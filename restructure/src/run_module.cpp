// vmm
#include "run_module.h"
#include "data_handler.h"

// std/stl
#include <math.h>
#include <iostream>
using namespace std;

// qt
#include <QTimer>

//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  RunModule
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
RunModule::RunModule(QObject *parent) :
    QObject(parent),
    m_dbg(true),
    m_externalTrigger(false),
    m_pulseCount(0),
    m_initSocketHandler(false),
    m_initConfigHandler(false),
    m_socketHandler(0),
    m_configHandler(0)
{
}
// ------------------------------------------------------------------------ //
RunModule& RunModule::LoadConfig(ConfigHandler& inconfig)
{
    m_configHandler = &inconfig;
    if(!m_configHandler) {
        cout << "RunModule::LoadConfig    ERROR ConfigHandler instance null" << endl;
        exit(1);
    }
    else if(dbg()){
        cout << "------------------------------------------------------" << endl;
        cout << "RunModule::LoadConfig    ConfigHandler instance loaded" << endl;
        cout << "------------------------------------------------------" << endl;
    }

    //depending on the run_mode set in the configuration
    //set whether doing a timed run or not
    bool setExternal = false;
    if(config().daqSettings().run_mode=="pulser")
        setExternal = false;
    else if(config().daqSettings().run_mode=="external") {
        setExternal = true;
        config().daqSettings().run_count *= 1000;
    }
    setExternalTrig(setExternal);

    m_initConfigHandler = true;

    return *this;
}
// ------------------------------------------------------------------------ //
RunModule& RunModule::LoadSocket(SocketHandler& socket)
{
    m_socketHandler = &socket;
    if(!m_socketHandler) {
        cout << "RunModule::LoadSocket    ERROR SocketHandler instance null" << endl;
        exit(1);
    }
    else if(dbg()){
        cout << "------------------------------------------------------" << endl;
        cout << "RunModule::LoadSocket    SocketHandler instance loaded" << endl;
        m_socketHandler->Print();
        cout << "------------------------------------------------------" << endl;
    }

    m_initSocketHandler = true;

    return *this;
}
// ------------------------------------------------------------------------ //
void RunModule::prepareRun()
{
    if(dbg()) cout << "RunModule::prepareRun    Preparing for DAQ" << endl;

    //configure the FE DAQ configuration
    setTriggerAcqConstants();
    setTriggerMode();
}
// ------------------------------------------------------------------------ //
void RunModule::Run()
{
    cout << "RunModule::Run    Starting run..." << endl;
    if(externalTrig())
        beginTimedRun();
    else
        beginPulserRun();
}
// ------------------------------------------------------------------------ //
void RunModule::beginTimedRun()
{
    QTimer::singleShot(config().daqSettings().run_count,
                        this, SLOT(finishRun()));
}
// ------------------------------------------------------------------------ //
void RunModule::beginPulserRun()
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(sendPulse()));
    timer->start(100);
}
// ------------------------------------------------------------------------ //
void RunModule::finishRun()
{
    cout << "RunModule::finishRun    Ending Run..." << endl;
    ACQoff();
    socket().closeAndDisconnect("daq","RunModule::finishRun");
    emit EndRun();
}
// ------------------------------------------------------------------------ //
void RunModule::readEvent()
{
    cout << "RunModule::readEvent    This method is not implemented!" << endl;
    exit(1);
    return;
}
// ------------------------------------------------------------------------ //
void RunModule::sendPulse()
{
    if(dbg()) cout << "RunModule::sendPulse    Sending pulse..." << endl;

    bool ok;
    QByteArray datagram;
    int send_to_port = config().commSettings().vmmapp_port;

    for(const auto& ip : socket().ipList()) {
 
        socket().updateCommandCounter();

        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0);

        ///////////////////////////
        // header info
        ///////////////////////////
        QString header = "FEAD";
        out << (quint32) socket().commandCounter()  //[0,3]
            << (quint16) header.toUInt(&ok,16); //[4,5]

        ///////////////////////////
        // header info
        ///////////////////////////
        out << (quint16) 2  //[6,7]
            << (quint32) 1  //[8,11]
            << (quint32) 32; //[12,15]

        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                            "RunModule::SendPulse");

        bool readOK = false;
        readOK = socket().waitForReadyRead("fec");
        if(readOK)
            socket().processReply("fec", ip);
        else
            cout << "RunModule::SendPulse    Timeout [1] while waiting for "
                 << "replies from VMM. Pulse lost." << endl;

        out.device()->seek(12);
        out << (quint32) 0;
        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                        "RunModule::SendPulse");
        readOK = socket().waitForReadyRead("fec");
        if(readOK)
            socket().processReply("fec", ip);
        else
            cout << "RunModule::SendPulse    Timeout [2] wile waiting for "
                 << "replies from VMM. Pulse lost." << endl;
    } // ip loop

    socket().closeAndDisconnect("fec","RunModule::SendPulse");

    updatePulseCount();
    if(pulseCount() == config().daqSettings().run_count) {
        bool read = socket().waitForReadyRead("daq");
        if(read) {
            readEvent();
        }
        finishRun();
    }

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

    socket().updateCommandCounter();

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
        out << (quint32)(socket().commandCounter() + msbCounter.toUInt(&ok,16)) //[0,3]
            << (quint32) 0 //[4,7]
            << (quint32) config().getHDMIChannelMap() //[8,11]
            << (quint8)  cmd.toUInt(&ok,16) //[12]
            << (quint8)  cmdType.toUInt(&ok,16) //[13]
            << (quint16) cmdLength.toUInt(&ok, 16); //[14,15]

        ///////////////////////////
        // trigger constants
        ///////////////////////////
        out << (quint32) 0 //[16,19]
            //trigger period
            << (quint32) 2 //[20,23]
            << (quint32) config().daqSettings().trigger_period.toInt(&ok,16) //[24,27]
            //pulser delay
            << (quint32) 4 //[28,31]
            << (quint32) config().daqSettings().tp_delay //[32,35]
            //acq. sync
            << (quint32) 5 //[36,39]
            << (quint32) config().daqSettings().acq_sync //[40,43]
            //acq. window
            << (quint32) 6 //[44,47]
            << (quint32) config().daqSettings().acq_window; //[48,51]

        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                "RunModule::setTriggerAcqConstants"); 

        bool readOK = true;
        readOK = socket().waitForReadyRead("fec");
        if(readOK) {
            if(dbg()) cout << "RunModule::setTriggerAcqConstants    "
                           << "Processing replies..." << endl;
            socket().processReply("fec", ip);
        }
        else {
            cout << "RunModule::setTriggerAcqConstants    "
                 << "Timeout while waiting for replies from VMM" << endl;
            socket().closeAndDisconnect("fec","RunModule::setTriggerAcqConstants");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec","RunModule::setTriggerAcqConstants");
}
// ------------------------------------------------------------------------ //
void RunModule::setTriggerMode()
{
    if(dbg()) cout << "RunModule::setTriggerMode" << endl;

    bool ok;
    QByteArray datagram;

    // send trigger mode to VMMAPP port
    int send_to_port = config().commSettings().vmmapp_port;

    for(const auto& ip : socket().ipList()) {
        // UPDATE COUNTER, ETC... SHOULD NOW BE DONE
        // SOLELY IN SOCKETHANDLER TO WHICH WE PASS
        // THE DATAGRAMS

        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0); // rewind

    socket().updateCommandCounter();

        ///////////////////////////
        // header info
        ///////////////////////////
        QString cmd, cmdType, cmdLength, msbCounter;
        cmd         = "AA";
        cmdType     = "AA";
        cmdLength   = "FFFF";
        msbCounter  = "0x80000000"; 

        out << (quint32)(socket().commandCounter() + msbCounter.toUInt(&ok,16)) //[0,3]
            << (quint16) 0 //[4,5]
            << (quint16) config().getHDMIChannelMap() //[6,7]
            << (quint8) cmd.toUInt(&ok,16) //[8]
            << (quint8) cmdType.toUInt(&ok,16) //[9]
            << (quint16) cmdLength.toUInt(&ok,16); //[10,11]

        ///////////////////////////
        // trigger mode
        ///////////////////////////
        out << (quint32) 0 //[12,15]
            << (quint32) 0; //[16,19]
        if(config().daqSettings().run_mode == "external"){
            out << (quint32) 4; //[20,23]
            if(dbg()) cout << "RunModule::setTriggerMode    "
                           << "External trigger enabled" << endl;
        } // external
        else {
            out << (quint32) 7; //[20,23]
            if(dbg()) cout << "RunModule::setTriggerMode    "
                           << "Internal trigger enabled" << endl;
        }
        
        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                "RunModule::setTriggerMode");

        bool readOK = true;
        readOK = socket().waitForReadyRead("fec");
        if(readOK) {
            if(dbg()) cout << "RunModule::setTriggerMode    Processing replies..." << endl;
            socket().processReply("fec", ip);
        }
        else {
            cout << "RunModule::setTriggerMode    Timeout while waiting for "
                 << "replies from VMM" << endl;
            socket().closeAndDisconnect("fec","RunModule::setTriggerMode");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec","RunModule::setTriggerMode");
}
// ------------------------------------------------------------------------ //
void RunModule::ACQon()
{
    if(dbg()) cout << "RunModule::ACQon" << endl;

    bool ok;
    QByteArray datagram;

    // send trigger mode to VMMAPP port
    int send_to_port = config().commSettings().vmmapp_port;

    for(const auto& ip : socket().ipList()) {
        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0); //rewind

    #warning IMPLMEENT CORRECT UPDATE OF COMMAND COUNTER
        socket().updateCommandCounter();

        ///////////////////////////
        // header info
        ///////////////////////////
        QString cmd, cmdType, cmdLength, msbCounter;
        cmd        = "AA";
        cmdType    = "AA";
        cmdLength  = "FFFF";
        msbCounter = "0x80000000"; 

        out << (quint32)(socket().commandCounter() + msbCounter.toUInt(&ok,16)) //[0,3]
            << (quint16) 0  //[4,5]
            << (quint16) config().getHDMIChannelMap() //[6,7]
            << (quint8) cmd.toUInt(&ok,16) //[8]
            << (quint8) cmdType.toUInt(&ok,16) //[9]
            << (quint16) cmdLength.toUInt(&ok,16); //[10,11]

        ///////////////////////////
        // ACQ on
        ///////////////////////////
        out << (quint32) 0 //[12,15]
            << (quint32) 15 //[16,19]
            << (quint32) 1; //[20,23]

        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                    "RunModule::ACQon");
        bool readOK = true;
        readOK = socket().waitForReadyRead("fec");
        if(readOK) {
            if(dbg()) cout << "RunModule::ACQon    Processing replies..." << endl;
            socket().processReply("fec", ip);

            #warning DO WE NEED SECOND WORD SENT FOR ACQon?
            /*
            QByteArray buffer = socket().buffer("fec");

            QString binary, hex;
            QDataStream out (&buffer, QIODevice::WriteOnly);
            hex = buffer.mid(12,4).toHex();
            QByteArray hex = datagram.mid(12,4).toHex(); // 32 bit chunk

          //  quint32 yep = hex.toUInt(&ok,16);
          //  QString yepstr;
          //  yepstr.setNum(yep,2);
          //  cout << "BEFORE REPLACE : " << yepstr.toStdString() << endl;
            
            #warning CHECK THAT THIS ACTUALLY SENDS THE 'CORRECT' DATAGRAM
            quint32 tmp32 = DataHandler::ValueToReplaceHEX32(hex, 0, true);
            out.device()->seek(12);
            out << tmp32;
            out.device()->seek(6);
            out << (quint16) 2;
            hex = datagram.mid(12,4).toHex();

            socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                    "RunModule::ACQon");
            */

        } // readOK
        else {
            cout << "RunModule::ACQon    Timeout while waiting for replies "
                 << "from VMM" << endl;
            socket().closeAndDisconnect("fec","RunModule::ACQon");
            exit(1);
        }
    } // ip
    socket().closeAndDisconnect("fec", "RunModule::ACQon");
}
// ------------------------------------------------------------------------ //
void RunModule::ACQoff()
{
    if(dbg()) cout << "RunModule::ACQoff" << endl;

    bool ok;
    QByteArray datagram;

    // send trigger mode to VMMAPP port
    int send_to_port = config().commSettings().vmmapp_port;

    for(const auto& ip : socket().ipList()) {
        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0); //rewind

    #warning IMPLMEENT CORRECT UPDATE OF COMMAND COUNTER
        socket().updateCommandCounter();

        ///////////////////////////
        // header info
        ///////////////////////////
        QString cmd, cmdType, cmdLength, msbCounter;
        cmd         = "AA";
        cmdType     = "AA";
        cmdLength   = "FFFF";
        msbCounter  = "0x80000000";

        out << (quint32)(socket().commandCounter() + msbCounter.toUInt(&ok,16)) //[0,3]
            << (quint16) 0 //[4,5]
            << (quint16) config().getHDMIChannelMap() //[6,7]
            << (quint8) cmd.toUInt(&ok,16) //[8]
            << (quint8) cmdType.toUInt(&ok,16) //[9]
            << (quint16) cmdLength.toUInt(&ok,16); //[10,11]

        ///////////////////////////
        // ACQ off
        ///////////////////////////
        out << (quint32) 0 //[12,15]
            << (quint32) 15 //[16,19]
            << (quint32) 0; //[20,23]

        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                                "RunModule::ACQoff [1]");
        bool readOK = true;
        readOK = socket().waitForReadyRead("fec");
        if(readOK) {
            if(dbg()) cout << "RunModule::ACQoff    Processing replies..." << endl;
            socket().processReply("fec", ip);

            QByteArray buffer = socket().buffer("fec");

            QString bin, hex;
            QDataStream out (&buffer, QIODevice::WriteOnly);
            hex = buffer.mid(12,4).toHex();
            quint32 tmp32 = DataHandler::ValueToReplaceHEX32(hex, 0, false);
            out.device()->seek(12);
            out << tmp32;
            out.device()->seek(6);
            out << (quint16) 2; // change to write mode ?
            socket().SendDatagram(buffer, ip, send_to_port, "fec",
                                                "RunModule::ACQoff [2]");
        }
        else {
            cout << "RunModule::ACQoff [1]    Timeout while waiting for replies "
                 << "from VMM" << endl;
            socket().closeAndDisconnect("fec","RunModule::ACQoff");
            exit(1);
        }

        readOK = socket().waitForReadyRead("fec");
        if(readOK) {
            socket().processReply("fec", ip);
        }
        else {
            cout << "RunModule::ACQoff [2]    Timeout while waiting for replies "
                 << "from VMM" << endl;
            socket().closeAndDisconnect("fec","RunModule::ACQoff"); 
            exit(1);
        }
    } // ip loop

    socket().closeAndDisconnect("fec", "RunModule::ACQoff");
}
// ------------------------------------------------------------------------ //
void RunModule::setEventHeaders(const int bld_info, const int bld_mode)
{
    if(dbg()) cout << "RunModule::setEventHeaders" << endl;

    bool ok;
    QByteArray datagram;

    // send trigger mode to VMMAPP port
    int send_to_port = config().commSettings().vmmapp_port;

    // headers
    QString cmd, msbCounter;
    cmd = "AAAAFFFF";
    msbCounter = "0x80000000";

    // setup the word
    quint32 evbldinfo = (quint32) 256 * pow(2, bld_info);
    quint32 evbldmode = (quint32)bld_mode;

    for(const auto& ip : socket().ipList()) {
        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0); //rewind

    #warning IMPLMEENT CORRECT UPDATE OF COMMAND COUNTER
        socket().updateCommandCounter();

        ///////////////////////////
        // header info
        ///////////////////////////
        out << (quint32)(socket().commandCounter() + msbCounter.toUInt(&ok,16)) //[0,3]
            << (quint32) config().getHDMIChannelMap() //[4,7]
            << (quint32) cmd.toUInt(&ok,16) //[8,11]
            << (quint32) 0; //[12,15]

        ///////////////////////////
        // event header
        ///////////////////////////
        out << (quint32) 10 //[16,19]
            << (quint32) evbldmode //[20,23]
            << (quint32) 12 //[24,27]
        #warning WHY IS bld info hard coded to 1280??
            << (quint32) 1280; //[28,31] 

        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                                "RunModule::setEventHeaders");
        bool readOK = true;
        readOK = socket().waitForReadyRead("fec");
        if(readOK) {
            if(dbg()) cout << "RunModule::setEventHeaders    Processing replies..." << endl;
            socket().processReply("fec", ip);
        } else {
            cout << "RunModule::setEventHeaders    Timeout while waiting for replies "
                 << "from VMM" << endl;
            socket().closeAndDisconnect("fec","RunModule::setEventHeaders");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec", "RunModule::setEventHeaders");
}
// ------------------------------------------------------------------------ //
void RunModule::resetASICs()
{
    if(dbg()) cout << "RunModule::resetASICs" << endl;

    bool ok;
    QByteArray datagram;

    // send trigger mode to VMMAPP port
    int send_to_port = config().commSettings().vmmapp_port;

    // headers
    QString cmd, cmdType, cmdLength, msbCounter;
    cmd = "AA";
    cmdType = "AA";
    cmdLength = "FFFF";
    msbCounter = "0x80000000"; 

    for(const auto& ip : socket().ipList()) {
        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0); //rewind

    #warning IMPLMEENT CORRECT UPDATE OF COMMAND COUNTER
        socket().updateCommandCounter();

        ///////////////////////////
        // header info
        ///////////////////////////
        out << (quint32)(socket().commandCounter() + msbCounter.toUInt(&ok,16)) //[0,3]
            << (quint16) 0 //[4,5] 
            << (quint16) config().getHDMIChannelMap() //[6,7]
            << (quint8) cmd.toUInt(&ok,16) //[8]
            << (quint8) cmdType.toUInt(&ok,16) //[9]
            << (quint16) cmdLength.toUInt(&ok,16); //[10,11]

        ///////////////////////////
        // reset
        ///////////////////////////
        out << (quint32) 0 //[12,15]
            << (quint32) 128 //[16,19]
            << (quint32) 2; //[20,23]

        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                                "RunModule::resetASICs");
        bool readOK = true;
        readOK = socket().waitForReadyRead("fec");
        if(readOK) {
            if(dbg()) cout << "RunModule::resetASICs    Processing replies..." << endl;
            socket().processReply("fec", ip);
        } else {
            cout << "RunModule::resetASICs    Timeout while waiting for replies "
                 << "from VMM" << endl;
            socket().closeAndDisconnect("fec","RunModule::resetASICs");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec", "RunModule::resetASICs");
}
// ------------------------------------------------------------------------ //
void RunModule::resetFEC(bool do_reset)
{
    if(dbg()) cout << "RunModule::resetFEC" << endl;

    bool ok;
    QByteArray datagram;

    // send reset call to FEC port
    int send_to_port = config().commSettings().fec_port;

    // headers
    QString cmd, cmdType, cmdLength, msbCounter;
    cmd = "AA";
    cmdType = "AA";
    cmdLength = "FFFF";
    msbCounter = "0x80000000";

    // setup
    QString address = "FFFFFFFF";
    QString value = "";
    if(do_reset) {
        value = "FFFF8000";
        if(dbg()) cout << "RunModule::resetFEC    Rebooting FEC..." << endl;
    } else {
        value = "FFFF0001";
        if(dbg()) cout << "RunModule::resetFEC    WarmInit FEC..." << endl;
    } 

    for(const auto& ip : socket().ipList()) {
        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0); //rewind

        socket().updateCommandCounter();

        ///////////////////////////
        // header info
        ///////////////////////////
        out << (quint32)(socket().commandCounter() + msbCounter.toUInt(&ok,16)) //[0,3] 
            << (quint16) 0 //[4,5]
            << (quint16) config().getHDMIChannelMap() //[6,7]
            << (quint8) cmd.toUInt(&ok,16) //[8]
            << (quint8) cmdType.toUInt(&ok,16) //[9]
            << (quint16) cmdLength.toUInt(&ok,16); //[10,11]

        ///////////////////////////
        // word
        ///////////////////////////
        out << (quint32) 0 //[12,15]
            << (quint32) address.toUInt(&ok,16) //[16,19]
            << (quint32) value.toUInt(&ok,16); //[20,23]

        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                                "RunModule::resetFEC");
        bool readOK = true;
        readOK = socket().waitForReadyRead("fec");
        if(readOK) {
            if(dbg()) cout << "RunModule::resetFEC    Processing replies..." << endl;
            socket().processReply("fec", ip);
        } else {
            cout << "RunModule::resetFEC    Timeout while waiting for replies "
                 << "from VMM" << endl;
            socket().closeAndDisconnect("fec","RunModule::resetFEC");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec", "RunModule::resetFEC");
}
// ------------------------------------------------------------------------ //
void RunModule::setMask()
{
    if(dbg()) cout << "RunModule::setMask" << endl;

    bool ok;
    QByteArray datagram;

    // send call to vmmapp port
    int send_to_port = config().commSettings().vmmapp_port;

    // header
    QString cmd, cmdType, cmdLength, msbCounter;
    cmd = "AA";
    cmdType = "AA";
    cmdLength = "FFFF";
    msbCounter = "0x80000000";

    for(const auto& ip : socket().ipList()) {
        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0); //rewind

        socket().updateCommandCounter();

        ////////////////////////////
        // header
        ////////////////////////////
        out << (quint32)(socket().commandCounter() + msbCounter.toUInt(&ok,16)) //[0,3]
            << (quint16) 0 //[4,5]
            << (quint16) config().getHDMIChannelMap() //[6,7]
            << (quint8) cmd.toUInt(&ok,16) //[8]
            << (quint8) cmdType.toUInt(&ok,16) //[9]
            << (quint16) cmdLength.toUInt(&ok,16); //[10,11]

        ////////////////////////////
        // command
        ////////////////////////////
        out << (quint32) 0 //[12,15]
            << (quint32) 8 //[16,19]
            << (quint32) config().getHDMIChannelMap(); //[20,23]

        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                                "RunModule::setMask");

        bool readOK = true;
        readOK = socket().waitForReadyRead("fec");
        if(readOK) {
            if(dbg()) cout << "RunModule::setMask    Processing replies..." << endl;
            socket().processReply("fec", ip);
        } else {
            cout << "RunModule::setMask    Timeout while waitinf ro replies "
                 << "from VMM" << endl;
            socket().closeAndDisconnect("fec", "RunModule::setMask");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec", "RunModule::setMask");

}
// ------------------------------------------------------------------------ //
void RunModule::checkLinkStatus()
{
    if(dbg()) cout << "RunModule::checkLinkStatus" << endl;

    bool ok;
    QByteArray datagram;

    // send call to vmmapp port
    int send_to_port = config().commSettings().vmmapp_port;

    // header
    QString cmd = "BBAAFFFF";
    QString msbCounter = "0x80000000"; 

    for(const auto& ip : socket().ipList()) {
        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0); //rewind

        socket().updateCommandCounter();

        ////////////////////////////
        // header
        ////////////////////////////
        out << (quint32)(socket().commandCounter() + msbCounter.toUInt(&ok,16)) //[0,3]
            << (quint32) config().getHDMIChannelMap() //[4,7]
            << (quint32) cmd.toUInt(&ok,16); //[8,11]

        ////////////////////////////
        // command
        ////////////////////////////
        out << (quint32) 0 //[12,15]
            << (quint32) 16; //[16,19]

        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                                "RunModule::checkLinkStatus");

        bool readOK = true;
        readOK = socket().waitForReadyRead("fec");
        if(readOK) {
            if(dbg()) cout << "RunModule::checkLinkStatus    Processing replies..." << endl;
            //socket().processReply("fec", ip);
        } else {
            cout << "RunModule::checkLinkStatus    Timeout while waiting for replies "
                 << "from VMM" << endl;
            socket().closeAndDisconnect("fec", "RunModule::checkLinkStatus");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec", "RunModule::checkLinkStatus");

}
// ------------------------------------------------------------------------ //
void RunModule::resetLinks()
{
    if(dbg()) cout << "RunModule::resetLinks" << endl;

    bool ok;
    QByteArray datagram;

    // send call to vmmapp port
    int send_to_port = config().commSettings().vmmapp_port;

    QString cmd, cmdType, cmdLength, msbCounter, cmdReset;
    cmd = "AA";
    cmdType = "AA";
    cmdLength = "FFFF";
    msbCounter = "0x80000000";
    cmdReset = "FF";

    for(const auto& ip : socket().ipList()) {
        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0); //rewind

        socket().updateCommandCounter();

        ////////////////////////////
        // header (1)
        ////////////////////////////
        out << (quint32)(socket().commandCounter() + msbCounter.toUInt(&ok,16)) //[0,3]
            << (quint16) 0 //[4,5]
            << (quint16) config().getHDMIChannelMap() //[6,7]
            << (quint8) cmd.toUInt(&ok,16) //[8]
            << (quint8) cmdType.toUInt(&ok,16) //[9]
            << (quint16) cmdLength.toUInt(&ok,16); //[10,11]

        ////////////////////////////
        // command (1)
        ////////////////////////////
        out << (quint32) 13 //[12,15]
            << (quint32) cmdReset.toUInt(&ok,16); //[16,19]
        
        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                                "RunModule::resetLinks");

        datagram.clear();
        out.device()->seek(0);
        socket().updateCommandCounter();
        ////////////////////////////
        // header (2)
        ////////////////////////////
        out << (quint32)(socket().commandCounter() + msbCounter.toUInt(&ok,16)) //[0,3]
            << (quint16) 0 //[4,5]
            << (quint16) config().getHDMIChannelMap() //[6,7]
            << (quint8) cmd.toUInt(&ok,16) //[8]
            << (quint8) cmdType.toUInt(&ok,16) //[9]
            << (quint16) cmdLength.toUInt(&ok, 16); //[10,11]

        ////////////////////////////
        // command (2)
        ////////////////////////////
        out << (quint32) 13 //[12,15]
            << (quint32) 0; //[16,19]

        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                                "RunModule::resetLinks");
    } // ip

    socket().closeAndDisconnect("fec", "RunModule::resetLinks");

}
            













