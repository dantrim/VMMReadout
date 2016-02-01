//vmmrun
#include "run_module.h"
#include <QtNetwork>

RunDAQ::RunDAQ(QObject *parent)
    : QObject(parent)
{
    m_config = new Configuration();
    m_has_config  = false;
    m_is_testmode = false;
    m_writeEvent = false;
    m_dbg = false;
    n_command_counter = 0;

    // this is our socket
    m_socket = new QUdpSocket(this);
    m_socketDAQ = new QUdpSocket(this);

    //vmmip map

    // configuration parameters
    m_tpdelay = -1;
    m_acqsync = -1;
    m_acqwindow = -1;
    m_runcount = -1;
    m_timedrun = -1;
    m_trigperiod = "";
    m_runmode = "";
    m_infilename = "";
    m_outfilepath = "";
    m_outfilename = "";

    m_useCustomName = false;
    m_userGivenName = "";

    //pulser
    n_pulse_count = 0;
}

void RunDAQ::ReadRFile(QString &file)
{
    m_infilename = file;

    QDomDocument doc;
    QFile* qfile = new QFile(file);
    //error checking here may be redundant since we do it in main
    QString errorMsg;
    int errorLine, errorColumn;
    if(!qfile->open(QIODevice::ReadOnly) || !doc.setContent(qfile, true, &errorMsg, &errorLine, &errorColumn)) {
        qDebug() << "[RunDAQ::ReadRFile]    The parameter file (" << file << ") doesn't exist. Aborting.";
        qDebug() << "[RunDAQ::ReadRFile]    Error message from the QDomDocument object: " << errorMsg;
        abort();
    }

    if(m_dbg) qDebug() << "[RunDAQ::ReadRFile]    Reading DAQ configuration from input file (" << file << ")";

    // ReadCFile method (c.f. include/configuration_module.h)
    // > Aborts if tags are not found. If this method succeeds then the datamembers of m_config will be filled.
    m_config->ReadCFile(file);
    m_has_config = true;
    // grab the IP's for the configured boards/hdmis
    m_config->getIPs(m_ips);
    qDebug() << "[RunDAQ::ReadRFile]    Pinging boards";
    m_config->Ping();
    if(m_dbg) {
        qDebug() << "[RunDAQ::ReadRFile]    IP's obtained from configuration:";
        qDebug() << m_ips;
    }

    // attach the sockets to the ports grabbed by Configuration
    if(m_dbg) qDebug() << "[RunDAQ::ReadRFile]    Attaching socket to FECPort: " << m_config->getFECPort();
    bool bind = m_socket->bind(m_config->getFECPort(), QUdpSocket::ShareAddress);
    if(!bind) {
        qDebug() << "[RunDAQ::ReadRFile]    Socket unable to be bound!";
        abort();
    }

    // grab the DAQ configuration items parsed and filled in ReadCFile
    QStringList daqList_str;
    vector<int> daqList_int;
    // getTrigDAQ(QstringList &strList, vector<int> &intList) method (c.f. include/configuration_module.h)
    //  > strList : { Trig_Period, Run_mode, Out_Path, Out_File }
    //  > intList : { TP_Delay, ACQ_Sync, ACQ_Window, Run_count }
    m_config->getTrigDAQ(daqList_str, daqList_int);

    // trig period
    if(daqList_str[0] != "") { m_trigperiod = daqList_str[0]; }
    else {
        qDebug() << "[RunDAQ::ReadRFile]    trigger.period variable returned by ReadCFile is \"\". Aborting.";
        abort();
    }

    // run mode
    if     (daqList_str[1] == "pulser")   { m_runmode = daqList_str[1]; m_timedrun = false; }
    else if(daqList_str[1] == "external") { m_runmode = daqList_str[1]; m_timedrun = true; }
    else {
        qDebug() << "[RunDAQ::ReadRFile]    run.mode value must be one of: pulser or external. You have: " << daqList_str[1];
        abort();
    }


    // tp.delay
    if(daqList_int[0] >= 0) { m_tpdelay = daqList_int[0]; }
    else { qDebug() << "[RunDAQ::ReadRFile]    tp.delay variable returned by ReadCFile is < 0. Aborting."; abort(); }

    // acq.sync
    if(daqList_int[1] >= 0) { m_acqsync = daqList_int[1]; }
    else { qDebug() << "[RunDAQ::ReadRm_runmode = daqList_str[1]; File]    acq.sync variable returned by ReadCFile is < 0. Aborting."; abort(); }

    // acq.window
    if(daqList_int[2] >= 0) { m_acqwindow = daqList_int[2]; }
    else { qDebug() << "[RunDAQ::ReadRFile]    acq.window variable returned by ReadCFile is < 0. Aborting."; abort(); }

    // run.count
    if(daqList_int[3] >= 0) { m_runcount = daqList_int[3]; }
    else { qDebug() << "[RunDAQ::ReadRFile]    run.count variable returned by ReadCFile is < 0. Aborting."; abort(); }

    // output file path
    if(daqList_str[2] != "") { m_outfilepath = daqList_str[2]; }
    else { qDebug() << "[RunDAQ::ReadRFile]    Out_Path variable returned by ReadCFile is \"\". Aborting."; abort(); }

    // output file name
    if(daqList_str[3] != "") { m_outfilename = daqList_str[3]; }
    else { qDebug() << "[RunDAQ::ReadRFile]    Out_File variable returned by ReadCFile is \"\". Aborting."; abort(); }

    // from the output path and output file name, construct the full path'ed output file
    if(!m_outfilepath.endsWith("/")) { m_outfilepath.append("/"); }
    m_outfilepath.append(m_outfilename);

    // if the user provided an output file name, use that one instead of the xml-defined one
    if(m_useCustomName) {
        qDebug() << "[RunDAQ::ReadRFile]    Overriding output filename set in configuration. Using the user-provided name (" << m_userGivenName << ").";
        m_outfilepath = m_userGivenName;
    }

    //set run time from seconds to milliseconds if the run is timed
    if(m_timedrun) { m_runcount = m_runcount * 1000; }

    //grab the channel map (filled in ReadCFile)
    m_channelmap = m_config->getChannelMap(); 
}


