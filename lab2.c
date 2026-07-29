#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

// Entradas: Cadena de texto a evaluar (const char *str).
// Salidas: Entero 1 si la cadena representa un entero positivo estricto (> 0), 0 en caso contrario.
// Descripción: Función de validación de entradas. Recorre carácter por carácter verificando que todos sean dígitos numéricos ('0'-'9'). Si detecta decimales (como 4.5), negativos o caracteres no numéricos, retorna 0.
int es_entero_positivo_estricto(const char *str) {
    // Condicional if: Retorna 0 si la cadena es nula o vacía.
    if (str == NULL || *str == '\0') return 0;
    
    // Ciclo for: Inspecciona cada carácter de la cadena.
    for (int i = 0; str[i] != '\0'; i++) {
        // Condicional if: Si algún carácter no es un dígito entero, rechaza la entrada (retorna 0).
        if (!isdigit(str[i])) return 0;
    }
    
    // Convierte a entero y verifica que sea estrictamente mayor que cero.
    int val = atoi(str);
    return val > 0;
}

// Entradas: Cadena de texto a evaluar (const char *str).
// Salidas: Entero 1 si la cadena representa un entero no negativo (>= 0), 0 en caso contrario.
// Descripción: Función de validación de entradas. Verifica que la cadena contenga únicamente dígitos enteros positivos o cero.
int es_entero_no_negativo_estricto(const char *str) {
    if (str == NULL || *str == '\0') return 0;
    
    // Ciclo for: Inspección de caracteres individuales.
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) return 0;
    }
    
    int val = atoi(str);
    return val >= 0;
}

