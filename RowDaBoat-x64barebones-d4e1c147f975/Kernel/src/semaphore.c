//
// Created by click on 19/10/19.
//

#include "../include/semaphore.h"
#include "../include/memManager.h"
#include "../include/intQueue.h"
#include "../include/scheduler.h"
#include "../include/console.h"
#include "../include/lib.h"

extern forceChangeProcess();

typedef struct SemaphoreCDT{

    sem semId;
    char name[MAX_SEMAPHORE_NAME];
    int value;

    IntQueue waitingProcesses;
}SemaphoreCDT;


static Semaphore theSemaphoreList[MAX_SEMAPHORE_COUNT];
static int highestSemId;


void initializeSemaphores() {
    for(int i = 0; i<MAX_SEMAPHORE_COUNT; i++){
        theSemaphoreList[i] = NULL;
    }
    highestSemId = 0;
};

const sem * openSemaphore(char *name) {

    //si el semaforo esta, devolver puntero a semId
    int i;
    for(i = 0; i<MAX_SEMAPHORE_COUNT; i++){
        if(theSemaphoreList[i] != NULL && strcmp(name, theSemaphoreList[i]->name) == 0){
            return &theSemaphoreList[i]->semId;
        }
    }

    //si no, busco el menor slot nulo para asignarle un nuevo semaphore... Es O(n) pero es mas prolijo que tener
    //un contador global que nunca se resettea...
    for(i = 0; i<MAX_SEMAPHORE_COUNT && theSemaphoreList[i]!=NULL; i++);
    Semaphore aux = mAlloc(sizeof(struct SemaphoreCDT));
    aux->semId = i;
    if(i>highestSemId) highestSemId = i;
    strcpy(aux->name, name);
    aux->value = 1;
    aux->waitingProcesses = newQueue(MAX_WAITING_PROCESSES);

    theSemaphoreList[i] = aux;
    return &aux->semId;
}

int semGetValue(const sem *semaphore) {
    return theSemaphoreList[*semaphore]->value;
}

void semWait(const sem *semaphore) {
    if(theSemaphoreList[*semaphore]->value <= 0){
        int pid = getProcessPid(getCurrentProcess());
        enqueue(theSemaphoreList[*semaphore]->waitingProcesses, pid);
        setProcessStateByPid(pid, STATE_BLOCKED);
        forceChangeProcess();
    }
    print("decreasing semaphore, current process: %s", getProcessName(getCurrentProcess()));
    theSemaphoreList[*semaphore]->value--;
}

void semPost(const sem *semaphore) {

    Semaphore aux = theSemaphoreList[*semaphore];
    if(aux->value<=0 && !isEmpty(aux->waitingProcesses)){
        int pid = dequeue(aux->waitingProcesses);
        setProcessStateByPid(pid, STATE_READY);
    }
    print("increasing semaphore, current process: %s", getProcessName(getCurrentProcess()));
    aux->value++;
}

void printAllSemaphores() {
    Semaphore aux;
    for(int i = 0; i<highestSemId; i++){
        aux = theSemaphoreList[i];
        print("Semaphore %s\n    Value: %d\n", aux->name, aux->value);
    }
}