/*********************************************************************************************
* Fichero:		maquina_estados.c
* Autor:		
* Descrip:		MÃ¡quina de estados del juego Sudoku - ImplementaciÃ³n
* Version:		1.0
* Comentarios:	Este mÃ³dulo contiene TODA la lÃ³gica de la mÃ¡quina de estados del juego.
*				Es independiente del mÃ©todo de entrada (botones fÃ­sicos o touchscreen).
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "maquina_estados.h"
#include "8led.h"
#include "cola.h"
#include "eventos.h"
#include "timer2.h"
#include "lcd.h"
#include "44blib.h"
#include "def.h"
#include "sudoku_2025.h"
#include "lcd.h"
#include "tableros.h"

/*=====================================================================================
 * VARIABLES DE ESTADO (privadas del mÃ³dulo)
 *====================================================================================*/

/* Variables de la mÃ¡quina de estados */
static volatile EstadoSudoku estado_juego = ESPERANDO_INICIO;
static volatile INT8U int_count = 0;              /* Contador para incremento de fila/columna/valor */
static volatile INT8U fila = 0;                   /* Fila seleccionada (0-8) */
static volatile INT8U columna = 0;                /* Columna seleccionada (0-8) */
static volatile INT8U valor = 0;                  /* Valor a insertar (0-9) */
static volatile INT8U valor_previo = 0;           /* Valor previo de la celda (para detectar modificaciÃ³n) */

/* Variables de control del juego */
static volatile INT8U pantalla_mostrada = 0;      /* Flag: pantalla inicial/final mostrada */
static volatile INT32U tiempo_inicio = 0;         /* Timestamp de inicio de partida (timer2_count) */
static volatile INT32U tiempo_final = 0;          /* Timestamp de fin de partida */

/* DeclaraciÃ³n externa de la cuadrÃ­cula del juego */
extern CELDA (*cuadricula)[NUM_COLUMNAS];
extern int celdas_vacias;


/*=====================================================================================
 * FUNCIONES AUXILIARES PRIVADAS
 *====================================================================================*/

/*********************************************************************************************
* name:		restaurar_cuadricula_original
* func:		Restaura la cuadrÃ­cula al estado inicial (para reiniciar partida)
* ret:		none
*********************************************************************************************/
static void restaurar_cuadricula_original(void)
{
	INT8U f, c;
	
	/* Copiar toda la cuadrÃ­cula original desde el backup */
	for (f = 0; f < NUM_FILAS; f++)
	{
		for (c = 0; c < NUM_COLUMNAS; c++)
		{
			cuadricula[f][c] = cuadricula_Respaldo[f][c];
		}
	}
}

/*********************************************************************************************
* name:		marcar_celdas_en_conflicto
* func:		Marca todas las celdas que estÃ¡n en conflicto con un valor introducido
* para:		fila_error, col_error - celda donde se intentÃ³ poner el valor
*			valor_error - valor que causÃ³ el conflicto
* ret:		none
* comment:	Marca la celda objetivo y todas las celdas con el mismo valor en:
*			- La misma fila
*			- La misma columna
*			- La misma regiÃ³n 3x3
*********************************************************************************************/
static void marcar_celdas_en_conflicto(INT8U fila_error, INT8U col_error, INT8U valor_error)
{
	INT8U i, f, c;
	INT8U region_fila_inicio, region_col_inicio;
	
	/* Primero limpiar todos los errores previos */
	for (f = 0; f < NUM_FILAS; f++)
	{
		for (c = 0; c < NUM_COLUMNAS; c++)
		{
			celda_limpiar_error(&cuadricula[f][c]);
		}
	}
	
	/* Marcar la celda donde intentamos poner el valor */
	celda_marcar_error(&cuadricula[fila_error][col_error]);
	
	/* Buscar y marcar todas las celdas con el mismo valor en la misma fila */
	for (i = 0; i < NUM_COLUMNAS; i++)
	{
		if (i != col_error && celda_leer_valor(cuadricula[fila_error][i]) == valor_error)
		{
			celda_marcar_error(&cuadricula[fila_error][i]);
		}
	}
	
	/* Buscar y marcar todas las celdas con el mismo valor en la misma columna */
	for (i = 0; i < NUM_FILAS; i++)
	{
		if (i != fila_error && celda_leer_valor(cuadricula[i][col_error]) == valor_error)
		{
			celda_marcar_error(&cuadricula[i][col_error]);
		}
	}
	
	/* Buscar y marcar todas las celdas con el mismo valor en la misma regiÃ³n 3x3 */
	region_fila_inicio = (fila_error / 3) * 3;
	region_col_inicio = (col_error / 3) * 3;
	
	for (f = region_fila_inicio; f < region_fila_inicio + 3; f++)
	{
		for (c = region_col_inicio; c < region_col_inicio + 3; c++)
		{
			if ((f != fila_error || c != col_error) && celda_leer_valor(cuadricula[f][c]) == valor_error)
			{
				celda_marcar_error(&cuadricula[f][c]);
			}
		}
	}
}


