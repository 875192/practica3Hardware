/*********************************************************************************************
* Fichero:	lcd.c
* Autor:	
* Descrip:	funciones de visualizaci�n y control LCD
* Version:	<P6-ARM>
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include <string.h>
#include "def.h"
#include "44b.h"
#include "44blib.h"
#include "lcd.h"
#include "Bmp.h"
#include "celda.h"
#include "sudoku_2025.h"
#include "maquina_estados.h"  /* Máquina de estados del juego (para integración con touchscreen) */

/*--- definici�n de macros ---*/
#define DMA_Byte  (0)
#define DMA_HW    (1)
#define DMA_Word  (2)
#define DW 		  DMA_Byte		//configura  ZDMA0 como media palabras
	
/*--- variables externas ---*/
extern INT8U g_auc_Ascii8x16[];
extern INT8U g_auc_Ascii6x8[];
extern STRU_BITMAP Stru_Bitmap_gbMouse;

/*--- variables globales para región expandida ---*/
static int g_region_expandida_activa = 0;  /* 1 si hay región expandida mostrada */
static int g_region_fila_actual = 0;       /* Fila de región actual (0-2) */
static int g_region_col_actual = 0;        /* Columna de región actual (0-2) */
static int g_celda_seleccionada_i = -1;    /* Fila seleccionada dentro de región (0-2, -1=ninguna) */
static int g_celda_seleccionada_j = -1;    /* Columna seleccionada dentro de región (0-2, -1=ninguna) */

/*--- c�digo de la funci�n ---*/
void Lcd_Init(void)
{       
	rDITHMODE=0x1223a;
	rDP1_2 =0x5a5a;      
	rDP4_7 =0x366cd9b;
	rDP3_5 =0xda5a7;
	rDP2_3 =0xad7;
	rDP5_7 =0xfeda5b7;
	rDP3_4 =0xebd7;
	rDP4_5 =0xebfd7;
	rDP6_7 =0x7efdfbf;

	rLCDCON1=(0)|(1<<5)|(MVAL_USED<<7)|(0x0<<8)|(0x0<<10)|(CLKVAL_GREY16<<12);
	rLCDCON2=(LINEVAL)|(HOZVAL<<10)|(10<<21); 
	rLCDSADDR1= (0x2<<27) | ( ((LCD_ACTIVE_BUFFER>>22)<<21 ) | M5D(LCD_ACTIVE_BUFFER>>1));
 	rLCDSADDR2= M5D(((LCD_ACTIVE_BUFFER+(SCR_XSIZE*LCD_YSIZE/2))>>1)) | (MVAL<<21);
	rLCDSADDR3= (LCD_XSIZE/4) | ( ((SCR_XSIZE-LCD_XSIZE)/4)<<9 );
	// enable,4B_SNGL_SCAN,WDLY=8clk,WLH=8clk,
	rLCDCON1=(1)|(1<<5)|(MVAL_USED<<7)|(0x3<<8)|(0x3<<10)|(CLKVAL_GREY16<<12);
	rBLUELUT=0xfa40;
	//Enable LCD Logic and EL back-light.
	rPDATE=rPDATE&0x0e;
	
	//DMA ISR
	rINTMSK &= ~(BIT_GLOBAL|BIT_ZDMA0);
    pISR_ZDMA0=(int)Zdma0Done;
}

/*********************************************************************************************
* name:		Lcd_Active_Clr()
* func:		clear LCD screen
* para:		none 
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_Active_Clr(void)
{
	INT32U i;
	INT32U *pDisp = (INT32U *)LCD_ACTIVE_BUFFER;
	
	for( i = 0; i < (SCR_XSIZE*SCR_YSIZE/2/4); i++ )
	{
		*pDisp++ = WHITE;
	}
}

/*********************************************************************************************
* name:		Lcd_GetPixel()
* func:		Get appointed point's color value
* para:		usX,usY -- pot's X-Y coordinate 
* ret:		pot's color value
* modify:
* comment:		
*********************************************************************************************/
INT8U LCD_GetPixel(INT16U usX, INT16U usY)
{
	INT8U ucColor;

	ucColor = *((INT8U*)(LCD_VIRTUAL_BUFFER + usY*SCR_XSIZE/2 + usX/8*4 + 3 - (usX%8)/2));
	ucColor = (ucColor >> ((1-(usX%2))*4)) & 0x0f;
	return ucColor;
}

/*********************************************************************************************
* name:		Lcd_Active_Clr()
* func:		clear virtual screen
* para:		none 
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_Clr(void)
{
	INT32U i;
	INT32U *pDisp = (INT32U *)LCD_VIRTUAL_BUFFER;
	
	for( i = 0; i < (SCR_XSIZE*SCR_YSIZE/2/4); i++ )
	{
		*pDisp++ = WHITE;
	}
}

/*********************************************************************************************
* name:		LcdClrRect()
* func:		fill appointed area with appointed color
* para:		usLeft,usTop,usRight,usBottom -- area's rectangle acme coordinate
*			ucColor -- appointed color value
* ret:		none
* modify:
* comment:	also as clear screen function 
*********************************************************************************************/
void LcdClrRect(INT16 usLeft, INT16 usTop, INT16 usRight, INT16 usBottom, INT8U ucColor)
{
	INT16 i,k,l,m;
	
	INT32U ulColor = (ucColor << 28) | (ucColor << 24) | (ucColor << 20) | (ucColor << 16) | 
				     (ucColor << 12) | (ucColor << 8) | (ucColor << 4) | ucColor;

	i = k = l = m = 0;	
	if( (usRight-usLeft) <= 8 )
	{
		for( i=usTop; i<=usBottom; i++)
		{
			for( m=usLeft; m<=usRight; m++)
			{
				(LCD_PutPixel(m, i, ucColor));
			}
		}	
		return;
	}

	/* check borderline */
	if( 0 == (usLeft%8) )
		k=usLeft;
	else
	{
		k=(usLeft/8)*8+8;
	}
	if( 0 == (usRight%8) )
		l= usRight;
	else
	{
		l=(usRight/8)*8;
	}

	for( i=usTop; i<=usBottom; i++ )
	{
		for( m=usLeft; m<=(k-1); m++ )
		{
			(LCD_PutPixel(m, i, ucColor));
		}
		for( m=k; m<l; m+=8 )
		{
			(*(INT32U*)(LCD_VIRTUAL_BUFFER + i * SCR_XSIZE / 2 + m / 2)) = ulColor;
		}
		for( m=l; m<=usRight; m++ )
		{
			(LCD_PutPixel(m, i, ucColor));
		}
	}
}

