#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File

#define LED1	GpioDataRegs.GPADAT.bit.GPIO25
#define LED2	GpioDataRegs.GPADAT.bit.GPIO24
#define LED1_H	GpioDataRegs.GPASET.bit.GPIO25 = 1		//GPIO25 핀 HIGH
#define LED1_L	GpioDataRegs.GPACLEAR.bit.GPIO25 = 1	//GPIO25 핀 LOW
#define LED2_H	GpioDataRegs.GPASET.bit.GPIO24 = 1		//GPIO24 핀 HIGH
#define LED2_L	GpioDataRegs.GPACLEAR.bit.GPIO24 = 1	//GPIO24 핀 LOW
#define LED1_T	GpioDataRegs.GPATOGGLE.bit.GPIO25 = 1	//GPIO25 핀 현재값에 반전
#define LED2_T	GpioDataRegs.GPATOGGLE.bit.GPIO24 = 1	//GPIO24 핀 현재값에 반전
#define DIP1	GpioDataRegs.GPBDAT.bit.GPIO48
#define DIP2	GpioDataRegs.GPBDAT.bit.GPIO49

#define FNDA_H	GpioDataRegs.GPBSET.bit.GPIO50 = 1
#define FNDB_H	GpioDataRegs.GPBSET.bit.GPIO51 = 1
#define FNDC_H	GpioDataRegs.GPBSET.bit.GPIO52 = 1
#define FNDD_H	GpioDataRegs.GPBSET.bit.GPIO53 = 1
#define FNDA_L	GpioDataRegs.GPBCLEAR.bit.GPIO50 = 1
#define FNDB_L	GpioDataRegs.GPBCLEAR.bit.GPIO51 = 1
#define FNDC_L	GpioDataRegs.GPBCLEAR.bit.GPIO52 = 1
#define FNDD_L	GpioDataRegs.GPBCLEAR.bit.GPIO53 = 1

#define LCDD0_H	GpioDataRegs.GPBSET.bit.GPIO57 = 1
#define LCDD1_H	GpioDataRegs.GPBSET.bit.GPIO56 = 1
#define LCDD2_H	GpioDataRegs.GPBSET.bit.GPIO61 = 1
#define LCDD3_H	GpioDataRegs.GPBSET.bit.GPIO60 = 1
#define LCDE_H	GpioDataRegs.GPBSET.bit.GPIO55 = 1
#define LCDRS_H	GpioDataRegs.GPBSET.bit.GPIO54 = 1
#define LCDD0_L	GpioDataRegs.GPBCLEAR.bit.GPIO57 = 1
#define LCDD1_L	GpioDataRegs.GPBCLEAR.bit.GPIO56 = 1
#define LCDD2_L	GpioDataRegs.GPBCLEAR.bit.GPIO61 = 1
#define LCDD3_L	GpioDataRegs.GPBCLEAR.bit.GPIO60 = 1
#define LCDE_L	GpioDataRegs.GPBCLEAR.bit.GPIO55 = 1
#define LCDRS_L	GpioDataRegs.GPBCLEAR.bit.GPIO54 = 1

#define LINE1 0x80
#define LINE2 0x40
#define CHARACTER_NUMBER_MAX 16

void Gpio_Fnd_out(unsigned char da);
void Gpio_select(void);

void lcdprint_data(char *str);
void lcd_write(char data,unsigned char Rs);
void lcd_Gpio_data_out(unsigned char da);
void lcd_init(void);

int Cycle = 0;
int Pointer = 0;
char Direction = 1;
void RemoveCharacter();
void MoveCharacter();
void WriteOnLCD(char character, int column, int row);

void main(){

	InitSysCtrl();        //basic core initialization
	DINT;                //Disable all interrupts
	Gpio_select();
	lcd_init();

	// Initialize
	lcd_write('0', 1);
	lcd_write(0xC0, 0);
	lcd_write('0', 1);

	while(1)
	{

		if(DIP1 && DIP2) // Display 1, 2 Column
		{
			RemoveCharacter();
			MoveCharacter();
			WriteOnLCD('0', 0, Pointer);
			WriteOnLCD('0', 1, Pointer);
		}
		else if(DIP1 && !DIP2) // Display 1 Column
		{
			RemoveCharacter();
			MoveCharacter();
			WriteOnLCD('0', 0, Pointer);
		}
		else if(!DIP1 && DIP2) // Display 2 Column
		{
			RemoveCharacter();
			MoveCharacter();
			WriteOnLCD('0', 1, Pointer);
		}
		else if(!DIP1 && !DIP2) // Pause
		{

		}
		DELAY_US(300000);
	}
}

