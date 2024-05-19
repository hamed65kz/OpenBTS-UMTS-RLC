TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

include(../umts-rlc-lib.pri)
SOURCES +=$$PWD/UMTS-RLC.cpp

TARGET=RLCAPP
