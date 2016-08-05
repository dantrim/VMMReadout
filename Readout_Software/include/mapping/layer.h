#ifndef LAYER_H
#define LAYER_H

#include <string>
#include <vector>

//boost
#include <boost/property_tree/ptree.hpp>

//nsw
#include "coordinates.h"
#include "readout.h"
#include "connectorinfo.h"

class Layer {

    public :
        Layer();
        void setMapDir(std::string mapdir) { m_map_dir = mapdir; }
        bool loadLayer(const boost::property_tree::ptree::value_type pt);

        int nReadout() { return n_readout; }

        // get the Readout object i
        Readout readout(int i);

        // check if this layer has a connector named 'name'
        bool hasConnector(std::string name);

        // get the connector info in this layer named 'connector_name'
        ConnectorInfo getConnectorInfo(std::string connector_name);

        Coordinates position() { return m_position; }
        Coordinates rotation() { return m_rotation; }

        std::string name() { return m_name; }
        std::string id() { return m_id; }

    private :
        std::string m_map_dir;
        std::string m_name;
        std::string m_id;

        int n_readout;
        int n_connector;

        Coordinates m_position;
        Coordinates m_rotation;

        std::vector<Readout> m_readoutArray;
        std::vector<ConnectorInfo> m_connectorInfoArray;

}; // class

#endif
