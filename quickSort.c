#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

void troca(int* a, int* b){
  int t = *a;
  *a = *b;
  *b = t;
}

void quicksort(int* vet, int inicio, int fim){
  if (inicio < fim){
    int pivot = vet[fim];
    size_t pos = inicio;
    
    for(size_t i = inicio; i < fim; i++) {
        if(vet[i] <= pivot) {
            troca(&vet[pos], &vet[i]);
            pos++;
        }
    }
    
    troca(&vet[pos], &vet[fim]);
    
    quicksort(vet, inicio, pos - 1);
    quicksort(vet, pos + 1, fim);
  }
}

int main(int argc, char *argv[]){
	int tam = atoi(argv[2]);
  int vet[tam];
  int log = atol(argv[1]);
  FILE *fp;
  double ini, fim;
  
  fp = fopen(argv[3], "r");

	printf("##########################################################\n\n");
	printf("Vetor incial: ");
	for(int i = 0; i < tam; i++)
	{
		fscanf(fp, "%d ", &vet[i]);
	}
	if(log) printf("\n\n");
	else printf("%d números\n\n", tam);
 
  printf("------------Algoritmo sequencial------------\n");
  
  GET_TIME(ini);
  quicksort(vet, 0, tam-1);
  GET_TIME(fim);
  
  if(log){
		printf("Vetor ordenado sequencial: ");
		for (int i = 0; i < tam; i++){
			printf("%d ", vet[i]);
		}
		printf("\n");
	}
	
	if(!log)printf("Tempo Sequencial: %lf segundos\n", fim-ini); 
	//CORRETUDE
	for(int i=0; i < tam-1; i++){
		if (vet[i]>vet[i+1]) exit(1);
	}
	printf("Vetor corretamente ordenado!");
	printf("\n\n");
	//printf("##########################################################\n");
	return 0;
}
