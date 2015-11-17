#include <iostream>
#include "configuration_module.h"
#include <QProcess>
#include <QFile>
#include <QSettings>
#include <QtCore>
#include <iostream>
#include <QUdpSocket>
#include <QBitArray>
#include <QByteArray>
#include <QThread>
#include <ctime>

Configuration::Configuration(){

	cout<<"Creating configuration object"<<endl;
	pinged=false;
	bnd=false;
	commandCounter=0;
	debug=false;
	boardip=new QHostAddress();
	socket = new QUdpSocket(this);
	
}

Configuration::~Configuration(){

	cout<<"Deleting configuration object"<<endl;

}

int Configuration::Ping(){

	foreach (const QString &ip, _ips){
#ifdef __linux__		
		int exitCode = QProcess::execute("ping", QStringList() << "-c1"<<ip); // Qt command to start a process. program is "ping"
#elif __APPLE__	
		int exitCode = QProcess::execute("ping", QStringList() << "-t1"<<ip); // Qt command to start a process. program is "ping"
#endif
		// ips is the list of ip addresses
		if (0 == exitCode) {
			pinged=true;
		} else {
			pinged=false;
			return 1;
		}
	}
	return 0;
}

int Configuration::ReadCFile(QString &filename){

	bool ok;

	//Structure for XML files
	QDomDocument doc;
	file = new QFile(filename);
	QString errorMsg;
	int errorLine, errorColumn;
	if (!file->open(QIODevice::ReadOnly) || !doc.setContent(file,true,&errorMsg, &errorLine, &errorColumn)) {
		qDebug()<<"The parameter file: "<<filename<<" doesn't exist, aborting";
		qDebug()<<"Error message from the QDomDocument object: "<<errorMsg;
		abort();
	}

	cout<<"Reading CFile"<<endl;
	
	//UDP.Setup
	QDomNodeList info = doc.elementsByTagName("udp.setup");
	//There should be only one element with this name
	if (info.size()==1) {
		n = new QDomNode(info.item(0));
		if (n->hasChildNodes())
			Validate("udp.setup");
		else {
			qDebug()<<"This node should have child nodes, aborting";
			abort();
		}
		FECPort = (n->firstChildElement("fec.port")).text().toInt();
		DAQPort = (n->firstChildElement("daq.port")).text().toInt();
		VMMASICPort = (n->firstChildElement("vmmasic.port")).text().toInt();
		VMMAPPPort = (n->firstChildElement("vmmapp.port")).text().toInt();
		S6Port = (n->firstChildElement("s6.port")).text().toInt();
		//cleanup
		delete n;
		n=0;
	}
	else {
		qDebug()<<"Problem with XML configuration file, aborting";
		abort();
	}

	//General.Info
	info = doc.elementsByTagName("general.info");
	//There should be only one element with this name
	if (info.size()==1) {
		n = new QDomNode(info.item(0));
		if (n->hasChildNodes())
			Validate("general.info");
		else {
			qDebug()<<"This node should have child nodes, aborting";
			abort();
		}
		//placeholder for validation of config.version
		// = n->firstChildElement("config.version").text();
		_ids<<(n->firstChildElement("vmm.id")).text().split(",");
		_ips<<(n->firstChildElement("ip")).text().split(",");
		_comment = (n->firstChildElement("comment")).text();
		if(!(n->firstChildElement("debug")).isNull()) {
			//qDebug()<<"Debug element is present in config file";
			debug = OnOffKey("debug",(n->firstChildElement("debug")).text());
		}
		if(_ips.size()!=_ids.size()){
			qDebug()<<"You have listed "<<_ips.size()<<" IP address(es) in the ip field of section general.info and "<<_ids.size()<<" vmm chip id(s) in the vmm.id field.	There should be the same number of entries in both fields.	Aborting, please review the config file information.";
			abort();
		}
		//cleanup
		delete n;
		n=0;
	}
	else {
		qDebug()<<"Problem with XML configuration file, aborting";
		abort();
	}

	//Trigger.DAQ
	info = doc.elementsByTagName("trigger.daq");
	//There should be only one element with this name
	if (info.size()==1) {
		n = new QDomNode(info.item(0));
		if (n->hasChildNodes())
			Validate("trigger.daq");
		else {
			qDebug()<<"This node should have child nodes, aborting";
			abort();
		}
		TP_Delay = (n->firstChildElement("tp.delay")).text().toInt();
		Trig_Period = (n->firstChildElement("trigger.period")).text();
		ACQ_Sync = (n->firstChildElement("acq.sync")).text().toInt();
		ACQ_Window = (n->firstChildElement("acq.window")).text().toInt();
		Run_mode = (n->firstChildElement("run.mode")).text();
		Run_count = (n->firstChildElement("run.count")).text().toInt();
		Out_Path = (n->firstChildElement("output.path")).text();
		Out_File = (n->firstChildElement("output.filename")).text();
		//cleanup
		delete n;
		n=0;
	}
	else {
		qDebug()<<"Problem with XML configuration file, aborting";
		abort();
	}

	//Channel.Map
	QString chMapString = "0000000000000000";//16bit
	info = doc.elementsByTagName("channel.map");
	//There should be only one element with this name
	if (info.size()==1) {
		n = new QDomNode(info.item(0));
		if (n->hasChildNodes()) {
			hdmis = n->childNodes();
			//Validate Channel Map
			if (hdmis.size() !=8) {
				qDebug()<<"This node should have exactly 8 child nodes, aborting"; 
				abort();
			}
			for (int l=0;l<hdmis.size();++l) {
				if (hdmis.at(l).nodeName() != QString("hdmi.").append(QString::number(l+1))) {
					qDebug() << "These subnodes should be of format hdmi.X, you have " << hdmis.at(l).nodeName() << ", aborting";
					abort();
				} else {
					n_sub = hdmis.at(l);
					Validate(hdmis.at(l).nodeName());
					hdmisw = OnOffKey("switch",hdmis.at(l).firstChildElement("switch").text());
					hdmifi = hdmis.at(l).firstChildElement("first").text();
					hdmise = hdmis.at(l).firstChildElement("second").text();
					if (hdmisw) {
						chMapString.replace(15-l*2,1,hdmifi);
						chMapString.replace(14-l*2,1,hdmise);
					}
					channelMap=(quint16)chMapString.toInt(&ok,2);
					if (debug) qDebug()<< "ChannelMap: " << chMapString << " " << channelMap;
				}
			}
		} else {
			qDebug()<<"This node should have child nodes, aborting";
			abort();
		}
		//cleanup
		delete n;
		n=0;
	}

	//Global.Settings
	info = doc.elementsByTagName("global.settings");
	//There should be only one element with this name
	if (info.size()==1) {
		n = new QDomNode(info.item(0));
		if (n->hasChildNodes())
			Validate("global.settings");
		else {
			qDebug()<<"This node should have child nodes, aborting";
			abort();
		}
		//ch.polarity
 		QString stmp = n->firstChildElement("ch.polarity").text();
		if(stmp=="wires" || stmp=="Wires"){
			_chSP=0;
		}else if(stmp=="strips" || stmp=="Strips"){
			_chSP=1; 
		}else{
			qDebug()<<"Ch. polarity value must be one of: strips or wires , you have: "<<stmp;
			abort();
		}
		//ch.leakage.current
	 	_leak = EnaDisableKey("ch.leakage.current",(n->firstChildElement("ch.leakage.current")).text());
		//analog.tristates
	 	_nanaltri = OnOffKey("analog.tristates",(n->firstChildElement("analog.tristates")).text());
		//double.leakage
	 	_doubleleak = OnOffKey("double.leakage",(n->firstChildElement("double.leakage")).text());
		//gain
		_gainstring = (n->firstChildElement("gain")).text();
		if(_gainstring=="0.5"){
			_gain=0;
		}else if(_gainstring=="1.0"){
			_gain=1; 
		}else if(_gainstring=="3.0"){
			_gain=2;
		}else if(_gainstring=="4.5"){
			_gain=3;
		}else if(_gainstring=="6.0"){
			_gain=4;
		}else if(_gainstring=="9.0"){
			_gain=5;
		}else if(_gainstring=="12.0"){
			_gain=6;
		}else if(_gainstring=="16.0"){
			_gain=7;
		}else{
			qDebug()<<"gain value must be one of: 0.5, 1.0, 3.0, 4.5, 6.0, 9.0, 12.0 or 16.0 mV/fC , you have: "<<_gainstring;
			abort();
		}
		//peak.time
		_peakint = (n->firstChildElement("peak.time")).text().toInt();
		if(_peakint==200){
			_peakt=0;
		}else if(_peakint==100){
			_peakt=1; 
		}else if(_peakint==50){
			_peakt=2;
		}else if(_peakint==25){
			_peakt=3;
		}else{
			qDebug()<<"peak.time value must be one of: 25, 50, 100 or 200 ns , you have: "<<_peakint;
			abort();
		}
		//neighbor.trigger
		_ntrigstring = (n->firstChildElement("neighbor.trigger")).text();
	 	_ntrig = OnOffKey("neighbor.trigger",_ntrigstring);
		//TAC.slop.adj
		int tmp = (n->firstChildElement("TAC.slop.adj")).text().toInt();
		if(tmp==125){
			_TACslop=0;
		}else if(tmp==250){
			_TACslop=1; 
		}else if(tmp==500){
			_TACslop=2;
		}else if(tmp==1000){
			_TACslop=3;
		}else{
			qDebug()<<"TAC.slop.adj value must be one of: 125, 250, 500 or 1000 ns , you have: "<<tmp;
			abort();
		}
		//disable.at.peak
	 	_dpeak = OnOffKey("disable.at.peak",(n->firstChildElement("disable.at.peak")).text());
		//ART
	 	_art = OnOffKey("ART",(n->firstChildElement("ART")).text());
		//ART.mode
		stmp = (n->firstChildElement("ART.mode")).text();
		if(stmp=="threshold"){
			_artm=0;
		}else if(stmp=="peak"){
			_artm=1; 
		}else{
			qDebug()<<"ART.mode value must be one of: threshold or peak , you have: "<<stmp;
			abort();
		}
		//dual.clock.ART
	 	_dualclock = OnOffKey("dual.clock.ART",(n->firstChildElement("dual.clock.ART")).text());
		//out.buffer.mo
	 	_sbfm = OnOffKey("out.buffer.mo",(n->firstChildElement("out.buffer.mo")).text());
		//out.buffer.pdo
	 	_sbfp = OnOffKey("out.buffer.pdo",(n->firstChildElement("out.buffer.pdo")).text());
		//out.buffer.tdo
	 	_sbft = OnOffKey("out.buffer.tdo",(n->firstChildElement("out.buffer.tdo")).text());
		//channel.monitoring
		_cmon = (n->firstChildElement("channel.monitoring")).text().toInt();
		if(_cmon<0 ||_cmon>63){
			qDebug()<<"channel.monitoring value must be between 0 and 63, you have: "<<_cmon;
			abort();
		}
		//monitoring.control
	 	_scmx = OnOffKey("monitoring.control",(n->firstChildElement("monitoring.control")).text());
		//monitor.pdo.out
	 	_sbmx = OnOffKey("monitor.pdo.out",(n->firstChildElement("monitor.pdo.out")).text());
		//ADCs
	 	_adcs = OnOffKey("ADCs",(n->firstChildElement("ADCs")).text());
		//sub.hyst.discr
	 	_hyst = OnOffKey("sub.hyst.discr",(n->firstChildElement("sub.hyst.discr")).text());
		//direct.time
	 	_dtime = OnOffKey("direct.time",(n->firstChildElement("direct.time")).text());
		//direct.time.mode
		stmp = (n->firstChildElement("direct.time.mode")).text();
		if (stmp=="TtP") { //threshold-to-peak
			_dtimeMode[0]=0;_dtimeMode[1]=0;//0
		} else if (stmp=="ToT") { //time-over-threshold
			_dtimeMode[0]=0;_dtimeMode[1]=1;//1
		} else if (stmp=="PtP") {//pulse-at-peak
			_dtimeMode[0]=1;_dtimeMode[1]=0;//2
		} else if (stmp=="PtT") {//peak-to-threshold
			_dtimeMode[0]=1;_dtimeMode[1]=1;//3
		} else {
			qDebug()<<"direct.time.mode value must be one of: TtP, ToT, PtP or PtT, you have: "<<stmp;
			abort();
		}
		//conv.mode.8bit
	 	_ebitconvmode = OnOffKey("conv.mode.8bit",(n->firstChildElement("conv.mode.8bit")).text());
		//enable.6bit
	 	_sbitenable = OnOffKey("enable.6bit",(n->firstChildElement("enable.6bit")).text());
		//ADC.10bit
		stmp = (n->firstChildElement("ADC.10bit")).text();
		if(stmp=="200ns"){
			_adc10b=0;
		}else if(stmp=="+60ns"){
			_adc10b=1; 
		}else{
			qDebug()<<"ADC.10bit value must be one of: 200ns or +60ns, you have: "<<stmp;
			abort();
		}
		//ADC.8bit
		stmp = (n->firstChildElement("ADC.8bit")).text();
		if(stmp=="100ns"){
			_adc8b=0;
		}else if(stmp=="+60ns"){
			_adc8b=1; 
		}else{
			qDebug()<<"ADC.8bit value must be one of: 100ns or +60ns, you have: "<<stmp;
			abort();
		}
		//ADC.6bit
		stmp = (n->firstChildElement("ADC.6bit")).text();
		if(stmp=="low" || stmp=="Low"){
			_adc6b=0;
		}else if(stmp=="middle" || stmp=="Middle"){
			_adc6b=1; 
		}else if(stmp=="up" || stmp=="Up"){
			_adc6b=2; 
		}else{
			qDebug()<<"ADC.6bit value must be one of: low, middle or up, you have: "<<stmp;
			abort();
		}
		//dual.clock.data
	 	_dualclockdata = OnOffKey("dual.clock.data",(n->firstChildElement("dual.clock.data")).text());
		//dual.clock.6bit
	 	_dualclock6bit = OnOffKey("dual.clock.6bit",(n->firstChildElement("dual.clock.6bit")).text());
		//threshold.DAC
		_thresDAC = (n->firstChildElement("threshold.DAC")).text().toInt();
		if(_thresDAC<0 ||_thresDAC>1023){
			qDebug()<<"threshold.DAC value must be between 0 and 1023, you have: "<<_thresDAC;
			abort();
		}
		//test.pulse.DAC
		_tpDAC = (n->firstChildElement("test.pulse.DAC")).text().toInt();
		if(_tpDAC<0 ||_tpDAC>1023){
			qDebug()<<"test.pulse.DAC value must be between 0 and 1023, you have: "<<_tpDAC;
			abort();
		}
		/* Old VMM1 registers
		//make.channel.7.neighbor.56
	 	//_n756 = OnOffKey("make.channel.7.neighbor.56",(n->firstChildElement("make.channel.7.neighbor.56")).text());
		//TAC.stop.setting
		//stmp = (n->firstChildElement("TAC.stop.setting")).text();
		//if(stmp=="ena-low"){
		//	_TACstop=0;
		//}else if(stmp=="stp-low"){
		//	_TACstop=1; 
		//}else{
		//	qDebug()<<"TAC.stop.setting value must be one of: ena-low or stp-low, you have: "<<stmp;
		//	abort();
		//}
		//ACQ.self.reset
	 	//_ACQreset = OnOffKey("ACQ.self.reset",(n->firstChildElement("ACQ.self.reset")).text());*/
		//cleanup
		delete n;
		n=0;
	}
	else {
		qDebug()<<"Problem with XML configuration file, aborting";
		abort();
	}

	//Channels
	QString chan("channel."), schannum; 
	for (int l=0; l<64; ++l) {
		schannum = QString("%1").arg(l, 2, 10, QChar('0'));
		info = doc.elementsByTagName(chan+schannum);
		//There should be only one element with this name
		if (info.size()==1) {
			n = new QDomNode(info.item(0));
			if (n->hasChildNodes())
				Validate(chan+schannum);
			else {
				qDebug()<<"This node should have child nodes, aborting";
				abort();
			}
			//polarity
	 		QString stmp = n->firstChildElement("polarity").text();
			if(stmp=="wires" || stmp=="Wires"){
				SP[l]=0;
			}else if(stmp=="strips" || stmp=="Strips"){
				SP[l]=1; 
			}else{
				qDebug()<<"polarity value in channel "<<l<<"	must be one of: strips or wires , you have: "<<stmp;
				abort();
			}
			//capacitance
	 		SC[l] = OnOffChannel("capacitance",(n->firstChildElement("capacitance")).text(),l);
			//leakage.current
	 		SL[l] = EnaDisableKey("leakage.current",(n->firstChildElement("leakage.current")).text(),l);
			//test.pulse
	 		ST[l] = OnOffChannel("test.pulse",(n->firstChildElement("test.pulse")).text(),l);
			//hidden.mode
	 		SM[l] = OnOffChannel("hidden.mode",(n->firstChildElement("hidden.mode")).text(),l);
			//trim
	 		trim[l] = n->firstChildElement("trim").text().toInt();
			if(trim[l]<0 || trim[l]>15){
				qDebug()<<"trim value in channel "<<l<<"	must be larger than 0 and less than 15,	you have: "<<trim[l];
				abort();
			}
			//monitor.mode
	 		SMX[l] = OnOffChannel("monitor.mode",(n->firstChildElement("monitor.mode")).text(),l);
			//ADC time settings
	 		tenbADC[l] = n->firstChildElement("s10bADC.time.set").text().toInt();
			if(tenbADC[l]<0 || tenbADC[l]>31){
				qDebug()<<"10b ADC time setting value in channel "<<l<<"	must be larger than 0 and less than 31,	you have: "<<tenbADC[l];
				abort();
			}
	 		eightbADC[l] = n->firstChildElement("s08bADC.time.set").text().toInt();
			if(eightbADC[l]<0 || eightbADC[l]>15){
				qDebug()<<"08b ADC time setting value in channel "<<l<<"	must be larger than 0 and less than 15,	you have: "<<eightbADC[l];
				abort();
			}
	 		sixbADC[l] = n->firstChildElement("s06bADC.time.set").text().toInt();
			if(sixbADC[l]<0 || sixbADC[l]>7){
				qDebug()<<"06b ADC time setting value in channel "<<l<<"	must be larger than 0 and less than 7,	you have: "<<sixbADC[l];
				abort();
			}
			//cleanup
			delete n;
			n=0;
		}
		else {
			qDebug()<<"Problem with XML configuration file, aborting";
			abort();
		}
	}

	return 0;
}

