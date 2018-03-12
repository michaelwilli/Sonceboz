/***************************************************************************************************
  * @file       NucleoF446_LCD.c
  * @author     BFH-TI / kemi
  * @version    V0.3
  * @date       01.03.2017
  * @brief      Display setup and communication for LCD on mbed-application-shield
  * @hardware   NucleoF446RB with mbed-application-shield
  **************************************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 BFH / Micha Kernen</center></h2>
  *
  */

/***************************************************************************************************
  **             Includes
  */
#include "NucleoF446_LCD.h"
#include "Font_6x8_h.h"
#include "Font_16x24_h.h"

#include <stdlib.h>
#include <string.h>

/***************************************************************************************************
  **            Local Variables
  */
uint32_t Display_Buffer[128];

/***************************************************************************************************
  **            Private Function Prototypes
  */
void Display_SPI_Config( void );
void Display_CS( uint8_t status );
void Display_A0( uint8_t status );
void Display_Reset( void );
void Display_Wr_Cmd( uint8_t data );
void Display_Wr_Dat( uint8_t data );

/***************************************************************************************************
  **            Private Functions
  */

/***************************************************************************************************
  * @name       Display_SPI_Config
  * @brief      Hardware Configuration of GPIOs and SPI Interface
  */
void Display_SPI_Config( void )
{
  GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

  /* Enable SPI2 clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  
  /* Enable GPIO clock for Port A and B */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1);

  /* Configure PA5 and PA7 as AF, for SPI */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure PA6 for Display Reset */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure PB6 for Display CS */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Chip Select auf aus */
  Display_CS(1);

  /* Configure PA8 for Display address */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* SPI configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &SPI_InitStructure);

  SPI_Cmd(SPI1, ENABLE);
}

/***************************************************************************************************
  * @name       Display_CS
  * @brief      Set or Reset the Displays Chip Select Pin (0 => Set, 1=> Reset)
  */
void Display_CS( uint8_t status )
{
  if(status) {
    GPIO_SetBits(GPIOB, GPIO_Pin_6);
  }
  else {
    GPIO_ResetBits(GPIOB, GPIO_Pin_6);
  }
}

/***************************************************************************************************
  * @name       Display_A0
  * @brief      Set or Reset the Displays Address Pin (0 => Command, 1=> Data)
  */
void Display_A0( uint8_t status )
{
  if(status) {
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
  }
  else {
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
  }
}

/***************************************************************************************************
  * @name       Display_Reset
  * @brief      Hardware Reset of Display
  */
void Display_Reset( void )
{
  int i;

  Display_CS(0);
  GPIO_ResetBits(GPIOA, GPIO_Pin_6);
  for(i=0;i<2000000;i++);
  GPIO_SetBits(GPIOA, GPIO_Pin_6);
  Display_CS(1);
}

/***************************************************************************************************
  * @name       Display_Wr_Cmd
  * @brief      Send Command to Display
  */
void Display_Wr_Cmd( uint8_t data ) 
{
  /* CS und A0 schalten */
  Display_A0(0);
  Display_CS(0);

  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  /* Send SPIz data */
  SPI_I2S_SendData(SPI1, (uint16_t) data);
  /* Wait until byte has been written */
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) != RESET);

  /* CS aufheben */
  Display_CS(1);
}

/***************************************************************************************************
  * @name       Display_Wr_Dat
  * @brief      Send Data to Display
  */
void Display_Wr_Dat( uint8_t data )
{
  /* CS und A0 schalten */
  Display_A0(1);
  Display_CS(0);

  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  /* Send SPIz data */
  SPI_I2S_SendData(SPI1, (uint16_t) data);
  /* Wait until byte has been written */
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) != RESET);

  /* CS aufheben */
  Display_CS(1);
}

/***************************************************************************************************
  **            Public Functions
  */

/***************************************************************************************************
  * @name       NUC_LCD_Init
  * @brief      Initialisation of Hardware and Software parts to use Display
  */
