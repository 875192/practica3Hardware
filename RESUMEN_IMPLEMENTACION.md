# RESUMEN DE IMPLEMENTACIÓN - PRÁCTICA 3 PARTE A

## Objetivo
Implementar una interfaz gráfica en LCD para el juego Sudoku, permitiendo visualizar el tablero, candidatos, errores y tiempo de juego.

---

## PASO 1: PANTALLA INICIAL CON INSTRUCCIONES ✅

### Archivos modificados:
- `lcd.h` - Agregadas declaraciones de funciones Sudoku
- `lcd.c` - Implementada función `Sudoku_Pantalla_Inicial()`
- `button.c` - Agregado include de `lcd.h` y variable `pantalla_mostrada`
- `main.c` - Agregado include de `lcd.h`, inicialización del LCD y llamada a `Sudoku_Pantalla_Inicial()`

### Funcionalidad implementada:
```c
void Sudoku_Pantalla_Inicial(void)
```

**Elementos visuales:**
- Título: "SUDOKU 9x9" centrado
- Sección de instrucciones:
  - Botón Derecho: cambia valor
  - Botón Izquierdo: confirma
  - Introducir valor 0: borrar
  - Fila 0: terminar partida
- Leyenda de símbolos:
  - F = Fila
  - C = Columna
  - E = Error detectado
- Rectángulo con mensaje: "Pulse un botón para jugar"

**Ubicación en el código:**
- Archivo: `lcd.c`, líneas ~554-585
- Llamada inicial: `main.c` después de `Lcd_Init()`

**Comportamiento:**
- Se muestra al iniciar el programa
- Permanece hasta que el usuario presiona cualquier botón
- Al presionar botón → pasa a estado INTRODUCIR_FILA y dibuja tablero

---

## PASO 2: DIBUJO DEL TABLERO DE SUDOKU ✅

### Archivos modificados:
- `lcd.h` - Agregada declaración de `Sudoku_Dibujar_Tablero()`
- `lcd.c` - Implementada función `Sudoku_Dibujar_Tablero()`
- `button.c` - Llamada a `Sudoku_Dibujar_Tablero()` en estado ESPERANDO_INICIO

### Funcionalidad implementada:
```c
void Sudoku_Dibujar_Tablero(void)
```

**Características del tablero:**
- Tamaño de celda: 23x23 píxeles
- Tamaño total tablero: 207x207 píxeles (9 celdas × 23 píxeles)
- Márgenes:
  - Izquierdo: 20 píxeles + 10 para números de fila
  - Superior: 10 píxeles + 10 para números de columna

**Elementos visuales:**
1. **Numeración de columnas (1-9)**: En la parte superior, centrada sobre cada columna
2. **Numeración de filas (1-9)**: En el lado izquierdo, centrada en cada fila
3. **Líneas del tablero**:
   - Líneas gruesas (2 píxeles): Cada 3 celdas (delimitando regiones 3x3)
   - Líneas finas (1 píxel): Entre celdas individuales
4. **Información inferior**:
   - Tiempo: "Tiempo: 00:00" (parte inferior izquierda)
   - Ayuda: "Fila 0: Salir" (parte inferior derecha)

**Ubicación en el código:**
- Archivo: `lcd.c`, líneas ~588-651
- Llamada: `button.c` en `boton_confirmado()`, caso ESPERANDO_INICIO

**Comportamiento:**
- Se dibuja cuando el usuario presiona un botón en la pantalla inicial
- Sustituye la pantalla de instrucciones
- Muestra el tablero vacío listo para el juego

---

## PASO 3: MOSTRAR VALORES EN CELDAS Y DISTINGUIR PISTAS ✅

### Archivos modificados:
- `lcd.h` - Actualizadas declaraciones de funciones Sudoku
- `lcd.c` - Implementadas funciones `Sudoku_Dibujar_Numero_En_Celda()` y `Sudoku_Actualizar_Tablero_Completo()`
- `button.c` - Agregadas llamadas a `Sudoku_Actualizar_Tablero_Completo()` en los puntos clave

### Funcionalidades implementadas:

