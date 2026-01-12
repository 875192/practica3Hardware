/*********************************************************************************************
* Fichero:	lcd.c
* Autor:	
* Descrip:	funciones de visualizaci�n y control LCD
* Version:	<P6-ARM>
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "def.h"
#include "44b.h"
#include "44blib.h"
#include "lcd.h"
#include "Bmp.h"
#include "celda.h"
#include "sudoku_2025.h"

/*--- definici�n de macros ---*/
#define DMA_Byte  (0)
#define DMA_HW    (1)
#define DMA_Word  (2)
#define DW 		  DMA_Byte		//configura  ZDMA0 como media palabras
	
/*--- variables externas ---*/
extern INT8U g_auc_Ascii8x16[];
extern INT8U g_auc_Ascii6x8[];
extern STRU_BITMAP Stru_Bitmap_gbMouse;

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
	Lcd_DspAscII8x16(85, 10, BLACK, "SUDOKU 9x9");
	
	/* Instrucciones */
	Lcd_DspAscII6x8(20, 40, DARKGRAY, "INSTRUCCIONES:");
	Lcd_DspAscII6x8(10, 55, BLACK, "1. Boton Derecho: cambia valor");
	Lcd_DspAscII6x8(10, 70, BLACK, "2. Boton Izquierdo: confirma");
	Lcd_DspAscII6x8(10, 85, BLACK, "3. Introducir valor 0: borrar");
	Lcd_DspAscII6x8(10, 100, BLACK, "4. Fila 0: terminar partida");
	
	/* Leyenda de símbolos en pantalla */
	Lcd_DspAscII6x8(20, 125, DARKGRAY, "LEYENDA:");
	Lcd_DspAscII6x8(10, 140, BLACK, "F = Fila    C = Columna");
	Lcd_DspAscII6x8(10, 155, BLACK, "E = Error detectado");
	
	/* Mensaje para iniciar */
	Lcd_Draw_Box(40, 190, 280, 220, BLACK);
	Lcd_DspAscII8x16(60, 198, BLACK, "Pulse un boton para jugar");
	
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
	Lcd_DspAscII8x16(80, 40, BLACK, "PARTIDA TERMINADA");
	
	/* Mostrar tiempo final */
	Lcd_DspAscII8x16(90, 80, BLACK, "Tiempo final:");
	Lcd_DspAscII8x16(130, 100, BLACK, tiempo_str);
	
	/* Mensaje para reiniciar */
	Lcd_DspAscII8x16(30, 158, BLACK, "Pulse un boton para reiniciar");
	
	/* Transferir a pantalla */
	Lcd_Dma_Trans();
}

void Sudoku_Dibujar_Teclado_Tactil(void)
{
	#define TECLADO_X 245
	#define TECLADO_Y 20
	#define TECLADO_W 70
	#define TECLADO_H 207
	#define TECLADO_COLS 3
	#define TECLADO_ROWS 4
	#define TECLADO_CELDA_W (TECLADO_W / TECLADO_COLS)
	#define TECLADO_CELDA_H (TECLADO_H / TECLADO_ROWS)

	INT16U fila;
	INT16U col;
	INT8U num_str[2] = "0";

	Lcd_Draw_Box(TECLADO_X, TECLADO_Y, TECLADO_X + TECLADO_W, TECLADO_Y + TECLADO_H, BLACK);

	for (col = 1; col < TECLADO_COLS; col++)
	{
		INT16U x = TECLADO_X + col * TECLADO_CELDA_W;
		Lcd_Draw_VLine(TECLADO_Y, TECLADO_Y + TECLADO_H, x, BLACK, 1);
	}

	for (fila = 1; fila < TECLADO_ROWS; fila++)
	{
		INT16U y = TECLADO_Y + fila * TECLADO_CELDA_H;
		Lcd_Draw_HLine(TECLADO_X, TECLADO_X + TECLADO_W, y, BLACK, 1);
	}

	for (fila = 0; fila < 3; fila++)
	{
		for (col = 0; col < 3; col++)
		{
			INT16U valor = fila * 3 + col + 1;
			num_str[0] = '0' + valor;
			Lcd_DspAscII6x8(TECLADO_X + col * TECLADO_CELDA_W + TECLADO_CELDA_W / 2 - 3,
			                TECLADO_Y + fila * TECLADO_CELDA_H + TECLADO_CELDA_H / 2 - 4,
			                BLACK, num_str);
		}
	}

	Lcd_DspAscII6x8(TECLADO_X + TECLADO_CELDA_W / 2 - 3,
	                TECLADO_Y + 3 * TECLADO_CELDA_H + TECLADO_CELDA_H / 2 - 4,
	                BLACK, "0");

	Lcd_DspAscII6x8(TECLADO_X + TECLADO_CELDA_W + (TECLADO_CELDA_W - 18) / 2,
	                TECLADO_Y + 3 * TECLADO_CELDA_H + TECLADO_CELDA_H / 2 - 4,
	                BLACK, "FIN");
}

void Sudoku_Resaltar_Celda(INT16U fila, INT16U col, INT8U color)
{
	#define MARGEN_IZQ 20
	#define MARGEN_SUP 10
	#define TAM_CELDA 23

	INT16U tablero_inicio_x = MARGEN_IZQ + 10;
	INT16U tablero_inicio_y = MARGEN_SUP + 10;
	INT16U celda_x = tablero_inicio_x + col * TAM_CELDA;
	INT16U celda_y = tablero_inicio_y + fila * TAM_CELDA;

	Lcd_Draw_Box(celda_x + 1, celda_y + 1, celda_x + TAM_CELDA - 1, celda_y + TAM_CELDA - 1, color);
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

	Sudoku_Dibujar_Teclado_Tactil();
	
	/* Transferir a pantalla */
	Lcd_Dma_Trans();
}
