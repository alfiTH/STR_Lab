#include <stdlib.h>
#include <stdio.h>
#include "funciones_time.h"
#include <ClasesPosix.h>
//Añadir la librería de utilización de señales
#include <signal.h>
//Añadir el include de los relojes
#include <time.h>

void imprimir_senial_capturada(int capturadas, int max_capturas) {
	printf("Se han capturado %d señales SIGINT de %d.\n", capturadas, max_capturas);
}

void imprimir_fecha_hora(int dia, int mes, int anio, int hora, int minuto, int segundos, int milisegundos) {
	printf("%d/%d/%d - %d:%d:%d.%d\n", dia, mes, anio, hora, minuto, segundos, milisegundos);
}

//Definir un tipo de datos seniales_t que sea una estructura con dos campos de tipo entero, uno para almacenar las señales recibidas y otro para cantidad de almacenar las señales esperadas

typedef struct 
{
	int coutRecibidas;
	int coutEsperadas;
}seniales_t;


/*Definir la función HiloSigInt para la creación de un hilo (seguir el formato indicado para este fin, ver el anexo I de la práctica) que capture la señal SIGINT.
Tendrá un parámetro de entrada de tipo puntero a seniales_t (esta estructura es compartida por todos los hilos, de ahí que sea un puntero)*/
void *HiloSigInt(void *aux) { 
	int sig;
	//Definir y asignar una variable de tipo puntero a seniales_t (debe ser puntero porque es compartida por todos los hilos)
	seniales_t *counter=(seniales_t *)aux;
	//Crear un conjunto de señales
	sigset_t set;
	//Inicializar el conjunto de señales a vacío
	sigemptyset(&set);
	//Añadir SIGINT y SIGUSR1 al conjunto de señales
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGUSR1);
	//Añadir el conjunto de señales a la máscara de señales bloqueadas
	pthread_sigmask(SIG_BLOCK, &set, NULL);
	//Quitar SIGUSR1 del conjunto de señales. Esto es necesario ya que, aunque bloqueamos las dos señales, en este hilo solamente esperamos por SIGINT
	sigdelset(&set, SIGUSR1);
	//Mientras las señales recibidas no lleguen a las esperadas
	while (counter->coutRecibidas < counter->coutEsperadas)
		{
			//Esperar por el conjunto de señales, que solamente contiene SIGINT (habrá que crear una variable para usarla como segundo parámetro de la función sigwait)
			if(sigwait(&set, &sig)!= 0)pthread_exit((void*)-1);
			//Incrementar las señales recibidas
			if(sig == SIGINT)
				counter->coutRecibidas++;
		}
	return nullptr;
}
	