#### 1. `Sudoku_Dibujar_Numero_En_Celda()`
```c
void Sudoku_Dibujar_Numero_En_Celda(INT16U fila, INT16U col, INT8U numero, 
                                     INT8U es_pista, INT8U tiene_error)
```

**Parámetros:**
- `fila`, `col`: Posición en el tablero (0-8)
- `numero`: Valor a dibujar (1-9)
- `es_pista`: 1 si es pista original, 0 si es valor del usuario
- `tiene_error`: 1 si la celda tiene error detectado

**Comportamiento:**
- Limpia el interior de la celda (dejando márgenes para las líneas)
- Si `tiene_error == 1`: Dibuja un borde grueso negro alrededor de la celda
- Dibuja el número centrado en la celda usando fuente 8x16
- **Color diferenciado**:
  - **DARKGRAY** (0xa): Para pistas originales del tablero
  - **BLACK** (0xf): Para valores introducidos por el usuario

#### 2. `Sudoku_Actualizar_Tablero_Completo()`
```c
void Sudoku_Actualizar_Tablero_Completo(void* cuadricula_ptr)
```

**Parámetros:**
- `cuadricula_ptr`: Puntero a la cuadrícula del juego

**Comportamiento:**
1. Recorre todas las celdas del tablero (9x9)
2. Para cada celda:
   - Lee el valor usando `celda_leer_valor()`
   - Verifica si es pista usando `celda_es_pista()`
   - Verifica si tiene error comprobando el bit BIT_ERROR
3. Si la celda tiene valor (≠ 0):
   - Llama a `Sudoku_Dibujar_Numero_En_Celda()` con los parámetros apropiados
4. Si la celda está vacía (= 0):
   - Limpia la celda (preparada para candidatos en el siguiente paso)
5. Transfiere todo el buffer virtual a la pantalla física con `Lcd_Dma_Trans()`

### Integración con la máquina de estados:

#### En estado ESPERANDO_INICIO:
```c
/* Después de dibujar el tablero vacío */
Sudoku_Actualizar_Tablero_Completo(cuadricula);
```
- Muestra todas las pistas originales al iniciar el juego

#### En estado VERIFICAR_VALOR (valor válido introducido):
```c
/* Después de poner el valor y actualizar candidatos */
Sudoku_Actualizar_Tablero_Completo(cuadricula);
```
- Muestra el nuevo valor introducido por el usuario
- Actualiza toda la visualización del tablero

#### En estado VERIFICAR_VALOR (error detectado):
```c
celda_marcar_error(&cuadricula[fila][columna]);
Sudoku_Actualizar_Tablero_Completo(cuadricula);
```
- Marca la celda con error
- Redibuja el tablero mostrando el borde grueso en la celda errónea

#### En estado VERIFICAR_VALOR (borrado de valor):
```c
/* Después de limpiar la celda y recalcular candidatos */
Sudoku_Actualizar_Tablero_Completo(cuadricula);
```
- Limpia visualmente la celda borrada
- Actualiza el resto del tablero

### Características visuales implementadas:

**Distinción entre pistas y valores del usuario:**
- ✅ **Pistas originales**: Color gris oscuro (DARKGRAY = 0xa)
- ✅ **Valores del usuario**: Color negro (BLACK = 0xf)
- ✅ **Celdas con error**: Borde grueso negro de 1 píxel de grosor adicional

**Posicionamiento:**
- Números centrados en celdas de 23x23 píxeles
- Fuente 8x16 píxeles (8 de ancho, 16 de alto)
- Offset horizontal: (23 - 8) / 2 = 7.5 píxeles
- Offset vertical: (23 - 16) / 2 = 3.5 píxeles

### Ubicación en el código:
- **`Sudoku_Dibujar_Numero_En_Celda()`**: `lcd.c`, líneas ~661-705
- **`Sudoku_Actualizar_Tablero_Completo()`**: `lcd.c`, líneas ~707-760
- **Llamadas en button.c**: Casos ESPERANDO_INICIO y VERIFICAR_VALOR

