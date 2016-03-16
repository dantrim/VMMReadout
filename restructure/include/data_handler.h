
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
#include "config_handler.h"
#include "message_handler.h"
class VMMSocket;

// ROOT
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"


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

        void setDebug(bool dbg) { m_dbg = dbg; }
        bool dbg() { return m_dbg; }
        void LoadMessageHandler(MessageHandler& msg);
        MessageHandler& msg() { return *m_msg; }

        void setCalibrationRun(bool calib) { m_calibRun = calib; }
        bool calibRun() { return m_calibRun; }
        void setIgnore16(bool doit) { m_ignore16 = doit; }
        bool ignore16() { return m_ignore16; }
        void setWriteNtuple(bool doit) { m_write = doit; }
        bool writeNtuple() { return m_write; }

        void LoadDAQSocket(VMMSocket& socket); 

        void setupOutputFiles(TriggerDAQ& daq, QString outdir = "",
                                            QString filename = "");

        void dataFileHeader(CommInfo& comm, GlobalSetting& global,
                                TriggerDAQ& daq);
        void getRunProperties(const GlobalSetting& global, int runNumber,
                                            int angle);

        QString getRootFileName(const QString& outdir);


        // output ntuples
        void setupOutputTrees();


        /////////////////////////////////////////
        // general methods
        // not necessarily class specific
        /////////////////////////////////////////
        static quint32 ValueToReplaceHEX32(QString datagramHex, int bitToChange,
                                                bool newBitValue);
        static QByteArray bitsToBytes(QBitArray bits);
        static QBitArray bytesToBits(QByteArray bytes); 
        static QString QBitArrayToString(const QBitArray &arr);
        static quint32 reverse32(QString hex);
        static uint grayToBinary(uint num);

        /////////////////////////////////////////
        // data handling
        /////////////////////////////////////////
        VMMSocket& daqSocket() { return *m_daqSocket; }
        void decodeAndWriteData(const QByteArray& datagram);
        void resetDAQCount() { n_daqCnt = 0; }
        int getDAQCount() { return n_daqCnt; } 
        void updateDAQCount() { n_daqCnt++; }
        void fillEventData();
        



    private :
        bool m_dbg;
        bool m_calibRun;
        bool m_write;
        int n_daqCnt;
        bool m_ignore16;

        VMMSocket *m_daqSocket;
        MessageHandler *m_msg;

        QFile m_daqFile;
        bool m_fileOK;
        QString m_outDir;

        ///////////////////////////////////////////
        // data decoding
        ///////////////////////////////////////////
        void clearData();

        // run data
        double m_gain;
        int m_runNumber;
        int m_tacSlope;
        int m_peakTime;
        int m_dacCounts;
        int m_pulserCounts;
        int m_angle;

        // event data
        int m_eventNumberFAFA;
        int m_daqCnt;
        std::vector<int> m_triggerTimeStamp;
        std::vector<int> m_triggerCounter;
        std::vector<int> m_chipId;
        std::vector<int> m_eventSize;
        std::vector< std::vector<int> > m_tdo;
        std::vector< std::vector<int> > m_pdo;
        std::vector< std::vector<int> > m_flag;
        std::vector< std::vector<int> > m_threshold;
        std::vector< std::vector<int> > m_bcid;
        std::vector< std::vector<int> > m_channelId;
        std::vector< std::vector<int> > m_grayDecoded;

        // calibration data
        int m_pulserCounts_calib;
        double m_gain_calib;
        int m_peakTime_calib;
        int m_dacCounts_calib;
        std::vector< std::vector<int> > m_neighbor_calib;
        

        ///////////////////////////////////////////
        // output ntuples
        ///////////////////////////////////////////

        TFile* m_daqRootFile;
        bool m_rootFileOK;
        bool m_treesSetup;
        bool m_runPropertiesFilled;

        // output trees
        TTree*  m_vmm2;
        TBranch *br_eventNumberFAFA;
        TBranch *br_triggerTimeStamp;
        TBranch *br_triggerCounter;
        TBranch *br_chipId;
        TBranch *br_evSize;
        TBranch *br_tdo;
        TBranch *br_pdo;
        TBranch *br_flag;
        TBranch *br_thresh;
        TBranch *br_bcid;
        TBranch *br_grayDecoded;
        TBranch *br_channelId;
        TBranch *br_pulserCalib;
        TBranch *br_gainCalib;
        TBranch *br_peakTimeCalib;
        TBranch *br_threshCalib;
        TBranch *br_calibRun;
        TBranch *br_neighborCalib;

        TTree*  m_runProperties;
        TBranch *br_runNumber;
        TBranch *br_gain;
        TBranch *br_tacSlope;
        TBranch *br_peakTime;
        TBranch *br_dacCounts;
        TBranch *br_pulserCounts;
        TBranch *br_angle;
        TBranch *br_calibrationRun;

        

    signals :
        void checkDAQCount();

    public slots :
        void readEvent();
        void EndRun();
        void writeAndCloseDataFile();

    private slots :


}; // class DataHandler

#endif
