#include "chip.h"

#include <iostream>

//boost
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

Chip::Chip() :
    m_name(""),
    m_type(""),
    m_id("")
{
}

bool Chip::loadChip(const boost::property_tree::ptree::value_type pt)
{
    bool ok = true;

    using boost::property_tree::ptree;
    using namespace boost;

    try
    {
        BOOST_FOREACH(ptree::value_type v, pt.second) {

            //////////////////////////////////////
            // name
            //////////////////////////////////////
            if(v.first == "name") {
                m_name = v.second.data();
                trim(m_name);
            }
            //////////////////////////////////////
            // type
            //////////////////////////////////////
            else if(v.first == "type") {
                m_type = v.second.data();
                trim(m_type);
            }
            //////////////////////////////////////
            // id
            //////////////////////////////////////
            else if(v.first == "id") {
                m_id = v.second.data();
                trim(m_id);
            }

            else {
                std::cout << "Chip::loadChip    ERROR Unknown key: " << v.first << std::endl;
                ok = false;
            }
        } // foreach
    } // try
    catch(std::exception &e)
    {
        std::cout << "Chip::loadChip    ERROR: " << e.what() << std::endl;
        ok = false;
    }

    if(ok) {
        std::cout << "Chip::loadChip" << std::endl;
        std::cout << "   > name     : " << name() << std::endl;
        std::cout << "   > id       : " << id() << std::endl;
        std::cout << "   > type     : " << type() << std::endl;
    }

    return ok;
}
