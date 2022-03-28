#pragma once

#define MAX_DATA_LENGTH 512

#include <windows.h>
#include <WinSock2.h>
#include <iostream>

class SerialPort
{
private:
    HANDLE handler;
    bool connected;
    COMSTAT status;
    DWORD errors;
public:
    explicit SerialPort(const char* portName);
    ~SerialPort();

    int readSerialPort(const unsigned char* buffer, int buf_size);
    int writeSerialPort(const unsigned char* buffer, int buf_size);
    bool isConnected();
    void closeSerial();
};