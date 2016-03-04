
#ifndef CONFIG_HANDLER_H
#define CONFIG_HANDLER_H

// qt
#include <QObject>
#include <QList>
#include <QStringList>

// boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
//
//// std/stdl
#include <iostream>


// ---------------------------------------------------------------------- //
//  Structures for holding config items
// ---------------------------------------------------------------------- //

////////////////////////////////////////////////////////
// structure for communication
////////////////////////////////////////////////////////
struct CommInfo {
    // UDP info
    int fec_port;
    int daq_port;
    int vmmasic_port;
    int vmmapp_port;
    int s6_port;

    // general info
    QString config_version;
    QString vmm_id_list;
    QString ip_list;
    QString comment;
    bool debug;

    void Print();
};

////////////////////////////////////////////////////////
// structure for DAQ configuration
////////////////////////////////////////////////////////
struct TriggerDAQ {
    int tp_delay;
    QString trigger_period;
    int acq_sync;
    int acq_window;
    QString run_mode;
    int run_count;
    QString output_path;
    QString output_filename;

    void Print();
}; 

////////////////////////////////////////////////////////
// stucture for the global configuration settings
////////////////////////////////////////////////////////
struct GlobalSetting {
    QString polarity;
    bool leakage_current;
    bool analog_tristates;
    bool double_leakage;
    QString gain;
    int peak_time;
    bool neighbor_trigger;
    int tac_slope;
    bool disable_at_peak;
    bool art;
    QString art_mode;
    bool dual_clock_art;
    bool out_buffer_mo;
    bool out_buffer_pdo;
    bool out_buffer_tdo;
    int channel_monitor;
    bool monitoring_control;
    bool monitor_pdo_out;
    bool adcs;
    bool sub_hysteresis;
    bool direct_time;
    QString direct_time_mode;
    bool conv_mode_8bit;
    bool enable_6bit;
    QString adc_10bit;
    QString adc_8bit;
    QString adc_6bit;
    bool dual_clock_data;
    bool dual_clock_6bit;
    int threshold_dac;
    int test_pulse_dac;

    void Validate();
    void Print();
};


////////////////////////////////////////////////////////
// HDMI containers
////////////////////////////////////////////////////////
struct ChannelMap {
    unsigned int hdmi_no;
    bool on;
    bool first;
    bool second;

    void Print();
};


////////////////////////////////////////////////////////
// structure holding configuration for the individual
// vmm channels
////////////////////////////////////////////////////////
struct Channel {
    unsigned int number;
    QString polarity;
    bool capacitance;
    bool leakage_current;
    bool test_pulse;
    bool hidden_mode;
    int trim;
    bool monitor;
    int s10bitADC;
    int s8bitADC;
    int s6bitADC;

    void Print();
};

// ---------------------------------------------------------------------- //
//  Main Configuration Handler tool
// ---------------------------------------------------------------------- //
class ConfigHandler : public QObject
{
    Q_OBJECT

    public :
        explicit ConfigHandler(QObject *parent = 0);
        virtual ~ConfigHandler(){};

        void LoadConfig(const QString &filename);
        void LoadCommInfo(const boost::property_tree::ptree& p);
        void LoadGlobalSettings(const boost::property_tree::ptree& p);
        void LoadDAQConfig(const boost::property_tree::ptree& p);
        void LoadHDMIChannels(const boost::property_tree::ptree& p);
        void LoadVMMChannelConfig(const boost::property_tree::ptree& p);

        bool isOn(std::string onOrOff = "");
        std::string isOnOrOff(int onOrOf);
        std::string isEnaOrDis(int enaOrDis);

        QString getIPList() { return m_commSettings.ip_list; }

        // retrieve the objects
        CommInfo& commSettings() { return m_commSettings; }
        TriggerDAQ& daqSettings() { return m_daqSettings; }
        GlobalSetting& globalSettings() { return m_globalSettings; }
        Channel& vmmChannel(int i) { return m_channels[i]; }

        ////////////////////////////
        // expected settings
        ////////////////////////////
        QStringList all_gains;
        QList<int>  all_peakTimes;
        QList<int>  all_TACslopes;
        QStringList all_polarities;
        QStringList all_ARTmodes;
        QStringList all_directTimeModes;
        QStringList all_ADC10bits;
        QStringList all_ADC8bits;
        QStringList all_ADC6bits;

    private :
        CommInfo m_commSettings;
        TriggerDAQ m_daqSettings;
        GlobalSetting m_globalSettings;
        std::vector<ChannelMap> m_channelmap;
        std::vector<Channel> m_channels;


}; // class ConfigHandler


#endif
