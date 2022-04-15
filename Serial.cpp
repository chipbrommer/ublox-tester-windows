#include "Serial.h"

SerialPort::SerialPort()
{
    // Do Nothing
}

int SerialPort::connect(const char* portName)
{
    this->connected = false;

    this->handler = CreateFileA(static_cast<LPCSTR>(portName),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (this->handler == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            std::cerr << "ERROR: Handle was not attached. Reason : " << portName << " not available\n";
        }
        else
        {
            std::cerr << "ERROR!!!\n";
            errno = GetLastError();
            return -1;
        }
    }
    else
    {
        DCB dcbSerialParameters = { 0 };

        if (!GetCommState(this->handler, &dcbSerialParameters))
        {
            std::cerr << "Failed to get current serial parameters\n";
        }
        else
        {
            dcbSerialParameters.BaudRate = CBR_9600;
            dcbSerialParameters.ByteSize = 8;
            dcbSerialParameters.StopBits = ONESTOPBIT;
            dcbSerialParameters.Parity = NOPARITY;
            dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

            if (!SetCommState(handler, &dcbSerialParameters))
            {
                std::cout << "ALERT: could not set serial port parameters\n";
            }
            else
            {
                this->connected = true;
                PurgeComm(this->handler, PURGE_RXCLEAR | PURGE_TXCLEAR);
                Sleep(1);
            }
        }
    }

    return 0;
}

SerialPort::~SerialPort()
{
    if (this->connected)
    {
        this->connected = false;
        CloseHandle(this->handler);
    }
}

// Reading bytes from serial port to buffer;
// returns read bytes count, or if error occurs, returns 0
int SerialPort::readSerialPort(const unsigned char* buffer, int buf_size)
{
    DWORD bytesRead{};
    unsigned int toRead = 0;

    ClearCommError(this->handler, &this->errors, &this->status);

    if (this->status.cbInQue > 0)
    {
        if (this->status.cbInQue > static_cast<DWORD>(buf_size))
        {
            toRead = buf_size;
        }
        else
        {
            toRead = this->status.cbInQue;
        }
    }

    memset((void*)buffer, 0, buf_size);

    if (ReadFile(this->handler, (void*)buffer, toRead, &bytesRead, NULL))
    {
        return bytesRead;
    }

    return 0;
}

// Sending provided buffer to serial port;
int SerialPort::writeSerialPort(const unsigned char* buffer, int buf_size)
{
    DWORD bytesSent;

    if (!WriteFile(this->handler, (void*)buffer, buf_size, &bytesSent, 0))
    {
        ClearCommError(this->handler, &this->errors, &this->status);
        return -1;
    }

    return bytesSent;
}

int SerialPort::bytesAvail()
{
    return ClearCommError(this->handler, &this->errors, &this->status);
}

// Checking if serial port is connected
bool SerialPort::isConnected()
{
    if (!ClearCommError(this->handler, &this->errors, &this->status))
    {
        this->connected = false;
    }

    return this->connected;
}

void SerialPort::closeSerial()
{
    CloseHandle(this->handler);
}