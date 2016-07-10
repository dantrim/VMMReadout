#-------------------------------------------------
#
# Project created by QtCreator 2012-06-18T11:31:24
#
#-------------------------------------------------

sourcepath= "../src"
includepath="../include"
boostinclude="../include/boost"
#boostinclude="/Users/dantrim/boost_1_60_0/"
imagepath="../images"

QT       += core gui
QT       += network
QT       += widgets
QT       += xml
CONFIG       += console
CONFIG+=declarative_debug
CONFIG+=c++11

TARGET   = vmmdcs
TEMPLATE = app

INCLUDEPATH += $(ROOTSYS)/include
win32:LIBS += -L$(ROOTSYS)/lib -llibCint -llibRIO -llibNet \
       -llibHist -llibGraf -llibGraf3d -llibGpad -llibTree \
       -llibRint -llibPostscript -llibMatrix -llibPhysics \
       -llibGui -llibRGL -llibMathCore
else:LIBS += -L$(ROOTSYS)/lib -lCore -lCint -lRIO -lNet \
       -lHist -lGraf -lGraf3d -lGpad -lTree \
       -lRint -lPostscript -lMatrix -lPhysics \
       -lGui -lMathCore #-lRGL -lMathCore
LIBS +=  -L/usr/local/opt/boost/lib -lboost_thread-mt -lboost_filesystem  -lboost_system

LIBS += -L./objects -lMylib

INCLUDEPATH += $$includepath
DEPENDPATH  += $$includepath
INCLUDEPATH += $$includepath/monitoring
DEPENDPATH  += $$includepath/monitoring
INCLUDEPATH += $$boostinclude
DEPENDPATH  += $$boostinclude

OBJECTS_DIR += ./objects/
MOC_DIR     += ./moc/
RCC_DIR     += ./rcc/
UI_DIR      += ./ui/

QMAKE_CXXFLAGS += -stdlib=libc++
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS   += -stdlib=libc++

SOURCES += $$sourcepath/main.cpp\
           $$sourcepath/mainwindow.cpp\
           $$sourcepath/run_module.cpp\
           $$sourcepath/config_handler.cpp\
           $$sourcepath/configuration_module.cpp\
           $$sourcepath/socket_handler.cpp\
           $$sourcepath/vmmsocket.cpp\
           $$sourcepath/data_handler.cpp\
           $$sourcepath/calibration_module.cpp\
           $$sourcepath/message_handler.cpp\
           ## monitoring related below
           $$sourcepath/monitoring/daqconfig.cpp\
           $$sourcepath/monitoring/srsconfig.cpp\
           $$sourcepath/monitoring/detectorconfig.cpp\
           $$sourcepath/monitoring/fec.cpp\
           $$sourcepath/monitoring/chip.cpp\
           $$sourcepath/monitoring/multilayer.cpp\
           $$sourcepath/monitoring/readout.cpp\
           $$sourcepath/monitoring/layer.cpp\
           $$sourcepath/monitoring/chamberspecs.cpp\
           $$sourcepath/monitoring/chamber.cpp\
           $$sourcepath/monitoring/connectorspecs.cpp\
           $$sourcepath/monitoring/connector.cpp\
           $$sourcepath/monitoring/AsioService.cpp\
           $$sourcepath/monitoring/sharedmemorywriter.cpp\
           $$sourcepath/monitoring/createevents.cpp\
           $$sourcepath/monitoring/event.cpp\
           $$sourcepath/monitoring/coordinates.cpp

HEADERS  += $$includepath/mainwindow.h\
            $$includepath/run_module.h\
            $$includepath/config_handler.h\
            $$includepath/configuration_module.h\
            $$includepath/socket_handler.h\
            $$includepath/vmmsocket.h\
            $$includepath/data_handler.h\
            $$includepath/calibration_module.h\
            $$includepath/message_handler.h\
            ## monitoring related below
            $$includepath/monitoring/daqconfig.h\
            $$includepath/monitoring/srsconfig.h\
            $$includepath/monitoring/detectorconfig.h\
            $$includepath/monitoring/fec.h\
            $$includepath/monitoring/chip.h\
            $$includepath/monitoring/multilayer.h\
            $$includepath/monitoring/readout.h\
            $$includepath/monitoring/layer.h\
            $$includepath/monitoring/chamberspecs.h\
            $$includepath/monitoring/chamber.h\
            $$includepath/monitoring/connectorspecs.h\
            $$includepath/monitoring/connector.h\
            $$includepath/monitoring/AsioService.h\
            $$includepath/monitoring/sharedmemorywriter.h\
            $$includepath/monitoring/createevents.h\
            $$includepath/monitoring/event.h\
            $$includepath/monitoring/coordinates.h 

FORMS    += $$sourcepath/mainwindow.ui

RESOURCES += \
    $$imagepath/icons.qrc
#    $$sourcepath_/icons.qrc \
#    $$sourcepath_/calibration_data.qrc

