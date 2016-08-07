#ifndef FEBCONFIG_H
#define FEBCONFIG_H

#include <string>
#include <vector>

#include "feb.h"


class FEBConfig {

    public :
        FEBConfig();
        bool loadFEBXml(std::string filename);
        int nFeb() { return n_feb; }

        void setMapDir(std::string mapdir) { m_map_dir = mapdir; }

        //useful methods

        // get the name of the FEB that has IP 'ip'
        std::string boardNameFromIp(std::string ip);

        // get the id of the FEB that has IP 'ip'
        std::string boardIdFromIp(std::string ip);

        // get the name of the FEB that the chip with
        // name 'chipname' is on
        std::string boardContainsChip(std::string chipname);

        // return the i'th FEB from m_febArray
        bool hasFEB(int i) { return i < (int)m_febArray.size(); }
        FEB getFEB(int i);

        FEB getFEBwithId(std::string id_no);

    private :
        int n_feb;
        std::string m_map_dir;

        std::vector<FEB> m_febArray;
        

}; // class


#endif
