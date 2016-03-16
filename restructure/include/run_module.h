
#ifndef RUN_MODULE_H
#define RUN_MODULE_H

// vmm
#include "socket_handler.h"
#include "config_handler.h"
#include "message_handler.h"

// qt
#include <QObject>

class RunModule : public QObject
{
    Q_OBJECT

    public :
        explicit RunModule(QObject *parent = 0);
        virtual ~RunModule(){};
        RunModule& setDebug(bool dbg) { m_dbg = dbg; return *this; }
        bool dbg() { return m_dbg; }

        void LoadMessageHandler(MessageHandler& msg);
        MessageHandler& msg() { return *m_msg; }

        ////////////////////////////////////////////
        // Run toggles
        ////////////////////////////////////////////
        /**
         set flag for running with external or internal
         triggers
        */
        void setExternalTrig(bool doit) { m_externalTrigger = doit; }
        bool externalTrig() { return m_externalTrigger; }

        /// pulse counter for running with internal trigger
        void updatePulseCount() { m_pulseCount++; }
        long int pulseCount() { return m_pulseCount; }

        ///////////////////////////////////////////
        // VMM handles
        ///////////////////////////////////////////
        RunModule& LoadConfig(ConfigHandler& config);
        RunModule& LoadSocket(SocketHandler& socket);

        SocketHandler& socket() { return *m_socketHandler; }
        ConfigHandler& config() { return *m_configHandler; }

        ///////////////////////////////////////////
        // Methods for setting up T/DAQ
        // and sending the configuration
        ///////////////////////////////////////////
        void prepareRun();
        void setTriggerAcqConstants();
        void setTriggerMode();
        void ACQon();
        void ACQoff();
        void beginTimedRun();
        void beginPulserRun();


        //////////////////////////////////////////////////////
        // Misc. methods
        //////////////////////////////////////////////////////
        void setEventHeaders(const int evbld_info, const int evbld_mode);
        void resetASICs();
        void resetFEC(bool do_reset);
        void setMask();
        void checkLinkStatus();
        void resetLinks();

    private :
        bool m_dbg;
        MessageHandler *m_msg;
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