/*********************************************************************************************
* name:		Lcd_Draw_Box()
* func:		Draw rectangle with appointed color
* para:		usLeft,usTop,usRight,usBottom -- rectangle's acme coordinate
*			ucColor -- appointed color value
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_Draw_Box(INT16 usLeft, INT16 usTop, INT16 usRight, INT16 usBottom, INT8U ucColor)
{
	Lcd_Draw_HLine(usLeft, usRight,  usTop,    ucColor, 1);
	Lcd_Draw_HLine(usLeft, usRight,  usBottom, ucColor, 1);
	Lcd_Draw_VLine(usTop,  usBottom, usLeft,   ucColor, 1);
	Lcd_Draw_VLine(usTop,  usBottom, usRight,  ucColor, 1);
}

/*********************************************************************************************
* name:		Lcd_Draw_Line()
* func:		Draw line with appointed color
* para:		usX0,usY0 -- line's start point coordinate
*			usX1,usY1 -- line's end point coordinate
*			ucColor -- appointed color value
*			usWidth -- line's width
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_Draw_Line(INT16 usX0, INT16 usY0, INT16 usX1, INT16 usY1, INT8U ucColor, INT16U usWidth)
{
	INT16 usDx;
	INT16 usDy;
	INT16 y_sign;
	INT16 x_sign;
	INT16 decision;
	INT16 wCurx, wCury, wNextx, wNexty, wpy, wpx;

	if( usY0 == usY1 )
	{
		Lcd_Draw_HLine (usX0, usX1, usY0, ucColor, usWidth);
		return;
	}
	if( usX0 == usX1 )
	{
		Lcd_Draw_VLine (usY0, usY1, usX0, ucColor, usWidth);
		return;
	}
	usDx = abs(usX0 - usX1);
	usDy = abs(usY0 - usY1);
	if( ((usDx >= usDy && (usX0 > usX1)) ||
        ((usDy > usDx) && (usY0 > usY1))) )
    {
        GUISWAP(usX1, usX0);
        GUISWAP(usY1, usY0);
    }
    y_sign = (usY1 - usY0) / usDy;
    x_sign = (usX1 - usX0) / usDx;

    if( usDx >= usDy )
    {
        for( wCurx = usX0, wCury = usY0, wNextx = usX1,
             wNexty = usY1, decision = (usDx >> 1);
             wCurx <= wNextx; wCurx++, wNextx--, decision += usDy )
        {
            if( decision >= usDx )
            {
                decision -= usDx;
                wCury += y_sign;
                wNexty -= y_sign;
            }
            for( wpy = wCury - usWidth / 2;
                 wpy <= wCury + usWidth / 2; wpy++ )
            {
                (LCD_PutPixel(wCurx, wpy, ucColor));
            }

            for( wpy = wNexty - usWidth / 2;
                 wpy <= wNexty + usWidth / 2; wpy++ )
            {
                (LCD_PutPixel(wNextx, wpy, ucColor));
            }
        }
    }
    else
    {
        for( wCurx = usX0, wCury = usY0, wNextx = usX1,
             wNexty = usY1, decision = (usDy >> 1);
             wCury <= wNexty; wCury++, wNexty--, decision += usDx )
        {
            if( decision >= usDy )
            {
                decision -= usDy;
                wCurx += x_sign;
                wNextx -= x_sign;
            }
            for( wpx = wCurx - usWidth / 2;
                 wpx <= wCurx + usWidth / 2; wpx++ )
            {
                (LCD_PutPixel(wpx, wCury, ucColor));
            }

            for( wpx = wNextx - usWidth / 2;
                 wpx <= wNextx + usWidth / 2; wpx++ )
            {
                (LCD_PutPixel(wpx, wNexty, ucColor));
            }
        }
    }
}

/*********************************************************************************************
* name:		Lcd_Draw_HLine()
* func:		Draw horizontal line with appointed color
* para:		usX0,usY0 -- line's start point coordinate
*			usX1 -- line's end point X-coordinate
*			ucColor -- appointed color value
*			usWidth -- line's width
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_Draw_HLine(INT16 usX0, INT16 usX1, INT16 usY0, INT8U ucColor, INT16U usWidth)
{
	INT16 usLen;

    if( usX1 < usX0 )
    {
        GUISWAP (usX1, usX0);
    }

    while( (usWidth--) > 0 )
    {
        usLen = usX1 - usX0 + 1;
        while( (usLen--) > 0 )
        {
        	(LCD_PutPixel(usX0 + usLen, usY0, ucColor));
        }
        usY0++;
    }
}

/*********************************************************************************************
* name:		Lcd_Draw_VLine()
* func:		Draw vertical line with appointed color
* para:		usX0,usY0 -- line's start point coordinate
*			usY1 -- line's end point Y-coordinate
*			ucColor -- appointed color value
*			usWidth -- line's width
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_Draw_VLine (INT16 usY0, INT16 usY1, INT16 usX0, INT8U ucColor, INT16U usWidth)
{
	INT16 usLen;

    if( usY1 < usY0 )
    {
        GUISWAP (usY1, usY0);
    }

    while( (usWidth--) > 0 )
    {
        usLen = usY1 - usY0 + 1;
        while( (usLen--) > 0 )
        {
        	(LCD_PutPixel(usX0, usY0 + usLen, ucColor));
        }
        usX0++;
    }
}

void Lcd_DisplayString(INT16U usX0, INT16U usY0, INT8U *pucStr){

}

/*********************************************************************************************
* name:		Lcd_DspAscII8x16()
* func:		display 8x16 ASCII character string 
* para:		usX0,usY0 -- ASCII character string's start point coordinate
*			ForeColor -- appointed color value
*			pucChar   -- ASCII character string
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_DspAscII8x16(INT16U x0, INT16U y0, INT8U ForeColor, INT8U * s)
{
	INT16 i,j,k,x,y,xx;
	INT8U qm;
	INT32U ulOffset;
	INT8 ywbuf[16],temp[2];
    
	for( i = 0; i < strlen((const char*)s); i++ )
	{
		if( (INT8U)*(s+i) >= 161 )
		{
			temp[0] = *(s + i);
			temp[1] = '\0';
			return;
		}
		else
		{
			qm = *(s+i);
			ulOffset = (INT32U)(qm) * 16;		//Here to be changed tomorrow
			for( j = 0; j < 16; j ++ )
			{
				ywbuf[j] = g_auc_Ascii8x16[ulOffset + j];
            }

            for( y = 0; y < 16; y++ )
            {
            	for( x = 0; x < 8; x++ ) 
               	{
                	k = x % 8;
			    	if( ywbuf[y]  & (0x80 >> k) )
			       	{
			       		xx = x0 + x + i*8;
			       		(LCD_PutPixel(xx, y + y0, (INT8U)ForeColor));
			       	}
			   	}
            }
		}
	}
}

/*********************************************************************************************
* name:		Lcd_DspAscII6x8()
* func:		display 6x8 ASCII character string
* para:		usX0,usY0 -- ASCII character string's start point coordinate
*			ForeColor -- appointed color value
*			pucChar   -- ASCII character string
* ret:		none
* modify:
* comment:
*********************************************************************************************/
void Lcd_DspAscII6x8(INT16U usX0, INT16U usY0,INT8U ForeColor, INT8U* pucChar)
{
	INT32U i,j;
	INT8U  ucTemp;

	while( *pucChar != 0 )
	{
		for( i=0; i < 8; i++ )
		{
  			ucTemp = g_auc_Ascii6x8[(*pucChar) * 8 + i];
  			for( j = 0; j < 8; j++ )
  			{
  				if( (ucTemp & (0x80 >> j)) != 0 )
  				{
  					LCD_PutPixel(usX0 + i, usY0 + 8 - j, (INT8U)ForeColor);
  				}
  			}
		}
		usX0 += XWIDTH;
		pucChar++;
	}
}


