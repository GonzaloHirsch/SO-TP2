#include <stdint.h>
//#include <string.h>
#include <strings.h>
#include <lib.h>
#include <moduleLoader.h>
#include <naiveConsole.h>
#include <irqDispatcher.h>
#include <time.h>
#include <keyboard.h>
#include <idtLoader.h>
#include <interrupts.h>
#include <syscall.h>
#include <videoDriver.h>
#include <console.h>
#include <pixelMap.h>
#include <exceptions.h>
#include <memManager.h>
#include <buddyManager.h>
#include <processes.h>
#include <scheduler.h>
#include "../include/processes.h"

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

//Addresses a donde copia los modulos
static void * const sampleCodeModuleAddress = (void*)0x400000;
static void * const sampleDataModuleAddress = (void*)0x500000;
static void * const memoryStartAddress = (void*)0x700000;

typedef int (*EntryPoint)();


void clearBSS(void * bssAddress, uint64_t bssSize)
{
	memset(bssAddress, 0, bssSize);
}

void * getStackBase()
{
	return (void*)(
		(uint64_t)&endOfKernel
		+ PageSize * 8				//The size of the stack itself, 32KiB
		- sizeof(uint64_t)			//Begin at the top of the stack
	);
}

void goToUserland(){
	((EntryPoint)sampleCodeModuleAddress)();
}

void * initializeKernelBinary()
{
	char buffer[10];

	cpuVendor(buffer);

	//Llena con los addresses a donde copia los modulos
	void * moduleAddresses[] = {
		sampleCodeModuleAddress,
		sampleDataModuleAddress
	};

	loadModules(&endOfKernelBinary, moduleAddresses);

	clearBSS(&bss, &endOfKernel - &bss);

	print("Initializing Video Driver\n");
    initVideoDriver();
    print("Initializing Console\n");
    init_console();
    print("Initializing Memmanager list\n");
    initializeBuddyMemory(memoryStartAddress, 80*1048576); //80MB de memoria dinamica
    print("Initializing Processes\n");
    initProcesses();
    print("Loading idt\n");
 	load_idt();
 	print("Loading exceptions\n");
	loadExceptions();
	print("Turning on rtc\n");
	turn_on_rtc();


	return getStackBase();
}

void testFunction1(){
    int i = 0;
    while((i++)<10) {
        print("Hello World!\n");
    }
}

void testFunction2(){
    int i = 0;
    while((i++)<10) {
        print("Trello world!\n");
    }
    sleep(2000);
}

void testFunction3(){
    int i = 0;
    while((i++)<10) {
        print("Return world!\n");
    }
    sleep(2000);
}

int main()
{
    print("\nIn main");

    
    uint64_t size = 1048576; //1mb

    print("\n\n");
    int cant = 0,m=0;
    int * arrayPointer[6] = {1,1,1,1,1,1};
    size_t sizes[5] = {2,2,1500,1500,1050000};

    for(int i=0;i<5;i++){
        
        if(i == 3){
            buddyFree(arrayPointer[0]);
            buddyFree(arrayPointer[1]);
        }

        arrayPointer[i]= (int *) buddyMalloc(sizes[i]);
        arrayPointer[i][0] = i;
    }

    for(int i=2;i<5;i++){
        new_line();
        print(" dir: "); printInteger(arrayPointer[i]);print(" Number"); printInteger(arrayPointer[i][0]);print(" --");
    }


    print("exiting");
    
    while(cant < 400){

        print("entering");
        
        cant++;
        arrayPointer[6]= (int *) buddyMalloc(sizes[cant%5]);
        
        if(arrayPointer[0]!=NULL) {
            for (int i = 0; i < 10; i++) {  
                arrayPointer[6][i] = i;
            }
            
            new_line();
            print(" dir: "); printInteger(arrayPointer[6]);print(" "); printInteger(cant);print(" --");


            for (int i = 0; i < 10; i++) {
                print("%d-", arrayPointer[6][i]);
            }

            if((477 + sizes[cant%5] * cant) % 3 == 0 ){
                buddyFree(arrayPointer[6]);
            }
        }
        else {
            print("got null bish\n");
        }
        
    }
    
    

    return 0;
}