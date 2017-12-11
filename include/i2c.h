//---------------------------------------------------------------------------------
#ifndef	_i2c_h_
#define	_i2c_h_
//---------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------------------
#include <gba_sio.h>
#define REGRCNT 0x4000134
#define i2c_send_int() (REG_RCNT = 0x80F0)
#define i2c_recv_int() (REG_RCNT = 0x80D0)
//#define SC_1() (REG_RCNT = REG_RCNT|GPIO_SC)
//#define SC_0() (REG_RCNT = REG_RCNT & ~GPIO_SC)
//#define SD_1() (REG_RCNT = REG_RCNT|GPIO_SD)
//#define SD_0() (REG_RCNT = REG_RCNT & ~GPIO_SD)
#define SC_1() ((RCNT*)REGRCNT)->SC=1
#define SC_0() ((RCNT*)REGRCNT)->SC=0
#define SD_1() ((RCNT*)REGRCNT)->SD=1
#define SD_0() ((RCNT*)REGRCNT)->SD=0
#define SO_1() ((RCNT*)REGRCNT)->SO=1
#define SO_0() ((RCNT*)REGRCNT)->SO=0
#define  I2C_ACK     1
#define  I2C_NACK    0
	
extern  void  I2C_start();
extern  void  I2C_stop();
//extern  void  iic_init();
//extern  void  iic_ack();
//extern  void  iic_noack();
//extern  bit   iic_testack();
extern  u8  I2C_read_byte(u8 ack);
extern  u8  I2C_write_byte(u8 indata);

typedef struct{
u16 SC:1;//1
u16 SD:1;//2
u16 SI:1;//4
u16 SO:1;//8
u16 SC_Dir:1;
u16 SD_Dir:1;
u16 SI_Dir:1;
u16 SO_Dir:1;
u16 SI_Int:1;
u16 Dummy:5;
u16 Mode:2;
}RCNT;





//---------------------------------------------------------------------------------
#ifdef __cplusplus
}	   // extern "C"
#endif
//---------------------------------------------------------------------------------
#endif // _i2c_h
