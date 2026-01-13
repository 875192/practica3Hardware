/*********************************************************************************************
* File��	tp.c
* Author:	embest	
* Desc��	LCD touch screen control function
* History:	
*********************************************************************************************/

/*--- include files ---*/
#include <string.h>
#include "tp.h"
#include "lcd.h"

void TSInt(void) __attribute__((interrupt("IRQ")));

/*===================================================================================
 * VARIABLES GLOBALES DE CALIBRACIÓN (Método 5 puntos)
 *===================================================================================*/
static int g_swap_xy = 0;           // 1 si los ejes están intercambiados
static long g_kx_fp;                // Factor de escala X en formato 16.16 fixed-point
static long g_ky_fp;                // Factor de escala Y en formato 16.16 fixed-point
static int g_ts_xc;                 // Centro X crudo del touch
static int g_ts_yc;                 // Centro Y crudo del touch
static int g_lcd_xc;                // Centro X del LCD
static int g_lcd_yc;                // Centro Y del LCD
static volatile int g_ts_ready = 0; // Flag para indicar que hay datos del touch
static int g_ts_raw_x;              // Última lectura X cruda
static int g_ts_raw_y;              // Última lectura Y cruda

void TS_Test(void)
{
		Lcd_TC();
		TS_init();
		Check_Sel();
			
//		Uart_Printf("\n Pixel: 320 X 240. Coordinate Rang in: (0,0) - (320,240)\n");
//		Uart_Printf("\nLCD TouchScreen Test Example(please touch LCD screen)\n");
}
/*--- function code ---*/
/*********************************************************************************************
* name:		TSInt
* func:		TouchScreen interrupt handler function
* para:		none
* ret:		none
* modify:
* comment:		
********************************************************************************************/
void TSInt(void)
{
    int   i;
    char fail = 0;
    ULONG tmp;
    ULONG Pt[6];

	// <X-Position Read>
	// TSPX(GPE4_Q4(+)) TSPY(GPE5_Q3(-)) TSMY(GPE6_Q2(+)) TSMX(GPE7_Q1(-))
    //       0               1                 1                0
	rPDATE=0x68;
	rADCCON=0x1<<2;			// AIN1
	
	DelayTime(1000);                // delay to set up the next channel
	for( i=0; i<5; i++ )
	{
		rADCCON |= 0x1;				// Start X-position A/D conversion
	    while( rADCCON & 0x1 );		// Check if Enable_start is low
    	while( !(rADCCON & 0x40) );	// Check ECFLG
	    Pt[i] = (0x3ff&rADCDAT);
	}
	// read X-position average value
	Pt[5] = (Pt[0]+Pt[1]+Pt[2]+Pt[3]+Pt[4])/5;
	
	tmp = Pt[5];	
	
    // <Y-Position Read>
	// TSPX(GPE4_Q4(-)) TSPY(GPE5_Q3(+)) TSMY(GPE6_Q2(-)) TSMX(GPE7_Q1(+))
    //       1               0                 0                1
	rPDATE=0x98;
	rADCCON=0x0<<2;		        	// AIN0
	
	DelayTime(1000);                // delay to set up the next channel
	for( i=0; i<5; i++ )
	{
    	rADCCON |= 0x1;             // Start Y-position conversion
	    while( rADCCON & 0x1 );     // Check if Enable_start is low
    	while( !(rADCCON & 0x40) ); // Check ECFLG
	    Pt[i] = (0x3ff&rADCDAT);
	}
	// read Y-position average value
	Pt[5] = (Pt[0]+Pt[1]+Pt[2]+Pt[3]+Pt[4])/5;
	
	// Reportar coordenadas crudas para calibración
	report_touch_data(tmp, Pt[5]);
     
	if(!(CheckTSP|(tmp < Xmin)|(tmp > Xmax)|(Pt[5] < Ymin)|(Pt[5] > Ymax)))   // Is valid value?
	  {
		tmp = 320*(tmp - Xmin)/(Xmax - Xmin);   // X - position
//		Uart_Printf("X-Posion[AIN1] is %04d   ", tmp);
			
		Pt[5] = 240*(Pt[5] - Xmin)/(Ymax - Ymin);
//		Uart_Printf("  Y-Posion[AIN0] is %04d\n", Pt[5]);
      }

    if(CheckTSP)
 	/*----------- check to ensure Xmax Ymax Xmin Ymin ------------*/
 	    DesignREC(tmp,Pt[5]);

	rPDATE = 0xb8;                  // should be enabled	
	DelayTime(3000);                // delay to set up the next channel	

    rI_ISPC = BIT_EINT2;            // clear pending_bit
}
			
