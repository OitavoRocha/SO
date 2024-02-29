#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include<semaphore.h>

#define N 8 // numero de cadeiras de espera
#define M 3 // numero de barbeiros

int clientesAtendidos = 0;
int clientesDesistiram = 0;
int cadeirasDisponiveis = N;
int MAX_CLIENTES;
int numClientes = 0;

sem_t mutex;
sem_t barbeiros;
sem_t clientes;
sem_t pentes;
sem_t tesouras;

void * barbeiro ( void *arg );
void * cliente ( void *arg );

int main () {
    printf("Iniciando barbearia\n");
    printf("Informe a quantidade maxima de clientes: ");
    scanf("%d", &MAX_CLIENTES);


    int idsClientes[MAX_CLIENTES];
    int idBarbeiros[M];
    pthread_t tBarbeiros[M];
    pthread_t tClientes[MAX_CLIENTES];

    sem_init( &pentes, 0, M/2 );
    sem_init( &tesouras, 0, M/2 );
    sem_init( &mutex, 0, 1 );
    sem_init( &barbeiros, 0, 0 );
    sem_init( &clientes, 0, 0 );

    for ( int  i = 0 ; i < M ; i++ ) {
        idBarbeiros[i] = i + 1;
        pthread_create( &tBarbeiros[i], NULL, barbeiro, &idBarbeiros[i] );
    }

    for ( int i = 0 ; i < MAX_CLIENTES ; i++ ) {
        idsClientes[i] = i + 1;
        pthread_create( &tClientes[i], NULL, cliente, &idsClientes[i] );
        sleep( rand() % 2 + 1 ); // tempo de chegada do cliente
    }

    for ( int i = 0 ; i < MAX_CLIENTES ; i++ ) {
        pthread_join( tClientes[i], NULL );
        //pthread_join( tBarbeiros[i], NULL );
    }

    printf("Clientes atendidos: %d\n", clientesAtendidos);
    printf("Clientes desistiram: %d\n", clientesDesistiram);

    return 0;
}

void * barbeiro( void *arg ) {

    while( 1 ) {
        sem_wait( &mutex );
        if ( cadeirasDisponiveis == N && numClientes == MAX_CLIENTES ) {
            printf("Barbeiro %d encerrando expediente\n", *((int *) arg));
            sem_post( &mutex );
            pthread_exit(0);
        }
        sem_post( &mutex );
        printf("Barbeiro %d esperando Cliente\n", *((int *) arg));
        sem_wait( &clientes );
        sem_wait( &tesouras);
        sem_wait( &pentes);
        printf("Barbeiro %d esperando por equipamento\n", *((int *) arg));
        sem_wait( &mutex );
        cadeirasDisponiveis++;
        sem_post( &tesouras );
        sem_post( &pentes );
        sem_post( &mutex );
        sem_post( &barbeiros ); //terminou de cortar
    }
    
    return NULL;
}

void * cliente( void *arg ) {
    int idCliente = *((int *) arg);
    int tempoCorte = rand() % 4 + 3;

    
    
    sem_wait( &mutex );
    printf("Cliente %d chegou", idCliente);
    numClientes++;

    if ( cadeirasDisponiveis > 0 ) {
        printf(" e esta esperando\n");
        cadeirasDisponiveis--;
        printf("Cadeiras disponiveis: %d\n", cadeirasDisponiveis);
        sem_post( &mutex );
        sem_post( &clientes );
        sem_wait( &barbeiros );
        printf("Cliente %d cortando cabelo\n", idCliente);
        sem_wait( &mutex );
        clientesAtendidos++;
        sleep(tempoCorte);
        sem_post( &mutex );
        printf("Cliente %d cortou o cabelo\n", idCliente);
    } else  {
        printf(" e desistiu\n");
        clientesDesistiram++;
        sem_post( &mutex );
    }
    

    return NULL;
}