int Configuration::ResetTriggerCounter() {

	qDebug()<<"Resetting Trigger Counter";

	//Bind socket to the listening port, to receive the replies
	bnd = socket->bind(FECPort, QUdpSocket::ShareAddress);
	if(bnd==false){
		qDebug()<<"Binding socket failed";
	}else{
		qDebug()<<"Binding socket successful";
	}

	QString datagramHex;
	QByteArray datagramAll;
	QString header = "FEAD";
	bool read=false;
	bool ok;

	foreach (const QString &ip, _ips){
		UpdateCounter();
		datagramAll.clear();
		QDataStream out (&datagramAll, QIODevice::WriteOnly);
		out<<(quint32)commandCounter<<(quint16)header.toUInt(&ok,16);
		out<<(quint16)3<<(quint32)0<<(quint32)0;
		Sender(datagramAll,ip,FECPort, *socket);

		read=false;
		read=socket->waitForReadyRead(5000);
	
		if (read) {
	
			processReply(ip, socket);

			QDataStream out (&buffer, QIODevice::WriteOnly);

			//Actual trigger reset code
			datagramHex = buffer.mid(12,4).toHex();
			quint32 temp32 = ValueToReplace(datagramHex, 13, true);
			out.device()->seek(12);
			out<<temp32;
	
			out.device()->seek(6);
			out<<(quint16)2; //change to write mode
	
			//UpdateCounter();
			Sender(buffer,ip,FECPort, *socket);
		
			read=socket->waitForReadyRead(5000);

			if (read) {

				dumpReply(ip);
				//Clear bits
				datagramHex = buffer.mid(12,4).toHex();
				quint32 temp32 = ValueToReplace(datagramHex, 13, false);
				out.device()->seek(12);
				out<<temp32;
				//UpdateCounter();
				Sender(buffer,ip,FECPort, *socket);

				read=socket->waitForReadyRead(5000);

				if(read){
					processReply(ip, socket);
				} else {
					qDebug()<<"WARNING: Timeout while waiting for replies from VMM "<<ip<<". Check that this is a valid address, that the card is online, and try again";
					return 1;
				}

			} else {
				qDebug()<<"WARNING: Timeout while waiting for replies from VMM "<<ip<<". Check that this is a valid address, that the card is online, and try again";
				return 1;
			}

		} else {
			qDebug()<<"WARNING: Timeout while waiting for replies from VMM "<<ip<<". Check that this is a valid address, that the card is online, and try again";
			return 1;
		}

	} //end of foreach ip

	return 0;
}

