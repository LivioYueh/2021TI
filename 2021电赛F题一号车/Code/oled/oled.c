#include "oled.h"
#include "oledfont.h"


//m^n函数
static u32 oled_pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}
static void OLED_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(OLED_GPIO_CLK, ENABLE);

	GPIO_InitStruct.GPIO_Pin = OLED_SCL_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(OLED_SCL_GPIO_PORT, &GPIO_InitStruct);
	GPIO_SetBits(OLED_SCL_GPIO_PORT, OLED_SCL_GPIO_PIN);

	GPIO_InitStruct.GPIO_Pin = OLED_SDA_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(OLED_SDA_GPIO_PORT, &GPIO_InitStruct);
	GPIO_SetBits(OLED_SDA_GPIO_PORT, OLED_SDA_GPIO_PIN);
}
static void OLED_IIC_Start(void)
{
	OLED_IIC_SCL(1);
	OLED_IIC_SDA(1);
	OLED_IIC_SDA(0);
	OLED_IIC_SCL(0);
}
static void OLED_IIC_Stop(void)
{
	OLED_IIC_SCL(1);
//	OLED_IIC_SCL(0);
	OLED_IIC_SDA(0);
	OLED_IIC_SDA(1);
}
static void OLED_IIC_Wait_Ack(void)
{
	OLED_IIC_SCL(1) ;
	OLED_IIC_SCL(0);
}
static void OLED_Write_IIC_Byte(unsigned char IIC_Byte)
{
	unsigned char i;
	unsigned char m,da;
	da=IIC_Byte;
	OLED_IIC_SCL(0);
	for(i=0;i<8;i++)		
	{
		m=da;
		m=m&0x80;
		if(m==0x80)
		OLED_IIC_SDA(1);
		else
		OLED_IIC_SDA(0);
		da=da<<1;
		OLED_IIC_SCL(1);
		OLED_IIC_SCL(0);
	}
}
static void OLED_Write_IIC_Command(unsigned char IIC_Command)
{
	OLED_IIC_Start();
	OLED_Write_IIC_Byte(0x78);            //Slave address,SA0=0
	OLED_IIC_Wait_Ack();	
	OLED_Write_IIC_Byte(0x00);			//write command
	OLED_IIC_Wait_Ack();	
	OLED_Write_IIC_Byte(IIC_Command); 
	OLED_IIC_Wait_Ack();	
	OLED_IIC_Stop();
}
static void OLED_Write_IIC_Data(unsigned char IIC_Data)
{
	OLED_IIC_Start();
	OLED_Write_IIC_Byte(0x78);			//D/C#=0; R/W#=0
	OLED_IIC_Wait_Ack();	
	OLED_Write_IIC_Byte(0x40);			//write data
	OLED_IIC_Wait_Ack();	
	OLED_Write_IIC_Byte(IIC_Data);
	OLED_IIC_Wait_Ack();	
	OLED_IIC_Stop();
}
static void OLED_WR_Byte(unsigned dat,unsigned cmd)
{
	if(cmd)
	OLED_Write_IIC_Data(dat);
	else
	OLED_Write_IIC_Command(dat);
}

