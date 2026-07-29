CC=gcc
CFLAGS=-Wall -Wextra
LIBS=-lm

TARGETS=lab2 cargaDatos preprocesamiento aDeRuido tHough resultados

all: $(TARGETS)

lab2: lab2.c
	$(CC) $(CFLAGS) -o lab2 lab2.c $(LIBS)

cargaDatos: cargaDatos.c fcargaDatos.c fcargaDatos.h
	$(CC) $(CFLAGS) -o cargaDatos cargaDatos.c fcargaDatos.c $(LIBS)

preprocesamiento: preprocesamiento.c fpreprocesamiento.c fpreprocesamiento.h
	$(CC) $(CFLAGS) -o preprocesamiento preprocesamiento.c fpreprocesamiento.c $(LIBS)

aDeRuido: aDeRuido.c faDeRuido.c faDeRuido.h
	$(CC) $(CFLAGS) -o aDeRuido aDeRuido.c faDeRuido.c $(LIBS)

tHough: tHough.c ftHough.c ftHough.h
	$(CC) $(CFLAGS) -o tHough tHough.c ftHough.c $(LIBS)

resultados: resultados.c fresultados.c fresultados.h
	$(CC) $(CFLAGS) -o resultados resultados.c fresultados.c $(LIBS)

clean:
	rm -f *.o $(TARGETS) preprocesada.bin ruido.bin