int Configuration::SendConfig(){

  //Checking if UDP Socket is still connected by checking for readyRead() signal
  if (socket->state()==QAbstractSocket::UnconnectedState) {
		if (debug) qDebug()<<"WARNING: About to rebind the socket";
		bnd = socket->bind(FECPort, QUdpSocket::ShareAddress);
		if(bnd==false){
			qDebug()<<"Binding socket failed";
		}else{
			qDebug()<<"Binding socket successful";
		}
	}
	
	bool ok;

	///////////////////////////////////////////////////////////////
	// Building the configuration word from the config file data //
	///////////////////////////////////////////////////////////////

	//qDebug()<<"##################################### GLOBAL BITS <<",	##################################";

	//*********** Not used ************
	//QDataStream::BigEndian	QSysInfo::BigEndian
	//out.device()->seek(4);
	//int headerLength = 0;//2*32;
	//QBitArray channelRegisters(2144+headerLength,false);//64*32 for channels + 3*32 for global - 1088 before
	//*********************************

	QString GlobalRegistersStrings[3],strTmp;
	int GlobalRegistersStringsSequence;

	//####################### Global SPI 0 ###############################################
	//		qDebug()<<"GL SP1 0";
	GlobalRegistersStrings[0]="00000000000000000000000000000000";//init
	GlobalRegistersStringsSequence=0;

	//10bit
	strTmp = QString("%1").arg(_adc10b, 2, 2, QChar('0')); //arg(int to convert, number of figures, base, leading character to use)
	GlobalRegistersStrings[0].replace(GlobalRegistersStringsSequence,strTmp.size(), strTmp);
	GlobalRegistersStringsSequence+=strTmp.size();

	//8bit
	strTmp = QString("%1").arg(_adc8b, 2, 2, QChar('0'));
	GlobalRegistersStrings[0].replace(GlobalRegistersStringsSequence,strTmp.size(), strTmp);
	GlobalRegistersStringsSequence+=strTmp.size();

	//6bit
	strTmp = QString("%1").arg(_adc6b, 3, 2, QChar('0'));
	GlobalRegistersStrings[0].replace(GlobalRegistersStringsSequence,strTmp.size(), strTmp);
	GlobalRegistersStringsSequence+=strTmp.size();

	GlobalRegistersStrings[0].replace(GlobalRegistersStringsSequence,1, QString::number(_ebitconvmode));//8-bit enable
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[0].replace(GlobalRegistersStringsSequence,1, QString::number(_sbitenable));//6-bit enable
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[0].replace(GlobalRegistersStringsSequence,1, QString::number(_adcs));//ADC enable
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[0].replace(GlobalRegistersStringsSequence,1, QString::number(_dualclockdata));//dual clock serialized
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[0].replace(GlobalRegistersStringsSequence,1, QString::number(_dualclock));//dual clock ART
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[0].replace(GlobalRegistersStringsSequence,1, QString::number(_dualclock6bit));//dual clk 6-bit
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[0].replace(GlobalRegistersStringsSequence,1, QString::number(_nanaltri));//tristates analog
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[0].replace(GlobalRegistersStringsSequence,1, QString::number(_dtimeMode[0]));//timing out 2

	if (debug) qDebug()<<"GL0: "<<GlobalRegistersStrings[0];
	//#######################################################################";

	//####################### Global SPI 1 ###############################################
	//qDebug()<<"GL SP1 1";
	GlobalRegistersStrings[1]="00000000000000000000000000000000";//init
	GlobalRegistersStringsSequence=0;

	//peak time
	strTmp = QString("%1").arg(_peakt, 2, 2, QChar('0'));
	GlobalRegistersStrings[1].replace(GlobalRegistersStringsSequence,strTmp.size(), strTmp);
	GlobalRegistersStringsSequence+=strTmp.size();

	GlobalRegistersStrings[1].replace(GlobalRegistersStringsSequence,1, QString::number(_doubleleak));//doubles the leakage current
	GlobalRegistersStringsSequence++;

	//gain
	strTmp = QString("%1").arg(_gain, 3, 2, QChar('0'));
	GlobalRegistersStrings[1].replace(GlobalRegistersStringsSequence,strTmp.size(), strTmp);
	GlobalRegistersStringsSequence+=strTmp.size();

	GlobalRegistersStrings[1].replace(GlobalRegistersStringsSequence,1, QString::number(_ntrig));//neighbor
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[1].replace(GlobalRegistersStringsSequence,1, QString::number(_dtimeMode[1]));//Direct outputs setting
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[1].replace(GlobalRegistersStringsSequence,1, QString::number(_dtime));//direct timing
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[1].replace(GlobalRegistersStringsSequence,1, QString::number(_hyst));//hysterisis
	GlobalRegistersStringsSequence++;

	//TAC slope adjustment
	strTmp = QString("%1").arg(_TACslop, 2, 2, QChar('0'));
	GlobalRegistersStrings[1].replace(GlobalRegistersStringsSequence,strTmp.size(), strTmp);
	GlobalRegistersStringsSequence+=strTmp.size();

	//thr dac
	strTmp = QString("%1").arg(_thresDAC, 10, 2, QChar('0'));
	GlobalRegistersStrings[1].replace(GlobalRegistersStringsSequence,strTmp.size(), strTmp);
	GlobalRegistersStringsSequence+=strTmp.size();

	//pulse dac
	strTmp = QString("%1").arg(_tpDAC, 10, 2, QChar('0'));
	GlobalRegistersStrings[1].replace(GlobalRegistersStringsSequence,strTmp.size(), strTmp);
	GlobalRegistersStringsSequence+=strTmp.size();

	if (debug) qDebug()<<"GL1: "<<GlobalRegistersStrings[1];

	//#######################################################################";
	//####################### Global SPI 2 ###############################################
	GlobalRegistersStrings[2]="00000000000000000000000000000000";//init
	GlobalRegistersStringsSequence=16;
	GlobalRegistersStrings[2].replace(GlobalRegistersStringsSequence,1, QString::number(_chSP));//polarity
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[2].replace(GlobalRegistersStringsSequence,1, QString::number(_dpeak));//disable at peak
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[2].replace(GlobalRegistersStringsSequence,1, QString::number(_sbmx));//analog monitor to pdo
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[2].replace(GlobalRegistersStringsSequence,1, QString::number(_sbft));//tdo buffer
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[2].replace(GlobalRegistersStringsSequence,1, QString::number(_sbfp));//pdo buffer
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[2].replace(GlobalRegistersStringsSequence,1, QString::number(_sbfm));//mo buffer
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[2].replace(GlobalRegistersStringsSequence,1, QString::number(_leak));//leakage current 0=enabled
	GlobalRegistersStringsSequence++;

	//10 bit adc lsb
	strTmp = QString("%1").arg(_cmon, 6, 2, QChar('0'));
	GlobalRegistersStrings[2].replace(GlobalRegistersStringsSequence,strTmp.size(), strTmp);
	GlobalRegistersStringsSequence+=strTmp.size();

	GlobalRegistersStrings[2].replace(GlobalRegistersStringsSequence,1, QString::number(_scmx));//multiplexer
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[2].replace(GlobalRegistersStringsSequence,1, QString::number(_art));//ART enable
	GlobalRegistersStringsSequence++;
	GlobalRegistersStrings[2].replace(GlobalRegistersStringsSequence,1, QString::number(_artm));//ART mode

	if (debug) qDebug()<<"GL2: "<<GlobalRegistersStrings[2];

	//#######################################################################";
	QString channelRegistersStrings[64];
	int bitSequenceCh;
	for(int i=0;i<64;++i){
		bitSequenceCh=8;
		channelRegistersStrings[i]="00000000000000000000000000000000";//init
		//				qDebug()<<"----------------- NEW CHANNEL: "<<i<<",	-------------------------------";
		channelRegistersStrings[i].replace(bitSequenceCh,1, QString::number(SP[i]));bitSequenceCh++;
		channelRegistersStrings[i].replace(bitSequenceCh,1, QString::number(SC[i]));bitSequenceCh++;
		channelRegistersStrings[i].replace(bitSequenceCh,1, QString::number(SL[i]));bitSequenceCh++;
		channelRegistersStrings[i].replace(bitSequenceCh,1, QString::number(ST[i]));bitSequenceCh++;
		channelRegistersStrings[i].replace(bitSequenceCh,1, QString::number(SM[i]));bitSequenceCh++;

		//trim
		strTmp = QString("%1").arg(trim[i], 4, 2, QChar('0'));
		reverse(strTmp.begin(),strTmp.end()); //bug in VMM2, needs to be reverted
		channelRegistersStrings[i].replace(bitSequenceCh,strTmp.size(), strTmp);
		bitSequenceCh+=strTmp.size();

		channelRegistersStrings[i].replace(bitSequenceCh,1, QString::number(SMX[i]));bitSequenceCh++;

		//10 bit adc lsb
		strTmp = QString("%1").arg(tenbADC[i], 5, 2, QChar('0'));
		channelRegistersStrings[i].replace(bitSequenceCh,strTmp.size(), strTmp);
		bitSequenceCh+=strTmp.size();

		//8 bit adc lsb
		strTmp = QString("%1").arg(eightbADC[i], 4, 2, QChar('0'));
		channelRegistersStrings[i].replace(bitSequenceCh,strTmp.size(), strTmp);
		bitSequenceCh+=strTmp.size();

		//6 bit adc lsb
		strTmp = QString("%1").arg(sixbADC[i], 3, 2, QChar('0'));
		channelRegistersStrings[i].replace(bitSequenceCh,strTmp.size(), strTmp);
		bitSequenceCh+=strTmp.size();

		if (debug) qDebug()<<"Channel["<<i<<"]: "<<channelRegistersStrings[i];
		if (debug) qDebug()<<"-----------------------------------------------------------------------";
	}

	/////////////////////////////////////
	// Sending config word to hardware //
	/////////////////////////////////////

	QByteArray datagramSPI;
	QString cmd = "AAAAFFFF";
	QString msbCounter = "0x80000000";
	bool read;
	quint32 firstRegisterChannelSPI = 0;
	quint32 lastRegisterChannelSPI = 63;
	quint32 firstRegisterGlobalSPI = 64;
	quint32 lastRegisterGlobalSPI = 66;

	foreach (const QString &ip, _ips){

		UpdateCounter();
		datagramSPI.clear();
		QDataStream out (&datagramSPI, QIODevice::WriteOnly);
		out.device()->seek(0);
		out<<(quint32)(commandCounter+msbCounter.toUInt(&ok,16))<<(quint32)channelMap;
		if (debug) qDebug()<<"out: " <<datagramSPI.toHex();
		out<<(quint32)cmd.toUInt(&ok,16)<<(quint32)0;
		if (debug) qDebug()<<"out: " <<datagramSPI.toHex();
	
	
		for(unsigned int i=firstRegisterChannelSPI;i<=lastRegisterChannelSPI;++i){
			out<<(quint32)(i)<<(quint32)channelRegistersStrings[i].toUInt(&ok,2);
			if (debug) qDebug()<<"out: " <<datagramSPI.toHex();
			if (debug) qDebug()<<"Address:"<<i<<", value:"<<(quint32)channelRegistersStrings[i].toUInt(&ok,2);
		}
		for(unsigned int i=firstRegisterGlobalSPI;i<=lastRegisterGlobalSPI;++i){
			out<<(quint32)(i)<<(quint32)GlobalRegistersStrings[i-firstRegisterGlobalSPI].toUInt(&ok,2);
			if (debug) qDebug()<<"out: " <<datagramSPI.toHex();
			if (debug) qDebug()<<"Address:"<<i<<", value:"<<(quint32)GlobalRegistersStrings[i-firstRegisterGlobalSPI].toUInt(&ok,2);
		}
	
		//Download the SPI (??)
		out<<(quint32)128<<(quint32)1;
		if (debug) qDebug()<<"out: " <<datagramSPI.toHex();

		Sender(datagramSPI, ip, VMMASICPort, *socket);
		read=socket->waitForReadyRead(1000);
		if(read){
			processReply(ip, socket);
			//should validate datagram content here
	
		}else{
			qDebug()<<"WaitForReadyRead timed out on big configuration package acknowledgement.";
			socket->close();
			socket->disconnectFromHost();
			abort();
		}
			
	} //end if foreach ip

	return 0;
		
}
		

		
void Configuration::Sender(QByteArray blockOfData, const QString &ip, quint16 destPort)
{
	if(pinged==true && bnd==true){
			
				
	socket->writeDatagram(blockOfData,QHostAddress(ip), destPort);
	if(debug)	qDebug()<<"IP:"<<ip<<", data:"<<blockOfData.toHex()<<", size:"<<blockOfData.size();

	}else{
			
		qDebug() <<"Error from socket: "<< socket->errorString()<<", Local Port = "<<socket->localPort()<< ", Bind Reply: "<<bnd<<" Ping status: "<<pinged;
		socket->close();
		socket->disconnectFromHost();
		abort();
	}
}

