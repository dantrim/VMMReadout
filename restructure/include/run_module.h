
#ifndef RUN_MODULE_H
#define RUN_MODULE_H

// vmm
#include "socket_handler.h"
#include "config_handler.h"

// qt
#include <QObject>

class RunModule : public QObject
{
    Q_OBJECT

    public :
        explicit RunModule(QObject *parent = 0);
        virtual ~RunModule(){};
        RunModule& setDebug(bool dbg) { m_dbg = dbg; return *this; }
        RunModule& setDoWrite(bool doit) { m_writeNtuple = doit; return *this; }
        bool dbg() { return m_dbg; }
        bool writeOut() { return m_writeNtuple; }
        void setExternalTrig(bool doit) { m_externalTrigger = doit; }
        bool externalTrig() { return m_externalTrigger; }
        void updatePulseCount() { m_pulseCount++; }
        long int pulseCount() { return m_pulseCount; }

        RunModule& LoadConfig(ConfigHandler& config);
        RunModule& LoadSocket(SocketHandler& socket);

        RunModule& initializeDataHandler();
        void setupOutputFiles(TriggerDAQ& daq, QString outdir="",
                                                    QString filename="");

        SocketHandler& socket() { return *m_socketHandler; }
        ConfigHandler& config() { return *m_configHandler; }

        // meaty methods
        void prepareRun();
        void setTriggerAcqConstants();
        void setTriggerMode();
        void ACQon();
        void ACQoff();

        void beginTimedRun();
        void beginPulserRun();

    private :
        bool m_dbg;
        bool m_writeNtuple;
        bool m_externalTrigger;
        long int m_pulseCount; 
        bool m_initSocketHandler;
        bool m_initConfigHandler;

        SocketHandler *m_socketHandler;
        ConfigHandler *m_configHandler;

    signals :
        void EndRun();

    public slots :
        void Run();
        void readEvent();
        void sendPulse();
        void finishRun();

}; // class RunModule

#endif
