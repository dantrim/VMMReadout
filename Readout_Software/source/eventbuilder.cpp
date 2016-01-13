//event_builder
#include "eventbuilder.h"

//qt
#include <QDebug>
#include <QtNetwork>

//stl/std
#include <vector>
using namespace std;

EventBuilder::EventBuilder(QObject *parent)
    : QObject(parent)

{
    m_dbg = false;
    m_useChanMap = false;
    m_ignore16 = false;
    m_writeData = false;
    m_socketDAQ = NULL;

}
void EventBuilder::initialize(QUdpSocket *socket, Configuration *config)
{
    if(socket->state() == QAbstractSocket::BoundState) {
        m_socketDAQ = socket;
    }
    else {
        qDebug() << "[EventBuilder::setSocket]    Provided DAQ socket is not bound!";
        abort();
    }
    m_config = config;

    EventBuilder::setupOutputTrees();
}
void EventBuilder::setupOutputTrees()
{

    m_fileDAQRoot = new TFile("test_DAQ.root", "UPDATE");

    vmm2 = new TTree("vmm2", "vmm2");
    // clear the branch variables
    m_tdo.clear();
    m_pdo.clear();
    m_chanId.clear();
    m_bcid.clear();
    m_flag.clear();
    m_thresh.clear();
    m_grayDecoded.clear();
    //m_neighborForCalibration.clear();
    m_chipId.clear();
    m_evSize.clear();
    m_triggerTimeStamp.clear();
    m_triggerCounter.clear();

    br_eventNumberFAFA  = vmm2->Branch("eventFAFA", &m_eventNumberFAFA);
    n_daqCnt = 0;
    br_triggerTimeStamp = vmm2->Branch("triggerTimeStamp", "std::vector<int>", &m_triggerTimeStamp);
    br_triggerCounter   = vmm2->Branch("triggerCounter", "std::vector<int>", &m_triggerCounter);
    br_chipId           = vmm2->Branch("chip", "std::vector<int>", &m_chipId);
    br_evSize           = vmm2->Branch("eventSize", "std::vector<int>", &m_evSize);
    br_tdo              = vmm2->Branch("tdo", "std::vector<vector<int> >", &m_tdo);
    br_pdo              = vmm2->Branch("pdo", "std::vector<vector<int> >", &m_pdo);
    br_flag             = vmm2->Branch("flag", "std::vector<vector<int> >", &m_flag);
    br_thresh           = vmm2->Branch("threshold", "std::vector<vector<int> >", &m_thresh);
    br_bcid             = vmm2->Branch("bcid", "std::vector<vector<int> >", &m_bcid);
    br_grayDecoded      = vmm2->Branch("grayDecoded", "std::vector<vector<int> >", &m_grayDecoded);

    run_properties = new TTree("run_properties", "run_properties");
    double gain;
    int runNumber;
    int tacSlope;
    int peakTime;
    int dacCounts;
    int pulserCounts;
    int angle;

    br_runNumber    = run_properties->Branch("runNumber", &runNumber);
    br_gain         = run_properties->Branch("gain", &gain);
    br_tacSlope     = run_properties->Branch("tacSlope", &tacSlope);
    br_peakTime     = run_properties->Branch("peakTime", &peakTime);
    br_dacCounts    = run_properties->Branch("dacCounts", &dacCounts);
    br_pulserCounts = run_properties->Branch("pulserCounts", &pulserCounts);
    br_angle        = run_properties->Branch("angle", &angle);

    gain         = m_config->getGainString().toDouble();
    tacSlope     = m_config->getTACSlope();
    peakTime     = m_config->getPeakInt();
    dacCounts    = m_config->getThresholdDAC();
    pulserCounts = m_config->getTestPulseDAC();

    // fill dac conf
    br_gain->Fill();
    br_tacSlope->Fill();
    br_peakTime->Fill();
    br_dacCounts->Fill();
    br_pulserCounts->Fill();

    run_properties->Fill();
    run_properties->Write("", TObject::kOverwrite);
    delete run_properties;

}

