#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

typedef struct PROC_MSG {
	int receiver;
    int sender;
    char* type;
	char* message;
} PROC_MSG;


typedef struct{
    int pid;
    int priority;
    char state[20];
    //char message[40]; 
    PROC_MSG *procmsg;
    // PCB* next;
    // PCB* prev;
} PCB;

typedef struct Semaphore {
    int value;
    PCB* blockedProcesses; // Queue of processes waiting on the semaphore
} Semaphore;


List* readyQueueHigh;
List* readyQueueNormal;
List* readyQueueLow;
List* runningProcessQueue;
List* sendBlockedQueue;
List* receiveOperationQueue;
List* msgQueue;


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
    PROC_MSG* newmsg = malloc(sizeof(PROC_MSG));
	newmsg->message = malloc(sizeof(char) * 100);;
    newmsg->message = "";
    newmsg->sender = -1;
    newmsg->receiver = -1;

    newProcess->procmsg = newmsg;
   
    
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
            if (strlen(nextProcess->procmsg->message) != 0) {
                printf("%s",nextProcess->procmsg->message);
            }
            strcpy(nextProcess->procmsg->message, "");
            nextProcess->procmsg->sender = -1;
            nextProcess->procmsg->receiver = -1;
        }
	}
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


void exitP(){
    if(List_count(readyQueueHigh) == 0 && List_count(readyQueueNormal) == 0 && List_count(readyQueueLow) == 0 && List_count(runningProcessQueue) == 1){
            PCB* kill_process = List_last(runningProcessQueue);
            if(kill_process->pid == 0) {
                if(List_remove(runningProcessQueue)!=NULL){
                    printf("Kill: Init with pid: %d\n", kill_process->pid );
                }
            }
    }
    else if(List_count(runningProcessQueue) == 2){
        PCB* p = List_last(runningProcessQueue);
        if(List_remove(runningProcessQueue) != NULL){
            printf("Kill: Init with pid: %d\n", p->pid);
        }
    }
    CPUScheduler();
}


void quantum(){
    PCB* currentRunning = List_last(runningProcessQueue);
    if(currentRunning){
        if(currentRunning->pid == 0){
            strcpy(currentRunning->state, "READY");
        }
        else{
            PCB* removedProces = List_trim(runningProcessQueue);
            strcpy(removedProces->state, "READY");
            if (removedProces->priority == 0 || removedProces->priority == 1) {
                removedProces->priority = removedProces->priority+1;
            }
            if (removedProces != NULL) {
                switch(removedProces->priority) {
                    case 0: // High priority
                        if (List_append(readyQueueHigh, removedProces) == LIST_FAIL) {
                            printf("Failed to add process to high priority ready queue\n");
                        }
                        break;
                    case 1: // Normal priority
                        if (List_append(readyQueueNormal, removedProces) == LIST_FAIL) {
                            printf("Failed to add process to normal priority ready queue\n");
                        }
                        break;
                    case 2: // Low priority
                        if (List_append(readyQueueLow, removedProces) == LIST_FAIL) {
                            printf("Failed to add process to low priority ready queue\n");
                        }
                        break;
                    default:
                        printf("Invalid priority level\n");
                        //free(newProcess);
                        break;
                }
                // free(removedProces);
                printf("Quantum - pid:%d is now in ready queue.\n", removedProces->pid);
		    }
        }
    }else{
        printf("Quantum: NO PROCESS RUNNING TO STOP\n");
    }
    CPUScheduler();
}

