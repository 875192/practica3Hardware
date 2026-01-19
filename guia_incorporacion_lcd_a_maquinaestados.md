# Guía: Incorporación del LCD a la Máquina de Estados

## 🎯 Objetivo de la Integración

Unificar la lógica del touchscreen LCD con la máquina de estados existente del juego Sudoku, de modo que las interacciones táctiles sigan el mismo flujo de estados que los botones físicos. Esto permite:

- Centralizar toda la lógica del juego en un solo lugar
- Mantener sincronizado el 8LED con las acciones táctiles
- Facilitar el mantenimiento y depuración del código
- Garantizar consistencia entre diferentes métodos de entrada

---

## 📁 Archivos Modificados

### 1. `button.h` - API Pública

Se añadieron nuevas funciones para permitir la interacción desde el módulo LCD:

```c
/*--- Funciones para integración con LCD touchscreen ---*/
void Sudoku_Cambiar_Estado(int nuevo_estado);
int Sudoku_Obtener_Estado(void);
void Sudoku_Insertar_Valor_Touch(int fila, int col, int valor);
void Sudoku_Borrar_Valor_Touch(int fila, int col);
```

**Propósito:**
- `Sudoku_Cambiar_Estado()`: Permite al LCD cambiar el estado del juego y actualizar automáticamente el 8LED
- `Sudoku_Obtener_Estado()`: Consulta el estado actual de la máquina de estados
- `Sudoku_Insertar_Valor_Touch()`: Maneja la inserción de valores desde el touchscreen con toda la lógica de validación
- `Sudoku_Borrar_Valor_Touch()`: Maneja el borrado de valores desde el touchscreen

---

### 2. `button.c` - Lógica de Estados del Juego

#### **Variables Añadidas**

```c
/*--- Variables para interacción con touchscreen ---*/
static volatile uint8_t celda_fila_touch = 0;  /* Fila seleccionada por touchscreen */
static volatile uint8_t celda_col_touch = 0;   /* Columna seleccionada por touchscreen */
```

---

#### **Función: `Sudoku_Cambiar_Estado()`**

**Funcionalidad:**
- Cambia el estado de la máquina de estados
- Actualiza automáticamente el 8LED según el nuevo estado
- Resetea contadores cuando es necesario

**Código:**
```c
void Sudoku_Cambiar_Estado(int nuevo_estado)
{
	estado_juego = (EstadoSudoku)nuevo_estado;
	
	/* Actualizar 8LED según el nuevo estado */
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
			D8Led_symbol(0);  /* Mostrar 0 para introducir valor */
			int_count = 0;
			break;
			
		case BORRAR_VALOR:
			D8Led_symbol(0);  /* Mostrar 0 al borrar */
			break;
			
		case VERIFICAR_VALOR:
		case VERIFICAR_CELDA:
			/* No cambiar el 8LED durante verificación */
			break;
			
		default:
			/* Otros estados no cambian el 8LED */
			break;
	}
}
```

**Mapeo Estado → 8LED:**
| Estado | Símbolo 8LED | Descripción |
|--------|--------------|-------------|
| INTRODUCIR_FILA | F | Seleccionando fila |
| INTRODUCIR_COLUMNA | C | Seleccionando columna |
| INTRODUCIR_VALOR | 0 | Listo para valor |
| BORRAR_VALOR | 0 | Borrando valor |
| Error detectado | E | Error en validación |

---

#### **Función: `Sudoku_Insertar_Valor_Touch()`**

**Propósito:**  
Maneja la inserción de un valor en una celda desde el touchscreen, incluyendo toda la validación y actualización de candidatos.

**Flujo de Ejecución:**

