# Fourier
_Major assignment of the course Computer Systems and C Programming Language with Thanh Sang, Thành Đạt, Tài Em and Long Nhi_
Continuously reading ADC from MCU and perform DFT on received data on Window using C.
- The data is sent from the ADC 12bit of the STM32 but you can use any other MCU (the resolution of the ADC in the code can be changed so it fit your MCU ADC in the define part in the routine.c file), in the transfer package with the size of 3 byte:
    The first byte is the Frame Number which continuously increment from 0 to 255 and then back to 0, it is used to dectect missing package event and can be used to find the starting point of the package.
    The two next byte used to carry high and low byte of ADC respectively.
- Data recieved then stored in a circular buffer, after storing a specific byte of data, a chunk of data is then used to feed to the DFT to extract it frequency in another thread parallel to data collectiion thread.
- The output of the fourier transform is then used to plot in the command prompt.
- Output (with sampling frequency equaled to 2kHz --> maximum frquency is 1kHz and frequency Resolution is 512 bit-->approximately 2Hz each step):
![image](https://github.com/Long-Tam/Fourier/assets/124179712/32002c9f-49fe-4ae4-bb59-f679f9b62c22)
As can be seen from the picture, when i touch the floating ADC pin, beside the DC value, at the position 26th, 51st, ... which correspond to 50Hz, 100Hz,... emerge the power at those frequency. The frequency 50Hz is also the frequency of the power grid of our place.
And this is the result when input a 500Hz square wave with 50% duty cycle:
![image](https://github.com/Long-Tam/Fourier/assets/124179712/1214d9c0-16e9-4d4d-a1e2-3dcfc8444533)
We see a peek at the 256th position, correspond to the frequency of 500Hz.  
