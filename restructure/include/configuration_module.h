
#ifndef CONFIGURATION_MODULE_H
#define CONFIGURATION_MODULE_H

// vmm
#include "config_handler.h"
#include "socket_handler.h"


//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  Configuration
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////

class Configuration : public QObject
{
    Q_OBJECT

    public :
        explicit Configuration(QObject *parent = 0);
        virtual ~Configuration(){};

        Configuration& LoadConfig(ConfigHandler& config);
        Configuration& LoadSocket(SocketHandler& socket);

        void SendConfig();
        ConfigHandler& config() { return *m_configHandler; }

    private :
        SocketHandler *m_socketHandler;
        ConfigHandler *m_configHandler;


}; // class Configuration



#endif
