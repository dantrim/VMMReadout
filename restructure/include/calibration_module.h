#ifndef CALIBRATION_MODULE_H
#define CALIBRATION_MODULE_H

// vmm
#include "message_handler.h"

// qt
#include <QObject>

// ------------------------------------------------------------------- //
//  Structures for holding the calib items
// ------------------------------------------------------------------- //

///////////////////////////////////////////////
// PDO calibration
///////////////////////////////////////////////
struct pdoCalibration {

    // IF YOU ADD ANY DATA MEMBERS
    // YOU MUST ADD THEM TO THE
    // CONSTRUCTOR!!!

    pdoCalibration();

    // gain range
    int gain_start;
    int gain_end;

    // threshold range
    int threshold_start;
    int threshold_end;
    int threshold_step;

    // pulser amplitude
    int pulser_start;
    int pulser_end;
    int pulser_step;

};

class CalibModule : public QObject
{
    Q_OBJECT

    public :
        explicit CalibModule(QObject *parent = 0);
        virtual ~CalibModule(){};
        CalibModule& setDebug(bool dbg) { m_dbg = dbg; return *this; }
        bool dbg() { return m_dbg; }

        void LoadMessageHandler(MessageHandler& msg);
        MessageHandler& msg() { return *m_msg; }

        void setNEvents(int number) { m_events_for_loop = number; }
        int getNEvents() { return m_events_for_loop; }

        bool setChannelRange(int chan_start = 1, int chan_end = 1);
        int getChannelStart() { return m_chan_start; }
        int getChannelEnd() { return m_chan_end; }

        /////////////////////////////////
        // calibration recipes
        /////////////////////////////////
        bool loadPDOCalibrationRecipe(pdoCalibration& calib);
        pdoCalibration& pdoCalib() { return m_pdoCalib; }
        bool beginPDOCalibration();

    private :
        bool m_dbg;
        MessageHandler *m_msg;

        int m_events_for_loop;
        int m_chan_start;
        int m_chan_end;

        pdoCalibration m_pdoCalib;

        bool m_advance;

    signals :
        void setPDOCalibrationState(int,int,int); // set gain, thresh, amplitude
        void setChannels(int);
        void setupCalibConfig();
        void setCalibACQon(int);
        void setCalibACQoff();
        void endCalibRun();

    public slots :



}; // class CalibModule


#endif
