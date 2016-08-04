#ifndef MAP_HANDLER_H
#define MAP_HANDLER_H

#include "daqconfig.h"
#include "element.h"

//boost
#include <boost/unordered_map.hpp>

#include <memory>
#include <map>

namespace mapping {

    // { FEB CHANNEL : ELEMENT }
    typedef boost::unordered_map<std::string, Element> channel_map;
    // { FEB ID : { FEB CHANNEL: ELEMENT } }
    typedef boost::unordered_map<std::string, channel_map> feb_map;

    //// { VMMCHANNEL : ELEMENT }
    //typedef boost::unordered_map<std::string, Element> vmmchan_element_map;
    //// { VMMID : { VMMCHAN : ELEMENT } }
    //typedef boost::unordered_map<std::string, vmmchan_element_map> vmmid_vmmchan_map;
    //// { FEB ID : { VMMID : { VMMCHAN : ELEMENT } } }
    //typedef boost::unordered_map<std::string, vmmid_vmmchan_map> febid_vmmid_map;
}

class MapHandler {

    public :
        MapHandler();

        bool loadDaqConfiguration(std::string filename);

        DaqConfiguration& config() { return *m_daqConfig; }

        void buildMapping();
        mapping::feb_map map() { return m_monitor_map; }

    private :
        bool m_initialized;
        bool m_map_loaded;
        DaqConfiguration* m_daqConfig;

        mapping::feb_map m_monitor_map;

}; // class

#endif
