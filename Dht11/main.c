/*-----------------------------------------------
显示模块引脚定义：1-VSS 2-VDD 3-V0 4-RS 5-R/W 6-E 7-14 DB0-DB7 15-BLA 16-BLK
------------------------------------------------*/
#include <reg52.h> //包含头文件，一般情况不需要改动，头⽂件包含特殊功能寄存器的定义
#include <intrins.h>
#include <math.h>
#define uchar unsigned char
#define uint unsigned int
sbit RS = P1^0; //定义端口
sbit RW = P1^1;
sbit EN = P2^5;
sbit DU = P2^0;
sbit WE = P2^1;
sbit Data = P3^6;//定义数据线
static uchar RH,RL,TH,TL;
uchar rec_dat[15]; //用于显示的接收数据数组
uchar uart_rece[15];
uchar count=0;
uchar flag_rece=0;
uchar len_rece;
#define RS_CLR RS=0
#define RS_SET RS=1
#define RW_CLR RW=0
#define RW_SET RW=1
#define EN_CLR EN=0
#define EN_SET EN=1
#define DataPort P0
void cmg88()//关数码管，点阵函数
{
	DU=1;
	P0=0X00;
	DU=0;
}
/*------------------------------------------------
uS延时函数，含有输入参数 unsigned char t，无返回值
unsigned char 是定义无符号字符变量，其值的范围是
0~255 这里使用晶振12M，精确延时请使用汇编,大致延时
长度如下 T=tx2+5 uS
------------------------------------------------*/
void DelayUs2x(unsigned char t)
{
	while(--t);
}
/*------------------------------------------------
mS延时函数，含有输入参数 unsigned char t，无返回值
unsigned char 是定义无符号字符变量，其值的范围是
0~255 这里使用晶振12M，精确延时请使用汇编
------------------------------------------------*/
void DelayMs(unsigned char t)
{
	while(t--)
	{
	//⼤致延时1mS
		DelayUs2x(245);
		DelayUs2x(245);
	}
}
/*------------------------------------------------
判忙函数
------------------------------------------------*/
bit LCD_Check_Busy(void)
{
	DataPort= 0xFF;
	RS_CLR;
	RW_SET;
	EN_CLR;
	_nop_();
	EN_SET;
	return (bit)(DataPort & 0x80);
}
/*------------------------------------------------
写入命令函数
------------------------------------------------*/

