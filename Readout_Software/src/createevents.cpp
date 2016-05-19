#include "createevents.h"

//std/stl
#include <set>

//boost
#include <boost/lexical_cast.hpp>

////////////////////////////////////////////////////////////////////////
// CREATEVENTS
////////////////////////////////////////////////////////////////////////
CreateEvents::CreateEvents() :
    n_eventCounter(-1),
    m_daq(0),
    m_srs(0),
    m_detector(0)
{
    cout << "INITIALIZE CREATEVENTS" << endl;
}
void CreateEvents::setDaq(DaqConfig* daq)
{
    m_daq = daq;
    m_srs = &daq->getSrsConfig();
    m_detector = &daq->getDetectorConfig();
    n_eventCounter = -1;
}


// this method is the "createNewEvents3()" from the older DAQ code
void CreateEvents::createEvents()
{
    for(int i = 0; i < getDetector().getChamberArraySize(); i++) {
        string chamber_name = getDetector().m_chamberArray[i]->getName();
        set<string> readout;
        for(int j = 0; j < getDetector().m_chamberArray[i]->connectorArraySize(); j++) {
            //if(!getDetector().m_chamberArray[i]->getChipName().compare(4,1,"2")) break; // this is an APV thing?
            cout << "CreateEvents::createEvents   " << getDetector().m_chamberArray[i]->m_connectorArray[j]->getChipName() << " : " << getDetector().m_chamberArray[i]->m_connectorArray[j]->getName() << endl;

            string chip_name = getDetector().m_chamberArray[i]->m_connectorArray[j]->getChipName();
            string connector_name = getDetector().m_chamberArray[i]->m_connectorArray[j]->getName();
            string chamber_name = getDetector().m_chamberArray[i]->getName();
            //cout << __LINE__ << endl;

            map_channel channels;
            for(int k = 0; k < getDetector().m_chamberArray[i]->m_chamberSpecs->getConnectorArraySize(); k++) {
                if(!connector_name.compare(getDetector().m_chamberArray[i]->m_chamberSpecs->m_connectorArray[k]->getName())) {
                    //cout << __LINE__ << endl;
                    string fec_name = getSrs().getFecContainsChipName(getDetector().m_chamberArray[i]->m_connectorArray[j]->getChipName());
                    for(vector<tuple<int, int, int, string, int>>::iterator it = getDetector().m_chamberArray[i]->m_chamberSpecs->m_connectorArray[k]->data.begin();
                                it != getDetector().m_chamberArray[i]->m_chamberSpecs->m_connectorArray[k]->data.end(); ++it) {

                        // map the chamber pin to the connector/chip pint
                        tuple<int, int, int> myTuple = getDetector().m_chamberArray[i]->m_connectorArray[j]->getPin(get<0>(*it));

                        // { connector strip : Event() }
                        // channels 0-63 of VMM are odd # VMMs
                        if(boost::lexical_cast<int>(get<0>(*it)) < 64) {
                            channels[get<1>(myTuple)] = new Event( chamber_name, to_string(get<1>(*it)), to_string(get<2>(*it)), get<3>(*it), to_string(get<4>(*it)), fec_name,
                                                                        "NaN", to_string(get<1>(myTuple)));
                        }
                        // channels 64-127 are even # VMMs
                        else if(boost::lexical_cast<int>(get<0>(*it)) >=64) {
                            channels[get<2>(myTuple)] = new Event( chamber_name, to_string(get<1>(*it)), to_string(get<2>(*it)), get<3>(*it), to_string(get<4>(*it)), fec_name,
                                                                        "NaN", to_string(get<2>(myTuple)));
                            //cout << "yep  " <<  chamber_name << "  " << to_string(get<1>(*it)) << "  " << to_string(get<2>(*it)) << "  " << get<3>(*it) << "  " << to_string(get<4>(*it)) << "  " << fec_name << "  " << "NaN" << "  " << to_string(get<2>(myTuple)) << endl;
                        }
                        
                    } // it
                } // this connector is attached to this chamber element
            } // k
            // { chip name : { connector strip : Event() } }
            m_chips_map[chip_name] = channels;
        } // j
    }//i
}

string CreateEvents::getEvent(int chip, int channel, int event, int charge, int time, int charge2, int time2)
{
    if(chip >= 16)
        chip -= 16;
    string chip_str;
    if (chip < 10) {
        chip_str = "VMM2.1.0";
        chip_str += boost::lexical_cast<std::string>(chip);
    }
    else {
        chip_str = "VMM2.1.";
        chip_str += boost::lexical_cast<std::string>(chip);
    }

    cout << "\n\nHardcoding chip name for mapping ==> fix!\n\n" << endl;
    chip_str = "VMM2.1.00";
    cout << "CreateEvents::getEvent    event for [" << chip_str << "][" << channel << "]" << endl;
    return ((m_chips_map.find(chip_str))->second).find(channel)->second->constructEvent(event, charge, time, charge2, time2);
    //((m_chips_map.find(chip_str))->second).find(channel)->second->constructEvent(event, charge, time, charge2, time2);

   // cout << "CreateEvents::getEvent Event msg(): " << ((m_chips_map.find(chip_str))->second).find(channel)->second->msg() << endl;
   // return ((m_chips_map.find(chip_str))->second).find(channel)->second->msg();
}
    
