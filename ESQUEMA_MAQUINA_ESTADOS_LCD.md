# Esquema: Máquina de Estados con Integración LCD

## 📊 Diagrama de Estados Unificado

```
                    ┌─────────────────────────────────────────┐
                    │        ESPERANDO_INICIO                 │
                    │                                         │
                    │  - Pantalla de instrucciones           │
                    │  - 8LED: apagado                       │
                    └─────────────┬───────────────────────────┘
                                  │
                    ┌─────────────┴─────────────────┐
                    │  Entrada: Botón físico        │
                    │           o Toque pantalla    │
                    └─────────────┬─────────────────┘
                                  ↓
                    ┌─────────────────────────────────────────┐
                    │       INTRODUCIR_FILA                   │
                    │                                         │
                    │  - Usuario selecciona fila (0-9)       │
                    │  - 8LED: muestra "F", luego número     │
                    │  - Botón derecho: incrementa fila      │
                    │  - Botón izquierdo: confirma           │
                    │  - Fila 0 → termina partida            │
                    └─────────────┬───────────────────────────┘
                                  │ (confirma fila 1-9)
                                  ↓
                    ┌─────────────────────────────────────────┐
                    │      INTRODUCIR_COLUMNA                 │
                    │                                         │
                    │  - Usuario selecciona columna (1-9)    │
                    │  - 8LED: muestra "C", luego número     │
                    │  - Botón derecho: incrementa columna   │
                    │  - Botón izquierdo: confirma           │
                    └─────────────┬───────────────────────────┘
                                  │ (confirma columna)
                                  ↓
                    ┌─────────────────────────────────────────┐
                    │        VERIFICAR_CELDA                  │
                    │                                         │
                    │  - Comprueba si la celda es pista      │
                    │  - 8LED: muestra número de columna     │
                    └──────┬─────────────────┬────────────────┘
                           │                 │
                    ¿Es pista?              ¿No es pista?
                           │                 │
                           ↓                 ↓
               ┌───────────────────┐   ┌──────────────────────────┐
               │ Volver a          │   │   INTRODUCIR_VALOR       │
               │ INTRODUCIR_FILA   │   │                          │
               │ 8LED: "F"         │   │ - 8LED: muestra "0"      │
               └───────────────────┘   │ - Botón derecho: +valor  │
                                       │ - Botón izquierdo: conf. │
                                       │ - Valor 0: borrar        │
                                       └────────┬─────────────────┘
                                                │
                        ┌───────────────────────┼───────────────────────┐
                        │                       │                       │
                   (Borra valor)           (Inserta valor)        (Zoom táctil)
                        │                       │                       │
                        ↓                       ↓                       ↓
           ┌──────────────────────┐  ┌─────────────────┐   ┌──────────────────────┐
           │    BORRAR_VALOR      │  │ VERIFICAR_VALOR │   │ Región expandida 3x3  │
           │                      │  │                 │   │                       │
           │ - Pone valor 0       │  │ - Verifica si   │   │ Estado: INTRODUCIR    │
           │ - Recalcula todos    │  │   es candidato  │   │         _VALOR        │
           │ - 8LED: "0"          │  │                 │   │                       │
           │ - Vuelve a           │  └──┬──────────┬───┘   │ - Teclado virtual     │
           │   INTRODUCIR_FILA    │     │          │       │ - 8LED: "0"           │
           └──────────────────────┘     │          │       │ - Toca número         │
                                        │          │       │ - Toca "Borrar"       │
                                   ¿Válido?   ¿Inválido?  │ - Toca "Volver"       │
                                        │          │       └────────┬──────────────┘
                                        ↓          ↓                │
                         ┌──────────────────────────────┐           │
                         │  INSERCIÓN EXITOSA           │           │
                         │                              │      (Botón Volver)
                         │ - Pone valor en celda       │           │
                         │ - Actualiza candidatos      │           ↓
                         │ - 8LED: "0"                 │  ┌────────────────────┐
                         │ - Vuelve a INTRODUCIR_FILA  │  │ Vuelve a          │
                         │                              │  │ INTRODUCIR_FILA   │
                         │ Si completa Sudoku:         │  │ 8LED: "F"         │
                         │   → PARTIDA_TERMINADA       │  │ Redibuja tablero  │
                         └──────────────────────────────┘  └────────────────────┘
                                                │
                         ┌──────────────────────────────┐
                         │  INSERCIÓN CON ERROR         │
                         │                              │
                         │ - Marca celda con error     │
                         │ - Marca celdas conflicto    │
                         │ - 8LED: "E" (2 segundos)    │
                         │ - Vuelve a INTRODUCIR_FILA  │
                         └──────────────────────────────┘
```

