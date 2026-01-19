/*********************************************************************************************
* Fichero:	button.h
* Autor:
* Descrip:	Funciones de manejo de los pulsadores (EINT6-7)
* Version:
*********************************************************************************************/

#ifndef _BUTTON_H_
#define _BUTTON_H_

/*--- Declaración de funciones visibles del módulo button.c/button.h ---*/

void Eint4567_init(void);
int Sudoku_Partida_Terminada(void);
int Sudoku_Juego_En_Progreso(void);
unsigned int Sudoku_Obtener_Tiempo_Inicio(void);

/*--- Funciones para integración con LCD touchscreen ---*/
void Sudoku_Cambiar_Estado(int nuevo_estado);
int Sudoku_Obtener_Estado(void);
void Sudoku_Insertar_Valor_Touch(int fila, int col, int valor);
void Sudoku_Borrar_Valor_Touch(int fila, int col);

#endif /* _BUTTON_H_ */