//坐标设置
void OLED_Set_Pos(unsigned char x, unsigned char y) 
{
	OLED_WR_Byte(0xb0+y,OLED_CMD);
	OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
	OLED_WR_Byte((x&0x0f),OLED_CMD); 
}
//开启OLED显示    
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//关闭OLED显示     
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}
//清屏函数,清完屏,整个屏幕是黑色的!
void OLED_Clear(void)  
{  
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA); 
	} //更新显示
}
//更新显示
void OLED_On(void)  
{  
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)OLED_WR_Byte(1,OLED_DATA); 
	}
}
//OLED初始化
void OLED_Init(void)
{
	OLED_Config();
	Delay_ms(800);

	OLED_WR_Byte(0xAE,OLED_CMD);//--display off
	OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
	OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
	OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  
	OLED_WR_Byte(0xB0,OLED_CMD);//--set page address
	OLED_WR_Byte(0x81,OLED_CMD); // contract control
	OLED_WR_Byte(0xFF,OLED_CMD);//--128   
	OLED_WR_Byte(0xA1,OLED_CMD);//set segment remap 
	OLED_WR_Byte(0xA6,OLED_CMD);//--normal / reverse
	OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
	OLED_WR_Byte(0x3F,OLED_CMD);//--1/32 duty
	OLED_WR_Byte(0xC8,OLED_CMD);//Com scan direction
	OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset
	OLED_WR_Byte(0x00,OLED_CMD);//
	
	OLED_WR_Byte(0xD5,OLED_CMD);//set osc division
	OLED_WR_Byte(0x80,OLED_CMD);//
	
	OLED_WR_Byte(0xD8,OLED_CMD);//set area color mode off
	OLED_WR_Byte(0x05,OLED_CMD);//
	
	OLED_WR_Byte(0xD9,OLED_CMD);//Set Pre-Charge Period
	OLED_WR_Byte(0xF1,OLED_CMD);//
	
	OLED_WR_Byte(0xDA,OLED_CMD);//set com pin configuartion
	OLED_WR_Byte(0x12,OLED_CMD);//
	
	OLED_WR_Byte(0xDB,OLED_CMD);//set Vcomh
	OLED_WR_Byte(0x30,OLED_CMD);//
	
	OLED_WR_Byte(0x8D,OLED_CMD);//set charge pump enable
	OLED_WR_Byte(0x14,OLED_CMD);//
	
	OLED_WR_Byte(0xAF,OLED_CMD);//--turn on oled panel
}
//在指定位置显示一个字符
void OLED_Show_Char(u8 x, u8 y, u8 chr, u8 size)
{      	
	unsigned char c=0,i=0;	
	c=chr-' ';//得到偏移后的值			
	if(x>Max_Column-1){x=0;y=y+2;}
	if(size ==16)
	{
		OLED_Set_Pos(x,y);	
		for(i=0;i<8;i++)
		OLED_WR_Byte(F8X16[c*16+i],OLED_DATA);
		OLED_Set_Pos(x,y+1);
		for(i=0;i<8;i++)
		OLED_WR_Byte(F8X16[c*16+i+8],OLED_DATA);
	}
	else 
	{	
		OLED_Set_Pos(x,y);
		for(i=0;i<6;i++)
		OLED_WR_Byte(F6x8[c][i],OLED_DATA);
	}
}
//在指定位置显示一个字符串
void OLED_Show_String(u8 x, u8 y, u8 *chr, u8 size)
{
	while (*chr!='\0')
	{
		OLED_Show_Char(x,y,*chr,size);
		x+=8;
		if(x>120){x=0;y+=2;}
		chr++;
	}
}
//在指定位置显示几位整数(有符号)
void OLED_Show_Int(u8 x, u8 y, int32_t num, u8 len, u8 size)
{         	
	u8 t,temp;
	u8 enshow=0;
	if(num<0)
	{
		num = -num;
		OLED_Show_Char(x, y, '-',size);
	}
	else
	    OLED_Show_Char(x, y, ' ',size);
	for(t=1;t<=len;t++)
	{
		temp=(num/oled_pow(10,len-t))%10;
		if(enshow==0&&t<(len))
		{
			if(temp==0)
			{
				OLED_Show_Char(x+(size/2)*t,y,' ',size);
				continue;
			}
			else enshow=1;
		}
	 	OLED_Show_Char(x+(size/2)*t,y,temp+'0',size); 
	}
}
//在指定位置显示几位整数(无符号)
void OLED_Show_Uint(u8 x, u8 y, u32 num, u8 len, u8 size)
{         	
	u8 t,temp;
	u8 enshow=0;
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				OLED_Show_Char(x+(size/2)*t,y,' ',size);
				continue;
			}
			else enshow=1;
		}
	 	OLED_Show_Char(x+(size/2)*t,y,temp+'0',size); 
	}
}
//在指定位置显示几位浮点数
void OLED_Show_Float(u8 x, u8 y, double num, u8 len1, u8 len2, u8 size) 
{
	int k = 1;
	int32_t num1, num2;

	if(num<0)
	{
		k = -1;
		num = -num;
	}
	num1 = (int)num;
	num2 = (int)(num*oled_pow(10,len2))-num1*oled_pow(10,len2);
	OLED_Show_Int(x, y, k*num1, len1, size);
	OLED_Show_Char(x+size/2*(1+len1), y, '.', size);
	OLED_Show_Uint(x+size/2*(2+len1), y, num2, len2, size);
}





