#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define MAX 5

// Buffer principal e auxiliar
int buffer[MAX], buffer_aux[MAX];

// Variável que controla a posição do buffer principal
int in = 0;

// Variável que controla a posição do buffer auxiliar
int out = 0;

int N;
int X;

// Variável que controla o término das threads
int end = 0;

// Mutex para controlar o acesso aos buffers
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Condição para bloquear/desbloquear as threads
pthread_cond_t cond_prod = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_cons = PTHREAD_COND_INITIALIZER;

// Gera um número da série de Fibonacci
int fibonacci(int n) {
	if (n == 0) return 0;
	if (n == 1) return 1;
	return fibonacci(n-1) + fibonacci(n-2);
}

// Verifica se o número é primo
int check_prime(int n) {
	int i;
	for (i = 2; i*i <= n; i++) {
		if (n % i == 0) return 0;
	}
	return 1;
}

// Thread produtora
void * produtor() {
	int i, j;
	int val;
	for (i = 0; i < 25; i += X) {
		val = fibonacci(i);
		
		// Bloqueia a thread caso não haja espaço no buffer
		pthread_mutex_lock(&mutex);
		while((in + X) % MAX == out) {
			pthread_cond_wait(&cond_prod, &mutex);
		}
		
		// Insere os números no buffer
		for (j = 0; j < X; j++) {
			if (in == MAX) in = 0;
			buffer[in] = val;
			in++;
			val++;
		}

		// Desbloqueia as threads consumidoras
		pthread_cond_broadcast(&cond_cons);
		pthread_mutex_unlock(&mutex);
	}
	
	// Termina o programa
	end = 1;
	
	// Desbloqueia as threads consumidoras
	pthread_cond_broadcast(&cond_cons);
	pthread_mutex_unlock(&mutex);
	
	pthread_exit(NULL);
}

// Thread consumidora
void * consumidor(void *arg) {
	int id = *(int *)arg;
	
	int val;
	
	while (!end || in != out) {
		// Bloqueia a thread caso não haja elementos no buffer
		pthread_mutex_lock(&mutex);
		while(in == out && !end) {
			pthread_cond_wait(&cond_cons, &mutex);
		}
		
		// Remove o elemento do buffer principal
		if (out == MAX) out = 0;
		val = buffer[out];
		out++;
		
		// Insere o elemento no buffer auxiliar, se for primo
		if (check_prime(val)) {
			if (id == 0) {
				buffer_aux[id] = val;
			} else {
				int i;
				for (i = 0; i < id; i++) {
					if (buffer_aux[i] > val) {
						int j;
						for (j = id; j > i; j--) {
							buffer_aux[j] = buffer_aux[j-1];
						}
						buffer_aux[i] = val;
						break;
					}
				}
				if (i == id) buffer_aux[id] = val;
			}
		}
		
		// Desbloqueia a thread produtora
		pthread_cond_signal(&cond_prod);
		pthread_mutex_unlock(&mutex);
	}
	
	pthread_exit(NULL);
}

int main(int argc, char **argv) {
	int i;
	
	if (argc != 3) {
		printf("Número incorreto de parâmetros!\n");
		return 0;
	}
	
	N = atoi(argv[1]);
	X = atoi(argv[2]);
	
	if (X < 1 || X > 5) {
		printf("Valor de X incorreto!\n");
		return 0;
	}
	
	pthread_t prod, cons[N];
	int ids[N];
	
	// Inicia a thread produtora
	pthread_create(&prod, NULL, produtor, NULL);
	
	// Inicia as threads consumidoras
	for (i = 0; i < N; i++) {
		ids[i] = i;
		pthread_create(&cons[i], NULL, consumidor, &ids[i]);
	}
	
	// Exibe a situação dos buffers
	while (!end || in != out) {
		printf("Buffer principal: ");
		for (i = 0; i < MAX; i++) {
			printf("%d ", buffer[i]);
		}
		printf("\nBuffer auxiliar: ");
		for (i = 0; i < N; i++) {
			printf("%d ", buffer_aux[i]);
		}
		printf("\n\n");
	}
	
	// Faz join nas threads
	pthread_join(prod, NULL);
	for (i = 0; i < N; i++) {
		pthread_join(cons[i], NULL);
	}
	
	// Libera o mutex
	pthread_mutex_destroy(&mutex);
	
	return 0;
}
