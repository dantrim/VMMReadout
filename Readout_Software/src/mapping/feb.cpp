#include "feb.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

FEB::FEB() :
    n_chip(0),
    m_map_dir(""),
    m_vmm_map_filename(""),
    m_name(""),
    m_id(""),
    m_ip(""),
    m_type("")
{
}

bool dataCompare(const std::tuple<int, int, int> &lhs,
                    const std::tuple<int, int, int> &rhs)
{
    return std::get<2>(lhs) < std::get<2>(rhs);
}

bool FEB::loadFEB(const boost::property_tree::ptree::value_type pt)
{
    bool ok = true;

    using boost::property_tree::ptree;
    using namespace boost;

    int chip_counter = 0;
    try
    {
        BOOST_FOREACH(const ptree::value_type &v, pt.second) {
            if(v.first == "chip") chip_counter++;
        } // foreach
    } // try
    catch(std::exception &e)
    {
        std::cout << "FEB::loadFEB    ERROR loading FEB: " << e.what() << std::endl;
        ok = false;
    } // catch

    if(ok) {
        n_chip = chip_counter;
        std::cout << "FEB::loadFEB    Loading " << n_chip << " chips..." << std::endl;

        try
        {
            BOOST_FOREACH(ptree::value_type v, pt.second) {
                //////////////////////////
                // name
                //////////////////////////
                if(v.first == "name") {
                    m_name = v.second.data();
                    trim(m_name);
                }
                //////////////////////////
                // ip
                //////////////////////////
                else if(v.first == "ip") {
                    m_ip = v.second.data();
                    trim(m_ip);
                }
                //////////////////////////
                // id
                //////////////////////////
                else if(v.first == "id") {
                    m_id = v.second.data();
                    trim(m_id);
                }
                //////////////////////////
                // board type
                //////////////////////////
                else if(v.first == "type") {
                    m_type = v.second.data();
                    trim(m_type);
                }
                //////////////////////////
                // load the chips
                //////////////////////////
                else if(v.first == "chip") {
                    Chip tmpChip;
                    if(!tmpChip.loadChip(v)) ok = false;
                    if(ok) m_chipArray.push_back(tmpChip);
                }
                //////////////////////////
                // load the (vmm id, vmmchannel) -> FEB channel map
                //////////////////////////
                else if(v.first == "map_file") {
                    m_vmm_map_filename = v.second.data(); 
                    boost::trim(m_vmm_map_filename);
                    std::string vmm_map_file = m_map_dir + "/" + m_vmm_map_filename;
                    if(!loadMapFile(vmm_map_file)) ok = false;
                    if(ok) {
                        m_vmm_map_filename = vmm_map_file;
                    }
                }
                else {
                    std::cout << "FEB::loadFEB    ERROR Unknown key: " << v.first << std::endl;
                    ok = false;
                }
            } //foreach
        } // try
        catch(std::exception &e)
        {
            std::cout << "FEB::loadFEB    ERROR: " << e.what() << std::endl;
            ok = false;
        } // catch
    } // ok

    if(ok) {
        std::cout << "FEB::loadFEB" << std::endl;
        std::cout << "  > name      : " << name() << std::endl;
        std::cout << "  > id        : " << id() << std::endl;
        std::cout << "  > ip        : " << ip() << std::endl;
    }

    return ok;
}

bool FEB::loadMapFile(std::string filename)
{
    bool ok = true;
    bool exists = std::ifstream(filename).good();
    if(!exists) {
        std::cout << "FEB::loadMapFile    ERROR Unable to laod file: " << filename << std::endl;
        ok = false;
    }

    std::cout << "FEB::loadMapFile    Found input vmm map file: " << filename << std::endl;

    if(ok) {
        if(!readMapFile(filename)) {
            std::cout << "FEB::loadMapFile    ERROR Unable to read vmm map file" << std::endl;
            ok = false;
        }
    }
    return ok;
}

bool FEB::readMapFile(std::string filename)
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

        std::string vmmid_str;
        std::string vmmchan_str;
        std::string febchan_str;

        enum { state_vmmid, state_vmmchan, state_feb } parse_state;

        parse_state = state_vmmid;

        for(tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter) {
            // vmm id
            if(parse_state == state_vmmid) {
                vmmid_str = boost::trim_copy(*tok_iter);
                parse_state = state_vmmchan;
            }
            // vmm channel
            else if(parse_state == state_vmmchan) {
                vmmchan_str = boost::trim_copy(*tok_iter);
                parse_state = state_feb;
            }
            // feb channel
            else if(parse_state == state_feb) {
                febchan_str = boost::trim_copy(*tok_iter);
            }
        } // tok_iter

        if(parse_state != state_feb) {
            ok = false;
            std::stringstream sx;
            sx << "FEB::readMapFile    ERROR Reading map file " << m_vmm_map_filename
               << " at line " << line_counter << "\n";
            throw std::runtime_error(sx.str().c_str());
        }

        //std::cout << "id: " << vmmid_str << "  vmmchan: " << vmmchan_str << "  febchan: " << febchan_str << std::endl;
        vmm_map.push_back( std::make_tuple(stoi(vmmid_str), stoi(vmmchan_str), stoi(febchan_str)) );

    } // while

    infile.close();
    sort(vmm_map.begin(), vmm_map.end(), dataCompare);

    return ok;
}

int FEB::getFEBChannel(int vmmid, int vmmchannel)
{
    for(std::tuple<int, int, int> vmmdata : vmm_map) {
        if( (std::get<0>(vmmdata) == vmmid) && (std::get<1>(vmmdata) == vmmchannel) )
            return std::get<2>(vmmdata);
    }
    return -1;
}

std::string FEB::VMMNameFromId(int vmmid)
{
    std::string out = "";
    for(int ichip = 0; ichip < nChip(); ichip++) {
        if(m_chipArray.at(ichip).id() == std::to_string(vmmid))
            out = m_chipArray.at(ichip).name();
    } // ichip 
    return out;
}