void Send(int pid, const char* msg){
    PCB* procs;
    PCB* senderID = List_last(runningProcessQueue);   
    // checks if there's a process in the receive queue waiting to receive a message with the specified PID
    if (List_search(receiveOperationQueue, compareInt, &pid) != NULL) {
        procs = List_curr(receiveOperationQueue);
        procs->procmsg->sender= senderID->pid;
        procs->procmsg->receiver= pid;
        strcpy(procs->procmsg->type, "SEND");
        strcpy(procs->procmsg->message, msg);
        strcpy(procs->state, "READY");
        PCB* p = List_remove(receiveOperationQueue);
        switch(p->priority) {
            case 0: // High priority
                if (List_append(readyQueueHigh, p) == LIST_FAIL) {
                    printf("Failed to add process to high priority ready queue\n");
                }
                break;
            case 1: // Normal priority
                if (List_append(readyQueueNormal, p) == LIST_FAIL) {
                    printf("Failed to add process to normal priority ready queue\n");
                }
                break;
            case 2: // Low priority
                if (List_append(readyQueueLow, p) == LIST_FAIL) {
                    printf("Failed to add process to low priority ready queue\n");
                }
                break;
            default:
                printf("Invalid priority level\n");
                break;
        }
        printf("Send successful: Process %d sent message to Process %d\n", senderID->pid, pid);
    }
    else{
        procs = findPCBByPID(pid);
        if(procs != NULL && pid != 0 ){  // not sure if we want init process to send message but it cannot be blocked
            PROC_MSG* pmsg = malloc(sizeof(pmsg));
            pmsg->message = malloc(sizeof(char) * 100);
            pmsg->sender = senderID->pid;
            pmsg->receiver = pid;
            strcpy(pmsg->type, "SEND");
            strcpy(pmsg->message, msg);
            List_append(msgQueue, pmsg); // list_add
            strcpy(senderID->state, "BLOCKED");
            PCB* p = List_trim(runningProcessQueue);
            List_append(sendBlockedQueue, p);
            CPUScheduler();
            printf("Sender Blocked: Process %d sent message to Process %d\n", senderID->pid, pid);
        }
    }
}

void receive(){
    // check if there is msg for receiver 
    PCB* receiver = List_last(runningProcessQueue);   
    int receiverPid = receiver->pid;
    if (List_search(msgQueue, compareInt, &receiverPid) != NULL) {
        PCB* procs = List_curr(msgQueue);
        printf("-------");
        printf("Sender Message:");
        printf("Type: %s\n", procs->procmsg->type);
        printf("Sender pid: %d - ", procs->procmsg->sender);
        printf("Message: %s\n", procs->procmsg->message);
        printf("-------");
        List_remove(msgQueue);
    }else{
        if (receiverPid != 0) {
			receiver = List_trim(runningProcessQueue);
            strcpy(receiver->state, "BLOCKED");
			List_prepend(receiveOperationQueue, receiver);
			CPUScheduler();
		}
    }
}

void reply(int pid, char *msg){
    // unblocks sender and delivers reply : success or failure
    PCB* receiver = List_last(runningProcessQueue);   
     if (List_search(sendBlockedQueue, compareInt, &pid) != NULL) {
        PCB* procs = List_curr(sendBlockedQueue);
 
        procs->procmsg->sender= receiver->pid;
        procs->procmsg->receiver= pid;
        strcpy(procs->procmsg->type, "REPLY");
        strcpy(procs->procmsg->message, msg);

        strcpy(procs->state, "READY");
        PCB* p = List_remove(sendBlockedQueue);

        if (p != NULL) {
                switch(p->priority) {
                    case 0: // High priority
                        if (List_append(readyQueueHigh, p) == LIST_FAIL) {
                            printf("Failed to add process to high priority ready queue\n");
                        }
                        break;
                    case 1: // Normal priority
                        if (List_append(readyQueueNormal, p) == LIST_FAIL) {
                            printf("Failed to add process to normal priority ready queue\n");
                        }
                        break;
                    case 2: // Low priority
                        if (List_append(readyQueueLow, p) == LIST_FAIL) {
                            printf("Failed to add process to low priority ready queue\n");
                        }
                        break;
                    default:
                        printf("Invalid priority level\n");
                        break;
                }
                printf("Reply - pid:%d is now in ready queue.\n", p->pid);
		    }
    }else{
        printf("reply: FAIL, PID not found");
    }
}

