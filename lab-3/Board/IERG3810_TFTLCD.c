#include "stm32f10x.h"
#include "IERG3810_TFTLCD.h"
#include "IERG3810_Clock.h"
#include "FONT.H"
#include "CFONT.H"

typedef struct{
    u16 LCD_REG;
    u16 LCD_RAM;
}LCD_TypeDef;


#define LCD_BASE ((u32)(0x6C000000|0x000007FE))
#define LCD ((LCD_TypeDef*) LCD_BASE)
#define LCD_LIGHT_ON GPIOB->BSRR |= 0x000000001;
void IERG3810_TFTLCD_init(void){//set FSMC
    RCC->AHBENR|=1<<8; //FSMC
    RCC->APB2ENR|=1<<3; //PORTB
    RCC->APB2ENR|=1<<5; //PORTD
    RCC->APB2ENR|=1<<6; //PORTE
    RCC->APB2ENR|=1<<8; //PORTG
    GPIOB->CRL&=0xFFFFFFF0; //PB0
    GPIOB->CRL|=0x00000003;

    //PORTD
    GPIOD->CRH&=0x00FFF000;
    GPIOD->CRH|=0xBB000BBB;
    GPIOD->CRL&=0xFF00FF00;
    GPIOD->CRL|=0x00BB00BB;
    //PORTE
    GPIOE->CRH&=0x00000000;
    GPIOE->CRH|=0xBBBBBBBB;
    GPIOE->CRL&=0x0FFFFFFF;
    GPIOE->CRL|=0xB0000000;
    //PORTG12
    GPIOG->CRH&=0xFFF0FFFF;
    GPIOG->CRH|=0x000B0000;
    GPIOG->CRL&=0xFFFFFFF0; //PG0->RS
    GPIOG->CRL|=0x0000000B;

    //LCD uses FSMC Bank 4 memory bank
    //Use mode A
    FSMC_Bank1->BTCR[6]=0x00000000; //FSMC_BCR4 (reset)
    FSMC_Bank1->BTCR[7]=0x00000000; //FSMC_BTR4 (reset)
    FSMC_Bank1E->BWTR[6]=0x00000000; //FSMC_BWTR4 (reset)
    FSMC_Bank1->BTCR[6]|=1<<12; //FSMC_BCR4 -> WREN
    FSMC_Bank1->BTCR[6]|=1<<14; //FSMC_BCR4 -> EXTMOD
    FSMC_Bank1->BTCR[6]|=1<<4; //FSMC_BCR4 -> MWID
    FSMC_Bank1->BTCR[7]|=0<<28; //FSMC_BTR4 -> ACCMOD
    FSMC_Bank1->BTCR[7]|=1<<0; //FSMC_BTR4 -> ADDSET
    FSMC_Bank1->BTCR[7]|=0xF<<8; //FSMC_BTR4 -> DATAST
    FSMC_Bank1E->BWTR[6]|=0<<28; //FSMC_BWTR4 -> ACCMOD
    FSMC_Bank1E->BWTR[6]|=0<<0; //FSMC_BWTR4 -> ADDSET
    FSMC_Bank1E->BWTR[6]|=3<<8; //FSMC_BWTR4 -> DATAST
    FSMC_Bank1->BTCR[6]|=1<<0; //FSMC_BCR4 -> FACCEN
    IERG3810_TFTLCD_SetParameter(); //special setting for LCD module
    LCD_LIGHT_ON;
}
void IERG3810_TFTLCD_SetParameter(void){
    IERG3810_TFTLCD_WrReg(0x01); //Software reset
    IERG3810_TFTLCD_WrReg(0x11); //Exit_sleep_mode

    IERG3810_TFTLCD_WrReg(0x3A); //Set_pixel_format
    IERG3810_TFTLCD_WrData(0x55); //65536 colors

    IERG3810_TFTLCD_WrReg(0x29); //Display ON

    IERG3810_TFTLCD_WrReg(0x36); //Memory Access Control (section 8.2.29, p. 127)
    IERG3810_TFTLCD_WrData(0xCA);
}

