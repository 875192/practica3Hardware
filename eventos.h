/*********************************************************************************************
* Fichero:		eventos.h
* Autor:		
* Descrip:		Definición de eventos para la cola de depuración
* Version:		1.0
*********************************************************************************************/

#ifndef _EVENTOS_H_
#define _EVENTOS_H_

/*--- Definición de identificadores de eventos ---*/
typedef enum {
        EVENTO_BOTON_IZQUIERDO = 4,
        EVENTO_BOTON_DERECHO   = 8,
} ID_Evento;


/*--- Maquina de estados ---*/
typedef enum {
    ESPERANDO_PULSACION,
    REBOTE_PRESION,
    MONITORIZANDO,
    AUTOREPETICION,
    REBOTE_DEPRESION
} EstadoPulsador;

/*--- Estados del Juego Sudoku ---*/
typedef enum {
    ESPERANDO_INICIO,
    INTRODUCIR_FILA,
    INTRODUCIR_COLUMNA,
    VERIFICAR_CELDA,
    INTRODUCIR_VALOR,
    VERIFICAR_VALOR,
    BORRAR_VALOR,
    PARTIDA_TERMINADA,
} EstadoSudoku;

#endif /* _EVENTOS_H_ */