```
1. Verificar que el estado sea INTRODUCIR_VALOR
2. Verificar que la celda no sea una pista
3. Guardar valor previo de la celda
4. Transicionar a VERIFICAR_VALOR
5. Limpiar todos los errores previos
6. Verificar si el valor es candidato válido:

   ┌─── SI ES VÁLIDO ────────────────────────────┐
   │                                              │
   │ • Poner el valor en la celda                │
   │ • Actualizar candidatos:                    │
   │   - Si había valor previo → recalcular todo │
   │   - Si celda vacía → propagar               │
   │ • Verificar si completó el Sudoku:          │
   │   - Sí → PARTIDA_TERMINADA                  │
   │   - No → INTRODUCIR_VALOR (8LED: 0)         │
   │                                              │
   └──────────────────────────────────────────────┘

   ┌─── SI NO ES VÁLIDO (ERROR) ─────────────────┐
   │                                              │
   │ • Marcar error en la celda                  │
   │ • Poner el valor incorrecto (para visual)   │
   │ • Actualizar candidatos                     │
   │ • Marcar TODAS las celdas en conflicto:     │
   │   - Buscar en la misma fila                 │
   │   - Buscar en la misma columna              │
   │   - Buscar en la misma región 3x3           │
   │ • Volver a INTRODUCIR_VALOR (8LED: E)       │
   │                                              │
   └──────────────────────────────────────────────┘
```

**Características clave:**
- Valida que el número sea candidato antes de insertarlo
- Maneja correctamente la actualización de candidatos (propagar vs recalcular)
- Detecta y marca todos los conflictos en fila/columna/región
- Detecta automáticamente la finalización del Sudoku
- Actualiza el 8LED según el resultado (0 si éxito, E si error)

---

#### **Función: `Sudoku_Borrar_Valor_Touch()`**

**Propósito:**  
Maneja el borrado de un valor de una celda desde el touchscreen.

**Flujo de Ejecución:**

```
1. Verificar que estamos en INTRODUCIR_VALOR o BORRAR_VALOR
2. Verificar que la celda no sea una pista
3. Transicionar temporalmente a BORRAR_VALOR
4. Limpiar todos los errores del tablero
5. Borrar el valor de la celda (poner 0)
6. Recalcular TODOS los candidatos del tablero
7. Volver a INTRODUCIR_VALOR
8. Actualizar 8LED (mostrar 0)
```

**Por qué recalcular todo:**  
Al borrar un valor, ese número vuelve a ser candidato en todas las celdas de su fila, columna y región. Por eso es necesario recalcular todos los candidatos, no solo propagar.

---

### 3. `lcd.c` - Integración con Touchscreen

#### **Inclusión de button.h**

```c
#include "button.h"  /* Para integración con máquina de estados */
```

Esto permite acceder a las funciones de gestión de estados desde el módulo LCD.

---

#### **Función Modificada: `Sudoku_Mostrar_Region_Expandida()`**

**Cambio realizado:**

```c
void Sudoku_Mostrar_Region_Expandida(int region_fila, int region_col)
{
	/* Guardar estado de región expandida */
	g_region_expandida_activa = 1;
	g_region_fila_actual = region_fila;
	g_region_col_actual = region_col;
	g_celda_seleccionada_i = -1;
	g_celda_seleccionada_j = -1;
	
	/* INTEGRACIÓN CON MÁQUINA DE ESTADOS */
	/* Al hacer zoom en una región, cambiar al estado INTRODUCIR_VALOR */
	if (Sudoku_Juego_En_Progreso())
	{
		Sudoku_Cambiar_Estado(4);  /* 4 = INTRODUCIR_VALOR (ver eventos.h) */
	}
	
	/* Dibujar interfaz */
	Sudoku_Redibujar_Region_Expandida();
	// ...
}
```

**Comportamiento:**
- Al hacer ZOOM táctil en una región 3x3 del tablero
- Se cambia automáticamente al estado `INTRODUCIR_VALOR`
- El 8LED se actualiza a "0" (gracias a `Sudoku_Cambiar_Estado`)
- El usuario puede inmediatamente empezar a introducir valores

---

#### **Función Modificada: `Sudoku_Procesar_Touch_Region_Expandida()`**

**Cambios en el manejo de clics de números:**

**ANTES (código duplicado):**
```c
// Toda la lógica de validación, propagación de candidatos,
// detección de errores, etc. estaba duplicada aquí
if (celda_es_candidato(...)) {
    celda_poner_valor(...);
    // ... 50+ líneas de lógica duplicada
}
```