/*********************************************************************************************
* name:		ReverseLine()
* func:		Reverse display some lines 
* para:		ulHeight -- line's height
*			ulY -- line's Y-coordinate
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void ReverseLine(INT32U ulHeight, INT32U ulY)
{
	INT32U i, j, temp;
	
	for( i = 0; i < ulHeight; i++ )
	{
		for( j = 0; j < (SCR_XSIZE/4/2) ; j++ )
		{
			temp = *(INT32U*)(LCD_VIRTUAL_BUFFER + (ulY+i)*SCR_XSIZE/2 + j*4);
			temp ^= 0xFFFFFFFF;
			*(INT32U*)(LCD_VIRTUAL_BUFFER + (ulY+i)*SCR_XSIZE/2 + j*4) = temp;
		}
	}
}

/*********************************************************************************************
* name:		Zdma0Done()
* func:		LCD dma interrupt handle function
* para:		none
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
volatile static INT8U ucZdma0Done=1;	//When DMA is finish,ucZdma0Done is cleared to Zero
void Zdma0Done(void)
{
	rI_ISPC=BIT_ZDMA0;	    //clear pending
	ucZdma0Done=0;
}

/*********************************************************************************************
* name:		Lcd_Dma_Trans()
* func:		dma transport virtual LCD screen to LCD actual screen
* para:		none
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_Dma_Trans(void)
{
	INT8U err;
	
	ucZdma0Done=1;
	//#define LCD_VIRTUAL_BUFFER	(0xc400000)
	//#define LCD_ACTIVE_BUFFER	(LCD_VIRTUAL_BUFFER+(SCR_XSIZE*SCR_YSIZE/2))	//DMA ON
	//#define LCD_ACTIVE_BUFFER	LCD_VIRTUAL_BUFFER								//DMA OFF
	//#define LCD_BUF_SIZE		(SCR_XSIZE*SCR_YSIZE/2)
	//So the Lcd Buffer Low area is from LCD_VIRTUAL_BUFFER to (LCD_ACTIVE_BUFFER+(SCR_XSIZE*SCR_YSIZE/2))
	rNCACHBE1=(((unsigned)(LCD_ACTIVE_BUFFER)>>12) <<16 )|((unsigned)(LCD_VIRTUAL_BUFFER)>>12);
  	rZDISRC0=(DW<<30)|(1<<28)     |LCD_VIRTUAL_BUFFER; // inc
  	rZDIDES0=( 2<<30)|(1<<28)     |LCD_ACTIVE_BUFFER; // inc
    rZDICNT0=( 2<<28)|(1<<26)     |(3<<22)     |(0<<20)      |(LCD_BUF_SIZE);
        //           |            |            |             |            |---->0 = Disable DMA
        //           |            |            |             |------------>Int. whenever transferred
        //           |            |            |-------------------->Write time on the fly
        //           |            |---------------------------->Block(4-word) transfer mode
        //           |------------------------------------>whole service
	    //reEnable ZDMA transfer
  	rZDICNT0 |= (1<<20);		//after ES3
    rZDCON0=0x1; // start!!!  

	//Delay(500);
	/* Esperar con timeout para evitar bloqueo */
	{
		INT32U timeout = 100000;
		while(ucZdma0Done && timeout > 0)
		{
			timeout--;
		}
	}
}

/*********************************************************************************************
* name:		Lcd_Test()
* func:		LCD test function
* para:		none
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_Test(void)
{
	/* initial LCD controller */
	Lcd_Init();
	/* clear screen */
	Lcd_Clr();
	Lcd_Active_Clr();

	/* draw rectangle pattern */ 
    #ifdef Eng_v // english version
	Lcd_DspAscII8x16(10,0,DARKGRAY,"Embest S3CEV40 ");
	#else
//	Lcd_DspHz16(10,0,DARKGRAY,"Ӣ��������ʵ��������");
	#endif
	Lcd_DspAscII8x16(10,20,BLACK,"Codigo del puesto: ");
	Lcd_Draw_Box(10,40,310,230,14);
	Lcd_Draw_Box(20,45,300,225,13);
	Lcd_Draw_Box(30,50,290,220,12);
	Lcd_Draw_Box(40,55,280,215,11);
	Lcd_Draw_Box(50,60,270,210,10);
	Lcd_Draw_Box(60,65,260,205,9);
	Lcd_Draw_Box(70,70,250,200,8);
	Lcd_Draw_Box(80,75,240,195,7);
	Lcd_Draw_Box(90,80,230,190,6);
	Lcd_Draw_Box(100,85,220,185,5);
	Lcd_Draw_Box(110,90,210,180,4);
	Lcd_Draw_Box(120,95,200,175,3);
	Lcd_Draw_Box(130,100,190,170,2);
	BitmapView(125,135,Stru_Bitmap_gbMouse);
	Lcd_Dma_Trans();

}

