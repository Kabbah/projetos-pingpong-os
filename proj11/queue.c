// =====================================
// queue.c
// Autor: Victor Barpp Gomes
// Data de in�cio: 17/08/2017
// Data de t�rmino: 17/08/2017
// Disciplina: Sistemas Operacionais
// =====================================

#include <stdio.h>
#include "queue.h"

void queue_append(queue_t** queue, queue_t* elem) {
    // Se a fila n�o existe, aborta.
    if (queue == NULL) {
#ifdef DEBUG
        printf("Erro queue_append: A fila nao existe.\n");
#endif
        return;
    }

    // Se o elemento n�o existe, aborta.
    if (elem == NULL) {
#ifdef DEBUG
        printf("Erro queue_append: O elemento nao existe.\n");
#endif
        return;
    }

    // Se o elemento j� est� em uma fila, aborta.
    if (elem->next != NULL || elem->prev != NULL) {
#ifdef DEBUG
        printf("Erro queue_append: O elemento ja esta em uma fila.\n");
#endif
        return;
    }

    // Se a fila est� vazia, faz ela apontar para o elemento.
    // Tamb�m ajusta os elementos pr�ximo e anterior.
    if (*queue == NULL) {
        *queue = elem;
        elem->next = elem;
        elem->prev = elem;
        return;
    }

    // Se a fila n�o est� vazia, adiciona o elemento no final da fila.
    // Isso � feito acessando o elemento anterior ao primeiro (que � o �ltimo, porque a fila � circular).
    elem->prev = (*queue)->prev; // O anterior ao novo elemento � o antigo �ltimo elemento.
    elem->next = (*queue);       // O pr�ximo ao novo elemento � o primeiro elemento.
    (*queue)->prev->next = elem; // O pr�ximo ao antigo �ltimo elemento � o novo elemento.
    (*queue)->prev = elem;       // O anterior ao primeiro elemento � o novo elemento.

    return;
}

queue_t* queue_remove(queue_t** queue, queue_t* elem) {
    // Se a fila n�o existe, aborta.
    if (queue == NULL) {
#ifdef DEBUG
        printf("Erro queue_remove: A fila nao existe.\n");
#endif
        return NULL;
    }

    // Se o elemento n�o existe, aborta.
    if (elem == NULL) {
#ifdef DEBUG
        printf("Erro queue_remove: O elemento nao existe.\n");
#endif
        return NULL;
    }

    queue_t* iterator = (*queue);

    queue_t* nextElem;
    queue_t* prevElem;

    // Se a fila � vazia, retorna NULL.
    if (*queue == NULL) {
#ifdef DEBUG
        printf("Erro queue_remove: A fila esta vazia.\n");
#endif
        return NULL;
    }

    // Verifica se o elemento realmente est� na fila fornecida.
    while (iterator != elem) {
        iterator = iterator->next;

        // Caso tenha chegado ao in�cio da fila novamente, n�o remove o elemento.
        if (iterator == (*queue)) {
#ifdef DEBUG
            printf("Erro queue_remove: O elemento nao pertence a fila indicada.\n");
#endif
            return NULL;
        }
    }

    // Se a fila tem s� um elemento, deixa a fila vazia.
    // Tamb�m ajusta os elementos pr�ximo e anterior.
    if (elem->next == elem && elem->prev == elem) {
        elem->next = NULL;
        elem->prev = NULL;
        (*queue) = NULL;

        return elem;
    }

    // Se a fila tem mais de um elemento, retira o elemento e "gruda" os peda�os.
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

    // Se a fila � nula, ent�o tem zero elementos.
    if (queue == NULL) {
        return 0;
    }

    iterator = queue->next;

    for (i = 1; iterator != queue; iterator = iterator->next, i++);

    return i;
}

void queue_print(char* name, queue_t* queue, void print_elem(void*)) {
    queue_t* iterator = queue;

    printf("%s", name);
    printf("[");

    // Se a fila � nula, ent�o aborta.
    if (queue == NULL) {
        printf("]\n");
        return;
    }

    // Caso externo (s� para n�o printar o primeiro espa�o a esquerda)
    print_elem(iterator);
    iterator = iterator->next;

    while (iterator != queue) {
        printf(" ");
        print_elem(iterator);
        iterator = iterator->next;
    }

    printf("]\n");
    return;
}
