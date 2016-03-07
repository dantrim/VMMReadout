
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

        RunModule& LoadConfig(ConfigHandler& config);
        RunModule& LoadSocket(SocketHandler& socket);

        SocketHandler& socket() { return *m_socketHandler; }
        ConfigHandler& config() { return *m_configHandler; }

        // meaty methods
        void prepareRun();
        void setTriggerAcqConstants();

    private :
        bool m_dbg;
        bool m_writeNtuple;

        SocketHandler *m_socketHandler;
        ConfigHandler *m_configHandler;




}; // class RunModule

#endif
