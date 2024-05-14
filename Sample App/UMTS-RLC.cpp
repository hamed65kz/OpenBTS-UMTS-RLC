// UMTS-RLC.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//

#ifdef _WIN32
#define HAVE_STRUCT_TIMESPEC // for resolve timespec redefinition,, msvc version                             // should be higher than vs2013
#define _TIMESPEC_DEFINED
#endif

#include "URlc.h"

#include <iostream>

#include <cstdlib>
#include <pthread.h>
#include <stdio.h>

// using namespace std;
char *LoadFromBinaryCharFile(const char *Path, unsigned int &InputFileLength);

void *macTX(void *) {

  while (true) {
    vector<ByteVector *> pdus = URlc::macReadTx();
    for (size_t i = pdus.size(); i >= 1; i--) {
      auto &item = pdus[i - 1];
      cout << "<Tx Pdu Received on MAC"
           << " size : " << item->sizeBits() << " paylod : " << item->hexstr()
           << "\n";
      delete item;
    }
    Sleep(1000);
  } 
  pthread_exit(0);
  return NULL;
}
void *macCCCHRX(void *) {
  return NULL;
  while (true) {
    BitVector *ccchpdu;
    int sendRRCRequest = rand() % 2;
    if (sendRRCRequest > 0) {
      int randUrnti = rand() % 40;
      if (randUrnti > 30) {
        randUrnti = 1;
      }
      Vector<char> v1 = Vector<char>(2);
      v1[0] = randUrnti;
      v1[1] = 0;
      ccchpdu = new BitVector(v1);
      cout << "Rx CCCH RRC Req Pdu Received on MAC, urnti =" << randUrnti
           << " , size = " << ccchpdu->size() << "\n";
    } else {
      ccchpdu = new BitVector();
      cout << "Rx CCCH Pdu Received on MAC\n";
    }
    URlc::macPushUpRxCCCH(ccchpdu);
    Sleep(1000);
  }
  pthread_exit(0);
  return NULL;
}
void *macDCCHRX(void *) {
  // while(true)
  //{
  for (size_t i = 1; i <= 110; i++) {
    unsigned int filelen = 0;
    char addr[500] = {0};
    sprintf(addr, "E:\\project repo\\UMTS RLC\\RLC-CMAKE\\Sample App\\mac_to_rrc_logs\\File_%03d.bin", i);
    printf("\n----%s\n", addr);
    char *data = LoadFromBinaryCharFile(addr, filelen);

    BitVector dcchpdu;
    Vector<char> v1 = Vector<char>(filelen - 24);
    for (int j = 24; j < filelen; j++) {
      v1[j - 24] = data[j];
    }
    free(data);
    dcchpdu = BitVector(v1);
    // while (true){
    int UEid = 1;
    RbId rb = 3;
    cout << "Rx DCCH Pdu Received on MAC\n";
    URlc::macPushUpRXDCCH(dcchpdu, rb, UEid);
    Sleep(1000);
    //}
  }
  //}
  pthread_exit(0);
  return NULL;
}
void *rrcRxCCCH(void *) {
  return NULL;
  while (true) {

    BitVector *pdu = URlc::rrcRecvCCCH();
    if (pdu != nullptr) {
      cout << "Rx CCCH Pdu Received on RRC\n";
      if (pdu->size() > 0) {
        auto urnti = pdu->mData[0];
        // AsnUeId asnid;
        // asnid.mPtmsi = urnti;
        ByteVector sdu;
        URlc::rrcSendRRCConnectionSetup(urnti, sdu);
        int a = 0;
      }
    }
    // delete pdu;
    Sleep(1000);
  }
  pthread_exit(0);
  return NULL;
}
void *rrcRxDCCH(void *) {
  while (true) {
    RrcUplinkMessage *pdu = URlc::rrcRecvDCCH();
    cout << "Rx DCCH Pdu Received on RRC\n";
    delete pdu;
    Sleep(1000);
  }
  pthread_exit(0);
}
void *rrcTxCCCH(void *) {
  return NULL;
  int count = 0;
  while (true) {
    char *tmp = new char[85]();
    sprintf(tmp, "ccch%6d", count);
    ByteVector ccchpdu = ByteVector(tmp, 10);
    // URlc::rrcSendCCCH(ccchpdu,"deschamed");
    Sleep(1000);
    count++;
  }
  pthread_exit(0);
  return NULL;
}
void *rrcTxDCCH(void *) {
   return NULL;
  int count = 0;
  while (true) {
    char *tmp = new char[85]();
    sprintf(tmp, "dcch%6d", count);
    ByteVector dcchpdu = ByteVector(tmp, 10);
    URlc::rrcSendDCCH(dcchpdu, 1, 3, "");
    Sleep(1000);
    count++;
  }
  pthread_exit(0);
  return NULL;
}

