#include "xmlparser.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QStringList>
#include <QDomDocument>
#include <QDir>
#include <QDebug>

XMLParser::XMLParser()
{

}

void XMLParser::loadXml(QString xmlfilename)
{

    QFile daqXMLFile(xmlfilename);
    if(!daqXMLFile.exists()) {
        qDebug() << "[XMLParser::loadXml]    File: " << xmlfilename << " is not found.";
        abort();
    }

    bool fileOpened = daqXMLFile.open(QIODevice::ReadOnly);
    if(fileOpened) {
        qDebug() << "[XMLParser::loadXml]    File opened: " << daqXMLFile.fileName();
    } else {
        qDebug() << "[XMLParser::loadXml]    Error opening file: " << daqXMLFile.fileName();
        abort();
    }

    // ----------------- parse the DAQ config xml ---------------------- //
    QDomDocument daqDoc;
    daqDoc.setContent(&daqXMLFile);

    QDomElement root = daqDoc.documentElement();
    QDomElement child = root.firstChildElement("");

    if(!child.isNull()) {
        while(!child.isNull()) {
            if(child.tagName()=="elx_file") {
                if(child.tagName().contains("mini2")) m_type = "MINI2";
                m_map_filename = child.text();
            }
            else if(child.tagName()=="data_directory") {
                m_output_directory = child.text();
            }
            child = child.nextSiblingElement("");
        } // while
    }


    // ---------------- get chip mapping ------------------- //
    getChipMapping();
}

void XMLParser::getChipMapping()
{
    if(m_type=="") {
        qDebug() << "[XMLParser::getChipMapping]    ELx type is \"\"!";
        abort();
    }
    if(m_map_filename=="") {
        qDebug() << "[XMLParser::getChipMapping]    Map filename is \"\"!";
        abort();
    }

    QFile mapFile(m_map_filename);
    if(!mapFile.exists()) {
        qDebug() << "[XMLParser::getChipMapping]    File: " << m_map_filename << " is not found.";
        abort();
    }

    bool fileOpened = mapFile.open(QIODevice::ReadOnly);
    if(fileOpened) {
        qDebug() << "[XMLParser::getChipMapping]    File opened: " << mapFile.fileName();
    } else {
        qDebug() << "[XMLParser::getChipMapping]    Error opening file: " << mapFile.fileName();
        abort();
    }
    QTextStream in(&mapFile);
    while(!in.atEnd()) {
        QString line = in.readLine();
        if(line.left(1)!="#") {

            // ----------------- MINI2 case -------------------- //
            if(m_type=="MINI2") {
                QStringList line_list = line.split("  ", QString::SkipEmptyParts); // assume column sepearted by 2-spaces
                std::vector<int> chip_list;
                chip_list.push_back(line_list.at(1).toInt());
                chip_list.push_back(line_list.at(2).toInt());
                m_mini2_map.insert(line_list.at(0).toInt(), chip_list);
            }
        } // if not #
    } // while

}

int XMLParser::getStripNumber(int chipNumber, int vmmChannel)
{
    int strip = 0; 
    if(m_type=="") {
        qDebug() << "[XMLParser::getStripNumber]    ELx type is \"\"!";
        abort();
    }
    // ---------------- MINI2 case ----------------- //
    if(m_type=="MINI2") {
        if(chipNumber%2==0) {
            strip = m_mini2_map[vmmChannel][0];
        } else {
            strip = m_mini2_map[vmmChannel][1];
        }
    }

    return strip;

}
