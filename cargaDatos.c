#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "fcargaDatos.h"

// Entradas: Cantidad de argumentos (int argc), Arreglo de cadenas de argumentos (char *argv[]).
// Salidas: Estado de finalización entero (0 en caso de éxito, 1 en caso de error).
// Descripción: Función principal del nodo cargaDatos. Parsea banderas pasadas desde lab2, lee la imagen binaria, genera una tubería IPC hacia preprocesamiento, hace fork+execv y envía los datos por el pipe STDOUT.
int main(int argc, char *argv[]) {
    // Variables para almacenar las opciones pasadas por la línea de comandos
    char *input = NULL;   // Ruta del archivo binario de entrada (-i)
    char *radio = NULL;   // Radio r para la Transformada de Hough (-r)
    char *umbral = NULL;  // Umbral tau (-t)
    char *vecindad = NULL;// Tamaño de vecindad para NMS (-v)
    char *output = NULL;  // Ruta del archivo CSV de salida (-o)
    int debug = 0;        // Flag indicador para exportar dumps de depuración (-d)
    int opt;              // Variable receptora del código de opción procesado por getopt

    // Ciclo while: Itera mientras la función getopt siga encontrando banderas válidas en los argumentos.
    while ((opt = getopt(argc, argv, "i:r:t:v:o:d")) != -1) {
        // Estructura switch: Asigna cada valor según la opción encontrada.
        switch (opt) {
            case 'i': input = optarg; break;    // Puntero a la cadena con la ruta de entrada
            case 'r': radio = optarg; break;    // Puntero al string del radio
            case 't': umbral = optarg; break;   // Puntero al string del umbral
            case 'v': vecindad = optarg; break; // Puntero al string de la vecindad
            case 'o': output = optarg; break;   // Puntero al string de salida
            case 'd': debug = 1; break;         // Activa el modo debug (1)
            default: break;
        }
    }

    // Condicional if: Valida que la ruta de entrada no sea NULL.
    if (input == NULL) {
        fprintf(stderr, "[cargaDatos] Error: Archivo de entrada no especificado.\n");
        exit(EXIT_FAILURE);
    }

    // Llamada a la función leer_imagen para cargar los datos del archivo en memoria principal.
    Imagen img = leer_imagen(input);

    // Variable 'pfd': Arreglo de 2 enteros para almacenar los descriptores de lectura pfd[0] y escritura pfd[1] de la tubería IPC.
    int pfd[2];
    
    // Condicional if: Crea la tubería IPC entre este nodo (cargaDatos) y el nodo sucesor (preprocesamiento).
    if (pipe(pfd) == -1) {
        perror("[cargaDatos] Error al crear pipe");
        exit(EXIT_FAILURE);
    }

    // Variable 'pid': Identificador de proceso retornado por la llamada al sistema fork().
    pid_t pid = fork();
    
    // Condicional if: Revisa si el fork falló (retorna valor negativo).
    if (pid < 0) {
        perror("[cargaDatos] Error en fork");
        exit(EXIT_FAILURE);
    }

    // Condicional if/else: Separa la ejecución del proceso Hijo (pid == 0) y el proceso Padre (pid > 0).
    if (pid == 0) {
        // --- CÓDIGO DEL PROCESO HIJO (Ejecutará el nodo preprocesamiento) ---
        // Cierra el extremo de escritura pfd[1] porque el hijo solo leerá datos por el pipe.
        close(pfd[1]);
        
        // Redirecciona la entrada estándar STDIN_FILENO para que apunte al extremo de lectura del pipe pfd[0].
        if (dup2(pfd[0], STDIN_FILENO) == -1) {
            perror("[cargaDatos] Error en dup2 para STDIN");
            exit(EXIT_FAILURE);
        }
        
        // Cierra el descriptor duplicado original pfd[0].
        close(pfd[0]);

        // Construcción dinámica del arreglo de argumentos 'exec_args' que se enviará al proceso sucesor mediante execv.
        char *exec_args[15];
        int idx = 0;
        exec_args[idx++] = "./preprocesamiento";
        if (radio) { exec_args[idx++] = "-r"; exec_args[idx++] = radio; }
        if (umbral) { exec_args[idx++] = "-t"; exec_args[idx++] = umbral; }
        if (vecindad) { exec_args[idx++] = "-v"; exec_args[idx++] = vecindad; }
        if (output) { exec_args[idx++] = "-o"; exec_args[idx++] = output; }
        if (debug) { exec_args[idx++] = "-d"; }
        exec_args[idx] = NULL; // Terminador NULL obligatorio para execv

        // Reemplaza la imagen del proceso hijo por el ejecutable './preprocesamiento'.
        execv(exec_args[0], exec_args);
        
        // Si execv retorna, ocurrió un error (no debería retornar si es exitoso).
        perror("[cargaDatos] Error al ejecutar execv preprocesamiento");
        exit(EXIT_FAILURE);
    } else {
        // --- CÓDIGO DEL PROCESO PADRE (CargaDatos transmisor) ---
        // Cierra el extremo de lectura pfd[0] porque el padre solo escribirá datos en el pipe.
        close(pfd[0]);
        
        // Redirecciona la salida estándar STDOUT_FILENO para que apunte al extremo de escritura del pipe pfd[1].
        if (dup2(pfd[1], STDOUT_FILENO) == -1) {
            perror("[cargaDatos] Error en dup2 para STDOUT");
            exit(EXIT_FAILURE);
        }
        
        // Cierra el descriptor duplicado original pfd[1].
        close(pfd[1]);

        // Envía las dimensiones y los píxeles de la imagen al hijo a través de STDOUT redirigido.
        enviar_imagen_pipe(img);
        
        // Libera la memoria dinamica de los píxeles cargados tras finalizar la transmisión.
        free(img.pixeles);

        // Variable 'status': Almacena el estado de salida del proceso hijo.
        int status;
        
        // Espera síncrona a que el proceso hijo finalice la ejecución de toda la cadena del pipeline.
        waitpid(pid, &status, 0);
    }

    return 0;
}