/*********************************************************************************************
* name:		Sudoku_Pantalla_Inicial()
* func:		Muestra la pantalla inicial del Sudoku con instrucciones
* para:		none
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Sudoku_Pantalla_Inicial(void)
{
	/* Limpiar pantalla */
	Lcd_Clr();
	
	/* Título */
	Lcd_DspAscII8x16(85, 10, BLACK, (INT8U*)"SUDOKU 9x9");
	
	/* Instrucciones */
	Lcd_DspAscII6x8(20, 40, DARKGRAY, (INT8U*)"INSTRUCCIONES:");
	Lcd_DspAscII6x8(10, 55, BLACK, (INT8U*)"1. Boton Derecho: cambia valor");
	Lcd_DspAscII6x8(10, 70, BLACK, (INT8U*)"2. Boton Izquierdo: confirma");
	Lcd_DspAscII6x8(10, 85, BLACK, (INT8U*)"3. Introducir valor 0: borrar");
	Lcd_DspAscII6x8(10, 100, BLACK, (INT8U*)"4. Fila 0: terminar partida");
	
	/* Leyenda de símbolos en pantalla */
	Lcd_DspAscII6x8(20, 125, DARKGRAY, (INT8U*)"LEYENDA:");
	Lcd_DspAscII6x8(10, 140, BLACK, (INT8U*)"F = Fila    C = Columna");
	Lcd_DspAscII6x8(10, 155, BLACK, (INT8U*)"E = Error detectado");
	
	/* Mensaje para iniciar */
	Lcd_Draw_Box(40, 190, 280, 220, BLACK);
	Lcd_DspAscII8x16(60, 198, BLACK, (INT8U*)"Pulse un boton para jugar");
	
	/* Transferir a pantalla */
	Lcd_Dma_Trans();
}

/*********************************************************************************************
* name:		Sudoku_Dibujar_Numero_En_Celda()
* func:		Dibuja un número en una celda del tablero
* para:		fila, col - posición en el tablero (0-8)
*           numero - valor a dibujar (1-9)
*           es_pista - 1 si es pista original, 0 si es valor del usuario
*           tiene_error - 1 si la celda tiene error
* ret:		none
* modify:
* comment:	Las pistas se dibujan en DARKGRAY, los valores del usuario en BLACK
*           Las celdas con error se resaltan con borde grueso
*********************************************************************************************/
void Sudoku_Dibujar_Numero_En_Celda(INT16U fila, INT16U col, INT8U numero, INT8U es_pista, INT8U tiene_error)
{
	#define MARGEN_IZQ 20
	#define MARGEN_SUP 10
	#define TAM_CELDA 23
	
	INT16U tablero_inicio_x = MARGEN_IZQ + 10;
	INT16U tablero_inicio_y = MARGEN_SUP + 10;
	
	/* Calcular posición de la celda en píxeles */
	INT16U celda_x = tablero_inicio_x + col * TAM_CELDA;
	INT16U celda_y = tablero_inicio_y + fila * TAM_CELDA;
	
	/* Si tiene error, rellenar la celda con negro */
	if (tiene_error)
	{
		/* Rellenar interior de la celda con negro */
		LcdClrRect(celda_x + 2, celda_y + 2, celda_x + TAM_CELDA - 2, celda_y + TAM_CELDA - 2, BLACK);
	}
	else
	{
		/* Limpiar el interior de la celda (dejar márgenes para las líneas) */
		LcdClrRect(celda_x + 2, celda_y + 2, celda_x + TAM_CELDA - 2, celda_y + TAM_CELDA - 2, WHITE);
	}
	
	/* Si hay un número, dibujarlo */
	if (numero >= 1 && numero <= 9)
	{
		INT8U num_str[2];
		num_str[0] = '0' + numero;
		num_str[1] = '\0';
		
		/* Color del número */
		INT8U color;
		if (tiene_error)
		{
			/* Si hay error, número en blanco sobre fondo negro */
			color = WHITE;
		}
		else if (es_pista)
		{
			/* Pista: gris oscuro */
			color = DARKGRAY;
		}
		else
		{
			/* Valor del usuario: negro */
			color = BLACK;
		}
		
		/* Centrar el número en la celda */
		/* La fuente 8x16 ocupa 8 píxeles de ancho */
		INT16U texto_x = celda_x + (TAM_CELDA - 8) / 2;
		INT16U texto_y = celda_y + (TAM_CELDA - 16) / 2;
		
		Lcd_DspAscII8x16(texto_x, texto_y, color, num_str);
	}
}

/*********************************************************************************************
* name:		Sudoku_Dibujar_Candidatos_En_Celda()
* func:		Dibuja los candidatos en una celda vacía usando un grid 3x3
* para:		fila, col - posición en el tablero (0-8)
*           celda - valor de la celda con los bits de candidatos
* ret:		none
* modify:
* comment:	Cada número (1-9) se representa en un grid 3x3. Si es candidato, se 
*           dibuja un círculo pequeño en esa posición.
*           Layout del grid:
*           1 2 3
*           4 5 6
*           7 8 9
*********************************************************************************************/
void Sudoku_Dibujar_Candidatos_En_Celda(INT16U fila, INT16U col, CELDA celda)
{
	#define MARGEN_IZQ 20
	#define MARGEN_SUP 10
	#define TAM_CELDA 23
	
	INT16U tablero_inicio_x = MARGEN_IZQ + 10;
	INT16U tablero_inicio_y = MARGEN_SUP + 10;
	
	/* Calcular posición de la celda en píxeles */
	INT16U celda_x = tablero_inicio_x + col * TAM_CELDA;
	INT16U celda_y = tablero_inicio_y + fila * TAM_CELDA;
	
	/* Limpiar el interior de la celda */
	LcdClrRect(celda_x + 2, celda_y + 2, celda_x + TAM_CELDA - 2, celda_y + TAM_CELDA - 2, WHITE);
	
	/* Dibujar grid de candidatos */
	/* Cada subcelda del grid 3x3 tiene aproximadamente 7 píxeles */
	#define TAM_SUBCELDA 7
	
	INT8U numero;
	for (numero = 1; numero <= 9; numero++)
	{
		/* Verificar si este número es candidato */
		if (celda_es_candidato(celda, numero))
		{
			/* Calcular posición en el grid 3x3 */
			/* Grid: 1 2 3 / 4 5 6 / 7 8 9 */
			INT8U grid_fila = (numero - 1) / 3;  /* 0, 1, 2 */
			INT8U grid_col = (numero - 1) % 3;   /* 0, 1, 2 */
			
			/* Calcular centro de la subcelda */
			INT16U centro_x = celda_x + 3 + grid_col * TAM_SUBCELDA + TAM_SUBCELDA / 2;
			INT16U centro_y = celda_y + 3 + grid_fila * TAM_SUBCELDA + TAM_SUBCELDA / 2;
			
			/* Dibujar un círculo más grande (3x3 píxeles en forma circular) */
			LCD_PutPixel(centro_x, centro_y, BLACK);           /* Centro */
			LCD_PutPixel(centro_x - 1, centro_y, BLACK);       /* Izquierda */
			LCD_PutPixel(centro_x + 1, centro_y, BLACK);       /* Derecha */
			LCD_PutPixel(centro_x, centro_y - 1, BLACK);       /* Arriba */
			LCD_PutPixel(centro_x, centro_y + 1, BLACK);       /* Abajo */
			LCD_PutPixel(centro_x - 1, centro_y - 1, BLACK);   /* Diagonal sup-izq */
			LCD_PutPixel(centro_x + 1, centro_y - 1, BLACK);   /* Diagonal sup-der */
			LCD_PutPixel(centro_x - 1, centro_y + 1, BLACK);   /* Diagonal inf-izq */
			LCD_PutPixel(centro_x + 1, centro_y + 1, BLACK);   /* Diagonal inf-der */
		}
	}
}

