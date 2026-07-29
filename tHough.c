#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "ftHough.h"

// Entradas: Cantidad de argumentos (int argc), Arreglo de cadenas de argumentos (char *argv[]).
// Salidas: Código de estado entero (0 si tuvo éxito).
// Descripción: Función principal del nodo tHough. Recibe la imagen preprocesada por STDIN, calcula la matriz acumuladora para el radio r dado, y envía el acumulador a través de un pipe al nodo resultados.
int main(int argc, char *argv[]) {
    int radio = 0;
    char *umbral = NULL;
    char *vecindad = NULL;
    char *output = NULL;
    int opt;

    // Ciclo getopt: Obtiene los parámetros requeridos (-r, -t, -v, -o) desde la línea de comandos pasados por execv.
    while ((opt = getopt(argc, argv, "r:t:v:o:")) != -1) {
        switch (opt) {
            case 'r': radio = atoi(optarg); break;
            case 't': umbral = optarg; break;
            case 'v': vecindad = optarg; break;
            case 'o': output = optarg; break;
            default: break;
        }
    }

    // Condicional if: Valida que el radio ingresado sea un valor entero positivo estricto.
    if (radio <= 0) {
        fprintf(stderr, "[tHough] Error: Radio debe ser un numero entero positivo.\n");
        exit(EXIT_FAILURE);
    }

    // Recibe la imagen limpia desde el pipe STDIN.
    Imagen img = recibir_imagen_pipe();
    
    // Ejecuta la Transformada de Hough acumulando votos para el radio r.
    int *acumulador = calcular_hough(img, radio);

    // Creación del pipe IPC para comunicarse con el nodo final (resultados).
    int pfd[2];
    if (pipe(pfd) == -1) {
        perror("[tHough] Error al crear pipe");
        exit(EXIT_FAILURE);
    }

    // Fork para instanciar el proceso hijo del nodo resultados.
    pid_t pid = fork();
    if (pid < 0) {
        perror("[tHough] Error en fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // --- PROCESO HIJO (Ejecutará resultados) ---
        close(pfd[1]); // Cierra extremo de escritura en el hijo
        
        // Redirecciona STDIN para leer desde el pipe
        if (dup2(pfd[0], STDIN_FILENO) == -1) {
            perror("[tHough] Error en dup2 STDIN");
            exit(EXIT_FAILURE);
        }
        close(pfd[0]);

        // Construcción del arreglo de argumentos para execv hacia resultados
        char *exec_args[10];
        int idx = 0;
        exec_args[idx++] = "./resultados";
        if (umbral) { exec_args[idx++] = "-t"; exec_args[idx++] = umbral; }
        if (vecindad) { exec_args[idx++] = "-v"; exec_args[idx++] = vecindad; }
        if (output) { exec_args[idx++] = "-o"; exec_args[idx++] = output; }
        exec_args[idx] = NULL;

        // Ejecución de resultados mediante execv
        execv(exec_args[0], exec_args);
        perror("[tHough] Error al ejecutar execv resultados");
        exit(EXIT_FAILURE);
    } else {
        // --- PROCESO PADRE (Nodo tHough) ---
        close(pfd[0]); // Cierra extremo de lectura en el padre
        
        // Redirecciona STDOUT al pipe para escribir el acumulador
        if (dup2(pfd[1], STDOUT_FILENO) == -1) {
            perror("[tHough] Error en dup2 STDOUT");
            exit(EXIT_FAILURE);
        }
        close(pfd[1]);

        // Transmite las dimensiones y la matriz acumuladora por STDOUT
        enviar_acumulador_pipe(acumulador, img.ancho, img.alto);

        // Liberación de memoria dinámica
        free(img.pixeles);
        free(acumulador);

        // Espera a la finalización del hijo
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}
