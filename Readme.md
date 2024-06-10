
# OpenBTS-UMTS-RLC

OpenBTS-UMTS-RLC is a versatile library that manages the RLC Layer functionalities of UMTS networks. It is derived from the RangeNetworks/OpenBTS-UMTS project and specifically focuses on the RLC component of the OpenBTS-UMTS system.

This Repository includes a Sample App more over the main library for testing and showing how to use library methods.

it supports both Cmake and Qmake build system.



## API Reference

#### Initialize RLC Configuration


```
    //call it only one time at project startup
    initRLCConfigs(); 
```



#### Push RX CCCH Frame from MAC to RRC
```
    //call it on each RX CCCH frame received in the MAC layer
    static void macPushUpRxCCCH(char* bit_pdu, int pdu_len); 
```

#### Push RX DCCH Frame from MAC to RRC
```
    //call it on each RX DCCH frame received in the MAC layer
    static void macPushUpRXDCCH(char* bit_pdu, int pdu_len, RbId rbid, UeIdType id_type,int UEid);
    //It takes PDU and Radio Bearer index and UEid and UeIdType(CRNTI/URNTI)
```

#### RRC Get RX CCCH from RLC 
```
    //call it in the RRC Layer to get Sdu's from RLC
    static RlcSdu* rrcRecvCCCH();
    //It returns just Sdu
    // call it in an infinite loop and on a separate thread
```

#### RRC Get RX DCCH from RLC 
```
    //call it in the RRC Layer to get Sdu's from RLC
    static RlcSdu* rrcRecvDCCH();
    // It return Sdu + Readio Bearer index and Ueid
    // call it in an infinite loop and on a separate thread
    //It is a Blocking read and the thread will be blocked until Rx Sdu arrives.
```

#### RRC Send TX DCCH Through RLC
```
    //call it in the RRC Layer for sending DCCH Sdu's through RLC
    static void rrcSendDCCH(char* sdu,int sdu_len, UeIdType id_type, int UEid, RbId rbid,std::string desc);
    // it takes Sdu + id_type(URNTI/CRNTI) + UeId + Readio Bearer + description text for show in logs
```

#### RRC Send TX CCCH Through RLC
```
    //call it in RRC Layer for sending CCCH Sdu's (Except RRC Connection Setup) through RLC
    static void rrcSendCCCH(char* sdu,int sdu_len, std::string desc);
    //It takes Sdu + description text to show in logs
```

#### RRC Send TX CCCH Through RLC
```
    //call it in the RRC Layer just for sending RRC Connection Setup Sdu through RLC
    static void rrcSendRRCConnectionSetup(uint32_t urnti, uint16_t crnti,char* sdu,int sdu_len);
    // it takes Sdu + Ue urnti and crnti
```

#### MAC get TX Pdu's from RLC
```
    //call it in MAC Layer  for getting vector of Pdu's for sending through MAC
    static ::std::vector<RlcPdu*> macReadTx();
    //It returns vector of CCCH/DCCH Pdu's
    // call it in an infinite loop and on a separate thread
```



## Installation

It just needs Pthread on Linux. use the below command for installing pthread.  

```
  sudo apt install libpthread-stubs0-dev 
```
    
## License

[AGPL-3.0 license]


## Features
OpenBTS-UMTS-RLC just Support FACH packets.

## Platforms
OpenBTS-UMTS-RLC compiled successfully with MSVC and MinGW on Windows and with gcc/g++ on Linux

## How to Use

#### CMake Build System 
###### For building the sample app

    create a Build directory at root of the project
    cd Build
    cmake ..
    cmake --build

###### For building RLC Library

    include UMTS-RLC/CMakeLists.txt from your main CMake file(use add_subdirectory)
    build your main application

#### QMAKE Build System 
###### For building the sample app

    open umts-rlc-proj.pro in qt creator and build it

###### For building the RLC Library
    include umts-rlc-proj.pri in your main .pro file (use include command) and build yourself application
    



    

