//
// Created by click on 29/10/19.
//

#include "../include/processStack.h"
#include "../include/processes.h"

static void entryPoint(uint64_t functionAddress, uint64_t pid);
extern void forceChangeProcess();

typedef struct ProcessStack{


    uint64_t gs;
    uint64_t fs;
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    uint64_t rip;
    uint64_t cs;
    uint64_t eflags;
    uint64_t rsp;
    uint64_t ss;

    uint64_t firstLine;
} ProcessStack;


uint64_t initializeProcessStack(uint64_t stackBaseAddress, uint64_t functionAddress, uint64_t processPtr){

    ProcessStack * processStack = (ProcessStack *)(stackBaseAddress + PROCESS_STACK_SIZE - sizeof(ProcessStack) - 1);
    //todo memset(&processStack->gs, (uint32_t) 0x0000, sizeof(uint32_t)*2*10); //setteo a 0 todos los registros gs a r8
    //argumentos del wrapper
    processStack->rdi = functionAddress; //Esto es para tener como parametro al functionAddress cuando llame al wrapper (entryPoint)
    processStack->rsi = processPtr;
    //todo memset(&processStack->rsi, (uint32_t) 0x0000, sizeof(uint32_t)*2*5); //setteo los registros restantes a 0

    //setteo los registros no usados a 0
    processStack->cs = processStack->fs = processStack->r15=processStack->r14=processStack->r13=0x0;
    processStack->r12=processStack->r11=processStack->r10=processStack->r9=processStack->r8 = 0x0;
    processStack->rbp=processStack->rdx=processStack->rcx=processStack->rbx=processStack->rax = 0x0;

    //Interrupt Stack Frame
    processStack->rip = (uint64_t) &entryPoint;
    processStack->cs = 0x8;
    processStack->eflags = 0x202;
    // setteo el stack pointer del proceso para que
    // apunte abajo del frame del iretq (el stack crece para arriba towards valores mas bajos de memoria)
    processStack->rsp = (uint64_t) (&processStack->firstLine);
    processStack->ss = 0x0;
    processStack->firstLine = 0x0;

    //devuelvo puntero al top del stack
    return (uint64_t) &processStack->gs;
}

static void entryPoint(uint64_t functionAddress, uint64_t processPtr){

    // ejecuto  la funcion en functionAddress
    ((void*(*)()) functionAddress)();

    // setteo el estado del proceso como terminado una vez finalizada
    // la ejecucion de la funcion
    setProcessState((Process)processPtr, STATE_TERMINATED);
    forceChangeProcess();
}