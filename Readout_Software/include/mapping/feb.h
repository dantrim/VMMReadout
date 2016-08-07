#ifndef FEB_H
#define FEB_H

#include <string>
#include <vector>
#include <tuple>

//mm
#include "chip.h"

//boost
#include <boost/property_tree/ptree.hpp>

class FEB {
    public :
        FEB();
        void setMapDir(std::string mapdir) { m_map_dir = mapdir; }
        bool loadFEB(const boost::property_tree::ptree::value_type pt);

        bool loadMapFile(std::string filename);
        bool readMapFile(std::string filename);

        int getFEBChannel(int vmmid, int vmmchannel);
        std::string VMMNameFromId(int vmm_id);

        std::string name() { return m_name; }
        std::string id()   { return m_id; }
        std::string ip()   { return m_ip; }
        std::string type() { return m_type; }

        int nChip() { return n_chip; }
        std::vector<Chip> m_chipArray;

        std::vector<std::tuple<int, int, int> > vmm_map; // (vmm id, vmm channel, FEB channel)

    private :
        int n_chip; 
        std::string m_map_dir;
        std::string m_vmm_map_filename;
        std::string m_name;
        std::string m_id;
        std::string m_ip;
        std::string m_type;
        
};

#endif
