#ifndef RUN_MODULE_H
#define RUN_MODULE_H

//qt
#include <QObject>
#include <QFile>
#include <QStringList>
#include <QByteArray>
#include <QMap>
#include <QList>
#include <QString>
#include <QDir>

//vmmrun
#include "configuration_module.h"

class RunDAQ : public QObject
{
    Q_OBJECT

    public :
        explicit RunDAQ(QObject *parent = 0);
        virtual ~RunDAQ(){};
        void UpdateCounter();
        void SetDebugMode(bool doDbg) { m_dbg = doDbg; }
        void SetTestMode(bool doTest) { m_is_testmode = doTest; }
        void SetOutputFileName(QString name) { m_useCustomName = true; m_userGivenName = name; }
        void ReadRFile(QString &file);
        void LoadDAQConstantsFromGUI(int pulserDelay, QString trigPeriod, int acqSync, int acqWindow);
        void LoadChannelMap(quint16 chanMapIn) { m_channelmap = chanMapIn; }
        void LoadIPList(QStringList ipListIn);

        // methods to set up the DAQ from the information read in ReadRFile
        void PrepareRun();
        void SetTrigAcqConstants();
        void SetTriggerMode(bool doExternalRunMode);
        void ACQOn();
        void ACQOff();

        // event loop method
        void DataHeader();
        void TimedRun(long time); 
        void PulserRun();

        // method to send datagrams to the boards
        void Sender(QByteArray datagram_);

    private :
        // global configuration parameters (c.f. vmmconfig/configuration_module.h)
        Configuration* m_config;

        bool m_has_config;
        bool m_is_testmode;
        bool m_dbg;
        quint32 n_command_counter; // keep track of number of commands passed to DAQ

        QUdpSocket* m_socket;
        QUdpSocket* m_socketDAQ;
        bool pinged;
        bool bound;
        QStringList m_replies;
        QByteArray m_buffer;
        QByteArray m_bufferDAQ;

        // configuration parameters
        int m_tpdelay;
        int m_acqsync;
        int m_acqwindow;
        int m_runcount;
        bool m_timedrun;
        QString m_trigperiod;
        QString m_runmode;
        QString m_infilename;
        QString m_outfilepath;
        QString m_outfilename;
        bool m_useCustomName;
        QString m_userGivenName;

        // channel map (which HDMI's to interact with)
        quint16 m_channelmap;

        QStringList m_ips;

        // output file
        QFile m_fileDaq;

        // pulser handling
        int n_pulse_count;

    signals :
        void ExitLoop();

    public slots :

    private slots :
        void Run();
        void processReply(const QString &queried_ip, int command_delay = 0);
        void ReadEvent();
        void SendPulse();
        void FinishRun();


}; // class RunDAQ


#endif
