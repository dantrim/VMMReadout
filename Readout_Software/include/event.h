#ifndef EVENT_H
#define EVENT_H

#include <iostream>
#include <string>

class Event {
    public :
        Event(std::string chamber,
                std::string multilayer,
                std::string layer,
                std::string readout,
                std::string strip,
                std::string fec,
                std::string chip,
                std::string channel);

        void constructEvent(int event, int charge, int time);
        std::string msg();

    private :
        int n_eventCounter;

        std::string m_eventNumberStr;
        std::string m_chamberStr;
        std::string m_multilayerStr;
        std::string m_layerStr;
        std::string m_readoutStr;
        std::string m_stripStr;
        std::string m_fecStr;
        std::string m_chipStr;
        std::string m_channelStr;
        std::string m_chargeStr;
        std::string m_timeStr;

}; // class


#endif