QByteArray Configuration::bitsToBytes(QBitArray bits) {
	QByteArray bytes;
	bytes.resize(bits.count()/8);//add more §for a header of 8 bits????
	bytes.fill(0);
	// Convert from QBitArray to QByteArray
	for(int b=0; b<bits.count(); ++b)
		bytes[b/8] = ( bytes.at(b/8) | ((bits[b]?1:0)<<(7-(b%8))));
	return bytes;
}

void Configuration::Sender(QByteArray blockOfData, const QString &ip, quint16 destPort, QUdpSocket& socket)
{
    // SocketState enum (c.f. http://doc.qt.io/qt-5/qabstractsocket.html#SocketState-enum)
    bool boards_pinged = pinged;
    bool socket_bound = socket.state() == QAbstractSocket::BoundState;
    bool send_ok = (boards_pinged && socket_bound);
    if(!send_ok) {
        if(!boards_pinged) {
            qDebug() << "[Configuration::Sender]    Ping status: " << boards_pinged;
        }
        if(!socket_bound) {
            qDebug() << "[Configuration::Sender]    Socket is not in bound state. Error from socket: " << socket.errorString() << ", Local Port: " << socket.localPort();
        }
        socket.close();
        socket.disconnectFromHost();
        abort();
    }
    else if(send_ok) {
        qDebug() << "[Configuration::Sender]    Pinged and bound";
        socket.writeDatagram(blockOfData, QHostAddress(ip), destPort);
        if(debug) qDebug() << "IP: " << ip << ", data sent: " << blockOfData.toHex() << ", size: " << blockOfData.size();
    }
}

