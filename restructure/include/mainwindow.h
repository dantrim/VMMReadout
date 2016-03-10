#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// qt
#include <QMainWindow>
#include <QScrollArea>
#include <QGridLayout>
#include <QPushButton>
#include <QComboBox>
#include <QFont>
//#include <constants.h>

// std/stl
//using std::vector;
using namespace std;

// vmm
#include "config_handler.h"
#include "configuration_module.h"
#include "run_module.h"
#include "socket_handler.h"
#include "data_handler.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();
    
        QFont Font;
        QGridLayout *channelGridLayout;
        void CreateChannelsFields();
        void SetInitialState();

        bool configOK() { return m_configOK; }

        //////////////////////////////////////////////////////
        // methods to grab the VMM tools
        //////////////////////////////////////////////////////
        ConfigHandler& configHandle() { return *vmmConfigHandler; }
        SocketHandler& socketHandle() { return *vmmSocketHandler; }
        Configuration& configModule() { return *vmmConfigModule;  }
        RunModule&     runModule()    { return *vmmRunModule;     }
    
        //////////////////////////////////////////////////////
        // IPs
        //////////////////////////////////////////////////////
        QString ips[16]; 

        /////////////////////////////////////////////////////
        // Channel Fields Buttons
        /////////////////////////////////////////////////////
        QLineEdit *VMMChannel[64];
        QPushButton *VMMNegativeButton[64];
        QComboBox *VMMSDVoltage[64];
        QComboBox *VMMSZ010bCBox[64];
        QComboBox *VMMSZ08bCBox[64];
        QComboBox *VMMSZ06bCBox[64];
    
        QPushButton *VMMSC[64];
        QPushButton *VMMSL[64];
        QPushButton *VMMST[64];
        QPushButton *VMMSM[64];
        QPushButton *VMMSMX[64];
    
        QPushButton *SPLabel;
        QPushButton *SCLabel;
        QPushButton *SLLabel;
        QPushButton *STLabel;
        QPushButton *SMLabel;
        QComboBox *SDLabel;
        QComboBox *SZ010bLabel;
        QComboBox *SZ08bLabel;
        QComboBox *SZ06bLabel;
        QPushButton *SMXLabel;
    
        QPushButton *SPLabel2;
        QPushButton *SCLabel2;
        QPushButton *SLLabel2;
        QPushButton *STLabel2;
        QPushButton *SMLabel2;
        QComboBox *SDLabel2;
        QComboBox *SZ010bLabel2;
        QComboBox *SZ08bLabel2;
        QComboBox *SZ06bLabel2;
        QPushButton *SMXLabel2;
    
        bool VMMSCBool[64];
        bool VMMSLBool[64];
        bool VMMSTBool[64];
        bool VMMSMBool[64];
        bool VMMSMXBool[64];
        bool VMMSPBool[64];
        quint8 VMMSDValue[64];
        quint8 VMMSZ010bValue[64];
        quint8 VMMSZ08bValue[64];
        quint8 VMMSZ06bValue[64];
    
        bool VMMSPBoolAll;
        bool VMMSCBoolAll;
        bool VMMSLBoolAll;
        bool VMMSTBoolAll;
        bool VMMSMBoolAll;
        bool VMMSMXBoolAll;
        bool VMMSZ010bBoolAll;
        bool VMMSZ08bBoolAll;
        bool VMMSZ06bBoolAll;
    
        bool VMMSPBoolAll2;
        bool VMMSCBoolAll2;
        bool VMMSLBoolAll2;
        bool VMMSTBoolAll2;
        bool VMMSMBoolAll2;
        bool VMMSMXBoolAll2;
        bool VMMSZ010bBoolAll2;
        bool VMMSZ08bBoolAll2;
        bool VMMSZ06bBoolAll2;
    
    private:
        Ui::MainWindow *ui;

        const int FECPORT;
        const int DAQPORT;
        const int VMMASICPORT;
        const int VMMAPPPORT;
        const int S6PORT;

        ConfigHandler *vmmConfigHandler;
        SocketHandler *vmmSocketHandler;
        Configuration *vmmConfigModule;
        RunModule     *vmmRunModule;


        bool m_commOK;
        bool m_configOK;

    signals :
        void checkFSM();

    public slots:
        // ~"FSM"
        void updateFSM();

        // counter
        void updateCounter();

        // send the board configuration
        void prepareAndSendBoardConfig();

        // number of FECs
        void setNumberOfFecs(int);

        // channel fields
        void updateChannelState();
        void updateChannelVoltages(int);
        void updateChannelADCs(int);

        // connect to IP
        void Connect();



};


#endif // MAINWINDOW_H
