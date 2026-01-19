/*********************************************************************************************
* Fichero:		maquina_estados.h
* Autor:		
* Descrip:		Máquina de estados del juego Sudoku - Interfaz pública
* Version:		1.0
* Comentarios:	Este módulo contiene la lógica central de la máquina de estados del juego.
*				Puede ser utilizado tanto por botones físicos como por el touchscreen LCD.
*********************************************************************************************/

#ifndef _MAQUINA_ESTADOS_H_
#define _MAQUINA_ESTADOS_H_

#include "def.h"      /* Tipos de datos: INT8U, INT32U, etc. */
#include "eventos.h"

/*=====================================================================================
 * FUNCIONES PARA CONSULTA DE ESTADO
 *====================================================================================*/

/*********************************************************************************************
* name:		Sudoku_Obtener_Estado
* func:		Obtiene el estado actual de la máquina de estados
* ret:		Estado actual (int que representa EstadoSudoku)
*********************************************************************************************/
int Sudoku_Obtener_Estado(void);

/*********************************************************************************************
* name:		Sudoku_Partida_Terminada
* func:		Consulta si la partida está terminada
* ret:		1 si terminada, 0 si no
*********************************************************************************************/
int Sudoku_Partida_Terminada(void);

/*********************************************************************************************
* name:		Sudoku_Juego_En_Progreso
* func:		Consulta si el juego está en progreso (no en menú ni terminado)
* ret:		1 si en progreso, 0 si no
*********************************************************************************************/
int Sudoku_Juego_En_Progreso(void);

/*********************************************************************************************
* name:		Sudoku_Obtener_Tiempo_Inicio
* func:		Obtiene el timestamp de inicio de la partida actual
* ret:		Tiempo de inicio (timer2_count)
*********************************************************************************************/
unsigned int Sudoku_Obtener_Tiempo_Inicio(void);


/*=====================================================================================
 * FUNCIONES PARA TRANSICIÓN DE ESTADOS
 *====================================================================================*/

/*********************************************************************************************
* name:		Sudoku_Cambiar_Estado
* func:		Cambia manualmente el estado de la máquina (usado por LCD)
* para:		nuevo_estado - Estado al que cambiar (de tipo EstadoSudoku)
* ret:		none
* comment:	Actualiza automáticamente el 8LED según el nuevo estado
*********************************************************************************************/
void Sudoku_Cambiar_Estado(int nuevo_estado);


/*=====================================================================================
 * FUNCIONES PARA BOTONES FÍSICOS (button.c)
 *====================================================================================*/

/*********************************************************************************************
* name:		Maquina_Procesar_Boton
* func:		Procesa una pulsación de botón confirmada (sin rebotes)
* para:		boton_id - ID del botón (EVENTO_BOTON_IZQUIERDO o EVENTO_BOTON_DERECHO)
* ret:		none
* comment:	Esta función contiene el switch principal de la máquina de estados.
*			Debe ser llamada desde el callback del timer de antirrebotes (timer3).
*********************************************************************************************/
void Maquina_Procesar_Boton(INT8U boton_id);


/*=====================================================================================
 * FUNCIONES PARA LCD TOUCHSCREEN (lcd.c)
 *====================================================================================*/

/*********************************************************************************************
* name:		Maquina_Procesar_Touch
* func:		Procesa una acción táctil (insertar o borrar valor)
* para:		accion - tipo de acción (EVENTO_INSERTAR_VALOR o EVENTO_BORRAR_VALOR)
*			fila, col - posición de la celda (0-8)
*			valor - valor a insertar (1-9), ignorado si accion=EVENTO_BORRAR_VALOR
* ret:		none
* comment:	Esta función es el adaptador principal del touchscreen.
*			Valida la entrada y delega al núcleo genérico de la máquina de estados.
*			Solo funciona si el estado actual es INTRODUCIR_VALOR.
*********************************************************************************************/
void Maquina_Procesar_Touch(EventoGenerico accion, int fila, int col, int valor);


#endif /* _MAQUINA_ESTADOS_H_ */