void RunDAQ::PrepareRun()
{
    qDebug() << "[RunDAQ::PrepareRun]    Preparing to run.";
    SetTrigAcqConstants();
    SetTriggerMode(m_timedrun); //pulser or external
    ACQOn();
}
void RunDAQ::LoadDAQConstantsFromGUI(int pulserDelay, QString trigPeriod, int acqSync, int acqWindow)
{
    m_tpdelay = pulserDelay;
    m_trigperiod = trigPeriod;
    m_acqsync = acqSync;
    m_acqwindow = acqWindow;
}
void RunDAQ::LoadIPList(QStringList ipListIn)
{
    if(ipListIn.size()==0) {
        qDebug() << "[RunDAQ::LoadIPList]    WARNING Input list of IPs is empty! Exitting.";
        abort();
    }

    foreach (const QString &ip, ipListIn) {
        if(m_dbg) qDebug() << "[RunDAQ::LoadIPList]    Loading IP address : " << ip;
        m_ips << ip;
    }

}

void RunDAQ::SetTrigAcqConstants()
{
    qDebug() << "[RunDAQ::SetTrigAcqConstants]    Setting the trigger/acq. constants";
    bool ok;
    QByteArray datagram;

    // connect the socket for this word
    if(m_socket->state()==QAbstractSocket::UnconnectedState) {
        if(m_dbg) qDebug() << "[RunDAQ::SetTrigAcqConstants]    About to rebind the socket";
        bool bnd = m_socket->bind(6007, QUdpSocket::ShareAddress); // connect to FECPort
        if(!bnd) {
            qDebug() << "[RunDAQ::SetTrigAcqConstants]    WARNING Unable to bind socket to FECPort (6007). Exitting.";
            abort();
        } else {
            if(m_dbg) qDebug() << "[RunDAQ::SetTrigAcqConstants]    Socket binding to FECPort (6007) successful";
        }
    }
        

    // temporarily add the IP here --> will need to grab them from the configuration object if using the GUI
    //m_ips.clear();
    //m_ips << "10.0.0.2";

    foreach(const QString &ip, m_ips) {
        UpdateCounter();
        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0); //rewind

        //header
        QString cmd, cmdType, cmdLength, msbCounter;
        cmd         = "AA";
        cmdType     = "AA";
        cmdLength   = "FFFF";
        msbCounter = "0x80000000";
        //m_channelmap = 3;
        out<<(quint32)(n_command_counter + msbCounter.toUInt(&ok,16))<<(quint16)0<<(quint16)m_channelmap;
        //out<<(quint32)(n_command_counter + msbCounter.toUInt(&ok,16))<<(quint16)9<<(quint16)m_channelmap;
        out<<(quint8)cmd.toUInt(&ok,16)<<(quint8)cmdType.toUInt(&ok,16)<<(quint16)cmdLength.toUInt(&ok,16);

        out<<(quint32)0;
        out<<(quint32)2<<(quint32) m_trigperiod.toInt(&ok,16); //trig period
        out<<(quint32)4<<(quint32) m_tpdelay; // pulser delay
        out<<(quint32)5<<(quint32) m_acqsync; // acq. sync
        out<<(quint32)6<<(quint32) m_acqwindow; // acq. window

        //m_config->Sender(datagram, ip, m_config->getVMMAPPPort(), *m_socket);
        m_config->Sender(datagram, ip, 6600, *m_socket); // send to VMMAPPPort (6600)
        //m_socket->writeDatagram(datagram, QHostAddress(ip), 6600);

        bool read = false;
        read = m_socket->waitForReadyRead(1000);
        if(read) {
            processReply(ip);
        }
        else {
            qDebug() << "[RunDAQ::SetTrigAcqConstants]    Timeout while waiting for replies from VMM while setting trigger constants.";
            if(m_dbg) qDebug() << "[RunDAQ::SetTrigAcqConstants]    Call socket close() and disconnectFromHost()";
            m_socket->close();
            m_socket->disconnectFromHost();
            abort();
        }
    } // loop over ips

    if(m_dbg) qDebug() << "[RunDAQ::SetTrigAcqConstants]    Call socket close() and disconnectFromHost()";
    m_socket->close();
    m_socket->disconnectFromHost();

}