void EventBuilder::dataPending()
{
    qDebug() << "[EventBuilder::dataPending]    dataPending()";
    if( !(m_socketDAQ->state() == QAbstractSocket::BoundState) ){
        qDebug() << "[EventBuilder::dataPending]    DAQ socket is not bound!";
        abort();
    }

    bool ok;
    while(m_socketDAQ->hasPendingDatagrams()) {
        m_bufferEVT.resize(m_socketDAQ->pendingDatagramSize());
        m_socketDAQ->readDatagram(m_bufferEVT.data(), m_bufferEVT.size());

        if(m_bufferEVT.size() == 12) {
            // bytes >12 are the event variables
            qDebug() << "[EventBuilder::dataPending]    Empty event";
        }

        QString dataFrameString = m_bufferEVT.mid(0,4).toHex();
        if(dataFrameString != "fafafafa") {
            // full incoming data buffer
            QString incomingData = m_bufferEVT.mid(12,m_bufferEVT.size()).toHex();
            // data header
            QString dataHeader = m_bufferEVT.mid(4,4).toHex();
            // chip #
            QString chip = m_bufferEVT.mid(7,1).toHex();
            // trigger counter
            QString trigCntString = m_bufferEVT.mid(8,2).toHex();
            // trigger time stamp
            QString trigTimeStampString = m_bufferEVT.mid(10,2).toHex();

            if(m_dbg) {
                qDebug() << "[EventBuilder::dataPending]    Header : " << dataHeader;
                qDebug() << "[EventBuilder::dataPending]    Chip   : " << chip;
                qDebug() << "[EventBuilder::dataPending]    Data   : " << incomingData;
            }

            // event vars (temp containers for this event to attach to the output tree)
            vector<int> pdo;            pdo.clear();
            vector<int> tdo;            tdo.clear();
            vector<int> bcid;           bcid.clear();
            vector<int> grayDecoded;    grayDecoded.clear();
            vector<int> channel;        channel.clear();
            vector<int> flag;           flag.clear();
            vector<int> thresh;         thresh.clear();
            vector<int> neighbor;       neighbor.clear();

            // now loop over the event data (incoming bytes >12) each packet is 8 bytes long
            for(int i = 12; i < m_bufferEVT.size(); ) {
                quint32 bytesInt1tmp = reverse32(m_bufferEVT.mid(i,4).toHex());
                QString bytesInt1String = m_bufferEVT.mid(i,4).toHex();
                quint32 bytesInt1 = bytesInt1String.toUInt(&ok,16);
                quint32 bytesInt2 = reverse32(m_bufferEVT.mid(i+4,4).toHex());

                if(m_dbg) {
                    QString b1_BeforeRev, b1_AftRev;
                    qDebug() << "[EventBuilder::dataPending]    bytes 1 before rev : " << b1_BeforeRev.number(bytesInt1);
                    qDebug() << "[EventBuilder::dataPending]    bytes 1 after rev  : " << b1_AftRev.number(bytesInt1tmp);
                } // dbg

                // dantrim -- do we even use the reversed ?

                // flag
                uint flag_ = (bytesInt2 & 0x1);
                flag.push_back(flag_);

                // threshold
                uint threshold_ = (bytesInt2 & 0x2) >> 1;
                thresh.push_back(threshold_);

                // channel input
                uint channelin_ = (bytesInt2 & 0xfc) >> 2; // 0xfc = 0000 0000 1111 1100
                uint channel_remapped = 0;
                // > use the vmmchannel # or the strip number (the pin numbers do not equal strip numbers)
                if(m_useChanMap) {
                    channel_remapped = EventBuilder::MappingMini2(channelin_, chip.toInt(&ok,16));
                } else {
                    channel_remapped = channelin_;
                }
                channel.push_back(channel_remapped);

                QString datastring    = "00000000000000000000000000000000";
                QString datastringtmp = "00000000000000000000000000000000";
                datastringtmp = datastringtmp.number(bytesInt1,2);
                for(int j = 0; j < datastringtmp.size(); j++) {
                    QString tmp = datastringtmp.at(datastringtmp.size()-1-j);
                    datastring.replace(j,1,tmp);
                }
                // i.e. datastring0 is reversed bytesInt1 (in string and binary)
                if(m_dbg) {
                    qDebug() << "[EventBuilder::dataPending]    bytes int 1 string       : " << datastringtmp;
                    qDebug() << "[EventBuilder::DataPending]    bytes int 1 string (rev) : " << datastring;
                }

                // grab the PDO
                QString q_s1 = datastring.left(8);
                QString q_s2 = datastring.mid(14,2); 
                QString charge_final;
                charge_final.append(q_s2);
                charge_final.append(q_s1);
                uint charge_ = 0;
                if(charge_final.right(4)=="0000" && m_ignore16) charge_ = 1025;
                else {
                    charge_ = charge_final.toUInt(&ok,2);
                }
                pdo.push_back(charge_);

                // grab the TAC info
                QString TAC_s1 = datastring.mid(8,6);
                QString TAC_s2 = datastring.mid(22,2);
                QString TAC_final;
                TAC_final.append(TAC_s2);
                TAC_final.append(TAC_s1);
                uint tac_ = TAC_final.toUInt(&ok,2);
                tdo.push_back(tac_);

                // bcid info
                QString bcid_s1 = datastring.mid(16,6);
                QString bcid_s2 = datastring.mid(26,6);
                QString bcid_final;
                bcid_final.append(bcid_s2);
                bcid_final.append(bcid_s1);
                uint bcid_ = bcid_final.toUInt(&ok,2);
                bcid.push_back(bcid_);

                // gray decoded
                uint gray_bin = EventBuilder::grayToBinary(bcid_);
                grayDecoded.push_back(gray_bin);

                if(m_dbg) {
                    QString yep = "[EventBuilder::dataPending]    ";
                    qDebug() << yep << "Before reverse, 1: " << m_bufferEVT.mid(i,4).toHex()<<",  2: " << m_bufferEVT.mid(i+4,4).toHex();
                    qDebug() << yep << "After, bytesInt 1: " << bytesInt1 << ",  2: " << bytesInt2;
                    qDebug() << yep << "Flag             : " << flag_;
                    qDebug() << yep << "Threshold        : " << threshold_;
                    qDebug() << yep << "Channel          : " << channel_remapped;
                    qDebug() << yep << "Charge           : " << charge_;
                    qDebug() << yep << "Bits             : " << datastring;
                    qDebug() << yep << "CH1              : " << q_s1;
                    qDebug() << yep << "CH2              : " << q_s2;
                    qDebug() << yep << "CH final         : " << charge_final;
                    qDebug() << yep << "TAC              : " << tac_;
                    qDebug() << yep << "BCID             : " << bcid_;
                }

                // move to next set of 8 bytes
                i = i + 8;
            } // i

            // now fill the branches with the event data
            if(m_writeData) {
             //   m_eventNumbefFAFA = // TODO : implement this var
                m_triggerTimeStamp.push_back(trigTimeStampString.toInt(&ok,16));
                m_triggerCounter.push_back(trigCntString.toInt(&ok,16));
                m_chipId.push_back(chip.toInt(&ok,16));
                m_evSize.push_back(m_bufferEVT.size()-12);
                
                m_tdo.push_back(tdo);
                m_pdo.push_back(pdo);
                m_flag.push_back(flag);
                m_thresh.push_back(thresh);
                m_bcid.push_back(bcid);
                m_chanId.push_back(channel);
                m_grayDecoded.push_back(grayDecoded);
            }
                

        } // !=fafafafa
        else if(dataFrameString == "fafafafa") {
            n_daqCnt++;
            m_eventNumberFAFA = n_daqCnt - 1;

            if(m_writeData) {
                if(m_dbg) {
                    qDebug() << "[EventBuilder::dataPending]    Writing event with size: " << m_chanId.size();
                    for(int i = 0; i < (int)m_chanId.size(); i++) {
                        for(int j = 0; j < (int)m_chanId[i].size(); j++) {
                            qDebug() << "              >> i: " << i << ",  j: " << j << ",  channel: " << m_chanId[i][j];
                        } // j
                    } // i
                } // dbg

                // fill the output tree
                vmm2->Fill();
                vmm2->Write("",TObject::kOverwrite); 

                // clear the data now that we have read in the full event
                m_triggerTimeStamp.clear();
                m_triggerCounter.clear();
                m_chipId.clear();
                m_evSize.clear();
                m_tdo.clear();
                m_pdo.clear();
                m_flag.clear();
                m_thresh.clear();
                m_bcid.clear();
                m_chanId.clear();
                m_grayDecoded.clear();
            }

        } // == fafafafa
    } // while datagrams
}

