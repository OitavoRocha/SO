#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include<semaphore.h>

#define N 6 // numero de cadeiras de espera
#define M 4 // numero de barbeiros

int clientesAtendidos = 0;
int clientesDesistiram = 0;
int cadeirasDisponiveis = N;
int MAX_CLIENTES;

sem_t mutex;
sem_t barbeiros;
sem_t clientes;
sem_t pentes;
sem_t tesouras;

void * barbeiro ( void *arg );
void * cliente ( void *arg );

int main () {
    int idsClientes[MAX_CLIENTES];
    int tempoChegada = 0;

    printf("Iniciando barbearia\n");
    printf("Informe a quantidade máxima de clientes: ");
    scanf("%d", &MAX_CLIENTES);

    pthread_t tBarbeiros[M];
    pthread_t tClientes[MAX_CLIENTES];

    sem_init( &pentes, 0, M/2 );
    sem_init( &tesouras, 0, M/2 );
    sem_init( &mutex, 0, 1 );
    sem_init( &barbeiros, 0, 0 );
    sem_init( &clientes, 0, 0 );

    for ( int  i = 0 ; i < M ; i++ ) {
        pthread_create( &tBarbeiros[i], NULL, barbeiro, NULL );
    }

    for ( int i = 0 ; i < MAX_CLIENTES ; i++ ) {
        idsClientes[i] = i + 1;
        pthread_create( &tClientes[i], NULL, cliente, &idsClientes[i] );
        sleep(rand() % 4 + 1); // tempo de chegada do cliente
    }

    for ( int i = 0 ; i < N + M ; i++ ) {
        pthread_join( tClientes[i], NULL );
    }

    printf("Clientes atendidos: %d\n", clientesAtendidos);
    printf("Clientes desistiram: %d\n", clientesDesistiram);

    return 0;
}

void * barbeiro( void *arg ) {

    sem_wait( &mutex );
    while( 1 ) {
        if ( cadeirasDisponiveis == N && MAX_CLIENTES == clientesAtendidos + clientesDesistiram ) {
            sem_post( &mutex );
            pthread_exit(0);
        }
        sem_wait( &clientes );
        sem_wait( &mutex );
        cadeirasDisponiveis++;
        sem_post( &barbeiros );
        sem_post( &mutex );
    }
    
    return NULL;
}

void * cliente( void *arg ) {
    int idCliente = *((int *) arg);
    int tempoCorte = rand() % 4 + 3;

    sem_wait( &mutex );
    if ( MAX_CLIENTES == clientesAtendidos + clientesDesistiram ) {
        sem_post( &mutex );
        pthread_exit(0);
    } else {
        if ( cadeirasDisponiveis > 0 ) {
            cadeirasDisponiveis--;
            sem_post( &clientes );
            sem_post( &mutex );
            sem_wait( &barbeiros );
            printf("Cliente %d cortando cabelo\n", idCliente);
            sleep(tempoCorte);
            sem_wait( &mutex );
            clientesAtendidos++;
            sem_post( &mutex );
        } else {
            printf("Cliente %d desistiu\n", idCliente);
            clientesDesistiram++;
            sem_post( &mutex );
        }
    }

    return NULL;
}
