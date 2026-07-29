#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "fpreprocesamiento.h"

// Entradas: Ninguna (los datos provienen del descriptor estándar STDIN_FILENO).
// Salidas: Estructura Imagen con las dimensiones y el arreglo de píxeles leído desde la tubería.
// Descripción: Lee las dimensiones (ancho y alto) y asigna memoria para recibir secuencialmente el buffer completo de píxeles desde el nodo previo (cargaDatos).
Imagen recibir_imagen_pipe() {
    Imagen img = {0, 0, NULL};

    // Condicional if: Lee las dimensiones ancho y alto enviadas desde STDIN (8 bytes totales).
    if (read(STDIN_FILENO, &img.ancho, sizeof(int)) != sizeof(int) ||
        read(STDIN_FILENO, &img.alto, sizeof(int)) != sizeof(int)) {
        fprintf(stderr, "[preprocesamiento] Error al leer dimensiones desde STDIN\n");
        exit(EXIT_FAILURE);
    }

    // Variable 'total_pixeles': Calcula la cantidad total de píxeles a recibir (ancho * alto).
    size_t total_pixeles = (size_t)img.ancho * img.alto;
    
    // Reserva de memoria en el Heap mediante malloc para alojar la imagen recibida.
    img.pixeles = (unsigned char *)malloc(total_pixeles);
    if (img.pixeles == NULL) {
        fprintf(stderr, "[preprocesamiento] Error de asignacion de memoria\n");
        exit(EXIT_FAILURE);
    }

    // Variable 'leidos': Acumulador de bytes leídos desde la tubería IPC.
    size_t leidos = 0;
    
    // Ciclo while: Garantiza la recepción completa del arreglo de píxeles por lectura en bloques.
    while (leidos < total_pixeles) {
        ssize_t bytes = read(STDIN_FILENO, img.pixeles + leidos, total_pixeles - leidos);
        if (bytes <= 0) {
            fprintf(stderr, "[preprocesamiento] Error al leer pixeles desde STDIN\n");
            exit(EXIT_FAILURE);
        }
        leidos += (size_t)bytes;
    }

    return img;
}

// Entradas: Estructura Imagen 'img' (matriz original recibida).
// Salidas: Estructura Imagen con la matriz resultante tras la erosión.
// Descripción: Aplica el operador morfológico de erosión utilizando un elemento estructurante SE 3x3 en cruz. Un píxel sobrevive (1) únicamente si él y sus 4 vecinos ortogonales son 1.
Imagen erosionar(Imagen img) {
    Imagen resultado;
    resultado.ancho = img.ancho;
    resultado.alto = img.alto;
    
    // Asigna la memoria inicializada en 0 (negro) usando calloc para la matriz de salida.
    resultado.pixeles = (unsigned char *)calloc((size_t)img.ancho * img.alto, sizeof(unsigned char));

    // Ciclo for exterior: Recorre las filas 'y' evitando bordes (desde 1 hasta alto-2) para evitar accesos fuera de rango.
    for (int y = 1; y < img.alto - 1; y++) {
        // Ciclo for interior: Recorre las columnas 'x' evitando bordes (desde 1 hasta ancho-2).
        for (int x = 1; x < img.ancho - 1; x++) {
            // Variable 'idx': Convierte las coordenadas 2D (x,y) en un índice 1D lineal (y * ancho + x).
            int idx = y * img.ancho + x;
            
            // Condicional if: Aplica la máscara del Elemento Estructurante SE 3x3 (Cruz).
            // Un píxel se conserva como 1 si el centro, arriba, abajo, izquierda y derecha son 1 en la imagen de entrada.
            if (img.pixeles[idx] == 1 &&
                img.pixeles[idx - img.ancho] == 1 && // Vecino Arriba
                img.pixeles[idx + img.ancho] == 1 && // Vecino Abajo
                img.pixeles[idx - 1] == 1 &&         // Vecino Izquierda
                img.pixeles[idx + 1] == 1) {         // Vecino Derecha
                resultado.pixeles[idx] = 1;
            }
        }
    }
    return resultado;
}

