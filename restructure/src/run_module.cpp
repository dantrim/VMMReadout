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
    m_msg(0),
    m_externalTrigger(false),
    m_pulseCount(0),
    m_initSocketHandler(false),
    m_initConfigHandler(false),
    m_socketHandler(0),
    m_configHandler(0)
{
}
// ------------------------------------------------------------------------ //
void RunModule::LoadMessageHandler(MessageHandler& m)
{
    m_msg = &m;
}
// ------------------------------------------------------------------------ //
RunModule& RunModule::LoadConfig(ConfigHandler& inconfig)
{
    m_configHandler = &inconfig;
    if(!m_configHandler) {
        msg()("ERROR ConfigHandler instance null", "RunModule::LoadConfig", true);
        exit(1);
    }
    else if(dbg()){
        msg()("ConfigHandler instance loaded", "RunModule::LoadConfig");
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
        msg()("ERROR SocketHandler instance null","RunModule::LoadSocket",true);
        exit(1);
    }
    else if(dbg()){
        msg()("SocketHandler instance loaded","RunModule::LoadSocket");
        m_socketHandler->Print();
    }

    m_initSocketHandler = true;

    return *this;
}
// ------------------------------------------------------------------------ //
void RunModule::prepareRun()
{
    if(dbg()) msg()("Preparing for DAQ...","RunModule::prepareRun");

    //configure the FE DAQ configuration
    setTriggerAcqConstants();
    setTriggerMode();
}
// ------------------------------------------------------------------------ //
void RunModule::Run()
{
    msg()("Starting run...","RunModule::Run");
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
    msg()("Ending run...","RunModule::finishRun");
    ACQoff();
    socket().closeAndDisconnect("daq","RunModule::finishRun");
    emit EndRun();
}
// ------------------------------------------------------------------------ //
void RunModule::readEvent()
{
    msg()("This method is not implemented!","RunModule::readEvent", true);
    exit(1);
    return;
}
// ------------------------------------------------------------------------ //
void RunModule::sendPulse()
{
    if(dbg()) msg()("Sending pulse...","RunModule::sendPulse");

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
            msg()("Timeout [1] while waiting for replies from VMM. Pulse lost.",
                    "RunModule::sendPulse");

        out.device()->seek(12);
        out << (quint32) 0;
        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                        "RunModule::SendPulse");
        readOK = socket().waitForReadyRead("fec");
        if(readOK)
            socket().processReply("fec", ip);
        else
            msg()("Timout [2] while waiting for replies from VMM. Pulse lost.",
                    "RunModule::sendPulse");
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
    if(dbg()) msg()("Sending trigger ACQ constants...","RunModule::setTriggerAcqConstants");

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
            if(dbg()) msg()("Processing replies...","RunModule::setTriggerAcqConstants");
            socket().processReply("fec", ip);
        }
        else {
            msg()("Timeout while waiting for replies from VMM",
                        "RunModule::setTriggerAcqConstants", true);
            socket().closeAndDisconnect("fec","RunModule::setTriggerAcqConstants");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec","RunModule::setTriggerAcqConstants");
}
// ------------------------------------------------------------------------ //
void RunModule::setTriggerMode()
{
    if(dbg()) msg()("Setting trigger mode...","RunModule::setTriggerMode");

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
            if(dbg()) msg()("External trigger enabled","RunModule::setTriggerMode");
        } // external
        else {
            out << (quint32) 7; //[20,23]
            if(dbg()) msg()("Internal trigger enabled","RunModule::setTriggerMode");
        }
        
        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                "RunModule::setTriggerMode");

        bool readOK = true;
        readOK = socket().waitForReadyRead("fec");
        if(readOK) {
            if(dbg()) msg()("Processing replies...","RunModule::setTriggerMode");
            socket().processReply("fec", ip);
        }
        else {
            msg()("Timeout while waiting for replies from VMM",
                        "RunModule::setTriggerMode",true);
            socket().closeAndDisconnect("fec","RunModule::setTriggerMode");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec","RunModule::setTriggerMode");
}
// ------------------------------------------------------------------------ //
void RunModule::ACQon()
{
    if(dbg()) msg()("Setting ACQ ON","RunModule::ACQon");

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
            if(dbg()) msg()("Processing replies...", "RunModule::ACQon");
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
            msg()("Timeout while waiting for replies from VMM","RunModule::ACQon",true);
            socket().closeAndDisconnect("fec","RunModule::ACQon");
            exit(1);
        }
    } // ip
    socket().closeAndDisconnect("fec", "RunModule::ACQon");
}
// ------------------------------------------------------------------------ //
void RunModule::ACQoff()
{
    if(dbg()) msg()("Setting ACQ OFF","RunModule::ACQoff");

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

        stringstream sx;
        sx << "AQCOFF command counter = " << socket().commandCounter();
        msg()(sx);sx.str("");

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
            if(dbg()) msg()("Processing replies...","RunModule::ACQoff");
            QByteArray buffer;
            buffer = socket().fecSocket().processReply(ip, 0, socket().commandCounter()); //.processReply("fec", ip);

            //QByteArray buffer = socket().buffer("fec");
            qDebug() << "CRAP : " << buffer.toHex() << " size : " << buffer.size();

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
            msg()("Timeout [1] while waiting for replies from VMM",
                    "RunModule::ACQoff", true); 
            socket().closeAndDisconnect("fec","RunModule::ACQoff");
            exit(1);
        }

        readOK = socket().waitForReadyRead("fec");
        if(readOK) {
            socket().processReply("fec", ip);
        }
        else {
            msg()("Timeout [2] while waiting for replies from VMM",
                    "RunModule::ACQoff", true);
            socket().closeAndDisconnect("fec","RunModule::ACQoff"); 
            exit(1);
        }
    } // ip loop

    socket().closeAndDisconnect("fec", "RunModule::ACQoff");
}
// ------------------------------------------------------------------------ //
void RunModule::setEventHeaders(const int bld_info, const int bld_mode)
{
    if(dbg()) msg()("Setting event headers...","RunModule::setEventHeaders");

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
            if(dbg()) msg()("Processing replies...","RunModule::setEventHeaders");
            socket().processReply("fec", ip);
        } else {
            msg()("Timeout while waiting for replies from VMM",
                    "RunModule::setEventHeaders",true);
            socket().closeAndDisconnect("fec","RunModule::setEventHeaders");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec", "RunModule::setEventHeaders");
}
// ------------------------------------------------------------------------ //
void RunModule::resetASICs()
{
    if(dbg()) msg()("Resetting ASICs...","RunModule::resetASICs");

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
            if(dbg()) msg()("Processing replies...","RunModule::resetASICs");
            socket().processReply("fec", ip);
        } else {
            msg()("Timeout while waiting for replies from VMM",
                    "RunModule::resetASICs",true);
            socket().closeAndDisconnect("fec","RunModule::resetASICs");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec", "RunModule::resetASICs");
}
// ------------------------------------------------------------------------ //
void RunModule::resetFEC(bool do_reset)
{
    if(dbg()) msg()("Resetting FEC...","RunModule::resetFEC");

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
        if(dbg()) msg()("Rebooting FEC...","RunModule::resetFEC");
    } else {
        value = "FFFF0001";
        if(dbg()) msg()("WarmInit FEC...","RunModule::resetFEC");
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
            if(dbg()) msg()("Processing replies...","RunModule::resetFEC");
            socket().processReply("fec", ip);
        } else {
            if(dbg()) msg()("Timeout while waiting for replies from VMM",
                            "RunModule::resetFEC",true);
            socket().closeAndDisconnect("fec","RunModule::resetFEC");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec", "RunModule::resetFEC");
}
// ------------------------------------------------------------------------ //
void RunModule::setMask()
{
    if(dbg()) msg()("Setting mask...","RunModule::setMask");

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
            if(dbg()) msg()("Processing replies...","RunModule::setMask");
            socket().processReply("fec", ip);
        } else {
            msg()("Timeout while waiting for replies from VMM",
                    "RunModule::setMask",true);
            socket().closeAndDisconnect("fec", "RunModule::setMask");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec", "RunModule::setMask");

}
// ------------------------------------------------------------------------ //
void RunModule::checkLinkStatus()
{
    if(dbg()) msg()("Checking link status...","RunModule::checkLinkStatus");

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
            emit checkLinks();
            //if(dbg()) msg()("Processing replies...","RunModule::checkLinkStatus");
            //socket().processReply("fec", ip);
        } else {
            msg()("Timeout while waiting for replies from VMM",
                    "RunModule::checkLinkStatus", true);
            socket().closeAndDisconnect("fec", "RunModule::checkLinkStatus");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec", "RunModule::checkLinkStatus");

}
// ------------------------------------------------------------------------ //
void RunModule::resetLinks()
{
    if(dbg()) msg()("Resetting links...","RunModule::resetLinks");

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
// ------------------------------------------------------------------------ //
void RunModule::s6clocks(int cktk, int ckbc, int ckbc_skew)
{
    if(dbg()) msg()("Setting S6 clocks...","RunModule::s6clocks");

    bool ok;
    QByteArray datagram;

    // send call to s6 port
    int send_to_port = config().commSettings().s6_port;

    QString cmd, msbCounter;
    cmd = "AAAAFFFF";
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
            << (quint32) config().getHDMIChannelMap() //[4,7]
            << (quint32) cmd.toUInt(&ok,16); //[8,11]

        ////////////////////////////
        // command
        ////////////////////////////
        out << (quint32) 0 //[12,15]
            << (quint32) 6 //[16,19]
            << (quint32) (cktk*16) //[20,23]
            << (quint32) 7 //[24,27]
            << (quint32) ( ckbc + (ckbc_skew*16) ); //[28,31]

        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                            "RunModule::s6clocks");

        bool readOK = true;
        readOK = socket().waitForReadyRead("fec");
        if(readOK) {
            if(dbg()) msg()("Processing replies...","RunModule::s6clocks");
            socket().processReply("fec",ip);
        } else {
            msg()("Timout while waiting for replies from VMM",
                    "RunModule::s6clocks", true);
            socket().closeAndDisconnect("fec","RunModule::s6clocks");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec","RunModule::s6clocks");

}
// ------------------------------------------------------------------------ //
void RunModule::configTP(int tpskew, int tpwidth, int tppolarity)
{
    if(dbg()) msg()("Configuring the pulser...","RunModule::configTP");

    bool ok;
    QByteArray datagram;

    // send call to s6 port
    int send_to_port = config().commSettings().s6_port;

    QString cmd, msbCounter;
    cmd = "AAAAFFFF";
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
            << (quint32) config().getHDMIChannelMap() //[4,7]
            << (quint32) cmd.toUInt(&ok,16); //[8,11]

        ////////////////////////////
        // command
        ////////////////////////////
        out << (quint32) 0 //[12,15]
            << (quint32) 2 //[16,19]
            << (quint32) (tpskew + (tpwidth*16) + (tppolarity*128)); //[20,23]

        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                                "RunModule::configTP");

        bool readOK = true;
        readOK = socket().waitForReadyRead("fec");
        if(readOK) {
            if(dbg()) msg()("Processing replies...","RunModule::configTP");
            socket().processReply("fec",ip);
        } else {
            msg()("Timeout while waiting for replies from VMM",
                    "RunModule::configTP", true);
            socket().closeAndDisconnect("fec","RunModule::configTP");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec","RunModule::configTP");

}