void RunDAQ::SetTriggerMode(bool externalRunMode)
{
    qDebug() << "[RunDAQ::SetTriggerMode]    Setting trigger mode";
    bool ok;
    QByteArray datagram;

    if(m_socket->state()==QAbstractSocket::UnconnectedState) {
        if(m_dbg) qDebug() << "[RunDAQ::SetTriggerMode]    About to rebind the socket";
        bool bnd = m_socket->bind(6007, QUdpSocket::ShareAddress);
        if(!bnd) {
            qDebug() << "[RunDAQ::SetTriggerMode]    WARNING Unable to bind socket to FECPort (6007). Exitting.";
            abort();
        } else {
            if(m_dbg) qDebug() << "[RunDAQ::SetTriggerMode]    Socket binding to FECPort (6007) successful";
        }
    }

    // temporarily add the IP here --> will need to grab them from the configuration object if using the GUI!
    //m_ips.clear();
    //m_ips << "10.0.0.2";

    foreach(const QString &ip, m_ips) {
        UpdateCounter();
        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0); // rewind

        // header
        QString cmd         = "AA";
        QString cmdType     = "AA";
        QString cmdLength   = "FFFF";
        QString msbCounter  = "0x80000000";
        // temporarily add the channel map here --> will need to grab them from the configuration object if using the GUI!
        //m_channelmap=3;
        out<<(quint32)(n_command_counter + msbCounter.toUInt(&ok,16))<<(quint16)0<<(quint16)m_channelmap;
        out<<(quint8)cmd.toUInt(&ok,16)<<(quint8)cmdType.toUInt(&ok,16)<<(quint16)cmdLength.toUInt(&ok,16);
        out<<(quint32)0;
        out<<(quint32)0;
        if(externalRunMode) { //external trigger
            out << (quint32)4;
            qDebug() << "[RunDAQ::SetTriggerMode]    External trigger enabled";
        }
        else if(!externalRunMode) { //interal (pulser)
            out << (quint32)7;
            qDebug() << "[RunDAQ::SetTriggerMode]    Pulser enabled";
        }
        if(m_dbg) qDebug() << "[RunDAQ::SetTriggerMode]    Send: " << datagram.toHex();

        //m_config->Sender(datagram, ip, m_config->getVMMAPPPort(), *m_socket);
        m_config->Sender(datagram, ip, 6600, *m_socket);

        bool read = false;
        read = m_socket->waitForReadyRead(1000);
        if(read) {
            processReply(ip);
        }
        else {
            qDebug() << "[RunDAQ::SetTriggerMode]    Timeout while waiting for replies from VMM while setting trigger mode.";
            if(m_dbg) qDebug() << "[RunDAQ::SetTriggerMode]    Call socket close() and disconnectFromHost()";
            m_socket->close();
            m_socket->disconnectFromHost();
            abort();
        }
    } // loop over ips

    if(m_dbg) qDebug() << "[RunDAQ::SetTriggerMode]    Call socket close() and disconnectFromHost()";
    m_socket->close();
    m_socket->disconnectFromHost();
}