---

## 🎮 Métodos de Entrada y Estados

### 1️⃣ **Botones Físicos** (Flujo Original)

| Estado | Botón Derecho | Botón Izquierdo | 8LED |
|--------|---------------|-----------------|------|
| ESPERANDO_INICIO | Inicia juego | Inicia juego | - |
| INTRODUCIR_FILA | Incrementa fila (0→9→0) | Confirma fila | F → número |
| INTRODUCIR_COLUMNA | Incrementa columna (1→9→1) | Confirma columna | C → número |
| INTRODUCIR_VALOR | Incrementa valor (0→9→0) | Confirma valor | número |

**Características:**
- Control secuencial: Fila → Columna → Valor
- Valor 0 = Borrar
- Fila 0 = Terminar partida
- Visualización en tablero completo

---

### 2️⃣ **Touchscreen LCD** (Nuevo Flujo)

#### **Modo Tablero Completo**

```
┌──────────────────────────────┐
│    Tablero Sudoku 9x9        │
│                              │
│  ┌───┬───┬───┐ ┌───┬───┬───┐│
│  │ 5 │   │ 3 │ │   │ 7 │   ││
│  ├───┼───┼───┤ ├───┼───┼───┤│
│  │   │ 9 │   │ │   │   │ 1 ││  ← Toca región
│  ├───┼───┼───┤ ├───┼───┼───┤│     para zoom
│  │ 1 │   │   │ │ 8 │   │ 5 ││
│  └───┴───┴───┘ └───┴───┴───┘│
│                              │
│     [Tiempo: 00:45]          │
└──────────────────────────────┘

Estado actual: INTRODUCIR_FILA
8LED: "F"
```

**Acción:** Usuario toca una región 3x3

↓

```
Transición:
  - Estado cambia a: INTRODUCIR_VALOR
  - 8LED cambia a: "0"
  - Pantalla: muestra región expandida
```

---

#### **Modo Región Expandida**

```
┌─────────────────────────────────────────┐
│  Región 3x3 Expandida                   │
│                                         │
│  ┌────────┬────────┬────────┐          │
│  │   5    │        │   3    │          │
│  │        │        │        │          │
│  ├────────┼────────┼────────┤          │
│  │        │   9    │        │  ← Toca │
│  │        │ [SEL] │        │    celda │
│  ├────────┼────────┼────────┤          │
│  │   1    │        │        │          │
│  │        │        │        │          │
│  └────────┴────────┴────────┘          │
│                                         │
│           Teclado Virtual               │
│         ┌───┬───┬───┐                  │
│         │ 1 │ 2 │ 3 │                  │
│         ├───┼───┼───┤                  │
│         │ 4 │ 5 │ 6 │  ← Toca número  │
│         ├───┼───┼───┤                  │
│         │ 7 │ 8 │ 9 │                  │
│         └───┴───┴───┘                  │
│         ┌───────────┐                  │
│         │  BORRAR   │  ← Borrar valor │
│         └───────────┘                  │
│         ┌───────────┐                  │
│         │  VOLVER   │  ← Salir zoom   │
│         └───────────┘                  │
└─────────────────────────────────────────┘

Estado actual: INTRODUCIR_VALOR
8LED: "0"
```

**Acciones Posibles:**

1. **Toca celda vacía** → Selecciona celda (borde resaltado)

2. **Toca número (1-9):**
   ```
   - Llama a: Sudoku_Insertar_Valor_Touch(fila, col, numero)
   - Transición: INTRODUCIR_VALOR → VERIFICAR_VALOR → INTRODUCIR_VALOR
   - Si válido: 8LED = "0"
   - Si error: 8LED = "E"
   - Redibuja región expandida
   ```

