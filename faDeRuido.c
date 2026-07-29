#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "faDeRuido.h"

// Entradas: Ruta del archivo destino (char *ruta), entero ancho, entero alto, puntero al arreglo de píxeles (unsigned char *pixeles).
// Salidas: Ninguna (void).
// Descripción: Escribe una imagen en formato binario (.bin) con encabezado de 8 bytes (ancho y alto de 4 bytes) seguido por la matriz de datos de píxeles en el disco.
void escribir_imagen_binaria(char *ruta, int ancho, int alto, unsigned char *pixeles) {
    // Variable 'archivo': Puntero de archivo para escritura binaria (wb).
    FILE *archivo = fopen(ruta, "wb");
    
    // Condicional if: Verifica la creación exitosa del archivo de salida.
    if (archivo == NULL) {
        perror("Error al abrir archivo para escribir debug bin");
        exit(EXIT_FAILURE);
    }

    // Escribe las dimensiones en la cabecera de 8 bytes (ancho y alto).
    fwrite(&ancho, sizeof(int), 1, archivo);
    fwrite(&alto, sizeof(int), 1, archivo);

    // Variable 'total_pixeles': Calcula el total de bytes a escribir.
    size_t total_pixeles = (size_t)ancho * alto;
    
    // Escribe la matriz completa de píxeles en el disco.
    fwrite(pixeles, sizeof(unsigned char), total_pixeles, archivo);
    
    // Cierra el descriptor del archivo.
    fclose(archivo);
}

// Entradas: Ninguna (los datos se leen directamente desde STDIN mediante el descriptor estándar).
// Salidas: Ninguna (void).
// Descripción: Recibe las matrices de píxeles original y preprocesada enviadas desde el nodo preprocesamiento. Exporta 'preprocesada.bin' y calcula el ruido aislado (original - preprocesada) exportando 'ruido.bin'.
void procesar_y_guardar_ruido() {
    int ancho, alto;

    // Condicional if: Lee la cabecera con el ancho y alto desde STDIN (8 bytes).
    if (read(STDIN_FILENO, &ancho, sizeof(int)) != sizeof(int) ||
        read(STDIN_FILENO, &alto, sizeof(int)) != sizeof(int)) {
        fprintf(stderr, "[aDeRuido] Error al leer dimensiones desde STDIN\n");
        exit(EXIT_FAILURE);
    }

    // Variable 'total_pixeles': Tamaño del bloque de píxeles en bytes.
    size_t total_pixeles = (size_t)ancho * alto;
    
    // Asignación de buffers de memoria dinámica para alojar la imagen original, la preprocesada y el ruido resultante.
    unsigned char *orig_pix = (unsigned char *)malloc(total_pixeles);
    unsigned char *prep_pix = (unsigned char *)malloc(total_pixeles);
    unsigned char *ruido_pix = (unsigned char *)malloc(total_pixeles);

    // Condicional if: Verifica la correcta reserva de memoria dinámica.
    if (!orig_pix || !prep_pix || !ruido_pix) {
        fprintf(stderr, "[aDeRuido] Error de asignacion de memoria\n");
        exit(EXIT_FAILURE);
    }

    // Ciclo 1: Lectura completa del buffer correspondiente a los píxeles originales.
    size_t leidos = 0;
    while (leidos < total_pixeles) {
        ssize_t bytes = read(STDIN_FILENO, orig_pix + leidos, total_pixeles - leidos);
        if (bytes <= 0) exit(EXIT_FAILURE);
        leidos += (size_t)bytes;
    }

    // Ciclo 2: Lectura completa del buffer correspondiente a los píxeles preprocesados.
    leidos = 0;
    while (leidos < total_pixeles) {
        ssize_t bytes = read(STDIN_FILENO, prep_pix + leidos, total_pixeles - leidos);
        if (bytes <= 0) exit(EXIT_FAILURE);
        leidos += (size_t)bytes;
    }

    // Exporta la imagen preprocesada al archivo binario 'preprocesada.bin'
    escribir_imagen_binaria("preprocesada.bin", ancho, alto, prep_pix);

    // Ciclo for: Calcula la resta píxel a píxel: Ruido = Original - Preprocesada
    // Se utiliza para aislar el ruido de sal eliminado por la operación morfológica de apertura.
    for (size_t i = 0; i < total_pixeles; i++) {
        // Expresión ternaria: Si el píxel original es mayor que el preprocesado, asigna la diferencia; si no, asigna 0.
        ruido_pix[i] = orig_pix[i] > prep_pix[i] ? orig_pix[i] - prep_pix[i] : 0;
    }

    // Exporta el mapa de ruido aislado al archivo binario 'ruido.bin'
    escribir_imagen_binaria("ruido.bin", ancho, alto, ruido_pix);

    // Liberación de memoria Heap de los 3 buffers.
    free(orig_pix);
    free(prep_pix);
    free(ruido_pix);
}