void RunDAQ::ACQOn()
{
    qDebug() << "[RunDAQ::ACQOn]    Setting ACQ On";
    bool ok;
    UpdateCounter();
    QByteArray datagram;

    if(m_socket->state()==QAbstractSocket::UnconnectedState) {
        if(m_dbg) qDebug() << "[RunDAQ::ACQOn]    About to rebind the socket";
        bool bnd = m_socket->bind(6007, QUdpSocket::ShareAddress);
        if(!bnd) {
            qDebug() << "[RunDAQ::ACQOn]    WARNING Unable to bind the socket to FECPort (6007). Exitting.";
            abort();
        } else {
            if(m_dbg) qDebug() << "[RunDAQ::ACQOn]    Socket binding to FECPort (6007) successful";
        }
    }

    // temporarily add the IP here
    //m_ips.clear();
    //m_ips << "10.0.0.2";

    foreach(const QString &ip, m_ips) {
        UpdateCounter();
        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0); // rewind

        //header
        QString cmd, cmdType, cmdLength, msbCounter;
        cmd         = "AA";
        cmdType     = "AA";
        cmdLength   = "FFFF";
        msbCounter  = "0x80000000";
        // temporarily fix the channel map until grabbing it from GUI
        //m_channelmap = 3;
        out << (quint32)(n_command_counter+msbCounter.toUInt(&ok,16)) << (quint16)0 << (quint16)m_channelmap;
        out << (quint8)cmd.toUInt(&ok,16) << (quint8)cmdType.toUInt(&ok,16) << (quint16)cmdLength.toUInt(&ok,16);
        out << (quint32)0;
        out << (quint32)15;
        out << (quint32)1;

        //m_config->Sender(datagram, ip, m_config->getVMMAPPPort(), *m_socket);
        m_config->Sender(datagram, ip, 6600, *m_socket);

        bool read = false;
        read = m_socket->waitForReadyRead(1000);
        if(read) {
            processReply(ip);
            QString datagram_bin, datagram_hex;
            QDataStream out (&m_buffer, QIODevice::WriteOnly);
            datagram_hex = m_buffer.mid(12,4).toHex();
            quint32 tmp32 = m_config->ValueToReplace(datagram_hex, 0, true);
            out.device()->seek(12);
            out << tmp32;
            out.device()->seek(6);
            out << (quint16)2;

            m_config->Sender(datagram, ip, 6600, *m_socket);
            //m_config->Sender(datagram, ip, m_config->getVMMAPPPort(), *m_socket);
        }
        else {
            qDebug() << "[RunDAQ::ACQOn]    WARNING Timeout (1) while waiting for replies from VMM while setting ACQ on. Exitting.";
            if(m_dbg) qDebug() << "[RunDAQ::ACQOn]    Call socket close() and disconnectFromHost()";
            m_socket->close();
            m_socket->disconnectFromHost();
            abort();
        }
        read = m_socket->waitForReadyRead(1000);
        if(read) {
            processReply(ip);
        }
        else {
            qDebug() << "[RunDAQ::ACQOn]    WARNING Timeout (2) while waiting for replies from VMM while setting ACQ on. Exitting.";
            if(m_dbg) qDebug() << "[RunDAQ::ACQOn]    Call socket close() and disconnectFromHost()";
            m_socket->close();
            m_socket->disconnectFromHost();
            abort();
        }
    } // loop over ips

    if(m_dbg) qDebug() << "[RunDAQ::ACQOn]    Call socket close() and disconnectFromHost()";
    m_socket->close();
    m_socket->disconnectFromHost();
}

