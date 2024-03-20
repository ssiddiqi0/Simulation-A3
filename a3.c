#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

typedef struct{
    int pid;
    int priority;
    char state[20];
    char message[40]; 
} PCB;

List* readyQueueHigh;
List* readyQueueNormal;
List* readyQueueLow;

static int nextPID = 1;

void createProcess(int priority){
    PCB* newProcess = (PCB*)malloc(sizeof(PCB));
    if (newProcess == NULL){
        printf("Failed to allocate memory");
        return;
    }

    
    newProcess->pid = nextPID++;
    newProcess->priority = priority;
    strcpy(newProcess->state, "ready");
    newProcess->message[0] = '\0';
    
    switch(priority) {
        case 0: // High priority
            if (List_append(readyQueueHigh, newProcess) == LIST_FAIL) {
                printf("Failed to add process to high priority ready queue\n");
                free(newProcess);
            }
            break;
        case 1: // Normal priority
            if (List_append(readyQueueNormal, newProcess) == LIST_FAIL) {
                printf("Failed to add process to normal priority ready queue\n");
                free(newProcess);
            }
            break;
        case 2: // Low priority
            if (List_append(readyQueueLow, newProcess) == LIST_FAIL) {
                printf("Failed to add process to low priority ready queue\n");
                free(newProcess);
            }
            break;
        default:
            printf("Invalid priority level\n");
            free(newProcess);
            break;
    }

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
    readyQueueHigh = List_create();
    readyQueueNormal = List_create();
    readyQueueLow = List_create();
    
    createProcess(0);
    createProcess(1);
    createProcess(2);

    return 0;

}
