
#ifndef CONFIG_HANDLER_H
#define CONFIG_HANDLER_H

// qt
#include <QObject>
#include <QList>
#include <QStringList>

// boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

// std/stdl
#include <iostream>

// vmm
#include "message_handler.h"

// ---------------------------------------------------------------------- //
//  Structures for holding config items
// ---------------------------------------------------------------------- //

////////////////////////////////////////////////////////
// structure for communication
////////////////////////////////////////////////////////
class CommInfo {
    public :
        CommInfo();
        virtual ~CommInfo(){};

        // UDP info
        int fec_port;
        int daq_port;
        int vmmasic_port;
        int vmmapp_port;
        int s6_port;

        // general info
        QString config_filename;
        QString vmm_id_list;
        QString ip_list;
        QString comment;

        void print();
        bool ok; // loading went ok
};

////////////////////////////////////////////////////////
// structure for DAQ configuration
////////////////////////////////////////////////////////
class TriggerDAQ {

    public :
        TriggerDAQ();
        virtual ~TriggerDAQ(){};

        int tp_delay;
        std::string trigger_period;
        int acq_sync;
        int acq_window;
        std::string run_mode;
        int run_count;
        int ignore16;
        std::string mapping_file;
        std::string output_path;
        std::string output_filename;
        int bcid_reset;
        int enable_holdoff;

        void print();
        bool ok; // loading went ok
}; 

////////////////////////////////////////////////////////
// stucture for the global configuration settings
////////////////////////////////////////////////////////
class GlobalSetting {

    public :
        GlobalSetting();
        virtual ~GlobalSetting(){};

        int polarity;               //SP
        int leakage_current;
        int analog_tristates;
        int double_leakage;
        int gain;
        int peak_time;
        int neighbor_trigger;
        int tac_slope;
        int disable_at_peak;
        int art;
        int art_mode;
        int dual_clock_art;
        int out_buffer_mo;          //sbfm
        int out_buffer_pdo;         //sbfp
        int out_buffer_tdo;         //sbft
        int channel_monitor;
        int monitoring_control;     //scmx
        int monitor_pdo_out;        //sbmx
        int adcs;
        int sub_hysteresis;
        int direct_time;
        int direct_time_mode;
        int direct_time_mode0;
        int direct_time_mode1;
        int conv_mode_8bit;
        int enable_6bit;
        int adc_10bit;
        int adc_8bit;
        int adc_6bit;
        int dual_clock_data;
        int dual_clock_6bit;
        int threshold_dac;
        int test_pulse_dac;
        
        void validate();
        void print();
        bool ok; // loading went ok
};


////////////////////////////////////////////////////////
// HDMI containers
////////////////////////////////////////////////////////
class ChannelMap {

    public :
        ChannelMap();
        virtual ~ChannelMap(){};

        int hdmi_no;
        bool on;
        bool first;
        bool second;
        bool art;

        void print();
        bool ok; // loading went ok
};


////////////////////////////////////////////////////////
// structure holding configuration for the individual
// vmm channels
////////////////////////////////////////////////////////
class Channel {

    public :
        Channel();
        virtual ~Channel(){};

        int number;
        int polarity;          //SP
        int capacitance;       //SC
        int leakage_current;   //SL
        int test_pulse;        //ST
        int hidden_mode;       //SM
        int trim;
        int monitor;           //SMX
        int s10bitADC;
        int s8bitADC;
        int s6bitADC;

        void print();
        bool ok; // loading went ok
};
////////////////////////////////////////////////////////
// structure for holding s6 settings
////////////////////////////////////////////////////////
class s6Setting {
    public :
        s6Setting();
        virtual ~s6Setting(){};

        int cktk;
        int ckbc;
        int ckbc_skew;
        int do_auto_reset;
        int do_fec_reset;
        int tk_pulses;
        int fec_reset_period;

        void print();
        bool ok; // loading went ok
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

        ConfigHandler& setDebug(bool dbg) { m_dbg = dbg; return *this;}
        bool dbg() { return m_dbg; }

        //addmmfe8
        void setMMFE8(bool set_to_run_mmfe8);
        bool mmfe8() { return m_mmfe8; }

        void LoadMessageHandler(MessageHandler& msg);
        MessageHandler& msg() { return *m_msg; }

        bool LoadConfig(const QString &filename);
        void WriteConfig(QString filename);

        CommInfo LoadCommInfo(const boost::property_tree::ptree& p);
        void LoadCommInfo(const CommInfo& info);

        GlobalSetting LoadGlobalSettings(const boost::property_tree::ptree& p);

        TriggerDAQ LoadDAQConfig(const boost::property_tree::ptree& p);

        std::vector<ChannelMap> LoadHDMIChannels(const boost::property_tree::ptree& p);
        bool setHDMIChannelMap();
        quint16 getHDMIChannelMap() { return m_channelMap; }
        quint32 getHDMIChannelMapART() { return m_channelMapART; }

        std::vector<Channel> LoadVMMChannelConfig(const boost::property_tree::ptree& p);

        s6Setting LoadS6Settings(const boost::property_tree::ptree& p);
        void LoadS6Configuration(s6Setting& s6);

        // methods for GUI interaction
        void LoadBoardConfiguration(GlobalSetting& global,
             std::vector<ChannelMap>& chMap, std::vector<Channel>& channels);
        void LoadTDAQConfiguration(TriggerDAQ& daq);

        int isOn(std::string onOrOff = "", std::string where ="");
        std::string isOnOrOff(int onOrOf);
        std::string isEnaOrDis(int enaOrDis);

        QString getIPList() { return m_commSettings.ip_list; }

        // retrieve the objects
        CommInfo& commSettings()                { return m_commSettings; }
        TriggerDAQ& daqSettings()               { return m_daqSettings; }
        GlobalSetting& globalSettings()         { return m_globalSettings; }
        s6Setting& s6Settings()                 { return m_s6Settings; }
        ChannelMap& hdmiChannelSettings(int i)  { return m_channelmap[i]; }
        Channel& channelSettings(int i)         { return m_channels[i]; }

        ////////////////////////////
        // expected settings
        ////////////////////////////
        static const QStringList all_gains;
        static const QList<int>  all_peakTimes;
        static const QStringList all_s6TPskews;
        static const QList<int>  all_TACslopes;
        static const QStringList all_polarities;
        static const QStringList all_ARTmodes;
        static const QStringList all_directTimeModes;
        static const QStringList all_ADC10bits;
        static const QStringList all_ADC8bits;
        static const QStringList all_ADC6bits;
        // s6
        static const QStringList all_CKTK;
        static const QStringList all_CKBC;
        static const QStringList all_CKBC_SKEW;

    private :
        bool m_dbg;
        //addmmfe8
        bool m_mmfe8;

        CommInfo                m_commSettings;
        TriggerDAQ              m_daqSettings;
        GlobalSetting           m_globalSettings;
        s6Setting               m_s6Settings;
        std::vector<ChannelMap> m_channelmap;
        quint16                 m_channelMap; 
        quint32                 m_channelMapART;
        std::vector<Channel>    m_channels;

        MessageHandler* m_msg;

}; // class ConfigHandler



#endif
