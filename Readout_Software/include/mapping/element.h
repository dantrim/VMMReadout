#ifndef ELEMENT_H
#define ELEMENT_H

//std/stl
#include <string>

//boost

class Element {

    public :
        Element();

        void init(int i) { m_id = i; }
        int id() { return m_id; }

        int id_to_int(std::string int_as_string);

        Element& setParentIP(std::string ip)           { m_parent_ip = ip; return *this; }
        Element& setParentID(std::string id)           { m_parent_id = id; return *this; }
        Element& setParentName(std::string name)       { m_parent_name = name; return *this; }
        Element& setVMMID(std::string id)              { m_vmmid = id; return *this; }
        Element& setVMMChan(std::string chan)          { m_vmmchan = chan; return *this; }
        Element& setFEBChan(std::string chan)          { m_FEBchan = chan; return *this; }
        Element& setElementType(std::string type)      { m_element_type = type; return *this; }
        Element& setReadoutType(std::string type)      { m_readout_type = type; return *this; }
        Element& setMultiLayerID(std::string id)       { m_multilayer_id = id; return *this; }
        Element& setMultiLayerName(std::string name)   { m_multilayer_name = name; return *this; }
        Element& setLayerID(std::string id)            { m_layer_id = id; return *this; }
        Element& setLayerName(std::string name)        { m_layer_name = name; return *this; }
        Element& setChamberID(std::string id)          { m_chamber_id = id; return *this; }
        Element& setChamberName(std::string name)      { m_chamber_name = name; return *this; }
        Element& setStripNumber(std::string strip)     { m_strip_number = strip; return *this; }

        const std::string parentIP()          { return m_parent_ip; }
        const std::string parentID()          { return m_parent_id; }
        const std::string parentName()        { return m_parent_name; }
        const std::string vmmID()             { return m_vmmid; }
        const std::string vmmChan()           { return m_vmmchan; }
        const std::string febChan()           { return m_FEBchan; }
        const std::string elementType()       { return m_element_type; }
        const std::string readoutType()       { return m_readout_type; }
        const std::string multilayerID()      { return m_multilayer_id; }
        const std::string multilayerName()    { return m_multilayer_name; }
        const std::string layerID()           { return m_layer_id; }
        const std::string layerName()         { return m_layer_name; }
        const std::string chamberID()         { return m_chamber_id; }
        const std::string chamberName()       { return m_chamber_name; }
        const std::string stripNumber()       { return m_strip_number; }

        std::string monitorString();

    private :
        int m_id;

        std::string m_parent_ip;    // IP of parent FEB that reads this element out
        std::string m_parent_id;    // ID of parent FEB that reads this element out
        std::string m_parent_name;
        std::string m_vmmid;        // ID of VMM that reads this element out
        std::string m_vmmchan;      // VMM channel that reads this element out
        std::string m_FEBchan;      // FEB channel that reads this element out (e.g. MMFE8 channel)
        std::string m_element_type; // element type (e.g. MM strip, sTGC pad, etc...)
        std::string m_readout_type; // type of FEB (e.g. MMFE8, sFEB, etc...)

        std::string m_multilayer_id; // Multilayer ID that contains this element
        std::string m_multilayer_name; // Multilayer name that contians this element

        std::string m_layer_id;         // Layer ID that contains this element
        std::string m_layer_name;       // Layer name that contains this element

        std::string m_chamber_id;       // Chamber ID that contains this element
        std::string m_chamber_name;     // Chamber name that contains this element

        std::string m_strip_number;     // Element number (e.g. MM strip number, sTGC pad number, etc...)

        void print();


}; // class

#endif