/*********************************************************************************************
* name:		Sudoku_Actualizar_Tablero_Completo()
* func:		Actualiza todo el tablero mostrando valores y candidatos
* para:		cuadricula - puntero a la cuadrícula del juego
* ret:		none
* modify:
* comment:	Recorre toda la cuadrícula y dibuja cada celda según su contenido
*********************************************************************************************/
void Sudoku_Actualizar_Tablero_Completo(void* cuadricula_ptr)
{
	CELDA (*cuadricula)[NUM_COLUMNAS] = (CELDA (*)[NUM_COLUMNAS])cuadricula_ptr;
	INT16U fila, col;
	
	/* Recorrer todas las celdas del tablero */
	for (fila = 0; fila < NUM_FILAS; fila++)
	{
		for (col = 0; col < NUM_FILAS; col++)
		{
			CELDA celda_actual = cuadricula[fila][col];
			INT8U valor = celda_leer_valor(celda_actual);
			INT8U es_pista = celda_es_pista(celda_actual);
			INT8U tiene_error = (celda_actual & (1 << BIT_ERROR)) != 0;
			
			if (valor != 0)
			{
				/* Hay un valor: dibujarlo */
				Sudoku_Dibujar_Numero_En_Celda(fila, col, valor, es_pista, tiene_error);
			}
			else
			{
				/* Celda vacía: dibujar candidatos */
				Sudoku_Dibujar_Candidatos_En_Celda(fila, col, celda_actual);
			}
		}
	}
	
	/* Transferir todo a la pantalla */
	Lcd_Dma_Trans();
}

/*********************************************************************************************
* name:		Sudoku_Actualizar_Tiempo()
* func:		Actualiza solo el área del tiempo en pantalla sin redibujar todo el tablero
* para:		tiempo_us - tiempo transcurrido en microsegundos
* ret:		none
* modify:
* comment:	Convierte microsegundos a formato MM:SS y actualiza solo esa área
*********************************************************************************************/
void Sudoku_Actualizar_Tiempo(INT32U tiempo_us)
{
	#define MARGEN_IZQ 20
	#define MARGEN_SUP 10
	#define TAM_CELDA 23
	
	INT16U tablero_inicio_y = MARGEN_SUP + 10;
	INT16U tablero_tam = TAM_CELDA * 9;
	
	/* Convertir microsegundos a segundos */
	INT32U segundos_totales = tiempo_us / 1000000;
	
	/* Calcular minutos y segundos */
	INT16U minutos = segundos_totales / 60;
	INT16U segundos = segundos_totales % 60;
	
	/* Crear string en formato MM:SS */
	INT8U tiempo_str[15];
	tiempo_str[0] = 'T';
	tiempo_str[1] = 'i';
	tiempo_str[2] = 'e';
	tiempo_str[3] = 'm';
	tiempo_str[4] = 'p';
	tiempo_str[5] = 'o';
	tiempo_str[6] = ':';
	tiempo_str[7] = ' ';
	tiempo_str[8] = '0' + (minutos / 10);    /* Decenas de minutos */
	tiempo_str[9] = '0' + (minutos % 10);    /* Unidades de minutos */
	tiempo_str[10] = ':';
	tiempo_str[11] = '0' + (segundos / 10);  /* Decenas de segundos */
	tiempo_str[12] = '0' + (segundos % 10);  /* Unidades de segundos */
	tiempo_str[13] = '\0';
	
	/* Limpiar el área del tiempo (aproximadamente 90 píxeles de ancho) */
	LcdClrRect(MARGEN_IZQ, tablero_inicio_y + tablero_tam + 5, 
	           MARGEN_IZQ + 90, tablero_inicio_y + tablero_tam + 15, WHITE);
	
	/* Dibujar el nuevo tiempo */
	Lcd_DspAscII6x8(MARGEN_IZQ, tablero_inicio_y + tablero_tam + 5, BLACK, tiempo_str);
	
	/* Transferir solo esta actualización a pantalla */
	Lcd_Dma_Trans();
}

