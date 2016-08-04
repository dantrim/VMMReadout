#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <string>

//boost
#include <boost/property_tree/ptree.hpp>

class Connector {
    public :
        Connector();
        bool loadConnector(const boost::property_tree::ptree::value_type pt);

        std::string name() { return m_connector_name; }
        std::string FEBname() { return m_connected_feb; }

    private :
        int n_connector;
        std::string m_connector_name;
        std::string m_connected_feb;

};

#endif
