#ifndef CHAMBER_H
#define CHAMBER_H

#include <string>
#include <vector>

//boost
#include <boost/property_tree/ptree.hpp>

//nsw
#include "coordinates.h"
#include "multilayer.h"
#include "connector.h"

class Chamber {
    public :
        Chamber();
        void setMapDir(std::string mapdir) { m_map_dir = mapdir; }
        bool loadChamber(const boost::property_tree::ptree::value_type pt);

        bool loadChamberSpecs(std::string filename);

        std::string name() { return m_name; }
        std::string id() { return m_id; }

        int nMultiLayer() { return n_multilayer; }
        int nConnector() { return n_connector; }

        MultiLayer multiLayer(int i);
        Connector connector(int i);
        bool hasConnector(std::string name);
        Connector getConnector(std::string name);

        std::string connectorNameFromFEBName(std::string feb_name);
        std::string connectorNameFromVMMName(std::string vmm_name);

        bool hasFEB(std::string name);


        Coordinates position() { return m_position; }
        Coordinates rotation() { return m_rotation; }

    private :
        std::string m_map_dir;
        int n_multilayer;
        int n_connector;
        std::string m_name;
        std::string m_id;

        std::string m_specfilename;

        std::vector<MultiLayer> m_multilayerArray;
        std::vector<Connector> m_connectorArray;

        Coordinates m_position;
        Coordinates m_rotation;

};

#endif
