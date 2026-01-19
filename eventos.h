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

/*--- Eventos genéricos para la máquina de estados ---*/
typedef enum {
    EVENTO_INICIAR_JUEGO,       /* Iniciar nueva partida */
    EVENTO_INCREMENTAR,         /* Incrementar valor (fila/columna/valor) */
    EVENTO_CONFIRMAR,           /* Confirmar selección actual */
    EVENTO_INSERTAR_VALOR,      /* Insertar un valor específico (desde touch) */
    EVENTO_BORRAR_VALOR,        /* Borrar valor de una celda (desde touch) */
    EVENTO_ZOOM_REGION,         /* Hacer zoom en una región (desde touch) */
    EVENTO_CERRAR_ZOOM,         /* Cerrar zoom y volver (desde touch) */
} EventoGenerico;


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