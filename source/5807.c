
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
	// ÊÕÒôÄ£¿éÐ´Èë²Ù×÷
	I2C_write_byte(0x20);
	// ¼Ä´æÆ÷Á¬ÐøÐ´²Ù×÷
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
   // ÊÕÒôÄ£¿é¶ÁÈ¡²Ù×÷
   I2C_write_byte(0x21);
   // ¼Ä´æÆ÷Á¬Ðø¶Á²Ù×÷
   reg_buf[0] = I2C_read_byte(I2C_ACK);
   reg_buf[1] = I2C_read_byte(I2C_ACK);
   reg_buf[2] = I2C_read_byte(I2C_ACK);
   reg_buf[3] = I2C_read_byte(I2C_NACK);
   I2C_stop();
}
/**********************************************************

 Ä£¿éÉÏµç³õÊ¼»¯×Óº¯Êý

**********************************************************/
void RDA5807_power(void)
{
   delayms(50);

    // ·¢ËÍÈí¼þ¸´Î»Ö¸Áî
   RDA_reg_data[0] = 0x00;
   RDA_reg_data[1] = 0x02;
   RDA5807_write_reg();

   delayms(10);

    // ÊÕÒôÄ£¿éÄ¬ÈÏ²ÎÊý
   RDA_reg_data[0] = 0xd0;
   RDA_reg_data[1] = 0x01;
   RDA5807_write_reg();
}

/**********************************************************

 ¹¦ÄÜÃèÊö£ºÊÕÒôÄ£¿é×Ô¶¯Ñ°Ì¨Ä£Ê½

**********************************************************/
void RDA5807_FM_seek(void)
{
   u16 chan;
   u8 i=0;
   u8  reg_data[4] = {0x00, 0x00, 0x00, 0x00};

   RDA_reg_data[3] &= ~(1 << 4);      //µ÷Ð³½ûÓÃ

   // ÄÚ²¿×Ô¶¯Ñ°Ì¨Ê¹ÄÜ
   RDA_reg_data[0] |=  (1 << 0);      //SEEKÎ»ÖÃ1
   RDA5807_write_reg();

    // µÈ´ýSTC ±êÖ¾ÖÃÎ»
   while(0 == (reg_data[0] & 0x40))
   {
     delayms(20);
      // ¶ÁÈ¡ÄÚ²¿×´Ì¬
     RDA5807_read_reg(reg_data);
	 if(i<10)
	 i++;
	 else break;
   }
    // »ñÈ¡µ±Ç°¹¤×÷Æµµã
   chan = reg_data[0] & 0x03;
   chan = reg_data[1] | (chan << 8);
   chan = chan << 6;

    // ±£´æµ±Ç°¹¤×÷Æµµã
   RDA_reg_data[2] = (chan >> 8) & 0xff;
   RDA_reg_data[3] = (chan & 0xff);
}

