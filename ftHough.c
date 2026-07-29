#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "ftHough.h"

// Definición auxiliar constante de PI en caso de no estar definida por el compilador
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Entradas: Ninguna (los datos provienen del descriptor estándar STDIN_FILENO).
// Salidas: Estructura Imagen con la imagen preprocesada leída desde la tubería IPC.
// Descripción: Recibe la cabecera (ancho y alto) y la matriz de píxeles limpia desde el nodo de preprocesamiento.
Imagen recibir_imagen_pipe() {
    Imagen img = {0, 0, NULL};

    // Condicional if: Lee las dimensiones ancho y alto enviadas desde el pipe por STDIN.
    if (read(STDIN_FILENO, &img.ancho, sizeof(int)) != sizeof(int) ||
        read(STDIN_FILENO, &img.alto, sizeof(int)) != sizeof(int)) {
        fprintf(stderr, "[tHough] Error al leer dimensiones desde STDIN\n");
        exit(EXIT_FAILURE);
    }

    size_t total_pixeles = (size_t)img.ancho * img.alto;
    img.pixeles = (unsigned char *)malloc(total_pixeles);
    if (img.pixeles == NULL) {
        fprintf(stderr, "[tHough] Error de asignacion de memoria\n");
        exit(EXIT_FAILURE);
    }

    // Ciclo while: Acumula las lecturas del pipe hasta obtener el total de píxeles.
    size_t leidos = 0;
    while (leidos < total_pixeles) {
        ssize_t bytes = read(STDIN_FILENO, img.pixeles + leidos, total_pixeles - leidos);
        if (bytes <= 0) {
            fprintf(stderr, "[tHough] Error al leer pixeles desde STDIN\n");
            exit(EXIT_FAILURE);
        }
        leidos += (size_t)bytes;
    }

    return img;
}

// Entradas: Estructura Imagen 'img' (imagen limpia) y entero 'radio' r de los círculos a detectar.
// Salidas: Puntero a la matriz acumuladora de votos de tipo entero int* (de tamaño ancho * alto).
// Descripción: Ejecuta el algoritmo de la Transformada de Hough para círculos de radio fijo r. Muestra el acumulador incrementando votos en (a, b) para cada píxel de borde activo (1) barriendo theta en 720 pasos de 0 a 2*PI.
int* calcular_hough(Imagen img, int radio) {
    int ancho = img.ancho;
    int alto = img.alto;
    size_t total = (size_t)ancho * alto;

    // Asignación de memoria inicializada en cero (calloc) para el plano acumulador H(a,b) de enteros.
    int *acumulador = (int *)calloc(total, sizeof(int));
    if (acumulador == NULL) {
        fprintf(stderr, "[tHough] Error de asignacion de memoria para acumulador\n");
        exit(EXIT_FAILURE);
    }

    // Ciclo 1 (y): Recorre cada fila de la imagen preprocesada.
    for (int y = 0; y < alto; y++) {
        // Ciclo 2 (x): Recorre cada columna de la imagen preprocesada.
        for (int x = 0; x < ancho; x++) {
            // Condicional if: Filtra los píxeles de borde activos (valor igual a 1).
            if (img.pixeles[y * ancho + x] == 1) {
                // Ciclo 3 (k): Recorre el espacio parametrizado del ángulo theta en 720 divisiones angulares discretas.
                // Muestra la fórmula paramétrica del círculo: a = x - r * cos(theta), b = y - r * sin(theta).
                for (int k = 0; k <= 720; k++) {
                    // Variable 'theta': Ángulo en radianes calculado como k * 2*PI / 720.
                    double theta = (2.0 * M_PI * k) / 720.0;
                    
                    // Variables 'a' y 'b': Coordenadas calculadas del posible centro del círculo usando redondeo entero.
                    int a = (int)round(x - radio * cos(theta));
                    int b = (int)round(y - radio * sin(theta));

                    // Condicional if: Valida límites del acumulador. Si el centro (a,b) está dentro de los límites de la imagen, suma un voto en H(a,b).
                    if (a >= 0 && a < ancho && b >= 0 && b < alto) {
                        acumulador[b * ancho + a]++;
                    }
                }
            }
        }
    }

    return acumulador;
}

// Entradas: Puntero a la matriz acumuladora (int *acumulador), entero ancho, entero alto.
// Salidas: Ninguna (void).
// Descripción: Transmite la cabecera (ancho y alto) y la matriz bidimensional de votos completa (ancho * alto * sizeof(int) bytes) hacia el descriptor STDOUT redirigido al pipe hacia el nodo resultados.
void enviar_acumulador_pipe(int *acumulador, int ancho, int alto) {
    // Escribe la cabecera (ancho y alto) en la tubería por STDOUT.
    if (write(STDOUT_FILENO, &ancho, sizeof(int)) != sizeof(int) ||
        write(STDOUT_FILENO, &alto, sizeof(int)) != sizeof(int)) {
        fprintf(stderr, "[tHough] Error al escribir dimensiones en STDOUT\n");
        exit(EXIT_FAILURE);
    }

    size_t total_elementos = (size_t)ancho * alto;
    size_t bytes_totales = total_elementos * sizeof(int);
    size_t escritos = 0;

    // Puntero en bytes para recorrer y escribir la matriz acumuladora en la tubería.
    unsigned char *ptr = (unsigned char *)acumulador;
    
    // Ciclo while: Garantiza la transmisión de todos los bytes del acumulador en el pipe STDOUT.
    while (escritos < bytes_totales) {
        ssize_t bytes = write(STDOUT_FILENO, ptr + escritos, bytes_totales - escritos);
        if (bytes <= 0) {
            fprintf(stderr, "[tHough] Error al escribir acumulador en STDOUT\n");
            exit(EXIT_FAILURE);
        }
        escritos += (size_t)bytes;
    }
}
