#include "func.h"

HANDLE SerialPortInit(char *port, uint32_t baudrate)
{
    // https://ds.opdenbrouw.nl/micprg/pdf/serial-win.pdf
    HANDLE hSerial;
    char serial_port[16] = "\\\\.\\";
    strcat(serial_port, port);
    hSerial = CreateFile(serial_port,
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         0,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         0);

    if (hSerial == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            // serial port does not exist. Inform user.
            printf("SERIAL PORT DOES NOT EXIST, ");
        }
        // some other error occurred. Inform user.
        printf("INVALID_HANDLE_VALUE\n");
        return 0;
    }

    // config
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        // error getting state
        printf("ERROR_GETTING_STATE\n");
        return 0;
    }
    dcbSerialParams.BaudRate = baudrate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        // error setting serial port state
        printf("ERROR_SETTING_SERIAL_PORT_STATE\n");
        return 0;
    }

    // set time out
    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutConstant = 1;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    timeouts.WriteTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    if (!SetCommTimeouts(hSerial, &timeouts))
    {
        // error occureed. Inform user
        printf("ERROR_SETTING_TIME_OUT\n");
        return 0;
    }
    return hSerial;
}

void DataAllign(HANDLE hSerial, uint8_t packagesize)
{
    uint16_t i = 0;
    uint8_t temp = 0;
    uint8_t now = 0;
    uint8_t szBuff[packagesize - 1];
    DWORD dwBytesRead = 0;
    while (1)
    {
        ReadFile(hSerial, &temp, 1, &dwBytesRead, NULL);

        ReadFile(hSerial, szBuff, packagesize - 1, &dwBytesRead, NULL);

        ReadFile(hSerial, &now, 1, &dwBytesRead, NULL);

        ReadFile(hSerial, szBuff, packagesize - 1, &dwBytesRead, NULL);
        if ((uint8_t)(now - temp) == 1)
        {
            i++;
            if (i == 300)
                break;
            else
                continue;
        }
        i = 0;
        ReadFile(hSerial, szBuff, 1, &dwBytesRead, NULL);
    }
}