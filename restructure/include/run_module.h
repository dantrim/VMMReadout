#ifndef RUN_MODULE_H
#define RUN_MODULE_H

// qt
#include <QObject>

class RunModule : public QObject
{
    Q_OBJECT

    public :
        explicit RunModule(QObject *parent = 0);
        virtual ~RunModule(){};




}; // class RunModule

#endif