**DESPUÉS (delegación a button.c):**
```c
/* Calcular posición real en el tablero */
int fila = g_region_fila_actual * 3 + g_celda_seleccionada_i;
int col = g_region_col_actual * 3 + g_celda_seleccionada_j;

/* Verificar que no sea una pista */
if (!celda_es_pista(cuadricula[fila][col]))
{
	/* INTEGRACIÓN CON MÁQUINA DE ESTADOS */
	/* Usar la función de button.c que maneja estados */
	Sudoku_Insertar_Valor_Touch(fila, col, numero);
	
	/* Redibujar */
	Sudoku_Redibujar_Region_Expandida();
}
```

**Cambios en el manejo del botón BORRAR:**

**ANTES:**
```c
// Lógica de borrado duplicada
celda_poner_valor(&cuadricula[fila][col], 0);
celdas_vacias = candidatos_actualizar_all(cuadricula);
// ...
```

**DESPUÉS:**
```c
/* Calcular posición real en el tablero */
int fila = g_region_fila_actual * 3 + g_celda_seleccionada_i;
int col = g_region_col_actual * 3 + g_celda_seleccionada_j;

/* Verificar que no sea una pista antes de borrar */
if (!celda_es_pista(cuadricula[fila][col]))
{
	/* INTEGRACIÓN CON MÁQUINA DE ESTADOS */
	/* Usar la función de button.c que maneja estados */
	Sudoku_Borrar_Valor_Touch(fila, col);
	
	/* Redibujar */
	Sudoku_Redibujar_Region_Expandida();
}
```

**Ventaja:**  
Se eliminaron ~100 líneas de código duplicado. Toda la lógica está centralizada en `button.c`.

---

#### **Función Modificada: `Sudoku_Cerrar_Region_Expandida()`**

**Cambio realizado:**

```c
void Sudoku_Cerrar_Region_Expandida(void)
{
	extern CELDA (*cuadricula)[NUM_COLUMNAS];
	
	g_region_expandida_activa = 0;
	g_celda_seleccionada_i = -1;
	g_celda_seleccionada_j = -1;
	
	/* INTEGRACIÓN CON MÁQUINA DE ESTADOS */
	/* Al volver del zoom, regresar al estado INTRODUCIR_FILA */
	if (Sudoku_Juego_En_Progreso())
	{
		Sudoku_Cambiar_Estado(1);  /* 1 = INTRODUCIR_FILA (ver eventos.h) */
	}
	
	/* Redibujar tablero completo */
	Sudoku_Dibujar_Tablero();
	Sudoku_Actualizar_Tablero_Completo(cuadricula);
}
```

**Comportamiento:**
- Al presionar el botón "VOLVER" en la región expandida
- Se cierra la vista expandida
- Se cambia automáticamente al estado `INTRODUCIR_FILA`
- El 8LED se actualiza a "F"
- El usuario vuelve al flujo normal de selección fila/columna

---

## 🔄 Diagrama de Flujo Unificado

```
┌──────────────────────┐
│  ESPERANDO_INICIO    │
│                      │
└──────────┬───────────┘
           │ (toque en pantalla o botón físico)
           ↓
┌──────────────────────┐
│  INTRODUCIR_FILA     │
│  [8LED: F]           │
└──────────┬───────────┘
           │ (confirma fila con botón o touchscreen)
           ↓
┌──────────────────────┐
│ INTRODUCIR_COLUMNA   │
│  [8LED: C]           │
└──────────┬───────────┘
           │ (confirma columna)
           ↓
┌──────────────────────┐
│  VERIFICAR_CELDA     │
│  (¿es pista?)        │
└──────────┬───────────┘
           │ (no es pista)
           │ (ZOOM TÁCTIL en región 3x3)
           ↓
┌──────────────────────┐
│  INTRODUCIR_VALOR    │◄──────────────┐
│  [8LED: 0]           │               │
└──────────┬───────────┘               │
           │ (toca número en teclado)  │
           ↓                           │
┌──────────────────────┐               │
│  VERIFICAR_VALOR     │               │
│  (¿es candidato?)    │               │
└──────┬───────┬───────┘               │
       │       │                       │
   VÁLIDO   INVÁLIDO                   │
       │       │                       │
       │       └─→ [Marca error]       │
       │           [8LED: E] ──────────┤
       │                               │
       ├─→ [Pone valor]                │
       │                               │
       └─→ ¿Completó? ─┬─→ NO ─────────┤
                       │               
                       └─→ SÍ          
                           ↓           
                   ┌───────────────┐  
                   │   PARTIDA     │  
                   │  TERMINADA    │  
                   └───────────────┘  

┌──────────────────────┐
│  INTRODUCIR_VALOR    │
│  [8LED: 0]           │
└──────────┬───────────┘
           │ (toca "Borrar")
           ↓
┌──────────────────────┐
│   BORRAR_VALOR       │
│  [Valor = 0]         │
│  [Recalcula todo]    │
└──────────┬───────────┘
           │
           ↓
┌──────────────────────┐
│  INTRODUCIR_VALOR    │
│  [8LED: 0]           │
└──────────┬───────────┘
           │ (botón "Volver")
           ↓
┌──────────────────────┐
│  INTRODUCIR_FILA     │
│  [8LED: F]           │
└──────────────────────┘
```

