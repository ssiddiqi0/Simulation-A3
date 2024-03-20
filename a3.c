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

bool compareInt(void* pItem, void* pComparisonArg) {
    return ((*(int*)pItem) == (*(int*)pComparisonArg));
}

void kill(int pid){
    PCB* kill_process;
    int status;
    if(List_count(readyQueueHigh) == 0 && List_count(readyQueueNormal) == 0 && List_count(readyQueueLow) == 0 && List_count(runningProcessQueue) == 0){
        printf("Kill: no process in system\n");
    }
    else if(pid == 0){     
        if(List_count(readyQueueHigh) == 0 && List_count(readyQueueNormal) == 0 && List_count(readyQueueLow) == 0 && List_count(runningProcessQueue) == 1){
            kill_process = List_last(runningProcessQueue);
            if(kill_process->pid == 0) {
                if(List_remove(runningProcessQueue)!=NULL){
                    printf("Kill: Init with pid: %d\n", pid);
                }
            }
        }
        else{
            printf("Cannot Kill init process when there are other process in system\n");
        }
    }
    else if (List_search(readyQueueHigh, compareInt, &pid) != NULL) {
        kill_process = List_curr(readyQueueHigh);
        if(List_remove(readyQueueHigh)!=NULL){
            printf("Kill: readyQueueHigh with pid: %d\n", pid);
        }
    } else if (List_search(readyQueueNormal, compareInt, &pid) != NULL) {
        kill_process = List_curr(readyQueueNormal);
        if(List_remove(readyQueueNormal)!=NULL){
            printf("Kill: readyQueueNormal with pid: %d\n", pid);
        }
    } else if (List_search(readyQueueLow, compareInt, &pid) != NULL) {
        kill_process = List_curr(readyQueueLow);
        if(List_remove(readyQueueLow)!=NULL){
            printf("Kill: readyQueueLow with pid: %d\n", pid);
        }
    } else if (List_search(runningProcessQueue, compareInt, &pid) != NULL) {
        kill_process = List_curr(runningProcessQueue);
        if(List_remove(runningProcessQueue)!=NULL){
            printf("Kill: runningProcessQueue with pid: %d\n", pid);
        }
    }else{
        printf("Kill: not found in search\n");
    }

}


void CPUScheduler(){
    PCB* nextProcess;
    if(List_count(readyQueueHigh) > 0){
        nextProcess = List_trim(readyQueueHigh);
    }else if(List_count(readyQueueNormal) > 0){
        nextProcess = List_trim(readyQueueNormal);
    }else if(List_count(readyQueueLow) > 0){
        nextProcess = List_trim(readyQueueNormal);
    }else{
        nextProcess = List_last(runningProcessQueue);
        strcpy(nextProcess->state, "RUNNING");
    }

    if (nextProcess) {
        if (List_append(runningProcessQueue, nextProcess)) {
			printf("FAIL: CPU Scheduler\n");
		}
		else {
            strcpy(nextProcess->state, "RUNNING");
			printf("SUCCESS: CPU Scheduler\n");
            printf("pid: %d is scheduled to run next. \n", nextProcess->pid);
            // if (strlen(nextProcess->message->body) != 0) {
            //     printf(nextProcess->message);
            // }
            // resetTheMsg(nextProcess->message);  // reset the msg body, src, dest
            // strcpy(nextProcess->message, "");
        }
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
