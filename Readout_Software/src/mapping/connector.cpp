#include "connector.h"

#include <iostream>

//boost
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>


Connector::Connector() :
    n_connector(0),
    m_connector_name(""),
    m_connected_feb("")
{
}

bool Connector::loadConnector(const boost::property_tree::ptree::value_type pt)
{
    bool ok = true;
    using boost::property_tree::ptree;
    using namespace boost;


    try
    {
        BOOST_FOREACH(const ptree::value_type &v, pt.second) {
            //////////////////////////////////
            // name of this connector
            //////////////////////////////////
            if(v.first == "name") {
                m_connector_name = v.second.data();
                boost::trim(m_connector_name);
            }
            //////////////////////////////////
            // name of FEB connected here
            //////////////////////////////////
            else if(v.first == "board") {
                m_connected_feb = v.second.data();
                boost::trim(m_connected_feb);
            }
            //////////////////////////////////
            // chip associated with this
            // connector
            //////////////////////////////////
            else if(v.first == "chip") {
                std::string chip_name = v.second.get<std::string>("name");
                m_chip_names.push_back(chip_name);
            }

            else {
                std::cout << "Connector::loadConnector    WARNING Unknown key: " << v.first << std::endl;
                ok = false;
            }
        } // foreach
    } // try
    catch(std::exception &e)
    {
        std::cout << "Connector::loadConnector    ERROR: " << e.what() << std::endl;
        ok = false;
    }
    return ok;
}

bool Connector::hasVMM(std::string vmm_name)
{
    bool connector_has_this_vmm = false;
    for(int i = 0; i < (int)m_chip_names.size(); i++) {
        if(m_chip_names.at(i) == vmm_name) {
            connector_has_this_vmm = true;
            break;
        }
    }
    return connector_has_this_vmm;
}
