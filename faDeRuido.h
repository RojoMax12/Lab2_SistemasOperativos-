#ifndef FADERUIDO_H
#define FADERUIDO_H

// Entradas: Ruta del archivo destino (char *ruta), entero ancho, entero alto, puntero al arreglo de píxeles (unsigned char *pixeles).
// Salidas: Ninguna (void).
// Descripción: Escribe una imagen en formato binario (.bin) con encabezado de 8 bytes (ancho y alto de 4 bytes) seguido por la matriz de datos de píxeles en el disco.
void escribir_imagen_binaria(char *ruta, int ancho, int alto, unsigned char *pixeles);

// Entradas: Ninguna (los datos se leen directamente desde STDIN mediante el descriptor estándar).
// Salidas: Ninguna (void).
// Descripción: Recibe las matrices de píxeles original y preprocesada enviadas desde el nodo preprocesamiento. Exporta 'preprocesada.bin' y calcula el ruido aislado (original - preprocesada) exportando 'ruido.bin'.
void procesar_y_guardar_ruido();

#endif
