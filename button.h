/*********************************************************************************************
* Fichero:	button.h
* Autor:
* Descrip:	Funciones de manejo de los pulsadores físicos (EINT6-7)
* Version:	2.0
* Comentarios: Este módulo solo maneja las interrupciones de hardware de los botones.
*              La lógica de la máquina de estados está en maquina_estados.c/h
*********************************************************************************************/

#ifndef _BUTTON_H_
#define _BUTTON_H_

/*=====================================================================================
 * FUNCIONES PÚBLICAS DEL MÓDULO BUTTON
 *====================================================================================*/

/*********************************************************************************************
* name:		Eint4567_init
* func:		Inicializa las interrupciones de los botones físicos (EINT6-7)
* ret:		none
* comment:	Configura el hardware, registra la ISR y conecta con timer3 (antirrebotes)
*********************************************************************************************/
void Eint4567_init(void);

#endif /* _BUTTON_H_ */