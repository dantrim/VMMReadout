#include "connector.h"

//std/stl
#include <fstream>
#include <stdlib.h> //getenv

//boost
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

////////////////////////////////////////////////////////////////////////
// CONNECTOR
////////////////////////////////////////////////////////////////////////
Connector::Connector() :
    m_name(""),
    m_id(""),
    m_chipName(""),
    m_map_filename("")
{
}

bool Connector::loadConnector(const boost::property_tree::ptree::value_type pt)
{
    bool ok = true;

    using boost::property_tree::ptree;
    using namespace boost;

    try {
        BOOST_FOREACH(const ptree::value_type &v, pt.second) {
            ////////////////////////////////////////////
            // chip
            ////////////////////////////////////////////
            if(v.first =="chip") {
                m_chipName = v.second.get<string>("name");
                trim(m_chipName);
            }
            ////////////////////////////////////////////
            // map file
            ////////////////////////////////////////////
            else if(v.first == "map_file") {
                m_map_filename = v.second.data();
                trim(m_map_filename);
                if(!loadMapFile(m_map_filename)) ok = false;
            }
            ////////////////////////////////////////////
            // name
            ////////////////////////////////////////////
            else if(v.first == "<xmlattr>") {
                m_name = v.second.get<string>("name", "Nan");
                trim(m_name);
            }

            else {
                cout << "Connector::loadConnector    WARNING Unknown key: " << v.first << endl;
                ok = false;
            }
        } //foreach

    } //try
    catch(std::exception &e) {
        cout << "Connector::loadConnector    ERROR Unable to load connector: " << e.what() << endl;
        ok = false;
    }

    return ok;
}

bool Connector::loadMapFile(string filename)
{
    bool ok = true;

    char* mmdaqpath;
    mmdaqpath = getenv("MMDAQDIR");
    if(mmdaqpath==NULL) {
        cout << "Connector::loadMapFile    ERROR Environment \"MMDAQDIR\" not found!" << endl;
        cout << "Connector::loadMapFile       -> Set this with \"set_daqenv.sh\" script." << endl;
        exit(1);
    }
    string path(mmdaqpath);

}