/*********************************************************************************************
* name:		TS_init
* func:		initialize TouchScreen
* para:		none
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void TS_init(void)
{
    /* enable interrupt */
	rINTMOD=0x0;
	rINTCON=0x1;
    rI_ISPC |= BIT_EINT2;            // clear pending_bit
	
	// TSPX(GPE4_Q4(-)) TSPY(GPE5_Q3(-)) TSMY(GPE6_Q2(-)) TSMX(GPE7_Q1(+)) 
	//          1               1                0                 1
    rPUPE  = 0x0;	                 // Pull up
    rPDATE = 0xb8;                   // should be enabled	
    DelayTime(100); 
    
    rEXTINT |= 0x200;                // falling edge trigger
    pISR_EINT2=(unsigned *)TSInt;       // set interrupt handler
    
    rCLKCON = 0x7ff8;                // enable clock
    rADCPSR = 0x1;//0x4;             // A/D prescaler
    rINTMSK &= ~(BIT_EINT2);

    oneTouch = 0;
}

/*********************************************************************************************
* name:		TS_close
* func:		close TouchScreen
* para:		none
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void TS_close(void)
{
	/* Mask interrupt */
	rINTMSK |=BIT_GLOBAL|BIT_EINT2;
	pISR_EINT2 = (int)NULL;
}

void Lcd_TC(void)
{
	/* initial LCD controller */
	Lcd_Init();
	/* clear screen */
	Lcd_Clr();
	Lcd_Active_Clr();
	/* draw rectangle pattern */ 
	Lcd_Draw_Box(0,0,80,60,15);
	Lcd_Draw_Box(80,0,160,60,15);
	Lcd_Draw_Box(160,0,240,60,15);
	Lcd_Draw_Box(240,0,320,60,15);
	Lcd_Draw_Box(0,60,80,120,15);
	Lcd_Draw_Box(80,60,160,120,15);
	Lcd_Draw_Box(160,60,240,120,15);
	Lcd_Draw_Box(240,60,320,120,15);
	Lcd_Draw_Box(0,120,80,180,15);
	Lcd_Draw_Box(80,120,160,180,15);
	Lcd_Draw_Box(160,120,240,180,15);
	Lcd_Draw_Box(240,120,320,180,15);
	Lcd_Draw_Box(0,180,80,240,15);
	Lcd_Draw_Box(80,180,160,240,15);
	Lcd_Draw_Box(160,180,240,240,15);
	Lcd_Draw_Box(240,180,320,240,15);
	/* output ASCII symbol */
	Lcd_DspAscII6x8(37,26,BLACK,"0");
	Lcd_DspAscII6x8(117,26,BLACK,"1");
	Lcd_DspAscII6x8(197,26,BLACK,"2");
	Lcd_DspAscII6x8(277,26,BLACK,"3");
	Lcd_DspAscII6x8(37,86,BLACK,"4");
	Lcd_DspAscII6x8(117,86,BLACK,"5");
	Lcd_DspAscII6x8(197,86,BLACK,"6");
	Lcd_DspAscII6x8(277,86,BLACK,"7");
	Lcd_DspAscII6x8(37,146,BLACK,"8");
	Lcd_DspAscII6x8(117,146,BLACK,"9");
	Lcd_DspAscII6x8(197,146,BLACK,"A");
	Lcd_DspAscII6x8(277,146,BLACK,"B");
	Lcd_DspAscII6x8(37,206,BLACK,"C");
	Lcd_DspAscII6x8(117,206,BLACK,"D");
	Lcd_DspAscII6x8(197,206,BLACK,"E");
	Lcd_DspAscII6x8(277,206,BLACK,"F");
	Lcd_Dma_Trans();
	Delay(100);
}

