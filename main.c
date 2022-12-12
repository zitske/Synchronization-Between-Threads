#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_BUFFER 5
#define MAX_CONSUMERS 5

// Buffer principal
int buffer[MAX_BUFFER];
// Buffer auxiliar
int bufferAux[MAX_BUFFER];
// Indice de insercao no buffer principal
int ind_insercao;
// Indice de remocao do buffer principal
int ind_remocao;
// Valor de X
int X;

// Semaforos
sem_t full, empty, mutex;

// Funcao para calcular fibonacci
int fibonacci(int n) {
    if(n == 0)
        return 0;
    if(n == 1)
        return 1;
    else
        return (fibonacci(n - 1) + fibonacci(n - 2));
}

// Funcao para verificar se o numero e primo
int primo(int n) {
    int i = 2;
    int primo = 1;

    while (primo && (i < n)) {
        if (n % i == 0)
            primo = 0;
        i++;
    }
    return primo;
}

// Funcao produtora
void *produtora(void *arg) {
    int i;
    for (i = 0; i < 25; i++) {
        sem_wait(&empty);

        // Acesso a area critica
        sem_wait(&mutex);
        int j;
        for (j = 0; j < X; j++) {
            buffer[(ind_insercao + j) % MAX_BUFFER] = fibonacci(i + j);
            printf("PRODUTORA: Inseriu %d no buffer principal\n", buffer[(ind_insercao + j) % MAX_BUFFER]);
        }
        ind_insercao = (ind_insercao + X) % MAX_BUFFER;
        sem_post(&mutex);

        sem_post(&full);
    }
    pthread_exit(NULL);
}

// Funcao consumidora
void *consumidora(void *arg) {
    while (1) {
        sem_wait(&full);

        // Acesso a area critica
        sem_wait(&mutex);
        int n = buffer[ind_remocao];
        printf("CONSUMIDORA: Retirou %d do buffer principal\n", n);
        ind_remocao = (ind_remocao + 1) % MAX_BUFFER;
        sem_post(&mutex);

        sem_post(&empty);

        if (primo(n)) {
            // Acesso a area critica
            sem_wait(&mutex);
            int i;
            for (i = 0; i < MAX_BUFFER; i++) {
                if (n > bufferAux[i]) {
                    int j;
                    for (j = MAX_BUFFER - 1; j > i; j--) {
                        bufferAux[j] = bufferAux[j - 1];
                    }
                    bufferAux[i] = n;
                    break;
                }
            }
            printf("CONSUMIDORA: Inseriu %d no buffer auxiliar\n", n);
            sem_post(&mutex);
        }
    }
    pthread_exit(NULL);
}

// Funcao principal
int main(int argc, char *argv[]) {
    // Verificando se foi passado corretamente os argumentos
    if (argc != 3) {
        printf("Argumentos incorretos.\n");
        printf("Digite ./[nome_programa] [num_consumidores] [X]\n");
        exit(1);
    }
    int N = atoi(argv[1]);
    X = atoi(argv[2]);

    // Inicializando semaforos
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, MAX_BUFFER);
    sem_init(&mutex, 0, 1);

    // Inicializando buffers
    int i;
    for (i = 0; i < MAX_BUFFER; i++) {
        buffer[i] = -1;
        bufferAux[i] = -1;
    }
    ind_insercao = 0;
    ind_remocao = 0;

    // Criando thread produtora
    pthread_t prod;
    pthread_create(&prod, NULL, produtora, NULL);

    // Criando threads consumidoras
    pthread_t cons[MAX_CONSUMERS];
    for (i = 0; i < N; i++) {
        pthread_create(&cons[i], NULL, consumidora, NULL);
    }

    // Aguardando o fim das threads
    pthread_join(prod, NULL);
    for (i = 0; i < N; i++) {
        pthread_join(cons[i], NULL);
    }

    // Destruindo semaforos
    sem_destroy(&full);
    sem_destroy(&empty);
    sem_destroy(&mutex);

    // Exibindo buffer auxiliar
    printf("Conteudo do buffer auxiliar:\n");
    for (i = 0; i < MAX_BUFFER; i++) {
        printf("%d ", bufferAux[i]);
    }
    printf("\n");

    return 0;
}
