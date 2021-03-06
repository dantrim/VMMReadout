// vmm
#include "config_handler.h"
//#include "string_utils.h"

// std/stl
#include <exception>
#include <sstream>
using namespace std;

// boost
#include <boost/format.hpp>



//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  Allowed/Expected values for several configurables
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
const QStringList ConfigHandler::all_gains
    = {"0.5", "1.0", "3.0", "4.5", "6.0", "9.0", "12.0", "16.0"}; //mV/fC
const QList<int> ConfigHandler::all_peakTimes
    = {200, 100, 50, 25}; //ns
const QStringList ConfigHandler::all_s6TPskews
    = {"0", "3.125", "6.25", "9.375", "12.5", "15.625", "18.75", "21.875" }; //ns
const QList<int> ConfigHandler::all_TACslopes
    = {125, 250, 500, 1000}; //ns
const QStringList ConfigHandler::all_polarities
    = {"wires", "strips"};
const QStringList ConfigHandler::all_ARTmodes
    = {"threshold", "peak"};
const QStringList ConfigHandler::all_directTimeModes
    = {"TtP", "ToT", "PtP", "PtT"};
const QStringList ConfigHandler::all_ADC10bits
    = {"200ns", "+60ns"};
const QStringList ConfigHandler::all_ADC8bits
    = {"100ns", "+60ns"};
const QStringList ConfigHandler::all_ADC6bits
    = {"low", "middle", "up"};

/// s6 setting values
const QStringList ConfigHandler::all_CKTK
    = {"0", "12.5", "25"}; // ns
const QStringList ConfigHandler::all_CKBC
    = {"160", "160inv", "80", "40", "20", "10", "5", "2.5"}; // MHz
const QStringList ConfigHandler::all_CKBC_SKEW
    = {"0", "6.26", "12.52", "18.78"}; // ns