/*Definir la función HiloPeriodicoConRetardos para la creación de un hilo (seguir el formato indicado para este fin, ver el anexo I de la práctica) que, mediante retardos,
muestre la hora de diferentes relojes. Tendrá un parámetro de entrada de tipo puntero a seniales_t (esta estructura es compartida por todos los hilos, de ahí que sea un puntero)*/
void *HiloPeriodicoConRetardos(void *aux) { 
	int sig;
	//Definir y asignar una variable de tipo puntero a seniales_t (debe ser puntero porque es compartida por todos los hilos)
	seniales_t *counter=(seniales_t *)aux;
	//Crear un conjunto de señales
	sigset_t set;
	//Inicializar el conjunto de señales a vacío
	sigemptyset(&set);
	//Añadir SIGINT y SIGUSR1 al conjunto de señales
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGUSR1);
	//Añadir el conjunto de señales a la máscara de señales bloqueadas
	pthread_sigmask(SIG_BLOCK, &set, NULL);

	//Definir tres variables de tipo struct timespec para almacenar la lectura de la hora actual, el instante siguiente y el periodo de repetición
	timespec t1, t2, period;
	/*Definir una variable de tipo puntero a struct tm para almacenar la transformación de la hora en formato calendario (no se debe reservar memoria).
	Debe ser de tipo puntero porque la función localtime devuelve la dirección de memoria donde se almacena la conversión.*/
	tm *calendar;
	//Leer la hora actual usando el reloj CLOCK_MONOTONIC
	clock_gettime(CLOCK_MONOTONIC, &t1);
	//Inicializar el periodo de repetición a 5 segundos (hay que inicializar también los nanosegundos)
	period.tv_sec = 5;
	period.tv_nsec = 0; /* 5 s */
	
	//Calcular el siguiente instante como la suma de la hora actual y el periodo de repetición (usar las funciones definidas en la librería funciones_time)
	t2 = SumaTimeSpec(t1, period);
	//Mientras las señales recibidas no lleguen a las esperadas
	while (counter->coutRecibidas < counter->coutEsperadas)
	{
		//Leemos la hora actual de CLOCK_MONOTONIC, Transformamos la hora actual a formato struct tm y mostramos
		clock_gettime(CLOCK_MONOTONIC, &t1);
		calendar=localtime(&t1.tv_sec);
		imprimir_fecha_hora(calendar->tm_mday, calendar->tm_mon+1, calendar->tm_year+1900, calendar->tm_hour, calendar->tm_min, calendar->tm_sec, t1.tv_nsec/1000000);
		
		//Leemos la hora actual de CLOCK_REALTIME, Transformamos la hora actual a formato struct tm y mostramos
		clock_gettime(CLOCK_REALTIME, &t1);
		calendar=localtime(&t1.tv_sec);
		imprimir_fecha_hora(calendar->tm_mday, calendar->tm_mon+1, calendar->tm_year+1900, calendar->tm_hour, calendar->tm_min, calendar->tm_sec, t1.tv_nsec/1000000);

		//Leemos la hora actual de CLOCK_THREAD_CPUTIME_ID, Transformamos la hora actual a formato struct tm y mostramos
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t1);
		calendar=localtime(&t1.tv_sec);
		imprimir_fecha_hora(calendar->tm_mday, calendar->tm_mon+1, calendar->tm_year+1900, calendar->tm_hour, calendar->tm_min, calendar->tm_sec, t1.tv_nsec/1000000);
		//Crear un bucle que realice 100000000 de vueltas
		for (int i=0; i< 100000000; i++)
			;

		//Dormir el hilo hasta el siguiente instante calculado usando CLOCK_MONOTONIC y retardo absoluto
		if (clock_nanosleep (CLOCK_MONOTONIC, TIMER_ABSTIME, &t2, NULL)) pthread_exit((void*)-1);
		//Calcular el siguiente instante
		t2 = SumaTimeSpec(t2, period);
		
	}
	return nullptr;
}


