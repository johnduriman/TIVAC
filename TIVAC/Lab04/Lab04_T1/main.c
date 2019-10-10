#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"




int main(void)
{
    uint32_t ui32PeriodHigh;
    uint32_t ui32PeriodLow;


    //Clock setup
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    //Enable GPIO and configure pins
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    //Configure timer for Timer 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

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


    while(1)
    {

    }

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
