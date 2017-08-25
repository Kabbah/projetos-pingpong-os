#include <stdio.h>
#include <stdlib.h>
#include "pingpong.h"
#include "queue.h"

#define STACKSIZE 32768

/* Task main */
task_t taskMain;

/* Task em execu��o */
task_t* taskExec;

/* Task do dispatcher */
task_t taskDisp;

/* Fila de tasks prontas */
task_t* readyQueue;

/* Fila de tasks suspensas */
task_t* suspendedQueue;

/* ID da pr�xima task a ser criada */
long nextid;

/* Fun��o a ser executada pela task do dispatcher*/
void bodyDispatcher(void* arg);

/* Fun��o que retorna a pr�xima task a ser executada. */
task_t* scheduler();

void pingpong_init() {
	/* Desativa o buffer de sa�da padr�o */
	setvbuf(stdout, 0, _IONBF, 0);

	/* INICIA A TASK MAIN */
	/* A task main n�o est� na fila...? */
	taskMain.next = NULL;
	taskMain.prev = NULL;
	
	/* Refer�ncia a si mesmo? */
	taskMain.main = &taskMain;

	/* A task main tem id 0. */
	taskMain.tid = 0;

	/* O id da pr�xima task a ser criada � 1. */
	nextid = 1;

	/* A primeira task em execu��o � a main. */
	taskExec = &taskMain;

	/* O contexto n�o precisa ser salvo agora, porque a primeira troca de contexto far� isso. */

	/* INICIA A TASK SCHEDULER */
	task_create(&taskDisp, &bodyDispatcher, NULL);
	queue_remove((queue_t**) &readyQueue, (queue_t*) &taskDisp);
	
	#ifdef DEBUG
	printf("PingPongOS iniciado.\n");
	#endif
}

int task_create(task_t* task, void (*start_func)(void*), void* arg) {
	char* stack;

	/* Coloca refer�ncia para task main. */
	task->main = &taskMain;

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
	
	#ifdef DEBUG
	printf("task_create: task %d criada.\n", task->tid);
	#endif

	/* Informa��es da fila. */
	queue_append((queue_t**) &readyQueue, (queue_t*) task);

	return (task->tid);
}

void task_exit(int exitCode) {
	#ifdef DEBUG
	printf("task_exit: encerrando task %d.\n", taskExec->tid);
	#endif

	/* TODO: free na pilha */
	
	if (taskExec == &taskDisp) {
		task_switch(&taskMain);
	}
	else {
		task_switch(&taskDisp);
	}
}

int task_switch(task_t *task) {
	task_t* prevTask;

	prevTask = taskExec;
	taskExec = task;
	
	#ifdef DEBUG
	printf("task_switch: trocando task %d -> %d.\n", prevTask->tid, task->tid);
	#endif
	
	if (swapcontext(&(prevTask->context), &(task->context)) < 0) {
		perror("Erro na troca de contexto: ");
		return -1;
	}

	return 0;
}

int task_id() {
	return (taskExec->tid);
}

void task_suspend(task_t *task, task_t **queue) {

}

void task_resume(task_t *task) {

}

void task_yield() {
	task_switch(&taskDisp);
}

void bodyDispatcher(void* arg) {
	while (queue_size((queue_t*) readyQueue)) {
		task_t* next = scheduler();

		if (next != 0) {
			task_switch(next);
		}
	}
	task_exit(0);
}

task_t* scheduler() {
	return queue_remove((queue_t**) &readyQueue, (queue_t*) readyQueue);
}
