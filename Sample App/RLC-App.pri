TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

include(../umts-rlc-lib.pri)
SOURCES +=$$PWD/UMTS-RLC.cpp

TARGET=RLCAPP
DESTDIR=$$PWD/../out

#Specifies the directory for Qt Compiler output files in the debug and release mode.
win32   { BUILDDIR = $$_PRO_FILE_PWD_/out/build/Qt/$${APPNAME} }
unix    { BUILDDIR = $$_PRO_FILE_PWD_/out/build/Qt/$${APPNAME} }
android { BUILDDIR = $$_PRO_FILE_PWD_/out/build/Qt/$${APPNAME} }

CONFIG(release, debug|release) {
        OBJECTS_DIR = $${BUILDDIR}/release
        MOC_DIR     = $${BUILDDIR}/release
        RCC_DIR     = $${BUILDDIR}/release
        UI_DIR      = $${BUILDDIR}/release
        DESTDIR     = build
}

CONFIG(debug, debug|release) {
        OBJECTS_DIR = $${BUILDDIR}/debug
        MOC_DIR     = $${BUILDDIR}/debug
        RCC_DIR     = $${BUILDDIR}/debug
        UI_DIR      = $${BUILDDIR}/debug
        DESTDIR     = build
}
