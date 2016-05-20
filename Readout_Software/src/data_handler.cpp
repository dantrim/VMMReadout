// vmm
#include "data_handler.h"
#include "vmmsocket.h"

// qt
#include <QByteArray>
#include <QBitArray>
#include <QUdpSocket>
#include <QList>
#include <QStringList>
#include <QFileInfo>
#include <QFileInfoList>
#include <QDir>
#include <QThread>

//mmdaq
//#include "createevents.h"
//#include "sharedmemorywriter.h"
//#include "daqconfig.h"

using namespace std;


//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  DataHandler
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
DataHandler::DataHandler(QObject *parent) :
    QObject(parent),
    m_dbg(false),
    m_doMonitoring(false),
    m_monitoringSetup(false),
    m_calibRun(false),
    m_write(false),
    n_daqCnt(0),
    m_ignore16(false),
    m_use_channelmap(false),
    //test shared
    m_daqConf(0),
    m_ce(0),
    m_sh(0),
    //thread
    testDAQSocket(0), 
    m_mapping_file(""),
    m_daqSocket(0),
    m_msg(0),
    m_fileOK(false),
    m_outDir(""),
    m_channel_for_calib(-1),
    m_daqRootFile(0),
    m_rootFileOK(false),
    m_treesSetup(false),
    m_runPropertiesFilled(false),
    m_vmm2(NULL),
    m_runProperties(NULL),
    m_artTree(NULL)
{
   // m_sharedDataStrips.clear();
   // m_daqConf = new DaqConfig();
   // m_daqConf->loadXml("DAQ_config.xml");
   // m_ce = new CreateEvents();
   // m_ce->setDaq(m_daqConf);
   // m_ce->createEvents();
   // m_sh = new SharedMemoryWriter();
   // m_sh->initializeSharedMemory();

    //thread
   // testDAQSocket = new QUdpSocket();
   // bool bind = testDAQSocket->bind(6006, QAbstractSocket::DefaultForPlatform); 
   // if(bind) {
   //     msg()("DAQ SOCKET SUCCESSFULLY BOUND");
   // } else {
   //     msg()("DAQ SOCKET NOT BOUND");
   // }
   // //testDAQSocket->bind(QHostAddress::LocalHost, 1235);
    //connect(testDAQSocket, SIGNAL(readyRead()), this, SLOT(testDAQSocketRead()));
   // connect(testDAQSocket, SIGNAL(readyRead()), this, SLOT(readEvent()));
}
// ------------------------------------------------------------------------ //
void DataHandler::testSharedMem()
{

    if(!m_monitoringSetup)
        setupMonitoring();

    int n = 1000;
    for (int i = 0; i < n; i++){
        //if(n>10) break;
        int chip_number = 1;
        int channel = 30;
        int event = 12;
        int charge = 720;
        int time = 70;
        string x = "10 T6 0 0 X 45 1 20 60 40 40 40 40";
        for(int i = 0; i < 10; i++) {
            m_sharedDataStrips.push_back(x);
            //outvector.push_back(m_ce->getEvent(chip_number, channel, event, charge, time, charge, time));
        }
    m_sh->publishEvent(m_sharedDataStrips);
    m_sharedDataStrips.clear();
    }

}
// ------------------------------------------------------------------------ //
void DataHandler::setupMonitoring()
{
    stringstream sx;
    if(!m_monitoringSetup) {
        //if(dbg()) {
        if(true){
            sx.str("");
            sx << "Initializing SharedMemory and Monitoring tools...";
            msg()(sx, "DataHandler::setupMonitoring");sx.str("");
        }
        m_sharedDataStrips.clear();
        m_daqConf = new DaqConfig();
        m_daqConf->loadXml("DAQ_config.xml");
        m_ce = new CreateEvents();
        m_ce->setDaq(m_daqConf);
        m_ce->buildMaps();
        //m_ce->createEvents();
        m_sh = new SharedMemoryWriter();
        m_sh->initializeSharedMemory();

        m_monitoringSetup = true;
    }

    //m_sharedDataStrips.clear();
    //m_daqConf = new DaqConfig();
    //m_daqConf->loadXml("DAQ_config.xml");
    //m_ce = new CreateEvents();
    //m_ce->setDaq(m_daqConf);
    //m_ce->createEvents();
    //m_sh = new SharedMemoryWriter();
    //m_sh->initializeSharedMemory();
}
// ------------------------------------------------------------------------ //
void DataHandler::connectDAQSocket()
{
    stringstream sx;

    int daqport = 6006;

    if(!testDAQSocket) {
        msg()("Initializing DAQ socket...","DataHandler::connectDAQSocket");
        testDAQSocket = new QUdpSocket();
        connect(testDAQSocket, SIGNAL(readyRead()), this, SLOT(readEvent()));
    }

    if(testDAQSocket->state() == QAbstractSocket::UnconnectedState) {
        if(dbg()){
            sx << "About to re-bind DAQ socket";
            msg()(sx,"DataHandler::connectDAQSocket"); sx.str("");
        }
        bool bnd = testDAQSocket->bind(daqport, QUdpSocket::ShareAddress);
        if(!bnd) {
            sx << "ERROR Unable to re-bind DAQ socket to port " << daqport;
            msg()(sx, "DataHandler::connectDAQSocket"); sx.str(""); 
            if(dbg()) {
                sx << "Closing and disconnecting DAQ socket";
                msg()(sx,"DataHandler::connectDAQSocket"); sx.str("");
            } 
            testDAQSocket->close();
            testDAQSocket->disconnectFromHost();
        } // not bnd correctly
        else {
            if(dbg()) {
                sx << "DAQ socket successfully bound to port " << daqport;
                msg()(sx,"DataHandler::connectDAQSocket"); sx.str("");
            }
        } // bnd ok
    }
}
// ------------------------------------------------------------------------ //
void DataHandler::closeDAQSocket()
{
    // close the socket
    if(dbg()) msg()("Closing DAQ socket", "DataHandler::closeDAQSocket");
    testDAQSocket->close();
    testDAQSocket->disconnectFromHost();
}
// ------------------------------------------------------------------------ //
void DataHandler::testFunction2()
{

    qDebug() << "+++++++++++++++++++++++++++++++";
    qDebug() << "  test call of function 2      ";
    qDebug() << "  > thread: " << QThread::currentThreadId();
    qDebug() << "+++++++++++++++++++++++++++++++";

    stringstream sx;
    sx << "FROM MESSAGE HANDLER, THREAD = " << QThread::currentThreadId();
    msg()(sx,"DataHandler::testFunction");
}
// ------------------------------------------------------------------------ //
void DataHandler::testFunction()
{
    qDebug() << "+++++++++++++++++++++++++++++++";
    qDebug() << "  test call of function        ";
    qDebug() << "  > thread: " << QThread::currentThreadId();
    qDebug() << "+++++++++++++++++++++++++++++++";

   // string thID = QThread::currentThreadId().toString().toStdString();

    stringstream sx;
    sx << "FROM MESSAGE HANDLER, THREAD = " << QThread::currentThreadId();
    msg()(sx,"DataHandler::testFunction");

}
// ------------------------------------------------------------------------ //
void DataHandler::testDAQSocketRead()
{
    QByteArray buffer;
    buffer.resize(testDAQSocket->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;
   
    testDAQSocket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort); 
    //QString sender_port = senderPort.toIPv4Address().toString();

    qDebug() << "--------------------------------";
    qDebug() << "--------------------------------";
    qDebug() << " TEST DAQ SOCKET RECEIVING DATA ";
    qDebug() << "   message   : " << buffer;
    qDebug() << "   from      : " << sender.toString();
    qDebug() << "   from port : " << senderPort;
    qDebug() << "- - - - - - - - - - - - - - - - ";
    qDebug() << "   IN THREAD : " << QThread::currentThreadId();
    qDebug() << "--------------------------------";
    qDebug() << "--------------------------------";

}
// ------------------------------------------------------------------------ //
void DataHandler::LoadMessageHandler(MessageHandler& m)
{
    m_msg = &m;
}
// ------------------------------------------------------------------------ //
void DataHandler::set_monitorData(bool doit)
{
    if(dbg()) {
        stringstream sx;
        if(doit)
            sx << "Setting monitoring on...";
        else {
            sx << "Setting monitoring off...";
        }
        msg()(sx, "DataHandler::set_monitorData");
    }
    m_doMonitoring = doit;
}
// ------------------------------------------------------------------------ //
void DataHandler::clearSharedMemory(/*int*/ /*dummy*/)
{
    if(m_monitoringSetup) {
        if(dbg()) {
            stringstream sx;
            sx << "Clearing shared memory...";
            msg()(sx, "DataHandler::clearSharedMemory");
        }
        m_sh->clearSharedMemory();
    }
}
// ------------------------------------------------------------------------ //
void DataHandler::setUseChannelMap(bool useMap)
{
    m_use_channelmap = useMap;
    if(dbg()) {
        stringstream sx;
        sx << "Using channel map: " << (useMap ? "YES" : "NO");
        msg()(sx,"DataHandler::setUseChannelMap");
    }
}
// ------------------------------------------------------------------------ //
void DataHandler::LoadDAQSocket(VMMSocket& vmmsocket)
{

    if(!m_daqSocket) {
        m_daqSocket = &vmmsocket;
        connect(m_daqSocket, SIGNAL(dataReady()), this, SLOT(readEvent()));
    }
    else {
        if(dbg())
            msg()("WARNING VMMSocket instance is already active (non-null). Will keep the first.",
                "DataHandler::LoadDAQSocket");
        return;
    }

    if(m_daqSocket) {
        msg()("VMMSocket loaded", "DataHandler::LoadDAQSocket");
        m_daqSocket->Print();
    }
    else {
        msg()("ERROR VMMSocket instance null", "DataHandler::LoadDAQSocket", true);
        exit(1);
    }
    return;
}
// ------------------------------------------------------------------------ //
void DataHandler::loadELxChannelMapping(QString mapname)
{
    stringstream sx;

    bool mapOK = true;

    if(mapname=="") {
        msg()("ERROR ELx channel map filename provided is empty",
                            "DataHandler::LoadELxChannelMap");
        mapOK = false;
    }

    // assume for now that use provides only the name, expecting
    // to be stored in /configs/
    #warning make env variable for VMMDIR
    string fullname = "../configs/" + mapname.toStdString();
    if(!checkQFileFound(fullname)) {
        sx << "ERROR Map file (" << fullname << ") not found";
        msg()(sx,"DataHandler::LoadELxChannelMap"); sx.str(""); 
        mapOK = false;
    }

    if(!checkQFileOK(fullname)) {
        sx << "ERROR Map file (" << fullname << ") unable to be opened";
        msg()(sx,"DataHandler::LoadELxChannelMap"); sx.str("");
        mapOK = false;
    }

    if(mapOK && dbg()) {
        sx << "VMM channel map file opened: " << fullname;
        msg()(sx,"DataHandler::LoadELxChannelMap"); sx.str("");
    }

    //cases
    if(mapname=="mini2_map.txt") { m_mapping = "mini2"; m_map_mini2.clear(); }
    else {
        sx << "ERROR Unhandled ELx map loaded: " << fullname;
        msg()(sx,"DataHandler::LoadELxChannelMap"); sx.str("");
        mapOK = false;
    }

    QFile mapfile(QString::fromStdString(fullname));
    mapfile.open(QIODevice::ReadOnly);
    QTextStream in(&mapfile);
    while(!in.atEnd()) {
        QString line = in.readLine();
        if(line.left(1)!="#") {

            ////////////////// mini2 [begin] /////////////////
            if(m_mapping=="mini2") {

                QStringList line_list = line.split("  ", QString::SkipEmptyParts);
                if(line_list.size()) {
                    std::vector<int> chip_list;
                    chip_list.push_back(line_list.at(1).toInt());
                    chip_list.push_back(line_list.at(2).toInt());
                    m_map_mini2.insert(line_list.at(0).toInt(), chip_list);
                } // non-empty line
            } // mini2
            ////////////////// mini2 [end] ///////////////////
        } // not a comment line
    } //while

    ///////////////////////////////////////////////////////////////
    // if the map is not "OK" turn off the use of the channel map
    // altogether
    ///////////////////////////////////////////////////////////////
    if(!mapOK) {
        sx << "WARNING Unable to load ELx channel map properly. Will use VMM \nchannels instead"
           << " of strip numbers.";
        msg()(sx, "DataHandler::loadELxChannelMapping");
        m_use_channelmap = false;
    }

}
// ------------------------------------------------------------------------ //
void DataHandler::setWriteNtuple(bool doit)
{
    m_write = doit;
}
// ------------------------------------------------------------------------ //
void DataHandler::setIgnore16(bool doit)
{
    m_ignore16 = doit;
}
// ------------------------------------------------------------------------ //
void DataHandler::setCalibrationRun(bool doit)
{
    m_calibRun = doit;
    if(dbg())
        msg()("Setting run as calibration run...", "DataHandler::setCalibrationRun");
}
// ------------------------------------------------------------------------ //
void DataHandler::setCalibrationChannel(int channel)
{
    stringstream sx;
    if(dbg()) {
        sx << "Setting calibration channel to " << channel;
        msg()(sx,"DataHandler::setCalibrationChannel"); sx.str("");
    }
    m_channel_for_calib = channel;
}
// ------------------------------------------------------------------------ //
void DataHandler::updateCalibrationState(int gainIdx, int threshDAC, int ampDAC,
                                            int tpSkewIdx, int peakTimeIdx)
{
    m_gain_calib = ConfigHandler::all_gains[gainIdx].toDouble();
    m_dacCounts_calib = threshDAC;
    m_pulserCounts_calib = ampDAC;
    m_tpSkew_calib = ConfigHandler::all_s6TPskews[tpSkewIdx].toDouble();
    m_peakTime_calib = ConfigHandler::all_peakTimes[peakTimeIdx];

    if(dbg()) {
        stringstream sx;
        sx << " ** Updating calibration state ** " << endl;
        sx << "     gain           : " << m_gain_calib << endl;
        sx << "     dac thresh     : " << m_dacCounts_calib << endl;
        sx << "     pulser ampl.   : " << m_pulserCounts_calib << endl;
        sx << "     s6 tp skew     : " << m_tpSkew_calib << endl;
        sx << "     peak/int. time : " << m_peakTime_calib << endl;
        cout << sx.str() << endl;
    }
}
// ------------------------------------------------------------------------ //
void DataHandler::testMultiARG(QString x, QString y, QString z)
{
    stringstream sx;
    sx << "MULTI ARG : " << x.toStdString() << " " << y.toStdString() << " " << z.toStdString();
    msg()(sx, "DataHandler::testMultiARG");

}
// ------------------------------------------------------------------------ //
int DataHandler::channelToStrip(int chipNumber, int channelNumber)
{
    stringstream sx;

    int out_strip_number = -1;
    bool ok = true;

    if(m_mapping=="mini2") {
        if(m_map_mini2.size()==0) {
            sx << "Channel map for mini2 is empty";
            msg()(sx,"DataHandler::channelToStrip");sx.str("");
            ok = false; 
        }
        if(ok) {
            if(chipNumber%2==0) { out_strip_number = m_map_mini2[channelNumber].at(0); }
            else { out_strip_number = m_map_mini2[channelNumber].at(1); }
        }
    } // mini2 map
    else {
        sx << "ERROR Unhandled ELx map loaded: " << m_mapping;
        msg()(sx,"DataHandler::channelToStrip"); sx.str("");
        ok = false;
    }

    return out_strip_number;

}
// ------------------------------------------------------------------------ //
void DataHandler::connectDAQ()
{
    // add a separte method for this so that we can control exactly
    // when we start reading in the data
    connect(m_daqSocket, SIGNAL(dataReady()), this, SLOT(readEvent));
}
// ------------------------------------------------------------------------ //
//bool DataHandler::setupOutputFiles(TriggerDAQ& daq, QString outdir_,
//                                                               QString filename)
bool DataHandler::setupOutputFiles(QString outdir_, QString filename)
{
    stringstream sx;

    bool status = true;
    //////////////////////////////////////////////////////////////////////
    // there are two methods here:
    //  1. if 'outdir' and 'filename' are provided we will use those
    //  2. if 'outdir' and 'filename' are empty "" then we will use the
    //     values from the config XML
    //////////////////////////////////////////////////////////////////////

    QString fullfilename = "";
/*
    // ------ use xml ------- //
    if(outdir_=="" && filename=="") {
        fullfilename = daq.output_path;
        if(!daq.output_path.endsWith("/")) fullfilename.append("/");

        // outputdirectory
        m_outDir = fullfilename;
        QDir outDir(m_outDir);
        if(!outDir.exists()) {
            msg()("ERROR Output directory from config XML does not exist!",
                    "DataHandler::setupOutputFiles");
            status = false;
        }

        //////////////////////////////////////////////////
        // binary dump file
        //////////////////////////////////////////////////
        fullfilename.append(daq.output_filename);
//        m_daqFile.setFileName(fullfilename);
//
//        if(dbg())
//            msg()("Setting output data file to: " + fullfilename.toStdString(),
//                    "DataHandler::setupOutputFiles");
        #warning implement test mode or increment of filename!!
//        if(m_daqFile.exists()) {
//            msg()("Output file exists. Erasing existing version.",
//                    "DataHandler::setupOutputFiles");
//            m_daqFile.remove();
//        }
//        bool fileOK = checkQFileOK(m_daqFile.fileName().toStdString());
//        if(fileOK) {
//            m_daqFile.open(QIODevice::ReadWrite);
//        } 
//        else if(!fileOK) {
//            sx << "WARNING QFile (" << m_daqFile.fileName().toStdString()
//               << ") unable to be opened.\nWARNING Try a new directory/name.";
//            msg()(sx,"DataHandler::setupOutputFiles");sx.str("");
//            status = false;
//        }

        //////////////////////////////////////////////////
        // output root file
        //////////////////////////////////////////////////
        if(writeNtuple()) {
            fullfilename = DataHandler::getRootFileName(m_outDir);
            if(dbg())
                msg()("Setting output ROOT file to: " + fullfilename.toStdString(),
                                    "DataHandler::setupOutputFiles");

            m_daqRootFile = new TFile(fullfilename.toStdString().c_str(), "UPDATE");
            if(m_daqRootFile->IsZombie()) {
                sx << "WARNING ROOT file (" << fullfilename.toStdString() << ") unable"
                   << " to be opened.\nWARNING Try a new directory/name.";
                msg()(sx,"DataHandler::setupOutputFiles"); sx.str(""); 
                status = false;
            } 
            m_rootFileOK = true;
        } // writeNtuple
    } // xml
    // ------ use provided names ------- //
    else if(!(outdir_=="" && filename=="")) {
*/
        m_outDir.clear();
        m_outDir = outdir_;

        //////////////////////////////////////////////////
        // binary dump file
        //////////////////////////////////////////////////

        QDir outdir(m_outDir);
        if(!outdir.exists()) {
            msg()("ERROR Provided output directory does not exist.",
                    "DataHandler::setupOutputFiles");
            status = false;
        }
        QString spacer = "";
        QString filename_dump = "";
        if(!m_outDir.endsWith("/")) spacer = "/";
        if(filename.endsWith(".root")) {
            filename_dump = filename.remove(".root");
            filename_dump = filename_dump + ".txt";
        }
        fullfilename = m_outDir + spacer + "dump_" + filename_dump;

//        m_daqFile.setFileName(fullfilename);
//        if(dbg())
//            msg()("Setting output data file to: " + fullfilename.toStdString(),
//                    "DataHandler::setupOutputFiles");
//        #warning implement test mode or increment of filename!!
//        if(m_daqFile.exists() /*&&testmode*/) {
//            msg()("Output file exists. Erasing existing version.",
//                    "DataHandler::setupOutputFiles");
//            m_daqFile.remove();
//        }
//        bool fileOK = checkQFileOK(m_daqFile.fileName().toStdString());
//        if(fileOK) {
//            m_daqFile.open(QIODevice::ReadWrite);
//        } 
//        else if(!fileOK) {
//            sx << "WARNING QFile (" << m_daqFile.fileName().toStdString()
//               << ") unable to be opened.\nWARNING Try a new directory/name.";
//            msg()(sx,"DataHandler::setupOutputFiles");sx.str("");
//            status = false;
//        }
        //////////////////////////////////////////////////
        // output root file
        //////////////////////////////////////////////////
        if(writeNtuple()) {
            fullfilename = m_outDir + spacer + filename + ".root";
            //fullfilename = DataHandler::getRootFileName(m_outDir);
            if(dbg())
                msg()("Setting output ROOT file to: " + fullfilename.toStdString(),
                        "DataHandler::setupOutputFiles");
            m_daqRootFile = new TFile(fullfilename.toStdString().c_str(), "UPDATE");
            if(m_daqRootFile->IsZombie()) {
                sx << "WARNING ROOT file (" << fullfilename.toStdString() << ") unable"
                   << " to be opened.\nWARNING Try a new directory/name.";
                msg()(sx,"DataHandler::setupOutputFiles"); sx.str(""); 
                status = false;
            } 
            m_rootFileOK = true;
        }
//    } // user-provided

    m_fileOK = status;
    m_rootFileOK = status;

    emit setRunDirOK(status);
    return status;


}
// ------------------------------------------------------------------------ //
int DataHandler::checkForExistingFiles(std::string dirname, int expectedNumber)
{

    // Here we expect that the output ntuple files are formatted as:
    //  run_XXXX.root
    // and that the ~binary dump files are formatted as:
    //  dump_run_XXXX.txt
    // TODO - we should store files as actual binary, no?

    QStringList filters;
    filters << "run_*.root";
    filters << "dump_run_*.txt";
    QDir dir(QString::fromStdString(dirname)); 
    dir.setNameFilters(filters);
    int max_run = -1;
    QFileInfoList listOfFiles = dir.entryInfoList();

    bool ok;
    if(listOfFiles.size()>0) {
        // get max run number of there are existing files
        for(int i = 0; i < listOfFiles.size(); i++) {
            QFileInfo fileInfo = listOfFiles.at(i);
            // avoid directory structure if needed
            QString fname = fileInfo.fileName().split("/").last();
            // get the run number part
            QString number = fname.split("_").last();
            number.replace(".root","");
            number.replace(".txt","");
            int other_run = number.toInt(&ok,10);
            if(other_run > max_run) max_run = other_run;
        } 
    }
    if( (max_run > expectedNumber) && max_run>=0)
        return max_run+1;
    else {
        return expectedNumber;
    }
}
// ------------------------------------------------------------------------ //
bool DataHandler::checkQFileFound(std::string name)
{
    QFile ftest;
    ftest.setFileName(QString::fromStdString(name));
    return ftest.exists();
}
// ------------------------------------------------------------------------ //
bool DataHandler::checkQFileOK(std::string name)
{
    QFile ftest;
    ftest.setFileName(QString::fromStdString(name));
    return ftest.open(QIODevice::ReadWrite);
}
// ------------------------------------------------------------------------ //
QString DataHandler::getRootFileName(const QString& dir)
{
    int number = 0;
    if(m_runPropertiesFilled && !(m_runNumber<0)) {
        number = m_runNumber;
    }
    else {
        stringstream sx;
        sx << "Run properties not yet filled. Will set initial ROOT filename"
           << " to run_0000.root";
        msg()(sx, "DataHandler::getRootFileName");
    }
    QString spacer = "";
    if(!m_outDir.endsWith("/")) spacer = "/";
    QString fname = dir + spacer + "run_%04d.root";
    const char* fname_formed = Form(fname.toStdString().c_str(), number);
    QString fname_final = QString(fname_formed);
    QFile tmpCheckFile(fname_final);
    bool exists = tmpCheckFile.exists();
    while(exists) {
        number += 1;
        fname_formed = Form(fname.toStdString().c_str(), number);
        fname_final = QString(fname_formed);
        QFile check(fname_final);
        exists = check.exists();
    } // while
    return fname_final;
}