/*=====================================================================================
 * IMPLEMENTACIÃ“N: FUNCIONES PÃšBLICAS - CONSULTA DE ESTADO
 *====================================================================================*/

int Sudoku_Obtener_Estado(void)
{
	return (int)estado_juego;
}

int Sudoku_Partida_Terminada(void)
{
	return (estado_juego == PARTIDA_TERMINADA);
}

int Sudoku_Juego_En_Progreso(void)
{
	return (estado_juego != ESPERANDO_INICIO && estado_juego != PARTIDA_TERMINADA);
}

unsigned int Sudoku_Obtener_Tiempo_Inicio(void)
{
	return tiempo_inicio;
}


/*=====================================================================================
 * IMPLEMENTACIÃ“N: TRANSICIÃ“N MANUAL DE ESTADOS
 *====================================================================================*/

void Sudoku_Cambiar_Estado(int nuevo_estado)
{
	estado_juego = (EstadoSudoku)nuevo_estado;
	
	/* Actualizar 8LED segÃºn el nuevo estado */
	switch (estado_juego)
	{
		case INTRODUCIR_FILA:
			D8Led_symbol(15);  /* 'F' de Fila */
			int_count = 0;
			break;
			
		case INTRODUCIR_COLUMNA:
			D8Led_symbol(12);  /* 'C' de Columna */
			int_count = 0;
			break;
			
		case INTRODUCIR_VALOR:
			/* Mostrar 0 para indicar que se puede introducir valor */
			D8Led_symbol(0);
			int_count = 0;
			break;
			
		case BORRAR_VALOR:
			/* Mostrar 0 al borrar */
			D8Led_symbol(0);
			break;
			
		case VERIFICAR_VALOR:
		case VERIFICAR_CELDA:
			/* No cambiar el 8LED durante verificaciÃ³n */
			break;
			
		default:
			/* Otros estados no cambian el 8LED */
			break;
	}
}


/*=====================================================================================
 * IMPLEMENTACIÃ“N: NÃšCLEO GENÃ‰RICO DE LA MÃ�QUINA DE ESTADOS
 * Esta funciÃ³n procesa eventos genÃ©ricos independientemente del origen (botones/LCD)
 *====================================================================================*/

/*********************************************************************************************
* name:		Maquina_Procesar_Evento
* func:		Procesa un evento genÃ©rico en la mÃ¡quina de estados
* para:		evento - Evento genÃ©rico a procesar
* ret:		none
* comment:	Esta es la funciÃ³n CENTRAL que contiene toda la lÃ³gica de la mÃ¡quina.
*			Es independiente del mÃ©todo de entrada (botones fÃ­sicos o touchscreen).
*********************************************************************************************/

