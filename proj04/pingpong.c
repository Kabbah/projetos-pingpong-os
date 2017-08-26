#include <stdio.h>
#include <stdlib.h>
#include "pingpong.h"
#include "queue.h"

#define STACKSIZE 32768

#define DEFAULT_PRIO 0
#define MIN_PRIO -20
#define MAX_PRIO 20
#define ALPHA_PRIO 1

/* Task main */
task_t taskMain;

/* Task em execu��o */
task_t* taskExec;

/* Task do dispatcher */
task_t taskDisp;

/* Ponteiro para uma task que deve ser liberada */
task_t* freeTask;

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
	taskMain.queue = NULL;
	
	/* Refer�ncia a si mesmo? */
	taskMain.main = &taskMain;

	/* A task main esta pronta. */
	taskMain.estado = 'r';

	/* A task main tem id 0. */
	taskMain.tid = 0;

	/* O id da pr�xima task a ser criada � 1. */
	nextid = 1;

	/* A primeira task em execu��o � a main. */
	taskExec = &taskMain;

	/* Nao ha nenhuma task para ser liberada. */
	freeTask = NULL;

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
	task->queue = &readyQueue;
	task->estado = 'r';
	task->prio = DEFAULT_PRIO;
	task->dynPrio = task->prio;

	return (task->tid);
}

void task_exit(int exitCode) {
	#ifdef DEBUG
	printf("task_exit: encerrando task %d.\n", taskExec->tid);
	#endif
	freeTask = taskExec;
	
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
	/* Se task for nulo, considera a tarefa corrente. */
	if (task == NULL) {
		task = taskExec;
	}

	/* Se queue for nulo, n�o retira a tarefa. */
	if (queue != NULL) {
		#ifdef DEBUG
		printf("task_suspend: queue � NULL, a tarefa %d n�o foi suspensa.\n", task->tid);
		#endif
		return;
	}

	/* Remove a task de sua fila atual e coloca-a na fila fornecida. */
	queue_remove((queue_t**)(task->queue), (queue_t*)task);
	queue_append((queue_t**)queue, (queue_t*)task);
	task->queue = queue;
	task->estado = 's';
}

void task_resume(task_t *task) {
	/* Remove a task de sua fila atual e coloca-a na fila de tasks prontas. */
	if (task->queue != NULL) {
		queue_remove((queue_t**)(task->queue), (queue_t*)task);
	}

	queue_append((queue_t**)readyQueue, (queue_t*)task);
	task->queue = &readyQueue;
	task->estado = 'r';
}

void task_yield() {
	/* Volta o controle para o dispatcher. */
	task_switch(&taskDisp);
}

void task_setprio(task_t* task, int prio) {
	if (task == NULL) {
		task = taskExec;
	}
	if (prio <= MAX_PRIO && prio >= MIN_PRIO) {
		task->prio = prio;
		task->dynPrio = prio;
	}
}

int task_getprio(task_t* task) {
	if (task == NULL) {
		task = taskExec;
	}
	return task->prio;
}

void bodyDispatcher(void* arg) {
	while (queue_size((queue_t*) readyQueue)) {
		task_t* next = scheduler();

		if (next != 0) {
			task_switch(next);

			/* Libera a memoria da task, caso ela tenha dado exit. */
			if (freeTask != NULL) {
				free(freeTask->context.uc_stack.ss_sp);
				freeTask = NULL;
			}
			else {
				/* Recoloca a task no final da fila de prontas, caso ela tenha dado yield. */
				queue_append((queue_t**)&readyQueue, (queue_t*)next);
			}
		}
	}
	task_exit(0);
}

task_t* scheduler() {
	task_t* iterator;
	task_t* nextTask;
	int minDynPrio;

	iterator = readyQueue;
	nextTask = NULL;
	minDynPrio = MAX_PRIO + 1;

	/* Se a fila estiver vazia, retorna NULL. */
	if (iterator == NULL) {
		return NULL;
	}

	#ifdef DEBUG
	printf("scheduler: buscando task com menor dynPrio.\n");
	#endif

	/* Busca a tarefa com menor dynPrio para executar. */
	do {
		#ifdef DEBUG
		printf("scheduler: task %d, prio %d, dynPrio %d.\n", iterator->tid, iterator->prio, iterator->dynPrio);
		#endif

		if (iterator->dynPrio < minDynPrio) {
			nextTask = iterator;
			minDynPrio = iterator->dynPrio;
		}

		iterator = iterator->next;
	} while (iterator != readyQueue);

	#ifdef DEBUG
	printf("scheduler: escolhida task %d, prio %d, dynPrio %d.\n", nextTask->tid, nextTask->prio, nextTask->dynPrio);
	#endif

	/* Retira a tarefa da fila e reseta sua prioridade dinamica. */
	queue_remove((queue_t**)&readyQueue, (queue_t*)nextTask);
	nextTask->dynPrio = nextTask->prio;

	/* Atualiza a dynprio das outras tarefas. */
	iterator = readyQueue;
	if (iterator != NULL) {
		do {
			iterator->dynPrio -= ALPHA_PRIO;
			#ifdef DEBUG
			printf("scheduler: atualizando task %d, prio %d, dynPrio %d.\n", iterator->tid, iterator->prio, iterator->dynPrio);
			#endif
			iterator = iterator->next;
		} while (iterator != readyQueue);
	}

	return nextTask;
}