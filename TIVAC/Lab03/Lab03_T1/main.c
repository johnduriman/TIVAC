#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

uint8_t ui8PinData = 2;

int main(void)
{
    //1/(400Mhz/(2*10)) = 50 ns delay
    //50 ns * 10,000,000 = 0.5s
    //Because the period is 0.5 seconds, the on and off time must be 0.25s each so the final delay is 5,000,000
    SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    while(1)
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, ui8PinData);
        SysCtlDelay(5000000);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00);
        SysCtlDelay(5000000);
        if(ui8PinData == 8) {ui8PinData = 2;} else {ui8PinData = ui8PinData * 2;}

    }
}

