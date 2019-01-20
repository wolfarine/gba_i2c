
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_timers.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/delay.h"
#include "../include/i2c.h"
//#include "../include/hs0038b.h"

#define KEY_any (DPAD|KEY_A|KEY_B|KEY_SELECT|KEY_START|KEY_R|KEY_L)
#define GPIO_IRQ 0x0100
//---------------------------------------------------------------------------------
// Program entry point
// RDA 5807 is a kind of i2c radio module, 
// that is manipulated from gba gpio port with i2c protocol，
// in this concept of proof routine.
//---------------------------------------------------------------------------------

u8 RDA_reg_data[8]=
{
   0xd0,0x00,  // 02H
   0x00,0x00,  // 03H
   0x00,0x40,  // 04H
   0x90,0x88,  //  05H
};

unsigned long frequency;

u8 IRCOM[4]={0,0,0,0};

void RDA5807_write_reg(void)
{
    	u8 k;
	I2C_start();
	// 收音模块写入操作 write to rda reg
	I2C_write_byte(0x20);
	// 寄存器连续写操作 write 8 times
	for(k=0; k<8; k++)
	{
		I2C_write_byte(RDA_reg_data[k]);
	}
	I2C_stop();
}
	
	
void RDA5807_read_reg(u8 *reg_buf)
{
   //delayms(50);
   I2C_start();
   // 收音模块读取操作 read rda reg
   I2C_write_byte(0x21);
   // 寄存器连续读操作 read 4 times
   reg_buf[0] = I2C_read_byte(I2C_ACK);
   reg_buf[1] = I2C_read_byte(I2C_ACK);
   reg_buf[2] = I2C_read_byte(I2C_ACK);
   reg_buf[3] = I2C_read_byte(I2C_NACK);
   I2C_stop();
}
/**********************************************************

 模块上电初始化子函数
 module startup routine

**********************************************************/
void RDA5807_power(void)
{
   delayms(50);

    // 发送软件复位指令 send soft reset cmd
   RDA_reg_data[0] = 0x00;
   RDA_reg_data[1] = 0x02;
   RDA5807_write_reg();

   delayms(10);

    // 收音模块默认参数 default parameter
   RDA_reg_data[0] = 0xd0;
   RDA_reg_data[1] = 0x01;
   RDA5807_write_reg();
}

/**********************************************************

 功能描述：收音模块自动寻台模式
 auto seek

**********************************************************/
void RDA5807_FM_seek(void)
{
   u16 chan;
   u8 i=0;
   u8  reg_data[4] = {0x00, 0x00, 0x00, 0x00};

   RDA_reg_data[3] &= ~(1 << 4);      //调谐禁用 tune disable

   // 内部自动寻台使能 auto seek enable
   RDA_reg_data[0] |=  (1 << 0);      //SEEK位置1 seek pos 1
   RDA5807_write_reg();

    // 等待STC 标志置位 wait for flag stc being set
   while(0 == (reg_data[0] & 0x40))
   {
     delayms(20);
      // 读取内部状态 read status
     RDA5807_read_reg(reg_data);
	 if(i<10)
	 i++;
	 else break;
   }
    // 获取当前工作频点 get freq
   chan = reg_data[0] & 0x03;
   chan = reg_data[1] | (chan << 8);
   chan = chan << 6;

    // 保存当前工作频点 save freq
   RDA_reg_data[2] = (chan >> 8) & 0xff;
   RDA_reg_data[3] = (chan & 0xff);
}

/**********************************************************

 频率显示子函数
 display freq routine

**********************************************************/
void  show_frequency(void)
{ 
   u8 i,display[5];
   u16 temp;

   temp = (RDA_reg_data[2]*256)+(RDA_reg_data[3]&0xc0);	 //计算 calc
   temp = temp>>6;
   frequency = (unsigned long)(100*temp+87000)/100;

   for(i=0; i<5; i++)  // 清显存单元 clear
   display[i] = 0x00;

   display[0] = (frequency)/1000 ;     //数据转换 freq to string
   display[1] = (frequency%1000)/100;
   display[2] = (frequency%100)/10;
   display[3] = 0x2e;                  //小数点 "."
   display[4] = (frequency%10);

   if(display[0] == 0)
   { 
     display[0] = display[1]+0x30;
     display[1] = display[2]+0x30;
     display[2] = display[3];
     display[3] = display[4]+0x30;
     display[4] = 0x20;
   }
   else
   { 
     display[0] += 0x30;
     display[1] += 0x30;
     display[2] += 0x30;
     display[4] += 0x30;
   }

   //lcd_pos_xy(3,2);                      //频率显示
   //lcd_wdat(display[0]);
   //lcd_wdat(display[1]);
   //lcd_wdat(display[2]);
   //lcd_wdat(display[3]);
   //lcd_wdat(display[4]);
   //irqEnable(IRQ_VBLANK);
   iprintf("\x1b[1;0HFM Radio %c%c%c%c%cMHz\n",display[0],display[1],display[2],display[3],display[4]);
   //irqDisable(IRQ_VBLANK);
}

