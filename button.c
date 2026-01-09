/*********************************************************************************************
* Fichero:      button.c
* Autor:
* Descrip:      Funciones de manejo de los pulsadores (EINT6-7)
* Version:
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "button.h"
#include "8led.h"
#include "cola.h"
#include "eventos.h"
#include "timer2.h"
#include "timer3.h"
#include "44blib.h"
#include "44b.h"
#include "def.h"
#include "sudoku_2025.h"
#include "lcd.h"

/*--- Variables del juego Sudoku ---*/
static volatile EstadoSudoku estado_juego = ESPERANDO_INICIO;
static volatile uint8_t int_count = 0;
static volatile uint8_t fila = 0;
static volatile uint8_t columna = 0;
static volatile uint8_t valor = 0;
static volatile uint8_t valor_previo = 0;  /* Para detectar modificación de valor */
static volatile uint8_t pantalla_mostrada = 0;  /* Flag para mostrar pantalla inicial solo una vez */
static volatile uint32_t tiempo_final = 0;  /* Tiempo final al terminar la partida */

/* Declaración externa de la cuadrícula del juego */
extern CELDA (*cuadricula)[NUM_COLUMNAS];
extern int celdas_vacias;

/* Callback para recibir la confirmación de pulsaciones filtradas por el timer3 */
static void boton_confirmado(uint8_t boton_id) // MODIFICAR FUNCIONES ACTUALIZAR Y PROPAGAR SEGUN LA VERSIÓN A PROBAR
{
        cola_depuracion(timer2_count(), boton_id, estado_juego);
        
        switch (estado_juego)
        {
                case ESPERANDO_INICIO:
                        /* Cualquier botón inicia el juego */
                        /* Calcular candidatos por primera vez */
                        celdas_vacias = candidatos_actualizar_all(cuadricula);
                        
                        /* Dibujar el tablero del juego */
                        Sudoku_Dibujar_Tablero();
                        
                        /* Actualizar con los valores de la cuadrícula */
                        Sudoku_Actualizar_Tablero_Completo(cuadricula);
                        
                        /* Pasar a introducir fila */
                        estado_juego = INTRODUCIR_FILA;
                        int_count = 9;  /* Iniciar en 9 para que al incrementar vaya a 0 */
                        D8Led_symbol(15);  /* Mostrar 'F' de Fila */
                        pantalla_mostrada = 0;  /* Resetear flag para próxima partida */
                        break;
                
                case INTRODUCIR_FILA:
                        if (boton_id == EVENTO_BOTON_DERECHO)
                        {
                                /* Incrementar fila (ciclo: 0 → 1 → 2 → ... → 9 → 0) */
                                int_count++;
                                if (int_count > 9)
                                {
                                        int_count = 0;  /* Volver a 0 */
                                }
                                D8Led_symbol(int_count & 0x000f);
                        }
                        else if (boton_id == EVENTO_BOTON_IZQUIERDO)
                        {
                                /* Verificar si se eligió fila 0 (terminar partida) */
                                if (int_count == 0)
                                {
                                        /* Fila 0: terminar la partida */
                                        estado_juego = PARTIDA_TERMINADA;
                                        tiempo_final = timer2_count();
                                        /* La pantalla final se mostrará en este estado */
                                }
                                else
                                {
                                        /* Confirmar fila y pasar a introducir columna */
                                        fila = int_count - 1;  /* Convertir a índice 0-8 */
                                        estado_juego = INTRODUCIR_COLUMNA;
                                        int_count = 0;
                                        D8Led_symbol(12);  /* Mostrar 'C' de Columna (índice 12 en el array Symbol) */
                                }
                        }
                        break;
                
                case INTRODUCIR_COLUMNA:
                        if (boton_id == EVENTO_BOTON_DERECHO)
                        {
                                /* Incrementar columna */
                                int_count++;
                                if (int_count > 9)
                                {
                                        int_count = 1;
                                }
                                D8Led_symbol(int_count & 0x000f);
                        }
                        else if (boton_id == EVENTO_BOTON_IZQUIERDO)
                        {
                                /* Confirmar columna y pasar a introducir valor */
                                columna = int_count - 1;  /* Convertir a índice 0-8 */
                                estado_juego = VERIFICAR_CELDA;
                                int_count = 0;
                                D8Led_symbol(int_count & 0x000f);
                        }
                        break;
                
                case VERIFICAR_CELDA:
                        /* Primero se verifica si la celda[fila][columna] no es una pista */
                        if (celda_es_pista(cuadricula[fila][columna]))
                        {
                                // Se vuelve al estado INTRODUCIR_FILA
                                estado_juego = INTRODUCIR_FILA;
                                int_count = 0;
                                D8Led_symbol(15);  /* Mostrar 'F' de Fila (índice 15 en el array Symbol) */
                        } 
                        else
                        {
                                // Si no es pista, se pasa a INTRODUCIR_VALOR
                                estado_juego = INTRODUCIR_VALOR;
                                int_count = 0;
                        }
                        break;
                
                case INTRODUCIR_VALOR:
                        if (boton_id == EVENTO_BOTON_DERECHO)
                        {
                                /* Incrementar valor */
                                int_count++;
                                if (int_count > 9)
                                {
                                        int_count = 0;
                                }
                                D8Led_symbol(int_count & 0x000f);
                        }
                        else if (boton_id == EVENTO_BOTON_IZQUIERDO)
                        {
                                /* Confirmar valor e intentar escribir en la celda */
                                valor = int_count;  /* Valor a escribir (0-9) */
                                
                                estado_juego = VERIFICAR_VALOR;
                                int_count = 0;
                        }
                        break;
                
                case VERIFICAR_VALOR:
                        /* Guardar valor previo de la celda */
                        valor_previo = celda_leer_valor(cuadricula[fila][columna]);
                        
                        if (valor == 0)
                        {
                                /* Valor 0 = borrar -> pasar a BORRAR_VALOR */
                                /* Borrar el valor de la celda */
                                celda_limpiar_error(&cuadricula[fila][columna]);
                                celda_poner_valor(&cuadricula[fila][columna], 0);
                                
                                /* Al borrar un valor, hay que recalcular todos los candidatos */
                                celdas_vacias = candidatos_actualizar_all(cuadricula);
                                
                                /* Actualizar la visualización del tablero */
                                Sudoku_Actualizar_Tablero_Completo(cuadricula);
                                
                                /* Volver a introducir fila */
                                estado_juego = INTRODUCIR_FILA;
                                int_count = 9;  /* Iniciar en 9 para que al incrementar vaya a 0 */
                                D8Led_symbol(15);  /* Mostrar 'F' de Fila */
                                break;
                        }
                        else
                        {
                                /* Verificar si el valor es candidato */
                                if (celda_es_candidato(cuadricula[fila][columna], valor))
                                {
                                        /* Es candidato: escribir el valor en la celda */
                                        celda_limpiar_error(&cuadricula[fila][columna]);
                                        celda_poner_valor(&cuadricula[fila][columna], valor);
                                        
                                        /* Decidir si propagar o actualizar según el caso */
                                        if (valor_previo != 0)
                                        {
                                                /* Se modificó un valor previo -> recalcular todo */
                                                celdas_vacias = candidatos_actualizar_all(cuadricula);
                                        }
                                        else
                                        {
                                                /* Celda vacía -> solo propagar el nuevo valor */
                                                candidatos_propagar_arm(cuadricula, fila, columna);
                                        }
                                        
                                        /* Actualizar la visualización del tablero */
                                        Sudoku_Actualizar_Tablero_Completo(cuadricula);
                                        
                                        /* Volver a introducir fila */
                                        estado_juego = INTRODUCIR_FILA;
                                        int_count = 9;  /* Iniciar en 9 para que al incrementar vaya a 0 */
                                        D8Led_symbol(15);  /* Mostrar 'F' de Fila */
                                }
                                else
                                {
                                        /* No es candidato: marcar error y mostrar 'E' */
                                        celda_marcar_error(&cuadricula[fila][columna]);
                                        
                                        /* Actualizar la visualización del tablero */
                                        Sudoku_Actualizar_Tablero_Completo(cuadricula);
                                        
                                        D8Led_symbol(14);  /* Mostrar 'E' de Error */
                                        
                                        /* Volver a introducir fila */
                                        estado_juego = INTRODUCIR_FILA;
                                        int_count = 9;  /* Iniciar en 9 para que al incrementar vaya a 0 */
                                        D8Led_symbol(15);  /* Mostrar 'F' de Fila */
                                }
                        }
                        break;
                
                case PARTIDA_TERMINADA:
                        /* Mostrar pantalla de despedida solo una vez */
                        if (!pantalla_mostrada)
                        {
                                /* Usar la función existente de lcd.c */
                                Sudoku_Pantalla_Final(tiempo_final);
                                
                                pantalla_mostrada = 1;
                        }
                        /* No hacer nada más - el juego permanece terminado */
                        break;
        }
}

