// vmm
#include "config_handler.h"
//#include "string_utils.h"

// std/stl
#include <exception>
#include <sstream>

// boost
#include <boost/format.hpp>

//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  ConfigHandler -- Constructor
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
ConfigHandler::ConfigHandler(QObject *parent)
    : QObject(parent)
{
    m_channelmap.clear();
    m_channels.clear();

    ////////////////////////////////////////
    // allowd values for global settings
    ////////////////////////////////////////
    all_gains<<"0.5"<<"1.0"<<"3.0"<<"4.5"<<"6.0"<<"9.0"<<"12.0"<<"16.0";
    all_peakTimes<<200<<100<<50<<25;
    all_TACslopes<<125<<250<<500<<1000;
    all_polarities<<"wires"<<"strips";
    all_ARTmodes<<"threshold"<<"peak";
    all_directTimeModes<<"TtP"<<"ToT"<<"PtP"<<"PtT";
    all_ADC10bits<<"200ns"<<"+60ns";
    all_ADC8bits<<"100ns"<<"+60ns";
    all_ADC6bits<<"low"<<"middle"<<"up";


}
//// ------------------------------------------------------------------------ //
void ConfigHandler::LoadConfig(const QString &filename)
{
    using boost::property_tree::ptree;
    using namespace boost::property_tree::xml_parser;
    ptree pt;
    read_xml(filename.toStdString(), pt, trim_whitespace | no_comments);

    LoadCommInfo(pt);
    LoadGlobalSettings(pt);
    LoadDAQConfig(pt);
    LoadHDMIChannels(pt);
    LoadVMMChannelConfig(pt);

    std::cout << "ChannelMap size   : " << m_channelmap.size() << std::endl;
    std::cout << "VMM channels size : " << m_channels.size()   << std::endl;

}
//// ------------------------------------------------------------------------ //
void ConfigHandler::LoadCommInfo(const boost::property_tree::ptree& pt)
{
    using boost::property_tree::ptree;
    using namespace std;
    try
    {
        CommInfo comm;
        for(const auto& conf : pt.get_child("configuration")) {
            if(!(conf.first == "udp_setup" || conf.first == "general_info"))
                continue;


            if(conf.first=="udp_setup") {
                // FEC port
                comm.fec_port = conf.second.get<int>("fec_port");
                // DAQ port
                comm.daq_port = conf.second.get<int>("daq_port");
                // VMMASIC port
                comm.vmmasic_port = conf.second.get<int>("vmmasic_port");
                // VMMAPP port
                comm.vmmapp_port = conf.second.get<int>("vmmapp_port");
                // S6 port
                comm.s6_port = conf.second.get<int>("s6_port");
            } // udp_setup
            else if(conf.first=="general_info") {
                // config version
                string ver = conf.second.get<string>("config_version");
                comm.config_version = QString::fromStdString(ver);
                // vmm id list
                // TODO split up the list here and store as vector<QString>!!
                string id = conf.second.get<string>("vmm_id");
                comm.vmm_id_list = QString::fromStdString(id);
                // ip list
                string ip = conf.second.get<string>("ip");
                comm.ip_list = QString::fromStdString(ip);
                // comment field
                string comment = conf.second.get<string>("comment");
                comm.comment = QString::fromStdString(comment);
                // debug
                string dbg = conf.second.get<string>("debug");
                comm.debug = isOn(dbg);
            }
        }

        m_commSettings = comm;
        m_commSettings.Print();

    }
    catch(std::exception &e)
    {
        std::cout << "!! --------------------------------- !!" << std::endl;
        std::cout << "ERROR CONF: " << e.what() << std::endl;
        std::cout << "!! --------------------------------- !!" << std::endl;
        exit(1);
    }


}
//// ------------------------------------------------------------------------ //
void ConfigHandler::LoadGlobalSettings(const boost::property_tree::ptree& pt)
{
    using boost::property_tree::ptree;
    using namespace std;
    try
    {
        for(const auto& conf : pt.get_child("configuration")) {
            if( !(conf.first == "global_settings") ) continue;

            GlobalSetting g;

            // ------------------------------------------------------------- //
            // global channel polarity
            string pol = conf.second.get<string>("ch_polarity"); 
            if(all_polarities.indexOf(QString::fromStdString(pol))>=0)
                g.polarity = all_polarities.indexOf(QString::fromStdString(pol));
            else {
                cout << "ERROR ch_polarity value must be one of: [";
                for(auto& i : all_polarities) cout << " " << i.toStdString() << " ";
                cout << "], you have provided: " << pol << endl;
                exit(1);
            }

            // ------------------------------------------------------------- //
            // global channel leakage current ena/dis
            string leak = conf.second.get<string>("ch_leakage_current");
            g.leakage_current = isOn(leak);
       
            // ------------------------------------------------------------- //
            // analog tristates
            string tristates = conf.second.get<string>("analog_tristates");
            g.analog_tristates = isOn(tristates);

            // ------------------------------------------------------------- //
            // double leakage
            string dleak = conf.second.get<string>("double_leakage");
            g.double_leakage = isOn(dleak);

            // ------------------------------------------------------------- //
            // gain
            string gain = conf.second.get<string>("gain");
            g.gain = QString::fromStdString(gain);
            if(all_gains.indexOf(QString::fromStdString(gain))>=0)
                g.gain = all_gains.indexOf(QString::fromStdString(gain));
            else {
                cout << "ERROR gain value must be one of: [";
                for(auto& i : all_gains)  cout << " " << i.toStdString() << " ";
                cout << "] mV/fC, you have provided: " << gain << endl;
                exit(1);
            }
            
            // ------------------------------------------------------------- //
            // peak_time
            g.peak_time = conf.second.get<int>("peak_time");
            if( !(all_peakTimes.indexOf(g.peak_time)>=0) ) {
                cout << "ERROR peak_time value must be one of: [";
                for(auto& i : all_peakTimes) cout << " " << i << " ";
                cout << "] ns, you have provided: " << g.peak_time << endl;
                exit(1);
            }

            // ------------------------------------------------------------- //
            // neighbor trigger
            string neighbor = conf.second.get<string>("neighbor_trigger");
            g.neighbor_trigger = isOn(neighbor);

            // ------------------------------------------------------------- //
            // TAC slope
            g.tac_slope = conf.second.get<int>("TAC_slop_adj");
            if( !(all_TACslopes.indexOf(g.tac_slope)>=0) ) {
                cout << "ERROR TAC_slop_adj value must be one of: [";
                for(auto& i : all_TACslopes) cout << " " << i << " ";
                cout << "] ns, you have provided: " << g.tac_slope << endl;
                exit(1);
            }

            // ------------------------------------------------------------- //
            // disable at peak
            string disable = conf.second.get<string>("disable_at_peak");
            g.disable_at_peak = isOn(disable);

            // ------------------------------------------------------------- //
            // ART
            string art = conf.second.get<string>("ART");
            g.art = isOn(art);

            // ------------------------------------------------------------- //
            // ART mode
            string artmode = conf.second.get<string>("ART_mode");
            if( all_ARTmodes.indexOf(QString::fromStdString(artmode))>=0)
                g.art_mode = QString::fromStdString(artmode);
            else {
                cout << "ERROR ART_mode value must be one of: [";
                for(auto& i : all_ARTmodes) cout << " " << i.toStdString() << " ";
                cout << "], you have provided: " << artmode << endl;
                exit(1);
            }

            // ------------------------------------------------------------- //
            // dual clock ART
            string dcart = conf.second.get<string>("dual_clock_ART");
            g.dual_clock_art = isOn(dcart);

            // ------------------------------------------------------------- //
            // out buffer mo
            string obmo = conf.second.get<string>("out_buffer_mo");
            g.out_buffer_mo = isOn(obmo);

            // ------------------------------------------------------------- //
            // out buffer pdo
            string obpdo = conf.second.get<string>("out_buffer_pdo");
            g.out_buffer_pdo = isOn(obpdo);

            // ------------------------------------------------------------- //
            // out buffer tdo
            string obtdo = conf.second.get<string>("out_buffer_tdo");
            g.out_buffer_tdo = isOn(obtdo);

            // ------------------------------------------------------------- //
            // channel for monitoring
            g.channel_monitor = conf.second.get<int>("channel_monitoring");

            // ------------------------------------------------------------- //
            // monitoring control
            string mcontrol = conf.second.get<string>("monitoring_control");
            g.monitoring_control = isOn(mcontrol);

            // ------------------------------------------------------------- //
            // monitor pdo out
            string mpdo = conf.second.get<string>("monitor_pdo_out");
            g.monitor_pdo_out = isOn(mpdo);

            // ------------------------------------------------------------- //
            // ADCs
            string adcs = conf.second.get<string>("ADCs");
            g.adcs = isOn(adcs);

            // ------------------------------------------------------------- //
            // sub hysteresis
            string subhyst = conf.second.get<string>("sub_hyst_discr");
            g.sub_hysteresis = isOn(subhyst);

            // ------------------------------------------------------------- //
            // direct time
            string direct = conf.second.get<string>("direct_time");
            g.direct_time = isOn(direct);

            // ------------------------------------------------------------- //
            // direct time mode
            string dtmode = conf.second.get<string>("direct_time_mode");
            g.direct_time_mode = QString::fromStdString(dtmode);

            // ------------------------------------------------------------- //
            // conv mode 8bit
            string cmode8bit = conf.second.get<string>("conv_mode_8bit");
            g.conv_mode_8bit = isOn(cmode8bit);

            // ------------------------------------------------------------- //
            // enable 6bit
            string ena6 = conf.second.get<string>("enable_6bit");
            g.enable_6bit = isOn(ena6);

            // ------------------------------------------------------------- //
            // ADC 10bit
            string adc10 = conf.second.get<string>("ADC_10bit");
            if(all_ADC10bits.indexOf(QString::fromStdString(adc10))>=0)
                g.adc_10bit = all_ADC10bits.indexOf(QString::fromStdString(adc10));
            else {
                cout << "ERROR ADC_10bit value must be one of: [";
                for(auto& i : all_ADC10bits) cout << " " << i.toStdString() << " ";
                cout << "], you have provided: " << adc10 << endl;
                exit(1);
            }

            // ------------------------------------------------------------- //
            // ADC 8bit
            string adc8 = conf.second.get<string>("ADC_8bit");
            if(all_ADC8bits.indexOf(QString::fromStdString(adc8))>=0)
                g.adc_8bit = all_ADC8bits.indexOf(QString::fromStdString(adc8));
            else {
                cout << "ERROR ADC_8bit value must be one of: [";
                for(auto& i : all_ADC8bits) cout << " " << i.toStdString() << " ";
                cout << "], you have provided: " << adc8 << endl;
                exit(1);
            }

            // ------------------------------------------------------------- //
            // ADC 6bit
            string adc6 = conf.second.get<string>("ADC_6bit");
            if(all_ADC6bits.indexOf(QString::fromStdString(adc6))>=0)
                g.adc_6bit = all_ADC6bits.indexOf(QString::fromStdString(adc6));
            else {
                cout << "ERROR ADC_6bit value must be one of: [";
                for(auto& i : all_ADC6bits) cout << " " << i.toStdString() << " ";
                cout << "], you have provided: " << adc6 << endl;
                exit(1);
            }

            // ------------------------------------------------------------- //
            // dual clock data
            string dcdata = conf.second.get<string>("dual_clock_data");
            g.dual_clock_data = isOn(dcdata);

            // ------------------------------------------------------------- //
            // dual clock 6bit
            string dc6 = conf.second.get<string>("dual_clock_6bit");
            g.dual_clock_6bit = isOn(dc6);

            // ------------------------------------------------------------- //
            // threshold DAC
            g.threshold_dac = conf.second.get<int>("threshold_DAC");
            
            // ------------------------------------------------------------- //
            // test pulse DAC
            g.test_pulse_dac = conf.second.get<int>("test_pulse_DAC");

            m_globalSettings = g;
            m_globalSettings.Print();
            
        }
    }
    catch(std::exception &e)
    {
        std::cout << "!! --------------------------------- !!" << std::endl;
        std::cout << "ERROR GLOBAL: " << e.what() << std::endl;
        std::cout << "!! --------------------------------- !!" << std::endl;
        exit(1);

    }


}
//// ------------------------------------------------------------------------ //
void ConfigHandler::LoadDAQConfig(const boost::property_tree::ptree& pt)
{
    using boost::property_tree::ptree;
    using namespace std;

    try
    {
        for(const auto& conf : pt.get_child("configuration")) {
            if( !(conf.first == "trigger_daq") ) continue;

            TriggerDAQ daq;

            // tp delay
            daq.tp_delay = conf.second.get<int>("tp_delay");
            // trigger period
            string tperiod = conf.second.get<string>("trigger_period");
            daq.trigger_period = QString::fromStdString(tperiod);
            // acq sync
            daq.acq_sync = conf.second.get<int>("acq_sync");
            // acq window
            daq.acq_window = conf.second.get<int>("acq_window");
            // run mode
            string rmode = conf.second.get<string>("run_mode");
            daq.run_mode = QString::fromStdString(rmode);
            // run count
            daq.run_count = conf.second.get<int>("run_count");
            // output path
            string opath = conf.second.get<string>("output_path");
            daq.output_path = QString::fromStdString(opath);
            // output filename
            string oname = conf.second.get<string>("output_filename");
            daq.output_filename = QString::fromStdString(oname);

            m_daqSettings = daq;
            m_daqSettings.Print();
        }
    }
    catch(std::exception &e)
    {
        std::cout << "!! --------------------------------- !!" << std::endl;
        std::cout << "ERROR DAQ: " << e.what() << std::endl;
        std::cout << "!! --------------------------------- !!" << std::endl;
        exit(1);
    }
}
//// ------------------------------------------------------------------------ //
void ConfigHandler::LoadHDMIChannels(const boost::property_tree::ptree& pt)
{
    using boost::property_tree::ptree;
    using namespace std;
    using boost::format;

    stringstream ss;
    stringstream val;
    try
    {
        for(const auto& conf : pt.get_child("configuration")) {
            if( !(conf.first == "channel_map") ) continue;
            for(int iHDMI = 1; iHDMI < 9; iHDMI++) {
                ss.str("");
                ss << "hdmi_" << format("%1i") % iHDMI;
                val.str("");

                // number
                ChannelMap chMap;
                chMap.hdmi_no = iHDMI;

                // HDMI switch
                val << ss.str() << ".switch";
                chMap.on = isOn(conf.second.get<string>(val.str()));
                // vmm0 on/off
                val.str("");
                val << ss.str() << ".first";
                chMap.first = bool(conf.second.get<int>(val.str()));
                // vmm1 on/off
                val.str("");
                val << ss.str() << ".second";
                chMap.second = bool(conf.second.get<int>(val.str()));

                //chMap.Print();
                m_channelmap.push_back(chMap);

            } // iHDMI
        }
    }
    catch(std::exception &e)
    {
        std::cout << "!! --------------------------------- !!" << std::endl;
        std::cout << "ERROR HDMI: " << e.what() << std::endl;
        std::cout << "!! --------------------------------- !!" << std::endl;
        exit(1);
    }

}
//// ------------------------------------------------------------------------ //
void ConfigHandler::LoadVMMChannelConfig(const boost::property_tree::ptree& pt)
{
    using boost::property_tree::ptree;
    using namespace std;
    using boost::format;

    stringstream ss;

    try {
        for(int iChan = 0; iChan < 64; iChan++) {
            ss.str("");
            ss << "channel_" << format("%02i") % iChan;
            for(const auto& conf : pt.get_child("configuration")) {
                size_t find = conf.first.find(ss.str());
                if(find==string::npos) continue;

                // fill a Channel struct
                Channel chan;
                // channel number
                chan.number = iChan;
                // channel polarity type
                string polarity = conf.second.get<string>("polarity");
                chan.polarity = QString::fromStdString(polarity);
                // capacitance
                string cap = conf.second.get<string>("capacitance");
                chan.capacitance = isOn(cap);
                // leakage_current
                string leakage = conf.second.get<string>("leakage_current");
                chan.leakage_current = isOn(leakage);
                // test pulse
                string testpulse = conf.second.get<string>("test_pulse");
                chan.test_pulse = isOn(testpulse);
                // hidden mode
                string hidden = conf.second.get<string>("hidden_mode");
                chan.hidden_mode = isOn(hidden);
                // trim
                chan.trim = conf.second.get<int>("trim");
                // monitor mode
                string mon = conf.second.get<string>("monitor_mode");
                chan.monitor = isOn(mon);
                // 10bADC time set
                chan.s10bitADC = conf.second.get<int>("s10bADC_time_set");
                // 8bADC time set
                chan.s8bitADC = conf.second.get<int>("s08bADC_time_set");
                // 6bADC time set
                chan.s6bitADC = conf.second.get<int>("s06bADC_time_set");

                //chan.Print();
                m_channels.push_back(chan);

            }
            
        }
    }
    catch(std::exception &e)
    {
        std::cout << "!! --------------------------------- !!" << std::endl;
        std::cout << "ERROR : " << e.what() << std::endl;
        std::cout << "!! --------------------------------- !!" << std::endl;
        exit(1);

    }

}
//// ------------------------------------------------------------------------ //
bool ConfigHandler::isOn(std::string onOrOff)
{
    if(onOrOff=="on" || onOrOff=="enabled")
        return true;
    else if(onOrOff=="off" || onOrOff=="disabled")
        return false;
    else {
        std::cout << "Expecting either 'on'/'enabled'"
                  << " or 'off'/'disabled'. Returning 'off'/'disabled'." 
        << std::endl;
        return false;
    } 
}
//// ------------------------------------------------------------------------ //
std::string ConfigHandler::isOnOrOff(int onOrOff)
{
    using namespace std;

    if(onOrOff==1)
        return "on";
    else if(onOrOff==0)
        return "off";
    else {
        cout << "ERROR isOnOrOff expects only '0' or '1' as input." << endl;
        cout << "ERROR You have provided: " << onOrOff << endl;
        exit(1);
    }
}
//// ------------------------------------------------------------------------ //
std::string ConfigHandler::isEnaOrDis(int enaOrDis)
{
    using namespace std;

    if(enaOrDis==1)
        return "enabled";
    else if(enaOrDis==0)
        return "disabled";
    else {
        cout << "ERROR isEnaOrDis expects only '0' or '1' as input." << endl;
        cout << "ERROR You have provided: " << enaOrDis << endl;
        exit(1);
    }
}
//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  CommInfo
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
void CommInfo::Print()
{
    using namespace std;
    stringstream ss;

    ss << "------------------------------------------------------" << endl;
    ss << " UDP Settings " << endl;
    ss << "     > FEC port          : "
        << fec_port << endl;
    ss << "     > DAQ port          : "
        << daq_port << endl;
    ss << "     > VMMASIC port      : "
        << vmmasic_port << endl;
    ss << "     > VMMAPP port       : "
        << vmmapp_port << endl;
    ss << "     > S6 port           : "
        << s6_port << endl;
    ss << "------------------------------------------------------" << endl;
    ss << " General Info " << endl;
    ss << "     > Config version    : "
        << config_version.toStdString() << endl;
    ss << "     > VMM ID list       : "
        << vmm_id_list.toStdString() << endl;
    ss << "     > IP list           : "
        << ip_list.toStdString() << endl;
    ss << "     > comment           : "
        << comment.toStdString() << endl;
    ss << "     > debug             : "
        << boolalpha << debug << endl;
    ss << "------------------------------------------------------" << endl;

    cout << ss.str() << endl;
}
//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  TriggerDAQ
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
void TriggerDAQ::Print()
{
    using namespace std;
    stringstream ss;

    ss << "------------------------------------------------------" << endl;
    ss << " Trigger/DAQ Settings " << endl;
    ss << "     > TP delay              : "
        << tp_delay << endl;
    ss << "     > trigger period        : "
        << trigger_period.toStdString() << endl;
    ss << "     > acq sync              : "
        << acq_sync << endl;
    ss << "     > acq window            : "
        << acq_window << endl;
    ss << "     > run mode              : "
        << run_mode.toStdString() << endl;
    ss << "     > run count             : "
        << run_count << endl;
    ss << "     > output path           : "
        << output_path.toStdString() << endl;
    ss << "     > output name           : "
        << output_filename.toStdString() << endl;
    ss << "------------------------------------------------------" << endl;

    cout << ss.str() << endl;
}
//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  GlobalSetting
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
void GlobalSetting::Print()
{
    using namespace std;
    stringstream ss;
    ss << "------------------------------------------------------" << endl;
    ss << " Global Settings " << endl;
    ss << "     > channel polarity          : "
        << polarity.toStdString() << endl;
    ss << "     > channel leakage current   : "
        << boolalpha << leakage_current << endl;
    ss << "     > analog tristates          : "
        << boolalpha << analog_tristates << endl;
    ss << "     > double leakage            : "
        << boolalpha << double_leakage << endl;
    ss << "     > gain                      : "
        << gain.toStdString() << endl;
    ss << "     > peak time                 : "
        << peak_time << endl;
    ss << "     > neighbor trigger          : "
        << boolalpha << neighbor_trigger << endl;
    ss << "     > TAC slope adj             : "
        << tac_slope << endl;
    ss << "     > disable at peak           : "
        << boolalpha << disable_at_peak << endl;
    ss << "     > ART                       : "
        << boolalpha << art << endl;
    ss << "     > ART mode                  : "
        << art_mode.toStdString() << endl;
    ss << "     > dual clock ART            : "
        << boolalpha << dual_clock_art << endl;
    ss << "     > out buffer mo             : "
        << boolalpha << out_buffer_mo << endl;
    ss << "     > out buffer pdo            : "
        << boolalpha << out_buffer_pdo << endl;
    ss << "     > out buffer tdo            : "
        << boolalpha << out_buffer_tdo << endl;
    ss << "     > channel monitoring        : "
        << channel_monitor << endl;
    ss << "     > monitoring control        : "
        << boolalpha << monitoring_control << endl;
    ss << "     > monitor pdo out           : "
        << boolalpha << monitor_pdo_out << endl;
    ss << "     > ADCs                      : "
        << boolalpha << adcs << endl;
    ss << "     > sub hysteresis discr      : "
        << boolalpha << sub_hysteresis << endl;
    ss << "     > direct time               : "
        << boolalpha << direct_time << endl;
    ss << "     > direct time mode          : "
        << direct_time_mode.toStdString() << endl;
    ss << "     > conv mode 8bit            : "
        << boolalpha << conv_mode_8bit << endl;
    ss << "     > enable 6bit               : "
        << boolalpha << enable_6bit << endl;
    ss << "     > ADC 10bit                 : "
        << adc_10bit.toStdString() << endl;
    ss << "     > ADC 8bit                  : "
        << adc_8bit.toStdString() << endl;
    ss << "     > ADC 6bit                  : "
        << adc_6bit.toStdString() << endl;
    ss << "     > dual clock data           : "
        << boolalpha << dual_clock_data << endl;
    ss << "     > dual clock 6bit           : "
        << boolalpha << dual_clock_6bit << endl;
    ss << "     > threshold DAC             : "
        << threshold_dac << endl;
    ss << "     > test pulse DAC            : "
        << test_pulse_dac << endl;
    ss << "------------------------------------------------------" << endl;

    cout << ss.str() << endl;

}

//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  Channel
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
void Channel::Print()
{
    using namespace std; 
    stringstream ss;
    ss << "Channel " << number << endl;
    ss << "    > polarity         : " << polarity.toStdString() << endl;
    ss << "    > capacitance      : " << std::boolalpha << capacitance << endl;
    ss << "    > leakage current  : " << std::boolalpha << leakage_current << endl;
    ss << "    > test pulse       : " << std::boolalpha << test_pulse << endl;
    ss << "    > hidden mode      : " << std::boolalpha << hidden_mode << endl;
    ss << "    > trim             : " << trim << endl;
    ss << "    > monitor          : " << std::boolalpha << monitor << endl;
    ss << "    > s10bADC time set : " << s10bitADC << endl;
    ss << "    > s08bADC time set : " << s8bitADC << endl;
    ss << "    > s06bADC time set : " << s6bitADC << endl;
    cout << ss.str() << endl;

}
//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  ChannelMap
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
void ChannelMap::Print()
{
    using namespace std;
    stringstream ss;
    ss << "HDMI " << hdmi_no << endl;
    ss << "    > switch          : " << ( on ? "on" : "off") << endl;
    ss << "    > (first, second) : (" << first << ", " << second << ")" << endl;
    cout << ss.str() << endl;
}