//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  ConfigHandler -- Constructor
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
ConfigHandler::ConfigHandler(QObject *parent) :
    QObject(parent),
    m_dbg(false),
    m_mmfe8(false),
    m_msg(0)
{
    m_channelmap.clear();
    m_channels.clear();
}
//// ------------------------------------------------------------------------ //
void ConfigHandler::setMMFE8(bool set_for_mmfe8)
{
    m_mmfe8 = set_for_mmfe8;
    if(dbg() && m_mmfe8) {
        msg()("Configuration set to MMFE8","ConfigHandler::setMMFE8");
    }
}
//// ------------------------------------------------------------------------ //
void ConfigHandler::LoadMessageHandler(MessageHandler& m)
{
    m_msg = &m;
}
//// ------------------------------------------------------------------------ //
bool ConfigHandler::LoadConfig(const QString &filename)
{
    bool ok = true;

    m_commSettings.config_filename = filename;

    using boost::property_tree::ptree;
    using namespace boost::property_tree::xml_parser;
    ptree pt;
    read_xml(filename.toStdString(), pt, trim_whitespace | no_comments);

    // load the comm settings (IPs, ports, etc..)
    m_commSettings = LoadCommInfo(pt);
    bool comm_ok = m_commSettings.ok;
    if(!comm_ok) {
        msg()("Problem loading CommInfo","ConfigHandler::LoadConfig");
    }

    m_globalSettings = LoadGlobalSettings(pt);
    bool global_ok = m_globalSettings.ok;
    if(!global_ok) {
        msg()("Problem loading GlobalSettings","ConfigHandler::LoadConfig");
    }

    m_daqSettings = LoadDAQConfig(pt);
    bool daq_ok = m_daqSettings.ok;
    if(!daq_ok) {
        msg()("Problem loading DAQ settings","ConfigHandler::LoadConfig");
    }

    m_channelmap = LoadHDMIChannels(pt);
    bool hdmi_ok = true;
    for(int ichan = 0; ichan < (int)m_channelmap.size(); ichan++) {
        bool chan_ok = m_channelmap.at(ichan).ok;
        if(!chan_ok) {
            stringstream message;
            message << "Problem loading channel map " << ichan;
            msg()(message,"ConfigHandler::LoadConfig");
            hdmi_ok = false;
        }
    }
    if(hdmi_ok) {
        hdmi_ok = setHDMIChannelMap();
        if(!hdmi_ok) {
            msg()("Problem loading HDMI channel map","ConfigHandler::LoadConfig");
        }
    }


    m_channels = LoadVMMChannelConfig(pt);
    bool vmmchan_ok = true;
    for(int ichan = 0; ichan < (int)m_channels.size(); ichan++) {
        bool chan_ok = m_channels.at(ichan).ok;
        if(!chan_ok) {
            stringstream message;
            message << "Problem loading channel " << ichan;
            msg()(message,"ConfigHandler::LoadConfig");
            vmmchan_ok = false;
        }
    }

    m_s6Settings = LoadS6Settings(pt);
    bool s6_ok = m_s6Settings.ok;
    if(!s6_ok)
        msg()("Problem loading s6 settings","ConfigHandler::LoadConfig");

    stringstream sx;
    if(dbg()) {
        m_commSettings.print();
        m_globalSettings.print();
        m_daqSettings.print();

        sx << "ChannelMap size        : " << m_channelmap.size () << "\n"
           << "HDMI channel map       : " << getHDMIChannelMap()  << "\n"
           << "HDMI channel map (ART) : " << getHDMIChannelMapART() << "\n"
           << "VMM channels size      : " << m_channels.size();
        msg()(sx,"ConfigHandler::LoadConfig");
    }

    return (comm_ok && global_ok && daq_ok && hdmi_ok && vmmchan_ok && s6_ok);
}
//// ------------------------------------------------------------------------ //
void ConfigHandler::WriteConfig(QString filename)
{
    using boost::format;
    using boost::property_tree::ptree;
    using namespace boost::property_tree::xml_parser;
    ptree defaultpt;
    ptree outpt;


    // read in the default configuration to ~get the structure
    string default_name = "../configs/config.xml";
    read_xml(default_name, defaultpt, trim_whitespace | no_comments);

    ptree out_root;

    // ----------------------------------------------- //
    //  udp_setup
    // ----------------------------------------------- //
    CommInfo def_commInfo = LoadCommInfo(defaultpt);

    ptree out_udpsetup;
    out_udpsetup.put("fec_port", commSettings().fec_port);
    out_udpsetup.put("daq_port", commSettings().daq_port);
    out_udpsetup.put("vmmasic_port", commSettings().vmmasic_port);
    out_udpsetup.put("vmmapp_port", commSettings().vmmapp_port);
    out_udpsetup.put("s6_port", commSettings().s6_port);

    // ----------------------------------------------- //
    // trigger_daq
    // ----------------------------------------------- //
    TriggerDAQ def_tdaq = LoadDAQConfig(defaultpt);

    ptree out_tdaq;
    out_tdaq.put("tp_delay", daqSettings().tp_delay);
    out_tdaq.put("trigger_period", daqSettings().trigger_period);
    out_tdaq.put("acq_sync", daqSettings().acq_sync);
    out_tdaq.put("acq_window", daqSettings().acq_window);
    out_tdaq.put("bcid_reset", daqSettings().bcid_reset);
    out_tdaq.put("ignore16", isOnOrOff(daqSettings().ignore16));
    out_tdaq.put("hold_off", isOnOrOff(daqSettings().enable_holdoff));
    string outpath = (daqSettings().output_path != "" ? daqSettings().output_path :
                                def_tdaq.output_path);
    out_tdaq.put("output_path", outpath);

    // ----------------------------------------------- //
    //  hdmi channel map
    // ----------------------------------------------- //
    std::vector<ChannelMap> def_hdmichan = LoadHDMIChannels(defaultpt);

    ptree out_hdmi;
    stringstream ss;
    stringstream val;
    for(int iHDMI = 1; iHDMI < 9; iHDMI++) {
        ss.str("");
        ss << "hdmi_" << format("%1i") % iHDMI;

        //switch
        val.str("");
        val << ss.str() << ".switch";
        out_hdmi.put(val.str(), isOnOrOff((int)hdmiChannelSettings(iHDMI-1).on));
        //first
        val.str("");
        val << ss.str() << ".first";
        out_hdmi.put(val.str(), (int)hdmiChannelSettings(iHDMI-1).first);
        //second
        val.str("");
        val << ss.str() << ".second";
        out_hdmi.put(val.str(), (int)hdmiChannelSettings(iHDMI-1).second);
        //art
        val.str("");
        val << ss.str() << ".art";
        out_hdmi.put(val.str(), (int)hdmiChannelSettings(iHDMI-1).art);
    } // iHDMI

    // ----------------------------------------------- //
    //  global settings
    // ----------------------------------------------- //
    GlobalSetting def_global = LoadGlobalSettings(defaultpt);

    ptree out_global;
    out_global.put("ch_polarity",
        ConfigHandler::all_polarities[globalSettings().polarity].toStdString());
    out_global.put("ch_leakage_current",
        isEnaOrDis(globalSettings().leakage_current));
    out_global.put("analog_tristates",
        isOnOrOff(globalSettings().analog_tristates));
    out_global.put("double_leakage",
        isOnOrOff(globalSettings().double_leakage));
    out_global.put("gain",
        ConfigHandler::all_gains[globalSettings().gain].toStdString());
    out_global.put("peak_time",
        ConfigHandler::all_peakTimes[globalSettings().peak_time]);
    out_global.put("neighbor_trigger",
        isOnOrOff(globalSettings().neighbor_trigger));
    out_global.put("TAC_slop_adj",
        ConfigHandler::all_TACslopes[globalSettings().tac_slope]);
    out_global.put("disable_at_peak",
        isOnOrOff(globalSettings().disable_at_peak));
    out_global.put("ART",
        isOnOrOff(globalSettings().art));
    out_global.put("ART_mode",
        ConfigHandler::all_ARTmodes[globalSettings().art_mode].toStdString());
    out_global.put("dual_clock_ART",
        isOnOrOff(globalSettings().dual_clock_art));
    out_global.put("out_buffer_mo",
        isOnOrOff(globalSettings().out_buffer_mo));
    out_global.put("out_buffer_pdo",
        isOnOrOff(globalSettings().out_buffer_pdo));
    out_global.put("out_buffer_tdo",
        isOnOrOff(globalSettings().out_buffer_tdo));
    out_global.put("channel_monitoring",
        globalSettings().channel_monitor);
    out_global.put("monitoring_control",
        isOnOrOff(globalSettings().monitoring_control));
    out_global.put("monitor_pdo_out",
        isOnOrOff(globalSettings().monitor_pdo_out));
    out_global.put("ADCs",
        isOnOrOff(globalSettings().adcs));
    out_global.put("sub_hyst_discr",
        isOnOrOff(globalSettings().sub_hysteresis));
    out_global.put("direct_time",
        isOnOrOff(globalSettings().direct_time));
    out_global.put("direct_time_mode",
        ConfigHandler::all_directTimeModes[globalSettings().direct_time_mode].toStdString());
    out_global.put("conv_mode_8bit",
        isOnOrOff(globalSettings().conv_mode_8bit));
    out_global.put("enable_6bit",
        isOnOrOff(globalSettings().enable_6bit));
    out_global.put("ADC_10bit",
        ConfigHandler::all_ADC10bits[globalSettings().adc_10bit].toStdString());
    out_global.put("ADC_8bit",
        ConfigHandler::all_ADC8bits[globalSettings().adc_8bit].toStdString());
    out_global.put("ADC_6bit",
        ConfigHandler::all_ADC6bits[globalSettings().adc_6bit].toStdString());
    out_global.put("dual_clock_data",
        isOnOrOff(globalSettings().dual_clock_data));
    out_global.put("dual_clock_6bit",
        isOnOrOff(globalSettings().dual_clock_6bit));
    out_global.put("threshold_DAC",
        globalSettings().threshold_dac);
    out_global.put("test_pulse_DAC",
        globalSettings().test_pulse_dac);

    // ----------------------------------------------- //
    //  S6 settings
    // ----------------------------------------------- //
    s6Setting def_s6 = LoadS6Settings(defaultpt);

    ptree out_s6;
    out_s6.put("cktk",
        ConfigHandler::all_CKTK[s6Settings().cktk].toStdString());
    out_s6.put("ckbc",
        ConfigHandler::all_CKBC[s6Settings().ckbc].toStdString());
    out_s6.put("ckbc_skew",
        ConfigHandler::all_CKBC_SKEW[s6Settings().ckbc_skew].toStdString());
    out_s6.put("vmm_auto_reset",
        isOnOrOff(s6Settings().do_auto_reset));
    out_s6.put("fec_reset",
        isOnOrOff(s6Settings().do_fec_reset));
    out_s6.put("tk_pulses",
        s6Settings().tk_pulses);
    out_s6.put("fec_reset_period",
        s6Settings().fec_reset_period);
   
    // ----------------------------------------------- //
    // stitch together the fields
    // ----------------------------------------------- //
    out_root.add_child("udp_setup", out_udpsetup);
    out_root.add_child("trigger_daq", out_tdaq);
    out_root.add_child("s6_settings", out_s6);
    out_root.add_child("channel_map", out_hdmi);
    out_root.add_child("global_settings", out_global);


    // ----------------------------------------------- //
    //  individual VMM channels
    // ----------------------------------------------- //
    std::vector<Channel> def_vmmchan = LoadVMMChannelConfig(defaultpt);

    for(int i = 0; i < 64; i++) {
        ptree out_channels;
        if((int)channelSettings(i).number != i) {
            stringstream sx;
            sx << "ERROR VMM Channel numbers out of sync! Expecting VMM channel "
               << i << " but at this index we have VMM " << channelSettings(i).number;
            msg()(sx, "ConfigHandler::WriteConfig", true);
           // exit(1);
        }

        ss.str("");
        ss << "channel_" << format("%02i") % i;

        //polarity (SP)
        out_channels.put("polarity",
            ConfigHandler::all_polarities[channelSettings(i).polarity].toStdString());

        //capacitance (SC)
        out_channels.put("capacitance",
            isOnOrOff(channelSettings(i).capacitance));

        //leakage_current (SL)
        out_channels.put("leakage_current",
            isEnaOrDis(channelSettings(i).leakage_current));

        //test_pulse (ST)
        out_channels.put("test_pulse",
            isOnOrOff(channelSettings(i).test_pulse));

        //hidden_mode (SM)
        out_channels.put("hidden_mode",
            isOnOrOff(channelSettings(i).hidden_mode));

        //trim
        out_channels.put("trim",
            channelSettings(i).trim);

        //monitor
        out_channels.put("monitor",
            isOnOrOff(channelSettings(i).monitor));

        //s10bADC
        out_channels.put("s10bADC_time_set",
            channelSettings(i).s10bitADC);

        //s08bADC_time_set
        out_channels.put("s08bADC_time_set",
            channelSettings(i).s8bitADC);

        //s06bADC_time_set
        out_channels.put("s06bADC_time_set",
            channelSettings(i).s6bitADC);

        out_root.add_child(ss.str(), out_channels);
        
    }

    // put everything under a global node
    outpt.add_child("configuration", out_root);


    #if BOOST_VERSION >= 105800
    write_xml(filename.toStdString(), outpt, std::locale(),
            boost::property_tree::xml_writer_make_settings<std::string>('\t',1));
    #else
    write_xml(filename.toStdString(), outpt, std::locale(),
            boost::property_tree::xml_writer_make_settings<char>('\t',1));
    #endif

}
//// ------------------------------------------------------------------------ //
CommInfo ConfigHandler::LoadCommInfo(const boost::property_tree::ptree& pt)
{
    using boost::property_tree::ptree;

    CommInfo comm;

    try
    {
        for(const auto& conf : pt.get_child("configuration")) {
            if(!(conf.first == "udp_setup"))
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

                comm.ok = true;
            } // udp_setup
        }
    }
    catch(std::exception &e)
    {
        stringstream sx;
        sx << "!! -------------------------------------------- !! \n"
           << "  ERROR CONF: " << e.what() << "\n"
           << "!! -------------------------------------------- !!"; 
        msg()(sx,"ConfigHandler::LoadCommInfo"); 
        comm.ok = false;
    }

    return comm;
}
//// ------------------------------------------------------------------------ //
void ConfigHandler::LoadCommInfo(const CommInfo& info)
{
    m_commSettings = info;
    m_commSettings.print();
}
//// ------------------------------------------------------------------------ //
GlobalSetting ConfigHandler::LoadGlobalSettings(const boost::property_tree::ptree& pt)
{
    using boost::property_tree::ptree;

    GlobalSetting g;

    bool outok = true;

    try
    {
        for(const auto& conf : pt.get_child("configuration")) {
            if( !(conf.first == "global_settings") ) continue;

            // ------------------------------------------------------------- //
            // global channel polarity
            string pol = conf.second.get<string>("ch_polarity"); 
            if(all_polarities.indexOf(QString::fromStdString(pol))>=0)
                g.polarity = all_polarities.indexOf(QString::fromStdString(pol));
            else {
                stringstream sx;
                sx << "ERROR ch_polarity value must be one of: [";
                for(auto& i : all_polarities) sx << " " << i.toStdString() << " ";
                sx << "]\nYou have provided: " << pol;
                msg()(sx,"ConfigHandler::LoadGlobalSettings");
                outok = false;
            }

            // ------------------------------------------------------------- //
            // global channel leakage current ena/dis
            string leak = conf.second.get<string>("ch_leakage_current");
            g.leakage_current = (bool)isOn(leak, "ch_leakage_current");
       
            // ------------------------------------------------------------- //
            // analog tristates
            string tristates = conf.second.get<string>("analog_tristates");
            g.analog_tristates = isOn(tristates, "analog_tristates");

            // ------------------------------------------------------------- //
            // double leakage
            string dleak = conf.second.get<string>("double_leakage");
            g.double_leakage = isOn(dleak, "double_leakage");

            // ------------------------------------------------------------- //
            // gain
            string gain = conf.second.get<string>("gain");
            if(all_gains.indexOf(QString::fromStdString(gain))>=0)
                g.gain = all_gains.indexOf(QString::fromStdString(gain));
            else {
                stringstream sx;
                sx << "ERROR gain value must be one of: [";
                for(auto& i : all_gains)  sx << " " << i.toStdString() << " ";
                sx << "] mV/fC\nYou have provided: " << gain;
                msg()(sx,"ConfigHandler::LoadGlobalSettings");
                outok = false;
            }
            
            // ------------------------------------------------------------- //
            // peak_time
            int ptime = conf.second.get<int>("peak_time");
            if(all_peakTimes.indexOf(ptime)>=0)
                g.peak_time = all_peakTimes.indexOf(ptime);
            else {
                stringstream sx;
                sx << "ERROR peak_time value must be one of: [";
                for(auto& i : all_peakTimes) sx << " " << i << " ";
                sx << "] ns\nYou have provided: " << ptime;
                msg()(sx,"ConfigHandler::LoadGlobalSettings");
                outok = false;
            }

            // ------------------------------------------------------------- //
            // neighbor trigger
            string neighbor = conf.second.get<string>("neighbor_trigger");
            g.neighbor_trigger = isOn(neighbor, "neighbor_trigger");

            // ------------------------------------------------------------- //
            // TAC slope
            int tacslope = conf.second.get<int>("TAC_slop_adj");
            if(all_TACslopes.indexOf(tacslope)>=0)
                g.tac_slope = all_TACslopes.indexOf(tacslope);
            else {
                stringstream sx;
                sx << "ERROR TAC_slope_adj value must be one of: [";
                for(auto& i : all_TACslopes) sx << " " << i << " ";
                sx << "] ns\nYou have provided: " << tacslope;
                msg()(sx,"ConfigHandler::LoadGlobalSettings");
                outok = false;
            }

            // ------------------------------------------------------------- //
            // disable at peak
            string disable = conf.second.get<string>("disable_at_peak");
            g.disable_at_peak = isOn(disable, "disable_at_peak");

            // ------------------------------------------------------------- //
            // ART
            string art = conf.second.get<string>("ART");
            g.art = isOn(art, "ART");

            // ------------------------------------------------------------- //
            // ART mode
            string artmode = conf.second.get<string>("ART_mode");
            if( all_ARTmodes.indexOf(QString::fromStdString(artmode))>=0) {
                QString mode = QString::fromStdString(artmode);
                g.art_mode = all_ARTmodes.indexOf(mode);
            }
            else {
                stringstream sx;
                sx << "ERROR ART_mode value must be one of: [";
                for(auto& i : all_ARTmodes) sx << " " << i.toStdString() << " ";
                sx << "]\nYou have provided: " << artmode;
                msg()(sx,"ConfigHandler::LoadGlobalSettings");
                outok = false;
            }

            // ------------------------------------------------------------- //
            // dual clock ART
            string dcart = conf.second.get<string>("dual_clock_ART");
            g.dual_clock_art = isOn(dcart, "dual_clock_ART");

            // ------------------------------------------------------------- //
            // out buffer mo
            string obmo = conf.second.get<string>("out_buffer_mo");
            g.out_buffer_mo = isOn(obmo, "out_buffer_mo");

            // ------------------------------------------------------------- //
            // out buffer pdo
            string obpdo = conf.second.get<string>("out_buffer_pdo");
            g.out_buffer_pdo = isOn(obpdo, "out_buffer_pdo");

            // ------------------------------------------------------------- //
            // out buffer tdo
            string obtdo = conf.second.get<string>("out_buffer_tdo");
            g.out_buffer_tdo = isOn(obtdo, "out_buffer_tdo");

            // ------------------------------------------------------------- //
            // channel for monitoring
            g.channel_monitor = conf.second.get<int>("channel_monitoring");

            // ------------------------------------------------------------- //
            // monitoring control
            string mcontrol = conf.second.get<string>("monitoring_control");
            g.monitoring_control = isOn(mcontrol, "monitoring_control");

            // ------------------------------------------------------------- //
            // monitor pdo out
            string mpdo = conf.second.get<string>("monitor_pdo_out");
            g.monitor_pdo_out = isOn(mpdo, "monitor_pdo_out");

            // ------------------------------------------------------------- //
            // ADCs
            string adcs = conf.second.get<string>("ADCs");
            g.adcs = isOn(adcs, "ADCs");

            // ------------------------------------------------------------- //
            // sub hysteresis
            string subhyst = conf.second.get<string>("sub_hyst_discr");
            g.sub_hysteresis = isOn(subhyst, "sub_hyst_discr");

            // ------------------------------------------------------------- //
            // direct time
            string direct = conf.second.get<string>("direct_time");
            g.direct_time = isOn(direct, "direct_time");

            // ------------------------------------------------------------- //
            // direct time mode
            string dtmode = conf.second.get<string>("direct_time_mode");
            if(all_directTimeModes.indexOf(QString::fromStdString(dtmode))>=0)
            {
                QString mode = QString::fromStdString(dtmode);
                int dtindex = all_directTimeModes.indexOf(mode);
                QString tmp = QString("%1").arg(dtindex, 2, 2, QChar('0'));
                g.direct_time_mode  = dtindex;
                g.direct_time_mode0 = tmp.at(0).digitValue();
                g.direct_time_mode1 = tmp.at(1).digitValue();
            }
            else {
                stringstream sx;
                sx << "ERROR direct_time_mode value must be one of: [";
                for(auto& i : all_directTimeModes) sx <<" "<< i.toStdString()<<" ";
                sx << "]\nYou have provided: " << dtmode;
                msg()(sx,"ConfigHandler::LoadGlobalSettings");
                outok = false;
            } 

            // ------------------------------------------------------------- //
            // conv mode 8bit
            string cmode8bit = conf.second.get<string>("conv_mode_8bit");
            g.conv_mode_8bit = isOn(cmode8bit, "conv_mode_8bit");

            // ------------------------------------------------------------- //
            // enable 6bit
            string ena6 = conf.second.get<string>("enable_6bit");
            g.enable_6bit = isOn(ena6, "enable_6bit");

            // ------------------------------------------------------------- //
            // ADC 10bit
            string adc10 = conf.second.get<string>("ADC_10bit");
            if(all_ADC10bits.indexOf(QString::fromStdString(adc10))>=0)
                g.adc_10bit = all_ADC10bits.indexOf(QString::fromStdString(adc10));
            else {
                stringstream sx;
                sx << "ERROR ADC_10bit value must be one of: [";
                for(auto& i : all_ADC10bits) sx << " " << i.toStdString() << " ";
                sx << "]\nYou have provided: " << adc10;
                msg()(sx,"ConfigHandler::LoadGlobalSettings");
                outok = false;
            }

            // ------------------------------------------------------------- //
            // ADC 8bit
            string adc8 = conf.second.get<string>("ADC_8bit");
            if(all_ADC8bits.indexOf(QString::fromStdString(adc8))>=0)
                g.adc_8bit = all_ADC8bits.indexOf(QString::fromStdString(adc8));
            else {
                stringstream sx;
                sx << "ERROR ADC_8bit value must be one of: [";
                for(auto& i : all_ADC8bits) sx << " " << i.toStdString() << " ";
                sx << "]\nYou have provided: " << adc8;
                msg()(sx,"ConfigHandler::LoadGlobalSettings");
                outok = false;
            }

            // ------------------------------------------------------------- //
            // ADC 6bit
            string adc6 = conf.second.get<string>("ADC_6bit");
            if(all_ADC6bits.indexOf(QString::fromStdString(adc6))>=0)
                g.adc_6bit = all_ADC6bits.indexOf(QString::fromStdString(adc6));
            else {
                stringstream sx;
                sx << "ERROR ADC_6bit value must be one of: [";
                for(auto& i : all_ADC6bits) sx << " " << i.toStdString() << " ";
                sx << "]\nYou have provided: " << adc6;
                msg()(sx,"ConfigHandler::LoadGlobalSettings");
                outok = false;
            }

            // ------------------------------------------------------------- //
            // dual clock data
            string dcdata = conf.second.get<string>("dual_clock_data");
            g.dual_clock_data = isOn(dcdata, "dual_clock_data");

            // ------------------------------------------------------------- //
            // dual clock 6bit
            string dc6 = conf.second.get<string>("dual_clock_6bit");
            g.dual_clock_6bit = isOn(dc6, "dual_clock_6bit");

            // ------------------------------------------------------------- //
            // threshold DAC
            g.threshold_dac = conf.second.get<int>("threshold_DAC");
            
            // ------------------------------------------------------------- //
            // test pulse DAC
            g.test_pulse_dac = conf.second.get<int>("test_pulse_DAC");
        }
    }
    catch(std::exception &e)
    {
        stringstream sx;
        sx << "!! -------------------------------------------- !! \n"
           << "  ERROR GLOBAL: " << e.what() << "\n"
           << "!! -------------------------------------------- !!"; 
        msg()(sx,"ConfigHandler::LoadCommInfo"); 
        outok = false;
    }
    g.ok = outok;

    return g;
}
//// ------------------------------------------------------------------------ //
TriggerDAQ ConfigHandler::LoadDAQConfig(const boost::property_tree::ptree& pt)
{
    using boost::property_tree::ptree;

    TriggerDAQ daq;

    bool outok = true;

    try
    {
        for(const auto& conf : pt.get_child("configuration")) {
            if( !(conf.first == "trigger_daq") ) continue;

            // tp delay
            daq.tp_delay = conf.second.get<int>("tp_delay");
            // trigger period
            string tperiod = conf.second.get<string>("trigger_period");
            daq.trigger_period = tperiod;
            // acq sync
            daq.acq_sync = conf.second.get<int>("acq_sync");
            // acq window
            daq.acq_window = conf.second.get<int>("acq_window");
            // bcid reset
            daq.bcid_reset = conf.second.get<int>("bcid_reset");
            // ignore16 flag
            daq.ignore16 = isOn(conf.second.get<string>("ignore16"), "ignore16");
            // hold off
            daq.enable_holdoff = isOn(conf.second.get<string>("hold_off"), "hold_off");
            // output path
            string opath = conf.second.get<string>("output_path");
            daq.output_path = opath;

        }
    }
    catch(std::exception &e)
    {
        stringstream sx;
        sx << "!! --------------------------------- !!\n"
           << "    ERROR DAQ: " << e.what() << "\n"
           << "!! --------------------------------- !!";
        msg()(sx, "ConfigHandler::LoadDAQConfig"); 
        outok = false;
    }

    daq.ok = outok;

    return daq;
}
//// ------------------------------------------------------------------------ //
std::vector<ChannelMap> ConfigHandler::LoadHDMIChannels(const boost::property_tree::ptree& pt)
{
    using boost::property_tree::ptree;
    using boost::format;

    // clear the channelmap vector
    m_channelmap.clear();
    std::vector<ChannelMap> outmap;

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
                // art on/off
                val.str("");
                val << ss.str() << ".art";
                chMap.second = bool(conf.second.get<int>(val.str()));

                if(dbg())
                    chMap.print();
                chMap.ok = true;
                outmap.push_back(chMap);

            } // iHDMI
        }
    }
    catch(std::exception &e)
    {
        stringstream sx;
        sx << "!! --------------------------------- !!\n"
           << "ERROR HDMI: " << e.what() << "\n"
           << "!! --------------------------------- !!"; 
        msg()(sx, "ConfigHandler::LoadHDMIChannels");
        for(int ichan = 0; ichan < outmap.size(); ichan++) {
            outmap.at(ichan).ok = false;
        }
    }
    return outmap;
}
//// ------------------------------------------------------------------------ //
bool ConfigHandler::setHDMIChannelMap()
{
    bool outok = true;

    stringstream sx;
    if(m_channelmap.size()==0) {
        sx.str("");
        sx << "ERROR Channel map vector is empty!";
        msg()(sx, "ConfigHandler::setHDMIChannelMap");
        outok = false;
    }
    if(m_channelmap.size()!=8) {
        sx.str("");
        sx << "ERROR Channel map vector must have 8 entries.\n"
           << "      Current vector has " << m_channelmap.size() << " entries.";
        msg()(sx, "ConfigHandler::setHDMIChannelMap");
        outok = false;
    }
    ///////////////////////////////////////////////////////////////////
    // build the channel (VMMID) map
    ///////////////////////////////////////////////////////////////////
    QString chMapString = "";
    bool ok;
    //addmmfe8
    if(!mmfe8()) {
        chMapString = "000000000000000000000000"; //24bit (16 vmm + 8 art)
        for(int i = 0; i < (int)m_channelmap.size(); ++i) {
            QString first, second, art;
            first  = m_channelmap[i].first ? "1" : "0";
            second = m_channelmap[i].second ? "1" : "0";
            art    = m_channelmap[i].art ? "1" : "0";
            if(m_channelmap[i].on) {
                chMapString.replace(23-2*i, 1, first);
                chMapString.replace(22-2*i, 1, second);
                chMapString.replace(7-i, 1, art);

//                chMapString.replace(15-2*i, 1, first);
//                chMapString.replace(14-2*i, 1, second);
            }
        } // i
        m_channelMap =    (quint16)chMapString.toInt(&ok,2);
        m_channelMapART = (quint32)chMapString.toInt(&ok,2); 
    } // mini2
    ///////////////////////////////////////////////////////////////////
    // build the channel (VMMID) map
    ///////////////////////////////////////////////////////////////////
    else if(mmfe8()) {
        chMapString = "";
        for(int i = 0; i < (int)m_channelmap.size(); ++i) {
            if(m_channelmap[i].on) {
                chMapString.append(QString::number(i+1));
                break;
            }
        } // i
        m_channelMap    = (quint16)chMapString.toInt(&ok,10);
        m_channelMapART = (quint32)0;
    }

    return outok;
}
//// ------------------------------------------------------------------------ //
std::vector<Channel> ConfigHandler::LoadVMMChannelConfig(const boost::property_tree::ptree& pt)
{
    using boost::property_tree::ptree;
    using boost::format;

    // clear the current channels
    m_channels.clear();
    std::vector<Channel> channels;

    stringstream ss;
    stringstream where;
    stringstream sx;

    try {
        for(int iChan = 0; iChan < 64; iChan++) {
            bool channel_ok = true;
            ss.str("");
            ss << "channel_" << format("%02i") % iChan;
            for(const auto& conf : pt.get_child("configuration")) {
                size_t find = conf.first.find(ss.str());
                if(find==string::npos) continue;
                where.str("");
                where << "(ch. " << iChan << ")";
                string value = "";

                // fill a Channel struct
                Channel chan;
                // channel number
                chan.number = iChan;
                // channel polarity type
                string polarity = conf.second.get<string>("polarity");
                if(all_polarities.indexOf(QString::fromStdString(polarity))>=0) {
                    QString pol = QString::fromStdString(polarity);
                    chan.polarity = all_polarities.indexOf(pol);
                }
                else {
                    sx.str("");
                    sx << "ERROR polarity value for channel " << iChan << " "
                         << " must be one of: [";
                    for(auto& i : all_polarities) sx <<" "<<i.toStdString()<<" ";
                    sx << "]\nYou have provided: " << polarity;
                    msg()(sx,"ConfigHandler::LoadVMMChannelConfig");
                    channel_ok = false;
                }
                // capacitance
                string cap = conf.second.get<string>("capacitance");
                value = "capacitance " + where.str();
                chan.capacitance = (bool)isOn(cap, value);
                // leakage_current
                string leakage = conf.second.get<string>("leakage_current");
                value = "leakage_current " + where.str();
                chan.leakage_current = (bool)isOn(leakage, value);
                // test pulse
                string testpulse = conf.second.get<string>("test_pulse");
                value = "test_pulse " + where.str();
                chan.test_pulse = (bool)isOn(testpulse, value);
                // hidden mode
                string hidden = conf.second.get<string>("hidden_mode");
                value = "hidden_mode " + where.str();
                chan.hidden_mode = (bool)isOn(hidden, value);
                // trim
                chan.trim = conf.second.get<int>("trim");
                #warning what values can channel trim take?
                // monitor mode
                string mon = conf.second.get<string>("monitor_mode");
                value = "monitor_mode " + where.str();
                chan.monitor = (bool)isOn(mon, value);
                // 10bADC time set
                #warning what values can the ADC time sets take?
                chan.s10bitADC = conf.second.get<int>("s10bADC_time_set");
                // 8bADC time set
                chan.s8bitADC = conf.second.get<int>("s08bADC_time_set");
                // 6bADC time set
                chan.s6bitADC = conf.second.get<int>("s06bADC_time_set");

                if(dbg())
                    chan.print();
                chan.ok = channel_ok;
                channels.push_back(chan);
            }
        }
    }
    catch(std::exception &e)
    {
        sx.str("");
        sx << "!! --------------------------------- !!\n"
           << "ERROR VMM Channel: " << e.what() << "\n"
           << "!! --------------------------------- !!";
        msg()(sx,"ConfigHandler::LoadVMMChannelConfig");
        for(int ichan = 0; ichan < channels.size(); ichan++) {
            channels.at(ichan).ok = false;
        }
    }
    return channels;
}
//// ------------------------------------------------------------------------ //
s6Setting ConfigHandler::LoadS6Settings(const boost::property_tree::ptree& pt)
{
    using boost::property_tree::ptree;
    s6Setting s6;
    bool outok = true;

    try
    {
        for(const auto& conf : pt.get_child("configuration")) {
            if(!(conf.first == "s6_settings")) continue;

            /////////////////////////////////////////////////
            // cktk period
            string cktk_in = conf.second.get<string>("cktk");
            if(all_CKTK.indexOf(QString::fromStdString(cktk_in))>=0)
                s6.cktk = all_CKTK.indexOf(QString::fromStdString(cktk_in));
            else {
                stringstream sx;
                sx << "ERROR cktk value must be one of: [";
                for(auto& i : all_CKTK) sx << " " << i.toStdString() << " ";
                sx << "]\nYou have provided: " << cktk_in;
                msg()(sx,"ConfigHandler::LoadS6Settings");
                outok = false;
            }
            /////////////////////////////////////////////////
            // ckbc frequency
            string ckbc_in = conf.second.get<string>("ckbc");
            if(all_CKBC.indexOf(QString::fromStdString(ckbc_in))>=0)
                s6.ckbc = all_CKBC.indexOf(QString::fromStdString(ckbc_in));
            else {
                stringstream sx;
                sx << "ERROR ckbc value must be one of: [";
                for(auto& i : all_CKBC) sx << " " << i.toStdString() << " ";
                sx << "]\nYou have provided: " << ckbc_in;
                msg()(sx,"ConfigHandler::LoadS6Settings");
                outok = false;
            }
            /////////////////////////////////////////////////
            // ckbc skew
            string ckbc_skew_in = conf.second.get<string>("ckbc_skew");
            if(all_CKBC_SKEW.indexOf(QString::fromStdString(ckbc_skew_in))>=0)
                s6.ckbc_skew = all_CKBC_SKEW.indexOf(QString::fromStdString(ckbc_skew_in));
            else {
                stringstream sx;
                sx << "ERROR ckbc_skew value must be one of: [";
                for(auto& i : all_CKBC_SKEW) sx << " " << i.toStdString() << " ";
                sx << "]\nYou have provided: " << ckbc_skew_in;
                msg()(sx,"ConfigHandler::LoadS6Settings");
                outok = false;
            }
            /////////////////////////////////////////////////
            // vmm auto reset
            string auto_reset = conf.second.get<string>("vmm_auto_reset");
            s6.do_auto_reset = isOn(auto_reset, "vmm_auto_reset");
            /////////////////////////////////////////////////
            // fec period reset
            string fec_reset = conf.second.get<string>("fec_reset");
            s6.do_fec_reset = isOn(fec_reset, "fec_reset");
            /////////////////////////////////////////////////
            // tk pulses before reset
            s6.tk_pulses = conf.second.get<int>("tk_pulses");
            /////////////////////////////////////////////////
            // fec reset period
            s6.fec_reset_period = conf.second.get<int>("fec_reset_period");
            
        } // conf

    } // try
    catch(std::exception &e)
    {
        stringstream sx;
        sx << "!! --------------------------------- !!\n"
           << "    ERROR S6: " << e.what() << "\n"
           << "!! --------------------------------- !!";
        msg()(sx, "ConfigHandler::LoadS6Settings");
        outok = false;
    } // catch
    s6.ok = outok;

    return s6;
}
//// ------------------------------------------------------------------------ //
void ConfigHandler::LoadBoardConfiguration(GlobalSetting& global,
                std::vector<ChannelMap>& chMap, std::vector<Channel>& channels)
{
    m_globalSettings = global;
 //   bool ok;
 //   QString tmp = "";
 //   tmp.append(QString::number(global.direct_time_mode0));
 //   tmp.append(QString::number(global.direct_time_mode1));
 //   m_globalSettings.direct_time_mode = tmp.toUInt(&ok,2);
    
    m_channelmap.clear();
    for(int i = 0; i < (int)chMap.size(); i++) {
        m_channelmap.push_back(chMap[i]);
        m_channelmap[i].print();
    }
    // build the HDMI channel map bit word
    setHDMIChannelMap();

    m_channels.clear();
    for(int i = 0; i < (int)channels.size(); i++) {
        m_channels.push_back(channels[i]);
        m_channels[i].print();
    }
    m_globalSettings.print();
}
//// ------------------------------------------------------------------------ //
void ConfigHandler::LoadTDAQConfiguration(TriggerDAQ& daq)
{
    m_daqSettings = daq; 
    m_daqSettings.print();
}

