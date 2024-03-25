#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "list.h"


typedef struct PROC_MSG {
	int receiver;
    int sender;
    int type_index;
	char* message;
} PROC_MSG;


typedef struct{
    int pid;
    int priority;
    char state[20];
    PROC_MSG *procmsg;
} PCB;

typedef struct Semaphore {
    int pid;
    int value;
    List* blockedProcesses; 
} Semaphore;

Semaphore semaphores[5];

char *process_states[] = {"","SEND", "RECEIVE", "REPLY"};

List* readyQueueHigh;
List* readyQueueNormal;
List* readyQueueLow;
List* runningProcessQueue;
List* sendBlockedQueue;
List* receiveOperationQueue;
List* msgQueue;
List* semaphoreQueue;
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

    newProcess->procmsg = (PROC_MSG*) malloc(sizeof (PROC_MSG));
    newProcess->procmsg->receiver = -1;
	newProcess->procmsg->sender = -1;
    newProcess->procmsg->type_index = 0;

    newProcess->procmsg->message = (char*) malloc(sizeof(char) * 100);
	strcpy(newProcess->procmsg->message, "");
    

    // PROC_MSG* newmsg = malloc(sizeof(PROC_MSG));
	// newmsg->message = malloc(sizeof(char) * 100);
    // strcpy(newmsg->message, "");
    // newmsg->sender = -1;
    // newmsg->receiver = -1;
    // newProcess->procmsg = newmsg;
   
    
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
        List_first(readyQueueHigh);
        nextProcess = List_remove(readyQueueHigh);
    }else if(List_count(readyQueueNormal) > 0){
        List_first(readyQueueNormal);
        nextProcess = List_remove(readyQueueNormal);
    }else if(List_count(readyQueueLow) > 0){
        List_first(readyQueueLow);
        nextProcess = List_remove(readyQueueNormal);
    }else{
        nextProcess = List_last(runningProcessQueue);
        strcpy(nextProcess->state, "RUNNING"); // Init
    }

    if (nextProcess) {
        if (List_append(runningProcessQueue, nextProcess)) {
			printf("FAIL: CPU Scheduler\n");
		}
		else {
            strcpy(nextProcess->state, "RUNNING");
			printf("SUCCESS: CPU Scheduler ");
            printf("pid: %d is Running now \n", nextProcess->pid);
            if (strlen(nextProcess->procmsg->message) != 0) {
                printf("%s",nextProcess->procmsg->message);
            }
            nextProcess->procmsg->receiver = -1;
            nextProcess->procmsg->sender = -1;
            strcpy(nextProcess->procmsg->message, "");
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

PCB* findPCBByPID(int pid){
    PCB* procs= NULL;
    List_first(readyQueueHigh);
    List_first(readyQueueNormal);
    List_first(readyQueueLow);
    List_first(runningProcessQueue);
    List_first(sendBlockedQueue);
    List_first(receiveOperationQueue);
    if (List_search(readyQueueHigh, compareInt, &pid) != NULL) {
        procs = List_curr(readyQueueHigh);
    } else if (List_search(readyQueueNormal, compareInt, &pid) != NULL) {
        procs = List_curr(readyQueueNormal);
    } else if (List_search(readyQueueLow, compareInt, &pid) != NULL) {
        procs = List_curr(readyQueueLow);
    } else if (List_search(runningProcessQueue, compareInt, &pid) != NULL) {
        procs = List_curr(runningProcessQueue);
    }else if (List_search(sendBlockedQueue, compareInt, &pid) != NULL) {
        procs = List_curr(sendBlockedQueue);
    }else if (List_search(receiveOperationQueue, compareInt, &pid) != NULL) {
        procs = List_curr(receiveOperationQueue);
    }else{
        printf("Cannot find the pid\n");
    }
    return procs;
}

void Send(int pid, const char* msg){
    PCB* procs;
    PCB* senderID = List_last(runningProcessQueue);   
    // checks if there's a process in the receive queue waiting to receive a message with the specified PID
    if (List_search(receiveOperationQueue, compareInt, &pid) != NULL) {
        procs = List_curr(receiveOperationQueue);
        procs->procmsg->sender = senderID->pid;
        procs->procmsg->receiver = pid;
        procs->procmsg->type_index = 1;
        strcpy(procs->procmsg->message, msg);
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
            pmsg->message = (char*) malloc(sizeof(char) * 40);
            pmsg->sender = senderID->pid;
            pmsg->receiver = pid;
            pmsg->type_index = 1;
            strcpy(pmsg->message, msg);
            List_append(msgQueue, pmsg); // list_add
            strcpy(senderID->state, "BLOCKED");
            List_last(runningProcessQueue);
            PCB* p = List_trim(runningProcessQueue);
            List_append(sendBlockedQueue, p);
            CPUScheduler();
            printf("Sender Blocked: Process: %d wants to message Process: %d\n", senderID->pid, pid);
        }
    }
}

void receive(){
    // check if there is msg for receiver 
    PCB* receiver = List_last(runningProcessQueue);   
    int receiverPid = receiver->pid;
    List_first(msgQueue);
    if (List_search(msgQueue, compareInt, &receiverPid) != NULL) {
        PROC_MSG* procs = List_curr(msgQueue);
        printf("---RECIVE---\n");
        printf("Sender Message:\n");
        printf("Type: %s\n", process_states[procs->type_index]);
        printf("Sender pid: %d - ", procs->sender);
        printf("Message: %s\n", procs->message);
        printf("-------\n");
    }else{
        printf("else part\n");
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
    List_first(sendBlockedQueue);
     if (List_search(sendBlockedQueue, compareInt, &pid) != NULL) {
        PCB* procs = List_curr(sendBlockedQueue);
 
        procs->procmsg->sender= receiver->pid;
        procs->procmsg->receiver= pid;
        procs->procmsg->type_index = 3;
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
        printf("reply: FAIL, PID not found\n");
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
    p = List_prev(list);
    while (p = List_next(list)) {
        printf("Running Process Queue \n");
        printf("Process ID:: %i\n", p->pid);
        printf("Process Priority: %i\n", p->priority);
        printf("Process State: %s\n", p->state);
        printf("Process Msg: %s\n", p->procmsg->message);
    }
}

void printSemList(){
    if (List_count(semaphoreQueue) > 0 ) {
		List_first(semaphoreQueue);
		List_prev(semaphoreQueue);
		for (int i = 0; i < List_count(semaphoreQueue); i++) {
			Semaphore *s = List_next(semaphoreQueue);
			printf("sid%i: ", s->pid);
			PrintList(s->blockedProcesses);
		}
	}
}
void totalinfo() {
	PCB* prc = List_last(runningProcessQueue);
    printf("Running Process Queue \n");
	printf("Process ID:: %i\n", prc->pid);
	printf("Process Priority: %i\n", prc->priority);
	printf("Process State: %s\n", prc->state);
	printf("Process Msg: %s\n", prc->procmsg->message);
	
	//Ready QUEUE
	printf("-----------READY QUEUES HIGH PRIORITY-------------\n");
    PrintList(readyQueueHigh);
    printf("-----------READY QUEUES NORMAL PRIORITY-------------\n");
    PrintList(readyQueueNormal);
    printf("-----------READY QUEUES LOW PRIORITY-------------\n");
    PrintList(readyQueueLow);

	// Send QUEUE
	printf("----------------SEND QUEUE------------------------\n");
	PrintList(sendBlockedQueue);

    // Receive QUEUE
	printf("-----------------RECEIVE QUEUE----------------------\n");
	PrintList(receiveOperationQueue);

    // MSG QUEUE
	printf("-----------------MESSAGE QUEUE-----------------------\n");
	PrintList(msgQueue);

    printf("----------------SEM QUEUES----------------------\n");
	printSemList();
}
Semaphore* findSem(int pid){
    Semaphore *found = NULL;
    found = List_search(semaphoreQueue, compareInt, &pid);
	if (found) {
        return found;
	}
    return found;
}
void newSemaphore(int semaphoreID, int initialValue) {
    if (semaphoreID < 0 || semaphoreID >= 5) {
        printf("Invalid semaphore ID\n");
        return;
    }
    List_first(semaphoreQueue);
    if (List_search(semaphoreQueue, compareInt, &semaphoreID) == NULL) {
        Semaphore *s = malloc(sizeof(Semaphore));
        s->pid = semaphoreID;
		s->value = initialValue;
		s->blockedProcesses = List_create();
		List_append(semaphoreQueue, s);
        printf("Semaphore %d initialized with value %d\n", semaphoreID, initialValue);
    }else{
        printf("Semaphore: %d Already exist\n", semaphoreID);
    }
}

void semaphoreP(int semaphoreID) {
    if (semaphoreID < 0 || semaphoreID >= 5) {
        printf("Invalid semaphore ID\n");
        return;
    }
	PCB* currentProcess = List_last(runningProcessQueue);
	if (currentProcess->pid != 0) {
        Semaphore *s;
        s = findSem(semaphoreID);
        if (s!=NULL) {
            s->value--;
            if (s->value < 0) {
                currentProcess = List_trim(runningProcessQueue);
                strcpy(currentProcess->state, "BLOCKED");
                List_append(s->blockedProcesses, currentProcess);
                printf("Process performed P operation on semaphore %d\n", semaphoreID);
			    CPUScheduler(); // Call scheduler as current process is blocked
		    }
        }else{
            printf("SemphoreP: Given semphore not found\n");
        }
	}else{
		printf(" Init Process cannot perfor P operation");
		return;
	}
}

void semaphoreV(int semaphoreID) {
	if (semaphoreID < 0 || semaphoreID >= 5) {
        printf("Invalid semaphore ID\n");
        return;
    }
    Semaphore *s;
    s = findSem(semaphoreID);
    if (s!=NULL) {
        s->value++;
		if (s->value <= 0) {
            PCB* p = List_trim(s->blockedProcesses);
			if(p!=NULL){
				switch(p->priority) {
                    case 0: List_prepend(readyQueueHigh, p); break;
                    case 1: List_prepend(readyQueueNormal, p); break;
                    case 2: List_prepend(readyQueueLow, p); break;
                    default: printf("Invalid priority level\n"); break;
                }
				strcpy(p->state, "READY");
                printf("SUCCESS: Process with pid %d unblocked from semaphoreID: %d and moved to ready queue\n", p->pid, semaphoreID);
			}
		}
    }
    else{
        printf("semaphoreV: semaphoreID not found\n");
    }
}

void initSystem() {
    // Initialize system lists
    readyQueueHigh = List_create();
    readyQueueNormal = List_create();
    readyQueueLow = List_create();
    runningProcessQueue = List_create();
    sendBlockedQueue = List_create();
    receiveOperationQueue = List_create();
    msgQueue = List_create();
    semaphoreQueue = List_create();

    PCB *init;
    init = malloc(sizeof(PCB));
	init->pid = 0;
	init->priority = 0;
	strcpy(init->state, "READY");
    List_append(runningProcessQueue, init);
    strcpy(init->state, "READY");
}

int main() {
    printf("Process Management Simulation\n");
    initSystem();
    char command;
    int pid, semID, initValue, priority;
    char message[100]; // Assuming messages are no longer than 100 characters
    
    while (1) {
        printf("Enter command: ");
        scanf(" %c", &command); // Note the space before %c to skip any leading whitespace

        switch (toupper(command)) {
            case 'C':
                printf("Enter priority level (0, 1, 2)\n");
                scanf("%d", &priority);
                createProcess(priority);
                break;
            case 'F': // Fork
                forkP();
                break;
            case 'K': // Kill
                printf("Enter PID to kill: ");
                scanf("%d", &pid);
                kill(pid);
                break;
            case 'E': // Exit
                exitP();
                break;
            case 'Q': // Quantum
                quantum();
                break;
            case 'N':
                printf("Please enter the semaphore ID (0-4): \n");
                scanf("%d", &semID);
                printf("Please enter the initial value: \n");
                scanf("%d", &initValue);
                newSemaphore(semID, initValue);
                break;
            case 'P':
                printf("Please enter the semaphore ID (0-4): \n");
                scanf("%d", &semID);
                semaphoreP(semID);
                break;
            case 'V':
                printf("Please enter the semaphore ID (0-4): \n");
                scanf("%d", &semID);
                semaphoreV(semID);
                break;
            case 'S': // Send
                printf("Enter receiver PID: \n");
                scanf("%d", &pid); 
                getchar(); // Consume the newline character left in the input buffer
                printf("Enter message: \n");
                fgets(message, sizeof(message), stdin); // Read the message, including spaces
                // Remove the newline character at the end of the message, if any
                size_t len1 = strlen(message);
                if (len1 > 0 && message[len1 - 1] == '\n') {
                    message[len1 - 1] = '\0';
                }
                Send(pid, message);
                break;
            case 'Y': // Reply
                printf("Enter receiver PID: \n");
                scanf("%d", &pid); // Read PID and message
                getchar();
                printf("Enter your message: \n");
                fgets(message, sizeof(message), stdin); // Read the message, including spaces
                // Remove the newline character at the end of the message, if any
                size_t len = strlen(message);
                if (len > 0 && message[len - 1] == '\n') {
                    message[len - 1] = '\0';
                }
                reply(pid, message);
                break;
            case 'R': // Receive
                receive();
                break;
            case 'I': // Process info
                printf("Enter PID for info: ");
                scanf("%d", &pid);
                procinfo(pid);
                break;
            case 'T': // Total system info
                totalinfo();
                break;
            default:
                printf("Unknown command.\n");
        }
    }
    return 0;

}
