#include "driverlib.h"
#define COMPARE_VALUE 4000000/100
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
//COMPARE_VALUE设置方法：以4M/64为例，数一个数需要1/4M秒，要数到4M/64才中断，需要1/64秒即15.625毫秒，翻转一次31.25毫秒

int main (void)
{
    //Stop WDT
    WDT_A_hold(WDT_A_BASE);
    myclock_init();
    //Set P1.0 to output direction
    GPIO_setAsOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN0
        );

    //Start timer in continuous mode sourced by SMCLK
    Timer_A_initContinuousModeParam initContParam = {0};
    initContParam.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    initContParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    initContParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    initContParam.timerClear = TIMER_A_DO_CLEAR;
    initContParam.startTimer = false;
    Timer_A_initContinuousMode(TIMER_A1_BASE, &initContParam);

    //Initiaze compare mode
	Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
		TIMER_A_CAPTURECOMPARE_REGISTER_0
		);

    Timer_A_initCompareModeParam initCompParam = {0};
    initCompParam.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;
    initCompParam.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    initCompParam.compareOutputMode = TIMER_A_OUTPUTMODE_OUTBITVALUE;
    initCompParam.compareValue = COMPARE_VALUE;
    Timer_A_initCompareMode(TIMER_A1_BASE, &initCompParam);

    Timer_A_startCounter( TIMER_A1_BASE,
    		TIMER_A_CONTINUOUS_MODE
                );

    //Enter LPM0, enable interrupts
    __bis_SR_register(GIE);

    //For debugger
    __no_operation();
}

//******************************************************************************
//
//This is the TIMER1_A3 interrupt vector service routine.
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(TIMER1_A0_VECTOR)))
#endif
void TIMER1_A0_ISR (void)
{
    uint16_t compVal = Timer_A_getCaptureCompareCount(TIMER_A1_BASE,
    		TIMER_A_CAPTURECOMPARE_REGISTER_0)
    		+ COMPARE_VALUE;

    //Toggle P1.0
    GPIO_toggleOutputOnPin(
        GPIO_PORT_P1,
        GPIO_PIN0
        );

    //Add Offset to CCR0
    Timer_A_setCompareValue(TIMER_A1_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0,
        compVal
        );
}
