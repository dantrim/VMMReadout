#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

//qt
class QFile;
class QMap;
class QByteArray;

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
        void setDAQConfig(QString file);
        void getDAQConfig();
        int mappedChannel(int chipNumber, int channelNumber);

        void parseData(QByteArray array);


        // for output if writing data
        void setupOutputFile();
        void setupOutputTrees();
        void fillRunProperties(int gain, int tacSlope, int peakTime, int dacCounts, int pulserCounts);
        void fillEventData();
        //fill run_properties / getRunProperties(x, y, z, t) from GUI perhaps
        //fill event_data(from or in parseData)

    private :
        bool m_writeData;

        // private methods
        void fillChannelMaps();
        // utility methods
        quint32 reverse32(QString hexDataGramString);
        QByteArray bitsToBytes(QBitArray bits); 
        uint grayToBinary(uint num);

        bool m_dbg; // debug level
        QString m_daqXmlFileName;
        QFile *m_daqConfigFile;

        QString m_dataType;
        QString m_mapFileName;
        QString m_outputDirectory; 

        bool m_configOK;

        // possible maps
        QMap<int, std::vector<int> > m_map_mini2;

        // data buffer
        QByteArray m_buffer;


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
