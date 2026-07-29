#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "fresultados.h"

// Entradas: Puntero a entero 'ancho', Puntero a entero 'alto'.
// Salidas: Puntero a la matriz acumuladora de enteros (int*) recibida desde la tubería.
// Descripción: Recibe las dimensiones y la matriz acumuladora de votos enviada por el nodo tHough a través del descriptor estándar STDIN_FILENO.
int* recibir_acumulador_pipe(int *ancho, int *alto) {
    // Condicional if: Lee las dimensiones ancho y alto (8 bytes totales) desde STDIN.
    if (read(STDIN_FILENO, ancho, sizeof(int)) != sizeof(int) ||
        read(STDIN_FILENO, alto, sizeof(int)) != sizeof(int)) {
        fprintf(stderr, "[resultados] Error al leer dimensiones desde STDIN\n");
        exit(EXIT_FAILURE);
    }

    // Variable 'total_elementos': Cantidad total de entradas en la matriz bidimensional.
    size_t total_elementos = (size_t)(*ancho) * (*alto);
    
    // Variable 'bytes_totales': Tamaño en bytes requeridos para alojar el arreglo de enteros int*.
    size_t bytes_totales = total_elementos * sizeof(int);

    // Reserva de memoria dinámica mediante malloc para la matriz acumuladora.
    int *acumulador = (int *)malloc(bytes_totales);
    if (acumulador == NULL) {
        fprintf(stderr, "[resultados] Error de asignacion de memoria para acumulador\n");
        exit(EXIT_FAILURE);
    }

    // Puntero casting a bytes (unsigned char*) para controlar la lectura del buffer por trozos.
    unsigned char *ptr = (unsigned char *)acumulador;
    size_t leidos = 0;
    
    // Ciclo while: Acumula las lecturas del pipe STDIN hasta completar todos los bytes requeridos.
    while (leidos < bytes_totales) {
        ssize_t bytes = read(STDIN_FILENO, ptr + leidos, bytes_totales - leidos);
        if (bytes <= 0) {
            fprintf(stderr, "[resultados] Error al leer acumulador desde STDIN\n");
            exit(EXIT_FAILURE);
        }
        leidos += (size_t)bytes;
    }

    return acumulador;
}

// Entradas: Puntero a la matriz acumuladora (int *acumulador), entero ancho, entero alto, entero umbral, entero vecindad v, cadena de ruta de salida (const char *ruta_salida).
// Salidas: Ninguna (void).
// Descripción: Aplica la Supresión de No Máximos (NMS) sobre un vecindario de v x v píxeles en el acumulador. Escribe las coordenadas detectadas (X,Y) que superan o igualan el umbral tau en el archivo CSV especificado por ruta_salida.
void supresion_no_maximos_y_exportar(int *acumulador, int ancho, int alto, int umbral, int vecindad, const char *ruta_salida) {
    // Variable 'f': Puntero de archivo para abrir/crear el archivo CSV de salida en modo escritura ("w").
    FILE *f = fopen(ruta_salida, "w");
    if (f == NULL) {
        perror("[resultados] Error al abrir el archivo de salida CSV");
        exit(EXIT_FAILURE);
    }

    // Escribe la cabecera CSV requerida exactamente ("X,Y\n") sin espacios.
    fprintf(f, "X,Y\n");

    // Variable 'radio_vecindad': Calcula la mitad del ancho de la ventana v x v (ej: si v=7, radio_vecindad=3).
    int radio_vecindad = vecindad / 2;

    // Ciclo 1 (b): Recorre cada fila 'b' de la matriz acumuladora.
    for (int b = 0; b < alto; b++) {
        // Ciclo 2 (a): Recorre cada columna 'a' de la matriz acumuladora.
        for (int a = 0; a < ancho; a++) {
            // Variable 'votos_actual': Cantidad de votos acumulados en la posición (a,b).
            int votos_actual = acumulador[b * ancho + a];
            
            // Condicional if: Filtra inicialmente solo aquellas posiciones que igualan o superan el umbral tau.
            if (votos_actual >= umbral) {
                // Variable 'es_maximo': Flag booleano (1 si es máximo local, 0 si se debe suprimir).
                int es_maximo = 1;

                // Ciclo 3 (dy): Recorre el vecindario vertical de [-radio_vecindad, +radio_vecindad].
                for (int dy = -radio_vecindad; dy <= radio_vecindad; dy++) {
                    // Ciclo 4 (dx): Recorre el vecindario horizontal de [-radio_vecindad, +radio_vecindad].
                    for (int dx = -radio_vecindad; dx <= radio_vecindad; dx++) {
                        // Salta el píxel central (dx=0, dy=0) para no compararse consigo mismo.
                        if (dx == 0 && dy == 0) continue;

                        // Variables 'ny' y 'nx': Coordenadas de los píxeles vecinos en la ventana Vp.
                        int ny = b + dy;
                        int nx = a + dx;

                        // Condicional if: Comprueba límites para evitar accesos fuera de la matriz.
                        if (ny >= 0 && ny < alto && nx >= 0 && nx < ancho) {
                            int votos_vecino = acumulador[ny * ancho + nx];
                            
                            // Si existe un vecino con estrictamente más votos, el píxel actual no es máximo local.
                            if (votos_vecino > votos_actual) {
                                es_maximo = 0;
                                break;
                            }
                            
                            // Desempate determinista cuando hay empate en la cantidad de votos para evitar centros duplicados.
                            if (votos_vecino == votos_actual) {
                                if (ny < b || (ny == b && nx < a)) {
                                    es_maximo = 0;
                                    break;
                                }
                            }
                        }
                    }
                    if (!es_maximo) break; // Si deja de ser máximo, rompe el ciclo exterior.
                }

                // Condicional if: Si el píxel sobrevivió a NMS siendo el máximo en su vecindad Vp, escribe sus coordenadas X,Y en el reporte.
                if (es_maximo) {
                    fprintf(f, "%d,%d\n", a, b);
                }
            }
        }
    }

    // Cierra el archivo de salida CSV.
    fclose(f);
}
