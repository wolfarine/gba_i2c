#include "../include/delay.h"

void delayNOP()
{

	//REG_TM3CNT_H=TIMER_1|TIMER_START;//it seems impossible to use time to do delay several ,including intrwait(1,timer).
	//REG_TM3CNT_L=0;
	//while(REG_TM3CNT_L<cycles);
	//REG_TM3CNT_H=0;
	//delay_10cycles();
	__asm("NOP");__asm("NOP");__asm("NOP");__asm("NOP");__asm("NOP");
	__asm("NOP");__asm("NOP");__asm("NOP");__asm("NOP");__asm("NOP");
	__asm("NOP");__asm("NOP");__asm("NOP");__asm("NOP");__asm("NOP");
	__asm("NOP");__asm("NOP");__asm("NOP");__asm("NOP");__asm("NOP");
	__asm("NOP");__asm("NOP");__asm("NOP");__asm("NOP");__asm("NOP");
	__asm("NOP");__asm("NOP");__asm("NOP");__asm("NOP");__asm("NOP");
	__asm("NOP");__asm("NOP");__asm("NOP");__asm("NOP");__asm("NOP");
	__asm("NOP");__asm("NOP");__asm("NOP");__asm("NOP");__asm("NOP");
	__asm("NOP");__asm("NOP");
	//some nop instructions to delay cpu cycles for i2c bus,1 cycle is about 60ns,42 cycles will be 2.5us (400khz).

}



void delayms(int ms)
{

	REG_TM3CNT_H=TIMER_256|TIMER_START;
	REG_TM3CNT_L=0;//clear timer3 for next time
	while(REG_TM3CNT_L<(65*ms));//65*256*60ns=0.99ms
	REG_TM3CNT_H=0;//close timer3
}

void delayus(int us)
{

	REG_TM3CNT_H=TIMER_1|TIMER_START;
	REG_TM3CNT_L=0;//clear timer3 for next time
	while(REG_TM3CNT_L<(18*us));//18*1*60ns=1000ns
	REG_TM3CNT_H=0;//close timer3
}

void delay014ms(u8 x)
{

	REG_TM3CNT_H=TIMER_256|TIMER_START;
	REG_TM3CNT_L=0;//clear timer3 for next time
	while(REG_TM3CNT_L<(10*x));//10*256*59.59ns=0.15ms
	REG_TM3CNT_H=0;//close timer3
}
