#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "fresultados.h"

// Entradas: Cantidad de argumentos (int argc), Arreglo de cadenas de argumentos (char *argv[]).
// Salidas: Código de estado entero (0 si fue exitoso).
// Descripción: Función principal del nodo de Salida de Resultados. Procesa los parámetros pasados (-t, -v, -o), recibe la matriz acumuladora por STDIN, aplica la Supresión de No Máximos (NMS) y genera el archivo final CSV.
int main(int argc, char *argv[]) {
    // Variables locales para almacenar los parámetros recibidos
    int umbral = 0;             // Umbral tau (-t)
    int vecindad = 7;           // Tamaño por defecto de la vecindad NMS (-v, default: 7)
    char *output = "reporte.csv";// Nombre por defecto del archivo de salida CSV (-o)
    int opt;                    // Receptora de getopt

    // Ciclo getopt: Extrae los argumentos pasados mediante execv
    while ((opt = getopt(argc, argv, "t:v:o:")) != -1) {
        switch (opt) {
            case 't': umbral = atoi(optarg); break;
            case 'v': vecindad = atoi(optarg); break;
            case 'o': output = optarg; break;
            default: break;
        }
    }

    // Condicional if: Valida la restricción del enunciado para la bandera -v (debe ser un entero impar >= 1).
    if (vecindad < 1 || vecindad % 2 == 0) {
        fprintf(stderr, "[resultados] Error: El tamano de vecindad (-v) debe ser un entero impar >= 1.\n");
        exit(EXIT_FAILURE);
    }

    int ancho = 0, alto = 0;
    
    // Recibe la matriz acumuladora transmitida por el nodo tHough a través de STDIN.
    int *acumulador = recibir_acumulador_pipe(&ancho, &alto);

    // Aplica NMS en la ventana v x v y exporta las coordenadas X,Y al archivo CSV.
    supresion_no_maximos_y_exportar(acumulador, ancho, alto, umbral, vecindad, output);

    // Liberación de la memoria asignada en el Heap para el acumulador.
    free(acumulador);
    
    return 0;
}
