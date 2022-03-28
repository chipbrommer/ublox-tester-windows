#include <iostream>
#include "Serial.h"
#include "uBloxGpsReceiver.h"
#include <stdio.h>
#include <string.h>

const char* portName = "\\\\.\\COM5";

int main()
{
    // SerialPort serial(portName);

    unsigned char readBuffer[1024];
    unsigned char sendBuffer[8];
    int bytesInBuffer = 0;
    int bytesRead;
    int bytesSent;
    uBloxGPS ublox;

    uint8_t test[28];
    test[0] = UBX::Header::SyncChar1;
    test[1] = UBX::Header::SyncChar2;
    test[2] = UBX::NAV::classId;
    test[3] = UBX::NAV::SAT::messageId;
    test[4] = 0x28;
    test[5] = 0x00;
    // itow
    test[6] = 0x01;
    test[7] = 0x00;
    test[8] = 0x00;
    test[9] = 0x00;
    // vers
    test[10] = 0x01;
    // num sv
    test[11] = 0x01;
    // res
    test[12] = 0x00;
    test[13] = 0x00;
    // sat 1
    test[14] = 0x03; 

    test[15] = 0x03;

    test[16] = 0x03;

    test[17] = 0x03;

    test[18] = 0x03;
    test[19] = 0x00;

    test[20] = 0x03;
    test[21] = 0x00;

    test[22] = 0x00;
    test[23] = 0x00;
    test[24] = 0x00;
    test[25] = 0x00;
    
    // check sum
    test[26] = 0x00;
    test[27] = 0x00;

    ublox.setUbxChecksum(test);
    ublox.printMessageInHex(test, sizeof(test));
    ublox.handleUbxMessage(test);

    uint8_t test2[28];
    test2[0] = UBX::Header::SyncChar1;
    test2[1] = UBX::Header::SyncChar2;
    test2[2] = UBX::NAV::classId;
    test2[3] = UBX::NAV::SAT::messageId;
    test2[4] = 0x28;
    test2[5] = 0x00;
    // itow
    test2[6] = 0x01;
    test2[7] = 0x00;
    test2[8] = 0x00;
    test2[9] = 0x00;
    // vers
    test2[10] = 0x01;
    // num sv
    test2[11] = 0x01;
    // res
    test2[12] = 0x00;
    test2[13] = 0x00;
    // sat 1
    test2[14] = 0x02;

    test2[15] = 0x02;

    test2[16] = 0x03;

    test2[17] = 0x03;

    test2[18] = 0x03;
    test2[19] = 0x00;

    test2[20] = 0x03;
    test2[21] = 0x00;

    test2[22] = 0x00;
    test2[23] = 0x00;
    test2[24] = 0x00;
    test2[25] = 0x00;

    // check sum
    test2[26] = 0x00;
    test2[27] = 0x00;

    ublox.setUbxChecksum(test2);
    ublox.printMessageInHex(test2, sizeof(test2));
    ublox.handleUbxMessage(test2);

    uint8_t test3[40];
    test3[0] = UBX::Header::SyncChar1;
    test3[1] = UBX::Header::SyncChar2;
    test3[2] = UBX::NAV::classId;
    test3[3] = UBX::NAV::SAT::messageId;
    test3[4] = 0x28;
    test3[5] = 0x00;
    // itow
    test3[6] = 0x01;
    test3[7] = 0x00;
    test3[8] = 0x00;
    test3[9] = 0x00;
    // vers
    test3[10] = 0x01;
    // num sv
    test3[11] = 0x02;
    // res
    test3[12] = 0x00;
    test3[13] = 0x00;
    // sat 1
    test3[14] = 0x1A;

    test3[15] = 0x1A;
        
    test3[16] = 0x03;
        
    test3[17] = 0x03;
        
    test3[18] = 0x03;
    test3[19] = 0x00;
        
    test3[20] = 0x03;
    test3[21] = 0x00;
        
    test3[22] = 0x00;
    test3[23] = 0x00;
    test3[24] = 0x00;
    test3[25] = 0x00;
    // s3t 2
    test3[26] = 0x1B;
        
    test3[27] = 0x13;
        
    test3[28] = 0x04;
        
    test3[29] = 0x04;
        
    test3[30] = 0x04;
    test3[31] = 0x00;
        
    test3[32] = 0x04;
    test3[33] = 0x00;
        
    test3[34] = 0x00;
    test3[35] = 0x00;
    test3[36] = 0x00;
    test3[37] = 0x00;
    // c3eck sum
    test3[38] = 0x00;
    test3[39] = 0x00;
    ublox.setUbxChecksum(test3);
    ublox.printMessageInHex(test3, sizeof(test3));
    ublox.handleUbxMessage(test3);

    ublox.printNavSatelliteData();

    return 0;
}

/*
 uint8_t test2[40];
    test2[0] = UBX::Header::SyncChar1;
    test2[1] = UBX::Header::SyncChar2;
    test2[2] = UBX::NAV::classId;
    test2[3] = UBX::NAV::SAT::messageId;
    test2[4] = 0x28;
    test2[5] = 0x00;
    // itow
    test2[6] = 0x01;
    test2[7] = 0x00;
    test2[8] = 0x00;
    test2[9] = 0x00;
    // vers
    test2[10] = 0x01;
    // num sv
    test2[11] = 0x02;
    // res
    test2[12] = 0x00;
    test2[13] = 0x00;
    // sat 1
    test2[14] = 0x1A;

    test2[15] = 0x02;

    test2[16] = 0x03;

    test2[17] = 0x03;

    test2[18] = 0x03;
    test2[19] = 0x00;

    test2[20] = 0x03;
    test2[21] = 0x00;

    test2[22] = 0x00;
    test2[23] = 0x00;
    test2[24] = 0x00;
    test2[25] = 0x00;
    // sat 2
    test2[26] = 0x1B;

    test2[27] = 0x03;

    test2[28] = 0x04;

    test2[29] = 0x04;

    test2[30] = 0x04;
    test2[31] = 0x00;

    test2[32] = 0x04;
    test2[33] = 0x00;

    test2[34] = 0x00;
    test2[35] = 0x00;
    test2[36] = 0x00;
    test2[37] = 0x00;
    // check sum
    test2[38] = 0x00;
    test2[39] = 0x00;

    ublox.setUbxChecksum(test2);

    ublox.printMessageInHex(test2, sizeof(test2));

    ublox.handleUbxMessage(test2);
*/