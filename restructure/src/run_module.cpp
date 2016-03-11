// vmm
#include "run_module.h"
#include "data_handler.h"

// std/stl
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
    m_writeNtuple(true),
    m_externalTrigger(false),
    m_pulseCount(0),
    m_initSocketHandler(false),
    m_initConfigHandler(false),
    m_socketHandler(0),
    m_configHandler(0),
    m_dataHandler(0)
{
}
// ------------------------------------------------------------------------ //
RunModule& RunModule::LoadConfig(ConfigHandler& inconfig)
{
    if(!m_configHandler)
        m_configHandler = &inconfig;
    else {
        cout << "RunModule::LoadConfig    WARNING ConfigHandler instance is "
             << "already active (non-null)!" << endl;
        cout << "RunModule::LoadConfig    WARNING Will keep the first "
             << "instance." << endl;
        return *this;
    }
        
    if(!m_configHandler) {
        cout << "RunModule::LoadConfig    ERROR ConfigHandler instance null" << endl;
        exit(1);
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
    if(!m_socketHandler /* !m_initSocketHandler */) {
        m_socketHandler = &socket;
    }
    else {
        cout << "RunModule::LoadSocket    WARNING SocketHandler instance is "
             << "already active (non-null)!" << endl;
        cout << "RunModule::LoadSocket    WARNING Will keep the first "
             << "instance." << endl;
        return *this;
    }

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

    m_initSocketHandler = true;

    return *this;
}

// ------------------------------------------------------------------------ //
RunModule& RunModule::initializeDataHandler()
{
    if(!m_initSocketHandler) {
        cout << "RunModule::initializeDataHandler    "
             << "ERROR SocketHandler must be initialized before calling this"
             << " method!"
             << endl;
        exit(1);
    }
    if(!m_initConfigHandler) {
        cout << "RunModule::initializeDataHandler    "
             << "ERROR ConfigHandler must be initialized before calling this"
             << " method!" << endl;
        exit(1);
    }
      
    cout << "RunModule::initializeDataHandler    Setting up DataHandler..." << endl;  
    m_dataHandler = new DataHandler();
    connect(this, SIGNAL(EndRun()), m_dataHandler, SLOT(EndRun()));
    m_dataHandler->LoadDAQSocket(m_socketHandler->daqSocket());
    m_dataHandler->setDebug(dbg());
    m_dataHandler->setWriteNtuple(writeOut());
    m_dataHandler->setIgnore16((bool)config().daqSettings().ignore16);

    return *this;
}
// ------------------------------------------------------------------------ //
void RunModule::setupOutputFiles(TriggerDAQ& daq, QString outdir,
                                                    QString filename)
{
    if(!m_dataHandler) {
        cout << "RunModule::setupOutputFiles    "
             << "ERROR You must call 'initializeDataHandler' before calling "
             << "this method! Exiting." << endl;
        exit(1);
    }
    m_dataHandler->setupOutputFiles(daq, outdir, filename);
    m_dataHandler->dataFileHeader(config().commSettings(),
                                    config().globalSettings(),
                                    config().daqSettings());
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
            






















