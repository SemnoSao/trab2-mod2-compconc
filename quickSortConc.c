#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

#define MAX_BUFFER_SIZE 100

pthread_mutex_t mutex;				
pthread_cond_t cond_populado;	// informa que o buffer está populado
pthread_cond_t cond_livre;		// informa que o buffer tem espaço livre

static int *v;					// vetor que será ordenado
static pthread_t* tid;	
long int tam;						// tamanho do vetor que será ordenado
// quantiodade de número que já foram ordenados para que o algoritimo saiba qnd deve terminar a execução
int ordenados = 0;			
int _log;								//informa se o log deve ser impresso ou não

// estrutura que aramazena o iniíco e fim (posições) do segmento do vetor a ser trabalhado
typedef struct{
    int ini;
    int* data_set;
    int fim;
} info;

info buffer[MAX_BUFFER_SIZE]; // buffer que será utilizado pelas threads
size_t buffer_lot = 0;				// quantos elementos estão no buffer atualmente

// função simples para trocar dois elementos de posição
void troca(int* a, int* b){
  int t = *a;
  *a = *b;
  *b = t;
}

// função para enfilar um elemento no buffer
void enfila_info(int* vet, int ini, int fim, int id){
	if(_log)printf("enfilou thread %d: ini:%d->fim:%d\n", id, ini, fim);
	pthread_mutex_lock(&mutex);
	
	//não permite enfilar se a fila tiver cheia
	while(buffer_lot+1 >= MAX_BUFFER_SIZE) {
		if(_log)printf("Thread [%d] bloqueada ao inserir!\n", id);
		pthread_cond_wait(&cond_livre, &mutex);
		if(_log)printf("Thread [%d] desbloqueada para inserir!\n", id);
	}
	
	buffer[buffer_lot].data_set = vet; 
	buffer[buffer_lot].ini = ini;				
	buffer[buffer_lot].fim = fim;
	buffer_lot++;
	pthread_cond_signal(&cond_populado); // como um elemento foi inserido sinaliza que o vetor está populado
	pthread_mutex_unlock(&mutex);
}

// função para desenfilar um elemento no buffer.
// ela retorna um elemento do buffer que será usado na função quicksort.
info desenfila_info(long int id){
	info tmp = {.ini = -1, .data_set = NULL, .fim=-1}; // objeto nulo para garantir a finalização do algoritmo
	pthread_mutex_lock(&mutex);
	
	//não permite desenfilar se a fila tiver vazia
	while(buffer_lot == 0 && ordenados < tam) {
		if(_log)printf("Thread [%d] bloqueada ao remover! \n", id);
		pthread_cond_wait(&cond_populado, &mutex);
		if(_log)printf("Thread [%d] desbloqueada para remover! \n", id);
	}
	
	// se o vetor estiver completamente ordenado isso garante que as threads 
	// que ainda estavam esperando elemntos serão liberadas.
	if(ordenados >= tam){
		pthread_mutex_unlock(&mutex);
	 	return tmp;
	}
	
	tmp = buffer[--buffer_lot];
	pthread_cond_signal(&cond_livre); // como um elemento foi retirado sinaliza que o buffer não está cheio
	
	pthread_mutex_unlock(&mutex);
	return tmp;
}

// função do quicksort
void quicksortbufferizado(long int id){
  info seg = desenfila_info(id); // tenta desenfilar um elemento
  //esse trecho garante que as threads que ainda estavam esperando elemntos serão liberadas no fim da execução.
  if(seg.data_set == NULL) return; 
  
  if(_log)printf("começa qs: thread %d\n", id);
  
  int pivot = seg.data_set[seg.fim]; //pega o último elemento da lista como pivo
  int pos = seg.ini;
  
  // em todo segmento, garante que os elementos maiores que o pivo estão a direita e os outros a esquerda
	// trocando os elemntos de posição quando inválido para que seja verdade.
  for(int i = seg.ini; i < seg.fim; i++) {
      if(seg.data_set[i] <= pivot) {
          troca(&seg.data_set[pos], &seg.data_set[i]);
          pos++;
      }
  }
  
  // bota o pivo na posição central
  troca(&seg.data_set[pos], &seg.data_set[seg.fim]);
  
  // se for necessário enfila o segmento a esquerda
  if (seg.ini <= (pos - 1)) enfila_info(seg.data_set, seg.ini, pos - 1, id);
  // se for necessário enfila o segmento a direita
  if ((pos + 1) <= seg.fim) enfila_info(seg.data_set, pos + 1, seg.fim, id);
}

// função das threads
void* run(void *t){
	long int id = (long int)t;  // armazena o id da thread
	pthread_mutex_lock(&mutex);
	int ord = ordenados; 				// utiliza uma varíavel local para manter controle do estado de ordenação do vetor
	pthread_mutex_unlock(&mutex);
	// enquanto o vetor não estiver completamente ordenado
	while (ord < tam){
		quicksortbufferizado(id);   // chama a função quicksort
		pthread_mutex_lock(&mutex);
		// ao final da execução dela incrementa o estado de ordenação, já que o pivot foi ordenado
		ord = ++ordenados;					
		pthread_mutex_unlock(&mutex);
	}
	pthread_cond_broadcast(&cond_populado); //libera as threads que estavam esperando info entrar no buffer
}

int main(int argc, char *argv[]){
	tam = atol(argv[2]);
  _log = atoi(argv[1]);
  FILE *fp;
  double ini, fim;
  int nthreads = atoi(argv[4]); 
  
  fp = fopen(argv[3], "r");
  
  // inicializa o vetor de trabalho e o vetor de controle das threads com os valores informados
  v = (int *)malloc(sizeof(int)*tam); 
  tid = (pthread_t *)malloc(sizeof(pthread_t)*nthreads);

	// lê os números do arquivo e insere no vetor
	for(int i = 0; i < tam; i++)
	{
		fscanf(fp, "%d ", &v[i]);
	}
 
  printf("------Algoritmo Concorrente: %d threads------\n", nthreads);
  
  /* Inicilaiza os controles de thread */
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond_populado, NULL);
  pthread_cond_init(&cond_livre, NULL);
  
  GET_TIME(ini);
  enfila_info(v, 0, tam-1, 0); // enfila o primeiro objeto, tendo inicio no 0 e fim no fim do vetor
  
  // inicia as threads
  for (long int i=0; i<nthreads; i++) {
    if(pthread_create(&tid[i], NULL, run, (void *)i+1)) exit(-1);
  }
  
  // espera pelo fim das threads
  for (int i=0; i<nthreads; i++) {
    pthread_join(*(tid+i), NULL);
  }
  GET_TIME(fim);
  
  // printa o vetor final se pedido log
  if(_log){
		printf("Vetor ordenado concorrente: ");
		for (int i = 0; i < tam; i++){
			printf("%d ", v[i]);
		}
		printf("\n");
	} 
	
	// não printa o tempo de execução quando escrevendo log já que essa informação deixa de ser relevante
	if(!_log)printf("Tempo Concorrente: %lf segundos\n", fim-ini); 
	
	//CORRETUDE
	for(int i=0; i < tam-1; i++){
		// checa para cada elemento do vetor se o próximo é menor, se for retorna um erro.
		if (v[i]>v[i+1]){ 
			printf("ORDEM INCORRETA");
			printf("\n\n");
			exit(1);
		}
	}
	printf("Vetor corretamente ordenado!");
	printf("\n\n");
	//printf("##########################################################\n");
	return 0;
}
