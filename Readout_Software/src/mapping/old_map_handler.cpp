#include "map_handler.h"

#include <iostream>
#include <string>
#include <tuple>

MapHandler::MapHandler() :
    m_initialized(false),
    m_map_loaded(false),
    m_daqConfig(NULL)
{
}
bool MapHandler::loadDaqConfiguration(std::string filename)
{
    m_daqConfig = new DaqConfiguration();

    bool ok = m_daqConfig->loadDaqXml(filename);
    if(ok) {
        std::cout << "MapHandler::loadDaqConfiguration    DAQ xml loaded" << std::endl;
    }
    ok = m_daqConfig->loadFEB();
    if(ok) {
        std::cout << "MapHandler::loadDaqConfiguration    FEB loaded" << std::endl;
    }
    ok = m_daqConfig->loadDetectorSetup();
    if(ok) {
        std::cout << "MapHandler::loadDetectorSetup    Detector setup loaded" << std::endl; 
    }
    m_initialized = ok;

    if(!ok) delete m_daqConfig;

    return ok;
}

void MapHandler::buildMapping()
{
    if(m_map_loaded) m_monitor_map.clear();

    using namespace std;
    int n_element = 0;
    for(int ifeb = 0; ifeb < config().febConfig().nFeb(); ifeb++) {

        FEB feb = config().febConfig().getFEB(ifeb);
        string feb_id = feb.id();
        string feb_name = feb.name();
        cout << " feb name: " << feb.name() << endl;

        mapping::channel_map feb_channel_map; // [feb channel : element ] 

        for(std::tuple<int, int, int> vmmdata : feb.vmm_map) {
            int vmm_id = get<0>(vmmdata);
            int vmm_channel = get<1>(vmmdata);
            int feb_channel = get<2>(vmmdata);
            //std::cout << "  id: " << get<0>(vmmdata) << "  chan: " << get<1>(vmmdata) << "   feb: " << get<2>(vmmdata) << std::endl;



            // want to find the connector <-- layer <-- multilayer <-- chamber that has this feb and then find detector strip connected to feb_channel
            for(int ichamber = 0; ichamber < config().detConfig().nChamber(); ichamber++) {
                Chamber chamber = config().detConfig().getChamber(ichamber);
                if(chamber.hasFEB(feb_name)) {
                    string chamber_name = chamber.name();
                    string connector_name = chamber.connectorNameFromFEBName(feb_name);
                    //cout << "chamber " << chamber.name() << "  has feb named: " << feb_name << "  at connector: " << connector_name << endl;
                    for(int iML = 0; iML < chamber.nMultiLayer(); iML++) {
                        string multilayer_name = chamber.multiLayer(iML).name();
                        string multilayer_id   = chamber.multiLayer(iML).id();
                        for(int iL = 0; iL < chamber.multiLayer(iML).nLayer(); iL++) {
                            Layer layer = chamber.multiLayer(iML).getLayer(iL);
                            if(layer.hasConnector(connector_name)) {
                                string layer_name = layer.name();
                                string layer_id   = layer.id();

                                ConnectorInfo connector_info = layer.getConnectorInfo(connector_name);
                                int strip_number = connector_info.stripNumberFromFEBChannel(feb_channel);
                                if(!(strip_number<0)) {
                                    std::cout << "feb name: " << feb_name << " (vmmid, vmmchan, febchan) = (" << vmm_id << ", " << vmm_channel << " , " << feb_channel << ")   chamber: " << chamber_name << "  connector: " << connector_name << "  strip: " << strip_number << std::endl;

                                    Element current_element;
                                    current_element.init(n_element);
                                    current_element.setVMMID(to_string(vmm_id))
                                                .setVMMChan(to_string(vmm_channel))
                                                .setFEBChan(to_string(feb_channel));
                                    //current_element.setVMMID(to_string(vmm_id));
                                    //current_element.setVMMChan(to_string(vmm_channel));
                                    //current_element.setFEBChan(to_string(feb_channel));
                                    current_element.setStripNumber(to_string(strip_number));
                                    feb_channel_map[to_string(feb_channel)] = current_element;
                                    n_element++;

                                } // strip_number is positive (i.e. the detector has a connection at feb_channel
                            } // this layer has the connector to which the FEB is connected
                        } // iL
                    } // iML 
                } // chamber has FEB
                //cout << " chamber name: " << chamber.name() << endl;
            } // ichamber  

        } // vmmdata for feb 

        cout << "FEB ID: " << feb_id << endl;
        m_monitor_map[feb_id] = feb_channel_map;

    } // ifeb

    //for(auto mon_map : m_monitor_map) {
    //    cout << "feb id: " << mon_map.first << endl;
    //    mapping::channel_map channel_mapping = mon_map.second; 
    //    for(auto chan_map : channel_mapping) {
    //        cout << "     feb chan: " << chan_map.first << "  strip: " << (chan_map.second).stripNumber() << endl;
    //    } // chan_map
    //} // mon_map

    cout << " [1][222] = " << m_monitor_map["1"]["222"].stripNumber() << std::endl;

    m_map_loaded = true;


}
