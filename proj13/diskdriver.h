// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DAINF UTFPR
// Versão 1.0 -- Março de 2015
//
// interface do driver de disco rígido

#ifndef __DISKDRIVER__
#define __DISKDRIVER__

#define DISK_REQUEST_READ 1
#define DISK_REQUEST_WRITE 0

// structura de dados que representa um pedido de leitura/escrita ao disco
typedef struct diskrequest_t {
    struct diskrequest_t* next;
    struct diskrequest_t* prev;

    task_t* task;
    unsigned char operation; // DISK_REQUEST_READ ou DISK_REQUEST_WRITE
    int block;
    void* buffer;
} diskrequest_t;

// structura de dados que representa o disco para o SO
typedef struct {
    int numBlocks;
    int blockSize;

    semaphore_t semaforo;

    unsigned char sinal;
    unsigned char livre;

    task_t* diskQueue;
    diskrequest_t* requestQueue;
} disk_t;

// inicializacao do driver de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int diskdriver_init (int *numBlocks, int *blockSize) ;

// leitura de um bloco, do disco para o buffer indicado
int disk_block_read (int block, void *buffer) ;

// escrita de um bloco, do buffer indicado para o disco
int disk_block_write (int block, void *buffer) ;

#endif
