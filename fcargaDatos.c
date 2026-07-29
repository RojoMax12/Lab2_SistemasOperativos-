#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "fcargaDatos.h"

// Entradas: Ruta del archivo binario (.bin) en el sistema de archivos (char *ruta).
// Salidas: Estructura Imagen con los miembros ancho, alto y puntero pixeles cargados en memoria dinámica.
// Descripción: Abre el archivo binario, lee la cabecera de 8 bytes (2 enteros de 4 bytes para ancho y alto) y asigna memoria dinámica para leer el bloque completo de píxeles.
Imagen leer_imagen(char *ruta) {
    // Variable 'archivo': Puntero de archivo para manejar la lectura binaria mediante fopen.
    FILE *archivo = fopen(ruta, "rb");
    
    // Variable 'img': Estructura local donde se almacenará el ancho, alto y el arreglo de píxeles.
    Imagen img = {0, 0, NULL};

    // Condicional if: Verifica si el archivo se abrió correctamente. Si fopen retorna NULL, la ruta es inválida.
    if (archivo == NULL) {
        perror("Error al abrir el archivo de entrada");
        exit(EXIT_FAILURE); // Finaliza con código de falla si no se puede abrir.
    }

    // Condicional if: Lee la cabecera del archivo binario (primeros 8 bytes).
    // Se leen 2 enteros de 4 bytes: ancho y alto. Si fread no retorna 1 para cada entero, el archivo está corrupto.
    if (fread(&img.ancho, sizeof(int), 1, archivo) != 1 ||
        fread(&img.alto, sizeof(int), 1, archivo) != 1) {
        fprintf(stderr, "Error al leer dimensiones de la imagen %s\n", ruta);
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    // Variable 'total_pixeles': Calcula el tamaño total de la matriz de píxeles (ancho * alto).
    size_t total_pixeles = (size_t)img.ancho * img.alto;
    
    // Asignación de memoria dinámica para el arreglo de píxeles usando malloc.
    img.pixeles = (unsigned char *)malloc(total_pixeles);
    
    // Condicional if: Revisa si la memoria se asignó correctamente en el Heap.
    if (img.pixeles == NULL) {
        fprintf(stderr, "Error al asignar memoria para pixeles\n");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    // Condicional if: Lee los 'total_pixeles' bytes del archivo directamente al buffer asignado.
    if (fread(img.pixeles, sizeof(unsigned char), total_pixeles, archivo) != total_pixeles) {
        fprintf(stderr, "Error al leer la matriz de pixeles de %s\n", ruta);
        free(img.pixeles);
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    // Cierra el descriptor del archivo después de completar la lectura.
    fclose(archivo);
    
    // Retorna la estructura con los datos cargados.
    return img;
}

// Entradas: Estructura Imagen conteniendo las dimensiones y el arreglo de píxeles cargado.
// Salidas: Ninguna (void).
// Descripción: Transmite las dimensiones (ancho y alto) y el bloque de píxeles hacia el descriptor STDOUT (que está redirigido mediante tubería/pipe al siguiente nodo).
void enviar_imagen_pipe(Imagen img) {
    // Condicional if: Envía primero las dimensiones (ancho y alto) por el descriptor STDOUT_FILENO (8 bytes).
    // Si la llamada write no escribe exactamente 4 bytes por cada entero, se genera un error.
    if (write(STDOUT_FILENO, &img.ancho, sizeof(int)) != sizeof(int) ||
        write(STDOUT_FILENO, &img.alto, sizeof(int)) != sizeof(int)) {
        fprintf(stderr, "Error al escribir dimensiones en el pipe STDOUT\n");
        exit(EXIT_FAILURE);
    }

    // Variable 'total_pixeles': Número total de bytes a transmitir en el cuerpo de la imagen.
    size_t total_pixeles = (size_t)img.ancho * img.alto;
    
    // Variable 'escritos': Contador acumulativo de bytes transmitidos por la tubería.
    size_t escritos = 0;
    
    // Ciclo while: Garantiza la transmisión completa del buffer en la tubería IPC.
    // write() puede transmitir menos bytes de los solicitados si el buffer del sistema operativo se llena temporalmente.
    while (escritos < total_pixeles) {
        ssize_t bytes = write(STDOUT_FILENO, img.pixeles + escritos, total_pixeles - escritos);
        
        // Condicional if: Si write retorna un valor <= 0, ocurrió un error o la tubería se cerró inesperadamente.
        if (bytes <= 0) {
            fprintf(stderr, "Error al escribir pixeles en el pipe STDOUT\n");
            exit(EXIT_FAILURE);
        }
        
        // Acumula la cantidad de bytes escritos exitosamente.
        escritos += (size_t)bytes;
    }
}