---

## 📊 Tabla de Transiciones de Estado

| Acción del Usuario | Estado Actual | Estado Siguiente | 8LED | Función Responsable |
|-------------------|---------------|------------------|------|---------------------|
| Inicio del juego | ESPERANDO_INICIO | INTRODUCIR_FILA | F | `boton_confirmado()` |
| Zoom en región | INTRODUCIR_FILA/COLUMNA | INTRODUCIR_VALOR | 0 | `Sudoku_Mostrar_Region_Expandida()` |
| Clic en número válido | INTRODUCIR_VALOR | INTRODUCIR_VALOR | 0 | `Sudoku_Insertar_Valor_Touch()` |
| Clic en número inválido | INTRODUCIR_VALOR | INTRODUCIR_VALOR | E | `Sudoku_Insertar_Valor_Touch()` |
| Clic en "Borrar" | INTRODUCIR_VALOR | INTRODUCIR_VALOR | 0 | `Sudoku_Borrar_Valor_Touch()` |
| Clic en "Volver" | INTRODUCIR_VALOR | INTRODUCIR_FILA | F | `Sudoku_Cerrar_Region_Expandida()` |
| Completa Sudoku | VERIFICAR_VALOR | PARTIDA_TERMINADA | - | `Sudoku_Insertar_Valor_Touch()` |

---

## ✅ Ventajas de la Integración

### 1. **Lógica Centralizada**
- Toda la gestión de estados y validaciones está en `button.c`
- No hay código duplicado entre botones físicos y touchscreen
- Un solo punto de mantenimiento

### 2. **8LED Sincronizado**
- Se actualiza automáticamente en cada cambio de estado
- El usuario siempre sabe en qué estado está el juego
- Consistente entre diferentes métodos de entrada

### 3. **Consistencia**
- Botones físicos y touchscreen siguen exactamente el mismo flujo
- Las mismas validaciones se aplican en ambos casos
- Comportamiento predecible para el usuario

### 4. **Validación Unificada**
- La verificación de candidatos está en un solo lugar
- Detección de errores centralizada
- Marcado de conflictos consistente

### 5. **Trazabilidad**
- Se puede usar `cola_depuracion()` para rastrear cambios de estado desde touchscreen
- Facilita la depuración de problemas
- Historia completa de interacciones del usuario

### 6. **Mantenibilidad**
- Cambios en la lógica del juego solo requieren modificar `button.c`
- El módulo LCD se enfoca solo en la interfaz gráfica
- Separación clara de responsabilidades

### 7. **Escalabilidad**
- Fácil añadir nuevos estados o transiciones
- Nuevos métodos de entrada pueden integrarse fácilmente
- Arquitectura extensible

---

## 🎮 Comportamiento del 8LED

### Mapeo Completo Estado → Símbolo

