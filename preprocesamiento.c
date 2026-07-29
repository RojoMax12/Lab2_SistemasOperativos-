#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "fpreprocesamiento.h"

// Entradas: Cantidad de argumentos (int argc), Arreglo de argumentos (char *argv[]).
// Salidas: Código de estado entero (0 si finaliza exitosamente).
// Descripción: Función principal del nodo preprocesamiento. Recibe la imagen original por STDIN, efectúa apertura morfológica (erosión + dilatación). Si la bandera debug (-d) está activa, bifurca datos hacia aDeRuido. Luego crea un pipe e invoca tHough enviando la imagen limpia.
int main(int argc, char *argv[]) {
    // Variables para capturar los argumentos pasados mediante execv
    char *radio = NULL;
    char *umbral = NULL;
    char *vecindad = NULL;
    char *output = NULL;
    int debug = 0;
    int opt;

    // Ciclo getopt: Procesa los argumentos de la consola recibidos desde el nodo anterior.
    while ((opt = getopt(argc, argv, "r:t:v:o:d")) != -1) {
        switch (opt) {
            case 'r': radio = optarg; break;
            case 't': umbral = optarg; break;
            case 'v': vecindad = optarg; break;
            case 'o': output = optarg; break;
            case 'd': debug = 1; break;
            default: break;
        }
    }

    // Recepción de la imagen binaria original a través de la tubería STDIN
    Imagen img_orig = recibir_imagen_pipe();
    
    // Etapa de Preprocesamiento: Erosión seguida de Dilatación (Apertura Morfológica)
    Imagen img_ero = erosionar(img_orig);
    Imagen img_pre = dilatatar(img_ero);

    // Variable 'pid_debug': Registra el PID del hijo aDeRuido si el modo depuración está activo (-d).
    pid_t pid_debug = -1;
    
    // Condicional if: Si la bandera -d fue ingresada por el usuario, crea una tubería síncrona hacia el nodo 'aDeRuido'.
    if (debug) {
        int pfd_debug[2];
        if (pipe(pfd_debug) == -1) {
            perror("[preprocesamiento] Error al crear pipe debug");
            exit(EXIT_FAILURE);
        }

        // Fork para crear el proceso hijo aDeRuido
        pid_debug = fork();
        if (pid_debug < 0) {
            perror("[preprocesamiento] Error en fork debug");
            exit(EXIT_FAILURE);
        }

        if (pid_debug == 0) {
            // --- PROCESO HIJO (Ejecutará aDeRuido) ---
            close(pfd_debug[1]); // Cierra extremo de escritura
            
            // Redirecciona STDIN para leer desde el pipe debug
            if (dup2(pfd_debug[0], STDIN_FILENO) == -1) {
                perror("[preprocesamiento] Error en dup2 debug STDIN");
                exit(EXIT_FAILURE);
            }
            close(pfd_debug[0]);

            // Reemplazo de imagen de proceso hacia ./aDeRuido
            char *exec_args[] = {"./aDeRuido", NULL};
            execv(exec_args[0], exec_args);
            perror("[preprocesamiento] Error al ejecutar execv aDeRuido");
            exit(EXIT_FAILURE);
        } else {
            // --- PROCESO PADRE (Envía las imágenes original y preprocesada) ---
            close(pfd_debug[0]);

            // Guarda el descriptor STDOUT original para restaurarlo posteriormente
            int saved_stdout = dup(STDOUT_FILENO);
            if (dup2(pfd_debug[1], STDOUT_FILENO) == -1) {
                perror("[preprocesamiento] Error en dup2 debug STDOUT");
                exit(EXIT_FAILURE);
            }
            close(pfd_debug[1]);

            // Transmite ambas imágenes hacia aDeRuido
            enviar_imagenes_debug_pipe(img_orig, img_pre);
            fflush(stdout);

            // Restaura el descriptor STDOUT original
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdout);
        }
    }

    // Tubería hacia el nodo tHough
    int pfd_hough[2];
    if (pipe(pfd_hough) == -1) {
        perror("[preprocesamiento] Error al crear pipe hough");
        exit(EXIT_FAILURE);
    }

    // Fork para crear el proceso hijo que ejecutará tHough
    pid_t pid_hough = fork();
    if (pid_hough < 0) {
        perror("[preprocesamiento] Error en fork hough");
        exit(EXIT_FAILURE);
    }

    if (pid_hough == 0) {
        // --- PROCESO HIJO (Ejecutará tHough) ---
        close(pfd_hough[1]);
        
        // Redirecciona STDIN para leer la imagen preprocesada desde el pipe
        if (dup2(pfd_hough[0], STDIN_FILENO) == -1) {
            perror("[preprocesamiento] Error en dup2 hough STDIN");
            exit(EXIT_FAILURE);
        }
        close(pfd_hough[0]);

        // Construcción de argumentos para tHough
        char *exec_args[12];
        int idx = 0;
        exec_args[idx++] = "./tHough";
        if (radio) { exec_args[idx++] = "-r"; exec_args[idx++] = radio; }
        if (umbral) { exec_args[idx++] = "-t"; exec_args[idx++] = umbral; }
        if (vecindad) { exec_args[idx++] = "-v"; exec_args[idx++] = vecindad; }
        if (output) { exec_args[idx++] = "-o"; exec_args[idx++] = output; }
        exec_args[idx] = NULL;

        // Ejecución de tHough mediante execv
        execv(exec_args[0], exec_args);
        perror("[preprocesamiento] Error al ejecutar execv tHough");
        exit(EXIT_FAILURE);
    } else {
        // --- PROCESO PADRE ---
        close(pfd_hough[0]);
        
        // Redirecciona STDOUT al pipe para transmitir la imagen preprocesada a tHough
        if (dup2(pfd_hough[1], STDOUT_FILENO) == -1) {
            perror("[preprocesamiento] Error en dup2 hough STDOUT");
            exit(EXIT_FAILURE);
        }
        close(pfd_hough[1]);

        // Transmisión de la imagen preprocesada limpia
        enviar_imagen_pipe(img_pre);

        // Liberación de estructuras de memoria asignadas en memoria Heap
        free(img_orig.pixeles);
        free(img_ero.pixeles);
        free(img_pre.pixeles);

        // Espera síncrona a la finalización de los procesos hijos creados
        int status;
        if (pid_debug > 0) waitpid(pid_debug, &status, 0);
        waitpid(pid_hough, &status, 0);
    }

    return 0;
}