3. **Toca "BORRAR":**
   ```
   - Llama a: Sudoku_Borrar_Valor_Touch(fila, col)
   - Transición: INTRODUCIR_VALOR → BORRAR_VALOR → INTRODUCIR_VALOR
   - 8LED = "0"
   - Redibuja región expandida
   ```

4. **Toca "VOLVER":**
   ```
   - Llama a: Sudoku_Cerrar_Region_Expandida()
   - Transición: INTRODUCIR_VALOR → INTRODUCIR_FILA
   - 8LED = "F"
   - Redibuja tablero completo
   ```

---

## 🔄 Transiciones de Estado Unificadas

### **Tabla de Transiciones**

| Estado Actual | Evento | Acción | Estado Siguiente | 8LED |
|--------------|--------|--------|------------------|------|
| ESPERANDO_INICIO | Botón/Toque | Inicia juego | INTRODUCIR_FILA | F |
| INTRODUCIR_FILA | Confirma fila 0 | Termina | PARTIDA_TERMINADA | - |
| INTRODUCIR_FILA | Confirma fila 1-9 | Valida | INTRODUCIR_COLUMNA | C |
| INTRODUCIR_COLUMNA | Confirma columna | Valida | VERIFICAR_CELDA | número |
| VERIFICAR_CELDA | Es pista | Rechaza | INTRODUCIR_FILA | F |
| VERIFICAR_CELDA | No es pista | Acepta | INTRODUCIR_VALOR | 0 |
| **ZOOM TÁCTIL** | **Toca región** | **Expande** | **INTRODUCIR_VALOR** | **0** |
| INTRODUCIR_VALOR | Inserta válido | Acepta | INTRODUCIR_FILA | 0 |
| INTRODUCIR_VALOR | Inserta inválido | Error | INTRODUCIR_FILA | E |
| INTRODUCIR_VALOR | Borra valor | Limpia | INTRODUCIR_FILA | 0 |
| **INTRODUCIR_VALOR (zoom)** | **Toca número** | **Valida** | **INTRODUCIR_VALOR** | **0/E** |
| **INTRODUCIR_VALOR (zoom)** | **Toca Borrar** | **Limpia** | **INTRODUCIR_VALOR** | **0** |
| **INTRODUCIR_VALOR (zoom)** | **Toca Volver** | **Cierra** | **INTRODUCIR_FILA** | **F** |
| VERIFICAR_VALOR | Completa Sudoku | Finaliza | PARTIDA_TERMINADA | - |

---

## 🧩 Integración: Cómo Afecta el LCD a la Máquina de Estados

### **Antes de la Integración** ❌

```
┌──────────────────┐         ┌──────────────────┐
│                  │         │                  │
│   BOTONES        │         │   TOUCHSCREEN    │
│   (button.c)     │         │   (lcd.c)        │
│                  │         │                  │
│ - Estados        │         │ - Lógica propia  │
│ - Validación     │         │ - Validación     │
│ - 8LED           │  ✗      │ - Sin 8LED       │
│ - Actualiza LCD  │         │ - Código duplic. │
│                  │         │                  │
└──────────────────┘         └──────────────────┘
     ↓                            ↓
  Máquina de estados        Lógica independiente
  en button.c               duplicada en lcd.c
```

**Problemas:**
- Lógica duplicada (~100 líneas)
- 8LED no sincronizado con touchscreen
- Inconsistencias posibles
- Difícil mantenimiento

---

### **Después de la Integración** ✅

```
┌────────────────────────────────────────────────┐
│         MÁQUINA DE ESTADOS CENTRAL             │
│              (button.c)                        │
│                                                │
│  • EstadoSudoku estado_juego                  │
│  • Gestión completa de estados                │
│  • Validación de candidatos                   │
│  • Control del 8LED                           │
│  • API pública para LCD                       │
│                                                │
│  Funciones Exportadas:                        │
│  ├─ Sudoku_Cambiar_Estado()                   │
│  ├─ Sudoku_Insertar_Valor_Touch()             │
│  └─ Sudoku_Borrar_Valor_Touch()               │
│                                                │
└────────────┬───────────────────────────────────┘
             │
     ┌───────┴────────┐
     │                │
     ↓                ↓
┌──────────┐    ┌──────────┐
│ BOTONES  │    │ LCD      │
│          │    │          │
│ Llama    │    │ Llama    │
│ callback │    │ funciones│
│ interno  │    │ públicas │
└──────────┘    └──────────┘
     │                │
     └────────┬───────┘
              ↓
     [Mismo flujo de estados]
     [Mismo control de 8LED]
     [Misma validación]
```

