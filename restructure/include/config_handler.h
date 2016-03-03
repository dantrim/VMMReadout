#ifndef CONFIG_HANDLER_H
#define CONFIG_HANDLER_H

// qt
#include <QObject>

// boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
//
//// std/stdl
#include <iostream>


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


}; // class ConfigHandler

// ---------------------------------------------------------------------- //
//  Structures for holding config items
// ---------------------------------------------------------------------- //

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
    QString storage_dir;
    QString output_filename;
}; 

////////////////////////////////////////////////////////
// stucture for the global configuration settings
////////////////////////////////////////////////////////
struct GlobalSetting {
    QString polarity;
    bool leakage_current;
    bool analog_tristates;
    bool double_leakage;
    int gain;
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
    bool monitoring;
    bool monitor_pdo_out;
    bool adcs;
    bool sub_hysteresis;
    bool direct_time;
    QString direct_time_mode;
    bool conversion_mode_8bit;
    bool enable_6bit;
    QString adc_10bit;
    QString adc_8bit;
    QString adc_6bit;
    bool dual_clock_data;
    bool dual_clock_6bit;
    int threshold_dac;
    int test_pulse_dac;
};


////////////////////////////////////////////////////////
// HDMI containers
////////////////////////////////////////////////////////
struct ChannelMap {
    unsigned int hdmi_no;
    bool on;
    bool first;
    bool second;
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
};

#endif
