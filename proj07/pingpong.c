#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include "pingpong.h"
#include "queue.h"

#define STACKSIZE 32768

#define DEFAULT_PRIO 0
#define MIN_PRIO -20
#define MAX_PRIO 20
#define ALPHA_PRIO 1

#define RESET_TICKS 10
#define TICK_MICROSECONDS 1000

/* Task main */
task_t taskMain;

/* Task em execução */
task_t* taskExec;

/* Task do dispatcher */
task_t taskDisp;

/* Ponteiro para uma task que deve ser liberada */
task_t* freeTask;

/* Fila de tasks prontas */
task_t* readyQueue;

/* Fila de tasks suspensas */
task_t* suspendedQueue;

/* ID da próxima task a ser criada */
long nextid;

/* Preempção por tempo */
void tickHandler();
short remainingTicks;
struct sigaction action;
struct itimerval timer;

/* Relógio do sistema */
unsigned int systemTime;

/* Função a ser executada pela task do dispatcher*/
void bodyDispatcher(void* arg);

/* Função que retorna a próxima task a ser executada. */
task_t* scheduler();

void pingpong_init() {
    /* Desativa o buffer de saída padrão */
    setvbuf(stdout, 0, _IONBF, 0);

    /* INICIA A TASK MAIN */
    /* Referência a si mesmo */
    taskMain.main = &taskMain;

    /* A task main esta pronta. */
    taskMain.estado = 'r';

    /* A task main tem id 0. */
    taskMain.tid = 0;

    /* Informações de tempo */
    taskMain.creationTime = systime();
    taskMain.lastExecutionTime = 0;
    taskMain.execTime = 0;
    taskMain.procTime = 0;
    taskMain.activations = 0;

    /* Coloca a tarefa na fila */
    queue_append((queue_t**)&readyQueue, (queue_t*)&taskMain);
    taskMain.queue = &readyQueue;

    /* O id da próxima task a ser criada é 1. */
    nextid = 1;

    /* A task que está executando nesse momento é a main (que chamou pingpong_init). */
    taskExec = &taskMain;

    /* Nao ha nenhuma task para ser liberada. */
    freeTask = NULL;

    /* Preempção por tempo */
    remainingTicks = RESET_TICKS;
    action.sa_handler = tickHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGALRM, &action, 0) < 0) {
        perror("Erro em sigaction: ");
        exit(1);
    }
    timer.it_value.tv_usec = TICK_MICROSECONDS;
    timer.it_value.tv_sec = 0;
    timer.it_interval.tv_usec = TICK_MICROSECONDS;
    timer.it_interval.tv_sec = 0;
    if (setitimer(ITIMER_REAL, &timer, 0) < 0) {
        perror("Erro em setitimer: ");
        exit(1);
    }

    systemTime = 0;

    /* O contexto não precisa ser salvo agora, porque a primeira troca de contexto fará isso. */

    /* INICIA A TASK SCHEDULER */
    task_create(&taskDisp, &bodyDispatcher, NULL);
    queue_remove((queue_t**)&readyQueue, (queue_t*)&taskDisp);

#ifdef DEBUG
    printf("PingPongOS iniciado.\n");
#endif

    /* Ativa o dispatcher */
    task_yield();
}

int task_create(task_t* task, void(*start_func)(void*), void* arg) {
    char* stack;

    /* Coloca referência para task main. */
    task->main = &taskMain;

    /* Inicializa o contexto. */
    getcontext(&(task->context));

    /* Aloca a pilha. */
    stack = malloc(STACKSIZE);
    if (stack == NULL) {
        perror("Erro na criação da pilha: ");
        return -1;
    }

    /* Seta a pilha do contexto. */
    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = STACKSIZE;
    task->context.uc_stack.ss_flags = 0;

    /* Não liga o contexto a outro. */
    task->context.uc_link = NULL;

    /* Cria o contexto com a função. */
    makecontext(&(task->context), (void(*)(void))start_func, 1, arg);

    /* Seta o id da task. */
    task->tid = nextid;
    nextid++;

#ifdef DEBUG
    printf("task_create: task %d criada.\n", task->tid);
#endif

    /* Informações da fila. */
    queue_append((queue_t**)&readyQueue, (queue_t*)task);
    task->queue = &readyQueue;
    task->estado = 'r';
    task->prio = DEFAULT_PRIO;
    task->dynPrio = task->prio;

    /* Informações de tempo */
    task->creationTime = systime();
    task->lastExecutionTime = 0;
    task->execTime = 0;
    task->procTime = 0;
    task->activations = 0;

    return (task->tid);
}

