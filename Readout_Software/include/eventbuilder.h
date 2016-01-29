#ifndef EVENT_BUILDER_H
#define EVENT_BUILDER_H

//qt
#include <QObject>
#include <QByteArray>
#include <QString>
#include <QUdpSocket>

//stl/std
#include <vector>

//ROOT
#include "TROOT.h"
#include "TTree.h"
#include "TBranch.h"
#include "TFile.h"

//vmm
#include "configuration_module.h"
#include "xmlparser.h"

class EventBuilder : public QObject
{
    public :
        EventBuilder();
        virtual ~EventBuilder(){};
        void setDebug(bool dbg) { m_dbg = dbg; }
        void setUseChanMap(bool useMap) { m_useChanMap = useMap; }
        void setIgnore16(bool ignore) { m_ignore16 = ignore; }
        void setWriteData(bool doWrite) { m_writeData = doWrite; }
        void initialize(QUdpSocket *socket, Configuration* config);

        void setupOutputTrees();

        // get the strip # from th corresponding vmm (chip#, channel#)
        int MappingMini2(int vmmchannel, int chipnumber);

        unsigned int grayToBinary(unsigned int bcid_in);

        void writeData(QByteArray& array);

        // output file for ROOT trees
        TFile *m_fileDAQRoot;

        // output trees
        // vmm2 outputs
        TTree *vmm2;
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
        TBranch *br_chanId;

        // run configuration
        TTree *run_properties;
        TBranch *br_runNumber;
        TBranch *br_gain;
        TBranch *br_tacSlope;
        TBranch *br_peakTime;
        TBranch *br_dacCounts;
        TBranch *br_pulserCounts;
        TBranch *br_angle;

    private :
        XMLParser *m_parser;
        // methods
        QUdpSocket *m_socketDAQ;
        Configuration* m_config;
        // convert the input array of bits to a QByteArray object
        QByteArray bitsToBytes(QBitArray bits);
        // reverse the order of the input hex datagram and return as an int
        quint32 reverse32(QString datagram_hex);

        // members
        bool m_dbg;
        bool m_useChanMap;
        bool m_ignore16;
        bool m_writeData;
        QByteArray m_bufferEVT;

        // event data from vmm
        int m_eventNumberFAFA;
        int n_daqCnt;
        std::vector<int> m_triggerTimeStamp;
        std::vector<int> m_triggerCounter;
        std::vector<int> m_chipId;
        std::vector<int> m_evSize;
        std::vector< std::vector<int> > m_tdo;
        std::vector< std::vector<int> > m_pdo;
        std::vector< std::vector<int> > m_flag;
        std::vector< std::vector<int> > m_thresh;
        std::vector< std::vector<int> > m_bcid;
        std::vector< std::vector<int> > m_chanId;
        std::vector< std::vector<int> > m_grayDecoded;


}; // class EventBuilder



#endif