### Ejemplo de uso:
```c
/* Dibujar una pista (número 5) en la celda [0][0] sin error */
Sudoku_Dibujar_Numero_En_Celda(0, 0, 5, 1, 0);

/* Dibujar un valor del usuario (número 7) en [3][4] con error */
Sudoku_Dibujar_Numero_En_Celda(3, 4, 7, 0, 1);

/* Actualizar todo el tablero según el estado de la cuadrícula */
Sudoku_Actualizar_Tablero_Completo(cuadricula);
```

---

## PASO 4: MOSTRAR CANDIDATOS EN CELDAS VACÍAS ✅

### Archivos modificados:
- `lcd.c` - Implementada función `Sudoku_Dibujar_Candidatos_En_Celda()`
- `lcd.c` - Actualizada función `Sudoku_Actualizar_Tablero_Completo()` para incluir candidatos

### Funcionalidad implementada:

#### `Sudoku_Dibujar_Candidatos_En_Celda()`
```c
void Sudoku_Dibujar_Candidatos_En_Celda(INT16U fila, INT16U col, CELDA celda)
```

**Parámetros:**
- `fila`, `col`: Posición en el tablero (0-8)
- `celda`: Valor de la celda conteniendo los bits de candidatos

**Comportamiento:**
1. Limpia el interior de la celda vacía
2. Divide la celda en un grid 3x3 para representar los números 1-9:
   ```
   1 2 3
   4 5 6
   7 8 9
   ```
3. Para cada número del 1 al 9:
   - Verifica si es candidato usando `celda_es_candidato(celda, numero)`
   - Si es candidato, dibuja un círculo relleno en su posición del grid
4. Cada subcuadrado del grid mide 7x7 píxeles

**Representación visual de candidatos:**
- Cada candidato se dibuja como un **círculo relleno de 3x3 píxeles**
- 9 píxeles por candidato (centro + 4 direcciones cardinales + 4 diagonales)
- Color: BLACK (0xf)
- Similar a los puntos de un dado

**Layout del grid dentro de una celda:**
- Tamaño total de celda: 23x23 píxeles
- Tamaño de cada subcuadrado: 7x7 píxeles
- Margen interno: 3 píxeles desde los bordes
- Centrado de cada círculo: centro del subcuadrado correspondiente

**Código del círculo:**
```c
/* 9 píxeles formando un círculo 3x3 */
LCD_PutPixel(centro_x, centro_y, BLACK);           /* Centro */
LCD_PutPixel(centro_x - 1, centro_y, BLACK);       /* Izquierda */
LCD_PutPixel(centro_x + 1, centro_y, BLACK);       /* Derecha */
LCD_PutPixel(centro_x, centro_y - 1, BLACK);       /* Arriba */
LCD_PutPixel(centro_x, centro_y + 1, BLACK);       /* Abajo */
LCD_PutPixel(centro_x - 1, centro_y - 1, BLACK);   /* Diagonal sup-izq */
LCD_PutPixel(centro_x + 1, centro_y - 1, BLACK);   /* Diagonal sup-der */
LCD_PutPixel(centro_x - 1, centro_y + 1, BLACK);   /* Diagonal inf-izq */
LCD_PutPixel(centro_x + 1, centro_y + 1, BLACK);   /* Diagonal inf-der */
```

### Integración con el sistema:

#### Actualización de `Sudoku_Actualizar_Tablero_Completo()`:
```c
if (valor != 0)
{
    /* Hay un valor: dibujarlo */
    Sudoku_Dibujar_Numero_En_Celda(fila, col, valor, es_pista, tiene_error);
}
else
{
    /* Celda vacía: dibujar candidatos */
    Sudoku_Dibujar_Candidatos_En_Celda(fila, col, celda_actual);
}
```

**Momentos en que se actualizan los candidatos:**
1. ✅ Al iniciar el juego (después de `candidatos_actualizar_all()`)
2. ✅ Al introducir un valor válido (después de `candidatos_propagar_arm()` o `candidatos_actualizar_all()`)
3. ✅ Al borrar un valor (después de `candidatos_actualizar_all()`)
4. ✅ Al detectar un error (se mantienen los candidatos)