void RunDAQ::ACQOff()
{
    if(m_dbg) qDebug() << "[RunDAQ::ACQOff]    Call to ACQ off";
    bool ok;
    QByteArray datagram;

    // connect the socket for this word
    if(m_socket->state()==QAbstractSocket::UnconnectedState) {
        if(m_dbg) qDebug() << "[RunDAQ::ACQOff]    About to rebind the socket";
        bool bnd = m_socket->bind(6007, QUdpSocket::ShareAddress);
        if(!bnd) {
            qDebug() << "[RunDAQ::ACQOff]    WARNING Unable to bind socket to FECPort (6007). Exitting.";
            abort();
        } else {
            if(m_dbg) qDebug() << "[RunDAQ::ACQOff]    Socket binding to FECPort (6007) successful";
        }
    }

    // temporarily add the ips here
    //m_ips.clear();
    //m_ips << "10.0.0.2";

    foreach(const QString &ip, m_ips) {
        UpdateCounter();
        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0); // rewind

        //header
        QString cmd, cmdType, cmdLength, msbCounter;
        cmd         = "AA";
        cmdType     = "AA";
        cmdLength   = "FFFF";
        msbCounter  = "0x80000000";
        //temporarily add the channel map
        //m_channelmap = 3;
        out << (quint32)(n_command_counter+msbCounter.toUInt(&ok,16)) << (quint16)0 << (quint16)m_channelmap;
        out << (quint8)cmd.toUInt(&ok,16) << (quint8)cmdType.toUInt(&ok,16) << (quint16)cmdLength.toUInt(&ok,16);
        out << (quint32)0;
        out << (quint32)15;
        out << (quint32)0;

        m_config->Sender(datagram, ip, 6600, *m_socket);
        //m_config->Sender(datagram, ip, m_config->getVMMAPPPort(), *m_socket);

        bool read = false;
        read = m_socket->waitForReadyRead(1000);
        if(read) {
            processReply(ip);

            QString datagram_bin, datagram_hex;
            QDataStream out (&m_buffer, QIODevice::WriteOnly);
            datagram_hex = m_buffer.mid(12,4).toHex();
            quint32 tmp32 = m_config->ValueToReplace(datagram_hex, 0, false);
            out.device()->seek(12);
            out << tmp32;
            out.device()->seek(6);
            out << (quint16) 2; // change to write mode
            m_config->Sender(m_buffer, ip, 6600, *m_socket);
            //m_config->Sender(m_buffer, ip, m_config->getVMMAPPPort(), *m_socket);
        }
        else {
            qDebug() << "[RunDAQ::ACQOff]    WARNING Timeout (1) while waiting for replies from VMM setting ACQ off. Exitting.";
            if(m_dbg) qDebug() << "[RunDAQ::ACQOff]    Call socket close() and disconnectFromHost()";
            m_socket->close();
            m_socket->disconnectFromHost();
            abort();
        }
        read = m_socket->waitForReadyRead(1000);
        if(read) {
            // check command count minus one since previous datagram did not change/update 
            // the header that contains the command counter header but we've called UpdateCounter
            // >> TODO why do we UpdateCounter??
            processReply(ip);
        }
        else {
            qDebug() << "[RunDAQ::ACQOff]    WARNING Timeout (2) while waiting for replies from VMM setting ACQ off. Exitting.";
            if(m_dbg) qDebug() << "[RunDAQ::ACQOff]    Call socket close() and disconnectFromHost()";
            m_socket->close();
            m_socket->disconnectFromHost();
            abort();
        }
    } // loop over ips

    if(m_dbg) qDebug() << "[RunDAQ::ACQOff]    Call socket close() and disconnectFromHost()";
    m_socket->close();
    m_socket->disconnectFromHost();
}

