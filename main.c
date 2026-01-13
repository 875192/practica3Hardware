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
#include "cola.h"
#include "sudoku_2025.h"
#include "lcd.h"

/* Variables globales del Sudoku */
#include "tableros.h"

/* Apuntar a una de las cuadrículas disponibles */
CELDA (*cuadricula)[NUM_COLUMNAS] = cuadricula_ARM_ARM; // CAMBIAR SEGUN LA VERSIÓN A PROBAR
int celdas_vacias = 0;

/*--- function declare ---*/
void Main(void);

/*--- function code ---*/
/*********************************************************************************************
* name:		Main
* func:		c code entry - VERSIÓN INTEGRADA: Touchscreen + Sudoku
* para:		none
* ret:		none
* modify:
*********************************************************************************************/
void Main(void)
{
    sys_init();        /* Initial 44B0X's Interrupt, Port and UART */
    //_Link();           /* Print Misc info */
    
    /* Inicializa controladores del Sudoku */
    timer2_init();      // Inicializacion del timer2 para medicion de tiempo
    timer1_init();      // Inicializacion del timer1 para latido (heartbeat) - Paso 6
    cola_init();        // Inicializacion de la cola de depuracion (Paso 4)
    Eint4567_init();    // inicializamos los pulsadores. Cada vez que se pulse se verá reflejado en el 8led
    D8Led_init();       // inicializamos el 8led

    /* Inicializar LCD y mostrar pantalla inicial */
    Lcd_Init();
    TS_init();
    
    /* Calibrar touchscreen */
    ts_calibrate_5pt(SCR_XSIZE, SCR_YSIZE, 30);
    
    Sudoku_Pantalla_Inicial();

    /* Valor inicial de los leds */
    leds_off();
    
    /* Variable para controlar actualización del tiempo */
    unsigned int tiempo_anterior = 0;
    unsigned int tiempo_actual = 0;
    
    /* Variables para touchscreen */
    int touch_x, touch_y;
    
    /* Bucle principal */
    while (1)
    {
        /* El latido (LED2 parpadeando) se gestiona automáticamente por timer1 */
        /* El programa está vivo mientras el LED2 parpadee a 6 Hz */
        
        /* Comprobar si hay toque en la pantalla */
        if (ts_read_calibrated(&touch_x, &touch_y) == 0)
        {
            /* Verificar si hay región expandida activa */
            if (Sudoku_Esta_Region_Expandida_Activa())
            {
                /* Procesar toque en la región expandida */
                Sudoku_Procesar_Touch_Region_Expandida(touch_x, touch_y);
            }
            else
            {
                /* Procesar toque en el tablero principal */
                Sudoku_Procesar_Touch(touch_x, touch_y);
            }
            
            Delay(30);  // Evitar múltiples detecciones
        }
        
        /* Actualizar el tiempo en pantalla cada segundo (solo si no hay región expandida) */
        if (!Sudoku_Esta_Region_Expandida_Activa())
        {
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
}