**Ventajas:**
- ✅ Lógica centralizada
- ✅ 8LED sincronizado
- ✅ Sin código duplicado
- ✅ Fácil mantenimiento
- ✅ Comportamiento consistente

---

## 🎯 Puntos Clave de la Integración

### 1. **Función Central: `Sudoku_Cambiar_Estado()`**

```c
void Sudoku_Cambiar_Estado(int nuevo_estado)
{
    estado_juego = nuevo_estado;
    
    // Actualizar 8LED automáticamente
    switch (estado_juego) {
        case INTRODUCIR_FILA:    D8Led_symbol(15); break; // F
        case INTRODUCIR_COLUMNA: D8Led_symbol(12); break; // C
        case INTRODUCIR_VALOR:   D8Led_symbol(0);  break; // 0
        // ...
    }
}
```

**Uso desde LCD:**
```c
// Al hacer zoom en región
Sudoku_Cambiar_Estado(4);  // INTRODUCIR_VALOR
// → Cambia estado + actualiza 8LED a "0" automáticamente

// Al volver del zoom
Sudoku_Cambiar_Estado(1);  // INTRODUCIR_FILA  
// → Cambia estado + actualiza 8LED a "F" automáticamente
```

---

### 2. **Inserción de Valores desde Touchscreen**

```c
// En lcd.c - Al tocar número en teclado virtual
int numero = 7;  // Usuario tocó el 7
int fila = 3, col = 5;

// Delegación a máquina de estados
Sudoku_Insertar_Valor_Touch(fila, col, numero);
// ↓
// button.c ejecuta:
//   1. Verifica estado = INTRODUCIR_VALOR
//   2. Valida si es candidato
//   3. Actualiza cuadrícula
//   4. Actualiza candidatos
//   5. Detecta errores/conflictos
//   6. Verifica si completó Sudoku
//   7. Actualiza 8LED (0 o E)
//   8. Transiciona estados
```

---

### 3. **Sincronización del 8LED**

| Acción | Método Entrada | Estado Resultante | 8LED |
|--------|----------------|-------------------|------|
| Inicio juego | Botón/Touch | INTRODUCIR_FILA | F |
| **Zoom en región** | **Touch** | **INTRODUCIR_VALOR** | **0** |
| Inserta número válido | Botón/Touch | INTRODUCIR_FILA / INTRODUCIR_VALOR | 0 |
| Inserta número inválido | Botón/Touch | INTRODUCIR_FILA / INTRODUCIR_VALOR | E |
| Borra valor | Botón/Touch | INTRODUCIR_FILA / INTRODUCIR_VALOR | 0 |
| **Volver de zoom** | **Touch** | **INTRODUCIR_FILA** | **F** |
| Completa Sudoku | Botón/Touch | PARTIDA_TERMINADA | - |

**Clave:** El 8LED **siempre** refleja el estado actual, sin importar el método de entrada.

---

## 📦 Arquitectura de Módulos

```
┌─────────────────────────────────────────────────────────┐
│                    main.c                               │
│  • Bucle principal                                      │
│  • Lee touchscreen: ts_read_calibrated()               │
│  • Actualiza tiempo en pantalla                        │
└─────────────────┬───────────────────────────────────────┘
                  │
         ┌────────┴─────────┐
         ↓                  ↓
┌──────────────┐    ┌──────────────┐
│  button.c    │    │    lcd.c     │
│              │    │              │
│ RESPONSABLE: │    │ RESPONSABLE: │
│ • Estados    │←───│ • Interfaz   │
│ • Validación │    │ • Touchscreen│
│ • 8LED       │    │ • Dibujo     │
│ • Lógica     │    │              │
└──────┬───────┘    └──────┬───────┘
       │                   │
       └────────┬──────────┘
                ↓
     ┌────────────────────┐
     │   sudoku_2025.c    │
     │                    │
     │ • candidatos       │
     │ • validaciones     │
     │ • algoritmos       │
     └────────────────────┘
```

---

## 🎬 Ejemplo de Flujo Completo

