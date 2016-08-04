#ifndef CHIP_H
#define CHIP_H

#include <string>


//boost
#include <boost/property_tree/ptree.hpp>

class Chip {
    public :
        Chip();
        bool loadChip(const boost::property_tree::ptree::value_type pt);

        std::string name() { return m_name; }
        std::string type() { return m_type; }
        std::string id()   { return m_id; }


    private :
        std::string m_name;
        std::string m_type;
        std::string m_id;

}; // class

#endif
