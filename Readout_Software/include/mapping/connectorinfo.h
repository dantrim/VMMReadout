#ifndef CONNECTORINFO_H
#define CONNECTORINFO_H

#include <string>
#include <vector>
#include <tuple>

//boost
#include <boost/property_tree/ptree.hpp>

//nsw

class ConnectorInfo {
    public :
        ConnectorInfo();
        void setMapDir(std::string mapdir) { m_map_dir = mapdir; }
        bool loadConnectorInfo(const boost::property_tree::ptree::value_type pt);

        bool loadMapFile(std::string filename);
        bool readMapFile(std::string filename);

        std::string name() { return m_name; }

        int stripNumberFromFEBChannel(int feb_channel);

        std::vector<std::tuple<int, int> > data; // detector strip, FEB channel

    private :
        std::string m_map_dir;
        std::string m_name;
        std::string m_map_filename;

}; // class

#endif