static void Maquina_Procesar_Evento(EventoGenerico evento)
{
	switch (estado_juego)
	{
		/*------------------------------------------------------------------
		 * ESTADO 0: ESPERANDO_INICIO
		 * Evento: INICIAR_JUEGO
		 *-----------------------------------------------------------------*/
		case ESPERANDO_INICIO:
			if (evento == EVENTO_INICIAR_JUEGO)
			{
				/* Guardar tiempo de inicio */
				tiempo_inicio = timer2_count();
				
				/* Calcular candidatos por primera vez */
				celdas_vacias = candidatos_actualizar_all(cuadricula);
				
				/* Dibujar el tablero del juego */
				Sudoku_Dibujar_Tablero();
				Sudoku_Actualizar_Tablero_Completo(cuadricula);
				
				/* Pasar a introducir fila */
				estado_juego = INTRODUCIR_FILA;
				int_count = 9;  /* Iniciar en 9 para que al incrementar vaya a 0 */
				D8Led_symbol(15);  /* Mostrar 'F' de Fila */
				pantalla_mostrada = 0;
			}
			break;
		
		/*------------------------------------------------------------------
		 * ESTADO 1: INTRODUCIR_FILA
		 * Eventos: INCREMENTAR (incrementa fila), CONFIRMAR (selecciona fila)
		 *-----------------------------------------------------------------*/
		case INTRODUCIR_FILA:
			if (evento == EVENTO_INCREMENTAR)
			{
				/* Incrementar fila (ciclo: 0â†’1â†’...â†’9â†’0) */
				int_count++;
				if (int_count > 9)
				{
					int_count = 0;
				}
				D8Led_symbol(int_count & 0x000f);
			}
			else if (evento == EVENTO_CONFIRMAR)
			{
				/* Verificar si se eligiÃ³ fila 0 (terminar partida) */
				if (int_count == 0)
				{
					/* Fila 0: terminar la partida */
					tiempo_final = timer2_count() - tiempo_inicio;
					estado_juego = PARTIDA_TERMINADA;
				}
				else
				{
					/* Confirmar fila y pasar a introducir columna */
					fila = int_count - 1;  /* Convertir a Ã­ndice 0-8 */
					estado_juego = INTRODUCIR_COLUMNA;
					int_count = 0;
					D8Led_symbol(12);  /* Mostrar 'C' de Columna */
				}
			}
			break;
		
		/*------------------------------------------------------------------
		 * ESTADO 2: INTRODUCIR_COLUMNA
		 * Eventos: INCREMENTAR (incrementa columna), CONFIRMAR (selecciona columna)
		 *-----------------------------------------------------------------*/
		case INTRODUCIR_COLUMNA:
			if (evento == EVENTO_INCREMENTAR)
			{
				/* Incrementar columna */
				int_count++;
				if (int_count > 9)
				{
					int_count = 1;
				}
				D8Led_symbol(int_count & 0x000f);
			}
			else if (evento == EVENTO_CONFIRMAR)
			{
				/* Confirmar columna y verificar celda */
				columna = int_count - 1;  /* Convertir a Ã­ndice 0-8 */
				estado_juego = VERIFICAR_CELDA;
				int_count = 0;
				D8Led_symbol(int_count & 0x000f);
			}
			break;
		
		/*------------------------------------------------------------------
		 * ESTADO 3: VERIFICAR_CELDA
		 * Verifica automÃ¡ticamente (sin evento externo)
		 *-----------------------------------------------------------------*/
		case VERIFICAR_CELDA:
			if (celda_es_pista(cuadricula[fila][columna]))
			{
				/* Es pista: volver a introducir fila */
				estado_juego = INTRODUCIR_FILA;
				int_count = 0;
				D8Led_symbol(15);  /* 'F' */
			}
			else
			{
				/* No es pista: pasar a introducir valor */
				estado_juego = INTRODUCIR_VALOR;
				int_count = 0;
			}
			break;
		
		/*------------------------------------------------------------------
		 * ESTADO 4: INTRODUCIR_VALOR
		 * Eventos: INCREMENTAR (incrementa valor), CONFIRMAR (selecciona valor),
		 *          INSERTAR_VALOR (touch directo), BORRAR_VALOR (touch borrar)
		 *-----------------------------------------------------------------*/
		case INTRODUCIR_VALOR:
			if (evento == EVENTO_INCREMENTAR)
			{
				/* Incrementar valor */
				int_count++;
				if (int_count > 9)
				{
					int_count = 0;
				}
				D8Led_symbol(int_count & 0x000f);
			}
			else if (evento == EVENTO_CONFIRMAR)
			{
				/* Confirmar valor e intentar escribir */
				valor = int_count;
				estado_juego = VERIFICAR_VALOR;
				int_count = 0;
			}
			else if (evento == EVENTO_INSERTAR_VALOR)
			{
				/* Touch directo: valor ya estÃ¡ en la variable 'valor' */
				estado_juego = VERIFICAR_VALOR;
			}
			else if (evento == EVENTO_BORRAR_VALOR)
			{
				/* Touch borrar: establecer valor a 0 */
				valor = 0;
				estado_juego = VERIFICAR_VALOR;
			}
			break;
		
		/*------------------------------------------------------------------
		 * ESTADO 5: VERIFICAR_VALOR
		 * Valida si el valor es candidato:
		 * - Si valor=0 â†’ borrar celda â†’ INTRODUCIR_FILA
		 * - Si es candidato â†’ insertar â†’ INTRODUCIR_FILA
		 * - Si NO es candidato â†’ ERROR â†’ INTRODUCIR_FILA
		 *-----------------------------------------------------------------*/
		case VERIFICAR_VALOR:
			/* Guardar valor previo de la celda */
			valor_previo = celda_leer_valor(cuadricula[fila][columna]);
			
			if (valor == 0)
			{
				/* BORRAR VALOR */
			INT8U f, c;
				{
					for (c = 0; c < NUM_COLUMNAS; c++)
					{
						celda_limpiar_error(&cuadricula[f][c]);
					}
				}
				
				/* Borrar el valor */
				celda_poner_valor(&cuadricula[fila][columna], 0);
				celdas_vacias = candidatos_actualizar_all(cuadricula);
				
				/* Actualizar pantalla segÃºn el modo */
				if (Sudoku_Esta_Region_Expandida_Activa())
				{
					/* En modo zoom: solo redibujar la regiÃ³n expandida */
					Sudoku_Redibujar_Region_Expandida();
				}
				else
				{
					/* Modo normal: redibujar tablero completo */
					Sudoku_Actualizar_Tablero_Completo(cuadricula);
				}
				
				/* Volver al estado apropiado */
				if (Sudoku_Esta_Region_Expandida_Activa())
				{
					/* En zoom: quedarse en INTRODUCIR_VALOR */
					estado_juego = INTRODUCIR_VALOR;
					int_count = 0;
					D8Led_symbol(0);
				}
				else
				{
					/* Modo normal: volver a introducir fila */
					estado_juego = INTRODUCIR_FILA;
					int_count = 9;
					D8Led_symbol(15);  /* 'F' */
				}
			} else
			{
				/* INSERTAR VALOR (verificar si es candidato) */
				if (celda_es_candidato(cuadricula[fila][columna], valor))
				{
					/* âœ… ES CANDIDATO: Insertar */
				INT8U f, c;
					{
						for (c = 0; c < NUM_COLUMNAS; c++)
						{
							celda_limpiar_error(&cuadricula[f][c]);
						}
					}
					
					/* Insertar valor */
					celda_poner_valor(&cuadricula[fila][columna], valor);
					
					/* Actualizar candidatos */
					if (valor_previo != 0)
					{
						celdas_vacias = candidatos_actualizar_all(cuadricula);
					}
					else
					{
						candidatos_propagar_arm(cuadricula, fila, columna);
						celdas_vacias--;
					}
					
					/* Verificar si se completÃ³ el Sudoku */
					if (celdas_vacias == 0)
					{
						/* Â¡Sudoku completado! */
						tiempo_final = timer2_count() - tiempo_inicio;
						estado_juego = PARTIDA_TERMINADA;
						D8Led_symbol(0);
					}
					else
					{
						/* Actualizar pantalla segÃºn el modo */
						if (Sudoku_Esta_Region_Expandida_Activa())
						{
							/* En modo zoom: solo redibujar la regiÃ³n expandida */
							Sudoku_Redibujar_Region_Expandida();
						}
						else
						{
							/* Modo normal: redibujar tablero completo */
							Sudoku_Actualizar_Tablero_Completo(cuadricula);
						}
						
						/* Volver al estado apropiado */
						if (Sudoku_Esta_Region_Expandida_Activa())
						{
							/* En zoom: quedarse en INTRODUCIR_VALOR */
							estado_juego = INTRODUCIR_VALOR;
							int_count = 0;
							D8Led_symbol(0);
						}
						else
						{
							/* Modo normal: volver a introducir fila */
							estado_juego = INTRODUCIR_FILA;
							int_count = 9;
							D8Led_symbol(15);  /* 'F' */
						}
					}
				} else
				{
					/* â�Œ NO ES CANDIDATO: Error */
					celda_marcar_error(&cuadricula[fila][columna]);
					D8Led_symbol(14);  /* 'E' de Error */
					
					/* Esperar 2 segundos para visualizar el error */
					Delay(200);
					
					/* Poner el valor incorrecto para visualizarlo */
					celda_poner_valor(&cuadricula[fila][columna], valor);
					
					/* Actualizar candidatos */
					if (valor_previo != 0)
					{
						celdas_vacias = candidatos_actualizar_all(cuadricula);
					}
					else
					{
						candidatos_propagar_arm(cuadricula, fila, columna);
					}
					
					/* Marcar todas las celdas en conflicto */
					marcar_celdas_en_conflicto(fila, columna, valor);
					
					/* Actualizar pantalla segÃºn el modo */
					if (Sudoku_Esta_Region_Expandida_Activa())
					{
						/* En modo zoom: solo redibujar la regiÃ³n expandida */
						Sudoku_Redibujar_Region_Expandida();
					}
					else
					{
						/* Modo normal: redibujar tablero completo */
						Sudoku_Actualizar_Tablero_Completo(cuadricula);
					}
					
					/* Volver al estado apropiado */
					if (Sudoku_Esta_Region_Expandida_Activa())
					{
						/* En zoom: quedarse en INTRODUCIR_VALOR */
						estado_juego = INTRODUCIR_VALOR;
						int_count = 0;
						D8Led_symbol(0);
					}
					else
					{
						/* Modo normal: volver a introducir fila */
						estado_juego = INTRODUCIR_FILA;
						int_count = 9;
						D8Led_symbol(15);  /* 'F' */
					}
				}
			}
			break;
		
		/*------------------------------------------------------------------
		 * ESTADO 7: PARTIDA_TERMINADA
		 * Evento: CONFIRMAR (reinicia el juego)
		 *-----------------------------------------------------------------*/
		case PARTIDA_TERMINADA:
			if (evento == EVENTO_CONFIRMAR)
			{
				if (!pantalla_mostrada)
				{
					/* Primera pulsaciÃ³n: mostrar pantalla final */
					Sudoku_Pantalla_Final(tiempo_final);
					pantalla_mostrada = 1;
				}
				else
				{
					/* Segunda pulsaciÃ³n: reiniciar juego */
					restaurar_cuadricula_original();
					Sudoku_Pantalla_Inicial();
					
					/* Volver al estado inicial */
					estado_juego = ESPERANDO_INICIO;
					int_count = 0;
					pantalla_mostrada = 0;
				}
			}
			break;
		
		default:
			/* Estado desconocido: no hacer nada */
			break;
	}
}