void procinfo(int pid) {
	PCB* prc = findPCBByPID(pid);
	if (prc != NULL) {
		printf("Process ID:: %i\n", prc->pid);
		printf("Process Priority: %i\n", prc->priority);
		printf("Process State: %s\n", prc->state);
		printf("Process Msg: %s\n", prc->procmsg->message);
	}
	else {
		printf("Given Pid is invalid. Not found");
	}
}
void PrintList(List* list){
    PCB *p = List_first(list);
    while (p) {
        printf("Running Process Queue \n");
        printf("Process ID:: %i\n", p->pid);
        printf("Process Priority: %i\n", p->priority);
        printf("Process State: %s\n", p->state);
        printf("Process Msg: %s\n", p->procmsg->message);
        p = List_next(list);
    }
}
	
void totalinfo() {
	PCB* prc = List_last(runningProcessQueue);
    printf("Running Process Queue \n");
	printf("Process ID:: %i\n", prc->pid);
	printf("Process Priority: %i\n", prc->priority);
	printf("Process State: %s\n", prc->state);
	printf("Process Msg: %s\n", prc->procmsg->message);
	
	// Ready QUEUE
	printf("READY QUEUES");
    PrintList(readyQueueHigh);
    PrintList(readyQueueNormal);
    PrintList(readyQueueLow);

	// Send QUEUE
	printf("SEND QUEUE");
	PrintList(sendBlockedQueue);

    // Receive QUEUE
	printf("RECEIVE QUEUE");
	PrintList(receiveOperationQueue);

    // MSG QUEUE
	printf("MESSAGE QUEUE");
	PrintList(msgQueue);

    // SEM QUEUE
	printf("SEM QUEUES");


}
void newSemaphore(int semaphoreID, int initialValue) {
    if (semaphoreID < 0 || semaphoreID >= 5) {
        printf("Invalid semaphore ID\n");
        return;
    }
    semaphores[semaphoreID].value = initialValue;
    semaphores[semaphoreID].blockedProcesses = List_create();
    printf("Semaphore %d initialized with value %d\n", semaphoreID, initialValue);
}

void semaphoreP(int semaphoreID) {
    if (semaphoreID < 0 || semaphoreID >= 5) {
        printf("Invalid semaphore ID\n");
        return;
    }
    if (semaphores[semaphoreID].value > 0) {
        semaphores[semaphoreID].value--;
    } else {

        PCB* currentProcess = List_curr(runningProcessQueue);
        strcpy(currentProcess->state, "blocked");
        List_append(semaphores[semaphoreID].blockedProcesses, currentProcess);
        CPUScheduler(); // call scheduler?
    }
    printf("Process performed P operation on semaphore %d\n", semaphoreID);
}

void semaphoreV(int semaphoreID) {
    if (semaphoreID < 0 || semaphoreID >= 5) {
        printf("Invalid semaphore ID\n");
        return;
    }
    semaphores[semaphoreID].value++;
    if (List_count(semaphores[semaphoreID].blockedProcesses) > 0) {
        PCB* unblockedProcess = List_trim(semaphores[semaphoreID].blockedProcesses);
        strcpy(unblockedProcess->state, "ready");
        switch(unblockedProcess->priority) {
            case 0: List_prepend(readyQueueHigh, unblockedProcess); break;
            case 1: List_prepend(readyQueueNormal, unblockedProcess); break;
            case 2: List_prepend(readyQueueLow, unblockedProcess); break;
            default: printf("Invalid priority level\n"); break;
        }
        printf("Process with pid %d unblocked from semaphore %d\n", unblockedProcess->pid, semaphoreID);
    } else {
        printf("No processes waiting on semaphore %d\n", semaphoreID);
    }
}

int main() {
    readyQueueHigh = List_create();
    readyQueueNormal = List_create();
    readyQueueLow = List_create();

    runningProcessQueue = List_create();
    PCB *init;
    init = malloc(sizeof(PCB));
	init->pid = 0;
	init->priority = 0;
	strcpy(init->state, "ready");
    strcpy(init->message, "init Process");
    List_append(runningProcessQueue, init);

    createProcess(0);
    strcpy(init->state, "ready");
    createProcess(1);
    createProcess(2);
    PCB* newP = List_curr(readyQueueHigh);
    List_append(runningProcessQueue, newP);
    List_trim(readyQueueHigh);
    quantum();

    //forkP();
    //kill(3);
    return 0;

}
