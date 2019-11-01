#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

uint32_t ui32ADC0Value[4];              //Array for storing ADC FIFO data
volatile uint32_t ui32TempAvg;          //Temp sensor data
volatile uint32_t ui32TempValueC;       //Celsius
volatile uint32_t ui32TempValueF;       //Fahrenheit
volatile uint32_t ui8PinData;           //LED
char string[10];                        //Holds Temperature in F
char string1[10];                       //Holds Temperature in C

void printm(char *str)
{
    while(*str != '\0')
    {
        UARTCharPut(UART0_BASE, *str);
        ++str;
    }
}

void reverse(char s[])
{
    int i, j;
    char c;

    for(i = 0, j = strlen(s)-1; i < j; i++, j--)
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char s[])
{
    int i, sign;

    if((sign = n ) < 0)
        n = -n;

    i = 0;
    do
    {
        s[i++] = n % 10 + '0';
    }
    while ((n/=10) > 0);
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

void UARTIntHandler(void)
{
    uint32_t ui32Status;
    char command;

    ui32Status = UARTIntStatus(UART0_BASE, true); //get interrupt status

    UARTIntClear(UART0_BASE, ui32Status); //clear the asserted interrupts

    while(UARTCharsAvail(UART0_BASE)) //loop while there are chars
    {
        //UARTCharPutNonBlocking(UART0_BASE, UARTCharGetNonBlocking(UART0_BASE)); //echo character
        command = UARTCharGet(UART0_BASE);
        UARTCharPut(UART0_BASE, command);
        switch(command)
        {
        case 'R':
            ui8PinData = 2;
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, ui8PinData);
            printm("\r\n");
            printm("Enter Text: ");
            break;
        case 'G':
            ui8PinData = 8;
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, ui8PinData);
            printm("\r\n");
            printm("Enter Text: ");
            break;
        case 'B':
            ui8PinData = 4;
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, ui8PinData);
            printm("\r\n");
            printm("Enter Text: ");
            break;
        case 'T':
            printm("\r\n");
            printm("Temp = ");
            printm(string);
            printm("F = ");
            printm(string1);
            printm("C");
            printm("\r\n");
            printm("Enter Text: ");
            break;
        default:
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0);
            printm("\r\n");
            printm("Enter Text: ");
            break;
        }
    }
}


int main(void) {

    //Setup clock and ADC
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);         //Enable ADC0
    ADCHardwareOversampleConfigure(ADC0_BASE, 64);

    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); //enable GPIO port for LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3); //enable pin for LED PF2

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    IntMasterEnable(); //enable processor interrupts
    IntEnable(INT_UART0); //enable the UART interrupt
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT); //only enable RX and TX interrupts

    //Configure ADC sequencer, sample sequencer 1, trigger the sequence at highest priority
    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

    //Configure all four steps in the ADC sequencer to sequencer 2
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
    //Final sequencer step
    ADCSequenceStepConfigure(ADC0_BASE, 1 ,3 , ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);

    //Enable ADC Sequencer 1
    ADCSequenceEnable(ADC0_BASE, 1);

    UARTCharPut(UART0_BASE, 'E');
    UARTCharPut(UART0_BASE, 'n');
    UARTCharPut(UART0_BASE, 't');
    UARTCharPut(UART0_BASE, 'e');
    UARTCharPut(UART0_BASE, 'r');
    UARTCharPut(UART0_BASE, ' ');
    UARTCharPut(UART0_BASE, 'T');
    UARTCharPut(UART0_BASE, 'e');
    UARTCharPut(UART0_BASE, 'x');
    UARTCharPut(UART0_BASE, 't');
    UARTCharPut(UART0_BASE, ':');
    UARTCharPut(UART0_BASE, ' ');

    while (1) //let interrupt handler do the UART echo function
    {
        //Clear ADC interrupt status flag
        ADCIntClear(ADC0_BASE, 1);
        //Trigger ADC conversion
        ADCProcessorTrigger(ADC0_BASE, 1);

        while(!ADCIntStatus(ADC0_BASE, 1, false)){}

        //Read ADC value
        ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);

        //Calculate average of the temperature sensor data
        ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
        ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;
        ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;
        itoa(ui32TempValueF, string);
        itoa(ui32TempValueC, string1);
    }

}