| Estado | Símbolo 8LED | Valor | Descripción |
|--------|--------------|-------|-------------|
| ESPERANDO_INICIO | - | - | Pantalla inicial |
| INTRODUCIR_FILA | F | 15 | Seleccionando fila (1-9) |
| INTRODUCIR_COLUMNA | C | 12 | Seleccionando columna (1-9) |
| VERIFICAR_CELDA | (número) | 0-9 | Mostrando última entrada |
| INTRODUCIR_VALOR | 0 | 0 | Listo para introducir valor |
| VERIFICAR_VALOR | (sin cambio) | - | Validando candidato |
| BORRAR_VALOR | 0 | 0 | Borrando valor |
| Error detectado | E | 14 | Error en validación |
| PARTIDA_TERMINADA | - | - | Sudoku completado |

### Secuencia Visual Típica

```
Inicio:    [F] → selecciona fila 5 → [5]
Confirma:  [C] → selecciona columna 3 → [3]
Zoom:      [0] → toca número 7 válido → [0]
Error:     [0] → toca número 7 inválido → [E]
Borrar:    [E] → toca "Borrar" → [0]
Volver:    [0] → toca "Volver" → [F]
```

---

## 🔧 Consideraciones de Implementación

### Estados en `eventos.h`

Los valores numéricos de los estados son:
```c
typedef enum {
    ESPERANDO_INICIO,       // 0
    INTRODUCIR_FILA,        // 1
    INTRODUCIR_COLUMNA,     // 2
    VERIFICAR_CELDA,        // 3
    INTRODUCIR_VALOR,       // 4
    VERIFICAR_VALOR,        // 5
    BORRAR_VALOR,           // 6
    PARTIDA_TERMINADA,      // 7
} EstadoSudoku;
```

**Importante:** Al llamar `Sudoku_Cambiar_Estado()` desde LCD, usar los valores numéricos correspondientes.

### Actualización de Candidatos

**Propagar vs Recalcular:**

- **Propagar** (`candidatos_propagar_arm`):
  - Se usa cuando se inserta un valor en una celda VACÍA
  - Más eficiente: solo actualiza fila/columna/región afectada
  - No borra candidatos de otras celdas innecesariamente

- **Recalcular** (`candidatos_actualizar_all`):
  - Se usa cuando se MODIFICA un valor existente o se BORRA
  - Menos eficiente pero necesario: reconstruye todos los candidatos
  - Garantiza consistencia total del tablero

### Detección de Finalización

```c
/* Verificar si se completó el Sudoku */
if (celdas_vacias == 0)
{
    /* ¡Sudoku completado! */
    tiempo_final = timer2_count();
    estado_juego = PARTIDA_TERMINADA;
}
```

La variable `celdas_vacias` se actualiza automáticamente en:
- `candidatos_propagar_arm()` → decrementa en 1
- `candidatos_actualizar_all()` → recalcula el total

---

## 📝 Ejemplo de Uso Completo

### Escenario: Jugador usa touchscreen para insertar valores

```
1. [Usuario toca el tablero en región superior-izquierda]
   → Sudoku_Mostrar_Region_Expandida(0, 0)
   → Sudoku_Cambiar_Estado(4)  // INTRODUCIR_VALOR
   → 8LED muestra: 0

2. [Usuario toca la celda (1,1) de la región]
   → g_celda_seleccionada_i = 1
   → g_celda_seleccionada_j = 1
   → Se redibuja con borde resaltado

3. [Usuario toca el número 7 en el teclado virtual]
   → Sudoku_Procesar_Touch_Region_Expandida(x, y)
   → Sudoku_Insertar_Valor_Touch(1, 1, 7)
   
   Dentro de Sudoku_Insertar_Valor_Touch:
   → Verifica estado = INTRODUCIR_VALOR ✓
   → Verifica que celda no sea pista ✓
   → Transiciona a VERIFICAR_VALOR
   → Verifica si 7 es candidato en (1,1)
   
   Si es VÁLIDO:
   → celda_poner_valor(&cuadricula[1][1], 7)
   → candidatos_propagar_arm(cuadricula, 1, 1)
   → celdas_vacias--
   → estado_juego = INTRODUCIR_VALOR
   → D8Led_symbol(0)
   → Sudoku_Redibujar_Region_Expandida()
   → 8LED muestra: 0
   
   Si es INVÁLIDO:
   → celda_marcar_error(&cuadricula[1][1])
   → Marca todas las celdas en conflicto
   → estado_juego = INTRODUCIR_VALOR
   → D8Led_symbol(14)
   → Sudoku_Redibujar_Region_Expandida()
   → 8LED muestra: E

4. [Usuario toca "Borrar"]
   → Sudoku_Borrar_Valor_Touch(1, 1)
   → celda_poner_valor(&cuadricula[1][1], 0)
   → candidatos_actualizar_all(cuadricula)
   → estado_juego = INTRODUCIR_VALOR
   → D8Led_symbol(0)
   → 8LED muestra: 0

5. [Usuario toca "Volver"]
   → Sudoku_Cerrar_Region_Expandida()
   → Sudoku_Cambiar_Estado(1)  // INTRODUCIR_FILA
   → Sudoku_Dibujar_Tablero()
   → 8LED muestra: F
```

