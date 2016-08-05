#include "detectorconfig.h"

#include <iostream>
#include <sstream>

//boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

DetectorConfig::DetectorConfig() :
    m_map_dir(""),
    m_name(""),
    n_chambers(0)
{
}

bool DetectorConfig::loadDetectorSetup(std::string filename)
{

    bool ok = true;
    using boost::property_tree::ptree;
    using namespace boost::property_tree::xml_parser;
    ptree pt;

    read_xml(filename, pt, trim_whitespace | no_comments);

    int chamber_counter = 0;
    try
    {
        BOOST_FOREACH(const ptree::value_type &v, pt.get_child("detector")) {
            if(v.first == "chamber") chamber_counter++;
        } // foreach
    } // try
    catch(std::exception &e)
    {
        std::stringstream sx;
        sx << "DetectorConfig::loadDetectorSetup    ERROR: " << e.what() << "\n";
        std::cout << sx.str() << std::endl;
        ok = false;
    } // catch

    if(ok) {
        n_chambers = chamber_counter;
        std::cout << "DetectorConfig::loadDetectorSetup    Loading " << n_chambers << " chambers..." << std::endl;

        try
        {
            BOOST_FOREACH(const ptree::value_type &v, pt.get_child("detector")) {

                ////////////////////////////////
                // load a chamber
                ////////////////////////////////
                if(v.first == "chamber") {
                    Chamber tmpChamber;
                    tmpChamber.setMapDir(m_map_dir); 
                    if(!tmpChamber.loadChamber(v)) ok = false;
                    if(ok) m_chamberArray.push_back(tmpChamber);
                }
                ////////////////////////////////
                // name
                ////////////////////////////////
                else if(v.first == "name") {
                    m_name = v.second.data();
                    boost::trim(m_name);
                }
                ////////////////////////////////
                // coordinates
                ////////////////////////////////
                else if(v.first == "position") {
                    m_position.set(v.second.get<double>("x"),
                                   v.second.get<double>("y"),
                                   v.second.get<double>("z")); 
                }
                else if(v.first == "rotation") {
                    m_rotation.set(v.second.get<double>("x"),
                                   v.second.get<double>("y"),
                                   v.second.get<double>("z"));
                }

                else {
                    std::cout << "DetectorConfig::loadDetectorSetup    WARNING Unknown key: " << v.first << std::endl;
                    ok = false;
                }
            } // foreach
        } // try
        catch(std::exception &e)
        {
            std::cout << "DetectorConfig::loadDetectorSetup    ERROR: " << e.what() << std::endl;
            ok = false;
        } // catch
    } // ok

    return ok;
}