void IERG3810_TFTLCD_WrReg(u16 regval){
    LCD->LCD_REG=regval;
}
void IERG3810_TFTLCD_WrData(u16 data){
    LCD->LCD_RAM=data;
}
void IERG3810_TFTLCD_DrawDot(u16 x, u16 y, u16 color)
{
	IERG3810_TFTLCD_WrReg(0x2A);        //set x position
	IERG3810_TFTLCD_WrData(x >> 8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData(0x01);
	IERG3810_TFTLCD_WrData(0x3F);
	IERG3810_TFTLCD_WrReg(0x2B);        //set y position
	IERG3810_TFTLCD_WrData(y >> 8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData(0x01);
	IERG3810_TFTLCD_WrData(0xDF);
	IERG3810_TFTLCD_WrReg(0x2C);        //set point with color
	IERG3810_TFTLCD_WrData(color);
}
void IERG3810_TFTLCD_FillRectangle(u16 color, u16 start_x, u16 length_x, u16 start_y, u16 length_y){
    u32 index=0;
    IERG3810_TFTLCD_WrReg(0x2A);
    IERG3810_TFTLCD_WrData(start_x>>8);
    IERG3810_TFTLCD_WrData(start_x & 0xFF);
    IERG3810_TFTLCD_WrData((length_x+start_x-1)>>8);
    IERG3810_TFTLCD_WrData((length_x+start_x-1) & 0xFF);
    IERG3810_TFTLCD_WrReg(0x2B);
    IERG3810_TFTLCD_WrData(start_y>>8);
    IERG3810_TFTLCD_WrData(start_y & 0xFF);
    IERG3810_TFTLCD_WrData((length_y+start_y-1)>>8);
    IERG3810_TFTLCD_WrData((length_y+start_y-1) & 0xFF);
    IERG3810_TFTLCD_WrReg(0x2C); //LCD_WriteRAM_Prepare()
    for(index=0;index<length_x*length_y;index++){
        IERG3810_TFTLCD_WrData(color);
    }
}
void IERG3810_TFTLCD_SevenSegment(u16 color, u16 start_x, u16 start_y, u8 digit){
    switch(digit){
        case(0x00): // 0, we need segment abcdef
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y+150,10); //segment a
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+85,65); //segment b
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+10,65); //segment c
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y,10); //segment d
            IERG3810_TFTLCD_FillRectangle(color,start_x,10,start_y+10,65); //segment e
            IERG3810_TFTLCD_FillRectangle(color,start_x,10,start_y+85,65); //segment f
            break;
        case(0x01): // 1, we need segment bc
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+85,65); //segment b
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+10,65); //segment c
            break;
        case(0x02): // 2, we need segment abdeg
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y+150,10); //segment a
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+85,65); //segment b
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y,10); //segment d
            IERG3810_TFTLCD_FillRectangle(color,start_x,10,start_y+10,65); //segment e
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y+75,10); //segment g
            break;
        case(0x03): // 3, we need segment abcdg
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y+150,10); //segment a
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+85,65); //segment b
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+10,65); //segment c
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y,10); //segment d
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y+75,10); //segment g
            break;
        case(0x04): //4, we need segment bcfg
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+85,65); //segment b
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+10,65); //segment c
            IERG3810_TFTLCD_FillRectangle(color,start_x,10,start_y+85,65); //segment f
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y+75,10); //segment g
            break;
        case(0x05): //5, we need segment acdfg
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y+150,10); //segment a
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+10,65); //segment c
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y,10); //segment d
            IERG3810_TFTLCD_FillRectangle(color,start_x,10,start_y+85,65); //segment f
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y+75,10); //segment g
            break;
        case(0x06): //6, we need segment acdefg
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y+150,10); //segment a
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+10,65); //segment c
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y,10); //segment d
            IERG3810_TFTLCD_FillRectangle(color,start_x,10,start_y+10,65); //segment e
            IERG3810_TFTLCD_FillRectangle(color,start_x,10,start_y+85,65); //segment f
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y+75,10); //segment g
            break;
        case(0x07): //7, we need segment abc
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y+150,10); //segment a
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+85,65); //segment b
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+10,65); //segment c
            break;
        case(0x08): //8, we need segment abcdefg
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y+150,10); //segment a
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+85,65); //segment b
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+10,65); //segment c
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y,10); //segment d
            IERG3810_TFTLCD_FillRectangle(color,start_x,10,start_y+10,65); //segment e
            IERG3810_TFTLCD_FillRectangle(color,start_x,10,start_y+85,65); //segment f
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y+75,10); //segment g
            break;
        case(0x09): //9, we need abcdfg
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y+150,10); //segment a
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+85,65); //segment b
            IERG3810_TFTLCD_FillRectangle(color,start_x+70,10,start_y+10,65); //segment c
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y,10); //segment d
            IERG3810_TFTLCD_FillRectangle(color,start_x,10,start_y+85,65); //segment f
            IERG3810_TFTLCD_FillRectangle(color,start_x+10,60,start_y+75,10); //segment g
            break;
        default:
            break;
    }
}
void IERG3810_TFTLCD_ShowChar(u16 x,u16 y,u8 ascii, u16 color, u16 bgcolor){
	u8 i,j;
	u8 index;
	u8 height = 16, length = 8;
	
	if(ascii < 32 || ascii > 127) return;
	ascii -= 32;
	IERG3810_TFTLCD_WrReg(0x2A);
	IERG3810_TFTLCD_WrData(x>>8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1)>>8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	
	IERG3810_TFTLCD_WrReg(0x2B);
	IERG3810_TFTLCD_WrData(y>>8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1)>>8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	
	IERG3810_TFTLCD_WrReg(0x2C);//LCD_WriteRAM_Prepare();
	
	for(j=0; j<height/8; j++){
		
		for(i=0; i<height/2; i++){
			
			for(index=0; index<length;index++){
				if((asc2_1608[ascii][index*2+1-j]>>i) & 0x01)
					IERG3810_TFTLCD_WrData(color);
			  else
					IERG3810_TFTLCD_WrData(bgcolor);
			}
		}
	}
	
}
void IERG3810_TFTLCD_ShowChinChar(u16 x,u16 y,u8 code, u16 color, u16 bgcolor){
	u8 i,j;
	u8 index;
	u8 height = 16, length = 16;
	if(code >= 0x03) return;
	
	IERG3810_TFTLCD_WrReg(0x2A);
	IERG3810_TFTLCD_WrData(x>>8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1)>>8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	
	IERG3810_TFTLCD_WrReg(0x2B);
	IERG3810_TFTLCD_WrData(y>>8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1)>>8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	
	IERG3810_TFTLCD_WrReg(0x2C);//LCD_WriteRAM_Prepare();
	
	for(j=0; j<height/8; j++){
		
		for(i=0; i<height/2; i++){
			
			for(index=0; index<length;index++){
				if((chi_1616[code][index*2+1-j]>>i) & 0x01)
					IERG3810_TFTLCD_WrData(color);
			  else
					IERG3810_TFTLCD_WrData(bgcolor);
				
			}
		}
	}
	
}
void IERG3810_TFTLCD_ShowChar_overlay(u16 x,u16 y,u8 ascii, u16 color){
	u8 i, j;
	u8 index;
	u8 height = 16, length = 8;
	if (ascii < 32 || ascii > 127) return;
	ascii -= 32;
	IERG3810_TFTLCD_WrReg(0x2A);
	IERG3810_TFTLCD_WrData(x >> 8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B);
	IERG3810_TFTLCD_WrData(y >> 8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C);//LCD_WriteRAM_Prepare();
	
	for (j = 0; j < height / 8; j++)
	{
		for (i = 0; i < height / 2; i++)
		{
			for (index = 0; index < length; index++)
			{
				if ((asc2_1608[ascii][index*2+1-j] >> i) & 0x01)
				{
					if (j == 0)
					{
						IERG3810_TFTLCD_DrawDot(x+index, y+i, color);
					}
					else if (j == 1)
					{
						IERG3810_TFTLCD_DrawDot(x+index, y+i+7, color);
					}
				}
			}
		}
	}
}
