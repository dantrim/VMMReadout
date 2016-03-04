
// vmm
#include "configuration_module.h"

// std/stl
#include <iostream>

// Qt
#include <QString>

//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  Configuration
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
Configuration::Configuration(QObject *parent)
    : QObject(parent)
{


}
// ------------------------------------------------------------------------ //
Configuration& Configuration::LoadConfig(ConfigHandler& config)
{
    using namespace std;
    m_configHandler = &config;
    cout << "LOADED IP FROM CONFIGURATION : "
        << m_configHandler->getIPList().toStdString() << endl; 
    return *this;
}
// ------------------------------------------------------------------------ //
Configuration& Configuration::LoadSocket(SocketHandler& socket)
{
    using namespace std;
    m_socketHandler = &socket;
    cout << "Configuration LoadSocket has loaded socket with name: "
        << m_socketHandler->getName() << endl;
    return *this;
}
// ------------------------------------------------------------------------ //
void Configuration::SendConfig()
{

    // NEED TO ADD SOCKET STATE CHECK

    bool ok;

    ///////////////////////////////////////////////////
    // build the configuration word(s) to be sent
    // to the front end
    ///////////////////////////////////////////////////
    QString gr[3];
    QString tmp;
    int gs; 
    using namespace std;

    ///////////////////////////////////////////////////
    // Global SPI 0
    ///////////////////////////////////////////////////
    gr[0]="00000000000000000000000000000000"; 
    gs = 0;

    //10bit

}
