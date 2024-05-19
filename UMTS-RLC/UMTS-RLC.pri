#TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


SOURCES += \
        $$PWD/MAC/MACEngine.cpp \
        $$PWD/MAC/MACLayer.cpp \
        $$PWD/MAC/TransportBlock.cpp \
        $$PWD/RLC/Configs/URlcConfigAm.cpp \
        $$PWD/RLC/Configs/URlcConfigAmUm.cpp \
        $$PWD/RLC/Recv/URlcRecv.cpp \
        $$PWD/RLC/Recv/URlcRecvAm.cpp \
        $$PWD/RLC/Recv/URlcRecvAmUm.cpp \
        $$PWD/RLC/Recv/URlcRecvTm.cpp \
        $$PWD/RLC/Recv/URlcRecvUm.cpp \
        $$PWD/RLC/Tran/URlcTrans.cpp \
        $$PWD/RLC/Tran/URlcTransAm.cpp \
        $$PWD/RLC/Tran/URlcTransAmUm.cpp \
        $$PWD/RLC/Tran/URlcTransTm.cpp \
        $$PWD/RLC/Tran/URlcTransUm.cpp \
        $$PWD/RLC/URlcAm.cpp \
        $$PWD/RLC/URlcBase.cpp \
        $$PWD/RLC/URlcBasePdu.cpp \
        $$PWD/RLC/URlcPair.cpp \
        $$PWD/RLC/URlcPdu.cpp \
        $$PWD/RRC/Configs/RrcDefs.cpp \
        $$PWD/RRC/Configs/RrcMasterChConfig.cpp \
        $$PWD/RRC/Configs/URRCTrCh.cpp \
        $$PWD/RRC/URRC.cpp \
        $$PWD/RRC/URRCMessages.cpp \
        $$PWD/URlc.cpp \
        $$PWD/Ue/MSUEAdapter.cpp \
        $$PWD/Ue/RBInfo.cpp \
        $$PWD/Ue/Sgsn.cpp \
        $$PWD/Ue/UEInfo.cpp \
        $$PWD/Utilities/BitVector.cpp \
        $$PWD/Utilities/ByteVector.cpp \
        $$PWD/Utilities/LinkedLists.cpp \
        $$PWD/Utilities/Logger.cpp \
        $$PWD/Utilities/Threads.cpp \
        $$PWD/Utilities/Time.cpp \
        $$PWD/Utilities/Timeval.cpp \
        $$PWD/Utilities/Utils.cpp \
        $$PWD/Utilities/Z100Timer.cpp

HEADERS += \
    $$PWD/MAC/MACEngine.h \
    $$PWD/MAC/MACLayer.h \
    $$PWD/MAC/TransportBlock.h \
    $$PWD/RLC/Configs/DownlinkRlcStatusInfo.h \
    $$PWD/RLC/Configs/TransmissionRlcDiscard.h \
    $$PWD/RLC/Configs/URlcConfigAm.h \
    $$PWD/RLC/Configs/URlcConfigAmUm.h \
    $$PWD/RLC/Configs/URlcConfigUm.h \
    $$PWD/RLC/Configs/URlcInfo.h \
    $$PWD/RLC/Recv/URlcRecv.h \
    $$PWD/RLC/Recv/URlcRecvAm.h \
    $$PWD/RLC/Recv/URlcRecvAmUm.h \
    $$PWD/RLC/Recv/URlcRecvTm.h \
    $$PWD/RLC/Recv/URlcRecvUm.h \
    $$PWD/RLC/RrcUplinkMessage.h \
    $$PWD/RLC/Tran/URlcTrans.h \
    $$PWD/RLC/Tran/URlcTransAm.h \
    $$PWD/RLC/Tran/URlcTransAmUm.h \
    $$PWD/RLC/Tran/URlcTransTm.h \
    $$PWD/RLC/Tran/URlcTransUm.h \
    $$PWD/RLC/URlcAm.h \
    $$PWD/RLC/URlcBase.h \
    $$PWD/RLC/URlcBasePdu.h \
    $$PWD/RLC/URlcDownSdu.h \
    $$PWD/RLC/URlcPair.h \
    $$PWD/RLC/URlcPdu.h \
    $$PWD/RRC/Configs/RrcDefs.h \
    $$PWD/RRC/Configs/RrcMasterChConfig.h \
    $$PWD/RRC/Configs/RrcPollingInfo.h \
    $$PWD/RRC/Configs/UMTSPhCh.h \
    $$PWD/RRC/Configs/URRCTrCh.h \
    $$PWD/RRC/UEDefs.h \
    $$PWD/RRC/URRC.h \
    $$PWD/RRC/URRCMessages.h \
    $$PWD/URlc.h \
    $$PWD/Ue/InitialUE-Identity.h \
    $$PWD/Ue/MSUEAdapter.h \
    $$PWD/Ue/RBInfo.h \
    $$PWD/Ue/Sgsn.h \
    $$PWD/Ue/UEInfo.h \
    $$PWD/Ue/UEState.h \
    $$PWD/Utilities/BitVector.h \
    $$PWD/Utilities/ByteVector.h \
    $$PWD/Utilities/Interthread.h \
    $$PWD/Utilities/LinkedLists.h \
    $$PWD/Utilities/Logger.h \
    $$PWD/Utilities/Threads.h \
    $$PWD/Utilities/Time.h \
    $$PWD/Utilities/Timeval.h \
    $$PWD/Utilities/Utils.h \
    $$PWD/Utilities/Vector.h \
    $$PWD/Utilities/Z100Timer.h \
    $$PWD/configurations.h \
    $$PWD/shareTypes.h

    QMAKE_INCDIR += $$PWD
win32 {
    LIBS+= '-L$$PWD\Externals\pthreads-w32-2-9-1-release\Pre-built.2\lib\x86 -lpthreadVC2'
    LIBS+= '-L$$PWD\Externals\pthreads-w32-2-9-1-release\Pre-built.2\lib\x64 -lpthreadVC2'
    QMAKE_INCDIR += '$$PWD\Externals\pthreads-w32-2-9-1-release\Pre-built.2\include'
    LIBS += -lws2_32
}
linux {
    #at first do : sudo apt install libpthread-stubs0-dev
    LIBS += -pthread
}