quint32 EventBuilder::reverse32(QString datagram_hex)
{
    bool ok;
    QBitArray commandReceived(32, false); // initialize 32-bit to all 0 
    QString datagram_bin, tmp;
    datagram_bin = tmp.number(datagram_hex.toUInt(&ok,16),2); // convert input hex to binary
    for(int i = 0; i < datagram_bin.size(); i++) {
        QString bit = datagram_bin.at(i);
        commandReceived.setBit(32-datagram_bin.size() + i, bit.toUInt(&ok, 10)); // pad left with 0's
    } // i
    QBitArray commandReversed(32, false);
    for(int i = 0; i < 32; i++) {
        commandReversed.setBit(31-i, commandReceived[i]);
    } // i
    QByteArray commandReversed_bytearray = EventBuilder::bitsToBytes(commandReversed);
    quint32 tmp32 = commandReversed_bytearray.toHex().toUInt(&ok,16);
    return tmp32;
}

QByteArray EventBuilder::bitsToBytes(QBitArray inbits)
{
    QByteArray bytes;
    bytes.resize(inbits.count() / 8); // integer division
    bytes.fill(0);
    for(int b = 0; b < inbits.count(); ++b) {
        bytes[b/8] = ( bytes.at(b/8) | ((inbits[b]?1:0)<<(7-(b%8))));
    }
    return bytes;
}

