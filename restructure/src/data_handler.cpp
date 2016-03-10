// vmm
#include "data_handler.h"
#include "vmmsocket.h"

// qt
#include <QByteArray>
#include <QBitArray>
#include <QUdpSocket>
#include <QMap>
#include <QList>
#include <QStringList>
#include <QDir>

using namespace std;


//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  DataHandler
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
DataHandler::DataHandler(QObject *parent) :
    QObject(parent),
    m_dbg(false),
    m_calibRun(false),
    m_writeNtuple(false),
    n_daqCnt(0),
    m_ignore16(false),
    m_daqSocket(0),
    m_fileOK(false),
    m_outDir(""),
    m_daqRootFile(0),
    m_rootFileOK(false),
    m_treesSetup(false),
    m_runPropertiesFilled(false),
    m_vmm2(NULL),
    m_runProperties(NULL)
{
}
// ------------------------------------------------------------------------ //
DataHandler& DataHandler::LoadDAQSocket(VMMSocket& vmmsocket)
{
    if(!m_daqSocket) {
        m_daqSocket = &vmmsocket;
        connect(m_daqSocket, SIGNAL(dataReady()), this, SLOT(readEvent()));
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
void DataHandler::setupOutputFiles(TriggerDAQ& daq, QString outdir,
                                                               QString filename)
{
    //////////////////////////////////////////////////////////////////////
    // there are two methods here:
    //  1. if 'outdir' and 'filename' are provided we will use those
    //  2. if 'outdir' and 'filename' are empty "" then we will use the
    //     values from the config XML
    //////////////////////////////////////////////////////////////////////

    QString fullfilename = "";

    // ------ use xml ------- //
    if(outdir=="" && filename=="") {
        fullfilename = daq.output_path;
        if(!daq.output_path.endsWith("/")) fullfilename.append("/");

        // outputdirectory
        m_outDir = fullfilename;
        QDir outDir(m_outDir);
        if(!outDir.exists()) {
            cout << "DataHandler::setupOutputFiles    ERROR Output directory "
                 << "from config XML does not exist. Exting." << endl;
            exit(1);
        }

        //////////////////////////////////////////////////
        // binary dump file
        //////////////////////////////////////////////////
        fullfilename.append(daq.output_filename);
        m_daqFile.setFileName(fullfilename);

        cout << "Setting output data file to: " << fullfilename.toStdString() << endl;
        #warning implement test mode or increment of filename!!
        if(m_daqFile.exists() /*&&testmode */) {
            cout << "Output file exists. Erasing existing version." << endl;
            m_daqFile.remove();
        }
        bool ok = m_daqFile.open(QIODevice::ReadWrite);
        if(!ok) {
            cout << "DataHandler::setupOutputFiles    Output data file unable "
                 << "to be opened! Exiting." << endl;
            exit(1);
        }
        //m_daqFile.close();
        m_fileOK = true;

        //////////////////////////////////////////////////
        // output root file
        //////////////////////////////////////////////////
        if(writeNtuple()) {
            fullfilename = DataHandler::getRootFileName(m_outDir);
            if(dbg()) cout << "DataHandler::setupOutputFiles    "
                           << "Setting output ROOT file to: "
                           << fullfilename.toStdString() << endl;

            m_daqRootFile = new TFile(fullfilename.toStdString().c_str(), "UPDATE");
            m_rootFileOK = true;
        } // writeNtuple
    } // xml
    // ------ use provided names ------- //
    else if(!(outdir=="" && filename=="")) {
        m_outDir.clear();
        m_outDir = outdir;

        //////////////////////////////////////////////////
        // binary dump file
        //////////////////////////////////////////////////

        QDir outdir(m_outDir);
        if(!outdir.exists()) {
            cout << "DataHandler::setupOutputFiles    ERROR Output directory "
                 << "from config XML does not exist. Exting." << endl;
            exit(1);
        }
        QString spacer = "";
        if(!m_outDir.endsWith("/")) spacer = "/";
        fullfilename = m_outDir + spacer + filename;

        m_daqFile.setFileName(fullfilename);
        cout << "Setting output data file to: " << fullfilename.toStdString() << endl;
        #warning implement test mode or increment of filename!!
        if(m_daqFile.exists() /*&&testmode*/) {
            cout << "Output file exists. Erasing existing version." << endl;
            m_daqFile.remove();
        }
        bool ok = m_daqFile.open(QIODevice::ReadWrite);
        if(!ok) {
            cout << "DataHandler::setupOutputFiles    Output data file unable "
                 << "to be opened! Exiting." << endl;
            exit(1);
        }
        //m_daqFile.close();
        m_fileOK = true;

        //////////////////////////////////////////////////
        // output root file
        //////////////////////////////////////////////////
        if(writeNtuple()) {
            fullfilename = DataHandler::getRootFileName(m_outDir);
            if(dbg()) cout << "DataHandler::setupOutputFiles    "
                           << "Setting output ROOT file to: "
                           << fullfilename.toStdString() << endl;
            m_daqRootFile = new TFile(fullfilename.toStdString().c_str(), "UPDATE");
            m_rootFileOK = true;
        }
    } // user-provided
}
// ------------------------------------------------------------------------ //
QString DataHandler::getRootFileName(const QString& dir)
{
    int number = 0;
    if(m_runPropertiesFilled && !(m_runNumber<0)) {
        number = m_runNumber;
    }
    else {
        cout << "DataHandler::getRootFileName    Run properties not yet filled."
             << " Will set initial ROOT filename to run_0000.root" << endl;
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
        cout << "DataHandler::dataFileHeader    "
             << "Output file has not been set up! Exiting." << endl;
        exit(1);
    }
    if(dbg()) cout << "DataHandler::dataFileHeader" << endl;


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
        br_calibRun         = m_vmm2->Branch("calibrationRun",
                                                &m_calibRun);
        br_neighborCalib    = m_vmm2->Branch("neighbor",
                                "std::vector< vector<int> >", &m_neighbor_calib);
    } //calib

    m_treesSetup = true;

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
}
// ------------------------------------------------------------------------ //
void DataHandler::EndRun()
{
    cout << "DataHandler::EndRun    Closing output files..." << endl;
    cout << " > Output data file saved to: " 
                                << m_daqFile.fileName().toStdString() << endl;
    m_daqFile.close();
}

// ------------------------------------------------------------------------ //
void DataHandler::readEvent()
{
    if(dbg()) cout << "DataHandler::readEvent    Receiving event packet..." << endl;

    QHostAddress vmmip;
    QByteArray datagram;
    datagram.clear();

/*
    // dummy testing
    while(daqSocket().hasPendingDatagrams()){
        datagram.resize(daqSocket().pendingDatagramSize());
        daqSocket().readDatagram(datagram.data(), datagram.size(), &vmmip);
        cout << " *** DAQ socket receiving data *** " << endl;
        cout << "   message from IP : " << vmmip.toString().toStdString() << endl;
        cout << "   message content : " << datagram.toStdString() << endl;
        cout << " ********* end of packet ********* " << endl;
    } // while
*/

    QMap<QString, QList<QByteArray> > datamap; // [IP : packets received]
    datamap.clear();
    QTextStream out(&m_daqFile);

    while(daqSocket().hasPendingDatagrams()){
        datagram.resize(daqSocket().socket().pendingDatagramSize());
        daqSocket().socket().readDatagram(datagram.data(), datagram.size(), &vmmip);
        if(dbg()) cout << "DataHandler::readEvent    DAQ datagram read: "
                       << datagram.toHex().toStdString()
                       << " from VMM with IP: " << vmmip.toString().toStdString()
                       << endl;

        QString ip = vmmip.toString();
        QList<QByteArray> arr = datamap.value(ip); // datamap[ip] = arr 
        arr.append(datagram);
        datamap.insert(ip, arr); // fill the value at datamap[ip] = arr

        #warning TODO IMPLEMENT NTUPLE FILLING AND DATA DECODING
        if(writeNtuple())
            decodeAndWriteData(datagram);

    } // while loop

    QMap<QString, QList<QByteArray> >::iterator it;
    for(it = datamap.begin(); it!=datamap.end(); it++) {
        if(dbg()) cout << "DataHandler::readEvent    "
                       << "Reading data from address: " << it.key().toStdString()
                       << endl;
        QList<QByteArray> arr = datamap.value(it.key());
        QList<QByteArray>::iterator bit;
        for(bit = arr.begin(); bit != arr.end(); bit++) {
            #warning should put this in as HEX
            out << QString::fromStdString((*bit).toStdString()) << "\n";
        } // bit
    } // it

    return;
}
// ------------------------------------------------------------------------ //
void DataHandler::decodeAndWriteData(const QByteArray& datagram)
{
    bool ok;

    QString frameCounterStr = datagram.mid(0,4).toHex(); //Frame Counter

    ///////////////////////////////////////
    //event data incoming from chip
    ///////////////////////////////////////
    if(frameCounterStr != "fafafafa") {

        QString fullEventDataStr, headerStr, chipNumberStr, trigCountStr, trigTimeStampStr;
        fullEventDataStr = datagram.mid(12, datagram.size()).toHex();
        headerStr        = datagram.mid(4,4).toHex();
        chipNumberStr    = datagram.mid(7,1).toHex();
        trigCountStr     = datagram.mid(8,2).toHex();
        trigTimeStampStr = datagram.mid(10,2).toHex();

        if(dbg()) {
            cout << "*****************************************************" << endl;
            cout << "DataHandler::decodeAndWriteData    "
                            << "Data from chip # : " << chipNumberStr.toStdString() << endl;
            cout << "DataHandler::decodeAndWriteData    "
                            << " > Header        : " << headerStr.toStdString() << endl;
            cout << "DataHandler::decodeAndWriteData    "
                            << " > Data          : " << fullEventDataStr.toStdString() << endl;
            cout << "*****************************************************" << endl;
            cout << endl;
        } //dbg

        if(datagram.size()==12)
            cout << "DataHandler::decodeAndWriteData    Empty event from chip #: "
                 << chipNumberStr.toInt(&ok,16) << endl;

        // data containers for this chip
        vector<int>     _pdo;           _pdo.clear();
        vector<int>     _tdo;           _tdo.clear();
        vector<int>     _bcid;          _bcid.clear();
        vector<int>     _gray;          _gray.clear();
        vector<int>     _channelNo;     _channelNo.clear();
        vector<int>     _flag;          _flag.clear();
        vector<int>     _thresh;        _thresh.clear();
        vector<int>     _neighbor;      _neighbor.clear();

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
            uint channel_no = (bytes2 & 0xfc) >> 2; // 0xfc = 0000 0000 1111 1100
            #warning IMPLEMENT CHANNEL MAP
            //if(useChannelMap()) {...}
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

            if(dbg()) {
                string fn = "DataHandler::decodeAndWriteData    ";
                cout << fn << "channel          : " << channel_no << endl;
                cout << fn << "flag             : " << flag << endl;
                cout << fn << "threshold        : " << threshold << endl;
                cout << fn << "charge           : " << outCharge_ << endl;
                cout << fn << "q_1              : " << q_1.toStdString() << endl;
                cout << fn << "q_2              : " << q_2.toStdString() << endl;
                cout << fn << "q_final          : " << q_final.toStdString() << endl;
                cout << fn << "tac              : " << outTac_ << endl;
                cout << fn << "bcid             : " << outBCID_ << endl;
                cout << endl;
            } // dbg

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
        } //writeNutple
    } // != fafafafa

    ///////////////////////////////////////
    // reading out chip complete
    ///////////////////////////////////////
    else if(frameCounterStr == "fafafafa") {

        if(getDAQCount()%100==0) {
            emit checkDAQCount();
        }

        if(calibRun()) {
            #warning IMPLEMENT STORAGE OF CALIB INFO
            // calib info is updated within calls to
            // updateCalibInfo
        }

        updateDAQCount();
        if(writeNtuple())
            fillEventData();

        // clear the data containers for this chip before the next one
        // is read int
        clearData();

    } // == fafafafa

}
// ------------------------------------------------------------------------ //
void DataHandler::fillEventData()
{
    if(!writeNtuple()) {
        if(dbg()) cout << "DataHandler::fillEventData    "
                       << "This method should only be called if writing an output"
                       << " ROOT ntuple. Skipping." << endl;
        return;
    }

    if(!m_treesSetup || !m_vmm2 || !m_rootFileOK) {
        cout << "DataHandler::fillEventData    Event data tree unable to be filled!"
             << " Exiting." << endl;
        if(dbg()) {
            cout << "--------------------------------------------- " << endl;
            cout << "   trees setup OK ? : " << (m_treesSetup ? "yes" : "no") << endl;
            cout << "   vmm2 tree OK   ? : " << (m_vmm2 ? "yes" : "no") << endl;
            cout << "   root file OK   ? : " << (m_rootFileOK ? "yes" : "no") << endl;
            cout << "--------------------------------------------- " << endl;
        } // dbg
        exit(1);
    }

    m_daqRootFile->cd();
    m_vmm2->Fill();
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
// ------------------------------------------------------------------------ //
quint32 DataHandler::reverse32(QString hex)
{
    bool ok;
    QBitArray received(32,false);
    QString bin, tmp;
    bin = tmp.number(hex.toUInt(&ok,16),2); // convert input to binary
    if(bin.size()>32) {
        cout << "DataHandler::reverse32    Input datagram is larger than 32 bits!" << endl;
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

