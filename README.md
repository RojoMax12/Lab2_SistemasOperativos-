# Laboratorio 2: Pipeline de Detección de Círculos mediante Transformada de Hough y Morfología Matemática

## Sistemas Operativos - Primer Semestre 2026

### 1. Descripción del Proyecto

Este programa implementa una herramienta en lenguaje C para el procesamiento de imágenes binarias (`.bin`). El objetivo principal es la detección de centros de círculos mediante la Transformada de Hough, previa limpieza de la imagen con operaciones de Morfología Matemática (apertura: erosión seguida de dilatación).

A diferencia del Laboratorio 1, este proyecto implementa un **pipeline multiproceso modular** en el cual cada etapa del procesamiento se ejecuta en un proceso independiente creado dinámicamente (`fork()`, `execv()`) y comunicado secuencialmente mediante tuberías IPC (`pipe()`, `dup2()`).

---

### 2. Estructura de Nodos y Comunicación IPC

El pipeline consta de los siguientes nodos independientes:

1. **`lab2`**: Nodo orquestador principal. Procesa banderas por consola (`getopt()`), valida la corrección de tipos e rangos de datos, y desencadena la ejecución delegando en `cargaDatos`.
2. **`cargaDatos`**: Lee el archivo binario (`.bin`) de entrada y transmite la cabecera (ancho, alto) y píxeles al nodo `preprocesamiento` vía pipe.
3. **`preprocesamiento`**: Aplica las operaciones morfológicas de erosión y dilatación con un elemento estructurante $3 \times 3$. Si la bandera `-d` está presente, bifurca los datos hacia el nodo `aDeRuido`. Transmite la imagen limpia hacia `tHough`.
4. **`aDeRuido`**: (Nodo opcional de depuración). Recibe la imagen original y preprocesada, exporta `preprocesada.bin` y calcula $ruido = original - preprocesada$ para generar `ruido.bin`.
5. **`tHough`**: Computa la Transformada de Hough acumulando votos para el radio $r$ indicado. Transmite la matriz acumuladora de enteros hacia `resultados`.
6. **`resultados`**: Aplica la Supresión de No Máximos (NMS) en un vecindario de $v \times v$ píxeles, filtra los centros que superan el umbral $\tau$ y exporta el reporte en formato CSV.

---

### 3. Instrucciones de Compilación

Para compilar todo el conjunto de ejecutables del laboratorio, ejecute:

```bash
make
```

Para limpiar los ejecutables y archivos temporales generados:

```bash
make clean
```

---

### 4. Guía de Ejecución y Flags

El programa se ejecuta indicando los parámetros requeridos por consola:

```bash
./lab2 -i <entrada.bin> -r <radio> -t <umbral> [-v <vecindad>] [-o <salida.csv>] [-d]
```

#### Descripción de Banderas:
- **`-i`**: Ruta del archivo de imagen de entrada (`.bin`). **(Obligatoria)**.
- **`-r`**: Radio $r$ de los círculos a detectar (entero positivo mayor a 0). **(Obligatoria)**.
- **`-t`**: Umbral de confianza $\tau$ (mínimo de votos en el acumulador). **(Obligatoria)**.
- **`-v`**: Tamaño del vecindario para NMS ($v \times v$). Debe ser un número entero impar $\ge 1$. **(Opcional, default: 7)**.
- **`-o`**: Nombre del archivo de salida CSV. **(Opcional, default: "reporte.csv")**.
- **`-d`**: Bandera de depuración (Debug Dump). Exporta `preprocesada.bin` y `ruido.bin`. **(Opcional)**.

---

### 5. Ejemplos de Ejecución

#### Ejemplo Estándar:
```bash
./lab2 -i imagen_ruido.bin -r 60 -t 70 -o reporte.csv
```

#### Ejemplo con Depuración y Vecindad Personalizada:
```bash
./lab2 -i imagen_ruido.bin -r 60 -t 70 -v 7 -o reporte.csv -d
```

#### Pruebas con Distintos Radios y Umbrales:
| Radio ($r$) | Umbral ($\tau$) | Vecindad ($v$) | Descripción |
| :--- | :--- | :--- | :--- |
| 60 | 70 | 7 | Detección óptima para los círculos de la imagen de prueba. |
| 30 | 40 | 7 | Detecta círculos de menor radio con posibles falsos positivos. |
| 90 | 55 | 9 | Detección ajustada para círculos grandes. |

---

### 6. Autores

- **Johan Neira Jans** - 21.163.695-5
- **Benjamin Vasquez Borghero** - 20.932.225-0

Sistemas Operativos 2026 - Universidad de Santiago de Chile