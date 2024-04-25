#include <stdint.h>
#include <math.h>
#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <unistd.h>

HANDLE SerialPortInit(char *port, uint32_t baudrate);
void DataAllign(HANDLE hSerial, uint8_t szBuff);
