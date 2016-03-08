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
#boostinclude = "/usr/local/include/"
boostinclude = "/Users/dantrim/boost_1_60_0/"
#boostlib     = "/usr/local/lib"


QT += core
QT += network
CONFIG += console
CONFIG += declarative_debug

TARGET   = vmmexec
TEMPLATE = app

INCLUDEPATH += $$includepath
INCLUDEPATH += $$boostinclude
DEPENDPATH  += $$includepath
DEPENDPATH  += $$boostinclude

QMAKE_CXXFLAGS += -stdlib=libc++
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS   += -stdlib=libc++

#LIBS += -L$$boostlib -lboost_system -lboost_filesystem

OBJECTS_DIR += ./objects/
MOC_DIR     += ./moc/
RCC_DIR     += ./rcc/
UI_DIR      += ./ui/

SOURCES     += $$sourcepath/vmmexec.cpp\
               $$sourcepath/run_module.cpp\
               $$sourcepath/config_handler.cpp\
               $$sourcepath/configuration_module.cpp\
               $$sourcepath/socket_handler.cpp\
               $$sourcepath/vmmsocket.cpp\
               $$sourcepath/data_handler.cpp

HEADERS     += $$includepath/config_handler.h\
               $$includepath/configuration_module.h\
               $$includepath/run_module.h\
               $$includepath/socket_handler.h\
               $$includepath/vmmsocket.h\
               $$includepath/data_handler.h
