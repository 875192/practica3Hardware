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

/*--- constantes para el teclado táctil ---*/
#define TABLERO_X 30
#define TABLERO_Y 20
#define TAM_CELDA 23
#define TABLERO_TAM (TAM_CELDA * 9)

#define TECLADO_X 245
#define TECLADO_Y 20
#define TECLADO_W 70
#define TECLADO_H 207
#define TECLADO_COLS 3
#define TECLADO_ROWS 4
#define TECLADO_CELDA_W (TECLADO_W / TECLADO_COLS)
#define TECLADO_CELDA_H (TECLADO_H / TECLADO_ROWS)

static int tactil_celda_activa = 0;
static uint8_t tactil_fila = 0;
static uint8_t tactil_col = 0;

static void procesar_toque_tactil(INT16U x, INT16U y)
{
	if (!Sudoku_Juego_En_Progreso())
	{
		if (Sudoku_Partida_Terminada())
		{
			Sudoku_Reiniciar_A_Esperando();
		}
		else
		{
			Sudoku_Iniciar_Partida();
		}
		tactil_celda_activa = 0;
		return;
	}

	if (x >= TABLERO_X && x < (TABLERO_X + TABLERO_TAM) &&
	    y >= TABLERO_Y && y < (TABLERO_Y + TABLERO_TAM))
	{
		tactil_col = (x - TABLERO_X) / TAM_CELDA;
		tactil_fila = (y - TABLERO_Y) / TAM_CELDA;
		tactil_celda_activa = 1;
		Sudoku_Actualizar_Tablero_Completo(cuadricula);
		Sudoku_Resaltar_Celda(tactil_fila, tactil_col, LIGHTGRAY);
		return;
	}

	if (x >= TECLADO_X && x < (TECLADO_X + TECLADO_W) &&
	    y >= TECLADO_Y && y < (TECLADO_Y + TECLADO_H))
	{
		uint8_t fila = (y - TECLADO_Y) / TECLADO_CELDA_H;
		uint8_t col = (x - TECLADO_X) / TECLADO_CELDA_W;

		if (fila < 3)
		{
			uint8_t valor = fila * 3 + col + 1;
			if (tactil_celda_activa)
			{
				Sudoku_Procesar_Entrada_Tactil(tactil_fila, tactil_col, valor);
				tactil_celda_activa = 0;
			}
			return;
		}

		if (fila == 3)
		{
			if (col == 0)
			{
				if (tactil_celda_activa)
				{
					Sudoku_Procesar_Entrada_Tactil(tactil_fila, tactil_col, 0);
					tactil_celda_activa = 0;
				}
				return;
			}
			if (col == 1)
			{
				Sudoku_Terminar_Partida();
				tactil_celda_activa = 0;
				return;
			}
		}
	}
}

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

    /* Inicializar LCD y calibrar el táctil */
    Lcd_Init();
    TS_init();
    TS_Calibrar();

    /* Mostrar pantalla inicial */
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
        INT16U touch_x;
        INT16U touch_y;

        if (TS_HayEvento())
        {
            TS_LeerEvento(&touch_x, &touch_y);
            procesar_toque_tactil(touch_x, touch_y);
        }

        /* El latido (LED2 parpadeando) se gestiona automáticamente por timer1 */
        /* El programa está vivo mientras el LED2 parpadee a 6 Hz */
        
        /* Actualizar el tiempo en pantalla cada segundo */
        tiempo_actual = timer2_count();
        
        /* Actualizar cada 1 segundo (1000000 microsegundos) */
        if ((tiempo_actual - tiempo_anterior) >= 1000000)
        {
            /* Solo actualizar si el juego está en progreso (no en pantalla inicial ni terminado) */
            if (Sudoku_Juego_En_Progreso())
            {
                /* Calcular tiempo transcurrido desde el inicio de la partida */
                unsigned int tiempo_transcurrido = tiempo_actual - Sudoku_Obtener_Tiempo_Inicio();
                Sudoku_Actualizar_Tiempo(tiempo_transcurrido);
            }
            tiempo_anterior = tiempo_actual;
        }
    }
}
