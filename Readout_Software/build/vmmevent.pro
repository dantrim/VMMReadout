#------------------------------------
#
# QT project - eventbuild
#
# daniel.joseph.antrim@cern.ch
# November 2015
#
#------------------------------------

sourcepath_="../source"
headerpath_="../include"
configpath_="../include"

QT += core
QT += network
CONFIG += console
CONFIG += declarative_debug

TARGET = vmmrun
TEMPLATE = app

INCLUDEPATH += $(ROOTSYS)/include
LIBS += -L$(ROOTSYS)/lib -lCore -lCint -lRIO -lNet \
            -lHist -lGraf -lGraf3d -lGpad -lTree \
            -lRint -lPostscript -lMatrix -lPhysics \
            -lGui -lRGL -lMathCore

INCLUDEPATH += $$headerpath_/
DEPENDPATH  += $$headerpath_/

OBJECTS_DIR += ./objects/
MOC_DIR +=./moc/
RCC_DIR += ./rcc/
UI_DIR += ./ui/

SOURCES += $$sourcepath_/eventbuilder.cpp
HEADERS += $$headerpath_/eventbuilder.h


