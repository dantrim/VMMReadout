#include "connectorinfo.h"

#include <iostream>
#include <sstream>

//boost
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

ConnectorInfo::ConnectorInfo() :
    m_map_dir(""),
    m_name(""),
    m_map_filename("")
{
}

bool dataCompare(const std::tuple<int, int> &lhs,
                    const std::tuple<int, int> &rhs)
{
    return std::get<0>(lhs) < std::get<0>(rhs);
}

bool ConnectorInfo::loadConnectorInfo(const boost::property_tree::ptree::value_type pt)
{
    bool ok = true;
    using boost::property_tree::ptree;
    using namespace boost;

    try
    {
        BOOST_FOREACH(const ptree::value_type &v, pt.second) {
            ////////////////////////////////
            // map file
            ////////////////////////////////
            if(v.first == "map_file") {
                m_map_filename = v.second.data();
                trim(m_map_filename);
                std::string mapfile = m_map_dir + "/" + m_map_filename;
                if(!loadMapFile(mapfile)) { 
                    ok = false;
                    std::cout << "ConnectorInfo::loadConnectorInfo    Unable to load map: " << mapfile << std::endl;
                }
            }
            ////////////////////////////////
            // name
            ////////////////////////////////
            else if(v.first == "name") {
                m_name = v.second.data();
                trim(m_name);
            }

            else {
                std::cout << "ConnectorInfo::loadConnectorInfo    WARNING Unknown key: " << v.first << std::endl;
                ok = false;
            }
        } // foreach
    } // try
    catch(std::exception &e)
    {
        std::cout << "ConnectorInfo::loadConnectorInfo    ERROR: " << e.what() << std::endl;
        ok = false;
    } // catch

    return ok;
}

bool ConnectorInfo::loadMapFile(std::string filename)
{
    bool ok = true;
    bool exists = std::ifstream(filename).good();
    if(!exists) {
        std::cout << "ConnectorInfo::loadMapFile    ERROR Unable to load file: " << filename << std::endl;
        ok = false;
    }

    std::cout << "ConnectorInfo::loadMapFile    Found input spec file: " << filename << std::endl;

    if(ok) {
        if(!readMapFile(filename)) {
            std::cout << "ConnectorInfo::loadMapFile    ERROR Unable to read spec file" << std::endl;
            ok = false;
        }
    } // ok

    return ok;
}

bool ConnectorInfo::readMapFile(std::string filename)
{
    bool ok = true;

    std::ifstream infile(filename.c_str());
    std::string line;

    int line_counter = 0;

    while(getline(infile, line)) {
        line_counter++;

        boost::trim(line);
        if(boost::starts_with(line, "#") || line.empty()) continue;

        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(", \t");
        tokenizer tokens(line, sep);

        std::string detector_strip_str;
        std::string feb_channel_str;

        enum { state_detector, state_feb } parse_state;

        parse_state = state_detector;

        for(tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter) {
            // detector strip
            if(parse_state == state_detector) {
                detector_strip_str = boost::trim_copy(*tok_iter);
                parse_state = state_feb;
            }
            // feb channel connected to detector strip
            else if(parse_state == state_feb) {
                feb_channel_str = boost::trim_copy(*tok_iter);
            }
        } // tok_iter

        if(parse_state != state_feb) {
            ok = false;
            std::stringstream sx;
            sx << "ConnectorInfo::readMapFile    ERROR Reading map file " << m_map_filename
                << " at line " << line_counter << "\n";
            throw std::runtime_error(sx.str().c_str());
        }

        //std::cout << "det strip: " << detector_strip_str << "  feb: " << feb_channel_str << std::endl;
        data.push_back( std::make_tuple(stoi(detector_strip_str), stoi(feb_channel_str)) );
    } // while

    infile.close();
    sort(data.begin(), data.end(), dataCompare);

    return ok;

}

int ConnectorInfo::stripNumberFromFEBChannel(int feb_channel)
{
    for(std::tuple<int, int> element_data : data) {
        if( std::get<1>(element_data) == feb_channel )
            return std::get<0>(element_data);
    }
    return -1;
}