// ------------------------------------------------------------------------ //
void DataHandler::dataFileHeader(CommInfo& info, GlobalSetting& global,
                                        TriggerDAQ& daq)
{
    if(!m_fileOK) {
        msg()("Output file has not been set up!",
                "DataHandler::DataFileHeader", true);
        exit(1);
    }
    if(dbg())
        msg()("Building data header...", "DataHandler::DataFileHeader");


    QTextStream out(&m_daqFile);

    QStringList vmmids, vmmips;
    QDir dir;
    vmmips = info.ip_list.split(",");
    vmmids = info.vmm_id_list.split(",");
    #warning TODO store xml filename input to ConfigHandler!
    out<<"##################################################################\n"; 
    //out<<"    Parameter file used: " + dir.absoluteFilePath(xmlfilename) + "\n";
    if(vmmids.size()>0) {
        for(int i = 0; i < vmmips.size(); i++) {
            out<<"    VMM ID: " + vmmids.at(i) + " with IP address: "
                + vmmips.at(i) + "\n";
        } //i
        out<<"    Gain: " + ConfigHandler::all_gains[global.gain] + ", ";
        out<<" Peak Time: "
           + QString::number(ConfigHandler::all_peakTimes[global.peak_time]) + ", ";
        out<<" Neighbor Trigger: " + QString::number(global.neighbor_trigger) + "\n";

        if(daq.run_mode=="external") {
            out<<"    Run mode = time, Duration: "
                + QString::number(daq.run_count/1000) + " seconds\n";
            out<<"    Threshold DAQ: " + QString::number(global.threshold_dac) + "\n";
        }
        else {
            out<<"    Run mode = pulser, Duration: "
                + QString::number(daq.run_count) + " pulses\n";
            out<<"    Threshold DAQ: " + QString::number(global.threshold_dac) + "\n";
        }

        out<<"    User comments: " + info.comment + "\n";
    } //ids>0
    else {
        for(int i = 0; i < vmmips.size(); i++) {
            out<<"    VMM with IP address: " + vmmips.at(i) + "\n";
        } // i
        if(daq.run_mode=="external") {
            out<<"    Run mode = time, Duration: "
                + QString::number(daq.run_count/1000) + " seconds\n";
        }
        else {
            out<<"    Run mode = pulser, Duration: "
                + QString::number(daq.run_count) + " pulses\n";
        }
    }
    out<<"##################################################################\n"; 

}
// ------------------------------------------------------------------------ //
// ------------------------------------------------------------------------ //
void DataHandler::getRunProperties(const GlobalSetting& global,
        int runNumber, int angle, double s6TPSkew)
{
    if(dbg())
        msg()("Filling run properties...",
                "DataHandler::getRunProperties");

    m_gain          = ConfigHandler::all_gains[global.gain].toDouble();
    m_runNumber     = runNumber;
    m_tacSlope      = global.tac_slope;
    m_peakTime      = global.peak_time;
    m_dacCounts     = global.threshold_dac;
    m_pulserCounts  = global.test_pulse_dac;
    m_angle         = angle;
    #warning use config_handler for_ getting tpskew value from index
    m_tpSkew        = s6TPSkew;

    if(writeNtuple()) {
        m_daqRootFile->cd();
        if(!m_runProperties) cout << "TREE NOT UP" << endl;
        m_runProperties->Fill();
        m_runProperties->Write("", TObject::kOverwrite);
        delete m_runProperties;
    }
   
    m_runPropertiesFilled = true; 
}
// ------------------------------------------------------------------------ //
void DataHandler::setupOutputTrees()
{
    // clear the data
    DataHandler::clearData();

    // run properties
    m_runProperties = new TTree("run_properties", "run_properties");

    br_runNumber        = m_runProperties->Branch("runNumber", &m_runNumber);
    br_gain             = m_runProperties->Branch("gain", &m_gain);
    br_tacSlope         = m_runProperties->Branch("tacSlope", &m_tacSlope);
    br_peakTime         = m_runProperties->Branch("peakTime", &m_peakTime);
    br_dacCounts        = m_runProperties->Branch("dacCounts", &m_dacCounts);
    br_pulserCounts     = m_runProperties->Branch("pulserCounts", &m_pulserCounts);
    br_s6TPskew         = m_runProperties->Branch("tpSkew", &m_tpSkew);
    br_angle            = m_runProperties->Branch("angle", &m_angle);
    br_calibrationRun   = m_runProperties->Branch("calibrationRun", &m_calibRun);

    // event data
    m_vmm2 = new TTree("vmm2", "vmm2");
    br_eventNumberFAFA      = m_vmm2->Branch("eventFAFA", &m_eventNumberFAFA);
    br_triggerTimeStamp     = m_vmm2->Branch("triggerTimeStamp",
                                "std::vector<int>", &m_triggerTimeStamp);
    br_triggerCounter       = m_vmm2->Branch("triggerCounter",
                                "std::vector<int>", &m_triggerCounter);
    br_chipId               = m_vmm2->Branch("chip",
                                "std::vector<int>", &m_chipId);
    br_evSize               = m_vmm2->Branch("eventSize",
                                "std::vector<int>", &m_eventSize);
    br_tdo                  = m_vmm2->Branch("tdo",
                                "std::vector< vector<int> >", &m_tdo);
    br_pdo                  = m_vmm2->Branch("pdo",
                                "std::vector< vector<int> >", &m_pdo);
    br_flag                 = m_vmm2->Branch("flag",
                                "std::vector< vector<int> >", &m_flag);
    br_thresh               = m_vmm2->Branch("threshold",
                                "std::vector< vector<int> >", &m_threshold);
    br_bcid                 = m_vmm2->Branch("bcid",
                                "std::vector< vector<int> >", &m_bcid);
    br_grayDecoded          = m_vmm2->Branch("grayDecoded",
                                "std::vector< vector<int> >", &m_grayDecoded);
    br_channelId            = m_vmm2->Branch("channel",
                                "std::vector< vector<int> >", &m_channelId);

    if(calibRun()) {
        br_pulserCalib      = m_vmm2->Branch("pulser",
                                                &m_pulserCounts_calib);
        br_gainCalib        = m_vmm2->Branch("gain",
                                                &m_gain_calib);
        br_peakTimeCalib    = m_vmm2->Branch("intTime",
                                                &m_peakTime_calib);
        br_threshCalib      = m_vmm2->Branch("thresholdSet",
                                                &m_dacCounts_calib);
        br_s6TPskewCalib      = m_vmm2->Branch("tpSkew",
                                                &m_tpSkew_calib);
        br_calibRun         = m_vmm2->Branch("calibrationRun",
                                                &m_calibRun);
        br_neighborCalib    = m_vmm2->Branch("neighbor",
                                "std::vector< vector<int> >", &m_neighbor_calib);
    } //calib

    // ART data
    m_artTree = new TTree("ART", "ART");
    br_art                  = m_artTree->Branch("art",
                                "std::vector<int>", &m_art);
    br_artFlag             = m_artTree->Branch("artFlag",
                                "std::vector<int>", &m_artFlag);

    m_treesSetup = true;

}
// ------------------------------------------------------------------------ //
void DataHandler::writeAndCloseDataFile()
{

  //  // close the socket
  //  msg()("Closing DAQ socket", "DataHandler::writeAndCloseDataFile");
  //  testDAQSocket->close();
  //  testDAQSocket->disconnectFromHost();

    if(dbg())
        msg()("Writing output files and closing...",
                "DataHandler::writeAndCloseDataFile");
    if(!m_vmm2 || !m_treesSetup) {
        msg()("Event data tree is null!",
                    "DataHandler::writeAndCloseDataFile", true);
        exit(1);
    }
    if(!m_artTree || !m_treesSetup) {
        msg()("ART data tree is null!",
                    "DataHandler::writeAndCloseDataFile", true);
        exit(1);
    }
    if(!m_daqRootFile || !m_rootFileOK) {
        msg()("Output ROOT file is not setup!",
                "DataHandler::writeAndCloseDataFile",true);
        exit(1);
    }
    if(!m_fileOK) {
        msg()("Dump file has not been setup!",
                "DataHandler::writeAndCloseDataFile", true);
        exit(1);
    }
    stringstream sx;
//    sx << "Save dump: " << m_daqFile.fileName().toStdString();
//    msg()(sx,"DataHandler::writeAndCloseDataFile()"); sx.str("");

    //close the dump file
//    m_daqFile.close();

    // ensure that we are writing to file OK
    if(writeNtuple()){
        m_daqRootFile->cd();
        if(!(m_vmm2->Write("", TObject::kOverwrite))) {
            msg()("ERROR writing event data to file!",
                        "DataHandler::writeAndCloseDataFile");
        }
        if(!(m_artTree->Write("", TObject::kOverwrite))) {
            msg()("ERROR writing ART data to file!",
                        "DataHandler::writeAndCloseDataFile");
        }
        if(!(m_daqRootFile->Write())) {
            msg()("ERROR Unable to successfully write output DAQ ROOT file!",
                        "DataHandler::writeAndCloseDataFile");
        }
        sx << "Save root: " << m_daqRootFile->GetName();
        msg()(sx,"DataHandler::writeAndCloseDataFile"); sx.str("");
        m_daqRootFile->Close();
    }//writeNtuple
}
// ------------------------------------------------------------------------ //
void DataHandler::clearData()
{
    m_gain          = -999;
    m_runNumber     = -999;
    m_tacSlope      = -999;
    m_peakTime      = -999;
    m_dacCounts     = -999;
    m_pulserCounts  = -999;
    m_tpSkew        = -999;
    m_angle         = -999;

    // clear the event data
    m_eventNumberFAFA = 0;
    m_triggerTimeStamp.clear();
    m_triggerCounter.clear();
    m_chipId.clear();
    m_eventSize.clear();
    m_tdo.clear();
    m_pdo.clear();
    m_flag.clear();
    m_threshold.clear();
    m_bcid.clear();
    m_channelId.clear();
    m_grayDecoded.clear();

    #warning TODO clear calib data?

//    m_pulserCounters_calib = -999;
//    m_gain_calib           = -999;
//    m_peakTime_calib       = -999;
//    m_daqCounts_calib      = -999;
    m_neighbor_calib.clear();

    // ART
    m_art.clear();
    m_artFlag.clear();
}
// ------------------------------------------------------------------------ //
void DataHandler::EndRun()
{
    msg()("Closing output files...", "DataHandler::EndRun");
    msg()("Output data file saved to: " + m_daqFile.fileName().toStdString(),
                "DataHandler::EndRun");
    m_daqFile.close();
}

