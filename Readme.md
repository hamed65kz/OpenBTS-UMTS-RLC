
# OpenBTS-UMTS-RLC

OpenBTS-UMTS-RLC is a library that manages the RLC Layer functionalities of UMTS network. It is derived from the OpenBTS-UMTS (https://github.com/PentHertz/OpenBTS) project and specifically focuses on the RLC component of the OpenBTS-UMTS system.

This Repository includes a Sample App more over the main library for testing and showing how to use library methods.

It supports both Cmake and Qmake build system.



## API Reference

#### Initialize RLC Configuration

Call it only one time at project startup. you can create multiple data path for different NodeB. 
```
    initRLCConfigs(int nodeBCount); 
```

#### Push RX CCCH Frame from MAC to RRC
Call it on each RX CCCH frame received in the MAC layer.
if you have multiple NodeB you should specify its index with nodeBIndex
```
    static void macPushUpRxCCCH(char* bit_pdu, int pdu_len, int nodeBIndex); 
```
#### Push RX DCCH Frame from MAC to RRC
Call it on each RX DCCH frame received in the MAC layer
```
    static void macPushUpRXDCCH(char* bit_pdu, int pdu_len, RbId rbid, UeIdType id_type,int UEid, int nodeBIndex);
```
It takes PDU and Radio Bearer index and UEid and UeIdType(CRNTI/URNTI) and 
if you have multiple NodeB you should specify its index with nodeBIndex

#### RRC get RX CCCH from RLC 
Call it in the RRC Layer to get Sdu's from RLC
```
    static RlcSdu* rrcRecvCCCH();
```
 It returns just Sdu
 Call it in an infinite loop and on a separate thread.
 
> It is a Blocking read and the thread will be blocked until Rx Sdu
> arrives.  
> *Dont forget to release RlcSdu object and its inner memory after analyzing it.*

#### RRC get RX DCCH from RLC 
 Call it in the RRC Layer to get Sdu's from RLC
```
    static RlcSdu* rrcRecvDCCH();
``` 
 It return Sdu + Readio Bearer index and Ueid
 Call it in an infinite loop and on a separate thread
 *

> It is a Blocking read and the thread will be blocked until Rx Sdu
> arrives.  
> Dont forget to release RlcSdu object and its inner memory
> after analyzing it.

#### RRC send TX DCCH Through RLC 
 Call it in the RRC Layer for sending DCCH Sdu's through RLC
```
    static void rrcSendDCCH(char* sdu,int sdu_len, UeIdType id_type, int UEid, RbId rbid,std::string desc, int nodeBIndex);
```
 It takes Sdu + id_type(URNTI/CRNTI) + UeId + Readio Bearer + description text to show in logs
 if you have multiple NodeB you should specify its index with nodeBIndex
 
#### RRC send TX CCCH Through RLC 
Call it in RRC Layer for sending CCCH Sdu's (Except RRC Connection Setup) through RLC
```
    static void rrcSendCCCH(char* sdu,int sdu_len, std::string desc, int nodeBIndex);
```
  It takes Sdu + description text to show in logs.
  if you have multiple NodeB you should specify its index with nodeBIndex

#### RRC send TX CCCH Through RLC
 Call it in the RRC Layer just for sending RRC Connection Setup Sdu through RLC
```
    static void rrcSendRRCConnectionSetup(uint32_t urnti, uint16_t crnti,char* sdu,int sdu_len, int nodeBIndex);
```
 It takes Sdu + Ue URNTI and CRNTI.
 if you have multiple NodeB you should specify its index with nodeBIndex
 
#### MAC get TX Pdu's from RLC

Call it in MAC Layer  for getting vector of Pdu's for sending through MAC.

```
    static ::std::vector<RlcPdu*> macReadTx();
```
   
It returns vector of CCCH/DCCH Pdu's.
Call it in an infinite loop and on a separate thread.
*

> Dont forget to release RlcPdu object and its inner memory after
> sending it.




## Installation

It just needs Pthread on Linux. use the below command for installing Pthread.  

```
  sudo apt install libpthread-stubs0-dev 
```
    
## License

[AGPL-3.0 license]


## Features
OpenBTS-UMTS-RLC just support FACH/RACH packets.

## Platforms
OpenBTS-UMTS-RLC compiled successfully with MSVC and MinGW on Windows and with gcc/g++ on Linux

## How to Use

#### CMake Build System 
###### For building the sample app

    Create a Build directory at root of the project
    cd Build
    cmake ..
    cmake --build

###### For building RLC Library

    Include UMTS-RLC/CMakeLists.txt from your main CMake file(use add_subdirectory)
    Add URlc.h to your main application
    Build your main application

#### QMake Build System 
###### For building the sample app

    Open umts-rlc-proj.pro in qt creator and build it

###### For building the RLC Library
    Include umts-rlc-proj.pri in your main .pro file (use include command)
    Add URlc.h to your main application
    Build your main application
    



    