/**********************************************************

 ÆµÂÊÏÔÊ¾×Óº¯Êý

**********************************************************/
void  show_frequency(void)
{ 
   u8 i,display[5];
   u16 temp;

   temp = (RDA_reg_data[2]*256)+(RDA_reg_data[3]&0xc0);	 //¼ÆËã
   temp = temp>>6;
   frequency = (unsigned long)(100*temp+87000)/100;

   for(i=0; i<5; i++)  // ÇåÏÔ´æµ¥Ôª
   display[i] = 0x00;

   display[0] = (frequency)/1000 ;     //Êý¾Ý×ª»»
   display[1] = (frequency%1000)/100;
   display[2] = (frequency%100)/10;
   display[3] = 0x2e;                  //Ð¡Êýµã
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

   //lcd_pos_xy(3,2);                      //ÆµÂÊÏÔÊ¾
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

 ÒôÁ¿ÏÔÊ¾×Óº¯Êý

**********************************************************/
void show_volume()
{
   u8 temp,display[2];

   temp = RDA_reg_data[7] & 0x0f; //È¡ÒôÁ¿Öµ

   display[0] = temp/10;
   display[1] = temp%10;

   if(display[0] == 0)            //Èç¹û¸ßÎ»Îª0
   { 
     display[0] = display[1];     //µÍÎ»ÏÔ´æÄÚÈÝ½øÈë¸ßÎ»ÏÔ´æ
     display[1] = 0x20;           //µÍÎ»²»ÏÔÊ¾
   }
   else 
   {
     display[1] += 0x30;
   }
   display[0] += 0x30;

   //lcd_pos_xy(13,2);              //ÒôÁ¿ÖµÏÔÊ¾
   //lcd_wdat(display[0]);
   //lcd_wdat(display[1]);
   //irqEnable(IRQ_VBLANK); 
   iprintf("\x1b[2;0HVolume %c%c\n",display[0],display[1]);
   //irqDisable(IRQ_VBLANK);
   
}

void IR_CODE(void)
{
	REG_IME = 0;
    //iprintf("\x1b[4;0Hin\n");            				//ÖÐ¶Ï×Ü¿ª¹Ø
	u8 j,k,N=0;
    delay014ms(15);
	if (REG_RCNT & GPIO_SI)
    { 
		REG_IF |=IRQ_SERIAL;
		REG_IME =1;
		return;
	}                           			//È·ÈÏIRÐÅºÅ³öÏÖ  
	while (!(REG_RCNT & GPIO_SI))           //µÈIR±äÎª¸ßµçÆ½£¬Ìø¹ý9msµÄÇ°µ¼µÍµçÆ½ÐÅºÅ¡£
	{delay014ms(1);}
	for (j=0;j<4;j++)         				//ÊÕ¼¯ËÄ×éÊý¾Ý
	{
		for (k=0;k<8;k++)        			//Ã¿×éÊý¾ÝÓÐ8Î»
		{
			while (REG_RCNT & GPIO_SI)      //µÈ IR ±äÎªµÍµçÆ½£¬Ìø¹ý4.5msµÄÇ°µ¼¸ßµçÆ½ÐÅºÅ¡£
			{delay014ms(1);}
			while (!(REG_RCNT & GPIO_SI))   //µÈ IR ±äÎª¸ßµçÆ½
			{delay014ms(1);}
			while (REG_RCNT & GPIO_SI)      //¼ÆËãIR¸ßµçÆ½Ê±³¤
			{
				delay014ms(1);
				N++;        
				if (N>=30)
				{ 
					REG_IF |=IRQ_SERIAL;
					REG_IME=1;
					return;
				}              				//0.14ms¼ÆÊý¹ý³¤×Ô¶¯Àë¿ª¡£
			}                        		//¸ßµçÆ½¼ÆÊýÍê±Ï             
			IRCOM[j]=IRCOM[j] >> 1;                  //Êý¾Ý×î¸ßÎ»²¹"0"
			if (N>=8) 
			{IRCOM[j] = IRCOM[j] | 0x80;} //Êý¾Ý×î¸ßÎ»²¹"1"
			N=0;
		}
	}
   //if (IRCOM[2]!=~IRCOM[3]) //²»µÈµÄ»°±íÊ¾½âÂëÊ§°Ü
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
	REG_IF |=IRQ_SERIAL;	//ÖÐ¶ÏÓ¦´ð±êÖ¾£¬±ØÐëÔÚÏàÓ¦Î»Ð´Ò»£¬Çå³ýirq
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
		//irqEnable(IRQ_SERIAL);//¿ªÆôÖÐ¶
		i=0;
		REG_IME = 1;
		//delayms(20);//µÈ´ýÖÐ¶Ï°ëÃë
		while(i<65535)
		{
		if(IRCOM[2]==0)
		i++;
		else break;
		}
		REG_IME = 0;
		//iprintf("\x1b[6;0HIR read=%2X\n",IRCOM[2]);
		//irqDisable(IRQ_SERIAL);//¹Ø±ÕÖÐ¶Ï
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

           RDA_reg_data[7]++;	 // ÒôÁ¿µÝÔö
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
           
           RDA_reg_data[7]--;	 // ÒôÁ¿µÝ¼õ
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
		



		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		