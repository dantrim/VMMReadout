#include "layer.h"

//boost
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

Layer::Layer() :
    m_map_dir(""),
    m_name(""),
    m_id(""),
    n_readout(0),
    n_connector(0)
{
}

bool Layer::loadLayer(const boost::property_tree::ptree::value_type pt)
{
    bool ok = true;
    using boost::property_tree::ptree;
    using namespace boost;

    int readout_count = 0;
    int connector_count = 0;

    try
    {
        BOOST_FOREACH(const ptree::value_type &v, pt.second) {
            if(v.first == "readout") readout_count++;
            else if(v.first == "connector") connector_count++;
        } // foreach
    } // try
    catch(std::exception &e)
    {
        std::cout << "Layer::loadLayer    ERROR: " << e.what() << std::endl;
        ok = false;
    } // catch

    if(ok) {
        n_readout = readout_count;
        n_connector = connector_count;

        try
        {
            BOOST_FOREACH(const ptree::value_type &v, pt.second) {
                /////////////////////////////////
                // name
                /////////////////////////////////
                if(v.first == "name") {
                    m_name = v.second.data();
                    trim(m_name);
                }
                /////////////////////////////////
                // id
                /////////////////////////////////
                else if(v.first == "id") {
                    m_id = v.second.data();
                    trim(m_id);
                }
                /////////////////////////////////
                // coordinates
                /////////////////////////////////
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
                /////////////////////////////////
                // readout
                /////////////////////////////////
                else if(v.first == "readout") {
                    Readout tmpReadout;
                    if(!tmpReadout.loadReadout(v)) ok = false;
                    if(ok) m_readoutArray.push_back(tmpReadout);
                }
                /////////////////////////////////
                // connector info
                /////////////////////////////////
                else if(v.first == "connector") {
                    ConnectorInfo tmpConnectorInfo;
                    tmpConnectorInfo.setMapDir(m_map_dir);
                    if(!tmpConnectorInfo.loadConnectorInfo(v)) ok = false;
                    if(ok) m_connectorInfoArray.push_back(tmpConnectorInfo);
                }
            } // foreach
        } // try
        catch(std::exception &e)
        {
            std::cout << "Layer:loadLayer    ERROR: " << e.what() << std::endl;
            ok = false;
        }
    } // ok

    return ok;
}

Readout Layer::readout(int i)
{
    if(i>nReadout())
        std::cout << "Layer::readout    ERROR Trying to access readout element " << i
                  << " but there are only " << nReadout() << " loaded!" << std::endl;
    return m_readoutArray.at(i);
}

bool Layer::hasConnector(std::string name)
{
    bool has_connector = false;
    for(int i = 0; i < n_connector; i++) {
        if(m_connectorInfoArray.at(i).name() == name) {
            has_connector = true;
            break;
        }
    }
    return has_connector;
}


ConnectorInfo Layer::getConnectorInfo(std::string name)
{
    ConnectorInfo out_;
    int out_index = -1;
    bool found = false;
    for(int i = 0; i < n_connector; i++) {
        if(m_connectorInfoArray.at(i).name() == name) {
            out_index = i;
            found = true;
            break;
        }
    } // i
    if(!found) {
        std::cout << "Layer::getConnectorInfo    WARNING Did not find the connector named "
                  << "\"" << name << "\"!" << std::endl;
        return out_;
    }
    else { return m_connectorInfoArray.at(out_index); }
}
