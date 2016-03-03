# ----------------------------------------------- #
#
#  vmmexec
#
# daniel.joseph.antrim@cern.ch
# March 2016
#
# ----------------------------------------------- #

sourcepath  = "../src"
includepath = "../include"
configpath  = "../include"


QT += core
QT += network
CONFIG += console
CONFIG += declarative_debug

TARGET = vmmexec
TEMPLATE = app

INCLUDEPATH += $$includepath
INCLUDEPATH += /usr/local/include/
DEPENDPATH  += $$includepath
DEPENDPATH += /usr/local/include/

CXXFLAGS += -std=c++0x

#LIBS += -L/usr/local/lib -lboost_system -lboost_filesystem

OBJECTS_DIR += ./objects/
MOC_DIR     += ./moc/
RCC_DIR     += ./rcc/
UI_DIR      += ./ui/

SOURCES     += $$sourcepath/vmmexec.cpp\
               $$sourcepath/config_handler.cpp

HEADERS     += $$includepath/config_handler.h
