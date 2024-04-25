#include "func.h"
#include "routine.h"

#define adc_resolution 12
#define max_adc ((1 << adc_resolution) - 1)
#define max_x 256
#define max_y 100
#define COM_PORT "COM21"
#define baudrate 115200
uint16_t pointer;
uint16_t buffer[0x10000];
uint8_t buff[max_x];
uint16_t flag;
uint16_t pointer1;
/*Reading from USB, storing data to Buffer (circular)*/
void *routine1()
{
    uint8_t pkg_sz = 3;

    HANDLE hSerial;
    char port[7];
    strcpy(port, COM_PORT);
    hSerial = SerialPortInit(port, baudrate);
    uint8_t frame_now, frame_old;
    DWORD dwBytesRead = 0;
    uint32_t i = 0;
    if (hSerial != 0)
    {
        DataAllign(hSerial, pkg_sz);
        // in case of needing to read the voltage, uncomment the line bellow
        // float voltage;
        ReadFile(hSerial, &frame_old, 3, &dwBytesRead, NULL);
        while (1)
        {
            ReadFile(hSerial, &frame_now, 1, &dwBytesRead, NULL);
            ReadFile(hSerial, &buffer[pointer], pkg_sz - 1, &dwBytesRead, NULL);
            if (((uint8_t)(frame_now - frame_old) != 1) && ((uint8_t)(frame_now - frame_old) != 0))
            {
                /* missing package detect*/
                // printf("now:%d      old: %d     %u\n", frame_now, frame_old, i);
                // break;
                DataAllign(hSerial, pkg_sz);
                ReadFile(hSerial, &frame_now, 1, &dwBytesRead, NULL);
                ReadFile(hSerial, &buffer[pointer], pkg_sz - 1, &dwBytesRead, NULL);

                i = 0;
            }
            frame_old = frame_now;
            i++;
            // buffer[pointer] = i; // for debugging
            // in case of needing to read the voltage, uncomment the bellow
            /*voltage = buffer[pointer];
            voltage = voltage / max_adc * 3.3;  //depend on your hardware set up, adjust this line to acquire the correct voltage
            printf("%.4f\n", voltage);*/
            pointer++;
            if ((pointer & (2 * max_x - 1)) == (2 * max_x - 1))
            {
                sem_post(&flag1);
                // printf("                                                                                                %d      flag: %d\n", max_x, (uint8_t)(log2(max_x) + 1));
                // flag = pointer >> (uint8_t)(log2(max_x) + 1);
                pointer1 = pointer;
                i = 0;
            }
        }
    }
    CloseHandle(hSerial);
}
#define pi 3.14159265358979323846
#define N_TIME (max_x * 2)
#define N_FREQ (N_TIME / 2 + 1)
uint16_t dft_buff[N_TIME];
double REX[N_FREQ];
double IMX[N_FREQ];
double MAG[N_FREQ];
void *routine2() // fourier
{
    //------------------------------fourier ici----------------------------------
    float cos_lookup[N_TIME];
    float sin_lookup[N_TIME];
    for (size_t i = 0; i < N_TIME; i++)
    {
        cos_lookup[i] = 1 * cos((float)(i * 2 * pi / N_TIME));
        sin_lookup[i] = 1 * sin((float)(i * 2 * pi / N_TIME));
    }

    while (1)
    {
        sem_wait(&flag1);
        //------------------------------fourier ici----------------------------------
        // Zero REX[] and IMX[] so they can be used as accumulators
        for (uint16_t k = 0; k < N_FREQ; k++)
        {
            REX[k] = 0;
            IMX[k] = 0;
        }
        // Loop through each sample in the frequency domain
        for (uint16_t k = 0; k < N_FREQ; k++)
        {
            // Loop through each sample in the time domain
            for (uint16_t i = 0; i < N_TIME; i++)
            {
                REX[k] += *(buffer + (uint16_t)(pointer1 - N_TIME + i)) * cos_lookup[(k * i) & (N_TIME - 1)];
                IMX[k] += -*(buffer + (uint16_t)(pointer1 - N_TIME + i)) * sin_lookup[(k * i) & (N_TIME - 1)];
            }

            // Calculate magnitude from real and imaginary part
            buff[k] = max_y * sqrt(REX[k] * REX[k] + IMX[k] * IMX[k]) / (N_TIME * max_adc);

            // buff[k] = (float)*(buffer + (uint16_t)(pointer1 - 0x0200 + k)) / (max_adc) * 100;
        }

        sem_post(&flag2);
    }
}

/*plotting data, taking value form buff[max=128], max val=50*/
void *routine3()
{
    system("cls");
    const uint16_t screen_x = max_x + 1;
    const uint16_t screen_y = max_y + 1;
    HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    SMALL_RECT WindowSize;
    WindowSize.Top = 0;
    WindowSize.Left = 0;
    WindowSize.Right = screen_x + 10;
    WindowSize.Bottom = screen_y + 10;
    COORD NewSize;
    NewSize.X = screen_x + 10;
    NewSize.Y = screen_y + 10;
    SetConsoleWindowInfo(stdOut, 1, &WindowSize);
    SetConsoleScreenBufferSize(stdOut, NewSize);

    COORD charBufSize = {screen_x, screen_y};
    COORD characterPos = {0, 0};
    SMALL_RECT writeArea = {6, 6, screen_x + 6, screen_y + 6};

    CHAR_INFO blank[screen_x * screen_y];
    for (int i = 0; i < (screen_x * screen_y); i++)
    {
        blank[i].Attributes = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED;
        if (i % screen_x == 0)
        {
            if ((i / screen_x) % 10 == 0)
                blank[i].Char.AsciiChar = '=';
            else
                blank[i].Char.AsciiChar = '|';
        }
        else
            blank[i].Char.AsciiChar = ' ';
        if (i > (screen_x * (screen_y - 1)))
        {
            if ((i - screen_y) % 100 == 0)
            {
                blank[i].Char.AsciiChar = ':';
                blank[i].Attributes = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
            }
            else if ((i - screen_y) % 10 == 0)
            {
                blank[i].Char.AsciiChar = '.';
                blank[i].Attributes = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
            }
            else
            {
                blank[i].Char.AsciiChar = '_';
            }
        }
    }

    //---------------------------------loop----------------------------------
    while (1)
    {
        sem_wait(&flag2); // sync

        for (uint16_t x = 0; x < screen_x - 1; x++)
        {

            // for (int i = (1 + x) + ((screen_y - 1) - buff[x]) * screen_x; i < screen_x * (screen_y - 1); i = i + screen_x)
            // {
            //     blank[i].Char.AsciiChar = ' ';
            //     blank[i].Attributes = BACKGROUND_BLUE;
            // }
            for (int i = (1 + x);
                 i < screen_x * (screen_y - 1);
                 i = i + screen_x)
            {
                if (i < (1 + x) + ((screen_y - 1) - buff[x]) * screen_x)
                {
                    blank[i].Char.AsciiChar = ' ';
                    blank[i].Attributes = 0;
                }
                else
                {
                    blank[i].Char.AsciiChar = ' ';
                    blank[i].Attributes = BACKGROUND_BLUE;
                }
            }
        }
        WriteConsoleOutput(stdOut, blank, charBufSize, characterPos, &writeArea);
    }
}
