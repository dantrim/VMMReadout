// vmm
#include "calibration_module.h"

// std/stl
#include <iostream>
using namespace std;

// qt
#include <QEventLoop>


//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  pdoCalibration struct
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
pdoCalibration::pdoCalibration()
{
    gain_start = 1;
    gain_end = 1;

    threshold_start = 200;
    threshold_end = 300;
    threshold_step = 50;

    pulser_start = 200;
    pulser_end = 300;
    pulser_step = 50;
}


//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  CalibModule
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
CalibModule::CalibModule(QObject *parent) :
    QObject(parent),
    m_dbg(true),
    m_msg(0),
    m_events_for_loop(1000),
    m_chan_start(1),
    m_chan_end(1),
    m_advance(false)
{
    connect(this, SIGNAL(calibACQcomplete()), this, SLOT(advanceCalibLoop()));
}
// ------------------------------------------------------------------------ //
void CalibModule::advanceCalibLoop()
{
    msg()("Setting advance to true", "CalibModule::advanceCalibLoop");
    m_advance = true;
}
// ------------------------------------------------------------------------ //
void CalibModule::LoadMessageHandler(MessageHandler& m)
{
    m_msg = &m;
}
// ------------------------------------------------------------------------ //
bool CalibModule::setChannelRange(int start, int end)
{
    stringstream sx;

    bool ok = true;

    m_chan_start = start;
    m_chan_end = end;

    if( (m_chan_start > 64 || m_chan_start < 1) ) {
        ok = false;
        sx << "ERROR Starting channel number is invalid.\n>>Must be between (inclusive)"
           << "1 and 64";
        msg()(sx,"CalibModule::setChannelRange"); sx.str("");
    }
    if( (m_chan_end > 64 || m_chan_start < 1) ) {
        ok = false;
        sx << "ERROR Ending channel number is invalid.\n>>Must be between (inclusive)"
           << "1 and 64";
        msg()(sx,"CalibModule::setChannelRange"); sx.str("");
    }
    if( m_chan_end < m_chan_start ) {
        ok = false;
        sx << "ERROR You have set the ending channel to be lower than the starting channel.";
        msg()(sx,"CalibModule::setChannelRange"); sx.str("");
    }

    return ok;
}
// ------------------------------------------------------------------------ //
bool CalibModule::loadPDOCalibrationRecipe(pdoCalibration& calib)
{

    stringstream sx;

    bool ok = true;

    sx << "Loading PDO calibration recipe...";
    msg()(sx,"CalibModule::loadPDOCalibrationRecipe"); sx.str("");


    if(calib.gain_start > calib.gain_end) {
        msg()("ERROR Invalid gain range","CalibModule::loadPDOCalibrationRecipe");
        ok = false;
    } 
    if(calib.threshold_start > calib.threshold_end) {
        msg()("ERROR Invalid threshold range","CalibModule::loadPDOCalibrationRecipe");
        ok = false;
    }
    if(calib.pulser_start > calib.pulser_end) {
        msg()("ERROR Invalid pulser amplitude range","CalibModule::loadPDOCalibrationRecipe");
        ok = false;
    }


    // gain ranges
    m_pdoCalib.gain_start = calib.gain_start;
    m_pdoCalib.gain_end = calib.gain_end;

    // threshold ranges
    m_pdoCalib.threshold_start = calib.threshold_start;
    m_pdoCalib.threshold_end = calib.threshold_end;
    m_pdoCalib.threshold_step = calib.threshold_step;

    // amplitude ranges
    m_pdoCalib.pulser_start = calib.pulser_start;
    m_pdoCalib.pulser_end = calib.pulser_end;
    m_pdoCalib.pulser_step = calib.pulser_step;

    return ok;

}
// ------------------------------------------------------------------------ //
bool CalibModule::beginPDOCalibration()
{
    stringstream sx;

    std::vector<string> gains {"0.5","1.0","3.0","4.5","6.0","9.0","12.0","16.0"};

    bool ok;

    sx << "-----------------------------------\n"
       << "   * Starting PDO calibration *\n"
       << "-----------------------------------";
    msg()(sx,"CalibModule::beginPDOCalibration"); sx.str("");

    int g_start = pdoCalib().gain_start;
    int g_end = pdoCalib().gain_end;

    int t_start = pdoCalib().threshold_start;
    int t_end = pdoCalib().threshold_end;
    int t_step = pdoCalib().threshold_step;

    int amp_start = pdoCalib().pulser_start;
    int amp_end = pdoCalib().pulser_end;
    int amp_step = pdoCalib().pulser_step;

    int chan_start = getChannelStart();
    int chan_end = getChannelEnd();
    int events_per_chan = getNEvents();

    for(int g = g_start; g <= g_end; g++) {
        for(int t = t_start; t <= t_end; t+= t_step) {
            for(int amp = amp_start; amp <= amp_end; amp += amp_step) {

                sx << "-----------------------------------\n"
                   << "  PDO calibration loop update\n"
                   << " - - - - - - - - - - - - - - - - - \n"
                   << "    > Gain      : " << gains[g] << " mV/fC\n"
                   << "    > Threshold : " << t << "\n"
                   << "    > Amplitude : " << amp << "\n" 
                   << "-----------------------------------";
                msg()(sx,"CalibModule::beginPDOCalibration"); sx.str("");

                emit setPDOCalibrationState(g, t, amp);

                for(int ch = chan_start; ch <= chan_end; ch++) {

                    m_advance = false;
                    sx << " - - - - - - - - - - - - - - - - - \n"
                       << "    > Channel   : " << ch << "\n"
                       << " - - - - - - - - - - - - - - - - - ";
                    msg()(sx,"CalibModule::beginPDOCalibration"); sx.str("");

                    emit setChannels(ch);
                    // put delays in within MainWindow

                    emit setupCalibrationConfig();

                    emit setCalibrationACQon(events_per_chan);


                    while(!m_advance) {}

                   // {
                   //     // mainwindow needs to have a signal attached to calibmodule slot
                   //     // whcih then emits 'calibACQcomplete'
                   //     // or in mainwindow just call "emit calibModule().calibACQcomplete()"
                   //     QEventLoop acqLoop;
                   //     acqLoop.connect(this, SIGNAL(calibACQcomplete()), SLOT(quit()));
                   //     acqLoop.exec();
                   // }
                    //while(!m_advance)
                    //{
                    //// in setCalibACQon MainWindow will have a signal
                    //// to toggle m_advance
                    //}

                emit setCalibrationACQoff();
                    
                } // ch

            } // amp
        } // t
    } // g
    sx << "-----------------------------------\n"
       << "   * PDO calibration finished *\n"
       << "-----------------------------------";
    msg()(sx,"CalibModule::beginPDOCalibration"); sx.str("");
    emit endCalibrationRun();

    return true;

}
