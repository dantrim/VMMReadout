#include <daqconfig.h>

#include <iostream>
#include <sstream>

//boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>


DaqConfiguration::DaqConfiguration() :
    m_map_dir(""),
    m_febConfigFile(""),
    m_detectorConfigFile(""),
    m_febConfig(nullptr)
{
    std::cout << "DaqConfiguration init" << std::endl; 
}

bool DaqConfiguration::loadDaqXml(std::string filename)
{
    std::cout << "DaqConfiguration::loadDaqXml   " << filename  << std::endl;

    //std::string path = "./";
    //std::string full_filename = path + filename;
    std::string full_filename = filename;
    bool exists = std::ifstream(full_filename).good();

    if(!exists) {
        std::cout << "DaqConfiguration::loadDaqXml   File (" << full_filename << ") not found" << std::endl;
        return false;
    }

    std::cout << "DaqConfiguration::loadDaqXml   Found input file: " << full_filename << std::endl;

    if(!readDaqXml(full_filename)) {
        std::cout << "DaqConfiguration::loadDaqXml    ERROR reading daq xml" << std::endl;
        return false;
    }

// size_t found;
//  cout << "Splitting: " << str << endl;
//  found=str.find_last_of("/\\");
//  cout << " folder: " << str.substr(0,found) << endl;
//  cout << " file: " << str.substr(found+1) << endl;
    //size_t dir_found;
    //dir_found = full_filename.find_last_of("/\\");
    //std::string full_feb_name = full_filename.substr(dir_found+1) + febConfigFile();
    //std::cout << "DaqConfiguration::loadDaqXml    FEB file: " << full_feb_name << std::endl;
//boost::filesystem::path p("C:\\folder\\foo.txt");
//boost::filesystem::path dir = p.parent_path();
    boost::filesystem::path p_config(full_filename);
    boost::filesystem::path dir = p_config.parent_path();
    std::string full_feb_name = dir.string() + "/" + febConfigFile();
    std::cout << "full_feb_name: " << full_feb_name << std::endl;
    std::string full_det_name = dir.string() + "/" + detectorConfigFile();
    std::cout << "full_det_name: " << full_det_name << std::endl;

    // set the map dir (this is propagated down)
    m_map_dir = dir.string();


    //std::string full_feb_name = "./" + febConfigFile();
    //std::string full_det_name = "./" + detectorConfigFile();
    if(febConfigFile()!="")
        exists = std::ifstream(full_feb_name).good();
    if(!exists) {
        std::cout << "**************************************************************************" << std::endl;
        std::cout << "DaqConfiguration::loadDaqXml    ERROR FEB config file does not exists: " << full_feb_name << std::endl;
        std::cout << "**************************************************************************" << std::endl;
        //std::cout << "DaqConfiguration::loadDaqXml    Unable to load FEB config file: " << full_feb_name << std::endl;
        return false;
    }
    if(detectorConfigFile()!="")
        exists = std::ifstream(full_det_name).good();
    if(!exists) {
        std::cout << "**************************************************************************" << std::endl;
        std::cout << "DaqConfiguration::loadDaqXml    ERROR detector config file does not exists: " << full_det_name << std::endl;
        std::cout << "**************************************************************************" << std::endl;
        //std::cout << "DaqConfiguration::loadDaqXml    Unable to load detector config file: " << full_det_name << std::endl;
        return false;
    }
    std::cout << "Successfully found DAQ files" << std::endl;
    std::cout << "   > map directory    : " << m_map_dir << std::endl;
    std::cout << "   > feb config       : " << full_feb_name << std::endl;
    std::cout << "   > det config       : " << full_det_name << std::endl;

    // update the filenames to have full path
    m_febConfigFile = full_feb_name;
    m_detectorConfigFile = full_det_name;

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
    m_febConfig->setMapDir(m_map_dir);
    ok = m_febConfig->loadFEBXml(febConfigFile());

    return ok;
}

bool DaqConfiguration::loadDetectorSetup()
{
    bool ok = true;
    std::cout << "DaqConfiguration::loadDetectorSetup" << std::endl;
    m_detConfig = new DetectorConfig();
    m_detConfig->setMapDir(m_map_dir);
    ok = m_detConfig->loadDetectorSetup(detectorConfigFile());
    return ok;
}