void NUC_LCD_Init( void ) 
{
  /* Clear LCD Buffer */
  NUC_LCD_Clear();

  /* Do Hardware Configuration */
  Display_SPI_Config();

  /* Display aus dem Reset Zustand rausholen */
  Display_Reset();

  Display_Wr_Cmd(0xa0); 		/* ADC select in normal mode */
  Display_Wr_Cmd(0xae); 		/* Display OFF */
  Display_Wr_Cmd(0xc8); 		/* Common output mode select: reverse direction (last 3 bits are ignored) */
  Display_Wr_Cmd(0xa2); 		/* LCD bias set at 1/9 */
  Display_Wr_Cmd(0x2f); 		/* Power control set to operating mode: 7 */
  Display_Wr_Cmd(0x21); 		/* Internal resistor ratio, set to: 1 */
  NUC_LCD_SetContrast(40); 	/* Set contrast, value experimentally determined, can set to 6-bit value, 0 to 63 */
  Display_Wr_Cmd(0xaf); 		/* Display on */
}

/***************************************************************************************************
  * @name       NUC_LCD_SetPixel
  * @brief      Set a pixel in the Display buffer
  */
void NUC_LCD_SetPixel ( uint8_t x, uint8_t y, uint8_t val )
{
  if((x >= 128) || (y >= 32)) {
    return;
  }
  if(val) {
    Display_Buffer[x] |= (1<<y);
  }
  else {
    Display_Buffer[x] &= ~(1<<y);
  }
}

/***************************************************************************************************
  * @name       NUC_LCD_Clear
  * @brief      Clear the Display buffer
  */
void NUC_LCD_Clear( void ) 
{
  uint8_t i;

  for(i=0;i<128;i++) {
    Display_Buffer[i] = 0;
  }
}

/***************************************************************************************************
  * @name       NUC_LCD_SetContrast
  * @brief      Set a Contrast Value for the LCD
  *             Can set a 6-bit value (0 to 63)
  */
void NUC_LCD_SetContrast(uint8_t val)
{
  /* Set electronic Volume mode */
  Display_Wr_Cmd(0x81);

  /* Set contrat value */
  if(val > 63) {
    Display_Wr_Cmd(63);
  } 
  else {
    Display_Wr_Cmd(val);
  }
}

/***************************************************************************************************
  * @name       NUC_LCD_SetString
  * @brief      Routine to set a String of Characters
  */
void NUC_LCD_SetString ( uint8_t x, uint8_t y, const char *s, uint8_t size )
{
  while (*s) {
    if(size == 8) {
      NUC_LCD_SetChar8(x, y, *s++);
      x+=6;
    }
    else if (size == 24){
      NUC_LCD_SetChar24(x, y, *s++);
      x+=16;
    }
    else {
      //nop
    }
  }
}

/***************************************************************************************************
  * @name       NUC_LCD_SetChar8
  * @brief      Routine to Set 8 px Characters
  */
static void NUC_LCD_SetChar8 ( uint8_t x, uint8_t y, unsigned char c )
{
  int i,j;
  c -= 32;
  for(i=0;i<8;i++) {
    for(j=0;j<6;j++) {
      if(Font_6x8_h[(c*8)+i] & (1<<j)) {
        NUC_LCD_SetPixel((x)+j, y+i, 1);
      }
      else {
        NUC_LCD_SetPixel((x)+j, y+i, 0);
      }
    }
  }
}

/***************************************************************************************************
  * @name       NUC_LCD_SetChar24
  * @brief      Routine to Set 24 px Characters
  */
static void NUC_LCD_SetChar24 ( uint8_t x, uint8_t y, unsigned char c )
{
  int i,j;
  c -= 32;
  for(i=0;i<24;i++) {
    for(j=0;j<16;j++) {
      if(Font_16x24_h[(c*24)+i] & (1<<j)) {
        NUC_LCD_SetPixel((x)+j, y+i, 1);
      }
      else {
        NUC_LCD_SetPixel((x)+j, y+i, 0);
      }
    }
  }
}

/***************************************************************************************************
  * @name       NUC_LCD_Show
  * @brief      Show content of Display buffer on LCD
  */
void NUC_LCD_Show( void )
{
  uint32_t l;

  // Display pointer auf 0 setzen
  Display_Wr_Cmd(0xB0);
  Display_Wr_Cmd(0x10);
  Display_Wr_Cmd(0x00);
  for(l=0;l<128;l++) {
    Display_Wr_Dat((Display_Buffer[l]>> 0) & 0xFF);
  }

  Display_Wr_Cmd(0xB1);
  Display_Wr_Cmd(0x10);
  Display_Wr_Cmd(0x00);
  for(l=0;l<128;l++) {
    Display_Wr_Dat((Display_Buffer[l]>> 8) & 0xFF);
  }

  Display_Wr_Cmd(0xB2);
  Display_Wr_Cmd(0x10);
  Display_Wr_Cmd(0x00);
  for(l=0;l<128;l++) {
    Display_Wr_Dat((Display_Buffer[l]>>16) & 0xFF);
  }

  Display_Wr_Cmd(0xB3);
  Display_Wr_Cmd(0x10);
  Display_Wr_Cmd(0x00);
  for(l=0;l<128;l++) {
    Display_Wr_Dat((Display_Buffer[l]>>24) & 0xFF);
  }
}

