// UMTS-RLC.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//

#ifdef _WIN32
#define HAVE_STRUCT_TIMESPEC // for resolve timespec redefinition,, msvc version                             // should be higher than vs2013
#define _TIMESPEC_DEFINED
#include <windows.h>
#define SLEEP(x) Sleep(x)
#elif __linux
#include<unistd.h> //for usleep
#define SLEEP(x) usleep(x*1000)
#include "string.h" // for memset
#endif

#include "URlc.h"

#include <iostream>
#include <vector>
#include <cstdlib>
#include <pthread.h>
#include <stdio.h>

using namespace std;
char *LoadFromBinaryCharFile(const char *Path, unsigned int &InputFileLength);

void *macTX(void *) {
    while (true) {
        vector<RlcPdu *> pdus = URlc::macReadTx();
        for (int i = 0; i < pdus.size(); i++) {
            //SEND PDU To MAC HERE
            cout << "<Tx Pdu Received on MAC" << " size : " << pdus[i]->payload_length << " paylod : " << pdus[i]->payload_string << "\n";
        }
        for (int i = pdus.size() - 1; i >= 0; i--) {
            delete[] pdus[i]->payload;
            delete pdus[i];
        }
        SLEEP(500);
    }
    pthread_exit(0);
    return NULL;
}
void *macCCCHRX(void *) {

    while (true) {
        char *ccchpdu;
        int sendRRCRequest = rand() % 2;
        if (sendRRCRequest > 0) {
            // SIMULATE SEND  RRC CONN REQ
            // GENERATE SAMPLE URNTI
            int randUrnti = rand() % 4;
            if (randUrnti > 4) {
                randUrnti = 1;
            }
            ccchpdu = new char[200]();
            ccchpdu[0] = randUrnti;
            ccchpdu[1] = 0;

            cout << "Rx CCCH RRC Req Pdu Received on MAC, urnti =" << randUrnti
                 << " , size = 2"  << "\n";
        } else {
            ccchpdu = new char[200]();
            cout << "Rx CCCH Pdu Received on MAC\n";
        }
        URlc::macPushUpRxCCCH(ccchpdu,200);
        SLEEP(1000);
    }
    pthread_exit(0);
    return NULL;
}
void *macDCCHRX(void *) {
    while(true)
    {
        for (size_t i = 1; i <= 110; i++) {
            unsigned int filelen = 0;
            char addr[500] = {0};
            sprintf(addr, "E:\\project repo\\UMTS RLC\\RLC-CMAKE\\Sample App\\mac_to_rrc_logs\\File_%03d.bin", i);
            printf("\n----%s\n", addr);
            char *data = LoadFromBinaryCharFile(addr, filelen);


            char* dcchpdu = new char[filelen - 24]();
            for (int j = 24; j < filelen; j++) {
                dcchpdu[j - 24] = data[j]; // REMOVE 24 BIT HEADERS
            }
            free(data);

            // while (true){
            int UEid = 1;
            RbId rb = 3;
            cout << "Rx DCCH Pdu Received on MAC\n";
            URlc::macPushUpRXDCCH(dcchpdu,filelen - 24, rb,UeIdType::URNTI, UEid);
            SLEEP(500);
            //}
        }
    }
    pthread_exit(0);
    return NULL;
}
void *rrcRxCCCH(void *) {
    while (true) {
        RlcSdu *sdu = URlc::rrcRecvCCCH();
        if (sdu != nullptr) {
            cout << "Rx CCCH Pdu Received on RRC\n";

            // SIMULATE RRC CONN SETUP
            if (sdu->payload_length > 0) {
                auto urnti = sdu->payload[0];
                auto crnti = sdu->payload[0];

                URlc::rrcSendRRCConnectionSetup(urnti,crnti, sdu->payload, sdu->payload_length);
                delete[] sdu->payload;
            }
            delete sdu;
        }
        SLEEP(500);
    }
    pthread_exit(0);
    return NULL;
}
void *rrcRxDCCH(void *) {
    while (true) {
        RlcSdu *sdu = URlc::rrcRecvDCCH();
        cout << "Rx DCCH Pdu Received on RRC\n";
        delete[] sdu->payload;
        delete sdu;
        SLEEP(500);
    }
    pthread_exit(0);
}
void *rrcTxCCCH(void *) {
    int count = 0;
    while (true) {
        char *ccchpdu = new char[200]();
        sprintf(ccchpdu, "ccch%6d", count);
        URlc::rrcSendCCCH(ccchpdu,200,"ccch sdu");
        SLEEP(500);
        count++;
    }
    pthread_exit(0);
    return NULL;
}
void *rrcTxDCCH(void *) {
    int count = 0;
    while (true) {
        char *dcchpdu = new char[200]();
        sprintf(dcchpdu, "dcch%6d", count);

        URlc::rrcSendDCCH(dcchpdu,200,UeIdType::URNTI, 1, 3, "");
        SLEEP(500);
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
    if(fod){
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
    return NULL;

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
    int crnti = 1;
    char* sdu = new char[2]();
    URlc::rrcSendRRCConnectionSetup(urnti,crnti, sdu,1);

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