---

## 🐛 Depuración y Verificación

### Puntos de Control

Para verificar que la integración funciona correctamente:

1. **Verificar cambios de estado:**
   ```c
   cola_depuracion(timer2_count(), evento_id, estado_juego);
   ```

2. **Verificar actualización del 8LED:**
   - Al hacer zoom → debe mostrar "0"
   - Al volver → debe mostrar "F"
   - Al error → debe mostrar "E"

3. **Verificar validación de candidatos:**
   - Insertar número válido → debe aceptarse
   - Insertar número inválido → debe marcarse error
   - Borrar → debe recalcular candidatos

4. **Verificar actualización de celdas_vacias:**
   - Insertar → decrementa
   - Borrar → incrementa
   - Al llegar a 0 → PARTIDA_TERMINADA

### Errores Comunes

1. **8LED no se actualiza:**
   - Verificar que se llama a `Sudoku_Cambiar_Estado()` y no se modifica `estado_juego` directamente

2. **Candidatos incorrectos:**
   - Verificar que se usa `candidatos_actualizar_all()` al borrar o modificar
   - Verificar que se usa `candidatos_propagar_arm()` al insertar en celda vacía

3. **No detecta finalización:**
   - Verificar que `celdas_vacias` se actualiza correctamente
   - Verificar que se comprueba `celdas_vacias == 0` tras cada inserción

---

## 📚 Referencias

### Archivos Relacionados

- `eventos.h` - Definición de estados (`EstadoSudoku`)
- `button.c` - Lógica de máquina de estados
- `button.h` - API pública de estados
- `lcd.c` - Interfaz touchscreen
- `lcd.h` - API pública de LCD
- `8led.c` - Control del display 8LED
- `sudoku_2025.c` - Lógica del Sudoku (candidatos, validación)

### Funciones Clave

**Gestión de Estados:**
- `Sudoku_Cambiar_Estado()` - Cambiar estado y actualizar 8LED
- `Sudoku_Obtener_Estado()` - Consultar estado actual
- `Sudoku_Juego_En_Progreso()` - Verificar si hay partida activa

**Interacción Touchscreen:**
- `Sudoku_Insertar_Valor_Touch()` - Insertar valor desde touch
- `Sudoku_Borrar_Valor_Touch()` - Borrar valor desde touch
- `Sudoku_Mostrar_Region_Expandida()` - Zoom en región
- `Sudoku_Cerrar_Region_Expandida()` - Volver a tablero completo

**Actualización de Candidatos:**
- `candidatos_propagar_arm()` - Propagar un valor nuevo
- `candidatos_actualizar_all()` - Recalcular todos los candidatos

---

## ✨ Conclusión

La integración del LCD con la máquina de estados ha resultado en:

- **Código más limpio y mantenible** (eliminadas ~100 líneas de duplicación)
- **Comportamiento consistente** entre botones y touchscreen
- **8LED sincronizado** con todas las interacciones
- **Arquitectura escalable** para futuras extensiones
- **Depuración facilitada** con estados bien definidos

Esta arquitectura sigue el principio de **separación de responsabilidades**:
- `button.c` → Lógica del juego y estados
- `lcd.c` → Interfaz gráfica y detección de toques
- `sudoku_2025.c` → Algoritmos de Sudoku

Cada módulo tiene una responsabilidad clara, lo que facilita el mantenimiento y la comprensión del código.

---

**Fecha de creación:** 19 de enero de 2026  
**Versión:** 1.0