void RunDAQ::Run()
{
    // set up and connect the DAQ socket
    if(m_dbg) qDebug() << "[RunDAQ::Run]    Connecting DAQ socket to DAQPort: " << m_config->getDAQPort();
    bool daq_bound = m_socketDAQ->bind(m_config->getDAQPort());
    if(!daq_bound) {
        qDebug() << "[RunDAQ::Run]    DAQ socket not able to be bound.";
        abort();
    }
    //#warning BINARY DUMP IS COMMENTED OUT!!!
    connect(m_socketDAQ, SIGNAL(readyRead()), this, SLOT(ReadEvent())); // call ReadEvent on any incoming data

    DataHeader();
    qDebug() << "[RunDAQ::Run]    Starting run...";
    if(m_timedrun) {
        TimedRun(m_runcount);
    }
    else {
        PulserRun();
    }
}

void RunDAQ::DataHeader()
{
    qDebug() << "[RunDAQ::DataHeader]    Output file: " << m_outfilepath; //we appended the outfilename to the path 
    m_fileDaq.setFileName(m_outfilepath);
    if(m_fileDaq.exists() && m_is_testmode) {
        qDebug() << "[RunDAQ::DataHeader]    WARNING: Output file exists. Erasing existing version of: " << m_outfilepath;
        m_fileDaq.remove(); //delete existing output to avoid mixing runs
    }
    else if(m_fileDaq.exists()) {
        qDebug() << "[RunDAQ::DataHeader]    ERROR: Output file exists: " << m_outfilepath << ". Aborting to preserve data.";
        abort();
    }

    QTextStream daqInput(&m_fileDaq);
    // SocketState enum (c.f. http://doc.qt.io/qt-5/qabstractsocket.html#SocketState-enum)
    if(m_socketDAQ->state() == QAbstractSocket::BoundState) {
        bool good = m_fileDaq.open(QIODevice::ReadWrite);
        if(!good) {
            qDebug() << "[RunDAQ::DataHeader]    ERROR: The output file: " << m_outfilepath << " cannot be created. Check path/file name. Aborting.";
            abort();
        }

        QStringList ids_, ips_;
        QDir dir_;
        m_config->getIPs(ips_);
        m_config->getVMMIDs(ids_);

        daqInput<<"##########################################################################\n";
        daqInput<<"    Parameter file used: "+dir_.absoluteFilePath(m_infilename)+"\n";
        if(ids_.size()>0) {
            for(int i = 0; i < ips_.size(); i++) {
                daqInput<<"    VMM ID = "+ids_.at(i)+" with IP address: "+ips_.at(i)+"\n";
            }//i
            daqInput<<"    Gain = "+m_config->getGain()+", Peak Time = "+QString::number(m_config->getPeakTime())+", Neighbor Trigger = "+m_config->getNeighborTrigger()+"\n";
            if(m_timedrun){
                daqInput<<"    Run mode = time, Duration = "+QString::number(m_runcount/1000)+" seconds\n";
                daqInput<<"    Threshold DAC = "+QString::number(m_config->getThresholdDAC())+"\n"; 
            }
            else {
                daqInput<<"    Run mode = pulser, Duration = "+QString::number(m_runcount)+" pulses\n";
                daqInput<<"    Threshold DAC = "+QString::number(m_config->getThresholdDAC())+ ", Test Pulse DAC = "+QString::number(m_config->getTestPulseDAC())+"\n"; 
            }
            daqInput<<"    User Comments = "+m_config->getComment()+"\n";
        } //ids_
        else {
            for(int i = 0; i<ips_.size(); i++) {
                daqInput<<"    VMM with IP address: "+ips_.at(i)+"\n";
            } // i
            if(m_timedrun) {
                daqInput<<"    Run mode = time, Duration = "+QString::number(m_runcount/1000)+" seconds\n";
            }
            else {
                daqInput<<"    Run mode = pulser, Duraction = "+QString::number(m_runcount)+" pulses\n";
            }
        }
        daqInput<<"##########################################################################\n";
    } //bounding
    else {
        qDebug() << "[RunDAQ::DataHeader]    DAQ Port problem when binding socket!";
        abort();
    }
}