void ConfigHandler::LoadS6Configuration(s6Setting& s6)
{
    m_s6Settings = s6;
    m_s6Settings.print();
}
//// ------------------------------------------------------------------------ //
int ConfigHandler::isOn(std::string onOrOff, std::string where)
{
    stringstream sx;

    #warning SEE WHY LOGIC IS REVERSED FOR ENA/DIS
    if(onOrOff=="on" || onOrOff=="disabled")
    //if(onOrOff=="on" || onOrOff=="enabled")
        return 1;
    else if(onOrOff=="off" || onOrOff=="enabled")
    //else if(onOrOff=="off" || onOrOff=="disabled")
        return 0;
    else {
        std::string from = "";
        if(where!="") from = " [for " + where + "]";
        sx << "Expecting either 'on'/'enabled' or 'off'/'disabled'" << from << ".\n"
           << ">>> Returning 'off'/'disabled'.";
        msg()(sx,"ConfigHandler::isOn");
        return 0;
    } 
}
//// ------------------------------------------------------------------------ //
std::string ConfigHandler::isOnOrOff(int onOrOff)
{
    stringstream sx;

    if(onOrOff==1)
        return "on";
    else if(onOrOff==0)
        return "off";
    else {
        sx << "ERROR Expect only '0' or '1' as input. You have provided: " << onOrOff;
        msg()(sx,"ConfigHandler::isOnOrOff");
    }
}
//// ------------------------------------------------------------------------ //
std::string ConfigHandler::isEnaOrDis(int enaOrDis)
{
    stringstream sx;
    #warning SEE WHY LOGIC IS REVERSED FOR ENA/DIS
    if(enaOrDis==0)
    //if(enaOrDis==1)
        return "enabled";
    else if(enaOrDis==1)
    //else if(enaOrDis==0)
        return "disabled";
    else {
        sx << "ERROR Expect only '0' or '1' as input. You have provided: " << enaOrDis;
        msg()(sx,"ConfigHandler::isEnaOrDis",true);
        exit(1);
    }
}

