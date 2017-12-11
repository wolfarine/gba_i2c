
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
	// ����ģ��д�����
	I2C_write_byte(0x20);
	// �Ĵ�������д����
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
   // ����ģ���ȡ����
   I2C_write_byte(0x21);
   // �Ĵ�������������
   reg_buf[0] = I2C_read_byte(I2C_ACK);
   reg_buf[1] = I2C_read_byte(I2C_ACK);
   reg_buf[2] = I2C_read_byte(I2C_ACK);
   reg_buf[3] = I2C_read_byte(I2C_NACK);
   I2C_stop();
}
/**********************************************************

 ģ���ϵ��ʼ���Ӻ���

**********************************************************/
void RDA5807_power(void)
{
   delayms(50);

    // ���������λָ��
   RDA_reg_data[0] = 0x00;
   RDA_reg_data[1] = 0x02;
   RDA5807_write_reg();

   delayms(10);

    // ����ģ��Ĭ�ϲ���
   RDA_reg_data[0] = 0xd0;
   RDA_reg_data[1] = 0x01;
   RDA5807_write_reg();
}

/**********************************************************

 ��������������ģ���Զ�Ѱ̨ģʽ

**********************************************************/
void RDA5807_FM_seek(void)
{
   u16 chan;
   u8 i=0;
   u8  reg_data[4] = {0x00, 0x00, 0x00, 0x00};

   RDA_reg_data[3] &= ~(1 << 4);      //��г����

   // �ڲ��Զ�Ѱ̨ʹ��
   RDA_reg_data[0] |=  (1 << 0);      //SEEKλ��1
   RDA5807_write_reg();

    // �ȴ�STC ��־��λ
   while(0 == (reg_data[0] & 0x40))
   {
     delayms(20);
      // ��ȡ�ڲ�״̬
     RDA5807_read_reg(reg_data);
	 if(i<10)
	 i++;
	 else break;
   }
    // ��ȡ��ǰ����Ƶ��
   chan = reg_data[0] & 0x03;
   chan = reg_data[1] | (chan << 8);
   chan = chan << 6;

    // ���浱ǰ����Ƶ��
   RDA_reg_data[2] = (chan >> 8) & 0xff;
   RDA_reg_data[3] = (chan & 0xff);
}

/**********************************************************

 Ƶ����ʾ�Ӻ���

**********************************************************/
void  show_frequency(void)
{ 
   u8 i,display[5];
   u16 temp;

   temp = (RDA_reg_data[2]*256)+(RDA_reg_data[3]&0xc0);	 //����
   temp = temp>>6;
   frequency = (unsigned long)(100*temp+87000)/100;

   for(i=0; i<5; i++)  // ���Դ浥Ԫ
   display[i] = 0x00;

   display[0] = (frequency)/1000 ;     //����ת��
   display[1] = (frequency%1000)/100;
   display[2] = (frequency%100)/10;
   display[3] = 0x2e;                  //С����
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

   //lcd_pos_xy(3,2);                      //Ƶ����ʾ
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

 ������ʾ�Ӻ���

**********************************************************/
void show_volume()
{
   u8 temp,display[2];

   temp = RDA_reg_data[7] & 0x0f; //ȡ����ֵ

   display[0] = temp/10;
   display[1] = temp%10;

   if(display[0] == 0)            //�����λΪ0
   { 
     display[0] = display[1];     //��λ�Դ����ݽ����λ�Դ�
     display[1] = 0x20;           //��λ����ʾ
   }
   else 
   {
     display[1] += 0x30;
   }
   display[0] += 0x30;

   //lcd_pos_xy(13,2);              //����ֵ��ʾ
   //lcd_wdat(display[0]);
   //lcd_wdat(display[1]);
   //irqEnable(IRQ_VBLANK); 
   iprintf("\x1b[2;0HVolume %c%c\n",display[0],display[1]);
   //irqDisable(IRQ_VBLANK);
   
}

void IR_CODE(void)
{
	REG_IME = 0;
    //iprintf("\x1b[4;0Hin\n");            				//�ж��ܿ���
	u8 j,k,N=0;
    delay014ms(15);
	if (REG_RCNT & GPIO_SI)
    { 
		REG_IF |=IRQ_SERIAL;
		REG_IME =1;
		return;
	}                           			//ȷ��IR�źų���  
	while (!(REG_RCNT & GPIO_SI))           //��IR��Ϊ�ߵ�ƽ������9ms��ǰ���͵�ƽ�źš�
	{delay014ms(1);}
	for (j=0;j<4;j++)         				//�ռ���������
	{
		for (k=0;k<8;k++)        			//ÿ��������8λ
		{
			while (REG_RCNT & GPIO_SI)      //�� IR ��Ϊ�͵�ƽ������4.5ms��ǰ���ߵ�ƽ�źš�
			{delay014ms(1);}
			while (!(REG_RCNT & GPIO_SI))   //�� IR ��Ϊ�ߵ�ƽ
			{delay014ms(1);}
			while (REG_RCNT & GPIO_SI)      //����IR�ߵ�ƽʱ��
			{
				delay014ms(1);
				N++;        
				if (N>=30)
				{ 
					REG_IF |=IRQ_SERIAL;
					REG_IME=1;
					return;
				}              				//0.14ms���������Զ��뿪��
			}                        		//�ߵ�ƽ�������             
			IRCOM[j]=IRCOM[j] >> 1;                  //�������λ��"0"
			if (N>=8) 
			{IRCOM[j] = IRCOM[j] | 0x80;} //�������λ��"1"
			N=0;
		}
	}
   //if (IRCOM[2]!=~IRCOM[3]) //���ȵĻ���ʾ����ʧ��
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
	REG_IF |=IRQ_SERIAL;	//�ж�Ӧ���־����������Ӧλдһ�����irq
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
		//irqEnable(IRQ_SERIAL);//�����ж
		i=0;
		REG_IME = 1;
		//delayms(20);//�ȴ��жϰ���
		while(i<65535)
		{
		if(IRCOM[2]==0)
		i++;
		else break;
		}
		REG_IME = 0;
		//iprintf("\x1b[6;0HIR read=%2X\n",IRCOM[2]);
		//irqDisable(IRQ_SERIAL);//�ر��ж�
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

           RDA_reg_data[7]++;	 // ��������
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
           
           RDA_reg_data[7]--;	 // �����ݼ�
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
		



		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		