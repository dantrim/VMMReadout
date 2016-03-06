
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
Configuration::Configuration(QObject *parent) :
    QObject(parent),
    m_dbg(false)
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
    // Global SPI
    ///////////////////////////////////////////////////
    vector<QString> globalSPI;
    globalSPI.clear();
    GlobalRegister(globalSPI);
    

    //10bit

}
// ------------------------------------------------------------------------ //
void Configuration::GlobalRegister(std::vector<QString> global)
{
    global.clear();
    int sequence = 0;

    //////////////////////////////////////////////////////////////////////
    // GLOBAL SPI 0
    //////////////////////////////////////////////////////////////////////

    QString spi0 = "00000000000000000000000000000000"; 
    QString tmp;

    //10bit ADC
    tmp = QString("%1").arg(config().globalSettings().adc_10bit,2,2,QChar('0'));
    spi0.replace(sequence, tmp.size(), tmp);
    sequence += tmp.size();

    //8bit ADC
    tmp = QString("%1").arg(config().globalSettings().adc_8bit,2,2,QChar('0'));
    spi0.replace(sequence, tmp.size(), tmp);
    sequence += tmp.size();

    //6bit
    tmp = QString("%1").arg(config().globalSettings().adc_6bit,3,2,QChar('0'));
    spi0.replace(sequence, tmp.size(), tmp);
    sequence += tmp.size();

    //8-bit enable
    spi0.replace(sequence,1,
        QString::number(config().globalSettings().conv_mode_8bit));
    sequence++;

    //6-bit enable
    spi0.replace(sequence,1,
        QString::number(config().globalSettings().enable_6bit));
    sequence++;

    //ADC enable
    spi0.replace(sequence,1,
        QString::number(config().globalSettings().adcs));
    sequence++;

    //dual clock serialized
    spi0.replace(sequence,1,
        QString::number(config().globalSettings().dual_clock_data));
    sequence++;

    //dual clock ART
    spi0.replace(sequence,1,
        QString::number(config().globalSettings().dual_clock_art));
    sequence++;

    //dual clock 6-bit
    spi0.replace(sequence,1,
        QString::number(config().globalSettings().dual_clock_6bit));
    sequence++;

    //analog tri-states
    spi0.replace(sequence,1,
        QString::number(config().globalSettings().analog_tristes));
    sequence++;

    //timing out 2
    spi0.replace(sequence,1,
        QString::number(config().globalSettings().direct_time_mode[0]));

    if(m_dbg) {
        cout << "Configuration::GlobalRegister    SPI[0] "
                << spi0.toStdString() << endl;

    global.push_back(spi0);
    
    
    //////////////////////////////////////////////////////////////////////
    // GLOBAL SPI 1
    //////////////////////////////////////////////////////////////////////

    QString spi1 = "00000000000000000000000000000000"; 
    sequence = 0;

    //peak_time
    tmp = QString("%1").arg(config().globalSettings().peak_time,2,2,QChar('0'));
    spi1.replace(sequence, tmp.size(),tmp);
    sequence += tmp.size();

    //double leakage (doubles the leakage current)
    spi1.replace(sequence,1,
        QString::number(config().globalSettings().double_leakage));
    sequence++;

    //gain
    tmp = QString("%1").arg(config().globalSettings().gain,3,2,QChar('0'));
    spi1.replace(sequence,tmp.size(),tmp);
    sequence += tmp.size();

    

}
