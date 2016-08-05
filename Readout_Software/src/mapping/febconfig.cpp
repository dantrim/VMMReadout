#include "febconfig.h"

#include <iostream>
#include <sstream>

//boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>


FEBConfig::FEBConfig() :
    n_feb(0),
    m_map_dir("")
{
}

bool FEBConfig::loadFEBXml(std::string filename)
{
    bool ok = true;

    using boost::property_tree::ptree;
    using namespace boost::property_tree::xml_parser;
    ptree pt;

    read_xml(filename, pt, trim_whitespace | no_comments);

    int feb_counter = 0;
    try
    {
        BOOST_FOREACH(const ptree::value_type &v, pt.get_child("feb")) {
            if(v.first == "board") feb_counter++;
        } // v
    } // try
    catch(std::exception &e)
    {
        std::stringstream sx;
        sx << "FEBConfig::loadFEBXml    ERROR: " << e.what() << std::endl;
        std::cout << sx.str() << std::endl;
        ok = false;
    } // catch

    if(ok) {
        n_feb = feb_counter;
        std::cout << "FEBConfig::loadFEBXml    Loading " << n_feb << " boards..." << std::endl;

        BOOST_FOREACH(const ptree::value_type &v, pt.get_child("feb")) {
            if(v.first == "board") {
                try
                {
                    FEB tmpFeb;
                    tmpFeb.setMapDir(m_map_dir);
                    if(!tmpFeb.loadFEB(v)) ok = false;
                    if(ok) m_febArray.push_back(tmpFeb);
                } // try
                catch(std::exception &e)
                {
                    std::cout << "FEBConfig::loadFEBXml    ERROR: " << e.what() << std::endl;
                    ok = false;
                }
            } // board
            else {
                std::cout << "FEBConfig::loadFEBXml   WARNING Unknown key: " << v.first << " in FEB xml" << std::endl;
                ok = false;
            }

        } // feb

    } // ok

    return ok;
}

FEB FEBConfig::getFEB(int i)
{
    if(!hasFEB(i)) {
        std::cout << "FEBConfig::getFEB    ERROR Attempting to access FEB number " << i
                  << " but only " << n_feb << " FEB are loaded!" << std::endl;
    }
    return m_febArray.at(i);
}

std::string FEBConfig::boardNameFromIp(std::string ip)
{
    std::string out_name = "";
    for(int ifeb = 0; ifeb < nFeb(); ifeb++) {
        if(m_febArray.at(ifeb).ip() == ip) {
            out_name = m_febArray.at(ifeb).name();
            break;
        }
    }
    return out_name;
}

std::string FEBConfig::boardIdFromIp(std::string ip)
{
    std::string out_id = "";
    for(int ifeb = 0; ifeb < nFeb(); ifeb++) {
        if(m_febArray.at(ifeb).ip() == ip) {
            out_id = m_febArray.at(ifeb).id();
            break;
        }
    } // ifeb
    return out_id;
}


std::string FEBConfig::boardContainsChip(std::string chipname)
{
    std::string out_name = "";
    for(int ifeb = 0; ifeb < nFeb(); ifeb++) {
        std::string current_feb = m_febArray.at(ifeb).name();
        int nchip = m_febArray.at(ifeb).nChip();
        for(int ichip = 0; ichip < nchip; ichip++) {
            if(m_febArray.at(ifeb).m_chipArray.at(ichip).name() == chipname) {
                out_name = current_feb;
            } 
        } // ichip
    } // ifeb
    return out_name;
}