uint EventBuilder::grayToBinary(uint num)
{
    uint mask;
    for( mask = num >> 1; mask != 0; mask = mask >> 1)
    {
        num = num ^ mask;
    }
    return num;
}

int EventBuilder::MappingMini2(int vmmchannel, int chipnumber)
{
    int strip = 0;
    if( (chipnumber%2 == 0) && chipnumber >= 0 && chipnumber <=14) {
        if(vmmchannel==0)  strip=32;
        if(vmmchannel==1)  strip=33;
        if(vmmchannel==2)  strip=31;
        if(vmmchannel==3)  strip=34;
        if(vmmchannel==4)  strip=30;
        if(vmmchannel==5)  strip=35;
        if(vmmchannel==6)  strip=29;
        if(vmmchannel==7)  strip=36;
        if(vmmchannel==8)  strip=28;
        if(vmmchannel==9)  strip=37;
        if(vmmchannel==10) strip=27;
        if(vmmchannel==11) strip=38;
        if(vmmchannel==12) strip=26;
        if(vmmchannel==13) strip=39;
        if(vmmchannel==14) strip=25;
        if(vmmchannel==15) strip=40;
        if(vmmchannel==16) strip=24;
        if(vmmchannel==17) strip=41;
        if(vmmchannel==18) strip=23;
        if(vmmchannel==19) strip=42;
        if(vmmchannel==20) strip=22;
        if(vmmchannel==21) strip=43;
        if(vmmchannel==22) strip=21;
        if(vmmchannel==23) strip=44;
        if(vmmchannel==24) strip=20;
        if(vmmchannel==25) strip=45;
        if(vmmchannel==26) strip=19;
        if(vmmchannel==27) strip=46;
        if(vmmchannel==28) strip=18;
        if(vmmchannel==29) strip=47;
        if(vmmchannel==30) strip=17;
        if(vmmchannel==31) strip=48;
        if(vmmchannel==32) strip=16;
        if(vmmchannel==33) strip=49;
        if(vmmchannel==34) strip=15;
        if(vmmchannel==35) strip=50;
        if(vmmchannel==36) strip=14;
        if(vmmchannel==37) strip=51;
        if(vmmchannel==38) strip=13;
        if(vmmchannel==39) strip=52;
        if(vmmchannel==40) strip=12;
        if(vmmchannel==41) strip=53;
        if(vmmchannel==42) strip=11;
        if(vmmchannel==43) strip=54;
        if(vmmchannel==44) strip=10;
        if(vmmchannel==45) strip=55;
        if(vmmchannel==46) strip=9;
        if(vmmchannel==47) strip=56;
        if(vmmchannel==48) strip=8;
        if(vmmchannel==49) strip=57;
        if(vmmchannel==50) strip=7;
        if(vmmchannel==51) strip=58;
        if(vmmchannel==52) strip=6;
        if(vmmchannel==53) strip=59;
        if(vmmchannel==54) strip=5;
        if(vmmchannel==55) strip=60;
        if(vmmchannel==56) strip=4;
        if(vmmchannel==57) strip=61;
        if(vmmchannel==58) strip=3;
        if(vmmchannel==59) strip=62;
        if(vmmchannel==60) strip=2;
        if(vmmchannel==61) strip=63;
        if(vmmchannel==62) strip=1;
        if(vmmchannel==63) strip=64;
    }
    else if( !(chipnumber%2==0) && chipnumber >=1 && chipnumber <=15) {
        if(vmmchannel==0)  strip=128;
        if(vmmchannel==1)  strip=65;
        if(vmmchannel==2)  strip=127;
        if(vmmchannel==3)  strip=66;
        if(vmmchannel==4)  strip=126;
        if(vmmchannel==5)  strip=67;
        if(vmmchannel==6)  strip=125;
        if(vmmchannel==7)  strip=68;
        if(vmmchannel==8)  strip=124;
        if(vmmchannel==9)  strip=69;
        if(vmmchannel==10) strip=123;
        if(vmmchannel==11) strip=70;
        if(vmmchannel==12) strip=122;
        if(vmmchannel==13) strip=71;
        if(vmmchannel==14) strip=121;
        if(vmmchannel==15) strip=72;
        if(vmmchannel==16) strip=120;
        if(vmmchannel==17) strip=73;
        if(vmmchannel==18) strip=119;
        if(vmmchannel==19) strip=74;
        if(vmmchannel==20) strip=118;
        if(vmmchannel==21) strip=75;
        if(vmmchannel==22) strip=117;
        if(vmmchannel==23) strip=76;
        if(vmmchannel==24) strip=116;
        if(vmmchannel==25) strip=77;
        if(vmmchannel==26) strip=115;
        if(vmmchannel==27) strip=78;
        if(vmmchannel==28) strip=114;
        if(vmmchannel==29) strip=79;
        if(vmmchannel==30) strip=113;
        if(vmmchannel==31) strip=80;
        if(vmmchannel==32) strip=112;
        if(vmmchannel==33) strip=81;
        if(vmmchannel==34) strip=111;
        if(vmmchannel==35) strip=82;
        if(vmmchannel==36) strip=110;
        if(vmmchannel==37) strip=83;
        if(vmmchannel==38) strip=109;
        if(vmmchannel==39) strip=84;
        if(vmmchannel==40) strip=108;
        if(vmmchannel==41) strip=85;
        if(vmmchannel==42) strip=107;
        if(vmmchannel==43) strip=86;
        if(vmmchannel==44) strip=106;
        if(vmmchannel==45) strip=87;
        if(vmmchannel==46) strip=105;
        if(vmmchannel==47) strip=88;
        if(vmmchannel==48) strip=104;
        if(vmmchannel==49) strip=89;
        if(vmmchannel==50) strip=103;
        if(vmmchannel==51) strip=90;
        if(vmmchannel==52) strip=102;
        if(vmmchannel==53) strip=91;
        if(vmmchannel==54) strip=101;
        if(vmmchannel==55) strip=92;
        if(vmmchannel==56) strip=100;
        if(vmmchannel==57) strip=93;
        if(vmmchannel==58) strip=99;
        if(vmmchannel==59) strip=94;
        if(vmmchannel==60) strip=98;
        if(vmmchannel==61) strip=95;
        if(vmmchannel==62) strip=97;
        if(vmmchannel==63) strip=96;
    }
    return strip;
}

