#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <iostream>
#include <string>
#include <tuple>
#include <vector>


//mmdaq
#include "srs.h"

//boost
#include <boost/property_tree/ptree.hpp>

class Connector {
    public :
        Connector();
        bool loadConnector(const boost::property_tree::ptree::value_type pt);

        bool loadMapFile(std::string filename);

    private :
        std::string m_name;
        std::string m_id;
        std::string m_chipName;
        std::string m_map_filename;

}; // class

#endif
