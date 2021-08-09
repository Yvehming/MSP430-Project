#include "driverlib.h"
#include "uart.h"
#include "string.h"
#include "stdlib.h"

#define UCS_MCLK_DESIRED_FREQUENCY_IN_KHZ   16000
#define UCS_MCLK_FLLREF_RATIO   4   //锁相环倍频数
#define UCS_XT1_CRYSTAL_FREQUENCY    32768
#define UCS_XT2_CRYSTAL_FREQUENCY   4000000
#define UCS_XT1_TIMEOUT 50000
#define TIMER_PERIOD 399
#define DUTY_CYCLE  100
#define UCS_XT2_TIMEOUT 50000
#define CPU_F                               ((double)16000000)
#define delay_us(x)                       __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define delay_ms(x)                      __delay_cycles((long)(CPU_F*(double)x/1000.0))

uint16_t status;
uint32_t clockValue1 ,
         clockValue2 ,
         clockValue3 ;

void myclock_init()
{
  //Set VCore = 1 for 12MHz clock
  PMM_setVCore(PMM_CORE_LEVEL_1);//主频提高后，VCore电压也需要随之配置

  //Initializes the XT1 and XT2 crystal frequencies being used
  UCS_setExternalClockSource(UCS_XT1_CRYSTAL_FREQUENCY,UCS_XT2_CRYSTAL_FREQUENCY);//设置外部时钟源的频率，没什么实际设定

  //Initialize XT1. Returns STATUS_SUCCESS if initializes successfully
  GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5,GPIO_PIN4 + GPIO_PIN5);//XT1口不作为普通IO
  UCS_turnOnLFXT1WithTimeout(UCS_XT1_DRIVE_0,UCS_XCAP_3,UCS_XT1_TIMEOUT);//启动XT1

  //Startup HF XT2 crystal Port select XT2
  GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5,GPIO_PIN2 + GPIO_PIN3);//XT2口不作为普通IO

  //Initialize XT2. Returns STATUS_SUCCESS if initializes successfully
  UCS_turnOnXT2WithTimeout(UCS_XT2_DRIVE_4MHZ_8MHZ,UCS_XT2_TIMEOUT);//启动XT2

  //Set DCO FLL reference = REFO
  UCS_initClockSignal(UCS_FLLREF,UCS_XT2CLK_SELECT,UCS_CLOCK_DIVIDER_1);//XT2作为FLL参考

  //Set Ratio and Desired MCLK Frequency  and initialize DCO
  UCS_initFLLSettle(UCS_MCLK_DESIRED_FREQUENCY_IN_KHZ,UCS_MCLK_FLLREF_RATIO);//MCLK设置为16MHz

  //Set ACLK = REFO
  UCS_initClockSignal(UCS_ACLK,UCS_REFOCLK_SELECT,UCS_CLOCK_DIVIDER_1);//ACLK设置为32.768kHz

  UCS_initClockSignal(UCS_SMCLK,UCS_XT2CLK_SELECT,UCS_CLOCK_DIVIDER_1);//SMCLK设置为4MHz
}

void mypwm_init()
{
  //P2.0 as PWM output
  GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2,GPIO_PIN0);

  //Generate PWM - Timer runs in Up mode
  Timer_A_outputPWMParam param = {0};
  param.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
  param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
  param.timerPeriod = TIMER_PERIOD;
  param.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
  param.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
  param.dutyCycle = DUTY_CYCLE;
  Timer_A_outputPWM(TIMER_A1_BASE, &param);
}


uint8_t transmitData = 0x00;

bool send_ready=0;

int main (void)
{
    uint8_t i;
    volatile float f = 0;
    //Stop WDT
    WDT_A_hold(WDT_A_BASE);

    myclock_init();

    myuart_init();
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN4 + GPIO_PIN7);
    //Verify if the Clock settings are as expected
    clockValue1 = UCS_getMCLK();
    clockValue2 = UCS_getACLK();
    clockValue3 = UCS_getSMCLK();

    // Enable global interrupt
    //  __bis_SR_register(GIE);
    //Enable  interrupt
    __enable_interrupt();

  //Loop in place
    while (1)
    {

        while(flag != 1);
        if(flag == 1)
        {
            for(i = 0;i < len;i++)
            {
                USCI_A_UART_transmitData(USCI_A0_BASE,receiveData[i]);
            }
            USCI_A_UART_transmitData(USCI_A0_BASE,0x20);
            flag = 0;
            f = atof((char*)receiveData);
        }
    }
}
//******************************************************************************
//
//This is the USCI_A0 interrupt vector service routine.
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_A0_VECTOR)))
#endif
void USCI_A0_ISR (void)
{
    switch (__even_in_range(UCA0IV,4))
    {
    //Vector 2 - RXIFG
        case 2:
            receivedByte = USCI_A_UART_receiveData(USCI_A0_BASE);
            if(receivedByte == 0x5B)
            {
                len = 0;
                flag = 0;
                break;
            }
            if(receivedByte == 0x5D)
            {
                flag = 1;
                break;
            }
            receiveData[len++] = receivedByte;
//            USCI_A_UART_transmitData(USCI_A0_BASE,receivedByte);
            break;
        default:
            break;
    }
}
