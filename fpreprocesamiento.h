#ifndef FPREPROCESAMIENTO_H
#define FPREPROCESAMIENTO_H

// Definición guardada de la estructura Imagen para evitar redundancia
#ifndef IMAGEN_STRUCT_DEFINED
#define IMAGEN_STRUCT_DEFINED

// Estructura que representa la imagen binaria en el nodo de preprocesamiento.
// - ancho: Número de columnas de la imagen.
// - alto: Número de filas de la imagen.
// - pixeles: Puntero al buffer de datos de píxeles (0 o 1).
typedef struct {
    int ancho;
    int alto;
    unsigned char *pixeles;
} Imagen;
#endif

// Entradas: Ninguna (los datos provienen del descriptor estándar STDIN_FILENO).
// Salidas: Estructura Imagen con las dimensiones y el arreglo de píxeles leído desde la tubería.
// Descripción: Lee las dimensiones (ancho y alto) y asigna memoria para recibir secuencialmente el buffer completo de píxeles desde el nodo previo (cargaDatos).
Imagen recibir_imagen_pipe();

// Entradas: Estructura Imagen 'img' (matriz original recibida).
// Salidas: Estructura Imagen con la matriz resultante tras la erosión.
// Descripción: Aplica el operador morfológico de erosión utilizando un elemento estructurante SE 3x3 en cruz. Un píxel sobrevive (1) únicamente si él y sus 4 vecinos ortogonales son 1.
Imagen erosionar(Imagen img);

// Entradas: Estructura Imagen 'img' (matriz previamente erosionada).
// Salidas: Estructura Imagen con la matriz resultante tras la dilatación (imagen preprocesada/limpia).
// Descripción: Aplica el operador morfológico de dilatación utilizando un elemento estructurante SE 3x3 en cruz. Si un píxel es 1, activa (1) en él y sus 4 vecinos ortogonales.
Imagen dilatatar(Imagen img);

// Entradas: Estructura Imagen 'img' conteniendo la imagen preprocesada resultante.
// Salidas: Ninguna (void).
// Descripción: Envía por STDOUT (redirigido al pipe hacia tHough) las dimensiones y la matriz de píxeles preprocesados.
void enviar_imagen_pipe(Imagen img);

// Entradas: Estructura Imagen 'orig' (original) e Imagen 'pre' (preprocesada limpia).
// Salidas: Ninguna (void).
// Descripción: Envía por STDOUT (redirigido al pipe hacia aDeRuido) ambas imágenes de forma consecutiva para generar los archivos binarios de depuración.
void enviar_imagenes_debug_pipe(Imagen orig, Imagen pre);

#endif
