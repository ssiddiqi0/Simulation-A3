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
List* runningProcessQueue;

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
    printf("Process with pID %d create and placed in ready Q with priority: %d\n", newProcess->pid, priority);
}

void forkP(){
    PCB* currentProcess = List_curr(runningProcessQueue);
    if(currentProcess != NULL){
        if(currentProcess->pid == 0){
            printf("Current running process is Init :- Cannot fork process\n");
        }
        else{
            if(currentProcess){
                createProcess(currentProcess->priority);
                printf("Forked the current running process: %d\n", currentProcess->pid);
            }else{
                printf("Cannot Fork: no process is running\n");
            }
        }
    }else{
        printf("Cannot Fork: no process is running\n");
    }
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
