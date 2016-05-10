#-------------------------------------------------
#
# Project created by QtCreator 2012-06-18T11:31:24
#
#-------------------------------------------------

sourcepath= "../src"
includepath="../include"
boostinclude="../include/boost"
#boostinclude="/Users/dantrim/boost_1_60_0/"

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
           $$sourcepath/daqconfig.cpp\
           $$sourcepath/srsconfig.cpp\
           $$sourcepath/detectorconfig.cpp\
           $$sourcepath/fec.cpp\
           $$sourcepath/chip.cpp\
           $$sourcepath/multilayer.cpp\
           $$sourcepath/readout.cpp\
           $$sourcepath/layer.cpp\
           $$sourcepath/chamberspecs.cpp\
           $$sourcepath/chamber.cpp\
           $$sourcepath/connectorspecs.cpp\
           $$sourcepath/connector.cpp\
           $$sourcepath/AsioService.cpp\
           $$sourcepath/sharedmemorywriter.cpp\
           $$sourcepath/createevents.cpp\
           $$sourcepath/event.cpp\
           $$sourcepath/coordinates.cpp

HEADERS  += $$includepath/mainwindow.h\
            $$includepath/run_module.h\
            $$includepath/config_handler.h\
            $$includepath/configuration_module.h\
            $$includepath/socket_handler.h\
            $$includepath/vmmsocket.h\
            $$includepath/data_handler.h\
            $$includepath/calibration_module.h\
            $$includepath/message_handler.h\
            $$includepath/daqconfig.h\
            $$includepath/srsconfig.h\
            $$includepath/detectorconfig.h\
            $$includepath/fec.h\
            $$includepath/chip.h\
            $$includepath/multilayer.h\
            $$includepath/readout.h\
            $$includepath/layer.h\
            $$includepath/chamberspecs.h\
            $$includepath/chamber.h\
            $$includepath/connectorspecs.h\
            $$includepath/connector.h\
            $$includepath/AsioService.h\
            $$includepath/sharedmemorywriter.h\
            $$includepath/createevents.h\
            $$includepath/event.h\
            $$includepath/coordinates.h 

FORMS    += $$sourcepath/mainwindow.ui

#RESOURCES += \
#    $$sourcepath_/icons.qrc \
#    $$sourcepath_/calibration_data.qrc

