#ifndef CONFMOD_H
#define CONFMOD_H

#include <QSettings>
#include <QFile>
#include <QByteArray>
#include <QUdpSocket>
#include <QBitArray>
#include <QStringList>
#include <QDomDocument>

using namespace std;

class Configuration : public QObject
{

  Q_OBJECT

    public: 

        Configuration();
        ~Configuration();
        int Ping();
        int ReadCFile(QString &filename);
        int SendConfig();
        void Sender(QByteArray blockOfData, const QString &ip, quint16 destPort);
        void Sender(QByteArray blockOfData, const QString &ip, quint16 destPort, QUdpSocket& socket);
        quint32 ValueToReplace(QString datagramHex, int bitToChange, bool bitValue);
        int ResetTriggerCounter();
        QByteArray bitsToBytes(QBitArray bits);

        //Getters
        QString getGain() {return _gainstring;}
        int getPeakTime() {return _peakint;}
        QString getNeighborTrigger() {return _ntrigstring;}
        void getIPs(QStringList &list) {list=_ips;}
        void getVMMIDs(QStringList &list) {list=_ids;}
        QString getComment() {return _comment;}
        int getThresholdDAC() {return _thresDAC;}
        int getTestPulseDAC() {return _tpDAC;}
        quint16 getChannelMap() {return channelMap;} 
        void getTrigDAQ(QStringList &slist, vector<int> &ilist){
	    	slist << Trig_Period;
	    	slist << Run_mode;
	    	slist << Out_Path;
	    	slist << Out_File;
	    	ilist.clear();
	    	ilist.push_back(TP_Delay);
	    	ilist.push_back(ACQ_Sync);
	    	ilist.push_back(ACQ_Window);
	    	ilist.push_back(Run_count);
        }
        quint16 getFECPort()      { return FECPort; }
        quint16 getDAQPort()      { return DAQPort; }
        quint16 getVMMASICPort()  { return VMMASICPort; }
        quint16 getVMMAPPPort()   { return VMMAPPPort; }
        quint16 getS6Port()       { return S6Port; }
        QString getGainString()   { return _gainstring; }
        int getTACSlope()         { return _TACslop; }
        int getPeakInt()          { return _peakint; }
        

    private:

        QStringList _ips;
        QStringList _ids;
        QStringList _replies;
        QHostAddress* boardip;
        quint16 FECPort, DAQPort, VMMASICPort, VMMAPPPort, S6Port;
        quint32 commandCounter;
        bool pinged;
        bool bnd;
        QUdpSocket* socket;
        QByteArray buffer;

        bool debug;
	    QSettings* settings;
	    QDomNode *n; 
	    QDomNode n_sub;
	    QDomNodeList hdmis;
	    QFile *file;

	    //Trigger.DAQ setting
	    int TP_Delay, ACQ_Sync, ACQ_Window, Run_count;
	    QString Trig_Period, Run_mode, Out_Path, Out_File;

	    //Channel Map settings
	    bool hdmisw; 
	    QString hdmifi, hdmise;
	    quint16 channelMap;

        //Global settings
	    int _chSP;//ch.polarity
        QString _comment;
        int _gain;//gain
        QString _gainstring;
        int _peakt;//peak.time
        int _peakint;
        int _art;//ART
        int _artm;//ART.mode
        int _cmon;//channel.monitoring
        int _scmx;//monitoring control
        int _sbmx;//monitor.pdo.out
        int _tpDAC;//test.pulse.DAC
        int _thresDAC;//threshold.DAC
        int _ntrig;//neighbor.trigger
        QString _ntrigstring;
        int _nanaltri;//analog.tristates
	    int _leak;//ch.leakage.current
	    int _doubleleak;//double.leakage
	    int _dualclock;//dual.clock.ART
        int _adcs;//ADCs
        int _TACslop;//TAC.slop.adj
        int _hyst;//sub.hyst.discr
        int _dtime;//direct.time
        int _dtimeMode[2];//direct.time.mode
        int _sbfm;//out.buffer.mo
        int _sbfp;//out.buffer.pdo
        int _sbft;//out.buffer.tdo
        int _dpeak;//disable.at.peak
	    int _ebitconvmode;//conv.mode.8bit
	    int _sbitenable;//enable.6bit
	    int _adc10b;//ADC.10bit
	    int _adc8b;//ADC.8bit
	    int _adc6b;//ADC.6bit
	    int _dualclockdata;//dual.clock.data
	    int _dualclock6bit;//dual.clock.6bit

        //Channel-by-channel info
        bool SMX[64];//smx
        int trim[64];//trim
        int tenbADC[64];//10bADCtimeset
        int eightbADC[64];//08bADCtimeset
        int sixbADC[64];//06bADCtimeset
        bool SM[64];//hidden.mode
        bool SP[64];//polarity
        bool SC[64];//sc
        bool SL[64];//sl
        bool ST[64];//test.pulse
 
        //Utilies
        void UpdateCounter();
        quint32 reverse32(QString datagramHex);
        bool EnaDisableKey( const QString &childKey, const QString &tmp);
        bool EnaDisableKey( const QString &childKey, const QString &tmp, int channel);
        bool OnOffKey( const QString &childKey, const QString &tmp);
        bool OnOffChannel( const QString &key, const QString &tmp, int channel);

        void Validate(QString sectionname);

  private slots:
        void processReply(const QString &_sentip, QUdpSocket* socket);
        void dumpReply(const QString &_sentip);
};

#endif