void Configuration::UpdateCounter(){
		commandCounter++;
		if (debug) qDebug()<<"Command counter at: "<<commandCounter;
}

quint32 Configuration::ValueToReplace(QString datagramHex, int bitToChange, bool bitValue){
	bool ok;
	QBitArray commandToSend(32,false);
	QString datagramBin, temp;
	datagramBin = temp.number(datagramHex.toUInt(&ok,16),2);
	for(int i=0;i<datagramBin.size();i++){
		QString bit = datagramBin.at(i);
		commandToSend.setBit(32-datagramBin.size()+i,bit.toUInt(&ok,10));
	}
	commandToSend.setBit(31-bitToChange,bitValue);//set 13th bit
	QByteArray byteArray = bitsToBytes(commandToSend);
	quint32 temp32 = byteArray.toHex().toUInt(&ok,16);
	return temp32;
}

quint32 Configuration::reverse32(QString datagramHex){
	bool ok;
	QBitArray commandToSend(32,false);
	QString datagramBin, temp;
	datagramBin = temp.number(datagramHex.toUInt(&ok,16),2);
	if (debug) qDebug()<<datagramBin;
	for(int i=0;i<datagramBin.size();i++){
		QString bit = datagramBin.at(i);
		commandToSend.setBit(32-datagramBin.size()+i,bit.toUInt(&ok,10));
		if (debug) qDebug()<<commandToSend;
	}
	QBitArray commandToSendR(32,false);
	for(int i=0;i<32;i++){
		commandToSendR.setBit(31-i,commandToSend[i]);
		if (debug) qDebug()<<commandToSendR;
	}
	QByteArray byteArray = bitsToBytes(commandToSendR);
	quint32 temp32 = byteArray.toHex().toUInt(&ok,16);
	return temp32;
}