/*********************************************************************************************
* name:		DesignREC
* func:		confirm the coordinate rang
* para:		none
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void DesignREC(ULONG tx, ULONG ty)
{
    int tm;
    
//  Uart_Printf("\n\r User touch coordinate(X,Y) is :");
//	Uart_Printf("(%04d",tx);	
//	Uart_Printf(",%04d)\n",ty);    
    if(oneTouch == 0)
     {
       Vx = tx;                   // Vx as Xmax
       Vy = ty;                   // Vy as Ymax
       oneTouch = 1;       		           
    }else{
    
    if(Vx < tx )
     {
       tm = tx; tx = Vx; Vx = tm; // tx as Xmin
     }
    if(Vy < ty )
     { 
       tm = ty; ty = Vy; Vy = tm; // ty as Ymin
     }
     
    Xmax = Vx;    Xmin = tx;
    Ymax = Vy;    Ymin = ty;
    
    oneTouch = 0;
	CheckTSP = 0;// has checked
	
	}// end if(oneTouch == 0)

}

void Check_Sel(void)
{
	char yn;
    
	do{
	  
	  rINTMSK |=BIT_GLOBAL|BIT_EINT2;
//    Uart_Printf("\n\r Touch Screen coordinate Rang in:\n");
//    Uart_Printf("   (Xmin,Ymin) is :(%04d,%04d)\n",Xmin,Ymin);	
//	  Uart_Printf("   (Xmax,Ymax) is :(%04d,%04d)\n",Xmax,Ymax);	
//	  Uart_Printf("\n  To use current settings. Press N/n key. ");
//	  Uart_Printf("\n\n\r  Want to Set Again(Y/N)? ");	        	      
//	  yn = Uart_Getch();
      rI_ISPC = BIT_EINT2;              // clear pending_bit
      rINTMSK =~(BIT_GLOBAL|BIT_EINT2);
//	  if((yn == 0x59)|(yn == 0x79)|(yn == 0x4E)|(yn == 0x6E)) Uart_SendByte(yn);	  
	  if((yn == 0x59)|(yn == 0x79))
	    {
//	      Uart_Printf("\n\n Touch TSP's Cornor to ensure Xmax,Ymax,Xmax,Xmin");	      

         //Init X Y rectangle
          Xmax = 750;    Xmin = 200;
          Ymax = 620;    Ymin = 120;

	      oneTouch = 0;
	      CheckTSP = 1; // mask to check
	      while(CheckTSP);
	      
	    }else break;	    
	  
	}while(1);
	
}

/*===================================================================================
 * FUNCIONES DE CALIBRACIÓN DE 5 PUNTOS
 *===================================================================================*/

