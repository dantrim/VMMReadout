#
# project file for vmmdcs
#
# daniel.joseph.antrim@cern.ch
# July 2016
#

#####################################################
# directory and machine specific items here
#####################################################
linebreak="---------------------------------------------------------------"


sourcepath=""
includepath=""
boostinclude=""
boostlib=""
imagepath=""
type=""

linux {
    sourcepath="../src"
    includepath="../include"
    boostinclude="/Software/boost/boost_1_60_0/include/boost"
    boostlib="/Software/boost/boost_1_60_0/lib"
    imagepath="../images"
} else {
    sourcepath="../src"
    includepath="../include"
    boostinclude="/Users/dantrim/boost_1_60_0/"
    boostlib="/usr/local/opt/boost/lib"
    imagepath="../images"
}

message($$linebreak)
message("qmake sourcepath:      $$sourcepath")
message("qmake includepath:     $$includepath")
message("qmake boostinclude:    $$boostinclude")
message("qmake boostlib:        $$boostlib")
message("qmake imagepath:       $$imagepath")
message("ROOTSYS:               $(ROOTSYS)")
message($$linebreak)

#####################################################

QT      += core gui
QT      += network
QT      += widgets
QT      += xml
CONFIG  += console
CONFIG  +=declarative_debug
CONFIG  +=c++11

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

linux {
    LIBS += -L$$boostlib -lboost_thread -lboost_filesystem -lboost_system -lrt

} else {
    LIBS +=  -L$$boostlib -lboost_thread-mt -lboost_filesystem  -lboost_system
}

LIBS += -L./objects -lMylib

INCLUDEPATH += $$includepath
DEPENDPATH  += $$includepath
INCLUDEPATH += $$includepath/mapping/
DEPENDPATH  += $$includepath/mapping/
INCLUDEPATH += $$includepath/monitoring/
DEPENDPATH  += $$includepath/monitoring/
INCLUDEPATH += $$boostinclude
DEPENDPATH  += $$boostinclude

OBJECTS_DIR += ./objects/
MOC_DIR     += ./moc/
RCC_DIR     += ./rcc/
UI_DIR      += ./ui/

linux {
    QMAKE_CXXFLAGS += -std=c++11
} else {
    QMAKE_CXXFLAGS += -stdlib=libc++
    QMAKE_CXXFLAGS += -std=c++11
    QMAKE_LFLAGS   += -stdlib=libc++
}

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
           $$sourcepath/daq_monitor.cpp\
           ## mapping related below
            $$sourcepath/mapping/readout.cpp\
            $$sourcepath/mapping/multilayer.cpp\
            $$sourcepath/mapping/layer.cpp\
            $$sourcepath/mapping/febconfig.cpp\
            $$sourcepath/mapping/feb.cpp\
            $$sourcepath/mapping/element.cpp\
            $$sourcepath/mapping/detectorconfig.cpp\
            $$sourcepath/mapping/daqconfig.cpp\
            $$sourcepath/mapping/coordinates.cpp\
            $$sourcepath/mapping/connectorinfo.cpp\
            $$sourcepath/mapping/connector.cpp\
            $$sourcepath/mapping/chip.cpp\
            $$sourcepath/mapping/chamber.cpp\
            $$sourcepath/mapping/map_handler.cpp\
           ## monitoring related below
            $$sourcepath/monitoring/OnlineMonTool.cpp
           
          ## $$sourcepath/monitoring/daqconfig.cpp\
          ## $$sourcepath/monitoring/srsconfig.cpp\
          ## $$sourcepath/monitoring/detectorconfig.cpp\
          ## $$sourcepath/monitoring/fec.cpp\
          ## $$sourcepath/monitoring/chip.cpp\
          ## $$sourcepath/monitoring/multilayer.cpp\
          ## $$sourcepath/monitoring/readout.cpp\
          ## $$sourcepath/monitoring/layer.cpp\
          ## $$sourcepath/monitoring/chamberspecs.cpp\
          ## $$sourcepath/monitoring/chamber.cpp\
          ## $$sourcepath/monitoring/connectorspecs.cpp\
          ## $$sourcepath/monitoring/connector.cpp\
          ## $$sourcepath/monitoring/AsioService.cpp\
          ## $$sourcepath/monitoring/sharedmemorywriter.cpp\
          ## $$sourcepath/monitoring/createevents.cpp\
          ## $$sourcepath/monitoring/event.cpp\
          ## $$sourcepath/monitoring/coordinates.cpp

HEADERS  += $$includepath/mainwindow.h\
            $$includepath/run_module.h\
            $$includepath/config_handler.h\
            $$includepath/configuration_module.h\
            $$includepath/socket_handler.h\
            $$includepath/vmmsocket.h\
            $$includepath/data_handler.h\
            $$includepath/calibration_module.h\
            $$includepath/message_handler.h\
            $$includepath/daq_monitor.h\
            ## mapping related below
            $$includepath/mapping/readout.h\
            $$includepath/mapping/multilayer.h\
            $$includepath/mapping/layer.h\
            $$includepath/mapping/febconfig.h\
            $$includepath/mapping/feb.h\
            $$includepath/mapping/element.h\
            $$includepath/mapping/detectorconfig.h\
            $$includepath/mapping/daqconfig.h\
            $$includepath/mapping/coordinates.h\
            $$includepath/mapping/connectorinfo.h\
            $$includepath/mapping/connector.h\
            $$includepath/mapping/chip.h\
            $$includepath/mapping/chamber.h\
            $$includepath/mapping/map_handler.h\
            ## monitoring related below
            $$includepath/monitoring/OnlineMonTool.h
           # $$includepath/monitoring/daqconfig.h\
           # $$includepath/monitoring/srsconfig.h\
           # $$includepath/monitoring/detectorconfig.h\
           # $$includepath/monitoring/fec.h\
           # $$includepath/monitoring/chip.h\
           # $$includepath/monitoring/multilayer.h\
           # $$includepath/monitoring/readout.h\
           # $$includepath/monitoring/layer.h\
           # $$includepath/monitoring/chamberspecs.h\
           # $$includepath/monitoring/chamber.h\
           # $$includepath/monitoring/connectorspecs.h\
           # $$includepath/monitoring/connector.h\
           # $$includepath/monitoring/AsioService.h\
           # $$includepath/monitoring/sharedmemorywriter.h\
           # $$includepath/monitoring/createevents.h\
           # $$includepath/monitoring/event.h\
           # $$includepath/monitoring/coordinates.h 

FORMS    += $$sourcepath/mainwindow.ui

RESOURCES += \
    $$imagepath/icons.qrc
#    $$sourcepath_/icons.qrc \
#    $$sourcepath_/calibration_data.qrc

