#pragma once

#define MAX_DATA_LENGTH 512

#include <windows.h>
#include <iostream>

class SerialPort
{
private:
    HANDLE handler = 0;
    bool connected = 0;
    COMSTAT status;
    DWORD errors =0;
public:
    SerialPort();
    ~SerialPort();

    int connect(const char* portName);
    int readSerialPort(const unsigned char* buffer, int buf_size);
    int writeSerialPort(const unsigned char* buffer, int buf_size);
    int bytesAvail();
    bool isConnected();
    void closeSerial();
};