
#include "driverlib.h"

#define CPU_F                               ((double)1048576)
#define delay_us(x)                       __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define delay_ms(x)                      __delay_cycles((long)(CPU_F*(double)x/1000.0))



int main (void)
{

    //Stop WDT
    WDT_A_hold(WDT_A_BASE);

    //PA.x output
    GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0 );

    while(1)
    {
        //Set all PA pins HI
        GPIO_setOutputHighOnPin(GPIO_PORT_P1,GPIO_PIN0);
        delay_ms(500);
        GPIO_setOutputLowOnPin(GPIO_PORT_P1,GPIO_PIN0);
        delay_ms(500);
    }

}