void task_exit(int exitCode) {
#ifdef DEBUG
    printf("task_exit: encerrando task %d.\n", taskExec->tid);
#endif
    freeTask = taskExec;
    freeTask->estado = 'x';

    freeTask->execTime = systime() - freeTask->creationTime;
    printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", freeTask->tid, freeTask->execTime, freeTask->procTime, freeTask->activations);

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

    prevTask->procTime += systime() - prevTask->lastExecutionTime;

    task->activations++;
    task->lastExecutionTime = systime();

#ifdef DEBUG
    printf("task_switch: trocando task %d -> %d.\n", prevTask->tid, task->tid);
#endif

    if (swapcontext(&(prevTask->context), &(task->context)) < 0) {
        perror("Erro na troca de contexto: ");
        taskExec = prevTask;
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

    /* Se queue for nulo, não retira a tarefa da fila atual. */
    if (queue != NULL) {
        queue_remove((queue_t**)(task->queue), (queue_t*)task);
        queue_append((queue_t**)queue, (queue_t*)task);
        task->queue = queue;
    }

    task->estado = 's';
}

void task_resume(task_t *task) {
    /* Remove a task de sua fila atual e coloca-a na fila de tasks prontas. */
    if (task->queue != NULL) {
        queue_remove((queue_t**)(task->queue), (queue_t*)task);
    }

    queue_append((queue_t**)&readyQueue, (queue_t*)task);
    task->queue = &readyQueue;
    task->estado = 'r';
}

void task_yield() {
    /* Recoloca a task no final da fila de prontas */
    queue_append((queue_t**)&readyQueue, (queue_t*)taskExec);
    taskExec->queue = &readyQueue;
    taskExec->estado = 'r';

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
    while (queue_size((queue_t*)readyQueue)) {
        task_t* next = scheduler();

        if (next != NULL) {
            /* Coloca a tarefa em execução */
            /* Reseta as ticks */
            remainingTicks = RESET_TICKS;
            queue_remove((queue_t**)&readyQueue, (queue_t*)next);
            next->queue = NULL;
            next->estado = 'e';
            task_switch(next);

            /* Libera a memoria da task, caso ela tenha dado exit. */
            if (freeTask != NULL) {
                free(freeTask->context.uc_stack.ss_sp);
                freeTask = NULL;
            }
        }
    }
    task_exit(0);
}

task_t* scheduler() {
    task_t* iterator;
    task_t* nextTask;
    int minDynPrio;
    int minPrio;

    iterator = readyQueue;
    nextTask = NULL;
    minDynPrio = MAX_PRIO + 1;
    minPrio = MAX_PRIO + 1;

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
            minPrio = iterator->prio;
        }
        else if (iterator->dynPrio == minDynPrio) { /* Desempate */
            if (iterator->prio < minPrio) {
                nextTask = iterator;
                minDynPrio = iterator->dynPrio;
                minPrio = iterator->prio;
            }
        }

        iterator = iterator->next;
    } while (iterator != readyQueue);

#ifdef DEBUG
    printf("scheduler: escolhida task %d, prio %d, dynPrio %d.\n", nextTask->tid, nextTask->prio, nextTask->dynPrio);
#endif

    /* Retira a tarefa da fila e reseta sua prioridade dinamica. */
    nextTask->dynPrio = nextTask->prio;
    nextTask->dynPrio += ALPHA_PRIO; /* Para não precisar verificar se cada outra task é a nextTask ou não. */

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

void tickHandler() {
    systemTime++;

    if (taskExec != &taskDisp) {
        remainingTicks--;

        if (remainingTicks == 0) {
#ifdef DEBUG
            printf("tickHandler: final do quantum da tarefa %d.\n", taskExec->tid);
#endif
            task_yield();
        }
    }
}

unsigned int systime() {
    return systemTime * TICK_MICROSECONDS / 1000;
}
