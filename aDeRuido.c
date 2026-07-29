#include <stdio.h>
#include <stdlib.h>
#include "faDeRuido.h"

// Entradas: Cantidad de argumentos (int argc), Arreglo de cadenas de argumentos (char *argv[]).
// Salidas: Código de estado entero (0 si finaliza correctamente).
// Descripción: Función principal (main) del nodo aDeRuido. Ejecuta la función procesar_y_guardar_ruido para generar los dumps de depuración preprocesada.bin y ruido.bin.
int main(int argc, char *argv[]) {
    // Cast a void para evitar advertencias del compilador por parámetros no utilizados.
    (void)argc;
    (void)argv;

    // Invocación del procesamiento de ruido y generación de archivos binarios.
    procesar_y_guardar_ruido();
    
    return 0;
}
