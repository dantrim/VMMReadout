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
//  Allowed/Expected values for several configurables
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
const QStringList ConfigHandler::all_gains
    = {"0.5", "1.0", "3.0", "4.5", "6.0", "9.0", "12.0", "16.0"}; //mV/fC
const QList<int> ConfigHandler::all_peakTimes
    = {200, 100, 50, 25}; //ns
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

//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  ConfigHandler -- Constructor
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
ConfigHandler::ConfigHandler(QObject *parent) :
    QObject(parent),
    m_dbg(false)
{
    m_channelmap.clear();
    m_channels.clear();
}
//// ------------------------------------------------------------------------ //
void ConfigHandler::LoadConfig(const QString &filename)
{

    using namespace std;
    m_commSettings.config_filename = filename.split("/").last();

    using boost::property_tree::ptree;
    using namespace boost::property_tree::xml_parser;
    ptree pt;
    read_xml(filename.toStdString(), pt, trim_whitespace | no_comments);

    LoadCommInfo(pt);
    LoadGlobalSettings(pt);
    LoadDAQConfig(pt);
    LoadHDMIChannels(pt);
    setHDMIChannelMap();
    LoadVMMChannelConfig(pt);

    bool ok;
    std::cout << "ChannelMap size   : " << m_channelmap.size() << std::endl;
    std::cout << "HDMI channel map  : " << getHDMIChannelMap() << std::endl;
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
        //    g.gain = QString::fromStdString(gain);
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
            int ptime = conf.second.get<int>("peak_time");
            if(all_peakTimes.indexOf(ptime)>=0)
                g.peak_time = all_peakTimes.indexOf(ptime);
            else {
                cout << "ERROR peak_time value must be one of: [";
                for(auto& i : all_peakTimes) cout << " " << i << " ";
                cout << "] ns, you have provided: " << ptime << endl;
                exit(1);
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
                cout << "ERROR TAC_slope_adj value must be one of: [";
                for(auto& i : all_TACslopes) cout << " " << i << " ";
                cout << "] ns, you have provided: " << tacslope << endl;
                exit(1);
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
                cout << "ERROR ART_mode value must be one of: [";
                for(auto& i : all_ARTmodes) cout << " " << i.toStdString() << " ";
                cout << "], you have provided: " << artmode << endl;
                exit(1);
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
                cout << "ERROR direct_time_mode value must be one of: [";
                for(auto& i : all_directTimeModes)cout<<" "<< i.toStdString()<<" ";
                cout << "], you have provided: " << dtmode << endl;
                exit(1);
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
void ConfigHandler::setHDMIChannelMap()
{
    if(m_channelmap.size()==0) {
        std::cout << "ERROR getChMapString    channel map vector is empty!" << std::endl;
        exit(1);
    }
    if(m_channelmap.size()!=8) {
        std::cout << "ERROR getChMapString    channel map vector must have 8 entries!" << std::endl;
        std::cout << "ERROR getChMapString    current vector has " << m_channelmap.size()
                    << " entries." << std::endl;
        exit(1);
    }

    bool ok;
    QString chMapString = "0000000000000000"; 
    for(int i = 0; i < (int)m_channelmap.size(); ++i) {
        QString first, second;
        first  = m_channelmap[i].first ? "1" : "0";
        second = m_channelmap[i].second ? "1" : "0";
        if(m_channelmap[i].on) {
            chMapString.replace(15-2*i, 1, first);
            chMapString.replace(14-2*i, 1, second);
        }
        m_channelMap = (quint16)chMapString.toInt(&ok,2);
    }
}
//// ------------------------------------------------------------------------ //
void ConfigHandler::LoadVMMChannelConfig(const boost::property_tree::ptree& pt)
{
    using boost::property_tree::ptree;
    using namespace std;
    using boost::format;

    stringstream ss;
    stringstream where;

    try {
        for(int iChan = 0; iChan < 64; iChan++) {
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
                    cout << "ERROR polarity value for channel " << iChan << " "
                         << " must be one of: [";
                    for(auto& i : all_polarities) cout<<" "<<i.toStdString()<<" ";
                    cout << "], you have provided: " << polarity << endl;
                    exit(1);
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
int ConfigHandler::isOn(std::string onOrOff, std::string where)
{
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
        std::cout << "Expecting either 'on'/'enabled'"
                  << " or 'off'/'disabled'" << from << "."
                  << " Returning 'off'/'disabled'."
        << std::endl; 
        return 0;
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

    #warning SEE WHY LOGIC IS REVERSED FOR ENA/DIS
    if(enaOrDis==0)
    //if(enaOrDis==1)
        return "enabled";
    else if(enaOrDis==1)
    //else if(enaOrDis==0)
        return "disabled";
    else {
        cout << "ERROR isEnaOrDis expects only '0' or '1' as input." << endl;
        cout << "ERROR You have provided: " << enaOrDis << endl;
        exit(1);
    }
}

//// ------------------------------------------------------------------------ //

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
void Channel::Print()
{
    using namespace std; 
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