///*******************************************************************************
//* Draw bargraph                                                                *
//*   Parameter:      x:        horizontal position                              *
//*                   y:        vertical position                                *
//*                   w:        maximum width of bargraph (in pixels)            *
//*                   h:        bargraph height                                  *
//*                   val:      value of active bargraph (in 1/1024)             *
//*   Return:                                                                    *
//*******************************************************************************/

//void GLCD_Bargraph (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int val) {
//  int i,j;

//  val = (val * w) >> 10;                /* Scale value                        */
//  GLCD_SetWindow(x, y, w, h);
//  wr_cmd(0x22);
//  wr_dat_start();
//  for (i = 0; i < h; i++) {
//    for (j = 0; j <= w-1; j++) {
//      if(j >= val) {
//        wr_dat_only(Color[BG_COLOR]);
//      } else {
//        wr_dat_only(Color[TXT_COLOR]);
//      }
//    }
//  }
//  wr_dat_stop();
//}


///** ----------------------------------------------------------------------------
//  * @name   GLCD_DrawLine
//  * @brief  Draws a line from position x1, y1 to position x2, y2
//  * @param  x1:	horizontal position 1
//  * @param  y1:	vertical position 1
//  * @param  x2:	horizontal position 2
//  * @param  y2:	vertical position 2
//  * @retval None
//----------------------------------------------------------------------------- */
//void BFH_GLCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
//{
//	int16_t i, x[2], y[2];
//	float xF, yF;
//	
//	xF = x1, x[1] = 0;
//	yF = y1, y[1] = 0;
//	
//	x[0] = abs(x2-x1);
//	y[0] = abs(y2-y1);
//	
//	if(x[0]) x[1] = (x2-x1)/x[0];
//	if(y[0]) y[1] = (y2-y1)/y[0];
//	
//	if(x[0] >= y[0]) {
//		for(i = 0; i<x[0]; i++) {
//			BFH_GLCD_PutPixel(xF, yF);
//			xF += x[1];
//			yF += ((float)y[0]/x[0])*y[1];
//		}
//	}
//	else {
//		for(i = 0; i<y[0]; i++) {
//			BFH_GLCD_PutPixel(xF, yF);
//			for(cnt=0;cnt<1000;cnt++);
//			xF += ((float)x[0]/y[0])*x[1];
//			yF += y[1];
//		}
//	}
//}

///** ----------------------------------------------------------------------------
//  * @name   GLCD_DrawRect
//  * @brief  Draws a rectangle at position x, y (upper left corner) with size of
//	*					w, h
//  * @param  x:	horizontal position
//  * @param  y:	vertical position
//  * @param  w:	width
//  * @param  h:	height
//  * @retval None
//----------------------------------------------------------------------------- */
//void BFH_GLCD_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
//	
//	#if (USE_UDP == 1)
//		sendbuf = udp_get_buf (SENDLEN);
//		sendbuf[0] = 0x09;
//		sendbuf[1] = (x&0xFF00)>>8;
//		sendbuf[2] = x&0xFF;
//		sendbuf[3] = (y&0xFF00)>>8;
//		sendbuf[4] = y&0xFF;
//		sendbuf[5] = (w&0xFF00)>>8;
//		sendbuf[6] = w&0xFF;
//		sendbuf[7] = (h&0xFF00)>>8;
//		sendbuf[8] = h&0xFF;
//		udp_send (socket_udp, Rem_IP, 1001, sendbuf, SENDLEN);
//	#endif
//	
//	/* Set window */
//	GLCD_SetWindow (x, y, w, h);
//	
//	/* Draw rectangle */
//	wr_cmd(0x22);
//  wr_dat_start();
//	
//	for (y = 0; y < h; y++) {
//		for(x = 0; x < w; x++) {
//			wr_dat_only (ColorBFH);
//		}
//  }
//	
//	wr_dat_stop();	
//}

/****** END OF FILE ******/
