#include "driverlib.h"
#include "msp430.h"
#include "oled.h"
#include "type.h"
#include "string.h"
#include "stdio.h"

#define UCS_MCLK_DESIRED_FREQUENCY_IN_KHZ   16000
#define UCS_MCLK_FLLREF_RATIO   4   //锁相环倍频数
#define UCS_XT1_CRYSTAL_FREQUENCY    32768
#define UCS_XT2_CRYSTAL_FREQUENCY   4000000
#define UCS_XT1_TIMEOUT 50000
#define TIMER_PERIOD 399
#define DUTY_CYCLE  200
#define UCS_XT2_TIMEOUT 50000
#define CPU_F                               ((double)16000000)
#define delay_us(x)                       __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define delay_ms(x)                      __delay_cycles((long)(CPU_F*(double)x/1000.0))

uint16_t status;
uint8_t returnValue = 0;

void myclock_init()
{
  //Set VCore = 1 for 12MHz clock
  PMM_setVCore(PMM_CORE_LEVEL_1);//主频提高后，VCore电压也需要随之配置

  //Initializes the XT1 and XT2 crystal frequencies being used
  UCS_setExternalClockSource(UCS_XT1_CRYSTAL_FREQUENCY,UCS_XT2_CRYSTAL_FREQUENCY);//设置外部时钟源的频率，没什么实际设定

  //Initialize XT1. Returns STATUS_SUCCESS if initializes successfully
  GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5,GPIO_PIN4 + GPIO_PIN5);//XT1口不作为普通IO
  returnValue = UCS_turnOnLFXT1WithTimeout(UCS_XT1_DRIVE_0,UCS_XCAP_3,UCS_XT1_TIMEOUT);//启动XT1

  //Startup HF XT2 crystal Port select XT2
  GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5,GPIO_PIN2 + GPIO_PIN3);//XT2口不作为普通IO

  //Initialize XT2. Returns STATUS_SUCCESS if initializes successfully
  returnValue = UCS_turnOnXT2WithTimeout(UCS_XT2_DRIVE_4MHZ_8MHZ,UCS_XT2_TIMEOUT);//启动XT2

  //Set DCO FLL reference = REFO
  UCS_initClockSignal(UCS_FLLREF,UCS_XT2CLK_SELECT,UCS_CLOCK_DIVIDER_1);//XT2作为FLL参考

  //Set Ratio and Desired MCLK Frequency  and initialize DCO
  UCS_initFLLSettle(UCS_MCLK_DESIRED_FREQUENCY_IN_KHZ,UCS_MCLK_FLLREF_RATIO);//MCLK设置为16MHz

  //Set ACLK = REFO
  UCS_initClockSignal(UCS_ACLK,UCS_REFOCLK_SELECT,UCS_CLOCK_DIVIDER_1);//ACLK设置为32.768kHz

  UCS_initClockSignal(UCS_SMCLK,UCS_XT2CLK_SELECT,UCS_CLOCK_DIVIDER_1);//SMCLK设置为4MHz
}

//频率和占空比计算方法：选择时钟源SMCLK，4MHz，timerPeriod=399,PWM频率4MHz/400=10KHz,dutyCycle=200，占空比200/400=50%
//改变占空比：Timer_A_setCompareValue(TIMER_A2_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1,100)，100/400=25%
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
  //P2.4 as PWM output
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2,GPIO_PIN4);

    //Generate PWM - Timer runs in Up mode
//    Timer_A_outputPWMParam param = {0};
    param.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    param.timerPeriod = TIMER_PERIOD;
    param.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    param.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    param.dutyCycle = DUTY_CYCLE;
    Timer_A_outputPWM(TIMER_A2_BASE, &param);
}
void myuart_init()
{
  //P3.3,4 = USCI_A0 TXD/RXD
  GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,GPIO_PIN3 + GPIO_PIN4);

  //Baudrate = 9600, clock freq =4MHz
  //UCBRx = 26, UCBRFx = 1, UCBRSx = 0, UCOS16 = 1
  USCI_A_UART_initParam param = {0};
  param.selectClockSource = USCI_A_UART_CLOCKSOURCE_SMCLK;
  param.clockPrescalar = 26;
  param.firstModReg = 1;
  param.secondModReg = 0;
  param.parity = USCI_A_UART_NO_PARITY;
  param.msborLsbFirst = USCI_A_UART_LSB_FIRST;
  param.numberofStopBits = USCI_A_UART_ONE_STOP_BIT;
  param.uartMode = USCI_A_UART_MODE;
  param.overSampling = USCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;

  if (STATUS_FAIL == USCI_A_UART_init(USCI_A0_BASE, &param))
  {
      return;
  }

  //Enable UART module for operation
  USCI_A_UART_enable(USCI_A0_BASE);

  //Enable Receive Interrupt
  USCI_A_UART_clearInterrupt(USCI_A0_BASE,USCI_A_UART_RECEIVE_INTERRUPT);
  USCI_A_UART_enableInterrupt(USCI_A0_BASE,USCI_A_UART_RECEIVE_INTERRUPT);
}
unsigned int i;
uint8_t transmitData = 0x00;
uint8_t receivedData = 0x00;
bool send_ready=0;

//OLED:SCL-p3.5,SDA-P3.6
int main (void)
{
    volatile long clockValue1,clockValue2,clockValue3;
    char str[8],str2[]="   ";
    u32 num = 1010;
    //Stop WDT
    WDT_A_hold(WDT_A_BASE);

    myclock_init();
    OLED_Init();    //初始化
    OLED_Clear();   //清屏
    myuart_init();
    mypwm_init();
    GPIO_setAsOutputPin(GPIO_PORT_P4,  GPIO_PIN7);
    //Verify if the Clock settings are as expected
    clockValue1 = UCS_getMCLK();
    clockValue2 = UCS_getACLK();
    clockValue3 = UCS_getSMCLK();
//    sprintf(str,"%d",num);
//    strcat(str,str2);
//    OLED_Print(8,16,(u8*)str,TYPE16X16,TYPE8X16);
    OLED_Print(8,0,(u8*)"中華人民共和國",TYPE16X16,TYPE8X16);
    OLED_Print(40,16,(u8*)"萬歲!",TYPE16X16,TYPE8X16);
    Timer_A_setCompareValue(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1,100);
    //Enable  interrupt
    __enable_interrupt();

  //Loop in place
    while (1)
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P4,GPIO_PIN7);

//        Timer_A_setCompareValue(TIMER_A2_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1,100);
//        num--;
//        sprintf(str,"%d",num);
//        strcat(str,str2);
//        OLED_Print(8,16,(u8*)str,TYPE16X16,TYPE8X16);
        delay_ms(500);
        GPIO_setOutputLowOnPin(GPIO_PORT_P4,GPIO_PIN7);
//        Timer_A_setCompareValue(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1,300);
//        Timer_A_setCompareValue(TIMER_A2_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1,300);
//        num--;
//        sprintf(str,"%d",num);
//        strcat(str,str2);
//        OLED_Print(8,16,(u8*)str,TYPE16X16,TYPE8X16);
        delay_ms(500);
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
      receivedData = USCI_A_UART_receiveData(USCI_A0_BASE);
      USCI_A_UART_transmitData(USCI_A0_BASE,receivedData);
      break;
    default:
      break;
  }
}
