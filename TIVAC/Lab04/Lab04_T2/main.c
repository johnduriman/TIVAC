#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "inc/hw_gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"

int main(void)
{
    uint32_t ui32PeriodHigh;
    uint32_t ui32PeriodLow;
    uint32_t ui32Period;

    //Clock setup
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    //Enable GPIO and configure pins as outputs
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    //Unlock Pin PF0
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = 0x1;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;

    //Switch Interrupt
    //Enable GPIO peripheral and configure pins as inputs
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0);
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_INT_PIN_0, GPIO_RISING_EDGE);
    IntEnable(INT_GPIOF);

    //Configure Timer 0 and Timer 1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);

    //Delay calculation
    //10 Hz duty cycle is 0.1s delay
    //With a 43% duty cycle, it will have 0.043s ON, and 0.057s OFF
    ui32PeriodHigh = 43 * (SysCtlClockGet() / 10) / 100;
    ui32PeriodLow = 57 * (SysCtlClockGet() / 10) / 100;
    TimerLoadSet(TIMER0_BASE, TIMER_A, ui32PeriodHigh - 1);

    //Interrupt enable
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();

    //Timer enable
    TimerEnable(TIMER0_BASE, TIMER_A);

    while(1){    }
}

void Timer0IntHandler(void)
{
    uint32_t ui32PeriodHigh = 43 * (SysCtlClockGet() / 10) / 100;
    uint32_t ui32PeriodLow = 57 * (SysCtlClockGet() / 10) / 100;

    //Clear timer interrupt
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    // Read the current state of the GPIO pin and
    // write back the opposite state
    if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2))
    {
        TimerLoadSet(TIMER0_BASE, TIMER_A, ui32PeriodHigh - 1);             //Load low
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
    }
    else
    {
        TimerLoadSet(TIMER0_BASE, TIMER_A, ui32PeriodLow - 1);             //Load High
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 4);
    }

}

void PortFPin0IntHandler(void)
{
    //Clear GPIO interrupt
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_0);

    //Configure delay and enable timer 1
    TimerLoadSet(TIMER1_BASE, TIMER_A, SysCtlClockGet()-1);
    TimerEnable(TIMER1_BASE, TIMER_A);

    //Make all Pins low
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);

    //Set Pin 3 to high
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 8);

    //TIMER 1 Count reached
    TimerIntEnable(TIMER1_BASE,TIMER_TIMA_TIMEOUT);
    while(1)
    {
       if(TimerIntStatus(TIMER1_BASE,true)&TIMER_TIMA_TIMEOUT==TIMER_TIMA_TIMEOUT)
       {
           TimerIntClear(TIMER1_BASE,TIMER_TIMA_TIMEOUT);
           break;
       }
    }

    TimerDisable(TIMER1_BASE, TIMER_A);
}
