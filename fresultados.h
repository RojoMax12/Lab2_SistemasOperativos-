#ifndef FRESULTADOS_H
#define FRESULTADOS_H

// Entradas: Puntero a entero 'ancho', Puntero a entero 'alto'.
// Salidas: Puntero a la matriz acumuladora de enteros (int*) recibida desde la tubería.
// Descripción: Recibe las dimensiones y la matriz acumuladora de votos enviada por el nodo tHough a través del descriptor estándar STDIN_FILENO.
int* recibir_acumulador_pipe(int *ancho, int *alto);

// Entradas: Puntero a la matriz acumuladora (int *acumulador), entero ancho, entero alto, entero umbral, entero vecindad v, cadena de ruta de salida (const char *ruta_salida).
// Salidas: Ninguna (void).
// Descripción: Aplica la Supresión de No Máximos (NMS) sobre un vecindario de v x v píxeles en el acumulador. Escribe las coordenadas detectadas (X,Y) que superan o igualan el umbral tau en el archivo CSV especificado por ruta_salida.
void supresion_no_maximos_y_exportar(int *acumulador, int ancho, int alto, int umbral, int vecindad, const char *ruta_salida);

#endif