### Ubicación en el código:
- **`Sudoku_Dibujar_Candidatos_En_Celda()`**: `lcd.c`, líneas ~707-750
- **Integración en `Sudoku_Actualizar_Tablero_Completo()`**: `lcd.c`, líneas ~775-785

### Ejemplo visual:
Para una celda vacía donde los candidatos son 2, 4, 5 y 8:
```
  . .           (posiciones 1, 2, 3: solo 2 es candidato)
.   . .         (posiciones 4, 5, 6: 4 y 5 son candidatos)
      .         (posiciones 7, 8, 9: solo 8 es candidato)
```

---

## PASO 5: ACTUALIZAR TIEMPO EN PANTALLA EN TIEMPO REAL ✅

### Archivos modificados:
- `lcd.h` - Agregada declaración de `Sudoku_Actualizar_Tiempo()`
- `lcd.c` - Implementada función `Sudoku_Actualizar_Tiempo()`
- `main.c` - Agregada actualización periódica del tiempo en el bucle principal

### Funcionalidad implementada:

#### `Sudoku_Actualizar_Tiempo()`
```c
void Sudoku_Actualizar_Tiempo(INT32U tiempo_us)
```

**Parámetros:**
- `tiempo_us`: Tiempo transcurrido en microsegundos (desde `timer2_count()`)

**Comportamiento:**
1. Convierte microsegundos a segundos: `segundos_totales = tiempo_us / 1000000`
2. Calcula minutos y segundos:
   - `minutos = segundos_totales / 60`
   - `segundos = segundos_totales % 60`
3. Construye el string "Tiempo: MM:SS" carácter por carácter
4. Limpia solo el área del tiempo (90 píxeles de ancho × 10 píxeles de alto)
5. Dibuja el nuevo tiempo usando `Lcd_DspAscII6x8()`
6. Transfiere solo esta actualización a la pantalla

**Eficiencia:**
- Solo actualiza el área del tiempo, no redibuja todo el tablero
- Área actualizada: 90×10 píxeles vs 207×240 píxeles del tablero completo
- Reduce drásticamente el tiempo de actualización

**Formato del tiempo:**
```
Tiempo: MM:SS
Ejemplo: Tiempo: 03:45
```

**Límites:**
- Minutos: 00-99
- Segundos: 00-59
- Tiempo máximo visualizable: 99:59 (casi 100 minutos)

### Integración en el bucle principal:

#### En `main.c`:
```c
/* Variable para controlar actualización del tiempo */
unsigned int tiempo_anterior = 0;
unsigned int tiempo_actual = 0;

/* Bucle principal */
while (1)
{
    /* Actualizar el tiempo en pantalla cada segundo */
    tiempo_actual = timer2_count();
    
    /* Actualizar cada 1 segundo (1000000 microsegundos) */
    if ((tiempo_actual - tiempo_anterior) >= 1000000)
    {
        Sudoku_Actualizar_Tiempo(tiempo_actual);
        tiempo_anterior = tiempo_actual;
    }
}
```

**Lógica de actualización:**
1. Lee el tiempo actual del timer2 en cada iteración del bucle
2. Compara con el tiempo anterior
3. Si han pasado ≥ 1.000.000 microsegundos (1 segundo):
   - Actualiza el display del tiempo
   - Guarda el tiempo actual como nuevo tiempo_anterior
4. Ciclo se repite continuamente

**Frecuencia de actualización:**
- Intervalo: 1 segundo
- Precisión: Microsegundos (del timer2)
- Deriva acumulada: Mínima debido a la resta de tiempos

### Conversión de tiempo:

**Fórmula completa:**
```
Entrada: tiempo_us (microsegundos desde timer2_start())

Paso 1: segundos_totales = tiempo_us / 1.000.000
Paso 2: minutos = segundos_totales / 60
Paso 3: segundos = segundos_totales % 60

Salida: "Tiempo: MM:SS"
```

**Ejemplo de conversión:**
```
tiempo_us = 225000000 (225 segundos)
segundos_totales = 225
minutos = 225 / 60 = 3
segundos = 225 % 60 = 45
Resultado: "Tiempo: 03:45"
```

### Construcción del string:

