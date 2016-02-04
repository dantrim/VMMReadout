#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

//qt
#include <QByteArray>
#include <QString>
#include <QMap>
class QFile;
class QBitArray;

//us
class XMLParser;

//root
#include "TROOT.h"
#include "TTree.h"
#include "TBranch.h"
#include "TFile.h"

class DataProcessor
{
    public :
        DataProcessor();
        virtual ~DataProcessor(){};
        void setDebug(bool dbg) { m_dbg = dbg; };
        void setWriteData(bool writeOut) { m_writeData = writeOut; };
        bool writeData() { return m_writeData; }

        void setDataType(QString type) { m_dataType = type; }
        void setMapFileName(QString file) { m_mapFileName = file; }
        QString getMapFileName() { return m_mapFileName; }
        void setOutputDirectory(QString dir) { m_outputDirectory = dir; }
        QString getOutputDirectory() { return m_outputDirectory; }

        void setUseChannelMap(bool useIt) { m_useChannelMap = useIt; };
        bool useChannelMap() { return m_useChannelMap; }
        void setIgnore16(bool ignoreIt) { m_ignore16 = ignoreIt; };
        bool ignore16() { return m_ignore16; }
        void fillChannelMaps();
        //void setDAQConfig(QString file); // -> run_module
        //void getDAQConfig(); // -> run_module
        //void printDAQConfig(); // -> run_module
        void resetDAQCount() { n_daqCnt = 0; };
        void updateDAQCount() { n_daqCnt++; };
        int getDAQCount() { return n_daqCnt; };
        int mappedChannel(int chipNumber, int channelNumber);

        void parseData(QByteArray array);

        //QString getTrigPeriod() { return m_trigPeriod; } // -> run_module
        //QString getRunMode() { return m_runMode; } // -> run_module
        //int getTPDelay() { return m_tpDelay; } // -> run_module
        //int getACQSync() { return m_acqSync; } // -> run_module
        //int getACQWindow() { return m_acqWindow; } // -> run_module
        //int getRunCount() { return m_runCount; } // -> run_module

        // for output if writing data
        QString getOutputFileName(QString outputDirectory);
        void setupOutputFile(QString outdirectory = "", QString filename = "");
        void setupOutputTrees();
        void fillRunProperties(int runNumber, double gain, int tacSlope, int peakTime, int dacCounts, int pulserCounts, int angle);
        void fillEventData();
        void writeAndCloseDataFile();
        //fill run_properties / getRunProperties(x, y, z, t) from GUI perhaps
        //fill event_data(from or in parseData)

    private :
        bool m_writeData;
        bool m_useChannelMap;
        bool m_ignore16;
        int n_daqCnt;

        // utility methods
        quint32 reverse32(QString hexDataGramString);
        QByteArray bitsToBytes(QBitArray bits); 
        uint grayToBinary(uint num);

        bool m_dbg; // debug level

        QString m_dataType;
        QString m_mapFileName;
        QString m_outputDirectory; 

        bool m_runPropertiesFilled;

        // possible maps
        QMap<int, std::vector<int> > m_map_mini2;

        // data buffer
        QByteArray m_buffer;

        // ----- trigger daq ---------- //
        // -> run_module //QString m_trigPeriod;
        // -> run_module //QString m_runMode; 
        // -> run_module //int m_tpDelay;
        // -> run_module //int m_acqSync;
        // -> run_module //int m_acqWindow;
        // -> run_module //int m_runCount;


        // --------- DATA ----------- //
        void clearData();

        // run data //
        double m_gain;
        int m_runNumber;
        int m_tacSlope;
        int m_peakTime;
        int m_dacCounts;
        int m_pulserCounts;
        int m_angle;

        // event data //
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


        // --------- OUTPUT ------------ //
        
        TFile* m_fileDAQ; // output file for ntuples
        bool m_outFileOK;
        bool m_treesSetup;

        // output trees
        TTree* m_vmm2;
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

        TTree* m_runProperties;
        TBranch *br_runNumber;
        TBranch *br_gain;
        TBranch *br_tacSlope;
        TBranch *br_peakTime;
        TBranch *br_dacCounts;
        TBranch *br_pulserCounts;
        TBranch *br_angle;
         
         


}; // class DataProcessor

#endif // DATAPROCESSOR_H