// Entradas: Parámetros recibidos desde la consola (int argc, char *argv[]).
// Salidas: Código de término del programa (0 para éxito, distinto de 0 para error).
// Descripción: Función principal orquestadora (main) del programa lab2. Lee las opciones mediante getopt, realiza una validación exhaustiva de los tipos y valores de los argumentos, crea la primera tubería e invoca al nodo ejecutable 'cargaDatos' con execv.
int main(int argc, char *argv[]) {
    // Declaración e inicialización de variables para las banderas de la línea de comandos
    char *input = NULL;   // Ruta del archivo binario de entrada (-i)
    char *r_str = NULL;   // Cadena del radio (-r)
    char *t_str = NULL;   // Cadena del umbral (-t)
    char *v_str = "7";    // Cadena de tamaño de vecindad (-v, valor por defecto: "7")
    char *output = "reporte.csv"; // Nombre del archivo CSV de salida (-o, valor por defecto: "reporte.csv")
    int debug = 0;        // Indicador booleano para exportar dumps de depuración (-d, por defecto: 0)
    int opt;              // Receptora de opciones del ciclo getopt

    // Ciclo getopt: Extrae cada flag pasada por la consola en la invocación del ejecutable ./lab2
    while ((opt = getopt(argc, argv, "i:r:t:v:o:d")) != -1) {
        switch (opt) {
            case 'i': input = optarg; break;    // Asigna la ruta de imagen de entrada
            case 'r': r_str = optarg; break;    // Asigna el string del radio
            case 't': t_str = optarg; break;    // Asigna el string del umbral
            case 'v': v_str = optarg; break;    // Asigna el string de vecindad NMS
            case 'o': output = optarg; break;   // Asigna el nombre de reporte CSV
            case 'd': debug = 1; break;         // Activa la bandera de depuración
            default:
                fprintf(stderr, "Uso: %s -i <entrada.bin> -r <radio> -t <umbral> [-v <vecindad>] [-o <reporte.csv>] [-d]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // --- SECCIÓN DE VALIDACIÓN EXHAUSTIVA DE PARÁMETROS DE ENTRADA ---
    
    // Condicional 1: Comprueba que la flag -i haya sido proporcionada (Obligatoria).
    if (input == NULL) {
        fprintf(stderr, "Error: La flag -i (archivo de entrada) es obligatoria.\n");
        exit(EXIT_FAILURE);
    }

    // Condicional 2: Comprueba que el archivo binario de entrada exista en el disco utilizando access().
    if (access(input, F_OK) != 0) {
        fprintf(stderr, "Error: El archivo de entrada '%s' no existe.\n", input);
        exit(EXIT_FAILURE);
    }

    // Condicional 3: Comprueba que la flag -r haya sido proporcionada (Obligatoria).
    if (r_str == NULL) {
        fprintf(stderr, "Error: La flag -r (radio) es obligatoria.\n");
        exit(EXIT_FAILURE);
    }

    // Condicional 4: Valida que el radio sea un entero positivo (Rechaza flotantes como 4.5 o valores <= 0).
    if (!es_entero_positivo_estricto(r_str)) {
        fprintf(stderr, "Error: El valor de la flag de radio (-r) debe ser un numero entero positivo mayor a cero (ingresado: '%s').\n", r_str);
        exit(EXIT_FAILURE);
    }

    // Condicional 5: Comprueba que la flag -t haya sido proporcionada (Obligatoria).
    if (t_str == NULL) {
        fprintf(stderr, "Error: La flag -t (umbral) es obligatoria.\n");
        exit(EXIT_FAILURE);
    }

    // Condicional 6: Valida que el umbral sea un número entero no negativo (>= 0).
    if (!es_entero_no_negativo_estricto(t_str)) {
        fprintf(stderr, "Error: El valor de la flag de umbral (-t) debe ser un numero entero no negativo (ingresado: '%s').\n", t_str);
        exit(EXIT_FAILURE);
    }

    // Condicional 7: Valida que el tamaño de vecindad sea un entero positivo.
    if (!es_entero_positivo_estricto(v_str)) {
        fprintf(stderr, "Error: El tamano de vecindad (-v) debe ser un entero positivo (ingresado: '%s').\n", v_str);
        exit(EXIT_FAILURE);
    }

    // Condicional 8: Comprueba la restricción de imparidad de la vecindad NMS (-v debe ser un número impar >= 1).
    int vecindad_val = atoi(v_str);
    if (vecindad_val % 2 == 0) {
        fprintf(stderr, "Error: El tamano de vecindad (-v) debe ser un numero impar mayor o igual a 1 (ingresado: %d).\n", vecindad_val);
        exit(EXIT_FAILURE);
    }

    // --- SECCIÓN DE INICIALIZACIÓN DEL PIPELINE MULTIPROCESO ---
    
    // Llamada al sistema fork() para crear el primer proceso hijo que ejecutará el nodo 'cargaDatos'.
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error en fork para cargaDatos");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // --- CÓDIGO DEL PROCESO HIJO (Punto de inicio del pipeline) ---
        // Construcción del arreglo dinámico de cadenas 'exec_args' conteniendo la llamada a ./cargaDatos.
        char *exec_args[15];
        int idx = 0;
        exec_args[idx++] = "./cargaDatos";
        exec_args[idx++] = "-i"; exec_args[idx++] = input;
        exec_args[idx++] = "-r"; exec_args[idx++] = r_str;
        exec_args[idx++] = "-t"; exec_args[idx++] = t_str;
        exec_args[idx++] = "-v"; exec_args[idx++] = v_str;
        exec_args[idx++] = "-o"; exec_args[idx++] = output;
        if (debug) { exec_args[idx++] = "-d"; }
        exec_args[idx] = NULL; // Terminación obligatoria en NULL para execv

        // Reemplazo del espacio de direcciones del hijo por el ejecutable './cargaDatos'
        execv(exec_args[0], exec_args);
        
        // Si execv retorna, se produjo un fallo en la ejecución.
        perror("Error al ejecutar execv cargaDatos");
        exit(EXIT_FAILURE);
    } else {
        // --- CÓDIGO DEL PROCESO PADRE (Orquestador principal lab2) ---
        int status;
        
        // Espera bloqueante síncrona a que termine toda la cadena del pipeline (que finalice cargaDatos y sus hijos sucesores).
        waitpid(pid, &status, 0);
        
        // Condicional if: Revisa si el proceso terminó normalmente con un código de salida igual a 0.
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Pipeline ejecutado exitosamente. Resultado guardado en: %s\n", output);
        } else {
            fprintf(stderr, "Ocurrio un error durante la ejecucion del pipeline.\n");
            return WEXITSTATUS(status);
        }
    }

    return 0;
}