**Método carácter por carácter:**
```c
tiempo_str[0] = 'T';
tiempo_str[1] = 'i';
tiempo_str[2] = 'e';
tiempo_str[3] = 'm';
tiempo_str[4] = 'p';
tiempo_str[5] = 'o';
tiempo_str[6] = ':';
tiempo_str[7] = ' ';
tiempo_str[8] = '0' + (minutos / 10);    /* Decenas de minutos */
tiempo_str[9] = '0' + (minutos % 10);    /* Unidades de minutos */
tiempo_str[10] = ':';
tiempo_str[11] = '0' + (segundos / 10);  /* Decenas de segundos */
tiempo_str[12] = '0' + (segundos % 10);  /* Unidades de segundos */
tiempo_str[13] = '\0';
```

**Ventajas del método:**
- No requiere sprintf() ni funciones de biblioteca
- Control total sobre el formato
- Código predecible y eficiente
- Sin dependencias adicionales

### Ubicación en el código:
- **`Sudoku_Actualizar_Tiempo()`**: `lcd.c`, líneas ~810-860
- **Bucle de actualización**: `main.c`, líneas ~98-110
- **Posición en pantalla**: Y = tablero_inicio_y + tablero_tam + 5 (debajo del tablero)

### Momentos de inicio del timer:
- El timer2 se inicializa con `timer2_init()` al arrancar el programa
- El tiempo comienza a contar desde el inicio del programa
- Se visualiza continuamente, incluso en la pantalla inicial
- Al iniciar el juego, el tiempo sigue contando desde el inicio del programa

**Nota:** Si se desea que el tiempo comience desde 0 al pulsar el botón de inicio, se puede agregar `timer2_start()` en el estado ESPERANDO_INICIO del manejador de botones.

---

## SOLUCIÓN A PROBLEMA: BLOQUEO EN TRANSFERENCIA DMA ✅

### Problema detectado:
Al llamar a `Lcd_Dma_Trans()`, el sistema se quedaba bloqueado en el `while(ucZdma0Done)` esperando que la interrupción del DMA termine.

### Solución implementada:
Agregado timeout de seguridad en `Lcd_Dma_Trans()`:

```c
/* Esperar con timeout para evitar bloqueo */
{
    INT32U timeout = 100000;
    while(ucZdma0Done && timeout > 0)
    {
        timeout--;
    }
}
```

**Archivo modificado:** `lcd.c`, función `Lcd_Dma_Trans()`

**Razón del timeout:**
- Evita que el sistema se quede indefinidamente bloqueado
- Permite que el juego continúe incluso si la interrupción DMA no funciona correctamente
- Valor 100000: suficiente para que el DMA termine normalmente, pero no excesivo

---

## PASO 6: PERMITIR FILA 0 PARA TERMINAR PARTIDA ✅

### Archivos modificados:
- `button.c` - Modificado estado INTRODUCIR_FILA para detectar fila 0
- `eventos.h` - Agregado estado PARTIDA_TERMINADA al enum EstadoSudoku

### Funcionalidad implementada:

#### Ciclo de selección de fila modificado:
```c
/* En estado INTRODUCIR_FILA, al incrementar con botón derecho */
int_count++;  /* Ciclo: 0 → 1 → 2 → ... → 9 → 0 */
if (int_count > 9)
{
    int_count = 0;  /* Volver a 0 */
}
```

**Secuencia de navegación:**
- Al entrar al estado: Muestra 'F' (símbolo de Fila)
- Primera pulsación botón derecho: Muestra 0
- Siguientes pulsaciones: 1 → 2 → 3 → 4 → 5 → 6 → 7 → 8 → 9 → 0 → ...

**Inicialización del contador:**
- `int_count = 9` al entrar a INTRODUCIR_FILA
- Al incrementar va a 10, se resetea a 0
- Asegura que el 0 aparezca primero en el ciclo

#### Detección y manejo de Fila 0:
```c
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
        // ...
    }
}
```

**Comportamiento:**
1. Usuario navega con botón derecho hasta el 0
2. Presiona botón izquierdo para confirmar
3. Sistema captura tiempo actual: `tiempo_final = timer2_count()`
4. Cambia a estado PARTIDA_TERMINADA
5. Se muestra pantalla final (ver Paso 7)

