#include "event.h"

//std/stl
#include <sstream>

using namespace std;

////////////////////////////////////////////////////////////////////////
// EVENT
////////////////////////////////////////////////////////////////////////
Event::Event(string chamber,
             string multilayer,
             string layer,
             string readout,
             string strip,
             string fec,
             string chip,
             string channel) :
    n_eventCounter(0),
    m_eventNumberStr(""),
    m_chargeStr(""),
    m_timeStr("")
{
    m_chamberStr = chamber;
    m_multilayerStr = multilayer;
    m_layerStr = layer;
    m_readoutStr = readout;
    m_stripStr = strip;
    m_fecStr = fec;
    m_chipStr = chip;
    m_channelStr = channel;
}

void Event::constructEvent(int event, int charge, int time)
{
    cout << "EVNET::CONSTRCUT EVENT" << endl;
    cout << "ev: " << event << "  charge: " << charge << "  time: " << time << endl;
    n_eventCounter = event;
    m_chargeStr = to_string(charge);
    m_timeStr = to_string(time);
    cout << "LEAVING EVNET::CONSTRCUT EVENT" << endl;
}

string Event::msg()
{
    cout << "msg() coutn : " << n_eventCounter << endl;
    cout << to_string(n_eventCounter) << endl;
    stringstream ss;
    ss  << to_string(n_eventCounter)    << " "
        << m_chamberStr                 << " "
        << m_multilayerStr              << " "
        << m_layerStr                   << " "
        << m_readoutStr                 << " "
        << m_stripStr                   << " "
        << m_fecStr                     << " "
        << m_chipStr                    << " "
        << m_channelStr                 << " "
        << m_chargeStr                  << " "
        << m_timeStr;
    return ss.str();
}
