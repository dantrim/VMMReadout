#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <Qstring>
#include <vector>
#include <QMap>

class XMLParser
{

    public :
        XMLParser();
        virtual ~XMLParser(){};
        void loadXml(QString xmlfilename);

        QString getChipType() { return m_type; };

        int getStripNumber(int chipNumber, int vmmChannel);

    private :

        void getChipMapping();

        QString m_type;
        QString m_map_filename;
        QString m_output_directory;
        QMap<QString, QMap<QString, int> > myMap;
        QMap<int, std::vector<int> > m_mini2_map;



};

#endif // XMLPARSER_H