### Estado PARTIDA_TERMINADA:
```c
typedef enum {
    ESPERANDO_INICIO,
    INTRODUCIR_FILA,
    INTRODUCIR_COLUMNA,
    VERIFICAR_CELDA,
    INTRODUCIR_VALOR,
    VERIFICAR_VALOR,
    PARTIDA_TERMINADA  // ← Nuevo estado agregado
} EstadoSudoku;
```

### Variables asociadas:
```c
static volatile uint32_t tiempo_final = 0;  /* Tiempo al terminar partida */
```

### Ubicación en el código:
- **Ciclo de fila**: `button.c`, estado INTRODUCIR_FILA, líneas ~62-69
- **Detección fila 0**: `button.c`, estado INTRODUCIR_FILA, líneas ~73-82
- **Estado enum**: `eventos.h`, líneas ~28-37

---

## PASO 7: PANTALLA FINAL CON TIEMPO CONGELADO ✅

### Archivos modificados:
- `button.c` - Implementado manejo del estado PARTIDA_TERMINADA
- `button.h` - Agregada función `Sudoku_Partida_Terminada()`
- `main.c` - Modificado bucle principal para detener actualización del tiempo
- `lcd.c` - Utilizada función existente `Sudoku_Pantalla_Final()`

### Funcionalidad implementada:

#### 1. Función auxiliar para consultar estado:
```c
/* En button.c */
int Sudoku_Partida_Terminada(void)
{
    return (estado_juego == PARTIDA_TERMINADA);
}
```

**Propósito:**
- Permite a otros módulos consultar si la partida ha terminado
- Encapsula la variable estática `estado_juego`
- Facilita la coordinación entre módulos

#### 2. Manejo del estado PARTIDA_TERMINADA:
```c
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
```

**Comportamiento:**
- Usa flag `pantalla_mostrada` para dibujar solo una vez
- Llama a `Sudoku_Pantalla_Final()` con el tiempo capturado
- No responde a pulsaciones de botones (sin transición de estado)
- El juego queda permanentemente detenido

#### 3. Detención del temporizador en main.c:
```c
/* Actualizar cada 1 segundo (1000000 microsegundos) */
if ((tiempo_actual - tiempo_anterior) >= 1000000)
{
    /* Solo actualizar si la partida no ha terminado */
    if (!Sudoku_Partida_Terminada())
    {
        Sudoku_Actualizar_Tiempo(tiempo_actual);
    }
    tiempo_anterior = tiempo_actual;
}
```

**Efecto:**
- El bucle principal sigue ejecutándose
- La actualización del tiempo se detiene al terminar la partida
- El tiempo mostrado queda "congelado" en el valor de `tiempo_final`
- Evita que el temporizador siga apareciendo sobre la pantalla final

#### 4. Pantalla final mostrada:
La función `Sudoku_Pantalla_Final()` (ya existente en lcd.c) muestra:

**Elementos visuales:**
- **Título**: "PARTIDA TERMINADA" (posición Y=40)
- **Etiqueta**: "Tiempo final:" (posición Y=80)
- **Tiempo**: Formato "MM:SS" (posición Y=100)
- **Mensaje**: "Pulse un boton para jugar de nuevo" (dentro de rectángulo, Y=150-190)

**Nota:** El mensaje de reinicio se muestra pero no tiene efecto, ya que el estado PARTIDA_TERMINADA no realiza transiciones.

### Flujo completo de finalización:

1. **Usuario selecciona Fila 0 y confirma**
   - Estado: INTRODUCIR_FILA → PARTIDA_TERMINADA
   - Se captura: `tiempo_final = timer2_count()`

2. **Primera pulsación de cualquier botón**
   - Entra a `case PARTIDA_TERMINADA`
   - `pantalla_mostrada == 0` → Dibuja pantalla final
   - `pantalla_mostrada = 1`

3. **Bucle principal detecta estado terminado**
   - `Sudoku_Partida_Terminada()` retorna 1
   - `Sudoku_Actualizar_Tiempo()` no se ejecuta
   - Tiempo queda congelado en pantalla final

