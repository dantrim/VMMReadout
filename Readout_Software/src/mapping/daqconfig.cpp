#include <daqconfig.h>

#include <iostream>
#include <sstream>

//boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>


DaqConfiguration::DaqConfiguration() :
    m_febConfigFile(""),
    m_detectorConfigFile(""),
    m_febConfig(nullptr)
{
    std::cout << "DaqConfiguration init" << std::endl; 
}

bool DaqConfiguration::loadDaqXml(std::string filename)
{
    std::cout << "DaqConfiguration::loadDaqXml   " << filename  << std::endl;

    std::string path = "./";
    std::string full_filename = path + filename;
    bool exists = std::ifstream(full_filename).good();

    if(!exists) {
        std::cout << "DaqConfiguration::loadDaqXml   File not found" << std::endl;
        return false;
    }

    std::cout << "DaqConfiguration::loadDaqXml   Found input file: " << full_filename << std::endl;

    if(!readDaqXml(full_filename)) {
        std::cout << "DaqConfiguration::loadDaqXml    ERROR reading daq xml" << std::endl;
        return false;
    }

    std::string full_feb_name = "./" + febConfigFile();
    std::string full_det_name = "./" + detectorConfigFile();
    if(febConfigFile()!="")
        exists = std::ifstream(full_feb_name).good();
    if(!exists) {
        std::cout << "DaqConfiguration::loadDaqXml    Unable to load FEB config file" << std::endl;
        return false;
    }
    if(detectorConfigFile()!="")
        exists = std::ifstream(full_det_name).good();
    if(!exists) {
        std::cout << "DaqConfiguration::loadDaqXml    Unable to load detector config file" << std::endl;
        return false;
    }
    std::cout << "Successfully found DAQ files" << std::endl;
    std::cout << "   > feb config       : " << febConfigFile() << std::endl;
    std::cout << "   > det config       : " << detectorConfigFile() << std::endl;

    return true;
}


bool DaqConfiguration::readDaqXml(std::string filename)
{
    bool ok = true;

    using boost::property_tree::ptree;
    using namespace boost::property_tree::xml_parser;
    ptree pt;

    read_xml(filename, pt, trim_whitespace | no_comments);

    std::string febxml = "";
    std::string detxml = "";

    try
    {
        febxml = pt.get<std::string>("daq_config.feb_file");
        detxml = pt.get<std::string>("daq_config.detector_file");

    } // try
    catch(std::exception &e)
    {
        std::stringstream sx;
        sx << "DaqConfiguration::readDaqXml   ERROR: " << e.what() << "\n";
        std::cout << sx.str() << std::endl;
        ok = false;

    } // catch

    m_febConfigFile = febxml;
    m_detectorConfigFile = detxml;
    return ok;
}

bool DaqConfiguration::loadFEB()
{
    bool ok = true;
    std::cout << "DaqConfiguration::loadFEB" << std::endl;
    m_febConfig = new FEBConfig();
    ok = m_febConfig->loadFEBXml(febConfigFile());

    return ok;
}

bool DaqConfiguration::loadDetectorSetup()
{
    bool ok = true;
    std::cout << "DaqConfiguration::loadDetectorSetup" << std::endl;
    m_detConfig = new DetectorConfig();
    ok = m_detConfig->loadDetectorSetup(detectorConfigFile());
    return ok;
}
