#include "readout.h"

//boost
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>

Readout::Readout() :
    m_name(""),
    m_id(""),
    m_minstrip(-1),
    m_maxstrip(-1)
{
}

bool Readout::loadReadout(const boost::property_tree::ptree::value_type pt)
{
    bool ok = true;
    using boost::property_tree::ptree;
    using namespace boost;

    try
    {
        BOOST_FOREACH(const ptree::value_type &v, pt.second) {
            ////////////////////////////////
            // name
            ////////////////////////////////
            if(v.first == "name") {
                m_name = v.second.data();
                trim(m_name);
            }
            ////////////////////////////////
            // id
            ////////////////////////////////
            else if(v.first == "id") {
                m_id = v.second.data();
                trim(m_id);
            }
            ////////////////////////////////
            // coordinates
            ////////////////////////////////
            else if(v.first == "position") {
                m_position.set(v.second.get<double>("x"),
                               v.second.get<double>("y"),
                               v.second.get<double>("z"));
            }
            else if(v.first == "rotation") {
                m_rotation.set(v.second.get<double>("x"),
                               v.second.get<double>("y"),
                               v.second.get<double>("z"));
            }
            ////////////////////////////////
            // strip range
            ////////////////////////////////
            else if(v.first == "strip_range") {
                m_minstrip = v.second.get<int>("min");
                m_maxstrip = v.second.get<int>("max");
            }
            ////////////////////////////////
            // pitch
            ////////////////////////////////
            else if(v.first == "pitch") {
                m_pitch = v.second.data();
            }

            else {
                std::cout << "Readout::loadReadout    WARNING Unknown key: " << v.first << std::endl;
                ok = false;
            }
        } // foreach
    } // try
    catch(std::exception &e)
    {
        std::cout << "Readout::loadReadout    ERROR: " << e.what() << std::endl;
        ok = false;
    } // catch

    return ok;
}