void RunDAQ::processReply(const QString &sentip, int command_delay)
{
    if(m_dbg) qDebug() << "[RunDAQ::processReply]    Processing datagram replies for IP: " << sentip;
    bool ok;
    QString datagram_hex;

    unsigned int command_count_to_check = n_command_counter - command_delay;
    QHostAddress vmmip;
    m_replies.clear();
    while(m_socket->hasPendingDatagrams()) {
        m_buffer.resize(m_socket->pendingDatagramSize());
        m_socket->readDatagram(m_buffer.data(), m_buffer.size(), &vmmip);

        if(m_dbg) qDebug() << "[RunDAQ::processReply]    Received datagram (in hex): " << m_buffer.toHex() << " from VMM with IP: " << vmmip.toString() << " and message size is: " << m_buffer.size();
        datagram_hex.clear();
        datagram_hex = m_buffer.mid(0,4).toHex();
        quint32 reqReceived = datagram_hex.toUInt(&ok,16);
        if(reqReceived != command_count_to_check) {
            qDebug() << "[RunDAQ::processReply]    Command number received: " << reqReceived << " does not match internal command counter (expected #): " << command_count_to_check;
            abort();
        }
        m_replies.append(vmmip.toString());
    } // while
    // compare list of addresses that sent replies to the list of vmm addresses we sent data to
    if(m_dbg) {
        foreach(const QString &ip, m_replies) {
            qDebug() << "[RunDAQ::processReply]    VMM with IP: " << ip << " sent a reply to command: " << command_count_to_check;
        } // ip
    } // dbg
    if(m_replies.size()>0) {
        foreach(const QString &ip, m_replies) {
            if(ip != sentip) {
                qDebug() << "[RunDAQ::processReply]    VMM with IP: " << ip << " sent a packet at command number: " << command_count_to_check << " which was not actually to it. Out of sync.";
                abort();
            }
        } // ip loop
    }

    if(!m_replies.contains(sentip)) {
        qDebug() << "[RunDAQ::processReply]    VMM with IP: " << sentip << " did not acknowledge command number: " << n_command_counter;
        abort();
    }

    if(m_dbg) qDebug() << "[RunDAQ::processReply]    IP processing ending for " << sentip;
}

void RunDAQ::ReadEvent()
{
    if(m_dbg) qDebug() << "[RunDAQ::ReadEvent]    Receiving event packet";

    QString newline = "\n";
    QTextStream daqInput(&m_fileDaq);
    QString tmpstring;

    QHostAddress vmmip;

    QMap<QString, QList<QByteArray> > datamap;
    datamap.clear();

    while(m_socketDAQ->hasPendingDatagrams()) {
        m_bufferDAQ.resize(m_socketDAQ->pendingDatagramSize());
        m_socketDAQ->readDatagram( m_bufferDAQ.data(), m_bufferDAQ.size(), &vmmip);
        if(m_dbg) qDebug() << "[RunDAQ::ReadEvent]    DAQ datagram read: " << m_bufferDAQ.toHex() << " from vmm with IP: " << vmmip.toString();
        QString ipstr(vmmip.toString());
        QList<QByteArray> arr = datamap.value(ipstr); // { IP : ByteArray = buffer }
        arr.append(m_bufferDAQ);
        datamap.insert(ipstr, arr);

     //   if(m_writeEvent) {
     //       // send the datagram to EventBuilder
     //       if(!m_builder) {
     //           qDebug() << "[RunDAQ::ReadEvent]    Attempting to write output ntuple but EventBuilder object has not been initialized!";
     //           abort();
     //       }
     //       m_builder->writeData(m_bufferDAQ);
     //   }
    }
    QMap<QString, QList<QByteArray> >::iterator it;
    for(it = datamap.begin(); it!=datamap.end(); it++) {
        if(m_dbg) qDebug() << "[RunDAQ::ReadEvent]    Reading data from address: " << it.key();
        //get the datagram for this address
        QList<QByteArray> arr = datamap.value(it.key());
        //iterate over the byte array
        QList<QByteArray>::iterator ba;
        for(ba = arr.begin(); ba != arr.end(); ba++) {
            // read in everything
            if(m_dbg) qDebug() << "[RunDAQ::ReadEvent]      > " << (*ba).toHex();
            daqInput << (*ba).toHex() << newline;
        } // ba
    } // it
}

