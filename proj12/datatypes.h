// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DAINF UTFPR
// Versão 1.0 -- Março de 2015
//
// Estruturas de dados internas do sistema operacional

#ifndef __DATATYPES__
#define __DATATYPES__

#include <ucontext.h>

// Estrutura que define uma tarefa
typedef struct task_t {
	struct task_t* prev;
	struct task_t* next;
	struct task_t** queue;
	
	struct task_t* main;
	
	ucontext_t context;

	char estado;
	int prio;
	int dynPrio;

	unsigned int creationTime;
	unsigned int lastExecutionTime;
	unsigned int execTime;
	unsigned int procTime;
	unsigned int activations;

	struct task_t* joinQueue;
	int exitCode;

    unsigned int awakeTime;

	int tid;
} task_t ;

// estrutura que define um semáforo
typedef struct {
    struct task_t* queue;
    int value;

    unsigned char active;
} semaphore_t ;

// estrutura que define um mutex
typedef struct {
    struct task_t* queue;
    unsigned char value;
    
    unsigned char active;
} mutex_t ;

// estrutura que define uma barreira
typedef struct {
    struct task_t* queue;
    int maxTasks;
    int countTasks;
    
    unsigned char active;
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct {
    void* content;
    int messageSize;
    int maxMessages;
    int countMessages;
    
    semaphore_t sBuffer;
    semaphore_t sItem;
    semaphore_t sVaga;
    
    unsigned char active;
} mqueue_t ;

#endif