void WriteOnLCD(char character, int column, int row)
{
	lcd_write(LINE1 + LINE2 * column + row, 0); // Move Cursor
	DELAY_US(10000);
	lcd_write(character, 1);
}

void RemoveCharacter()
{
	WriteOnLCD(' ', 0, Pointer);
	WriteOnLCD(' ', 1, Pointer);
}

void MoveCharacter()
{
	if(Pointer >= 15 && Direction == 1) // Right End
	{
		Direction = -1;
	}
	else if(Pointer <= 0 && Direction == -1) // Left End
	{
		Direction = 1;

		Gpio_Fnd_out(++Cycle);
	}

	if(Direction == 1) // Move Right
	{
		Pointer++;
		LED1_L; LED2_T;
	}
	else if(Direction == -1) // Move Left
	{
		Pointer--;
		LED1_T; LED2_L;
	}
	else
	{
		lcd_write(0x01,0); // Clear Display
		DELAY_US(100000);
		lcdprint_data("Error2 - MoveChar");
		while(1);
	}
}

void Gpio_Fnd_out(unsigned char da)
{
	if(da & 0x01)	FNDA_H;
	else			FNDA_L;
	if(da & 0x02)	FNDB_H;
	else			FNDB_L;
	if(da & 0x04)	FNDC_H;
	else			FNDC_L;
	if(da & 0x08)	FNDD_H;
	else			FNDD_L;
}

void Gpio_select(void)
{
   EALLOW; // 보호 해제(레지스터를 사용할 때 보호를 풀었다가 다시 아요해주어야함)

    GpioCtrlRegs.GPAMUX2.all = 0; //GPIO16...gpio31
   	GpioCtrlRegs.GPBMUX2.all = 0;

   	// LED Register
   	GpioCtrlRegs.GPADIR.bit.GPIO25 = 1;	//OUTPUT 설정
   	GpioCtrlRegs.GPADIR.bit.GPIO24 = 1;	//OUTPUT 설정

   	// FND Register
   	GpioCtrlRegs.GPBDIR.bit.GPIO50 = 1;
   	GpioCtrlRegs.GPBDIR.bit.GPIO51 = 1;
   	GpioCtrlRegs.GPBDIR.bit.GPIO52 = 1;
   	GpioCtrlRegs.GPBDIR.bit.GPIO53 = 1;

	// LCD Register
	GpioCtrlRegs.GPBDIR.bit.GPIO54 = 1;
	GpioCtrlRegs.GPBDIR.bit.GPIO55 = 1;
	GpioCtrlRegs.GPBDIR.bit.GPIO57 = 1;
	GpioCtrlRegs.GPBDIR.bit.GPIO56 = 1;
	GpioCtrlRegs.GPBDIR.bit.GPIO61 = 1;
	GpioCtrlRegs.GPBDIR.bit.GPIO60 = 1;

	// DIP Register
	GpioCtrlRegs.GPBPUD.bit.GPIO48 = 0;
	GpioCtrlRegs.GPBPUD.bit.GPIO49 = 0;
	GpioCtrlRegs.GPBDIR.bit.GPIO48 = 0;
	GpioCtrlRegs.GPBDIR.bit.GPIO49 = 0;

   EDIS; // 보호 사용
}

void lcdprint_data(char *str)
{
	unsigned char i=0;
	while(str[i]!='\0')
	{
		lcd_write(str[i++],1);
	}
}

void lcd_write(char data,unsigned char Rs)
{
	if(Rs)	LCDRS_H;
	else	LCDRS_L;
	lcd_Gpio_data_out((data>>4) & 0x0f);
	DELAY_US(1);
	LCDE_H;
	DELAY_US(1);
	LCDE_L;
	DELAY_US(1);

	lcd_Gpio_data_out(data & 0x0f);
	DELAY_US(1);
	LCDE_H;
	DELAY_US(1);
	LCDE_L;
	DELAY_US(41);
}

void lcd_Gpio_data_out(unsigned char da)
{
	if(da & 0x1)	LCDD0_H;
	else	LCDD0_L;
	if(da & 0x2)	LCDD1_H;
	else	LCDD1_L;
	if(da & 0x4)	LCDD2_H;
	else	LCDD2_L;
	if(da & 0x8)	LCDD3_H;
	else	LCDD3_L;
}

void lcd_init(void)
{
	lcd_write(0x28,0);		//4bit data mode, 2 line, 5x7 dot
	lcd_write(0x28,0);
	lcd_write(0x0C,0);		//display on
	lcd_write(0x01,0);		//Display Clear
    DELAY_US(1960);
	lcd_write(0x06,0);		//Entry mode
}