bool rxCCCHPathEnabled = true;
bool rxDCCHPathEnabled = true;
bool txPathEnabled = true;

bool macEnabled = true;
bool rrcEnabled = true;

char *LoadFromBinaryCharFile(const char *Path, unsigned int &InputFileLength) {
  // FILE* fod;
  // fopen_s(&fod, Path, "rb");
  FILE *fod = fopen(Path, "rb");
  fseek(fod, 0, SEEK_END);
  int bytelength = ftell(fod);
  fseek(fod, 0, SEEK_SET);

  InputFileLength = bytelength / sizeof(char);

  char *InPutFile = (char *)malloc(sizeof(char) * InputFileLength);
  memset(InPutFile, 0, sizeof(char) * InputFileLength);
  size_t size = fread(InPutFile, sizeof(char), InputFileLength, fod);
  fclose(fod);

  return InPutFile;
}
int main() {
  unsigned int l = 0;
  // char*
  // ff=LoadFromBinaryCharFile("C:\\Users\\APA\\Desktop\\Result\\File_01.txt",l);
  cout << "Hello World!" << endl;
  pthread_t macCCCHRxthread;
  pthread_t rrcCCCHRxthread;
  pthread_t macRxDCCHthread;
  pthread_t rrcRxDCCHthread;
  pthread_t rrctxCCCHthread;
  pthread_t rrctxDCCHthread;
  pthread_t mactxthread;

  URlc::initRLCConfigs();

  int urnti = 1;
  ByteVector sdu;
  URlc::rrcSendRRCConnectionSetup(urnti, sdu);

  if (rxCCCHPathEnabled) {
    if (macEnabled) {
      // macCCCHRxthread = std::thread(macCCCHRX, 2);
      pthread_create(&macCCCHRxthread, NULL, macCCCHRX, NULL);
    }
    if (rrcEnabled) {
      // rrcCCCHRxthread = std::thread(rrcRxCCCH);
      pthread_create(&rrcCCCHRxthread, NULL, rrcRxCCCH, NULL);
    }
  }

  if (rxDCCHPathEnabled) {
    if (macEnabled) {
      // macRxDCCHthread = std::thread(macDCCHRX);
      pthread_create(&macRxDCCHthread, NULL, macDCCHRX, NULL);
    }
    if (rrcEnabled) {
      // rrcRxDCCHthread = std::thread(rrcRxDCCH);
      pthread_create(&rrcRxDCCHthread, NULL, rrcRxDCCH, NULL);
    }
  }
  if (txPathEnabled) {

    if (rrcEnabled) {
      // rrctxCCCHthread = std::thread(rrcTxCCCH);
      // rrctxDCCHthread = std::thread(rrcTxDCCH);
      pthread_create(&rrctxCCCHthread, NULL, rrcTxCCCH, NULL);
      pthread_create(&rrctxDCCHthread, NULL, rrcTxDCCH, NULL);
    }
    if (macEnabled) {
      // mactxthread = std::thread(macTX);
      pthread_create(&mactxthread, NULL, macTX, NULL);
    }
  }

  pthread_join(macCCCHRxthread, (void**)NULL);
  pthread_join(rrcCCCHRxthread, (void**)NULL);
  pthread_join(macRxDCCHthread, (void**)NULL);
  pthread_join(rrcRxDCCHthread, (void**)NULL);
  pthread_join(rrctxCCCHthread, (void**)NULL);
  pthread_join(rrctxDCCHthread, (void**)NULL);
  pthread_join(mactxthread,  (void**)NULL);
  char c = getchar();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add
//   Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project
//   and select the .sln file
