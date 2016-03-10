#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "constants.h"

// qt
#include <QDir>
#include <QFileDialog>


// ------------------------------------------------------------------------- //
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    FECPORT(6007),
    DAQPORT(6006),
    VMMASICPORT(6603),
    VMMAPPPORT(6600),
    S6PORT(6602),
    vmmConfigHandler(0),
    vmmSocketHandler(0),
    vmmConfigModule(0),
    vmmRunModule(0),
    m_commOK(false),
    m_configOK(false)
{

    ui->setupUi(this);
    //this->setFixedSize(1200,720); // 1400, 725
    this->setMinimumHeight(720);
    this->setMinimumWidth(1200);
    Font.setFamily("Arial");

    ui->tabWidget->setStyleSheet("QTabBar::tab { height: 18px; width: 100px; }");
    ui->tabWidget->setTabText(0,"Channel Registers");
    ui->tabWidget->setTabText(1,"Calibration");
    ui->tabWidget->setTabText(2,"Response");

    /////////////////////////////////////////////////////////////////////
    //-----------------------------------------------------------------//
    // VMM handles
    //-----------------------------------------------------------------//
    /////////////////////////////////////////////////////////////////////
    vmmConfigHandler = new ConfigHandler();
    vmmConfigHandler->setDebug(true);
    vmmSocketHandler = new SocketHandler();
    vmmSocketHandler->setDebug(true);

    connect(vmmSocketHandler, SIGNAL(commandCounterUpdated()),
                                            this, SLOT(updateCounter()));

    vmmConfigModule  = new Configuration();
    vmmConfigModule->setDebug(true);
    vmmRunModule     = new RunModule();
    vmmRunModule->setDebug(true);

    channelGridLayout = new QGridLayout(this);
    CreateChannelsFields();

    /////////////////////////////////////////////////////////////////////
    //-----------------------------------------------------------------//
    // make widget connections
    //-----------------------------------------------------------------//
    /////////////////////////////////////////////////////////////////////

    // ~FSM preliminary
    connect(this, SIGNAL(checkFSM()), this, SLOT(updateFSM()));

    // ping the boards
    connect(ui->openConnection, SIGNAL(pressed()),
                                            this, SLOT(Connect()));

    // prepare the configuration to be sent
    connect(ui->SendConfiguration, SIGNAL(pressed()),
                                    this, SLOT(prepareAndSendBoardConfig()));

    // set number of FECs
    connect(ui->numberOfFecs, SIGNAL(valueChanged(int)),
                                            this, SLOT(setNumberOfFecs(int)));


    /////////////////////////////////////////////////////////////////////
    //-----------------------------------------------------------------//
    // initial state
    //-----------------------------------------------------------------//
    /////////////////////////////////////////////////////////////////////
    SetInitialState();
}

