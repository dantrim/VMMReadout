
#ifndef MAP_HANDLER_H
#define MAP_HANDLER_H

//nsw
#include "daqconfig.h"
#include "element.h"

//std/stl
#include <string>
#include <unordered_map>

//boost

//qt
#include <QObject>

namespace nsw_map {

    // { FEB CHANNEL : ELEMENT }
    typedef std::unordered_map<std::string, Element> channel_map;
    // { FEB ID : { FEB CHANNEL : ELEMENT } }
    typedef std::unordered_map<std::string, channel_map> id_map;

}

class MapHandler : public QObject
{
    Q_OBJECT

    public :
        explicit MapHandler(QObject *parent = 0);
        virtual ~MapHandler(){ delete m_daqConfig; }

        bool loadDaqConfiguration(std::string filename);
        DaqConfiguration& config() { return *m_daqConfig; }

        // clear the maps
        void clearMaps();

        // from the XML build the FEB channel to detector element mapping
        void buildMapping();

        // get the board ID from the packet IP
        std::string boardIDfromIP(std::string ip_string);

        // obtain the strip given the FEB ID, VMM ID, and VMM CHANNEL
        int elementStripNumber(std::string board_id, std::string vmm_id, std::string vmm_chan);

        // get the FEB channel from the (vmmid, vmmchan)
        int febChannel(std::string boardid, int vmmid, int vmmchan);

        // mapping [ FEB ID : [ FEB CHANNEL : ELEMENT ] ]
        nsw_map::id_map map() { return m_daq_map; }

    private :
        bool m_initialized;
        bool m_map_loaded;
        DaqConfiguration* m_daqConfig;

        nsw_map::id_map m_daq_map;

    signals :

    public slots :


}; // class

#endif