// Entradas: Estructura Imagen 'img' (matriz previamente erosionada).
// Salidas: Estructura Imagen con la matriz resultante tras la dilatación (imagen preprocesada/limpia).
// Descripción: Aplica el operador morfológico de dilatación utilizando un elemento estructurante SE 3x3 en cruz. Si un píxel es 1, activa (1) en él y sus 4 vecinos ortogonales.
Imagen dilatatar(Imagen img) {
    Imagen resultado;
    resultado.ancho = img.ancho;
    resultado.alto = img.alto;
    
    // Asigna la memoria inicializada en 0 usando calloc.
    resultado.pixeles = (unsigned char *)calloc((size_t)img.ancho * img.alto, sizeof(unsigned char));

    // Ciclos for anidados: Recorren los píxeles interiores evitando accesos fuera de límites en bordes.
    for (int y = 1; y < img.alto - 1; y++) {
        for (int x = 1; x < img.ancho - 1; x++) {
            int idx = y * img.ancho + x;
            
            // Condicional if: Si el píxel en la imagen erosionada es 1, expande el objeto activando sus vecinos.
            if (img.pixeles[idx] == 1) {
                resultado.pixeles[idx] = 1;               // Píxel Centro
                resultado.pixeles[idx - img.ancho] = 1; // Expande Arriba
                resultado.pixeles[idx + img.ancho] = 1; // Expande Abajo
                resultado.pixeles[idx - 1] = 1;         // Expande Izquierda
                resultado.pixeles[idx + 1] = 1;         // Expande Derecha
            }
        }
    }
    return resultado;
}

// Entradas: Estructura Imagen 'img' conteniendo la imagen preprocesada resultante.
// Salidas: Ninguna (void).
// Descripción: Envía por STDOUT (redirigido al pipe hacia tHough) las dimensiones y la matriz de píxeles preprocesados.
void enviar_imagen_pipe(Imagen img) {
    // Escribe la cabecera (ancho y alto) a la tubería mediante STDOUT_FILENO.
    if (write(STDOUT_FILENO, &img.ancho, sizeof(int)) != sizeof(int) ||
        write(STDOUT_FILENO, &img.alto, sizeof(int)) != sizeof(int)) {
        fprintf(stderr, "[preprocesamiento] Error al escribir dimensiones en STDOUT\n");
        exit(EXIT_FAILURE);
    }

    size_t total_pixeles = (size_t)img.ancho * img.alto;
    size_t escritos = 0;
    
    // Ciclo while: Transmite el bloque de píxeles completo hacia el pipe STDOUT.
    while (escritos < total_pixeles) {
        ssize_t bytes = write(STDOUT_FILENO, img.pixeles + escritos, total_pixeles - escritos);
        if (bytes <= 0) {
            fprintf(stderr, "[preprocesamiento] Error al escribir pixeles en STDOUT\n");
            exit(EXIT_FAILURE);
        }
        escritos += (size_t)bytes;
    }
}

// Entradas: Estructura Imagen 'orig' (original) e Imagen 'pre' (preprocesada limpia).
// Salidas: Ninguna (void).
// Descripción: Envía por STDOUT (redirigido al pipe hacia aDeRuido) ambas imágenes de forma consecutiva para generar los archivos binarios de depuración.
void enviar_imagenes_debug_pipe(Imagen orig, Imagen pre) {
    // Escribe las dimensiones compartidas (ancho y alto).
    if (write(STDOUT_FILENO, &orig.ancho, sizeof(int)) != sizeof(int) ||
        write(STDOUT_FILENO, &orig.alto, sizeof(int)) != sizeof(int)) {
        fprintf(stderr, "[preprocesamiento] Error al escribir dimensiones debug\n");
        exit(EXIT_FAILURE);
    }

    size_t total_pixeles = (size_t)orig.ancho * orig.alto;

    // Ciclo 1: Transmite los píxeles de la imagen original.
    size_t escritos = 0;
    while (escritos < total_pixeles) {
        ssize_t bytes = write(STDOUT_FILENO, orig.pixeles + escritos, total_pixeles - escritos);
        if (bytes <= 0) exit(EXIT_FAILURE);
        escritos += (size_t)bytes;
    }

    // Ciclo 2: Transmite secuencialmente los píxeles de la imagen preprocesada.
    escritos = 0;
    while (escritos < total_pixeles) {
        ssize_t bytes = write(STDOUT_FILENO, pre.pixeles + escritos, total_pixeles - escritos);
        if (bytes <= 0) exit(EXIT_FAILURE);
        escritos += (size_t)bytes;
    }
}
