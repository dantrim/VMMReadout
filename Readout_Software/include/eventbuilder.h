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

class EventBuilder : public QObject
{
    Q_OBJECT

    public :
        explicit EventBuilder(QObject *parent = 0);
        virtual ~EventBuilder(){};
        void setDebug(bool dbg) { m_dbg = dbg; }
        void setUseChanMap(bool useMap) { m_useChanMap = useMap; }
        void setIgnore16(bool ignore) { m_ignore16 = ignore; }
        void setWriteData(bool doWrite) { m_writeData = doWrite; }

        void setupOutputTree();

        // output file for ROOT trees
        TFile *m_fileDAQRoot;

        // output trees
        // vmm2 outputs
        TTree *m_vmm2;
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
        TTree *m_run_properties;
        TBranch *br_runNumber;
        TBranch *br_gain;
        TBranch *br_tacSlope;
        TBranch *br_peakTime;
        TBranch *br_dacCounts;
        TBranch *br_pulserCounts;
        TBranch *br_angle;

    private :
        // methods
        QUdpSocket *m_socketDAQ;
        Configuration* m_config;
        void initialize(const QUdpSocket *socket, const Configuration* config);
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
        int m_eventNumberFAFAVariable;
        std::vector<int> m_triggerTimeStampVariable;
        std::vector<int> m_triggerCounterVariable;
        std::vector<int> m_chipIdVariable;
        std::vector<int> m_evSizeVariable;
        std::vector< std::vector<int> > m_tdoVariable;
        std::vector< std::vector<int> > m_pdoVariable;
        std::vector< std::vector<int> > m_flagVariable;
        std::vector< std::vector<int> > m_threshVariable;
        std::vector< std::vector<int> > m_bcidVariable;
        std::vector< std::vector<int> > m_chanIdVariable;
        std::vector< std::vector<int> > m_grayDecodedVariable;


    signals :

    public slots :
        void dataPending(); 

    private slots :

}; // class EventBuilder



#endif
