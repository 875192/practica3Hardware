/*********************************************************************************************
* Fichero:      button.c
* Autor:
* Descrip:      Manejo de interrupciones de botones físicos (EINT6-7)
* Version:      2.0
* Comentarios:  Este módulo SOLO maneja el hardware de los botones físicos.
*               La lógica de la máquina de estados está en maquina_estados.c
*               Flujo: ISR → Timer3 (antirrebotes) → boton_confirmado → Maquina_Procesar_Boton
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "button.h"
#include "maquina_estados.h"  /* Máquina de estados del juego */
#include "eventos.h"
#include "timer3.h"
#include "44blib.h"
#include "44b.h"
#include "def.h"

/*=====================================================================================
 * CALLBACK PARA BOTONES CONFIRMADOS (sin rebotes)
 *====================================================================================*/

/*********************************************************************************************
* name:		boton_confirmado
* func:		Callback invocado por timer3 cuando un botón está confirmado (sin rebotes)
* para:		boton_id - ID del botón (EVENTO_BOTON_IZQUIERDO o EVENTO_BOTON_DERECHO)
* ret:		none
* comment:	Esta función solo delega el procesamiento a la máquina de estados.
*			NO contiene lógica del juego, solo interfaz hardware→software.
*********************************************************************************************/
static void boton_confirmado(INT8U boton_id)
{
	/* Delegar el procesamiento a la máquina de estados */
	Maquina_Procesar_Boton(boton_id);
}


/*=====================================================================================
 * RUTINA DE SERVICIO DE INTERRUPCIÓN (ISR)
 *====================================================================================*/

/* declaración de función que es rutina de servicio de interrupción
 * https://gcc.gnu.org/onlinedocs/gcc/ARM-Function-Attributes.html */
void Eint4567_ISR(void) __attribute__((interrupt("IRQ")));

/*********************************************************************************************
* name:		Eint4567_ISR
* func:		Rutina de servicio de interrupción para botones físicos EINT6-7
* ret:		none
* comment:	Identifica qué botón fue presionado e inicia el proceso de antirrebotes.
*			NO procesa la lógica del juego directamente, solo identifica el botón.
*********************************************************************************************/
void Eint4567_ISR(void)
{
	/* Identificar la interrupción (hay dos pulsadores) */
	unsigned int pending = rEXTINTPND & 0xF;
	INT8U boton_id = 0;

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
		/* Iniciar la máquina de antirrebotes. Si está ocupada, no hacer nada */
		timer3_start_antirrebote(boton_id);
	}

	/* Finalizar ISR: limpiar banderas de interrupción */
	rEXTINTPND = 0xf;              // borra los bits en EXTINTPND
	rI_ISPC   |= BIT_EINT4567;     // borra el bit pendiente en INTPND
}


/*=====================================================================================
 * INICIALIZACIÓN DEL HARDWARE
 *====================================================================================*/

/*********************************************************************************************
* name:		Eint4567_init
* func:		Inicializa el hardware de los botones físicos EINT6-7
* ret:		none
* comment:	Configura:
*			- Timer3 para antirrebotes
*			- Controlador de interrupciones
*			- Puerto G (pines físicos de los botones)
*********************************************************************************************/
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
