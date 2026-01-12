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
void Sudoku_Iniciar_Partida(void);
void Sudoku_Reiniciar_A_Esperando(void);
void Sudoku_Terminar_Partida(void);
void Sudoku_Procesar_Entrada_Tactil(unsigned char fila, unsigned char columna, unsigned char valor);




#endif /* _BUTTON_H_ */