/*********************************************************************************************
* name:		Sudoku_Pantalla_Final()
* func:		Muestra la pantalla final cuando termina la partida
* para:		tiempo_us - tiempo total de la partida en microsegundos
* ret:		none
* modify:
* comment:	Muestra el tiempo final y un mensaje para reiniciar
*********************************************************************************************/
void Sudoku_Pantalla_Final(INT32U tiempo_us)
{
	/* Convertir microsegundos a segundos */
	INT32U segundos_totales = tiempo_us / 1000000;
	
	/* Calcular minutos y segundos */
	INT16U minutos = segundos_totales / 60;
	INT16U segundos = segundos_totales % 60;
	
	/* Crear string en formato MM:SS */
	INT8U tiempo_str[10];
	tiempo_str[0] = '0' + (minutos / 10);    /* Decenas de minutos */
	tiempo_str[1] = '0' + (minutos % 10);    /* Unidades de minutos */
	tiempo_str[2] = ':';
	tiempo_str[3] = '0' + (segundos / 10);  /* Decenas de segundos */
	tiempo_str[4] = '0' + (segundos % 10);  /* Unidades de segundos */
	tiempo_str[5] = '\0';
	
	/* Limpiar pantalla */
	Lcd_Clr();
	
	/* Título */
	Lcd_DspAscII8x16(80, 40, BLACK, (INT8U*)"PARTIDA TERMINADA");
	
	/* Mostrar tiempo final */
	Lcd_DspAscII8x16(90, 80, BLACK, (INT8U*)"Tiempo final:");
	Lcd_DspAscII8x16(130, 100, BLACK, tiempo_str);
	
	/* Mensaje para reiniciar */
	Lcd_DspAscII8x16(30, 158, BLACK, (INT8U*)"Pulse un boton para reiniciar");
	
	/* Transferir a pantalla */
	Lcd_Dma_Trans();
}
/*********************************************************************************************
* name:		Sudoku_Dibujar_Tablero()
* func:		Dibuja el tablero de Sudoku 9x9 con numeración
* para:		none
* ret:		none
* modify:
* comment:	Tamaño de celda: 23x23 píxeles, tablero total: 207x207
*********************************************************************************************/
void Sudoku_Dibujar_Tablero(void)
{
	INT16U i, j;
	INT8U fila_str[2] = "1";
	INT8U col_str[2] = "1";
	
	/* Constantes del tablero */
	#define MARGEN_IZQ 20
	#define MARGEN_SUP 10
	#define TAM_CELDA 23
	#define GROSOR_FINO 1
	#define GROSOR_GRUESO 2
	
	INT16U tablero_inicio_x = MARGEN_IZQ + 10;  /* Espacio para números de fila */
	INT16U tablero_inicio_y = MARGEN_SUP + 10;  /* Espacio para números de columna */
	INT16U tablero_tam = TAM_CELDA * 9;         /* 207 píxeles */
	
	/* Limpiar pantalla */
	Lcd_Clr();
	
	/* Dibujar números de columnas (1-9) en la parte superior */
	for (i = 0; i < 9; i++)
	{
		col_str[0] = '1' + i;
		Lcd_DspAscII6x8(tablero_inicio_x + i * TAM_CELDA + TAM_CELDA/2 - 3, 
		                MARGEN_SUP + 2, BLACK, col_str);
	}
	
	/* Dibujar números de filas (1-9) en el lado izquierdo */
	for (i = 0; i < 9; i++)
	{
		fila_str[0] = '1' + i;
		Lcd_DspAscII6x8(MARGEN_IZQ + 2, 
		                tablero_inicio_y + i * TAM_CELDA + TAM_CELDA/2 - 4, 
		                BLACK, fila_str);
	}
	
	/* Dibujar líneas horizontales */
	for (i = 0; i <= 9; i++)
	{
		INT16U grosor = (i % 3 == 0) ? GROSOR_GRUESO : GROSOR_FINO;
		INT16U y = tablero_inicio_y + i * TAM_CELDA;
		Lcd_Draw_HLine(tablero_inicio_x, tablero_inicio_x + tablero_tam, y, BLACK, grosor);
	}
	
	/* Dibujar líneas verticales */
	for (i = 0; i <= 9; i++)
	{
		INT16U grosor = (i % 3 == 0) ? GROSOR_GRUESO : GROSOR_FINO;
		INT16U x = tablero_inicio_x + i * TAM_CELDA;
		Lcd_Draw_VLine(tablero_inicio_y, tablero_inicio_y + tablero_tam, x, BLACK, grosor);
	}
	
	/* Mostrar tiempo en la parte inferior */
	Lcd_DspAscII6x8(MARGEN_IZQ, tablero_inicio_y + tablero_tam + 5, 
	                BLACK, "Tiempo: 00:00");
	
	/* Mensaje de ayuda */
	Lcd_DspAscII6x8(MARGEN_IZQ + 100, tablero_inicio_y + tablero_tam + 5, 
	                DARKGRAY, "Fila 0: Salir");
	
	/* Transferir a pantalla */
	Lcd_Dma_Trans();
}

/*********************************************************************************************
* name:		Sudoku_Procesar_Touch()
* func:		Detecta región 3x3 tocada y la muestra expandida
* para:		x, y - coordenadas del toque
* ret:		none
*********************************************************************************************/
void Sudoku_Procesar_Touch(int x, int y)
{
	#define MARGEN_IZQ 20
	#define MARGEN_SUP 10
	#define TAM_CELDA 23
	#define REGION_TAM 3
	
	/* Solo procesar toques si el juego está en progreso */
	extern int Sudoku_Juego_En_Progreso(void);
	if (!Sudoku_Juego_En_Progreso())
	{
		return;
	}
	
	INT16U tablero_inicio_x = MARGEN_IZQ + 10;
	INT16U tablero_inicio_y = MARGEN_SUP + 10;
	INT16U tablero_fin_x = tablero_inicio_x + TAM_CELDA * 9;
	INT16U tablero_fin_y = tablero_inicio_y + TAM_CELDA * 9;
	
	/* Verificar si el toque está dentro del tablero */
	if (x < tablero_inicio_x || x > tablero_fin_x || 
	    y < tablero_inicio_y || y > tablero_fin_y)
		return;
	
	/* Calcular celda tocada */
	int celda_col = (x - tablero_inicio_x) / TAM_CELDA;
	int celda_fila = (y - tablero_inicio_y) / TAM_CELDA;
	
	/* Calcular región 3x3 */
	int region_col = celda_col / REGION_TAM;
	int region_fila = celda_fila / REGION_TAM;
	
	/* Expandir región en pantalla completa */
	Sudoku_Mostrar_Region_Expandida(region_fila, region_col);
}

/*********************************************************************************************
* name:		Sudoku_Dibujar_Candidatos_Pequenos()
* func:		Dibuja los candidatos en pequeño dentro de una celda
*********************************************************************************************/
static void Sudoku_Dibujar_Candidatos_Pequenos(int x, int y, int tam, CELDA celda)
{
	int k;
	INT8U num_str[2];
	num_str[1] = '\0';
	
	/* Dibujar candidatos en 3x3 dentro de la celda */
	for (k = 1; k <= 9; k++)
	{
		if (celda_es_candidato(celda, k))
		{
			int cand_fila = (k - 1) / 3;
			int cand_col = (k - 1) % 3;
			int cand_x = x + 4 + cand_col * (tam / 3);
			int cand_y = y + 2 + cand_fila * (tam / 3);
			
			num_str[0] = '0' + k;
			Lcd_DspAscII6x8(cand_x, cand_y, DARKGRAY, num_str);
		}
	}
}