bool Configuration::EnaDisableKey(const QString &childKey, const QString &tmp){
	bool val=0;
	if(tmp=="enabled" || tmp=="Enabled"){
		val=0;
	}else if(tmp=="disabled" || tmp=="Disabled"){
		val=1; 
	}else{
		qDebug()<<childKey<<" value must be one of: enabled or disabled , you have: "<<tmp;
		abort();
	}
		
	return val;
}

bool Configuration::EnaDisableKey(const QString &childKey, const QString &tmp, int channel){
	bool val=0;
	if(tmp=="enabled" || tmp=="Enabled"){
		val=0;
	}else if(tmp=="disabled" || tmp=="Disabled"){
		val=1; 
	}else{
		qDebug()<<childKey<<" value in channel "<<channel<<" must be one of: enabled or disabled , you have: "<<tmp;
		abort();
	}
		
	return val;
}

bool Configuration::OnOffKey(const QString &childKey, const QString &tmp){
	bool val=0;
	if(tmp=="off" || tmp=="Off"){
		val=0;
	}else if(tmp=="on" || tmp=="On"){
		val=1; 
	}else{
		qDebug()<<childKey<<" value must be one of: on or off , you have: "<<tmp;
		abort();
	}
		
	return val;
}
bool Configuration::OnOffChannel(const QString &childKey, const QString &tmp, int channel){
	bool val=0;
	if(tmp=="off" || tmp=="Off"){
		val=0;
	}else if(tmp=="on" || tmp=="On"){
		val=1; 
	}else{
		qDebug()<<childKey<<" value in channel "<<channel<<" must be one of: on or off , you have: "<<tmp;
		abort();
	}
		
	return val;
}

