#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

//qt
class QFile;
class QMap;
class QByteArray;

//us
class XMLParser;

class DataProcessor
{
    public :
        DataProcessor();
        virtual ~DataProcessor(){};
        void setDebug(bool dbg) { m_dbg = dbg; };
        void setDAQConfig(QString file);
        void getDAQConfig();
        int mappedChannel(int chipNumber, int channelNumber);

        void parseData(QByteArray array);

    private :

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


}; // class DataProcessor

#endif // DATAPROCESSOR_H
