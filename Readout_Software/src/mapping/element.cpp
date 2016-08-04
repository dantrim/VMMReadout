#include "element.h"

//std/stl
#include <iostream>
#include <sstream>

//boost
#include <boost/lexical_cast.hpp>

Element::Element() :
    m_id(-1),
    m_parent_ip(""),
    m_parent_id(""),
    m_parent_name(""),
    m_vmmid(""),
    m_vmmchan(""),
    m_FEBchan(""),
    m_element_type(""),
    m_readout_type(""),
    m_multilayer_id(""),
    m_multilayer_name(""),
    m_layer_id(""),
    m_layer_name(""),
    m_chamber_id(""),
    m_chamber_name(""),
    m_strip_number("")
{
}

int Element::id_to_int(std::string int_as_string)
{
    int out = -1;
    try
    {
        out = boost::lexical_cast<int>(int_as_string);
    } // try
    catch(const boost::bad_lexical_cast &e)
    {
        std::cout << "Element::id_to_int    Bad lexical cast (unable to convert " << int_as_string << " to integer)!" << std::endl;
    } // catch
    return out;
}

std::string Element::monitorString()
{
    using namespace std;
    stringstream sx;
    sx << parentName() << "\t"
       << vmmID() << "\t"
       << vmmChan() << "\t"
       << febChan() << "\t"
       << stripNumber();
    return sx.str();
}

void Element::print()
{
    using namespace std;
    stringstream sx;

    sx << " --------------------------------------------------------- \n"
        << "  Element " << id() << "\n"
        << "    > Type         : " << elementType() << "\n"
        << "    > Readout      : " << readoutType() << "\n"
        << "        - strip    : " << stripNumber() << "\n"
        << "    > FEB \n"
        << "        - id       : " << parentID() << "\n"
        << "        - ip       : " << parentIP() << "\n"
        << "        - name     : " << parentName() << "\n"
        << "        - - - - - - - - - - - - - - - - - - - - \n"
        << "        - vmm id   : " << vmmID() << "\n"
        << "        - vmm chan : " << vmmChan() << "\n"
        << "        - FEB chan : " << febChan() << "\n"
        << "    > Chamber \n"
        << "        - id       : " << chamberID() << "\n"
        << "        - name     : " << chamberName() << "\n"
        << "    > MultiLayer \n"
        << "        - id       : " << multilayerID() << "\n"
        << "        - name     : " << multilayerName() << "\n"
        << "    > Layer \n"
        << "        - id       : " << layerID() << "\n"
        << "        - name     : " << layerName() << "\n"
        << " --------------------------------------------------------- ";
    cout << sx.str() << endl;
}
