/*********************************************************************************************
* File	main.c
* Author: embest
* Description: main entry
* History:	
* - Código original de touchscreen y LCD
* - Código integrado de Sudoku desde main_sudoku.c
*********************************************************************************************/

/*--- include files ---*/
#include "44blib.h"
#include "44b.h"
#include "tp.h"

/* Includes del Sudoku */
#include "8led.h"
#include "button.h"
#include "led.h"
#include "timer1.h"
#include "timer2.h"
#include "timer3.h"
#include "cola.h"
#include "sudoku_2025.h"
#include "lcd.h"

/*--- variables globales ---*/
char yn;

/* Variables globales del Sudoku */
#include "tableros.h"

/* Apuntar a una de las cuadrículas disponibles */
CELDA (*cuadricula)[NUM_COLUMNAS] = cuadricula_ARM_ARM; // CAMBIAR SEGUN LA VERSIÓN A PROBAR
int celdas_vacias = 0;

/*--- function declare ---*/
void Main(void);

/*--- extern function ---*/
extern void Lcd_Test();

/*--- function code ---*/
/*********************************************************************************************
* name:		Main
* func:		c code entry - VERSIÓN INTEGRADA: Touchscreen + Sudoku
* para:		none
* ret:		none
* modify:
* comment:	Descomenta el bloque que quieras ejecutar (TOUCHSCREEN o SUDOKU)
*********************************************************************************************/
void Main(void)
{
    sys_init();        /* Initial 44B0X's Interrupt, Port and UART */
    //_Link();           /* Print Misc info */
    
    /* ======================================================================
     * OPCIÓN 1: CÓDIGO TOUCHSCREEN Y LCD (ORIGINAL)
     * Descomenta este bloque para ejecutar el código del touchscreen
     * ====================================================================== */
    /*
    Lcd_Test();
    TS_Test();
    
    while(1)
    {
       //yn = Uart_Getch();
       
       //if(yn == 0x52) TS_Test();// R to reset the XY REC
       //else break;
    }
    
    TS_close();
    */
    
    /* ======================================================================
     * OPCIÓN 2: CÓDIGO SUDOKU (desde main_sudoku.c)
     * Este bloque está activo por defecto
     * ====================================================================== */
    
    /* Variables para observar la cola durante la depuración */
    ColaDebug* p_cola;              // Puntero a la estructura completa de la cola
    
    /* Inicializa controladores del Sudoku */
    timer2_init();      // Inicializacion del timer2 para medicion de tiempo
    timer1_init();      // Inicializacion del timer1 para latido (heartbeat) - Paso 6
    cola_init();        // Inicializacion de la cola de depuracion (Paso 4)
    Eint4567_init();    // inicializamos los pulsadores. Cada vez que se pulse se verá reflejado en el 8led
    D8Led_init();       // inicializamos el 8led

    /* Inicializar LCD y mostrar pantalla inicial */
    Lcd_Init();
    Sudoku_Pantalla_Inicial();

    /* Valor inicial de los leds */
    leds_off();
    
    /* Apuntar a la cola para poder observarla en el depurador */
    p_cola = cola_global;
    
    /* Variable para controlar actualización del tiempo */
    unsigned int tiempo_anterior = 0;
    unsigned int tiempo_actual = 0;
    
    /* Bucle principal */
    while (1)
    {
        /* El latido (LED2 parpadeando) se gestiona automáticamente por timer1 */
        /* El programa está vivo mientras el LED2 parpadee a 6 Hz */
        
        /* Actualizar el tiempo en pantalla cada segundo */
        tiempo_actual = timer2_count();
        
        /* Actualizar cada 1 segundo (1000000 microsegundos) */
        if ((tiempo_actual - tiempo_anterior) >= 1000000)
        {
            /* Solo actualizar si la partida no ha terminado */
            if (!Sudoku_Partida_Terminada())
            {
                Sudoku_Actualizar_Tiempo(tiempo_actual);
            }
            tiempo_anterior = tiempo_actual;
        }
    }
}