/* declaración de función que es rutina de servicio de interrupción
 * https://gcc.gnu.org/onlinedocs/gcc/ARM-Function-Attributes.html */
void Eint4567_ISR(void) __attribute__((interrupt("IRQ")));

/*--- código de funciones ---*/
void Eint4567_ISR(void)
{
        /* Identificar la interrupción (hay dos pulsadores) */
        unsigned int pending = rEXTINTPND & 0xF;
        uint8_t boton_id = 0;

        if (pending & 0x4)
        {
                boton_id = EVENTO_BOTON_IZQUIERDO;
        }
        else if (pending & 0x8)
        {
                boton_id = EVENTO_BOTON_DERECHO;
        }

        if (boton_id != 0U)
        {
                /* No registrar la pulsación inicial, solo las confirmadas */

                /* Iniciar la máquina de antirrebotes. Si está ocupada, no hacer nada */
                timer3_start_antirrebote(boton_id);
        }

        /* Finalizar ISR */
        rEXTINTPND = 0xf;                               // borra los bits en EXTINTPND
        rI_ISPC   |= BIT_EINT4567;              // borra el bit pendiente en INTPND
}
/* Función para consultar si la partida está terminada */
int Sudoku_Partida_Terminada(void)
{
	return (estado_juego == PARTIDA_TERMINADA);
}
void Eint4567_init(void)
{
        /* Inicializar el temporizador dedicado a la eliminación de rebotes */
        timer3_init(boton_confirmado);

        /* Configuracion del controlador de interrupciones. Estos registros están definidos en 44b.h */
        rI_ISPC    = 0x3ffffff;         // Borra INTPND escribiendo 1s en I_ISPC
        rEXTINTPND = 0xf;               // Borra EXTINTPND escribiendo 1s en el propio registro
        rINTMOD    = 0x0;               // Configura las lineas como de tipo IRQ
        rINTCON    = 0x1;               // Habilita int. vectorizadas y la linea IRQ (FIQ no)
        rINTMSK    &= ~(BIT_EINT4567);  // habilitamos interrupcion linea eint4567 en vector de mascaras

        /* Establece la rutina de servicio para Eint4567 */
        pISR_EINT4567 = (int) Eint4567_ISR;

        /* Configuracion del puerto G */
        rPCONG  = 0xffff;                       // Establece la funcion de los pines (EINT0-7)
        rPUPG   = 0x0;                  // Habilita el "pull up" del puerto
        rEXTINT = rEXTINT | 0x22222222;   // Configura las lineas de int. como de flanco de bajada

        /* Por precaucion, se vuelven a borrar los bits de INTPND y EXTINTPND */
        rEXTINTPND = 0xf;                               // borra los bits en EXTINTPND
        rI_ISPC   |= BIT_EINT4567;              // borra el bit pendiente en INTPND
}