/*=====================================================================================
 * ADAPTADORES: BOTONES FÃ�SICOS â†’ EVENTOS GENÃ‰RICOS
 *====================================================================================*/

/*********************************************************************************************
* name:		Maquina_Procesar_Boton
* func:		Adaptador para botones fÃ­sicos: traduce botones â†’ eventos genÃ©ricos
* para:		boton_id - ID del botÃ³n (EVENTO_BOTON_IZQUIERDO o EVENTO_BOTON_DERECHO)
* ret:		none
* comment:	Esta funciÃ³n es un WRAPPER que adapta la entrada de botones fÃ­sicos
*			al formato genÃ©rico que entiende la mÃ¡quina de estados.
*			Flujo: BotÃ³n â†’ Adaptador â†’ Maquina_Procesar_Evento()
*********************************************************************************************/
void Maquina_Procesar_Boton(INT8U boton_id)
{
	/* Registrar en la cola de depuración */
	cola_depuracion(timer2_count(), boton_id, estado_juego);
	
	/* MODO ZOOM CON CELDA SELECCIONADA: Actuar directamente sobre la celda */
	if (Sudoku_Hay_Celda_Seleccionada() && estado_juego == INTRODUCIR_VALOR)
	{
		int fila_sel, col_sel;
		if (Sudoku_Obtener_Celda_Seleccionada(&fila_sel, &col_sel))
		{
			/* Establecer contexto en las variables globales */
			fila = fila_sel;
			columna = col_sel;
			
			if (boton_id == EVENTO_BOTON_DERECHO)
			{
				/* Incrementar valor */
				int_count++;
				if (int_count > 9)
				{
					int_count = 0;
				}
				D8Led_symbol(int_count & 0x000f);
				
				/* Redibujar solo la región zoom */
				Sudoku_Redibujar_Region_Expandida();
				return;
			}
			else if (boton_id == EVENTO_BOTON_IZQUIERDO)
			{
				/* Confirmar: insertar valor directamente */
				Maquina_Procesar_Touch(EVENTO_INSERTAR_VALOR, fila_sel, col_sel, int_count);
				int_count = 0;
				return;
			}
		}
	}
	
	/* MODO ZOOM SIN CELDA SELECCIONADA: No hacer nada */
	if (Sudoku_Esta_Region_Expandida_Activa() && estado_juego == INTRODUCIR_VALOR)
	{
		/* Usuario debe seleccionar una celda primero con el touch */
		return;
	}
	
	/* MODO NORMAL: Traducir botón físico → evento genérico */
	EventoGenerico evento;
	
	if (boton_id == EVENTO_BOTON_DERECHO)
	{
		/* En estados de pantallas estÃ¡ticas, ambos botones hacen lo mismo */
		if (estado_juego == ESPERANDO_INICIO)
		{
			evento = EVENTO_INICIAR_JUEGO;
		}
		else if (estado_juego == PARTIDA_TERMINADA)
		{
			evento = EVENTO_CONFIRMAR;
		}
		else
		{
			evento = EVENTO_INCREMENTAR;
		}
	}
	else if (boton_id == EVENTO_BOTON_IZQUIERDO)
	{
		if (estado_juego == ESPERANDO_INICIO)
		{
			evento = EVENTO_INICIAR_JUEGO;
		}
		else
		{
			evento = EVENTO_CONFIRMAR;
		}
	}
	else
	{
		return;  /* BotÃ³n desconocido */
	}
	
	/* Delegar al nÃºcleo genÃ©rico de la mÃ¡quina */
	Maquina_Procesar_Evento(evento);
	
	/* Si estamos en VERIFICAR_CELDA, procesarlo automÃ¡ticamente */
	if (estado_juego == VERIFICAR_CELDA)
	{
		Maquina_Procesar_Evento(EVENTO_CONFIRMAR);  /* Auto-procesar verificaciÃ³n */
	}
	
	/* Si estamos en VERIFICAR_VALOR, procesarlo automÃ¡ticamente */
	if (estado_juego == VERIFICAR_VALOR)
	{
		Maquina_Procesar_Evento(EVENTO_CONFIRMAR);  /* Auto-procesar verificaciÃ³n */
	}
}