/*********************************************************************************************
* name:		Sudoku_Dibujar_Teclado_Virtual()
* func:		Dibuja teclado virtual con números 1-9
*********************************************************************************************/
static void Sudoku_Dibujar_Teclado_Virtual(void)
{
	int num;
	INT8U num_str[2];
	num_str[1] = '\0';
	
	#define TECLADO_X 200
	#define TECLADO_Y 30
	#define TECLA_TAM 35
	
	/* Dibujar 9 teclas en 3x3 */
	for (num = 1; num <= 9; num++)
	{
		int tecla_fila = (num - 1) / 3;
		int tecla_col = (num - 1) % 3;
		int tx = TECLADO_X + tecla_col * TECLA_TAM;
		int ty = TECLADO_Y + tecla_fila * TECLA_TAM;
		
		/* Dibujar borde de tecla */
		Lcd_Draw_Box(tx, ty, tx + TECLA_TAM, ty + TECLA_TAM, BLACK);
		
		/* Dibujar número centrado */
		num_str[0] = '0' + num;
		Lcd_DspAscII8x16(tx + TECLA_TAM/2 - 4, ty + TECLA_TAM/2 - 8, BLACK, num_str);
	}
	
	/* Botón borrar (debajo del teclado) - Más grande para facilitar el toque */
	Lcd_Draw_Box(TECLADO_X, TECLADO_Y + 3 * TECLA_TAM + 10, 
	             TECLADO_X + TECLA_TAM * 3, TECLADO_Y + 3 * TECLA_TAM + 10 + 40, BLACK);
	Lcd_DspAscII6x8(TECLADO_X + 30, TECLADO_Y + 3 * TECLA_TAM + 23, BLACK, 
	                (INT8U*)"BORRAR");
	
	/* Botón VOLVER (debajo del botón BORRAR) */
	Lcd_Draw_Box(TECLADO_X, TECLADO_Y + 3 * TECLA_TAM + 55, 
	             TECLADO_X + TECLA_TAM * 3, TECLADO_Y + 3 * TECLA_TAM + 55 + 40, LIGHTGRAY);
	Lcd_DspAscII6x8(TECLADO_X + 30, TECLADO_Y + 3 * TECLA_TAM + 68, BLACK, 
	                (INT8U*)"VOLVER");
}

/*********************************************************************************************
* name:		Sudoku_Redibujar_Region_Expandida()
* func:		Redibuja la región expandida completa
*********************************************************************************************/
void Sudoku_Redibujar_Region_Expandida(void)
{
	extern CELDA (*cuadricula)[NUM_COLUMNAS];
	int i, j;
	int fila_inicio = g_region_fila_actual * 3;
	int col_inicio = g_region_col_actual * 3;
	
	#define CELDA_GRANDE 60
	#define MARGEN_IZQ_EXP 10
	#define MARGEN_SUP_EXP 30
	
	/* Limpiar pantalla */
	Lcd_Clr();
	
	/* Dibujar cuadrícula 3x3 a la izquierda */
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			int x = MARGEN_IZQ_EXP + j * CELDA_GRANDE;
			int y = MARGEN_SUP_EXP + i * CELDA_GRANDE;
			int fila = fila_inicio + i;
			int col = col_inicio + j;
			
			/* Obtener celda */
			CELDA celda = cuadricula[fila][col];
			INT8U valor = celda_leer_valor(celda);
			int es_pista = celda_es_pista(celda);
			
			/* Color de fondo si está seleccionada */
			INT8U color_borde = BLACK;
			if (g_celda_seleccionada_i == i && g_celda_seleccionada_j == j)
			{
				/* Celda seleccionada - borde más grueso */
				Lcd_Draw_Box(x-1, y-1, x + CELDA_GRANDE+1, y + CELDA_GRANDE+1, BLACK);
			}
			
			/* Dibujar borde de celda */
			Lcd_Draw_Box(x, y, x + CELDA_GRANDE, y + CELDA_GRANDE, color_borde);
			
			if (valor != 0)
			{
				/* Dibujar número grande */
				INT8U num_str[2];
				num_str[0] = '0' + valor;
				num_str[1] = '\0';
				
				/* Número centrado */
				INT8U color_num = es_pista ? BLACK : DARKGRAY;
				Lcd_DspAscII8x16(x + CELDA_GRANDE/2 - 4, y + CELDA_GRANDE/2 - 8, 
				                 color_num, num_str);
			}
			else
			{
				/* Celda vacía - dibujar candidatos */
				Sudoku_Dibujar_Candidatos_Pequenos(x, y, CELDA_GRANDE, celda);
			}
		}
	}
	
	/* Dibujar teclado virtual a la derecha */
	Sudoku_Dibujar_Teclado_Virtual();
	
	/* Transferir a pantalla */
	Lcd_Dma_Trans();
}

/*********************************************************************************************
* name:		Sudoku_Mostrar_Region_Expandida()
* func:		Muestra una región 3x3 expandida con teclado virtual
* para:		region_fila, region_col - región 3x3 (0-2)
* ret:		none
*********************************************************************************************/
void Sudoku_Mostrar_Region_Expandida(int region_fila, int region_col)
{
	extern CELDA (*cuadricula)[NUM_COLUMNAS];
	
	/* Guardar estado de región expandida */
	g_region_expandida_activa = 1;
	g_region_fila_actual = region_fila;
	g_region_col_actual = region_col;
	g_celda_seleccionada_i = -1;
	g_celda_seleccionada_j = -1;
	
	/* Seleccionar automáticamente la primera celda no-pista */
	int i, j;
	int fila_inicio = region_fila * 3;
	int col_inicio = region_col * 3;
	
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			int fila_global = fila_inicio + i;
			int col_global = col_inicio + j;
			
			if (!celda_es_pista(cuadricula[fila_global][col_global]))
			{
				/* Encontramos una celda no-pista, seleccionarla */
				g_celda_seleccionada_i = i;
				g_celda_seleccionada_j = j;
				goto celda_encontrada;
			}
		}
	}
	
celda_encontrada:
	/* INTEGRACIÓN CON MÁQUINA DE ESTADOS */
	/* Al hacer zoom en una región, cambiar al estado INTRODUCIR_VALOR */
	if (Sudoku_Juego_En_Progreso())
	{
		Sudoku_Cambiar_Estado(4);  /* 4 = INTRODUCIR_VALOR (ver eventos.h) */
	}
	
	/* Dibujar interfaz */
	Sudoku_Redibujar_Region_Expandida();
	
	/* El bucle de interacción se maneja en main.c */
	/* Esta función solo inicializa la vista expandida */
}

/*********************************************************************************************
* name:		Sudoku_Esta_Region_Expandida_Activa()
* func:		Verifica si hay una región expandida activa
* ret:		1 si activa, 0 si no
*********************************************************************************************/
int Sudoku_Esta_Region_Expandida_Activa(void)
{
	return g_region_expandida_activa;
}

/*********************************************************************************************
* name:		Sudoku_Hay_Celda_Seleccionada()
* func:		Verifica si hay una celda seleccionada en modo zoom
* ret:		1 si hay celda seleccionada, 0 si no
*********************************************************************************************/
int Sudoku_Hay_Celda_Seleccionada(void)
{
	return (g_region_expandida_activa && g_celda_seleccionada_i >= 0 && g_celda_seleccionada_j >= 0);
}

