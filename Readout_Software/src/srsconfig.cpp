#include "srsconfig.h"

//boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

using namespace std;

////////////////////////////////////////////////////////////////////////
// SRSConfig
////////////////////////////////////////////////////////////////////////
SRSConfig::SRSConfig() :
    m_configXml(""),
    m_fecArraySize(0),
    m_fecArray(NULL)
{
}

void SRSConfig::loadXml(string filename)
{
    // already should have checked file and should be providing full filepath
    if(!readXMLfile(filename)) {
        cout << "SRSConfig::loadXml    Exitting." << endl;
        exit(1);
    }

    cout << "SRSConfig::loadXml    INFO SRS configuration loaded from file: " << filename << endl;

    m_configXml = filename;
}

bool SRSConfig::readXMLfile(const string filename)
{
    bool ok = true;

    using boost::property_tree::ptree;
    using namespace boost::property_tree::xml_parser;
    ptree pt;

    read_xml(filename, pt, trim_whitespace | no_comments);

    int fec_counter = 0;

    try {
        BOOST_FOREACH(const ptree::value_type &v, pt.get_child("srs")) {
            if(v.first == "fec") {
                fec_counter++;
            }
        }
    }
    catch(std::exception &e)
    {
        stringstream sx;
        sx << "SRSConfig::readXMLfile    ERROR: " << e.what() << endl;
        cout << sx.str() << endl;
        ok = false;
    }

    m_fecArraySize = fec_counter;
    m_fecArray = new Fec*[fec_counter];
    int n_fec = 0;

    if(ok) {
        BOOST_FOREACH(const ptree::value_type &v, pt.get_child("srs")) {
            if(v.first == "fec") {
                try {
                    cout << "SRSConfig::readXMLfile    at fec[" << n_fec << "] --> NOT LOADING IT YET" << endl;
                    m_fecArray[n_fec] = new Fec();
                    if(!m_fecArray[n_fec]->loadFec(v)) ok = false;
                    #warning previous version sets f++ in array index??
                }
                catch(std::exception &e) {
                    cout << "SRSConfig::readXMLfile    ERROR creating FEC[" << n_fec << "]" << endl;
                    throw;
                }
                n_fec++;
            } // fec
            // previous version had more children, e.g. 'chip_hybrid_maps'
            //else if {

            //}
            else {
                cout << "SRSConfig::readXMLfile    WARNING Unknown key: " << v.first << " in SRS config file: "
                     << m_configXml << endl;
                ok = false;
            }
        } // srs child loop

    }

    for(int ifec = 0; ifec < fec_counter; ifec++) {
        m_fecArray[ifec]->print();
    }

    return ok;
}

void SRSConfig::print()
{
    cout << "-------------------------------------------" << endl;
    cout << "  Print SRS " << endl;
    cout << "    > srs config file: " << m_configXml << endl;
    cout <<"     > # fecs loaded  : " << m_fecArraySize << endl;
    cout << "- - - - - - - - - - - - - - - - - - - - - -" << endl;
    for(int ifec = 0; ifec < m_fecArraySize; ifec++) {
        m_fecArray[ifec]->print();
    }
}

