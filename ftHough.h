#ifndef FTHOUGH_H
#define FTHOUGH_H

// Definición de la estructura Imagen protegida ante inclusión múltiple
#ifndef IMAGEN_STRUCT_DEFINED
#define IMAGEN_STRUCT_DEFINED

// Estructura que representa la imagen binaria preprocesada en el nodo tHough.
// - ancho: Ancho de la matriz de píxeles.
// - alto: Alto de la matriz de píxeles.
// - pixeles: Puntero al buffer de píxeles limpios (0 o 1).
typedef struct {
    int ancho;
    int alto;
    unsigned char *pixeles;
} Imagen;
#endif

// Entradas: Ninguna (los datos provienen del descriptor estándar STDIN_FILENO).
// Salidas: Estructura Imagen con la imagen preprocesada leída desde la tubería IPC.
// Descripción: Recibe la cabecera (ancho y alto) y la matriz de píxeles limpia desde el nodo de preprocesamiento.
Imagen recibir_imagen_pipe();

// Entradas: Estructura Imagen 'img' (imagen limpia) y entero 'radio' r de los círculos a detectar.
// Salidas: Puntero a la matriz acumuladora de votos de tipo entero int* (de tamaño ancho * alto).
// Descripción: Ejecuta el algoritmo de la Transformada de Hough para círculos de radio fijo r. Muestra el acumulador incrementando votos en (a, b) para cada píxel de borde activo (1) barriendo theta en 720 pasos de 0 a 2*PI.
int* calcular_hough(Imagen img, int radio);

// Entradas: Puntero a la matriz acumuladora (int *acumulador), entero ancho, entero alto.
// Salidas: Ninguna (void).
// Descripción: Transmite la cabecera (ancho y alto) y la matriz bidimensional de votos completa (ancho * alto * sizeof(int) bytes) hacia el descriptor STDOUT redirigido al pipe hacia el nodo resultados.
void enviar_acumulador_pipe(int *acumulador, int ancho, int alto);

#endif