void Configuration::Validate(QString sectionname){
	
	QStringList fields,childs;
	bool general=false;
	bool chanmap=false;
	
	if (sectionname.compare("udp.setup",Qt::CaseSensitive)==0) { 
		fields<<"fec.port"<<"daq.port"<<"vmmasic.port"<<"vmmapp.port"<<"s6.port";
	} else if (sectionname.compare("trigger.daq",Qt::CaseSensitive)==0) {
		fields<<"tp.delay"<<"trigger.period"<<"acq.sync"<<"acq.window"<<"run.mode"<<"run.count"<<"output.path"<<"output.filename";
	} else if (sectionname.startsWith("hdmi.",Qt::CaseSensitive)) {
		fields<<"switch"<<"first"<<"second";
		chanmap=true;
	} else if (sectionname.compare("global.settings",Qt::CaseSensitive)==0) {
		fields<<"ch.polarity"<<"analog.tristates"<<"ch.leakage.current"<<"double.leakage"
		<<"dual.clock.ART"<<"monitor.pdo.out"<<"ADCs"<<"gain"<<"peak.time"<<"neighbor.trigger"<<"TAC.slop.adj"<<"disable.at.peak"
		<<"ART"<<"ART.mode"<<"out.buffer.mo"<<"out.buffer.pdo"<<"out.buffer.tdo"<<"channel.monitoring"<<"monitoring.control"
		<<"sub.hyst.discr"<<"direct.time"<<"direct.time.mode"
		<<"conv.mode.8bit"<<"enable.6bit"<<"ADC.10bit"<<"ADC.8bit"<<"ADC.6bit"
		<<"dual.clock.data"<<"dual.clock.6bit"
		<<"threshold.DAC"<<"test.pulse.DAC";
	} else if (sectionname.compare("general.info",Qt::CaseSensitive)==0) {
		fields<<"config.version"<<"ip"<<"vmm.id";
		general=true;
	} else if (sectionname.startsWith("channel.",Qt::CaseSensitive)) {
		fields<<"polarity"<<"capacitance"<<"leakage.current"<<"test.pulse"<<"hidden.mode"<<"trim"<<"monitor.mode"<<"s10bADC.time.set"<<"s08bADC.time.set"<<"s06bADC.time.set";
	} else {
		qDebug()<< sectionname<<" doesn't exist as an XML config tag name, check config file.";
		abort();
	}
	
	QDomNodeList infos;
	if (chanmap) infos = n_sub.childNodes();
	else infos = n->childNodes();
	for (int j=0; j<infos.count(); ++j) {
		childs<<infos.item(j).nodeName();
		//if (debug) qDebug()<<infos.item(j).nodeName();
	}

	if ( (general && ((childs.contains("debug") && infos.size()>5) || (!childs.contains("debug") && infos.size()>4))) || (!general && infos.size()>fields.size()) ) {
		qDebug()<<"Error: more items than allowed in the " << sectionname << " section "<<infos.size();
		abort();
	}

	for (int i = 0; i < fields.size(); ++i){
		
		if(!childs.contains(fields.at(i))){
			qDebug()<<"Error: The configuration file: "<<file->fileName()<<"must contain a "<<fields.at(i)<<" item in the " << sectionname << " section"; 
			abort();
		}
		
	}
	
}