void LCD_Write_Com(unsigned char com)
{
	while(LCD_Check_Busy()); //忙则等待
	RS_CLR;
	RW_CLR;
	EN_SET;
	DataPort= com;
	_nop_();
	EN_CLR;
}
/*------------------------------------------------
写入数据函数
------------------------------------------------*/
void LCD_Write_Data(unsigned char Data)
{
	while(LCD_Check_Busy()); //忙则等待
	RS_SET;
	RW_CLR;
	EN_SET;
	DataPort= Data;
	_nop_();
	EN_CLR;
}
/*------------------------------------------------
清屏函数
------------------------------------------------*/
void LCD_Clear(void)
{
	LCD_Write_Com(0x01);
	DelayMs(5);
}
/*------------------------------------------------
写入字符串函数
------------------------------------------------*/
void LCD_Write_String(unsigned char x,unsigned char y,unsigned char *s)
{
	if (y == 0)
	{
		LCD_Write_Com(0x80 + x); //表示第一行
	}
	else
	{
		LCD_Write_Com(0xC0 + x); //表示第二行
	}
	while (*s)
	{
		LCD_Write_Data( *s);
		s ++;
	}
}
/*------------------------------------------------
写入字符函数
------------------------------------------------*/
void LCD_Write_Char(unsigned char x,unsigned char y,unsigned char Data)
{
	if (y == 0)
	{
		LCD_Write_Com(0x80 + x);
	}
	else
	{
		LCD_Write_Com(0xC0 + x);
	}
	LCD_Write_Data( Data);
}
/*------------------------------------------------
初始化函数
------------------------------------------------*/
void LCD_Init(void)
{
	LCD_Write_Com(0x38); /*显示模式设置*/
	DelayMs(5);
	LCD_Write_Com(0x38);
	DelayMs(5);
	LCD_Write_Com(0x38);
	DelayMs(5);
	LCD_Write_Com(0x38);
	LCD_Write_Com(0x08); /*显示关闭*/
	LCD_Write_Com(0x01); /*显示清屏*/
	LCD_Write_Com(0x06); /*显示光标移动设置*/
	DelayMs(5);
	LCD_Write_Com(0x0C); /*显示开及光标设置*/
}
/*------------------------------------------------
DHT11延时函数
------------------------------------------------*/
void Delay_us(uchar n)
{
	while(--n);
}
void Delay_ms(uint z)
{
	uint i,j;
	for(i=z;i>0;i--)
	for(j=110;j>0;j--);
}
/*------------------------------------------------
DHT11开始信号
------------------------------------------------*/
void DHT11_start()
{
	Data=1;
	Delay_us(2);
	Data=0;
	Delay_ms(20); //延时18ms以上
	Data=1;
	Delay_us(30);
}
/*------------------------------------------------
接收8位2进制
------------------------------------------------*/
uchar DHT11_rec_byte() //接收1个字节
{
	uchar i,dat=0;
	for(i=0;i<8;i++) //从高到低依次接收8位数据
	{
		while(!Data);// 等待50us低电平过去
		Delay_us(8); //延时60us，如果还为高则数据为1，否则为0
		dat<<=1; //移位使正确接收8位数据，数据为0时直接移位
		if(Data==1) //数据为1时，使dat加1来接收数据1
		dat+=1;
		while(Data); //等待数据线拉低
	}
	return dat;
}
void send_char(unsigned char dat)
{
	ES=0;
	TI=0;
	SBUF=dat;
	while(!TI);
	TI=0;
	ES=1;
}
/*------------------------------------------------
接收40bit数据
------------------------------------------------*/
void DHT11_receive() //接收40位的数据
{
	uchar R_H,R_L,T_H,T_L,revise;
	DHT11_start();
	while(Data==1);
	if(Data==0)
	{
		while(Data==0); //等待拉高
		while(Data==1); 
//		Delay_us(40); //拉高后延时80us
		R_H=DHT11_rec_byte(); //接收湿度高8位
		R_L=DHT11_rec_byte(); //接收湿度低8位
		T_H=DHT11_rec_byte(); //接收温度高8位
		T_L=DHT11_rec_byte(); //接收温度低8位
		revise=DHT11_rec_byte(); //接收校正位
		Delay_us(25); //结束
		if((R_H+R_L+T_H+T_L)==revise) //校正
		{
			RH=R_H;
			RL=R_L;
			TH=T_H;
			TL=T_L;
		}
		/*数据处理，方便显示*/
		rec_dat[0]='0'+(RH/10);
		rec_dat[1]='0'+(RH%10);
		rec_dat[2] =' ';
		rec_dat[3] = 'R' ;
		rec_dat[4] = 'H';
		rec_dat[5]=' ';
		rec_dat[6]=' ';
		rec_dat[7]=' ';
		rec_dat[8]=' ';
		rec_dat[9]='0'+(TH/10);
		rec_dat[10]='0'+(TH%10);
		rec_dat[11] =' ';
		rec_dat[12] = 'C';
		rec_dat[13] = ' ' ;
		rec_dat[14]=' ';
		send_char(rec_dat[0]);
		send_char(rec_dat[1]);
		send_char(rec_dat[9]);
		send_char(rec_dat[10]);
	}
}
/*------------------------------------------------
串口初始化
------------------------------------------------*/
void UART_Init()
{
	SCON = 0x50; /* SCON: 模式1, 8位 UART, 使能接收*/
  TMOD |= 0x20; /* TMOD: timer 1, mode 2, 8位 reload*/
  TH1 = 0xFD; /* TH1:  reload value for 9600 baud @ 11.0592MHz  */
	TL1 = TH1;
  TR1 = 1; /* TR1:  timer 1 run   */
  EA = 1;    /*打开总中断*/
  ES = 1;       /*打开串口中断*/
}

void UART_SER (void) interrupt 4 //串行中断服务程序
{
  if(RI)          //判断是接收中断产生
  {
    RI=0;            //标志位清零
   	uart_rece[count]=SBUF;        //读入缓冲区的值
		if(uart_rece[count]=='\n')
		{
			flag_rece=1;
			len_rece=count;
			count=0;
		}
		else
			count++;
  }
} 
/*------------------------------------------------
主函数
------------------------------------------------*/
void main(void)
{
	uint i;
	UART_Init();
	cmg88();//关数码管，点阵函数
	LCD_Init();
	LCD_Clear();//清屏
	Delay_ms(1500); //DHT11上电后要等待1S以越过不稳定状态在此期间不能发送任何指令
	while(1)
	{
		if(flag_rece==1)
		{
			flag_rece=0;
//			LCD_Clear();
				for(i=0;i<len_rece;i++)
				{
					LCD_Write_Char(i,0,uart_rece[i]);
				}
		}
		DHT11_receive();
		for(i=0;i<15;i++)
		{
			LCD_Write_Char(i,1,rec_dat[i]);
		}
		Delay_ms(1500);
//LCD_Write_Char(0,0,'T');
//LCD_Write_Char(0,1,'H');
//LCD_Write_String(0,1,"HELLO");
	}
}