/**********************************************************

 音量显示子函数
 display voice routine

**********************************************************/
void show_volume()
{
   u8 temp,display[2];

   temp = RDA_reg_data[7] & 0x0f; //取音量值 get volume value

   display[0] = temp/10;
   display[1] = temp%10;

   if(display[0] == 0)            //如果高位为0 
   { 
     display[0] = display[1];     //低位显存内容进入高位显存
     display[1] = 0x20;           //低位不显示
   }
   else 
   {
     display[1] += 0x30;
   }
   display[0] += 0x30;

   //lcd_pos_xy(13,2);              //音量值显示
   //lcd_wdat(display[0]);
   //lcd_wdat(display[1]);
   //irqEnable(IRQ_VBLANK); 
   iprintf("\x1b[2;0HVolume %c%c\n",display[0],display[1]);
   //irqDisable(IRQ_VBLANK);
   
}
/**********************************************************

 hs0038b ir module routine

**********************************************************/
void IR_CODE(void)
{
	REG_IME = 0;//中断总开关 set ime off
    	//iprintf("\x1b[4;0Hin\n");            				
	u8 j,k,N=0;
    	delay014ms(15);
	if (REG_RCNT & GPIO_SI)
    	{ 
		REG_IF |=IRQ_SERIAL;
		REG_IME =1;
		return;
	}                           		
	//确认IR信号出现  
	while (!(REG_RCNT & GPIO_SI))           //等IR变为高电平，跳过9ms的前导低电平信号。
	{	delay014ms(1);}
		for (j=0;j<4;j++)         				//收集四组数据
		{
			for (k=0;k<8;k++)        			//每组数据有8位
			{
				while (REG_RCNT & GPIO_SI)      //等 IR 变为低电平，跳过4.5ms的前导高电平信号。
				{delay014ms(1);}
				while (!(REG_RCNT & GPIO_SI))   //等 IR 变为高电平
				{delay014ms(1);}
				while (REG_RCNT & GPIO_SI)      //计算IR高电平时长
				{
					delay014ms(1);
					N++;        
					if (N>=30)
					{ 
						REG_IF |=IRQ_SERIAL;
						REG_IME=1;
						return;
					}              			//0.14ms计数过长自动离开。
				}                        		//高电平计数完毕             
				IRCOM[j]=IRCOM[j] >> 1;                 //数据最高位补"0"
				if (N>=8) 
				{IRCOM[j] = IRCOM[j] | 0x80;} //数据最高位补"1"
				N=0;
		}
	}
   //if (IRCOM[2]!=~IRCOM[3]) //不等的话表示解码失败
   //if(IRCOM[0]!=0x00)||(IRCOM[1]!=0xff)
   //{
	//	IRCOM[4]={0,0,0,0};
	//	REG_IF |=IRQ_SERIAL;
	//	REG_IME=1;
	//	return;
    //}
	
	if((IRCOM[2]+IRCOM[3])!=0xff)
   	{
		//IRCOM[4]={0,0,0,0};
		REG_IF |=IRQ_SERIAL;
		REG_IME=1;
		return;
    	}
	//iprintf("\x1b[5;0H%2X\n",IRCOM[2]);
	REG_IF |=IRQ_SERIAL;	//中断应答标志，必须在相应位写一，清除irq
	//REG_IME=1;
	return;
}