void Configuration::processReply(const QString &sentip, QUdpSocket* socket)
{
	if(debug) qDebug()<<"Calling to process datagrams for "<<sentip;
	//! [2]
	bool ok;
	QString datagramHex;

	boardip->clear();
	_replies.clear();
	if (debug) qDebug()<<boardip->toString();
	while (socket->hasPendingDatagrams()) {
		buffer.resize(socket->pendingDatagramSize());
		socket->readDatagram(buffer.data(), buffer.size(), boardip);
		if(debug) qDebug()<<"Received datagram (in hex): "<<buffer.toHex()<<" from vmm with ip: "<<boardip->toString()<<" and the message size is: "<<buffer.size();
		
		datagramHex.clear();
		datagramHex = buffer.mid(0,4).toHex();
		quint32 reqReceived = datagramHex.toUInt(&ok,16);
		if(reqReceived!=commandCounter){
			
			qDebug()<<"Command number received: "<<reqReceived<<" doesn't match at command number: "<<commandCounter;
			abort();
		}
		_replies.append(boardip->toString());
	}

	//Compare list of addresses that sent replies to the list of vmm addresses

	if(debug){
		foreach (const QString &ip, _replies){
			qDebug()<<"VMM with IP: "<<ip<<" sent a reply to command: "<<commandCounter;
			
		}
	}
	
		
	if(_replies.size()>1){
		foreach (const QString &ip, _replies){

			if(ip != sentip){
				qDebug()<<"VMM with IP: "<<ip<<" sent a packet at command number: "<<commandCounter<<" which was not actually sent to it.	Out of synch, aborting";
				abort();
			}
		}
	}
	
	if(!_replies.contains(sentip)){
		qDebug()<<"VMM with IP: "<<sentip<<" did not acknowledge command number: "<<commandCounter;
		abort();
	}

	if(debug) qDebug()<<"IP processing ending for "<<sentip;

	//! [2]

}

void Configuration::dumpReply(const QString &sentip){
	QByteArray dummyb;
	if(debug) qDebug()<<"Calling to drop datagrams for "<<sentip;
	//! [2]
	boardip->clear();
	_replies.clear();
	//qDebug()<<boardip->toString();
	while (socket->hasPendingDatagrams()) {
		//	QByteArray dummyb;
		dummyb.resize(socket->pendingDatagramSize());
		//socket->readDatagram(dummyb.data(), dummyb.size());
			socket->readDatagram(dummyb.data(), dummyb.size(), boardip);
		//qDebug()<<tr("Received datagram: \"%1\"").arg(dummyb.data());
		//qDebug()<<"Received datagram (in hex): "<<dummyb.toHex();
			if(debug) qDebug()<<"Received datagram (in hex): "<<dummyb.toHex()<<" from vmm with ip: "<<boardip->toString()<<" and the message size is: "<<dummyb.size();
		
		bool ok;
		QString datagramBin,datagramHex;
		datagramHex = dummyb.mid(0,4).toHex();
		quint32 reqReceived =datagramHex.toUInt(&ok,16);
		if(reqReceived!=commandCounter){
			
			qDebug()<<"Command number received: "<<reqReceived<<" doesn't match at command number: "<<commandCounter;
			abort();
		}
		_replies.append(boardip->toString());
	}

	//Compare list of addresses that sent replies to the list of vmm addresses

	if(debug){
		foreach (const QString &ip, _replies){
			qDebug()<<"VMM with IP: "<<ip<<" sent a reply to command: "<<commandCounter;
			
		}
	}
	
		
	if(_replies.size()>1){
		foreach (const QString &ip, _replies){

			if(ip != sentip){
				qDebug()<<"VMM with IP: "<<ip<<" sent a packet at command number: "<<commandCounter<<" which was not actually sent to it.	Out of synch, aborting";
				abort();
			}
		}
	}
	
	if(!_replies.contains(sentip)){
		qDebug()<<"VMM with IP: "<<sentip<<" did not acknowledge command number: "<<commandCounter;
		abort();
	}

	if(debug) qDebug()<<"IP processing ending for "<<sentip;

	//! [2]

}
