#ifndef EVENT_BUILDER_H
#define EVENT_BUILDER_H

//qt
#include <QObject>
#include <QByteArray>
#include <QString>

//stl/std
#include <vector>

////ROOT
//#include "TROOT.h"
//#include "TTree.h"
//#include "TBranch.h"
//#include "TFile.h"

class EventBuilder : public QObject
{
    Q_OBJECT

    public :
        explicit EventBuilder(QObject *parent = 0);
        virtual ~EventBuilder(){};

    private :

    signals :

    public slots :

    private slots :

}; // class EventBuilder



#endif
