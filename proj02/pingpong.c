#include <stdio.h>
#include "pingpong.h"

#define STACKSIZE 32768

task_t main;

task_t* task_exec;

long nextid;

void pingpong_init() {
	/* Desativa o buffer de sa�da padr�o */
	setvbuf(stdout, 0, _IONBF, 0);

	/* A task main n�o est� na fila...? */
	main.next = NULL;
	main.prev = NULL;

	/* A task main tem id 0. */
	main.tid = 0;

	/* O id da pr�xima task a ser criada � 1. */
	nextid = 1;

	/* A primeira task em execu��o � a main. */
	task_exec = &main;
}

int task_create(task_t* task, void (*start_func)(void*), void* arg) {
	char* stack;

	/* Informa��es da fila. */
	task->next = NULL;
	task->prev = NULL;

	/* Coloca refer�ncia para task main. */
	task->main = &main;

	/* Inicializa o contexto. */
	getcontext(&(task->context));

	/* Aloca a pilha. */
	stack = malloc(STACKSIZE);
	if (stack == NULL) {
		perror("Erro na cria��o da pilha: ");
		return -1;
	}

	/* Seta a pilha do contexto. */
	task->context.uc_stack.ss_sp = stack;
	task->context.uc_stack.ss_size = STACKSIZE;
	task->context.uc_stack.ss_flags = 0;

	/* N�o liga o contexto a outro. */
	task->context.uc_link = NULL;

	/* Cria o contexto com a fun��o. */
	makecontext(&(task->context), start_func, 1, arg);

	/* Seta o id da task. */
	task->tid = nextid;
	nextid++;

	return (task->tid);
}

void task_exit(int exitCode) {
	task_switch(&main);
}

int task_switch(task_t *task) {
	task_t* prev_task;

	prev_task = task_exec;
	task_exec = task;

	if (swapcontext(&(prev_task->context), &(task->context)) < 0) {
		perror("Erro na troca de contexto: ");
		return -1;
	}

	return 0;
}

int task_id() {
	return (task_exec->tid);
}