/*********************************************************************************************
* name:     clamp
* func:     Limita un valor entre mínimo y máximo
*********************************************************************************************/
static int clamp(int val, int min, int max)
{
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

/*********************************************************************************************
* name:     draw_cross
* func:     Dibuja una cruz en las coordenadas especificadas
*********************************************************************************************/
static void draw_cross(int x, int y)
{
    int i;
    // Línea horizontal
    for (i = -10; i <= 10; i++)
    {
        if (x + i >= 0 && x + i < SCR_XSIZE)
            (LCD_PutPixel(x + i, y, 0xf));
    }
    // Línea vertical
    for (i = -10; i <= 10; i++)
    {
        if (y + i >= 0 && y + i < SCR_YSIZE)
            (LCD_PutPixel(x, y + i, 0xf));
    }
}

/*********************************************************************************************
* name:     report_touch_data
* func:     Llamada desde TSInt() para reportar coordenadas crudas
*********************************************************************************************/
void report_touch_data(int x, int y)
{
    if (g_ts_ready == 0)
    {
        g_ts_raw_x = x;
        g_ts_raw_y = y;
        g_ts_ready = 1;
    }
}

/*********************************************************************************************
* name:     ts_wait_for_touch
* func:     Espera hasta detectar un toque
*********************************************************************************************/
static void ts_wait_for_touch(void)
{
    g_ts_ready = 0;
    while (g_ts_ready == 0)
    {
        // Esperar interrupción
    }
}

/*********************************************************************************************
* name:     ts_read_raw
* func:     Lee coordenadas crudas del ADC (bloqueante)
*********************************************************************************************/
static void ts_read_raw(int *xr, int *yr)
{
    ts_wait_for_touch();
    *xr = g_ts_raw_x;
    *yr = g_ts_raw_y;
}

/*********************************************************************************************
* name:     get_cal_point
* func:     Muestra cruz, espera 5 toques y captura promedio de coordenadas crudas
*********************************************************************************************/
static void get_cal_point(int lcd_x, int lcd_y, int *ts_x, int *ts_y)
{
    int i;
    int sum_x = 0, sum_y = 0;
    int temp_x, temp_y;
    
    draw_cross(lcd_x, lcd_y);
    Lcd_Dma_Trans();
    Delay(50);
    
    // Tomar 5 muestras y promediar
    for (i = 0; i < 5; i++)
    {
        ts_read_raw(&temp_x, &temp_y);
        sum_x += temp_x;
        sum_y += temp_y;
        Delay(20);  // Pequeña pausa entre muestras
    }
    
    // Calcular promedio
    *ts_x = sum_x / 5;
    *ts_y = sum_y / 5;
    
    Delay(30);
    draw_cross(lcd_x, lcd_y);
    Lcd_Dma_Trans();
}

/*********************************************************************************************
* name:     ts_calibrate_5pt
* func:     Calibración de 5 puntos del touchscreen
* para:     XRES - resolución horizontal, YRES - vertical, M - margen
* comment:  Implementa método con detección swap XY y escalas con signo (fixed-point 16.16)
*********************************************************************************************/
void ts_calibrate_5pt(int XRES, int YRES, int M)
{
    int A_lcd_x, A_lcd_y, A_ts_x, A_ts_y;
    int B_lcd_x, B_lcd_y, B_ts_x, B_ts_y;
    int C_lcd_x, C_lcd_y, C_ts_x, C_ts_y;
    int D_lcd_x, D_lcd_y, D_ts_x, D_ts_y;
    int E_lcd_x, E_lcd_y, E_ts_x, E_ts_y;
    
    int dx, dy;
    int lcd_s, lcd_d;
    int ts_s1, ts_s2, ts_d1, ts_d2;
    long long temp;
    
    Lcd_Clr();
    Lcd_Active_Clr();
    Lcd_DspAscII8x16(60, 110, BLACK, (unsigned char *)"Calibracion 5pts");
    Lcd_Dma_Trans();
    Delay(100);
    
    Lcd_Clr();
    Lcd_Dma_Trans();
    
    // Definir posiciones LCD de los 5 puntos
    A_lcd_x = M;            A_lcd_y = M;
    B_lcd_x = XRES - M;     B_lcd_y = M;
    C_lcd_x = XRES - M;     C_lcd_y = YRES - M;
    D_lcd_x = M;            D_lcd_y = YRES - M;
    E_lcd_x = XRES / 2;     E_lcd_y = YRES / 2;
    
    // Capturar punto A
    Lcd_DspAscII6x8(10, 10, BLACK, (unsigned char *)"Toca punto A");
    Lcd_Dma_Trans();
    get_cal_point(A_lcd_x, A_lcd_y, &A_ts_x, &A_ts_y);
    
    // Capturar punto B
    Lcd_Clr();
    Lcd_DspAscII6x8(10, 10, BLACK, (unsigned char *)"Toca punto B");
    Lcd_Dma_Trans();
    get_cal_point(B_lcd_x, B_lcd_y, &B_ts_x, &B_ts_y);
    
    // Capturar punto C
    Lcd_Clr();
    Lcd_DspAscII6x8(10, 10, BLACK, (unsigned char *)"Toca punto C");
    Lcd_Dma_Trans();
    get_cal_point(C_lcd_x, C_lcd_y, &C_ts_x, &C_ts_y);
    
    // Capturar punto D
    Lcd_Clr();
    Lcd_DspAscII6x8(10, 10, BLACK, (unsigned char *)"Toca punto D");
    Lcd_Dma_Trans();
    get_cal_point(D_lcd_x, D_lcd_y, &D_ts_x, &D_ts_y);
    
    // Capturar punto E (centro)
    Lcd_Clr();
    Lcd_DspAscII6x8(10, 10, BLACK, (unsigned char *)"Toca centro E");
    Lcd_Dma_Trans();
    get_cal_point(E_lcd_x, E_lcd_y, &E_ts_x, &E_ts_y);
    
    // DETECCIÓN DE SWAP XY
    dx = B_ts_x - A_ts_x;
    dy = B_ts_y - A_ts_y;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    
    if (dx > dy)
    {
        g_swap_xy = 0;
    }
    else
    {
        g_swap_xy = 1;
        int tmp;
        #define SWAP(a, b) { tmp = a; a = b; b = tmp; }
        SWAP(A_ts_x, A_ts_y);
        SWAP(B_ts_x, B_ts_y);
        SWAP(C_ts_x, C_ts_y);
        SWAP(D_ts_x, D_ts_y);
        SWAP(E_ts_x, E_ts_y);
    }
    
    // CÁLCULO DEL CENTRO - usar promedio de los 4 puntos para mayor precisión
    g_ts_xc = (A_ts_x + B_ts_x + C_ts_x + D_ts_x) / 4;
    g_ts_yc = (A_ts_y + B_ts_y + C_ts_y + D_ts_y) / 4;
    g_lcd_xc = XRES / 2;
    g_lcd_yc = YRES / 2;
    
    // CÁLCULO DE kx y ky (CON SIGNO - fixed-point 16.16)
    lcd_s = XRES - 2 * M;
    lcd_d = YRES - 2 * M;
    
    ts_s1 = B_ts_x - A_ts_x;
    ts_s2 = C_ts_x - D_ts_x;
    ts_d1 = D_ts_y - A_ts_y;
    ts_d2 = C_ts_y - B_ts_y;
    
    // Usar promedios para mayor precisión
    temp = (long long)lcd_s << 17;  // Multiplicar por 2 usando shift (<<17 = <<16 * 2)
    g_kx_fp = (long)(temp / (ts_s1 + ts_s2));
    
    temp = (long long)lcd_d << 17;  // Multiplicar por 2 usando shift
    g_ky_fp = (long)(temp / (ts_d1 + ts_d2));
    
    // Mostrar resultados de calibración
    Lcd_Clr();
    
    Lcd_DspAscII6x8(10, 10, BLACK, (unsigned char *)"=== CALIBRACION OK ===");
    
    if (g_swap_xy)
        Lcd_DspAscII6x8(10, 30, BLACK, (unsigned char *)"Swap XY: SI");
    else
        Lcd_DspAscII6x8(10, 30, BLACK, (unsigned char *)"Swap XY: NO");
    
    Lcd_DspAscII6x8(10, 50, BLACK, (unsigned char *)"Puntos capturados OK");
    Lcd_DspAscII6x8(10, 70, BLACK, (unsigned char *)"Factores calculados");
    
    // Mostrar signo de las escalas
    if (g_kx_fp > 0)
        Lcd_DspAscII6x8(10, 90, BLACK, (unsigned char *)"kx: POSITIVO");
    else
        Lcd_DspAscII6x8(10, 90, BLACK, (unsigned char *)"kx: NEGATIVO");
        
    if (g_ky_fp > 0)
        Lcd_DspAscII6x8(10, 110, BLACK, (unsigned char *)"ky: POSITIVO");
    else
        Lcd_DspAscII6x8(10, 110, BLACK, (unsigned char *)"ky: NEGATIVO");
    
    Lcd_DspAscII6x8(10, 140, BLACK, (unsigned char *)"[Toca para continuar]");
    
    Lcd_Dma_Trans();
    
    // Esperar toque para continuar
    ts_wait_for_touch();
    Delay(50);
    
    Lcd_Clr();
    Lcd_Dma_Trans();
}

/*********************************************************************************************
* name:     ts_read_calibrated
* func:     Lee coordenadas calibradas del touchscreen
* ret:      0 si éxito, -1 si no hay toque
*********************************************************************************************/
int ts_read_calibrated(int *x, int *y)
{
    int xr, yr;
    long long temp_x, temp_y;
    
    if (g_ts_ready == 0)
        return -1;
    
    xr = g_ts_raw_x;
    yr = g_ts_raw_y;
    g_ts_ready = 0;
    
    // Aplicar swap si necesario
    if (g_swap_xy)
    {
        int tmp = xr;
        xr = yr;
        yr = tmp;
    }
    
    // CONVERSIÓN con fixed-point 16.16
    temp_x = (long long)g_kx_fp * (xr - g_ts_xc);
    *x = (int)(temp_x >> 16) + g_lcd_xc;
    
    temp_y = (long long)g_ky_fp * (yr - g_ts_yc);
    *y = (int)(temp_y >> 16) + g_lcd_yc;
    
    // Clampear
    *x = clamp(*x, 0, SCR_XSIZE - 1);
    *y = clamp(*y, 0, SCR_YSIZE - 1);
    
    return 0;
}

/*********************************************************************************************
* name:     ts_test_calibracion
* func:     Función de test completa: calibra y permite dibujar
*********************************************************************************************/
void ts_test_calibracion(void)
{
    int x, y;
    
    Lcd_Init();
    TS_init();
    
    // Calibrar con margen de 50 píxeles
    ts_calibrate_5pt(SCR_XSIZE, SCR_YSIZE, 50);
    
    Lcd_Clr();
    Lcd_Active_Clr();
    Lcd_DspAscII8x16(80, 110, BLACK, (unsigned char *)"Dibuja!");
    Lcd_Dma_Trans();
    Delay(100);
    
    Lcd_Clr();
    Lcd_Dma_Trans();
    
    // Loop de dibujo
    while (1)
    {
        if (ts_read_calibrated(&x, &y) == 0)
        {
            (LCD_PutPixel(x, y, 0xf));
            
            static int count = 0;
            if (++count >= 5)
            {
                Lcd_Dma_Trans();
                count = 0;
            }
        }
    }
}

/*********************************************************************************************
* name:     ts_test_numeros
* func:     Test con dos números: muestra coordenadas al tocar
*********************************************************************************************/
void ts_test_numeros(void)
{
    int x, y;
    int num1_x = 60, num1_y = 100;   // Posición número 1 (izquierda)
    int num2_x = 220, num2_y = 100;  // Posición número 2 (derecha)
    
    Lcd_Init();
    TS_init();
    
    // Calibrar con margen de 30 píxeles (más cerca de los bordes)
    ts_calibrate_5pt(SCR_XSIZE, SCR_YSIZE, 30);
    
    // Mostrar los dos números
    Lcd_Clr();
    Lcd_DspAscII8x16(num1_x, num1_y, BLACK, (unsigned char *)"1");
    Lcd_DspAscII8x16(num2_x, num2_y, BLACK, (unsigned char *)"2");
    Lcd_DspAscII6x8(60, 20, BLACK, (unsigned char *)"Toca un numero");
    Lcd_Dma_Trans();
    
    // Loop de detección de toques
    while (1)
    {
        if (ts_read_calibrated(&x, &y) == 0)
        {
            // Limpiar zona de información
            int i, j;
            for (i = 0; i < 320; i++)
                for (j = 150; j < 200; j++)
                    (LCD_PutPixel(i, j, 0x0));
            
            // Mostrar coordenadas
            Lcd_DspAscII6x8(10, 160, BLACK, (unsigned char *)"Toque detectado:");
            
            // Mostrar X (solo texto fijo, sin sprintf)
            Lcd_DspAscII6x8(10, 175, BLACK, (unsigned char *)"Coordenadas guardadas");
            
            // Dibujar punto donde se tocó
            int px, py;
            for (px = x-3; px <= x+3; px++)
                for (py = y-3; py <= y+3; py++)
                    if (px >= 0 && px < SCR_XSIZE && py >= 0 && py < SCR_YSIZE)
                        (LCD_PutPixel(px, py, 0xf));
            
            // Detectar qué número se tocó (área más amplia para test)
            if (x >= (num1_x-30) && x <= (num1_x+30) && y >= (num1_y-30) && y <= (num1_y+30))
            {
                Lcd_DspAscII8x16(80, 190, BLACK, (unsigned char *)"NUMERO 1");
            }
            else if (x >= (num2_x-30) && x <= (num2_x+30) && y >= (num2_y-30) && y <= (num2_y+30))
            {
                Lcd_DspAscII8x16(80, 190, BLACK, (unsigned char *)"NUMERO 2");
            }
            else
            {
                Lcd_DspAscII6x8(70, 195, BLACK, (unsigned char *)"Fuera de zona");
            }
            
            Lcd_Dma_Trans();
            Delay(50);
        }
    }
}