/*=====================================================================================
 * ADAPTADORES: LCD TOUCHSCREEN â†’ EVENTOS GENÃ‰RICOS
 *====================================================================================*/

/*********************************************************************************************
* name:		Maquina_Procesar_Touch
* func:		Adaptador para touchscreen: traduce acciones tÃ¡ctiles â†’ eventos genÃ©ricos
* para:		accion - tipo de acciÃ³n (EVENTO_INSERTAR_VALOR o EVENTO_BORRAR_VALOR)
*			fila_param, col_param - posiciÃ³n de la celda (0-8)
*			valor_param - valor a insertar (1-9), ignorado si accion=BORRAR
* ret:		none
* comment:	Esta funciÃ³n es un WRAPPER que adapta la entrada del touchscreen
*			al formato genÃ©rico que entiende la mÃ¡quina de estados.
*			Flujo: Touch â†’ Validar â†’ Adaptador â†’ Maquina_Procesar_Evento()
*********************************************************************************************/
void Maquina_Procesar_Touch(EventoGenerico accion, int fila_param, int col_param, int valor_param)
{
	/* En modo zoom, permitir insertar/borrar siempre si hay celda seleccionada */
	/* En modo normal, solo permitir en estado INTRODUCIR_VALOR */
	if (!Sudoku_Esta_Region_Expandida_Activa())
	{
		/* Modo normal: verificar estado */
		if (estado_juego != INTRODUCIR_VALOR)
			return;
	}
	
	/* Verificar que es una celda válida y no es pista */
	if (fila_param >= NUM_FILAS || col_param >= NUM_FILAS || 
	    celda_es_pista(cuadricula[fila_param][col_param]))
		return;
	
	/* Verificar que la acciÃ³n es vÃ¡lida */
	if (accion != EVENTO_INSERTAR_VALOR && accion != EVENTO_BORRAR_VALOR)
		return;
	
	/* Guardar contexto para la mÃ¡quina de estados */
	fila = fila_param;
	columna = col_param;
	valor = (accion == EVENTO_INSERTAR_VALOR) ? valor_param : 0;
	
	/* Delegar al nÃºcleo genÃ©rico de la mÃ¡quina */
	Maquina_Procesar_Evento(accion);
}



