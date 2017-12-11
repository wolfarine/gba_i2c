#include "../include/i2c.h"
#include "../include/delay.h"
	
/**********************************************************

 启动子程序

 在 SCL 高电平期间 SDA 发生负跳变

**********************************************************/
void I2C_start()
{	
   //i2c_send_int();
   //REG_RCNT = 0x80F3;
   SD_1();
   SC_1();
   delayNOP();
   SD_0();
   delayNOP();
   SC_0();
}

/**********************************************************

 停止子函数

 在 SCL 高电平期间 SDA 发生正跳变

**********************************************************/
void I2C_stop()
{
   //i2c_send_int();
   SD_0();
   //REG_RCNT = 0x80F0;
   SC_1();
   delayNOP();
   SD_1();
   delayNOP();
}

/**********************************************************

 发送应答位子函数

 在 SDA 低电平期间 SCL 发生一个正脉冲

**********************************************************/
/*
void  iic_ack()
{
   i2c_send_int();
   SD_0();
   SC_1();
   delayNOP();
   SC_0();
   NOP;
   SD_1();
}
*/
/**********************************************************

 发送非应答位子函数

 在 SDA 高电平期间 SCL 发生一个正脉冲

**********************************************************/
/*
void  iic_noack()
{
   i2c_send_int();
   SD_1();
   SC_1();
   delayNOP();
   SC_0();
   delayNOP();
   SD_0();
}
*/
/**********************************************************

 应答位检测子函数

**********************************************************/
/*
bit iic_testack()
{
   bit ack_bit;
   i2c_recv_int();
   SD_1();          //置SDA为输入方式
   SC_1();
   delayNOP();
   ack_bit = (REG_RCNT & GPIO_SD);
   SC_0();
   NOP;
   return ack_bit;  //返回应答位
}
*/
/**********************************************************

 发送一个字节子程序

**********************************************************/
u8 I2C_write_byte(u8 indata)
{
	
    u8 i,j,ack=0;
	
	delayNOP();
    // I2C 发送8 位数据
   for (i=0; i<8; i++)
   {
     // 高在前低在后
     if (indata & 0x80)
      SD_1();
     else
      SD_0();

     // 发送左移一位
     indata <<= 1;
     delayNOP();
     SC_1();
     delayNOP();
     SC_0();
   }

    // 设置SDA为输入
   //i2c_recv_int();
   ((RCNT*)REGRCNT)->SD_Dir=0;
   delayNOP();

    // 读取从机应答状态
    SC_1();

    delayNOP();
	for(j=255;j>0;j--)
    {if(REG_RCNT & GPIO_SD)

    {
      ack = I2C_NACK;
	  
    }
    else
    {
      ack = I2C_ACK;
	  break;
    }
	}
    SC_0();
	//REG_RCNT = 0x80F0;
    ((RCNT*)REGRCNT)->SD_Dir=1;
	delayNOP();
    return ack;
}

/**********************************************************

  读一个字节子程序

**********************************************************/
u8 I2C_read_byte(u8 ack)
{
    u8 i, data1 = 0;

    // SDA 设置输入方向
    //i2c_recv_int();
	((RCNT*)REGRCNT)->SD_Dir=0;
	delayNOP();
    // I2C 接收8位数据
    for(i = 8; i > 0; i--)
    {
      data1 <<= 1;
      SC_1();
      delayNOP();

        // 高在前低在后
        if (REG_RCNT & GPIO_SD)
        {
          data1++;
		}
        SC_0();
        delayNOP();
    }
	((RCNT*)REGRCNT)->SD_Dir=1;
    // 主机发送应答状态
    if(ack == I2C_ACK)
      SD_0();
    else
      SD_1();

    delayNOP();
    SC_1();
    delayNOP();
    SC_0();
	
    return data1;
}

/*********************************************************/