// ------------------------------------------------------------------------ //
void DataHandler::readEvent()
{
    stringstream sx;

    bool ok_to_read = true;
 
    if(!m_fileOK) ok_to_read = false;
    if(m_write && !m_rootFileOK) ok_to_read = false;
    if(!ok_to_read) return;

    //if(dbg()) msg()("Receiving event packet...", "DataHandler::readEvent");

    QHostAddress vmmip;
    QByteArray datagram;
    //datagram.clear();

/*
    // dummy testing
    while(daqSocket().hasPendingDatagrams()){
        datagram.resize(daqSocket().pendingDatagramSize());
        daqSocket().readDatagram(datagram.data(), datagram.size(), &vmmip);
        msg()(" *** DAQ socket receiving data *** ", "DataHandler::readEvent");
        msg()("  message from IP : " + vmmip.toString().toStdString()",
                    "DataHandler::readEvent");
        msg()("  message content : " + datagram.toStdString()",
                    "DataHandler::ReadEvent");
        msg()(" ********* end of packet ********* ",
                    "DataHandler::readEvent");
    } // while
*/

 //   QMap<QString, QList<QByteArray> > datamap; // [IP : packets received]
 //   datamap.clear();
 //   QTextStream out(&m_daqFile);

    //thread
  //  while(daqSocket().hasPendingDatagrams()){
  //      datagram.resize(daqSocket().socket().pendingDatagramSize());
  //      daqSocket().socket().readDatagram(datagram.data(), datagram.size(), &vmmip);

    if(monitoring())
        m_sharedDataStrips.clear();
    while(testDAQSocket->hasPendingDatagrams()) {
        //shared

        //shared
        datagram.resize(testDAQSocket->pendingDatagramSize());
        testDAQSocket->readDatagram(datagram.data(), datagram.size(), &vmmip);


        //qDebug() << datagram.toHex();
        // cout << daqSocket().socket().state() << endl;

     //   if(dbg()){
     //       sx.str("");
     //       sx << "DAQ datagram read: " << datagram.toHex().toStdString()
     //          << " from VMM with IP: " << vmmip.toString().toStdString();
     //       msg()(sx, "DataHandler::readEvent");
     //   }

//        QString ip = vmmip.toString();
//        QList<QByteArray> arr = datamap.value(ip); // datamap[ip] = arr 
//        arr.append(datagram);
//        datamap.insert(ip, arr); // fill the value at datamap[ip] = arr

        // here is where we write the ntuple
        //if(writeNtuple())
        // still decode even if not writing out
            decodeAndWriteData(datagram);

    } // while loop

//    QMap<QString, QList<QByteArray> >::iterator it;
//    for(it = datamap.begin(); it!=datamap.end(); it++) {
//      //  if(dbg()) {
//      //      sx.str("");
//      //      sx << "Reading data from address: " << it.key().toStdString();
//      //      msg()(sx, "DataHandler::readEvent");
//      //  }
//        QList<QByteArray> arr = datamap.value(it.key());
//        QList<QByteArray>::iterator bit;
//        for(bit = arr.begin(); bit != arr.end(); bit++) {
//            #warning should put this in as HEX
//            out << (*bit).toHex() << "\n";
//        } // bit
//    } // it

    return;
}
// ------------------------------------------------------------------------ //
void DataHandler::decodeAndWriteData(const QByteArray& datagram)
{
    bool ok;
    stringstream sx;

    bool verbose = false;

    QString frameCounterStr = datagram.mid(0,4).toHex(); //Frame Counter

    ///////////////////////////////////////
    //event data incoming from chip
    ///////////////////////////////////////
    if(frameCounterStr != "fafafafa") {
        QString headerID = datagram.mid(4,3).toHex();

        ///////////////////////////////////////////////
        // readout the VMM2 event data [begin]
        ///////////////////////////////////////////////
        if(headerID == "564d32") {

            QString fullEventDataStr, headerStr, chipNumberStr, trigCountStr, trigTimeStampStr;
            chipNumberStr    = datagram.mid(7,1).toHex();
            trigCountStr     = datagram.mid(8,2).toHex();
            trigTimeStampStr = datagram.mid(10,2).toHex();

            if(dbg() && verbose) {
                sx.str("");
                headerStr        = datagram.mid(4,4).toHex();
                fullEventDataStr = datagram.mid(12, datagram.size()).toHex();
                sx << "*****************************************************\n"
                   << " Data from chip # : " << chipNumberStr.toInt(&ok,16) << "\n" //toStdString() << "\n"
                   << "  > Header        : " << headerStr.toStdString() << "\n"
                   << "  > Data          : " << fullEventDataStr.toStdString() << "\n"
                   << "*****************************************************"; 
                cout << sx.str() << endl;
                //msg()(sx,"DataHandler::decodeAndWriteData");
            } //dbg

            if(datagram.size()==12 && dbg()) {
                sx.str("");
                sx << "Empty event from chip #: " << chipNumberStr.toInt(&ok,16);
                cout << sx.str() << endl;
                msg()(sx,"DataHandler::decodeAndWriteData"); sx.str("");
            }

            // data containers for this chip
            _pdo.clear();
            _tdo.clear();
            _bcid.clear();
            _gray.clear();
            _channelNo.clear();
            _flag.clear();
            _thresh.clear();
            _neighbor.clear();

            // -------------- begin loop over chip's channels ---------------- // 
            for(int i = 12; i < datagram.size(); ) {
                quint32 bytes1 = reverse32(datagram.mid(i, 4).toHex());
                quint32 bytes2 = reverse32(datagram.mid(i+4, 4).toHex());

                // --- flag --- //
                uint flag = (bytes2 & 0x1);
                _flag.push_back(flag);

                // --- threshold --- //
                uint threshold = (bytes2 & 0x2) >> 1;
                _thresh.push_back(threshold);

                // --- channel number --- //
                int channel_no = (bytes2 & 0xfc) >> 2; // 0xfc = 0000 0000 1111 1100

                if(calibRun()) {
                    if(m_channel_for_calib < 0) {
                        sx.str("");
                        sx << "Channel for calibration has not been set!\n"
                           << "Make sure that it is set correctly in"
                           << " DataHandler::setCalibrationChannel";
                        msg()(sx, "DataHandler::decodeAndWriteData"); sx.str(""); 
                        _neighbor.push_back(0);
                    }
                    else {
                        _neighbor.push_back(!(m_channel_for_calib == channel_no));
                    }
                }
                int unmapped_channel = channel_no;
                if(useChannelMap()) {
                    int chan_test = channelToStrip(chipNumberStr.toInt(&ok,16),
                                channel_no);
                    if(chan_test>0) channel_no = chan_test;
                    else {
                        msg()("Not using channel map");
                    }
                }
                _channelNo.push_back(channel_no);

                // use QString methods instead of using bytes1 (for now)
                QString bytes1_str = "00000000000000000000000000000000"; // 32bit 
                QString tmpStr     = "00000000000000000000000000000000"; // 32bit 
                quint32 bytes1_ex  = datagram.mid(i, 4).toHex().toUInt(&ok,16);
                tmpStr = tmpStr.number(bytes1_ex,2);
                for(int j = 0; j < tmpStr.size(); j++) {
                    QString tmp = tmpStr.at(tmpStr.size()-1-j);
                    bytes1_str.replace(j,1,tmp); // bytes1_str is now QString of bytes1
                } // j

                // --- amplitude / pdo --- //
                QString q_1 = bytes1_str.left(8);
                QString q_2 = bytes1_str.mid(14,2);
                QString q_final;
                q_final.append(q_2);
                q_final.append(q_1);
                uint outCharge_ = 0;
                if(q_final.right(4)=="0000" && ignore16()) {
                    outCharge_ = 1025;
                } else {
                    outCharge_ = q_final.toUInt(&ok,2);
                }
                _pdo.push_back(outCharge_);

                // --- TAC / tdo --- //
                QString tac_1 = bytes1_str.mid(8,6);
                QString tac_2 = bytes1_str.mid(22,2);
                QString tac_final;
                tac_final.append(tac_2);
                tac_final.append(tac_1);
                uint outTac_ = tac_final.toUInt(&ok,2);
                _tdo.push_back(outTac_);

                // --- bcid --- //
                QString bcid_1 = bytes1_str.mid(16,6);
                QString bcid_2 = bytes1_str.mid(26,6);
                QString bcid_final;
                bcid_final.append(bcid_2);
                bcid_final.append(bcid_1);
                uint outBCID_ = bcid_final.toUInt(&ok,2);
                _bcid.push_back(outBCID_);

                // --- gray --- //
                uint gray = DataHandler::grayToBinary(outBCID_);
                _gray.push_back(gray);

                if(dbg() && verbose) {
                //if(true) {
                    sx.str("");
                    sx << "channel          : " << channel_no << "\n"
                       << "flag             : " << flag << "\n"
                       << "threshold        : " << threshold << "\n"
                       << "charge           : " << outCharge_ << "\n"
                       << "q_1              : " << q_1.toStdString() << "\n"
                       << "q_2              : " << q_2.toStdString() << "\n"
                       << "q_final          : " << q_final.toStdString() << "\n"
                       << "tac              : " << outTac_ << "\n"
                       << "bcid             : " << outBCID_;
                    cout << sx.str() << endl;
                    //msg()(sx,"DataHandler::decodeAndWriteData");
                    //msg()(" "," ");
                } // dbg

                //shared
                if(monitoring()) {
                //if(true){
                    //string x = "10 TL2 0 0 X 45 1 20 60 40 40 40 40";
                    string x = "";
                    x = m_ce->getEvent(chipNumberStr.toInt(&ok,16), unmapped_channel, outBCID_, outCharge_, outTac_, outCharge_, outTac_); 
                    //cout << "-------------------------------" << endl;
                    //cout << "DataHandler::decodeAndWriteData RETURN FROM GET EVENT: " << x << endl; 
                    //cout << "-------------------------------" << endl;
                    m_sharedDataStrips.push_back(x);
                }

                // move to next channel (8 bytes forward)
                i += 8;
            } // i

            if(writeNtuple()) {
                m_triggerTimeStamp.push_back(trigTimeStampStr.toInt(&ok,16));
                m_triggerCounter.push_back(trigCountStr.toInt(&ok,16));
                m_chipId.push_back(chipNumberStr.toInt(&ok,16));
                m_eventSize.push_back(datagram.size()-12);

                m_tdo.push_back(_tdo);
                m_pdo.push_back(_pdo);
                m_flag.push_back(_flag);
                m_threshold.push_back(_thresh);
                m_bcid.push_back(_bcid);
                m_channelId.push_back(_channelNo);
                m_grayDecoded.push_back(_gray);

                if(calibRun())
                    m_neighbor_calib.push_back(_neighbor);

            } //writeNutple

        } // VMM2 event data
        ///////////////////////////////////////////////
        // readout the VMM2 event data [end]
        ///////////////////////////////////////////////

        ///////////////////////////////////////////////
        // readout the ART data [begin]
        ///////////////////////////////////////////////
        else if(headerID=="564132") {

            bool test_art = true;
         //   qDebug() << "art: " << datagram.toHex();

            // clear the ART data containers
            m_art.clear();
            m_artFlag.clear();

            QString art_channel = datagram.mid(7,1).toHex();
           // qDebug() << " > art size : " << datagram.size();
            for(int i = 12; i < (int)datagram.size(); ) {
                quint32 art_timestamp = datagram.mid(i,2).toHex().toUInt(&ok,16);
                quint32 art_data = datagram.mid(i+2,2).toHex().toUInt(&ok,16);

                uint art1 = (art_data & 0x3f00) >> 8; // 0x3f000 = 0011 1111 0000 0000
                uint art2 = (art_data & 0x3f); // 0x3f = 0000 0000 0011 1111

                uint art1_flag = (art_data & 0x8000) >> 15; // 0x8000 = 1000 0000 0000 0000
                uint art2_flag = (art_data & 0x80) >> 7; // 0x80 = 0000 0000 1000 0000

                // push back to store later
                if(writeNtuple()) {
                    m_art.push_back(art1);
                    m_artFlag.push_back(art1_flag);
                    m_art.push_back(art2);
                    m_artFlag.push_back(art2_flag);

                    if(m_art.size()>2 || m_artFlag.size()>2) {
                        cout << "storing more than 2 ART data" << endl;
                    }
                }

                if(dbg() && test_art) {
               // if(test_art) {
                    QString art_timestamp_test = datagram.mid(i,2).toHex();
                    QString art1_test = datagram.mid(i+2,1);
                    QString art2_test = datagram.mid(i+3,1);
                    cout << "ART timestamp: " << art_timestamp_test.toStdString()
                         << ", art1_test: " << art1_test.toStdString()
                         << ", art2_test: " << art2_test.toStdString()
                         << ", art1: " << art1 << " (flag: " << art1_flag << ")"
                         << ", art2: " << art2 << " (flag: " << art2_flag << ")" << endl;
                } // test art

                i += 4;

            } // i
        } // ART data
        ///////////////////////////////////////////////
        // readout the ART data [end]
        ///////////////////////////////////////////////
    } // != fafafafa

    ///////////////////////////////////////
    // reading out chip complete
    ///////////////////////////////////////
    else if(frameCounterStr == "fafafafa") {

        if((int)getDAQCount()%100==0) {
            emit checkDAQCount();
        }
        m_eventNumberFAFA = getDAQCount() - 1;

        if(calibRun()) {
            #warning CONFIRM THAT CALIB INFORMATION IS FILLED // in "updateCalibrationState"
            m_calibRun = true;

            //pulser amplitude
     //       // pulser amplitude
     //       m_pulserCounts_calib = ui->sdp_2->value();

     //       // gain
     //       QString tmpGainString = ui->sg->currentText().left(3);
     //       m_gain_calib = tmpGainString.toDouble(&ok);

     //       // integration time (peak time)
     //       QString tmpIntTimeString = ui->st->currentText().left(3);
     //       m_peakTime_calib = tmpIntTimeString.toInt(&ok,10);

     //       // threshold
     //       m_dacCounts_calib = ui->sdt->value(); 

     //       // bool bool bool
     //       m_calibRun = true;

        }

        updateDAQCount();
        if(writeNtuple())
            fillEventData();

        // clear the data containers for this chip before the next one
        // is read int

        if(monitoring()) {
        //if(true) {
            m_sh->publishEvent(m_sharedDataStrips);
            m_sharedDataStrips.clear();
        }

        clearData();

    } // == fafafafa

}
// ------------------------------------------------------------------------ //
void DataHandler::fillEventData()
{
    stringstream sx;
    if(!writeNtuple()) {
        if(dbg()) {
            sx.str("");
            sx << "This method should only be called if writing an output ROOT ntuple. Skipping.";
            msg()(sx,"DataHandler::fillEventData");
        }
        return;
    }

    if(!m_treesSetup || !m_vmm2 || !m_artTree || !m_rootFileOK) {
        msg()("Event data tree unable to be filled!",
                "DataHandler::fillEventData",true);
        if(dbg()) {
            sx.str("");
            sx << "---------------------------------------------\n"
               << "  trees setup OK ? : " << (m_treesSetup ? "yes" : "no") << "\n"
               << "  vmm2 tree OK   ? : " << (m_vmm2 ? "yes" : "no") << "\n"
               << "  ART tree OK    ? : " << (m_artTree ? "yes":"no") << "\n"
               << "  root file OK   ? : " << (m_rootFileOK ? "yes" : "no") << "\n"
               << "---------------------------------------------";
            msg()(sx,"DataHandler::fillEventData");
        } // dbg
        exit(1);
    }

    m_daqRootFile->cd();
    m_vmm2->Fill();
    m_artTree->Fill();
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
    for(int i = 0; i < (int)array.size(); i++)
    {
        value <<= 1;
        value += (int)array.at(i);
    }
    QString str;
    str.setNum(value, 10);
    
    return str;
}
// ------------------------------------------------------------------------ //
quint32 DataHandler::reverse32(QString hex)
{
    bool ok;
    QBitArray received(32,false);
    QString bin, tmp;
    bin = tmp.number(hex.toUInt(&ok,16),2); // convert input to binary
    if(bin.size()>32) {
        cout << "DataHandler::reverse32    Input datagram is larger than 32 bits!" << endl;
        cout << "DataHandler::reverse32    >>> Exiting." << endl;
        exit(1);
    }
    // turn input array into QBitArray
    for(int i = 0; i < bin.size(); i++) {
        QString bit = bin.at(i);
        received.setBit(32-bin.size() + i, bit.toUInt(&ok,10)); // pad left with 0's
    } // i

    // now reverse
    QBitArray reversed(32, false);
    for(int j = 0; j < 32; j++) {
        reversed.setBit(31-j, received[j]);
    } // j

    // turn into QByteArray and return
    QByteArray reversed_byte = DataHandler::bitsToBytes(reversed);
    return reversed_byte.toHex().toUInt(&ok,16);
}
// ------------------------------------------------------------------------ //
uint DataHandler::grayToBinary(uint num)
{
    uint mask;
    for( mask = num >> 1; mask != 0; mask = mask >> 1)
    {
        num = num ^ mask;
    }
    return num;
}
// ------------------------------------------------------------------------ //
void DataHandler::daqThreadWhere(QString name)
{
    qDebug() << "DATAHANDLE THREAD WHERE   : " << QThread::currentThreadId();
    qDebug() << "DATAHANDLE THREAD WHERE fname : " << name;
}