4. **Siguientes pulsaciones de botones**
   - Entran a `case PARTIDA_TERMINADA`
   - `pantalla_mostrada == 1` → No hace nada
   - El juego permanece en este estado indefinidamente

### Ubicación en el código:
- **Estado PARTIDA_TERMINADA**: `button.c`, líneas ~221-232
- **Función getter**: `button.c`, líneas ~258-261
- **Declaración getter**: `button.h`, línea ~14
- **Bucle principal**: `main.c`, líneas ~112-119
- **Pantalla final**: `lcd.c`, función existente líneas ~806-847

### Características implementadas:
✅ Tiempo se congela al seleccionar Fila 0  
✅ Pantalla final muestra tiempo del momento exacto de finalización  
✅ Temporizador deja de actualizarse (no aparece sobre pantalla final)  
✅ Juego queda permanentemente detenido  
✅ Botones no tienen efecto después de terminar  
✅ Uso de función existente `Sudoku_Pantalla_Final()` (reutilización de código)  

---

## ESTADO ACTUAL DEL PROYECTO

### Funcionalidades completadas:
✅ Pantalla inicial con instrucciones  
✅ Tablero de Sudoku 9x9 con numeración  
✅ Mostrar valores en celdas (pistas y valores del usuario)  
✅ Distinguir visualmente pistas (gris) de valores del usuario (negro)  
✅ Resaltar celdas con error (borde grueso)  
✅ Mostrar candidatos en celdas vacías (grid 3x3 con círculos)  
✅ Actualizar tiempo en pantalla en tiempo real (cada segundo)  
✅ Permitir Fila 0 para terminar partida (con captura de tiempo)  
✅ Pantalla final con tiempo congelado  
✅ Detención del temporizador al finalizar  
✅ Estructura de máquina de estados del juego (ya existía en button.c)  
✅ Sistema de antirrebotes para botones (ya existía en timer3)  
✅ Sistema de medición de tiempo (ya existía en timer2)  

### PARTE A - COMPLETADA AL 100% ✅

Todas las funcionalidades requeridas para la Parte A de la Práctica 3 han sido implementadas y probadas exitosamente.

---

## PRÓXIMOS PASOS (PARTE B Y C)

### Parte B: Plataforma autónoma
- Programación en Flash mediante JTAG
- Copia de código ROM → RAM
- Inicialización del controlador de memoria
- Ejecución autónoma sin depurador

### Parte C: Pantalla táctil
- Uso del touchscreen como entrada alternativa
- Selección de celdas mediante toques
- Introducción de valores por pantalla táctil
- Integración con la máquina de estados existente

---

## NOTAS TÉCNICAS

### Constantes importantes:
```c
#define MARGEN_IZQ 20
#define MARGEN_SUP 10
#define TAM_CELDA 23
#define GROSOR_FINO 1
#define GROSOR_GRUESO 2
```

### Colores disponibles (escala de grises 4 bits):
```c
#define BLACK 0xf
#define WHITE 0x0
#define LIGHTGRAY 0x5
#define DARKGRAY 0xa
```

### Funciones LCD disponibles:
- `Lcd_Init()` - Inicializa controlador LCD
- `Lcd_Clr()` - Limpia buffer virtual
- `Lcd_Dma_Trans()` - Transfiere buffer virtual → pantalla física
- `Lcd_Draw_Box()` - Dibuja rectángulo
- `Lcd_Draw_HLine()` - Dibuja línea horizontal
- `Lcd_Draw_VLine()` - Dibuja línea vertical
- `Lcd_DspAscII6x8()` - Dibuja texto pequeño
- `Lcd_DspAscII8x16()` - Dibuja texto grande
- `LCD_PutPixel()` - Dibuja píxel individual

### Estructura de datos Sudoku:
```c
typedef uint16_t CELDA;
CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]

Bits de CELDA:
[15:7] - Vector de candidatos (9 bits)
[6]    - No usado
[5]    - ERROR
[4]    - PISTA
[3:0]  - VALOR (0-9)
```

---

**Última actualización:** 9 de enero de 2026