//// ------------------------------------------------------------------------ //

//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  CommInfo
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
CommInfo::CommInfo() :
    fec_port(6007),
    daq_port(6006),
    vmmasic_port(6603),
    vmmapp_port(6600),
    s6_port(6602),
    config_filename(""),
    ip_list(""),
    ok(false)
{
}
// ------------------------------------------------------------------------ //
void CommInfo::print()
{
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
    ss << "     > IP list           : "
        << ip_list.toStdString()  << endl;
    ss << "------------------------------------------------------" << endl;

    cout << ss.str() << endl;
}
//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  TriggerDAQ
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
TriggerDAQ::TriggerDAQ() :
    tp_delay(81),
    trigger_period("61A80"),
    acq_sync(100),
    acq_window(4096),
    ignore16(0),
    output_path(""),
    bcid_reset(0),
    enable_holdoff(0),
    ok(false)
{
}
void TriggerDAQ::print()
{
    stringstream ss;

    ss << "------------------------------------------------------" << endl;
    ss << " Trigger/DAQ Settings " << endl;
    ss << "     > TP delay              : "
        << tp_delay << endl;
    ss << "     > trigger period        : "
        << trigger_period << endl;
    ss << "     > acq sync              : "
        << acq_sync << endl;
    ss << "     > acq window            : "
        << acq_window << endl;
    ss << "     >> bcid reset           : "
        << bcid_reset << endl;
    ss << "     > ignore16              : "
        << ignore16 << endl;
    ss << "     > enable holdoff        : "
        << enable_holdoff << endl;
    ss << "     > output path           : "
        << output_path << endl;
    ss << "------------------------------------------------------" << endl;

    cout << ss.str() << endl;
}
//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  GlobalSetting
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
GlobalSetting::GlobalSetting() :
    polarity(0),
    leakage_current(1),
    analog_tristates(0),
    double_leakage(1),
    gain(2),
    peak_time(0),
    neighbor_trigger(0),
    tac_slope(0),
    disable_at_peak(0),
    art(1),
    art_mode(0),
    dual_clock_art(0),
    out_buffer_mo(0),
    out_buffer_pdo(0),
    out_buffer_tdo(0),
    channel_monitor(0),
    monitoring_control(1),
    monitor_pdo_out(0),
    adcs(1),
    sub_hysteresis(0),
    direct_time(0),
    direct_time_mode(1),
    direct_time_mode0(0),
    direct_time_mode1(1),
    conv_mode_8bit(1),
    enable_6bit(0),
    adc_10bit(0),
    adc_8bit(0),
    adc_6bit(0),
    dual_clock_data(0),
    dual_clock_6bit(0),
    threshold_dac(200),
    test_pulse_dac(300),
    ok(false)
{
}
void GlobalSetting::print()
{
    stringstream ss;
    ss << "------------------------------------------------------" << endl;
    ss << " Global Settings " << endl;
    #warning TODO: add register_ names for_ these

    ss << "     > channel polarity          : "
        << polarity << " ("
        << ConfigHandler::all_polarities[polarity].toStdString() << ")" << endl;

    ss << "     > channel leakage current   : "
        << boolalpha << leakage_current << endl;

    ss << "     > analog tristates          : "
        << boolalpha << analog_tristates << endl;

    ss << "     > double leakage            : "
        << boolalpha << double_leakage << endl;

    ss << "     > gain                      : "
        << gain << " ("
        << ConfigHandler::all_gains[gain].toStdString() << " mV/fC)" << endl;

    ss << "     > peak time                 : "
        << peak_time << " ("
        << ConfigHandler::all_peakTimes[peak_time] << " ns)" << endl;

    ss << "     > neighbor trigger          : "
        << boolalpha << neighbor_trigger << endl;

    ss << "     > TAC slope adj             : "
        << tac_slope << " ("
        << ConfigHandler::all_TACslopes[tac_slope] << " ns)" << endl;

    ss << "     > disable at peak           : "
        << boolalpha << disable_at_peak << endl;

    ss << "     > ART                       : "
        << boolalpha << art << endl;

    ss << "     > ART mode                  : "
        << art_mode << " ("
        << ConfigHandler::all_ARTmodes[art_mode].toStdString() << ")" << endl;

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
        << direct_time_mode << " ("
        << ConfigHandler::all_directTimeModes[direct_time_mode].toStdString()
        << ")" << endl;

    ss << "     > conv mode 8bit            : "
        << boolalpha << conv_mode_8bit << endl;

    ss << "     > enable 6bit               : "
        << boolalpha << enable_6bit << endl;

    ss << "     > ADC 10bit                 : "
        << adc_10bit << " ("
        << ConfigHandler::all_ADC10bits[adc_10bit].toStdString() << ")" << endl;

    ss << "     > ADC 8bit                  : "
        << adc_8bit << " ("
        << ConfigHandler::all_ADC8bits[adc_8bit].toStdString() << ")" << endl;

    ss << "     > ADC 6bit                  : "
        << adc_6bit << " ("
        << ConfigHandler::all_ADC6bits[adc_6bit].toStdString() << ")" << endl;

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
Channel::Channel() :
    number(0),
    polarity(0),
    capacitance(0),
    leakage_current(0),
    test_pulse(0),
    hidden_mode(0),
    trim(0),
    monitor(0),
    s10bitADC(0),
    s8bitADC(0),
    s6bitADC(0),
    ok(false)
{
}
void Channel::print()
{
    stringstream ss;
    ss << "Channel " << number << endl;
    ss << "    > polarity         : " << polarity << endl;
    ss << "    > capacitance      : " << capacitance << endl;
    ss << "    > leakage current  : " << leakage_current << endl;
    ss << "    > test pulse       : " << test_pulse << endl;
    ss << "    > hidden mode      : " << hidden_mode << endl;
    ss << "    > trim             : " << trim << endl;
    ss << "    > monitor          : " << monitor << endl;
    ss << "    > s10bADC time set : " << s10bitADC << endl;
    ss << "    > s08bADC time set : " << s8bitADC << endl;
    ss << "    > s06bADC time set : " << s6bitADC << endl;
    cout << ss.str() << endl;

}
//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  S6 settings
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
s6Setting::s6Setting() :
    cktk(0), // 0ns
    ckbc(2), // 80 MHz
    ckbc_skew(0), // 0 ns
    do_auto_reset(1),
    do_fec_reset(1),
    tk_pulses(2),
    fec_reset_period(4094),
    ok(false)
{
}
void s6Setting::print()
{
    stringstream ss;
    ss << "s6 settings " << endl;
    ss << "    > CKTK                  : " << ConfigHandler::all_CKTK[cktk].toStdString() << " ns" << endl;
    ss << "    > CKBC                  : " << ConfigHandler::all_CKBC[ckbc].toStdString() << " MHz" << endl;
    ss << "    > CKBC skew             : " << ConfigHandler::all_CKBC_SKEW[ckbc_skew].toStdString() << " ns" << endl;
    ss << "    > do vmm auto reset     : " << (do_auto_reset==1 ? "on" : "off") << endl;
    ss << "    > do FEC periodic reset : " << (do_fec_reset==1 ? "on" : "off") << endl;
    ss << "    > TK pulses for FEC reset : " << tk_pulses << endl;
    ss << "    > FEC reset period      : " << fec_reset_period << endl;
    cout << ss.str() << endl;
} 
//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  ChannelMap
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
ChannelMap::ChannelMap() :
    hdmi_no(0),
    on(false),
    first(false),
    second(false),
    art(false),
    ok(false)
{
}
void ChannelMap::print()
{
    stringstream ss;
    ss << "HDMI " << hdmi_no << endl;
    ss << "    > switch          : " << ( on ? "on" : "off") << endl;
    ss << "    > (first, second) : (" << first << ", " << second << ")" << endl;
    ss << "    > art             : " << art << endl;
    cout << ss.str() << endl;
}


