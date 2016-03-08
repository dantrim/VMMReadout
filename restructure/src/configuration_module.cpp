
// vmm
#include "configuration_module.h"

// std/stl
#include <iostream>
using namespace std;

// Qt
#include <QString>
#include <QDataStream>
#include <QByteArray>

// boost
#include <boost/format.hpp>

//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  Configuration
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
Configuration::Configuration(QObject *parent) :
    QObject(parent),
    m_dbg(false),
    m_socketHandler(0),
    m_configHandler(0)
{
}
// ------------------------------------------------------------------------ //
Configuration& Configuration::LoadConfig(ConfigHandler& config)
{
    if(!m_configHandler)
        m_configHandler = &config;
    else {
        cout << "Configuration::LoadConfig    WARNING ConfigHandler instance "
             << "is already active (non-null)!" << endl;
        cout << "Configuration::LoadConfig    WARNING Will keep first "
             << "instance." << endl;
        return *this;
    }
    if(!m_configHandler) {
        cout << "Configuration::LoadConfig    ERROR ConfigHandler instance null" << endl;
        exit(1);
    }
    return *this;
}
// ------------------------------------------------------------------------ //
Configuration& Configuration::LoadSocket(SocketHandler& socket)
{
    if(!m_socketHandler)
        m_socketHandler = &socket;
    else {
        cout << "Configuration::LoadSocket    WARNING SocketHandler instance "
             << "is already active (non-null)!" << endl;
        cout << "Configuration::LoadSocket    WARNING Will keep first "
             << "instance." << endl;
        return *this;
    }
    if(m_socketHandler) {
        cout << "----------------------------------------------------------" << endl; 
        cout << "Configuration::LoadSocket    SocketHandler instance loaded" << endl;
        m_socketHandler->Print();
        cout << "----------------------------------------------------------" << endl; 
    }
    else {
        cout << "Configuration::LoadSocket    ERROR SocketHandler instance null" << endl;
        exit(1);
    }
        
    return *this;
}
// ------------------------------------------------------------------------ //
void Configuration::SendConfig()
{

    // NEED TO ADD SOCKET STATE CHECK

    bool ok;

    int send_to_port = config().commSettings().vmmasic_port;

    ///////////////////////////////////////////////////
    // build the configuration word(s) to be sent
    // to the front end
    ///////////////////////////////////////////////////

    ///////////////////////////////////////////////////
    // Global SPI
    ///////////////////////////////////////////////////
    std::vector<QString> globalRegisters;
    globalRegisters.clear();
    fillGlobalRegisters(globalRegisters);
    if(globalRegisters.size()!=3){
        std::cout << "ERROR global SPI does not have 3 words." << std::endl;
        exit(1);
    }
    ///////////////////////////////////////////////////
    // Channel Registers
    ///////////////////////////////////////////////////
    std::vector<QString> channelRegisters;
    channelRegisters.clear();
    fillChannelRegisters(channelRegisters);
    if(channelRegisters.size()!=64){
        std::cout << "ERROR channel registers does not have 64 values." << std::endl;
        exit(1);
    }

    ///////////////////////////////////////////////////
    // Now begin to send out the word
    ///////////////////////////////////////////////////
    QByteArray datagram;
    QString cmd, msbCounter;
    cmd = "AAAAFFFF";
    msbCounter = "0x80000000"; 
    unsigned int firstChRegSPI = 0;
    unsigned int lastChRegSPI  = 63;
    unsigned int firstGlobalRegSPI  = 64;
    unsigned int lastGlobalRegSPI   = 66; 

    for(const auto& ip : socket().ipList()) {
        // update global command counter
        socket().updateCommandCounter();

        datagram.clear();
        QDataStream out (&datagram, QIODevice::WriteOnly);
        out.device()->seek(0); //rewind
        out << (quint32)(socket().commandCounter() + msbCounter.toUInt(&ok,16)) //[0,3]
            << (quint32)config().getHDMIChannelMap() //[4,7]
            << (quint32)cmd.toUInt(&ok,16) << (quint32) 0; //[8,11]

        //channel SPI
        for(unsigned int i = firstChRegSPI; i <= lastChRegSPI; ++i) {
            out << (quint32)(i)
                << (quint32)channelRegisters[i].toUInt(&ok,2);
        } // i
        //[12,523]

        // global SPI
        for(unsigned int i = firstGlobalRegSPI; i <= lastGlobalRegSPI; ++i) {
            out << (quint32)(i)
                << (quint32)globalRegisters[i-firstGlobalRegSPI].toUInt(&ok,2);
        } // i
        //[524,547]

        out << (quint32)128 //[548,551]
            << (quint32) 1; //[552,555]

        bool readOK = true;
        socket().SendDatagram(datagram, ip, send_to_port, "fec",
                                        "Configuration::SendConfig"); 
        readOK = socket().waitForReadyRead("fec");
        if(readOK) {
            if(dbg()) cout << "Configuration::SendConfig    "
                           << "Processing replies..." << endl;
            socket().processReply("fec", ip);
        }
        else {
            cout << "Configuration::SendConfig    Timeout while waiting for replies "
                 << "from VMM" << endl;
            socket().closeAndDisconnect("fec","Configuration::SendConfig");
            exit(1);
        }
    } // ip

    socket().closeAndDisconnect("fec","Configuration::SendConfig");
}
// ------------------------------------------------------------------------ //
void Configuration::fillGlobalRegisters(std::vector<QString>& global)
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
        QString::number(config().globalSettings().analog_tristates));
    sequence++;

    //timing out 2
    spi0.replace(sequence,1,
        QString::number(config().globalSettings().direct_time_mode0));

    if(m_dbg)
    {
        std::cout << "Configuration::GlobalRegister    SPI[0] "
                << spi0.toStdString() << std::endl;
    }

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

    //neighbor trigger
    spi1.replace(sequence,1,
        QString::number(config().globalSettings().neighbor_trigger));
    sequence++;

    //direct outputs settings
    spi1.replace(sequence,1,
        QString::number(config().globalSettings().direct_time_mode1));
    sequence++;

    //direct timing
    spi1.replace(sequence,1,
        QString::number(config().globalSettings().direct_time));
    sequence++;

    //sub-hysteresis
    spi1.replace(sequence,1,
        QString::number(config().globalSettings().sub_hysteresis));
    sequence++;

    //TAC slope adjustment
    tmp = QString("%1").arg(config().globalSettings().tac_slope,
                                                            2,2,QChar('0'));
    spi1.replace(sequence,tmp.size(),tmp);
    sequence += tmp.size();

    //threshold DAC
    tmp = QString("%1").arg(config().globalSettings().threshold_dac,
                                                            10,2,QChar('0'));
    spi1.replace(sequence,tmp.size(),tmp);
    sequence += tmp.size();

    //pulse DAC
    tmp = QString("%1").arg(config().globalSettings().test_pulse_dac,
                                                            10,2,QChar('0'));
    spi1.replace(sequence,tmp.size(),tmp);
    sequence += tmp.size();

    if(m_dbg)
    {
        std::cout << "Configuration::GlobalRegister    SPI[1] "
                << spi1.toStdString() << std::endl;
    }

    global.push_back(spi1);

    //////////////////////////////////////////////////////////////////////
    // GLOBAL SPI 2
    //////////////////////////////////////////////////////////////////////

    QString spi2 = "00000000000000000000000000000000"; 
    sequence = 16;

    //polarity
    spi2.replace(sequence,1,
        QString::number(config().globalSettings().polarity));
    sequence++;

    //disable at peak
    spi2.replace(sequence,1,
        QString::number(config().globalSettings().disable_at_peak));
    sequence++;

    //analog monitor to pdo
    spi2.replace(sequence,1,
        QString::number(config().globalSettings().monitor_pdo_out));
    sequence++;

    //tdo buffer
    spi2.replace(sequence,1,
        QString::number(config().globalSettings().out_buffer_tdo));
    sequence++;

    //pdo buffer
    spi2.replace(sequence,1,
        QString::number(config().globalSettings().out_buffer_pdo));
    sequence++;

    //mo buffer
    spi2.replace(sequence,1,
        QString::number(config().globalSettings().out_buffer_mo));
    sequence++;

    //leakage current
    spi2.replace(sequence,1,
        QString::number(config().globalSettings().leakage_current));
    sequence++;

    //channel to monitor
    tmp = QString("%1").arg(config().globalSettings().channel_monitor,
                                                    6,2,QChar('0'));
    spi2.replace(sequence,tmp.size(),tmp);
    sequence += tmp.size();

    //multiplexer
    spi2.replace(sequence,1,
        QString::number(config().globalSettings().monitoring_control));
    sequence++;

    //ART enable
    spi2.replace(sequence,1,
        QString::number(config().globalSettings().art));
    sequence++;

    //ART mode
    spi2.replace(sequence,1,
        QString::number(config().globalSettings().art_mode));
    sequence++;

    if(m_dbg)
    {
        std::cout << "Configuration::GlobalRegister    SPI[2] "
                << spi2.toStdString() << std::endl;
    }

    global.push_back(spi2);

}
// ------------------------------------------------------------------------ //
void Configuration::fillChannelRegisters(std::vector<QString>& registers)
{
    registers.clear();
    int sequence;
    QString tmp;
    QString reg;

    bool do_check = false;
    for(int i = 0; i < 64; ++i){
        sequence=8;
        reg = "00000000000000000000000000000000";

        //SP
        reg.replace(sequence,1,
            QString::number(config().channelSettings(i).polarity));
        sequence++; 
        if(do_check) std::cout << " SP : " << reg.toStdString() << std::endl;

        //SC
        reg.replace(sequence,1,
            QString::number(config().channelSettings(i).capacitance));
        sequence++;
        if(do_check) std::cout << " SC : " << reg.toStdString() << std::endl;

        //SL
        reg.replace(sequence,1,
            QString::number(config().channelSettings(i).leakage_current));
        sequence++;
        if(do_check) std::cout << " SL : " << reg.toStdString() << std::endl;

        //ST
        reg.replace(sequence,1,
            QString::number(config().channelSettings(i).test_pulse));
        sequence++;
        if(do_check) std::cout << " ST : " << reg.toStdString() << std::endl;

        //SM
        reg.replace(sequence,1,
            QString::number(config().channelSettings(i).hidden_mode));
        sequence++;
        if(do_check) std::cout << " SM : " << reg.toStdString() << std::endl;

        //trim
        tmp = "0000";
        tmp = QString("%1").arg(config().channelSettings(i).trim,
                                                    4,2,QChar('0'));
        std::reverse(tmp.begin(),tmp.end()); //bug in VMM2, needs to be reversed
        reg.replace(sequence, tmp.size(), tmp);
        sequence += tmp.size();
        if(do_check) std::cout << " TRIM : " << reg.toStdString() << std::endl;

        //SMX
        reg.replace(sequence,1,
            QString::number(config().channelSettings(i).monitor));
        sequence++;
        if(do_check) std::cout << " SMX : " << reg.toStdString() << std::endl;

        //10 bit adc lsb
        tmp = QString("%1").arg(config().channelSettings(i).s10bitADC,
                                                    5,2,QChar('0'));
        reg.replace(sequence,tmp.size(),tmp);
        sequence += tmp.size();
        if(do_check) std::cout << " 10bit : " << reg.toStdString() << std::endl;

        //8 bit adc lsb
        tmp = QString("%1").arg(config().channelSettings(i).s8bitADC,
                                                    4,2,QChar('0'));
        reg.replace(sequence,tmp.size(),tmp);
        sequence += tmp.size();
        if(do_check) std::cout << " 8bit : " << reg.toStdString() << std::endl;

        //6 bit adc lsb
        tmp = QString("%1").arg(config().channelSettings(i).s6bitADC,
                                                    3,2,QChar('0'));
        reg.replace(sequence,tmp.size(),tmp);
        sequence += tmp.size();
        if(do_check) std::cout << " 6bit : " << reg.toStdString() << std::endl;
        
        if(m_dbg) {
            using boost::format; 
            std::stringstream chan;
            chan.str("");
            chan << format("%02i") % i;
            std::cout << "-----------------------------------------------" << std::endl;
            std::cout << " Channel [" << chan.str() << "] register "
                 << reg.toStdString() << std::endl;
        } 

        registers.push_back(reg);

    } // i


}