int main(void) {
//---------------------------------------------------------------------------------
	int keys_pressed, d;
	u16 i=0;
	REG_RCNT=0;
	REG_RCNT|=R_GPIO|GPIO_IRQ|GPIO_SC_OUTPUT|GPIO_SD_OUTPUT|GPIO_SO_OUTPUT;
	irqInit();
	consoleDemoInit();
	irqEnable(IRQ_VBLANK);
    	iprintf("\x1b[0;0HRDA5807 Demo.Chn L/R Vol U/D\n");
    	irqDisable(IRQ_VBLANK);
	RDA5807_power();
	delayms(20);
	RDA_reg_data[0] |= (1 << 1);
    	RDA5807_FM_seek();
	show_volume();
	show_frequency();
	//irqSet(IRQ_VBLANK,readkeys);
	//irqEnable(IRQ_VBLANK);
	irqSet(IRQ_SERIAL,IR_CODE);
	irqEnable(IRQ_SERIAL);
	REG_IME=0;
	while (1) 
	{	
		while(1)
		{
			//delayms(17);
			scanKeys();
			//VBlankIntrWait();
			keys_pressed = keysDown();
			if( keys_pressed & KEY_L) 
				{d=1;break;}
			else if(keys_pressed & KEY_R)
				{d=2;break;}
			else if(keys_pressed & KEY_UP)
				{d=3;break;}
			else if(keys_pressed & KEY_DOWN)
				{d=4;break;}
			d=0;
			//IR_Setup();
			//REG_IME = 0;
			//irqDisable(IRQ_VBLANK);
			//irqSet(IRQ_SERIAL,IR_CODE);
			//((RCNT*)REGRCNT)->SI_Int=1;
			//irqEnable(IRQ_SERIAL);//开启中�
			i=0;
			REG_IME = 1;
			//delayms(20);//等待中断半秒

			while(i<65535)
			{
				if(IRCOM[2]==0)
				{i++;}else{break;}
			}

			REG_IME = 0;
			//iprintf("\x1b[6;0HIR read=%2X\n",IRCOM[2]);
			//irqDisable(IRQ_SERIAL);//关闭中断
			if(IRCOM[2]==0x43)
			{
				d=1;iprintf("\x1b[7;0Hd=%d.\n",d);
				IRCOM[0]=IRCOM[1]=IRCOM[2]=IRCOM[3]=0;break;
			}
			else if(IRCOM[2]==0x40)
			{
				d=2;iprintf("\x1b[7;0Hd=%d.\n",d);
				IRCOM[0]=IRCOM[1]=IRCOM[2]=IRCOM[3]=0;break;
			}
			else if(IRCOM[2]==0x09)
			{
				d=3;iprintf("\x1b[7;0Hd=%d.\n",d);
				IRCOM[0]=IRCOM[1]=IRCOM[2]=IRCOM[3]=0;break;
			}
			else if(IRCOM[2]==0x15)
			{
				d=4;iprintf("\x1b[7;0Hd=%d.\n",d);
				IRCOM[0]=IRCOM[1]=IRCOM[2]=IRCOM[3]=0;break;
			}
			IRCOM[0]=IRCOM[1]=IRCOM[2]=IRCOM[3]=d=0;
		}
				
		if(d==2)
		{
			//iprintf("\x1b[7;0Hd=%d\n",d);
			delayms(20);
			RDA_reg_data[0] |= (1 << 1); 	//SEEK UP	 
			RDA5807_FM_seek();
			iprintf("\x1b[8;0Hseek up ok.\n");
		}
		if(d==1)
		{
			//iprintf("\x1b[7;0Hd=%d\n",d);
			delayms(20);
			RDA_reg_data[0] &= ~(1 << 1);  //SEEK DOWN		 
			RDA5807_FM_seek();
			iprintf("\x1b[8;0Hseek down ok.\n");
		}
		if(d==3)
		{
			//iprintf("\x1b[7;0Hd=%d\n",d);
			delayms(20);
			if((RDA_reg_data[7] & 0x0f) < 0x0f)
			 {
			   RDA_reg_data[0] = 0xd0;
			   RDA_reg_data[1] = 0x01;
			   RDA_reg_data[3] &= ~(1 << 4);

			   RDA_reg_data[7]++;	 // 音量递增
			   RDA5807_write_reg();
			  }
		}
		if(d==4)
		{
			//iprintf("\x1b[7;0Hd=%d\n",d);
			delayms(20);
			if((RDA_reg_data[7] & 0x0f) > 0x00)
			 {
				   RDA_reg_data[0] = 0xd0;
				   RDA_reg_data[1] = 0x01;
				   RDA_reg_data[3] &= ~(1 << 4);

				   RDA_reg_data[7]--;	 // 音量递减
				   RDA5807_write_reg();
			}
		}
		show_volume();
		show_frequency();
		d=0;
		//irqSet(IRQ_VBLANK,scanKeys);
		//irqEnable(IRQ_VBLANK);
	}
}		
		



		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
