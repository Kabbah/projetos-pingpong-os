// =====================================
// queue.c
// Autor: Victor Barpp Gomes
// Data de início: 17/08/2017
// Data de término: 17/08/2017
// Disciplina: Sistemas Operacionais
// =====================================

#include <stdio.h>
#include "queue.h"

void queue_append(queue_t** queue, queue_t* elem) {
	// Se a fila não existe, aborta.
	if (queue == NULL) {
		printf("Erro queue_append: A fila nao existe.\n");
		return;
	}

	// Se o elemento não existe, aborta.
	if (elem == NULL) {
		printf("Erro queue_append: O elemento nao existe.\n");
		return;
	}

	// Se o elemento já está em uma fila, aborta.
	if (elem->next != NULL || elem->prev != NULL) {
		printf("Erro queue_append: O elemento ja esta em uma fila.\n");
		return;
	}

	// Se a fila está vazia, faz ela apontar para o elemento.
	// Também ajusta os elementos próximo e anterior.
	if (*queue == NULL) {
		*queue = elem;
		elem->next = elem;
		elem->prev = elem;
		return;
	}

	// Se a fila não está vazia, adiciona o elemento no final da fila.
	// Isso é feito acessando o elemento anterior ao primeiro (que é o último, porque a fila é circular).
	elem->prev = (*queue)->prev; // O anterior ao novo elemento é o antigo último elemento.
	elem->next = (*queue);       // O próximo ao novo elemento é o primeiro elemento.
	(*queue)->prev->next = elem; // O próximo ao antigo último elemento é o novo elemento.
	(*queue)->prev = elem;       // O anterior ao primeiro elemento é o novo elemento.

	return;
}

queue_t* queue_remove(queue_t** queue, queue_t* elem) {
	// Se a fila não existe, aborta.
	if (queue == NULL) {
		printf("Erro queue_remove: A fila nao existe.\n");
		return NULL;
	}

	// Se o elemento não existe, aborta.
	if (elem == NULL) {
		printf("Erro queue_remove: O elemento nao existe.\n");
		return NULL;
	}

	queue_t* iterator = (*queue);

	queue_t* nextElem;
	queue_t* prevElem;

	// Se a fila é vazia, retorna NULL.
	if (*queue == NULL) {
		printf("Erro queue_remove: A fila esta vazia.\n");
		return NULL;
	}

	// Verifica se o elemento realmente está na fila fornecida.
	while (iterator != elem) {
		iterator = iterator->next;

		// Caso tenha chegado ao início da fila novamente, não remove o elemento.
		if (iterator == (*queue)) {
			printf("Erro queue_remove: O elemento nao pertence a fila indicada.\n");
			return NULL;
		}
	}

	// Se a fila tem só um elemento, deixa a fila vazia.
	// Também ajusta os elementos próximo e anterior.
	if (elem->next == elem && elem->prev == elem) {
		elem->next = NULL;
		elem->prev = NULL;
		(*queue) = NULL;

		return elem;
	}

	// Se a fila tem mais de um elemento, retira o elemento e "gruda" os pedaços.
	// Se o elemento for o primeiro, reajusta o ponteiro da fila para o segundo.
	if ((*queue) == elem) {
		(*queue) = elem->next;
	}
	nextElem = elem->next;
	prevElem = elem->prev;

	nextElem->prev = prevElem;
	prevElem->next = nextElem;

	elem->next = NULL;
	elem->prev = NULL;

	return elem;
}

int queue_size(queue_t* queue) {
	int i = 0;
	queue_t* iterator;

	// Se a fila é nula, então tem zero elementos.
	if (queue == NULL) {
		return 0;
	}

	iterator = queue->next;

	for (i = 1; iterator != queue; iterator = iterator->next, i++);

	return i;
}

void queue_print(char* name, queue_t* queue, void print_elem(void*)) {
	queue_t* iterator = queue;

	printf(name);
	printf("[");

	// Se a fila é nula, então aborta.
	if (queue == NULL) {
		printf("]\n");
		return;
	}

	// Caso externo (só para não printar o primeiro espaço a esquerda)
	print_elem(iterator);
	iterator = iterator->next;

	while(iterator != queue) {
		printf(" ");
		print_elem(iterator);
		iterator = iterator->next;
	}

	printf("]\n");
	return;
}