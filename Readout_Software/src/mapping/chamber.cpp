#include "chamber.h"

#include <iostream>
#include <sstream>

//boost
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>

Chamber::Chamber() :
    m_map_dir(""),
    n_multilayer(0),
    n_connector(0),
    m_name(""),
    m_id(""),
    m_specfilename("")
{
}

bool Chamber::loadChamber(const boost::property_tree::ptree::value_type pt)
{
    bool ok = true;
    using boost::property_tree::ptree;
    using namespace boost;

    int connector_counter = 0;
    try
    {
        BOOST_FOREACH(const ptree::value_type &v, pt.second) {
            if(v.first == "connector") connector_counter++;
        } // foreach
    } // try
    catch(std::exception &e) {
        std::cout << "Chamber::loadChamber    ERROR: " << e.what() << std::endl;
        ok = false;
    } // catch

    if(ok) {
        n_connector = connector_counter;

        try
        {
            BOOST_FOREACH(const ptree::value_type &v, pt.second) {
                ///////////////////////////////
                // chamber spec file
                ///////////////////////////////
                if(v.first == "config_file") {
                    m_specfilename = v.second.data();
                    boost::trim(m_specfilename);
                    std::string spec_file = m_map_dir + "/" + m_specfilename;
                    if(!loadChamberSpecs(spec_file)) { 
                        ok = false;
                        std::cout << "Chamber::loadChamber    Unable to load spec file: " << spec_file << std::endl;
                    }
                    if(ok) {
                        m_specfilename = spec_file;
                        std::cout << "Chamber::loadChamber    Chamber spec laoded: " << m_specfilename << std::endl;
                    }
                }
                ///////////////////////////////
                // name
                ///////////////////////////////
                else if(v.first == "name") {
                    m_name = v.second.data();
                    trim(m_name);
                }
                ///////////////////////////////
                // id
                ///////////////////////////////
                else if(v.first == "id") {
                    m_id = v.second.data();
                    trim(m_id);
                }
                ///////////////////////////////
                // connectors
                ///////////////////////////////
                else if(v.first == "connector") {
                    Connector tmpConnector;
                    if(!tmpConnector.loadConnector(v)) ok = false;
                    if(ok) m_connectorArray.push_back(tmpConnector); 
                }
                ///////////////////////////////
                // coordinates
                ///////////////////////////////
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
                    std::cout << "Chamber::loadChamber    WARNING Unknown key: " << v.first << std::endl;
                    ok = false;
                }
                
            } // foreach
        } // try
        catch(std::exception &e)
        {
            std::cout << "Chamber::loadChamber    ERROR parsing chamber file: " << e.what() << std::endl;
            ok = false;
        } // catch
    } // ok

    return ok;
}


bool Chamber::loadChamberSpecs(std::string filename)
{
    bool ok = true;
    using boost::property_tree::ptree;
    using namespace boost::property_tree::xml_parser;
    ptree pt;

    read_xml(filename, pt, trim_whitespace | no_comments);

    bool exists = std::ifstream(filename).good();
    if(!exists) {
        std::cout << "Chamber::loadChamberSpecs    ERROR Unable to find chamber spec file: " << filename << std::endl;
        ok = false;
    }

    std::cout << "Chamber::loadChamberSpecs    Found chamber spec file: " << filename << std::endl;

    int multilayer_count = 0;

    try
    {
        BOOST_FOREACH(const ptree::value_type &v, pt.get_child("chamber")) {
            if(v.first == "multilayer") multilayer_count++;
        } // foreach
    } // try
    catch(std::exception &e)
    {
        std::cout << "Chamber::loadChamberSpecs    ERROR: " << e.what() << std::endl;
        ok = false;
    } // catch

    if(ok) {
        n_multilayer = multilayer_count;

        try
        {
            BOOST_FOREACH(const ptree::value_type &v, pt.get_child("chamber")) {
                //////////////////////////////////
                // load the multilayer
                //////////////////////////////////
                if(v.first == "multilayer") {
                    MultiLayer tmpMultiLayer;
                    tmpMultiLayer.setMapDir(m_map_dir);
                    if(!tmpMultiLayer.loadMultiLayer(v)) ok = false;
                    if(ok) m_multilayerArray.push_back(tmpMultiLayer);
                }

                else {
                    std::cout << "Chamber::loadChamberSpecs    WARNING Unknown key: " << v.first << std::endl;
                    ok = false;
                }
            } // foreach
        } // try
        catch(std::exception &e)
        {
            std::cout << "Chamber::loadChamberSpecs    ERROR: " << e.what() << std::endl;
            ok = false;
        } // catch
    } // ok

    return ok;
}

MultiLayer Chamber::multiLayer(int i)
{
    if(i>n_multilayer) {
        std::cout << "Chamber::multiLayer    ERROR Attempting to grab multilayer " << i
                  << " but chamber " << name() << " has " << nMultiLayer() << " multilayers!" << std::endl;
    }
    return m_multilayerArray.at(i);
}

Connector Chamber::connector(int i)
{
    if(i>n_connector) {
        std::cout << "Chamber::connector    ERROR Attempting to grab connector " << i
                  << " but chamber " << name() << " has " << nConnector() << " connectors!" << std::endl;
    }
    return m_connectorArray.at(i);
}

bool Chamber::hasConnector(std::string name)
{
    bool found = false;
    for(int i = 0; i < nConnector(); i++) {
        if(m_connectorArray.at(i).name() == name) {
            found = true;
            break;
        }
    } // i
    return found;
}

bool Chamber::hasFEB(std::string name)
{
    bool found = false;
    for(int i = 0; i < nConnector(); i++) {
        if(m_connectorArray.at(i).FEBname() == name) {
            found = true;
            break;
        }
    } // i
    return found;
}

Connector Chamber::getConnector(std::string name)
{
    Connector dummy;
    if(hasConnector(name)) {
        int out_index = -1;
        for(int i = 0; i < nConnector(); i++) {
            if(m_connectorArray.at(i).name() == name) {
                out_index = i;
                break;
            }
        } // i
        return m_connectorArray.at(out_index);
    }
    else { return dummy; }
}

std::string Chamber::connectorNameFromFEBName(std::string feb_name)
{
    std::string out = "";
    for(int i = 0; i < nConnector(); i++) {
        if(m_connectorArray.at(i).FEBname() == feb_name) { 
            out = m_connectorArray.at(i).name();
            break;
        }
    } // i
    return out;
}

std::string Chamber::connectorNameFromVMMName(std::string vmm_name)
{
    std::string out = "";
    for(int i = 0; i < nConnector(); i++) {
        if(m_connectorArray.at(i).hasVMM(vmm_name)) {
            out = m_connectorArray.at(i).name();
            break;
        }
    } // i
    return out;
}
