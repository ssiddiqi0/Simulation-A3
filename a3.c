#include <stdio.h>
#include <stdlib.h>

typedef struct{
    int pid;
    int priority;
    char state[20];
    char message[40]; 
} PCB;

PCB* readyQueue[3] = {NULL};

void createProcess(int priority){
    PCB* newProcess = (PCB*)malloc(sizeof(PCB));
    if (newProcess == NULL){
        printf("Failed to allocate memory");
        return;
    }

    static int nextPID = 1;
    newProcess->pid = nextPID++;
    newProcess->priority = priority;
    printf(newProcess->state, "ready");
    newProcess->message[0] = '\0';
    
    readyQueue[priority] = newProcess;

    printf("Process with pID %d create and placed in ready Q with priority: %d", newProcess->pid, priority);
}
void handleForkCommand(int currentPID){

    PCB *currentProcess = NULL;
    int priority = -1;

    for (int i=0; i<3; i++){
        if(readyQueue[i]!=NULL && readyQueue[i]->pid == currentPID){
            currentProcess = readyQueue[i];
            priority = i;
            break;
        }
    }

    if(currentProcess == NULL){
        printf("Error: Process with PID %d not found \n", currentPID);
        return;
    }
    PCB *newProcess = (PCB*)malloc(sizeof(PCB));
    if(newProcess == NULL){
        printf("Failed to allocate memory");
        return;
    }
    // Not finished

}

int main() {
    createProcess(0);
    createProcess(1);
    createProcess(2);

    return 0;

}