/*Definir la función HiloPeriodicoConTemporizador para la creación de un hilo (seguir el formato indicado para este fin, ver el anexo I de la práctica) que, mediante un temporizador,
muestre las señales capturadas actualmente. Tendrá un parámetro de entrada de tipo puntero a seniales_t (esta estructura es compartida por todos los hilos, de ahí que sea un puntero)*/
void *HiloPeriodicoConTemporizador(void *aux) { 

	int contSenates = 0;
	int sig;
	//Definir y asignar una variable de tipo puntero a seniales_t (debe ser puntero porque es compartida por todos los hilos)
	seniales_t *counter=(seniales_t *)aux;
	//Crear un conjunto de señales
	sigset_t set;
	//Inicializar el conjunto de señales a vacío
	sigemptyset(&set);
	//Añadir SIGINT y SIGUSR1 al conjunto de señales
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGUSR1);
	//Añadir el conjunto de señales a la máscara de señales bloqueadas
	pthread_sigmask(SIG_BLOCK, &set, NULL);
	//Quitar SIGINT del conjunto de señales. Esto es necesario ya que, aunque bloqueamos las dos señales, solamente esperamos por SIGUSR1
	sigdelset(&set, SIGINT);

	//Definir una variable tipo timer_t que sirva de manejador del temporizador
	timer_t man;
	//Definir una variable evento de tipo struct sigevent para indicar la acción del temporizador
	struct sigevent evento;
	//Indicar en la variable evento que el temporizador generará una señal
	evento.sigev_notify = SIGEV_SIGNAL;
	//Indicar en la variable evento que la señal generada por el temporizador es SIGUSR1
	evento.sigev_signo = SIGUSR1;
	//Crear el temporizador usando CLOCK_MONOTONIC, evento y el manejador del temporizador
	timer_create(CLOCK_MONOTONIC, &evento, &man);
	//Definir una variable datos_temporizador de tipo struct itimerspec
	struct itimerspec datos_temporizador;
	//Leer la hora actual de CLOCK_MONOTONIC y almacenarla en el campo it_value de datos_temporizador
	clock_gettime(CLOCK_MONOTONIC, &datos_temporizador.it_value);

	//Añadir dos segundos al campo it_value de datos_temporizador
	datos_temporizador.it_value.tv_sec = 2 + datos_temporizador.it_value.tv_sec;
	//Establecer el campo it_interval de datos_temporizador a 2 segundos (hay que inicializar también los nanosegundos)
	datos_temporizador.it_interval.tv_sec = 2;
	datos_temporizador.it_interval.tv_nsec = 0;
	/*Armar el temporizador usando el manejador del temporizador, retardos absolutos y datos_temporizador (no es necesario guardar el tiempo restante en caso de salida prematura,
	por lo que se puede usar NULL)*/
	timer_settime(man, TIMER_ABSTIME, &datos_temporizador, NULL);

	//Mientras las señales recibidas no lleguen a las esperadas
	while (counter->coutRecibidas < counter->coutEsperadas)
	{
		//Imprimir el número de señales recibidas actualmente (usar la función imprimir_senial_capturada)
		imprimir_senial_capturada(counter->coutRecibidas , counter->coutEsperadas);
		//Esperar por el conjunto de señales, que solamente contiene SIGUSR1 (habrá que crear una variable para usarla como segundo parámetro de la función sigwait)
		if (sigwait(&set, &sig)!= 0)pthread_exit((void*)-1);
	}
	return nullptr;
}

int main (int argc, char *argv[]) {
	if (argc!=2) {
		printf("Error en la ejecución. El formato correcto es %s num_capturas\n", argv[0]);
		return 0;
	}
	int num_capturas=atoi(argv[1]);
	if (num_capturas<1) {
		printf("Error en la ejecución. El número mínimo señales a capturar debe ser 1\n");
		return 0;
	}


	seniales_t seniales;
	seniales.coutEsperadas= atoi(argv[1]);
	seniales.coutRecibidas=0;
	//Crear un conjunto de señales
	sigset_t set;
	//Inicializar el conjunto de señales a vacío
	sigemptyset(&set);
	//Añadir SIGINT y SIGUSR1 al conjunto de señales
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGUSR1);
	//Añadir el conjunto de señales a la máscara de señales bloqueadas
	pthread_sigmask(SIG_BLOCK, &set, NULL);
	//Crear un manejador de hilo para cada uno de los hilos que se van a crear (uno por cada una de las funciones anteriores)
	hilo_t hilo[3];
	//Asignara cada Hilo su función y la variable compartida seniales
	hilo[0].AsignarFuncionYDato(HiloSigInt, &seniales);
	hilo[1].AsignarFuncionYDato(HiloPeriodicoConRetardos, &seniales);
	hilo[2].AsignarFuncionYDato(HiloPeriodicoConTemporizador, &seniales);

	//Lanzamos todos los Hilos
	for (int i =0; i<3; i++)
		hilo[i].Lanzar();

	//Esperamos todos lo Hilos
	for (int i =0; i<3; i++)
		hilo[i].Join();

}
