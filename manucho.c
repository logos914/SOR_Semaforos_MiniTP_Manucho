#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#define CANTIDAD_INVITADOS 5
#define CANTIDAD_MOZOS 3
#define CUANTO_TARDA_EN_COMER 999999999
int cantidadInvitados;
int cantidadMozos;
sem_t manuSePuedeSentar;
sem_t mozosPuedenServir;
sem_t puedenComer;
int invitadosSentados = 0;
sem_t semaforoInvitadosSentados;

int platosQueFaltanServir;
sem_t semaforoPlatos;

bool seHaRespondido = false;
sem_t semaforoRespuesta;
sem_t hayPregunta;
sem_t hayRespuesta;
sem_t sePuedenIr;

void sentarse(char);
void servircomida();
void enojarse();
void levantarse(char);
void lanzar_pregunta_mundialista();
void lanzar_respuesta_mundialista();

void sentarse(char quienSeSienta)
{
  if (quienSeSienta == 'm')
  {
    printf("Manucho se ha sentado e indica a los mozos que pueden servir\n");
  }
  else if (quienSeSienta == 'i')
  {
    printf("Se sienta un invitado\n");
  }
}

void servircomida()
{
  for (int i = 0; i < CUANTO_TARDA_EN_COMER; i++)
  {
    int hagoOperaciones = i + 1;
  }
  printf("Un mozo ha servidor un plato\n");
}

void lanzar_pregunta_mundialista()
{
  printf("Manucho hizo la pregunta\n");
}

void lanzar_respuesta_mundialista()
{
  printf("Han respondido la pregunta de futbol\n");
}

void enojarse()
{
  printf("Manucho ha golpeado la mesa furioso\n");
}

void levantarse(char quienSeLevanta)
{
  if (quienSeLevanta == 'm')
  {
    printf("Manucho se ha levantado\n");
  }
  else if (quienSeLevanta == 'i')
  {
    printf("Se ha retirado de la mesa un invitado\n");
  }
}

void *tareas_de_manucho(void *arg)
{

  sem_wait(&manuSePuedeSentar);
  sentarse('m');

  // Permitir que los mozos puedan servir los platos necesarios
  for (int i = 0; i < cantidadInvitados; i++)
  {
    sem_post(&mozosPuedenServir);
  }

  sem_wait(&puedenComer);
  printf("Manucho ha comenzado a comer\n");

  // Un poco de tiempo para simular que Manucho almuerza
  for (int i = 0; i < CUANTO_TARDA_EN_COMER; i++)
  {
    int hagoOperaciones = i + 1;
  }
  printf("Manucho ha terminado de comer\n");

  lanzar_pregunta_mundialista();
  sem_post(&hayPregunta);

  sem_wait(&hayRespuesta);
  enojarse();

  // Permitir a todos los invitados retirarse
  for (int i = 0; i < cantidadInvitados; i++)
  {
    sem_post(&sePuedenIr);
  }

  levantarse('m');
}

void *tareas_de_invitado(void *arg)
{

  sem_wait(&semaforoInvitadosSentados);
  sentarse('i');
  invitadosSentados++;

  // El último invitado en sentarse, habilita a Manucho a sentarse
  if (invitadosSentados == cantidadInvitados)
  {
    sem_post(&manuSePuedeSentar);
  }

  sem_post(&semaforoInvitadosSentados);

  sem_wait(&puedenComer);
  printf("Un invitado ha comenzado de comer\n");

  // Simular un tiempo en que está almorzando
  for (int i = 0; i < CUANTO_TARDA_EN_COMER; i++)
  {
    int hagoOperaciones = i + 1;
  }
  printf("Un invitado ha terminado de comer\n");

  sem_wait(&hayPregunta);
  sem_wait(&semaforoRespuesta);

  if (!seHaRespondido)
  {

    seHaRespondido = true;
    sem_post(&hayRespuesta);
    lanzar_respuesta_mundialista();

    // Permitir a los demás invitados
    for (int i = 0; i < cantidadInvitados; i++)
    {
      sem_post(&hayPregunta);
      sem_post(&semaforoRespuesta);
    }
  }

  sem_wait(&sePuedenIr);
  levantarse('i');
}

void *tareas_de_mozo(void *arg)
{
  sem_wait(&mozosPuedenServir);

  while (true)
  {
    sem_wait(&semaforoPlatos);

    if (platosQueFaltanServir > 0)
    {
      platosQueFaltanServir--;
      sem_post(&semaforoPlatos);
      servircomida();
      sem_post(&puedenComer);
    }
    else
    {
      sem_post(&semaforoPlatos);
      break;
    }
  }
}

int main(int argc, char **argv)
{

  // Analizamos los argumentos para determinar cuantos invitados hay.

  if (argc == 2 && atoi(argv[1]) > 1)
  {
    cantidadInvitados = atoi(argv[1]);
    cantidadMozos = atoi(argv[1]) - 1;
  }
  else if (argc == 3 && atoi(argv[1]) > 1 && atoi(argv[2]) < atoi(argv[1]))
  {
    cantidadInvitados = atoi(argv[1]);
    cantidadMozos = atoi(argv[2]);
  }
  else
  {
    cantidadInvitados = CANTIDAD_INVITADOS;
    cantidadMozos = CANTIDAD_MOZOS;
  }

  platosQueFaltanServir = cantidadInvitados + 1;

  sem_init(&manuSePuedeSentar, 0, 0);
  sem_init(&semaforoInvitadosSentados, 0, 1);
  sem_init(&mozosPuedenServir, 0, 0);
  sem_init(&puedenComer, 0, 0);
  sem_init(&semaforoPlatos, 0, 1);
  sem_init(&semaforoRespuesta, 0, 1);
  sem_init(&hayPregunta, 0, 0);
  sem_init(&hayRespuesta, 0, 0);
  sem_init(&sePuedenIr, 0, 0);

  pthread_t anfitrion;

  pthread_t invitado[cantidadInvitados];
  pthread_t mozo[cantidadMozos];

  pthread_create(&anfitrion, NULL, *tareas_de_manucho, NULL);

  for (int i = 0; i < cantidadInvitados; i++)
  {
    pthread_create(&invitado[i], NULL, *tareas_de_invitado, NULL);

    if (i < cantidadMozos)
    {
      pthread_create(&mozo[i], NULL, *tareas_de_mozo, NULL);
    }
  }

  for (int i = 0; i < cantidadInvitados; i++)
  {
    pthread_join(invitado[i], NULL);

    if (i < cantidadMozos)
    {
      pthread_join(mozo[i], NULL);
    }
  }

  pthread_join(anfitrion, NULL);

  return 0;
}
