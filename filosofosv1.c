#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>

volatile int runnig = 1;
int *ids;
int *eat;
int *think;
int *tried;
int numthr;

pthread_mutex_t *mutex;

void eating(void *id) {
    int threadId = *(int *)id;
    int previousFork = threadId - 1;
    if (previousFork < 0)
        previousFork = numthr - 1;

    eat[threadId] += 1;
    usleep(2000); // 2 microsegundos
}

void thinking(void *id) {
    int threadId = *(int *)id;
    think[threadId] += 1;
    sleep(5); // 5 segundos
}

void tryEating(void *id) {
    int threadId = *(int *)id;
    tried[threadId] += 1;
    int r = rand() % 4; // numero aleatorio entre 0 y 3
    sleep(r); // entre 0 a 3 segundos
}

void *dinner(void *id) {
    int threadId = *(int *)id;

    while (runnig) {

        // intenta tomar tenedor
        if (pthread_mutex_trylock(&mutex[threadId]) == 0) {
            int previousFork = threadId - 1;

            // ciclo para la mesa
            if (previousFork < 0)
                previousFork = numthr;

            // intenta tomar tenedor anterior
            if (pthread_mutex_trylock(&mutex[previousFork]) == 0) {
                eating(id);

                // Deja los 2 tenedores
                pthread_mutex_unlock(&mutex[threadId]);
                pthread_mutex_unlock(&mutex[previousFork]);

                // pensando
                thinking(id);
            }
            // Si no es así, libera los tenedores
            else {
                pthread_mutex_unlock(&mutex[threadId]);
                tryEating(id);
            }
        }
        else
            tryEating(id);
    }
}

int main(int argc, char **argv) {

    pthread_t *philosopher;
    int i, runningTime;

    numthr = 5;       // filosofos
    runningTime = 1; // tiempo de ejecucion

    ids = (int *)malloc(numthr * sizeof(int));
    eat = (int *)malloc(numthr * sizeof(int));
    think = (int *)malloc(numthr * sizeof(int));
    tried = (int *)malloc(numthr * sizeof(int));

    // inicializa contadores
    for (i = 0; i < numthr; i++) {
        ids[i] = i;
        eat[i] = tried[i] = think[i] = 0;
    }

    philosopher = (pthread_t *)malloc(numthr * sizeof(pthread_t));
    mutex = (pthread_mutex_t *)malloc(numthr * sizeof(pthread_mutex_t));

    //un mutex para cada tenedor
    for (i = 0; i < numthr; i++)
        pthread_mutex_init(&mutex[i], NULL);


    // Crea un Hilo para Cada Filosofo
    for (i = 0; i < numthr; i++) {
        if (pthread_create(&philosopher[i], NULL, dinner, (void *)&ids[i])) {
            fprintf(stderr, "Error creating thread\n");
            return 2;
        }
    }

    sleep(runningTime);
    runnig = 0;

    // Imprime resultado
    for (i = 0; i < numthr; i++) {
        printf("Filosofo %d: \n Pensando %d \n Comiendo %d \n con Hambre %d \n",
               i+1, think[i], eat[i], tried[i]);
    }

    return 0;
}