// ------------------------------------------------------------------------- //
MainWindow::~MainWindow()
{
    delete channelGridLayout;



    delete ui;
}
// ------------------------------------------------------------------------- //
void MainWindow::updateFSM()
{
    if(m_commOK && !m_configOK) {
        ui->SendConfiguration->setEnabled(true);
        ui->cmdlabel->setText("No\nConfig.");
    }
    else if(m_commOK & m_configOK) {
        ui->SendConfiguration->setEnabled(true);
        ui->cmdlabel->setText(
                QString::number(socketHandle().commandCounter()));
    }
}
// ------------------------------------------------------------------------- //
void MainWindow::updateCounter()
{
    ui->cmdlabel->setText(QString::number(socketHandle().commandCounter()));
}
// ------------------------------------------------------------------------- //
void MainWindow::prepareAndSendBoardConfig()
{

    // ------------------------------------------------- //
    //  global settings
    // ------------------------------------------------- //
    GlobalSetting global;

    global.polarity            = ui->spg->currentIndex();
    global.leakage_current     = ui->slg->currentIndex();
    global.analog_tristates    = ui->sdrv->currentIndex();
    global.double_leakage      = ui->sfm->currentIndex();
    global.gain                = ui->sg->currentIndex();
    global.peak_time           = ui->st->currentIndex();
    global.neighbor_trigger    = ui->sng->currentIndex();
    global.tac_slope           = ui->stc->currentIndex();
    global.disable_at_peak     = ui->sdp->currentIndex();
    global.art                 = ui->sfa->currentIndex();
    global.art_mode            = ui->sfam->currentIndex();
    global.dual_clock_art      = ui->sdcka->currentIndex();
    global.out_buffer_mo       = ui->sbfm->currentIndex();
    global.out_buffer_pdo      = ui->sbfp->currentIndex();
    global.out_buffer_tdo      = ui->sbft->currentIndex();
    global.channel_monitor     = ui->sm5_sm0->currentIndex();
    global.monitoring_control  = ui->scmx->currentIndex();
    global.monitor_pdo_out     = ui->sbmx->currentIndex();
    global.adcs                = ui->spdc->currentIndex();
    global.sub_hysteresis      = ui->ssh->currentIndex();
    global.direct_time         = ui->sttt->currentIndex();


    global.direct_time_mode0   = ui->stpp->currentIndex();
    global.direct_time_mode1   = ui->stot->currentIndex();

    bool ok;
    QString tmp;
    tmp.append(QString::number(global.direct_time_mode0));
    tmp.append(QString::number(global.direct_time_mode1));
    global.direct_time_mode = tmp.toUInt(&ok,2);

    global.conv_mode_8bit      = ui->s8b->currentIndex();
    global.enable_6bit         = ui->s6b->currentIndex();
    global.adc_10bit           = ui->sc010b->currentIndex();
    global.adc_8bit            = ui->sc08b->currentIndex();
    global.adc_6bit            = ui->sc06b->currentIndex();
    global.dual_clock_data     = ui->sdcks->currentIndex();
    global.dual_clock_6bit     = ui->sdck6b->currentIndex();
    global.threshold_dac       = ui->sdt->value();
    global.test_pulse_dac      = ui->sdp_2->value();
    

    // ------------------------------------------------- //
    //  HDMI channel map
    // ------------------------------------------------- //
    vector<ChannelMap> chMap;
    ChannelMap chm;
    // do this by hand
    for(int i = 0; i < 8; i++) {
        chm.hdmi_no = i;
        if(i==0) {
            chm.on     = (ui->hdmi1->isChecked() ? true : false);
            chm.first  = (ui->hdmi1_1->isChecked() ? true : false);
            chm.second = (ui->hdmi1_2->isChecked() ? true : false);
        } else if(i==1) {
            chm.on     = (ui->hdmi2->isChecked() ? true : false);
            chm.first  = (ui->hdmi2_1->isChecked() ? true : false);
            chm.second = (ui->hdmi2_2->isChecked() ? true : false);
        } else if(i==2) {
            chm.on     = (ui->hdmi3->isChecked() ? true : false);
            chm.first  = (ui->hdmi3_1->isChecked() ? true : false);
            chm.second = (ui->hdmi3_2->isChecked() ? true : false);
        } else if(i==3) {
            chm.on     = (ui->hdmi4->isChecked() ? true : false);
            chm.first  = (ui->hdmi4_1->isChecked() ? true : false);
            chm.second = (ui->hdmi4_2->isChecked() ? true : false);
        } else if(i==4) {
            chm.on     = (ui->hdmi5->isChecked() ? true : false);
            chm.first  = (ui->hdmi5_1->isChecked() ? true : false);
            chm.second = (ui->hdmi5_2->isChecked() ? true : false);
        } else if(i==5) {
            chm.on     = (ui->hdmi6->isChecked() ? true : false);
            chm.first  = (ui->hdmi6_1->isChecked() ? true : false);
            chm.second = (ui->hdmi6_2->isChecked() ? true : false);
        } else if(i==6) {
            chm.on     = (ui->hdmi7->isChecked() ? true : false);
            chm.first  = (ui->hdmi7_1->isChecked() ? true : false);
            chm.second = (ui->hdmi7_2->isChecked() ? true : false);
        } else if(i==7) {
            chm.on     = (ui->hdmi8->isChecked() ? true : false);
            chm.first  = (ui->hdmi8_1->isChecked() ? true : false);
            chm.second = (ui->hdmi8_2->isChecked() ? true : false);
        }
        chMap.push_back(chm);
    } //i


    // ------------------------------------------------- //
    //  configuration of indiviual VMM channels
    // ------------------------------------------------- //
    vector<Channel> channels;
    for(int i = 0; i < 64; i++) {
        Channel ch;
        ch.polarity         = VMMSPBool[i];
        ch.capacitance      = VMMSCBool[i];
        ch.leakage_current  = VMMSLBool[i];
        ch.test_pulse       = VMMSTBool[i];
        ch.hidden_mode      = VMMSMBool[i];
        ch.trim             = VMMSDValue[i];
        ch.monitor          = VMMSMXBool[i];
        ch.s10bitADC        = VMMSZ010bValue[i];
        ch.s8bitADC         = VMMSZ08bValue[i];
        ch.s6bitADC         = VMMSZ06bValue[i];
        channels.push_back(ch);
    } // i

    // global, chMap, channels
    configHandle().LoadBoardConfiguration(global, chMap, channels);
    configModule().LoadConfig(configHandle()).LoadSocket(socketHandle()); 

    m_configOK = true;

    emit checkFSM();
}
// ------------------------------------------------------------------------- //
void MainWindow::setNumberOfFecs(int)
{
    ui->setVMMs->clear();
    QString counter;
    for(int i = 0; i < ui->numberOfFecs->value(); i++){
        ips[i] = ui->ip1->text() + "." + ui->ip2->text() + "." + ui->ip3->text()
                    + "." + counter.setNum(ui->ip4->text().toInt()+i);
        ui->setVMMs->addItem(counter.setNum(i+1));
        ui->setVMMs->setCurrentIndex(i);
    }
    ui->setVMMs->addItem("All");
    ui->setVMMs->setCurrentIndex(ui->setVMMs->currentIndex()+1);

}
// ------------------------------------------------------------------------- //
void MainWindow::SetInitialState()
{
    // write and SPI configuration
    ui->spiRB->setChecked(true);
    ui->writeRB->setChecked(true);

    // don't yet prepare the config
    ui->prepareConfigButton->setEnabled(false);

    // disable ability to send config
    ui->SendConfiguration->setEnabled(false);
    ui->cmdlabel->setText("No\nComm.");

    // disable sending of T/DAQ settings and constants
    ui->setTrgAcqConst->setEnabled(false);
    ui->trgPulser->setEnabled(false);
    ui->trgExternal->setEnabled(false);
    ui->onACQ->setEnabled(false);
    ui->offACQ->setEnabled(false);
}
// ------------------------------------------------------------------------- //
void MainWindow::CreateChannelsFields()
{
    Font.setPointSize(8);
    int margin = 10;
    channelGridLayout->setContentsMargins(margin, margin, margin, margin);
    channelGridLayout->setHorizontalSpacing(1);
    channelGridLayout->setVerticalSpacing(2);
    QString initialValueRadio = "";
    QString counter;

    SPLabel = new QPushButton("SP");
    SCLabel = new QPushButton("SC");
    SLLabel = new QPushButton("SL");
    STLabel = new QPushButton("ST");
    SMLabel = new QPushButton("SM");
    SDLabel = new QComboBox();
    SZ010bLabel = new QComboBox();
    SZ08bLabel = new QComboBox();
    SZ06bLabel = new QComboBox();
    SMXLabel = new QPushButton("SMX");
    
    SPLabel2 = new QPushButton("SP");
    SCLabel2 = new QPushButton("SC");
    SLLabel2 = new QPushButton("SL");
    STLabel2 = new QPushButton("ST");
    SMLabel2 = new QPushButton("SM");
    SDLabel2 = new QComboBox();
    SZ010bLabel2 = new QComboBox();
    SZ08bLabel2 = new QComboBox();
    SZ06bLabel2 = new QComboBox();
    SMXLabel2 = new QPushButton("SMX");

    for(int i = 0; i < 16; i++) {
        SDLabel->addItem(counter.setNum(i)+" mV");
        SDLabel2->addItem(counter.setNum(i)+" mV");
    }
    for(int i = 0; i < 32; i++) {
        SZ010bLabel->addItem(counter.setNum(i)+" ns");
        SZ010bLabel2->addItem(counter.setNum(i)+" ns");
    }
    for(int i = 0; i < 16; i++) {
        SZ08bLabel->addItem(counter.setNum(i)+" ns");
        SZ08bLabel2->addItem(counter.setNum(i)+" ns");
    }
    for(int i = 0; i < 8; i++) {
        SZ06bLabel->addItem(counter.setNum(i)+" ns");
        SZ06bLabel2->addItem(counter.setNum(i)+" ns");
    }

    VMMSPBoolAll=0;
    VMMSCBoolAll=0;
    VMMSLBoolAll=0;
    VMMSTBoolAll=0;
    VMMSMBoolAll=0;
    VMMSMXBoolAll=0;
    VMMSZ010bBoolAll=0;
    VMMSZ08bBoolAll=0;
    VMMSZ06bBoolAll=0;

    VMMSPBoolAll2=0;
    VMMSCBoolAll2=0;
    VMMSLBoolAll2=0;
    VMMSTBoolAll2=0;
    VMMSMBoolAll2=0;
    VMMSMXBoolAll2=0;
    VMMSZ010bBoolAll2=0;
    VMMSZ08bBoolAll2=0;
    VMMSZ06bBoolAll2=0;

    SPLabel->setFixedSize(20,15);
    SCLabel->setFixedSize(20,15);
    SLLabel->setFixedSize(20,15);
    STLabel->setFixedSize(20,15);
    SMLabel->setFixedSize(20,15);
    SDLabel->setFixedSize(50,20);
    SZ010bLabel->setFixedSize(50,20);
    SZ08bLabel->setFixedSize(50,20);
    SZ06bLabel->setFixedSize(50,20);
    SMXLabel->setFixedSize(30,15);
    SPLabel2->setFixedSize(20,15);
    SCLabel2->setFixedSize(20,15);
    SLLabel2->setFixedSize(20,15);
    STLabel2->setFixedSize(20,15);
    SMLabel2->setFixedSize(20,15);
    SDLabel2->setFixedSize(50,20);
    SZ010bLabel2->setFixedSize(50,20);
    SZ08bLabel2->setFixedSize(50,20);
    SZ06bLabel2->setFixedSize(50,20);
    SMXLabel2->setFixedSize(30,15);

    SPLabel->setFont(Font);
    SCLabel->setFont(Font);
    SLLabel->setFont(Font);
    STLabel->setFont(Font);
    SMLabel->setFont(Font);
    SDLabel->setFont(Font);
    SZ010bLabel->setFont(Font);
    SZ08bLabel->setFont(Font);
    SZ06bLabel->setFont(Font);
    SMXLabel->setFont(Font);
    SPLabel2->setFont(Font);
    SCLabel2->setFont(Font);
    SLLabel2->setFont(Font);
    STLabel2->setFont(Font);
    SMLabel2->setFont(Font);
    SDLabel2->setFont(Font);
    SZ010bLabel2->setFont(Font);
    SZ08bLabel2->setFont(Font);
    SZ06bLabel2->setFont(Font);
    SMXLabel2->setFont(Font);


    for (int i = 0; i<64; i++){
        VMMChannel[i] = new QLineEdit(counter.setNum(i+1),ui->tab_3);
        
        VMMSC[i] = new QPushButton(initialValueRadio,ui->tab_3);
        VMMSL[i] = new QPushButton(initialValueRadio,ui->tab_3);
        VMMST[i] = new QPushButton(initialValueRadio,ui->tab_3);
        VMMSM[i] = new QPushButton(initialValueRadio,ui->tab_3);
        VMMSMX[i] = new QPushButton(initialValueRadio,ui->tab_3);
        
        
        VMMSC[i]->setFixedSize(15,15);
        VMMST[i]->setFixedSize(15,15);
        VMMSL[i]->setFixedSize(15,15);
        VMMSM[i]->setFixedSize(15,15);
        VMMSMX[i]->setFixedSize(15,15);

        VMMSC[i]->setStyleSheet("background-color: lightGray");
        VMMSM[i]->setStyleSheet("background-color: lightGray");
        VMMST[i]->setStyleSheet("background-color: lightGray");
        VMMSL[i]->setStyleSheet("background-color: lightGray");
        VMMSMX[i]->setStyleSheet("background-color: lightGray");
   
   
        VMMSPBool[i]=0;
        VMMSCBool[i]=0;
        VMMSMBool[i]=0;
        VMMSTBool[i]=0;
        VMMSLBool[i]=0;
        VMMSMXBool[i]=0;

        VMMChannel[i]->setEnabled(0);
        VMMSDVoltage[i] = new QComboBox(ui->tab_3);
        VMMSDVoltage[i]->setFixedSize(50,20);
        VMMSDVoltage[i]->setFont(Font);
 
        for(int j=0;j<16;j++){
            VMMSDVoltage[i]->addItem(counter.setNum(j)+" mV");
        }

    VMMSZ010bCBox[i] = new QComboBox(ui->tab_3);
    VMMSZ010bCBox[i]->setFixedSize(50,20);
    VMMSZ010bCBox[i]->setFont(Font);

    VMMSZ08bCBox[i] = new QComboBox(ui->tab_3);
    VMMSZ08bCBox[i]->setFixedSize(50,20);
    VMMSZ08bCBox[i]->setFont(Font);

    VMMSZ06bCBox[i] = new QComboBox(ui->tab_3);
    VMMSZ06bCBox[i]->setFixedSize(50,20);
    VMMSZ06bCBox[i]->setFont(Font);

    for(int j=0;j<32;j++){
        VMMSZ010bCBox[i]->addItem(counter.setNum(j)+" ns");
    }
    for(int j=0;j<16;j++){
        VMMSZ08bCBox[i]->addItem(counter.setNum(j)+" ns");
    }
    for(int j=0;j<8;j++){
        VMMSZ06bCBox[i]->addItem(counter.setNum(j)+" ns");
    }

    VMMNegativeButton[i] = new QPushButton(ui->tab_3);
    VMMNegativeButton[i]->setText("negative");

    VMMChannel[i]->setFixedSize(20,18);
    VMMNegativeButton[i]->setFixedSize(40,18);
    VMMNegativeButton[i]->setFont(Font);
    QLabel *spacer = new QLabel(" ");

    if(i<32){
        if(i==0){
            channelGridLayout->addWidget(SPLabel,     i,2, Qt::AlignCenter);
            channelGridLayout->addWidget(SCLabel,     i,3, Qt::AlignCenter);
            channelGridLayout->addWidget(SLLabel,     i,4, Qt::AlignCenter);
            channelGridLayout->addWidget(STLabel,     i,5, Qt::AlignCenter);
            channelGridLayout->addWidget(SMLabel,     i,6, Qt::AlignCenter);
            channelGridLayout->addWidget(SDLabel,     i,7, Qt::AlignCenter);
            channelGridLayout->addWidget(SMXLabel,    i,8, Qt::AlignCenter);
            channelGridLayout->addWidget(SZ010bLabel, i,9, Qt::AlignCenter);
            channelGridLayout->addWidget(SZ08bLabel,  i,10, Qt::AlignCenter);
            channelGridLayout->addWidget(SZ06bLabel,  i,11, Qt::AlignCenter);

            channelGridLayout->addWidget(SPLabel2,     i,14, Qt::AlignCenter);
            channelGridLayout->addWidget(SCLabel2,     i,15, Qt::AlignCenter);
            channelGridLayout->addWidget(SLLabel2,     i,16, Qt::AlignCenter);
            channelGridLayout->addWidget(STLabel2,     i,17, Qt::AlignCenter);
            channelGridLayout->addWidget(SMLabel2,     i,18, Qt::AlignCenter);
            channelGridLayout->addWidget(SDLabel2,     i,19, Qt::AlignCenter);
            channelGridLayout->addWidget(SMXLabel2,    i,20, Qt::AlignCenter);
            channelGridLayout->addWidget(SZ010bLabel2, i,21, Qt::AlignCenter);
            channelGridLayout->addWidget(SZ08bLabel2,  i,22, Qt::AlignCenter);
            channelGridLayout->addWidget(SZ06bLabel2,  i,23, Qt::AlignCenter);
        }
        channelGridLayout->addWidget(VMMChannel[i],         i+1,1, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMNegativeButton[i],  i+1,2, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMSC[i],              i+1,3, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMSL[i],              i+1,4, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMST[i],              i+1,5, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMSM[i],              i+1,6, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMSDVoltage[i],       i+1,7, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMSMX[i],             i+1,8, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMSZ010bCBox[i],      i+1,9, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMSZ08bCBox[i],       i+1,10, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMSZ06bCBox[i],       i+1,11, Qt::AlignCenter);
        channelGridLayout->addWidget(spacer,                 i+1,12, Qt::AlignCenter);
    }
    else{
        channelGridLayout->addWidget(VMMChannel[i],         i-32+1,13, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMNegativeButton[i],  i-32+1,14, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMSC[i],              i-32+1,15, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMSL[i],              i-32+1,16, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMST[i],              i-32+1,17, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMSM[i],              i-32+1,18, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMSDVoltage[i],       i-32+1,19, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMSMX[i],             i-32+1,20, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMSZ010bCBox[i],      i-32+1,21, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMSZ08bCBox[i],       i-32+1,22, Qt::AlignCenter);
        channelGridLayout->addWidget(VMMSZ06bCBox[i],       i-32+1,23, Qt::AlignCenter);
    }
    } // i

    ui->tab_3->setGeometry(QRect(620,12,730,700));
    ui->tab_3->setLayout(channelGridLayout);

    //////////////////////////////////////////////////////////////////////////
    // -------------------------------------------------------------------- //
    // channel connections
    // -------------------------------------------------------------------- //
    //////////////////////////////////////////////////////////////////////////

    // -------------------------------------------------------------------- //
    // update channel voltages
    // -------------------------------------------------------------------- //
    connect(SDLabel,  SIGNAL(currentIndexChanged(int)),
                                        this, SLOT(updateChannelVoltages(int)));
    connect(SDLabel2, SIGNAL(currentIndexChanged(int)),
                                        this, SLOT(updateChannelVoltages(int)));


    // -------------------------------------------------------------------- //
    // update channel ADC values
    // -------------------------------------------------------------------- //
    connect(SZ010bLabel,  SIGNAL(currentIndexChanged(int)),
                                        this, SLOT(updateChannelADCs(int)));
    connect(SZ08bLabel,   SIGNAL(currentIndexChanged(int)),
                                        this, SLOT(updateChannelADCs(int)));
    connect(SZ06bLabel,   SIGNAL(currentIndexChanged(int)),
                                        this, SLOT(updateChannelADCs(int)));
    connect(SZ010bLabel2, SIGNAL(currentIndexChanged(int)),
                                        this, SLOT(updateChannelADCs(int)));
    connect(SZ08bLabel2,  SIGNAL(currentIndexChanged(int)),
                                        this, SLOT(updateChannelADCs(int)));
    connect(SZ06bLabel2,  SIGNAL(currentIndexChanged(int)),
                                        this,SLOT(updateChannelADCs(int)));


    // -------------------------------------------------------------------- //
    // update channel states (green boxes in Channel Fields)
    // -------------------------------------------------------------------- //
    connect(SPLabel,   SIGNAL(pressed()), this, SLOT(updateChannelState()));
    connect(SCLabel,   SIGNAL(pressed()), this, SLOT(updateChannelState()));
    connect(SLLabel,   SIGNAL(pressed()), this, SLOT(updateChannelState()));
    connect(STLabel,   SIGNAL(pressed()), this, SLOT(updateChannelState()));
    connect(SMLabel,   SIGNAL(pressed()), this, SLOT(updateChannelState()));
    connect(SMXLabel,  SIGNAL(pressed()), this, SLOT(updateChannelState()));
    connect(SPLabel2,  SIGNAL(pressed()), this, SLOT(updateChannelState()));
    connect(SCLabel2,  SIGNAL(pressed()), this, SLOT(updateChannelState()));
    connect(SLLabel2,  SIGNAL(pressed()),
                                          this, SLOT(updateChannelState()));
    connect(STLabel2,  SIGNAL(pressed()),
                                          this, SLOT(updateChannelState()));
    connect(SMLabel2,  SIGNAL(pressed()), this, SLOT(updateChannelState()));
    connect(SMXLabel2, SIGNAL(pressed()), this, SLOT(updateChannelState()));

    ///////////////////////////////////////////////////////////////////
    // do updates for individual channels
    ///////////////////////////////////////////////////////////////////
    for(int i = 0; i < 64; i++) {

        // ---------- channel voltages ----------- //
        connect(VMMSDVoltage[i],SIGNAL(currentIndexChanged(int)),
                                        this,SLOT(updateChannelVoltages(int)));

        // ---------------- ADCs ---------------- //
        connect(VMMSZ010bCBox[i],SIGNAL(currentIndexChanged(int)),
                                        this,SLOT(updateChannelADCs(int)));
        connect(VMMSZ08bCBox[i],SIGNAL(currentIndexChanged(int)),
                                            this,SLOT(updateChannelADCs(int)));
        connect(VMMSZ06bCBox[i],SIGNAL(currentIndexChanged(int)),
                                            this,SLOT(updateChannelADCs(int)));
        // ----------- channel states ----------- //
        connect(VMMNegativeButton[i],SIGNAL(pressed()),
                                            this,SLOT(updateChannelState()));
        connect(VMMSC[i],SIGNAL(pressed()),this,SLOT(updateChannelState()));
        connect(VMMSM[i],SIGNAL(pressed()),this,SLOT(updateChannelState()));
        connect(VMMST[i],SIGNAL(pressed()),this,SLOT(updateChannelState()));
        connect(VMMSL[i],SIGNAL(pressed()),this,SLOT(updateChannelState()));
        connect(VMMSMX[i],SIGNAL(pressed()),this,SLOT(updateChannelState()));
    }

}
// ------------------------------------------------------------------------- //
void MainWindow::updateChannelState()
{
    // ***********************  SC  ********************************* //
    if(SCLabel == QObject::sender()){
        if(VMMSCBoolAll==0){
            for(int j=0;j<32;j++){
                VMMSC[j]->setStyleSheet("background-color: green");
                VMMSCBool[j]=true;
            }
            VMMSCBoolAll=1;
        }else{
            for(int j=0;j<32;j++){
                VMMSC[j]->setStyleSheet("background-color: lightGray");
                VMMSCBool[j]=0;
            }
            VMMSCBoolAll=0;
        }
    }else if(SCLabel2 == QObject::sender()){
        if(VMMSCBoolAll2==0){
            for(int j=32;j<64;j++){
                VMMSC[j]->setStyleSheet("background-color: green");
                VMMSCBool[j]=true;
            }
            VMMSCBoolAll2=1;
        }else{
            for(int j=32;j<64;j++){
                VMMSC[j]->setStyleSheet("background-color: lightGray");
                VMMSCBool[j]=0;
            }
            VMMSCBoolAll2=0;
        }
    }

    // ***********************  SL  ********************************* //
    if(SLLabel == QObject::sender()){
        if(VMMSLBoolAll==0){
            for(int j=0;j<32;j++){
                VMMSL[j]->setStyleSheet("background-color: green");
                VMMSLBool[j]=true;
            }
            VMMSLBoolAll=1;
        }else{
            for(int j=0;j<32;j++){
                VMMSL[j]->setStyleSheet("background-color: lightGray");
                VMMSLBool[j]=0;
            }
            VMMSLBoolAll=0;
        }
    }else if(SLLabel2 == QObject::sender()){
        if(VMMSLBoolAll2==0){
            for(int j=32;j<64;j++){
                VMMSL[j]->setStyleSheet("background-color: green");
                VMMSLBool[j]=true;
            }
            VMMSLBoolAll2=1;
        }else{
            for(int j=32;j<64;j++){
                VMMSL[j]->setStyleSheet("background-color: lightGray");
                VMMSLBool[j]=0;
            }
            VMMSLBoolAll2=0;
        }
    }
    // ***********************  ST  ********************************* //
    if(STLabel == QObject::sender()){
        if(VMMSTBoolAll==0){
            for(int j=0;j<32;j++){
                VMMST[j]->setStyleSheet("background-color: green");
                VMMSTBool[j]=true;
            }
            VMMSTBoolAll=1;
        }else{
            for(int j=0;j<32;j++){
                VMMST[j]->setStyleSheet("background-color: lightGray");
                VMMSTBool[j]=0;
            }
            VMMSTBoolAll=0;
        }
    }else if(STLabel2 == QObject::sender()){
        if(VMMSTBoolAll2==0){
            for(int j=32;j<64;j++){
                VMMST[j]->setStyleSheet("background-color: green");
                VMMSTBool[j]=true;
            }
            VMMSTBoolAll2=1;
        }else{
            for(int j=32;j<64;j++){
                VMMST[j]->setStyleSheet("background-color: lightGray");
                VMMSTBool[j]=0;
            }
            VMMSTBoolAll2=0;
        }
    }
    // ***********************  SM  ********************************* //
    if(SMLabel == QObject::sender()){
        if(VMMSMBoolAll==0){
            for(int j=0;j<32;j++){
                VMMSM[j]->setStyleSheet("background-color: green");
                VMMSMBool[j]=true;
            }
            VMMSMBoolAll=1;
        }else{
            for(int j=0;j<32;j++){
                VMMSM[j]->setStyleSheet("background-color: lightGray");
                VMMSMBool[j]=0;
            }
            VMMSMBoolAll=0;
        }
    }else if(SMLabel2 == QObject::sender()){
        if(VMMSMBoolAll2==0){
            for(int j=32;j<64;j++){
                VMMSM[j]->setStyleSheet("background-color: green");
                VMMSMBool[j]=true;
            }
            VMMSMBoolAll2=1;
        }else{
            for(int j=32;j<64;j++){
                VMMSM[j]->setStyleSheet("background-color: lightGray");
                VMMSMBool[j]=0;
            }
            VMMSMBoolAll2=0;
        }
    }
    // ***********************  SMX  ********************************* //
    if(SMXLabel == QObject::sender()){
        if(VMMSMXBoolAll==0){
            for(int j=0;j<32;j++){
                VMMSMX[j]->setStyleSheet("background-color: green");
                VMMSMXBool[j]=true;
            }
            VMMSMXBoolAll=1;
        }else{
            for(int j=0;j<32;j++){
                VMMSMX[j]->setStyleSheet("background-color: lightGray");
                VMMSMXBool[j]=0;
            }
            VMMSMXBoolAll=0;
        }
    }else if(SMXLabel2 == QObject::sender()){
        if(VMMSMXBoolAll2==0){
            for(int j=32;j<64;j++){
                VMMSMX[j]->setStyleSheet("background-color: green");
                VMMSMXBool[j]=true;
            }
            VMMSMXBoolAll2=1;
        }else{
            for(int j=32;j<64;j++){
                VMMSMX[j]->setStyleSheet("background-color: lightGray");
                VMMSMXBool[j]=0;
            }
            VMMSMXBoolAll2=0;
        }
    }

    // ******************  SMP (negative buttons) *********************** //
    if(SPLabel == QObject::sender()){
        if(VMMSPBoolAll==0){
            for(int j=0;j<32;j++){
                VMMNegativeButton[j]->setStyleSheet("background-color: green");
                VMMSPBool[j]=true;
            }
            VMMSPBoolAll=1;
        }else{
            for(int j=0;j<32;j++){
                VMMNegativeButton[j]->setStyleSheet("background-color: light");
                VMMSPBool[j]=0;
            }
            VMMSPBoolAll=0;
        }
    }else if(SPLabel2 == QObject::sender()){
        if(VMMSPBoolAll2==0){
            for(int j=32;j<64;j++){
                VMMNegativeButton[j]->setStyleSheet("background-color: green");
                VMMSPBool[j]=true;
            }
            VMMSPBoolAll2=1;
        }else{
            for(int j=32;j<64;j++){
                VMMNegativeButton[j]->setStyleSheet("background-color: light");
                VMMSPBool[j]=0;
            }
            VMMSPBoolAll2=0;
        }
    }
    // *********************  Loop Individually  ********************** //
    for(int i=0;i<64;i++){
        if(VMMSC[i] == QObject::sender()){
            if(VMMSCBool[i]==0){
                VMMSC[i]->setStyleSheet("background-color: green");
                VMMSCBool[i]=true;
            }else if(VMMSCBool[i]==1){
                VMMSC[i]->setStyleSheet("background-color: lightGray");
                VMMSCBool[i]=false;
            }
        }else if(VMMST[i] == QObject::sender()){
            if(VMMSTBool[i]==0){
                VMMST[i]->setStyleSheet("background-color: green");
                VMMSTBool[i]=true;
            }else if(VMMSTBool[i]==1){
                VMMST[i]->setStyleSheet("background-color: lightGray");
                VMMSTBool[i]=false;
            }
        }else if(VMMSL[i] == QObject::sender()){
            if(VMMSLBool[i]==0){
                VMMSL[i]->setStyleSheet("background-color: green");
                VMMSLBool[i]=true;
            }else if(VMMSLBool[i]==1){
                VMMSL[i]->setStyleSheet("background-color: lightGray");
                VMMSLBool[i]=false;
            }
        }else if(VMMSM[i] == QObject::sender()){
            if(VMMSMBool[i]==0){
                VMMSM[i]->setStyleSheet("background-color: green");
                VMMSMBool[i]=true;
            }else if(VMMSMBool[i]==1){
                VMMSM[i]->setStyleSheet("background-color: lightGray");
                VMMSMBool[i]=false;
            }
        }else if(VMMSMX[i] == QObject::sender()){
            if(VMMSMXBool[i]==0){
                VMMSMX[i]->setStyleSheet("background-color: green");
                VMMSMXBool[i]=true;
            }else if(VMMSMXBool[i]==1){
                VMMSMX[i]->setStyleSheet("background-color: lightGray");
                VMMSMXBool[i]=false;
            }
        }else if(VMMNegativeButton[i] == QObject::sender()){
            if(VMMSPBool[i]==0){
                VMMNegativeButton[i]->setStyleSheet("background-color: green");
                VMMSPBool[i]=true;
            }else if(VMMSPBool[i]==1){
                VMMNegativeButton[i]->setStyleSheet("background-color: light");
                VMMSPBool[i]=false;
            }
        }
    }
}
// ------------------------------------------------------------------------- //
void MainWindow::updateChannelVoltages(int index){
    // ***********************  SD  ******************************** //
    if(SDLabel == QObject::sender()){
        for(int j=0;j<32;j++){
            VMMSDVoltage[j]->setCurrentIndex(index);
            VMMSDValue[j]=index;
        }
    }
    if(SDLabel2 == QObject::sender()){
        for(int j=32;j<64;j++){
            VMMSDVoltage[j]->setCurrentIndex(index);
            VMMSDValue[j]=index;
        }
    }
    for(int i=0;i<64;i++){
        if(VMMSDVoltage[i] == QObject::sender()){
            VMMSDValue[i]=index;
        }
    }
}
// ------------------------------------------------------------------------- //
void MainWindow::updateChannelADCs(int index)
{
    // ***********************  SD  ******************************* //
    for(int j=0;j<32;j++){
        if(SZ010bLabel == QObject::sender()){
            VMMSZ010bCBox[j]->setCurrentIndex(index);
            VMMSZ010bValue[j]=index;
        }
        if(SZ08bLabel == QObject::sender()){
            VMMSZ08bCBox[j]->setCurrentIndex(index);
            VMMSZ08bValue[j]=index;
        }
        if(SZ06bLabel == QObject::sender()){
            VMMSZ06bCBox[j]->setCurrentIndex(index);
            VMMSZ06bValue[j]=index;
        }
    }

    for(int j=32;j<64;j++){
        if(SZ010bLabel2 == QObject::sender()){
            VMMSZ010bCBox[j]->setCurrentIndex(index);
            VMMSZ010bValue[j]=index;
        }
        if(SZ08bLabel2 == QObject::sender()){
            VMMSZ08bCBox[j]->setCurrentIndex(index);
            VMMSZ08bValue[j]=index;
        }
        if(SZ06bLabel2 == QObject::sender()){
            VMMSZ06bCBox[j]->setCurrentIndex(index);
            VMMSZ06bValue[j]=index;
        }
    }
    for(int i=0;i<64;i++){
        if(VMMSZ010bCBox[i] == QObject::sender()){
            VMMSZ010bValue[i]=index;
        }
        if(VMMSZ08bCBox[i] == QObject::sender()){
            VMMSZ08bValue[i]=index;
        }
        if(VMMSZ06bCBox[i] == QObject::sender()){
            VMMSZ06bValue[i]=index;
        }
    }
}
// ------------------------------------------------------------------------- //
void MainWindow::Connect()
{
    if(ui->numberOfFecs->value() == 0) {
        ui->connectionLabel->setText("Select number of FECs");
        ui->connectionLabel->setStyleSheet("background-color: light");
        return;
    }

    // ------------------------------------------------- //
    //  "comm info"
    // ------------------------------------------------- //
    CommInfo commInfo;

    commInfo.fec_port       = FECPORT;
    commInfo.daq_port       = DAQPORT;
    commInfo.vmmasic_port   = VMMASICPORT;
    commInfo.vmmapp_port    = VMMAPPPORT;
    commInfo.s6_port        = S6PORT;

    QString iplist = "";
    QString separator = "";
    for(int i = 0; i < ui->numberOfFecs->value(); i++) {
        if(i==(ui->numberOfFecs->value()-1)) {
            separator = "";
        } else {
            separator = ",";
        }
        iplist += ips[i] + separator;
    }
    commInfo.ip_list = iplist;

    configHandle().LoadCommInfo(commInfo);
    bool pingOK = socketHandle().loadIPList(iplist).ping();

    if(pingOK) {
        ui->connectionLabel->setText("all alive");
        ui->connectionLabel->setStyleSheet("background-color: green");
    } else {
        ui->connectionLabel->setText("ping failed"); 
        ui->connectionLabel->setStyleSheet("background-color: lightGray");
    }

    m_commOK = pingOK;

    emit checkFSM();

}