void RunDAQ::TimedRun(long time)
{
    QTimer::singleShot(time, this, SLOT(RunDAQ::FinishRun()));
}

void RunDAQ::PulserRun()
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(SendPulse()));
    timer->start(100); // the interval at which the timer emits the timeout signal
}

void RunDAQ::SendPulse()
{
    //if(m_dbg) qDebug() << "[RunDAQ::SendPulse]    Sending pulse";
    qDebug() << "[RunDAQ::SendPulse]    Sending pulse";
    bool ok;
    QByteArray datagram;

    // connect the socket for this word
    if(m_socket->state()==QAbstractSocket::UnconnectedState) {
        if(m_dbg) qDebug() << "[RunDAQ::SetTrigAcqConstants]    About to rebind the socket";
        bool bnd = m_socket->bind(6007, QUdpSocket::ShareAddress); // connect to FECPort
        if(!bnd) {
            qDebug() << "[RunDAQ::SetTrigAcqConstants]    WARNING Unable to bind socket to FECPort (6007). Exitting.";
            abort();
        } else {
            if(m_dbg) qDebug() << "[RunDAQ::SetTrigAcqConstants]    Socket binding to FECPort (6007) successful";
        }
    }

    foreach(const QString &ip, m_ips) {
        UpdateCounter();
        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0); // rewind

        // header
        QString header = "FEAD";
        out << (quint32) n_command_counter << (quint16) header.toUInt(&ok,16) << (quint16) 2;
        out << (quint32) 1 << (quint32) 32;

        m_config->Sender(datagram, ip, m_config->getVMMAPPPort(), *m_socket);

        bool read = false;
        read = m_socket->waitForReadyRead(1000);
        if(read) {
            processReply(ip);
        }
        else { qDebug() << "[RunDAQ::SendPulse]    Timeout (1) while waiting for replies from VMM in pulser. Pulse lost.";
        }

        out.device()->seek(12);
        out << (quint32) 0;
        m_config->Sender(datagram, ip, m_config->getVMMAPPPort(), *m_socket);
        read = m_socket->waitForReadyRead(1000);
        if(read) {
            //command counter is the same as in previous processReply (we did not UpdateCounter in between)
            processReply(ip);
        }
        else {
            qDebug() << "[RunDAQ::SendPulse]    Timeout (2) while waiting for replies from VMM in pulser. Pulse lost.";
        }
    } // loop over ips

    m_socket->close();
    m_socket->disconnectFromHost();

    n_pulse_count++;
    if(n_pulse_count == m_runcount) {
        bool read = m_socketDAQ->waitForReadyRead(1000);
        if(read) {
            ReadEvent();
        }
        FinishRun();
    }

}

void RunDAQ::FinishRun()
{
    qDebug() << "[RunDAQ::FinishRun]    Ending run.";
    ACQOff();
    m_socketDAQ->close();
    m_fileDaq.close();
    emit ExitLoop();
}

void RunDAQ::UpdateCounter()
{
    n_command_counter++;
    if(m_dbg) qDebug() << "[RunDAQ::UpdateCounter]    Command counter at: " << n_command_counter;
}
