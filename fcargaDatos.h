#ifndef FCARGADATOS_H
#define FCARGADATOS_H

// Definición guardada de la estructura Imagen para evitar doble declaración
#ifndef IMAGEN_STRUCT_DEFINED
#define IMAGEN_STRUCT_DEFINED

// Estructura que representa una imagen binaria en memoria.
// - ancho: Almacena el número de columnas (ancho en píxeles).
// - alto: Almacena el número de filas (alto en píxeles).
// - pixeles: Puntero a un arreglo de bytes en memoria dinámica conteniendo los valores de píxeles (0 o 1).
typedef struct {
    int ancho;
    int alto;
    unsigned char *pixeles;
} Imagen;
#endif

// Entradas: Ruta del archivo binario (.bin) en el sistema de archivos (char *ruta).
// Salidas: Estructura Imagen con los miembros ancho, alto y puntero pixeles cargados en memoria dinámica.
// Descripción: Abre el archivo binario, lee la cabecera de 8 bytes (2 enteros de 4 bytes para ancho y alto) y asigna memoria dinámica para leer el bloque completo de píxeles.
Imagen leer_imagen(char *ruta);

// Entradas: Estructura Imagen conteniendo las dimensiones y el arreglo de píxeles cargado.
// Salidas: Ninguna (void).
// Descripción: Transmite las dimensiones (ancho y alto) y el bloque de píxeles hacia el descriptor STDOUT (que está redirigido mediante tubería/pipe al siguiente nodo).
void enviar_imagen_pipe(Imagen img);

#endif