### **Escenario: Usuario juega con touchscreen**

```
1. INICIO
   ├─ Estado: ESPERANDO_INICIO
   ├─ Usuario: toca pantalla
   ├─ button.c: recibe evento
   └─ Transición: INTRODUCIR_FILA (8LED: F)

2. MODO BOTONES
   ├─ Estado: INTRODUCIR_FILA
   ├─ Usuario: botón derecho (incrementa) → fila = 5
   ├─ Usuario: botón izquierdo (confirma)
   └─ Transición: INTRODUCIR_COLUMNA (8LED: C)
   
   ├─ Estado: INTRODUCIR_COLUMNA  
   ├─ Usuario: botón derecho → columna = 3
   ├─ Usuario: botón izquierdo (confirma)
   └─ Transición: VERIFICAR_CELDA → INTRODUCIR_VALOR

3. USUARIO DECIDE USAR TOUCHSCREEN (ZOOM)
   ├─ Estado: INTRODUCIR_VALOR (con botones)
   ├─ Usuario: toca región 3x3 en pantalla
   ├─ lcd.c: Sudoku_Mostrar_Region_Expandida(region_fila, region_col)
   │    └─ Llama: Sudoku_Cambiar_Estado(4) // INTRODUCIR_VALOR
   └─ Resultado: Vista zoom + 8LED: "0"

4. MODO TOUCHSCREEN (ZOOM ACTIVO)
   ├─ Estado: INTRODUCIR_VALOR (zoom)
   ├─ 8LED: "0"
   ├─ Usuario: toca celda (selecciona)
   ├─ Usuario: toca número "7"
   ├─ lcd.c: Sudoku_Insertar_Valor_Touch(fila, col, 7)
   │    └─ button.c:
   │         ├─ Verifica es candidato: ✓ SÍ
   │         ├─ celda_poner_valor(..., 7)
   │         ├─ candidatos_propagar_arm(...)
   │         ├─ celdas_vacias--
   │         ├─ 8LED: "0" (éxito)
   │         └─ Permanece en: INTRODUCIR_VALOR
   └─ Resultado: Valor insertado, región redibujada

5. CONTINUAR EN ZOOM
   ├─ Usuario: selecciona otra celda
   ├─ Usuario: toca número "3"
   ├─ button.c: Verifica candidato: ✗ NO (conflicto)
   │    ├─ Marca error en celda
   │    ├─ Marca celdas en conflicto
   │    ├─ 8LED: "E" (error)
   │    └─ Permanece en: INTRODUCIR_VALOR
   └─ Resultado: Error mostrado, región redibujada con errores

6. BORRAR VALOR INCORRECTO
   ├─ Usuario: toca "BORRAR"
   ├─ lcd.c: Sudoku_Borrar_Valor_Touch(fila, col)
   │    └─ button.c:
   │         ├─ celda_poner_valor(..., 0)
   │         ├─ candidatos_actualizar_all(...)
   │         ├─ celdas_vacias++
   │         ├─ 8LED: "0"
   │         └─ Permanece en: INTRODUCIR_VALOR
   └─ Resultado: Valor borrado, errores limpiados

7. SALIR DEL ZOOM
   ├─ Usuario: toca "VOLVER"
   ├─ lcd.c: Sudoku_Cerrar_Region_Expandida()
   │    └─ Llama: Sudoku_Cambiar_Estado(1) // INTRODUCIR_FILA
   └─ Transición: INTRODUCIR_FILA (8LED: F)
       └─ Tablero completo visible

8. VOLVER A MODO BOTONES
   ├─ Estado: INTRODUCIR_FILA
   ├─ 8LED: "F"
   └─ Usuario puede continuar con botones físicos
```

---

## 🔑 Conclusión

La integración del LCD con la máquina de estados logra:

✅ **Unificación**: Un solo flujo de estados para todos los métodos de entrada  
✅ **Sincronización**: 8LED siempre refleja el estado actual  
✅ **Flexibilidad**: Usuario puede alternar entre botones y touchscreen libremente  
✅ **Consistencia**: Misma validación y comportamiento en ambos modos  
✅ **Mantenibilidad**: Lógica centralizada en un solo módulo  

El LCD ya no es un sistema paralelo, sino una **extensión natural** de la máquina de estados existente.