/*********************************************************************************************
* name:		Sudoku_Obtener_Celda_Seleccionada()
* func:		Obtiene la posición de la celda seleccionada en el tablero completo
* para:		fila, col - punteros para devolver la posición (0-8)
* ret:		1 si hay celda seleccionada, 0 si no
*********************************************************************************************/
int Sudoku_Obtener_Celda_Seleccionada(int* fila, int* col)
{
	if (!Sudoku_Hay_Celda_Seleccionada())
		return 0;
	
	*fila = g_region_fila_actual * 3 + g_celda_seleccionada_i;
	*col = g_region_col_actual * 3 + g_celda_seleccionada_j;
	return 1;
}

/*********************************************************************************************
* name:		Sudoku_Cerrar_Region_Expandida()
* func:		Cierra la región expandida y vuelve al tablero completo
*********************************************************************************************/
void Sudoku_Cerrar_Region_Expandida(void)
{
	extern CELDA (*cuadricula)[NUM_COLUMNAS];
	
	g_region_expandida_activa = 0;
	g_celda_seleccionada_i = -1;
	g_celda_seleccionada_j = -1;
	
	/* INTEGRACIÓN CON MÁQUINA DE ESTADOS */
	/* Al volver del zoom, regresar al estado INTRODUCIR_FILA */
	if (Sudoku_Juego_En_Progreso())
	{
		Sudoku_Cambiar_Estado(1);  /* 1 = INTRODUCIR_FILA (ver eventos.h) */
	}
	
	/* Redibujar tablero completo */
	Sudoku_Dibujar_Tablero();
	Sudoku_Actualizar_Tablero_Completo(cuadricula);
}

/*********************************************************************************************
* name:		Sudoku_Procesar_Touch_Region_Expandida()
* func:		Procesa toques en la región expandida (selección de celda o número)
* para:		x, y - coordenadas del toque
* ret:		1 si se procesó el toque, 0 si no
*********************************************************************************************/
int Sudoku_Procesar_Touch_Region_Expandida(int x, int y)
{
	extern CELDA (*cuadricula)[NUM_COLUMNAS];
	
	if (!g_region_expandida_activa)
		return 0;
	
	#define CELDA_GRANDE 60
	#define MARGEN_IZQ_EXP 10
	#define MARGEN_SUP_EXP 30
	#define TECLADO_X 200
	#define TECLADO_Y 30
	#define TECLA_TAM 35
	
	/* Verificar si se tocó una celda de la cuadrícula */
	if (x >= MARGEN_IZQ_EXP && x < MARGEN_IZQ_EXP + 3 * CELDA_GRANDE &&
	    y >= MARGEN_SUP_EXP && y < MARGEN_SUP_EXP + 3 * CELDA_GRANDE)
	{
		int j = (x - MARGEN_IZQ_EXP) / CELDA_GRANDE;
		int i = (y - MARGEN_SUP_EXP) / CELDA_GRANDE;
		
		/* Calcular posición real en el tablero */
		int fila = g_region_fila_actual * 3 + i;
		int col = g_region_col_actual * 3 + j;
		
		/* Verificar que no sea una pista */
		CELDA celda = cuadricula[fila][col];
		if (!celda_es_pista(celda))
		{
			/* Seleccionar esta celda */
			g_celda_seleccionada_i = i;
			g_celda_seleccionada_j = j;
			Sudoku_Redibujar_Region_Expandida();
		}
		
		return 1;
	}
	
	/* Verificar si se tocó una tecla del teclado virtual */
	if (x >= TECLADO_X && x < TECLADO_X + 3 * TECLA_TAM &&
	    y >= TECLADO_Y && y < TECLADO_Y + 3 * TECLA_TAM)
	{
		/* Solo procesar si hay celda seleccionada */
		if (g_celda_seleccionada_i >= 0 && g_celda_seleccionada_j >= 0)
		{
			int tecla_col = (x - TECLADO_X) / TECLA_TAM;
			int tecla_fila = (y - TECLADO_Y) / TECLA_TAM;
			INT8U numero = tecla_fila * 3 + tecla_col + 1;
			
			/* Calcular posición real en el tablero */
			int fila = g_region_fila_actual * 3 + g_celda_seleccionada_i;
			int col = g_region_col_actual * 3 + g_celda_seleccionada_j;
			
			/* Verificar que no sea una pista */
			if (!celda_es_pista(cuadricula[fila][col]))
			{
				/* INTEGRACIÓN CON MÁQUINA DE ESTADOS */
				/* Llamar directamente al adaptador del touchscreen */
				Maquina_Procesar_Touch(EVENTO_INSERTAR_VALOR, fila, col, numero);
				
				/* Redibujar */
				Sudoku_Redibujar_Region_Expandida();
			}
		}
		
		return 1;
	}
	
	/* Verificar si se tocó el botón BORRAR */
	if (x >= TECLADO_X && x < TECLADO_X + 3 * TECLA_TAM &&
	    y >= TECLADO_Y + 3 * TECLA_TAM + 10 && y < TECLADO_Y + 3 * TECLA_TAM + 50)
	{
		/* Solo procesar si hay celda seleccionada */
		if (g_celda_seleccionada_i >= 0 && g_celda_seleccionada_j >= 0)
		{
			/* Calcular posición real en el tablero */
			int fila = g_region_fila_actual * 3 + g_celda_seleccionada_i;
			int col = g_region_col_actual * 3 + g_celda_seleccionada_j;
			
			/* Verificar que no sea una pista antes de borrar */
			if (!celda_es_pista(cuadricula[fila][col]))
			{
				/* INTEGRACIÓN CON MÁQUINA DE ESTADOS */
				/* Llamar directamente al adaptador del touchscreen */
				Maquina_Procesar_Touch(EVENTO_BORRAR_VALOR, fila, col, 0);
				
				/* Redibujar */
				Sudoku_Redibujar_Region_Expandida();
			}
		}
		
		return 1;
	}
	
	/* Verificar si se tocó el botón VOLVER */
	if (x >= TECLADO_X && x < TECLADO_X + 3 * TECLA_TAM &&
	    y >= TECLADO_Y + 3 * TECLA_TAM + 55 && y < TECLADO_Y + 3 * TECLA_TAM + 95)
	{
		/* Volver al tablero principal */
		Sudoku_Cerrar_Region_Expandida();
		return 1;
	}
	
	/* Si se tocó fuera de todas las áreas interactivas, ignorar el toque */
	return 0;
}