# ---------------------------------------- #
#
# vmmdcs
#
# daniel.joseph.antrim@cern.ch
# March 2016
# 
# ---------------------------------------- #

sourcepath      = "../src"
includepath     = "../include"
configpath      = "../include"
boostinclude    = "/Users/dantrim/boost_1_60_0/"

QT += core gui
QT += network
QT += widgets

CONFIG += declarative_debug

TARGET = vmmdcs
TEMPLATE = app

# ROOT
INCLUDEPATH += $(ROOTSYS)/include
win32:LIBS += -L$(ROOTSYS)/lib -llibCint -llibRIO -llibNet \
              -llibHist -llibGraf -llibGraf3d -llibGpad -llibTree \
              -llibRint -llibPostscript -llibMatrix -llibPhysics \
              -llibGui -llibRGL -llibMathCore
else:LIBS += -L$(ROOTSYS)/lib -lCore -lCint -lRIO -lNet \
             -lHist -lGraf -lGraf3d -lGpad -lTree \
             -lRint -lPostscript -lMatrix -lPhysics \
             -lGui -lRGL -lMathCore

# OUR STUFF
INCLUDEPATH += $$includepath
INCLUDEPATH += $$boostinclude
DEPENDPATH  += $$includepath
DEPENDPATH  += $$boostinclude

QMAKE_CXXFLAGS += -stdlib=libc++
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS   += -stdlib=libc++

OBJECTS_DIR     += ./objects/
MOC_DIR         += ./moc/
RCC_DIR         += ./rcc/
UI_DIR          += ./ui/

SOURCES     += $$sourcepath/vmmdcs.cpp \
               $$sourcepath/run_module.cpp \
               $$sourcepath/config_handler.cpp \
               $$sourcepath/configuration_module.cpp \
               $$sourcepath/socket_handler.cpp \
               $$sourcepath/vmmsocket.cpp \
               $$sourcepath/data_handler.cpp

HEADERS     += $$includepath/run_module.h \
               $$includepath/config_handler.h \
               $$includepath/configuration_module.h \
               $$includepath/socket_handler.h \
               $$includepath/vmmsocket.h \
               $$includepath/data_handler.h

FORMS       += $$sourcepath/vmmdcs.ui

# RESOURCES   += $$sourcepath/icons.qrc \
#                $$sourcepath/calibration_data.qrc
                
