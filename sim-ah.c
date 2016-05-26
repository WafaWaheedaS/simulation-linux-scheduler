#include <stdlib.h>

#include <stdio.h>

struct task{
	int id;
	int bt;
	int at;
	int pr;
	int ts;
	int type;//0: real time 1: none real time
};

typedef struct task Task;
typedef Task *TaskPtr;


struct qnode{
	Task data;
	struct qnode *nextPtr;
};
typedef struct qnode Qnode;
typedef Qnode *QnodePtr; 

struct schedEvent{
	int event;
	int time;
	Task task;
	int processor;
};
typedef struct schedEvent SchedEvent;
typedef SchedEvent *SchedEventPtr;

struct eventQnode{
	SchedEvent data;
	struct eventQnode *nextPtr;
};
typedef struct eventQnode EventQnode;
typedef EventQnode *EventQnodePtr;	




void enqueue(QnodePtr *headPtr, QnodePtr *tailPtr,Task task);
void enqueuepr(QnodePtr *headPtr, QnodePtr *tailPtr,Task task);
void enqueueevent(EventQnodePtr *headPtr, EventQnodePtr *tailPtr,SchedEvent e);
Task dequeue(QnodePtr *headPtr, QnodePtr *tailPtr); 
SchedEvent dequeueevent(EventQnodePtr *headPtr, EventQnodePtr *tailPtr); 
int isEmpty(QnodePtr headPtr);
int isEmptyEQ(EventQnodePtr headPtr);
void displayEvents(EventQnodePtr currentPtr);
void calcTasksStat(EventQnodePtr currentPtr);

const int MAXQUEUES = 2;
const int MAXTASKS = 1000;
const int MAXBURSTTIME = 70;
const int IAT = 90;
const int QTRT = 30;
const int QTNRT = 10;

double avrWT=0, avrWTRT=0, avrWTNRT=0;
int wt=0,wtRT=0,wtNRT=0;
int fTasks=0, fTasksRT=0, fTasksNRT=0;

void main(){
	QnodePtr headPtr[MAXQUEUES], tailPtr[MAXQUEUES];
	EventQnodePtr eventsQheadPtr=NULL, eventsQtailPtr=NULL;
	EventQnodePtr fEQheadPtr=NULL, fEQtailPtr=NULL;
	Task tasks[MAXTASKS];
	SchedEvent sysevents[MAXTASKS];
	
	int i=0;
	for(i=0;i<MAXQUEUES;i++){
		headPtr[i]=NULL;
		tailPtr[i]=NULL;
	}

	int prevAt = 0;
	//int i;
	Task task;
	SchedEvent sysevent;
	for(i=0;i<MAXTASKS;i++){
		task.id=i;
		task.at=rand()%IAT+prevAt;
		prevAt=task.at;
		task.bt=rand()%MAXBURSTTIME+1;
		task.ts=0;
		task.type=rand()%2;
		if(task.type==0)
			task.pr=rand()%(100);//RT
		else
			task.pr=rand()%41 + 100; //NRT
		sysevent.event=0;
		sysevent.time=task.at;
		sysevent.processor=-1;
		sysevent.task=task;
		enqueueevent(&eventsQheadPtr,&eventsQtailPtr,sysevent);
	}

	int simTime=0;
	int idle=1;
	SchedEvent currentEvent;
	QnodePtr aqheadPtr=headPtr[0], aqtailPtr=tailPtr[0];
	QnodePtr eqheadPtr=headPtr[1], eqtailPtr=tailPtr[1];
	QnodePtr tqPtr;
	printf("\nTime: %d: Simulation is started ...\n",simTime);
	while(!isEmptyEQ(eventsQheadPtr)){
		currentEvent = dequeueevent(&eventsQheadPtr,&eventsQtailPtr);
		simTime=currentEvent.time;
		if(currentEvent.event==1){
			idle=1;
			if(currentEvent.task.ts < currentEvent.task.bt){
				printf("\nTime: %d: enqueu task %d in expired Q.\n",
					simTime, currentEvent.task.id);
				if(currentEvent.task.type==1)//NRT
					currentEvent.task.pr+=5;
				enqueuepr(&eqheadPtr,&eqtailPtr,currentEvent.task);
			}
			else{
				printf("\nTime: %d: task %d has finished.\n",
					simTime, currentEvent.task.id);
				enqueueevent(&fEQheadPtr,&fEQtailPtr,currentEvent);
			}
			if(isEmpty(aqheadPtr)&&!isEmpty(eqheadPtr)){
				printf("\nTime: %d:...switching queues...\n",simTime);
				tqPtr = aqheadPtr;
				aqheadPtr = eqheadPtr;
				eqheadPtr = tqPtr;
				tqPtr = aqtailPtr;
				aqtailPtr = eqtailPtr;
				eqtailPtr = tqPtr;
			}
		}
		if(currentEvent.event==0){
			printf("\nTime: %d: enqueu task %d in active Q.\n", 
				simTime, currentEvent.task.id);
			enqueuepr(&aqheadPtr,&aqtailPtr,currentEvent.task);
			
		}
		if(idle==1&&!isEmpty(aqheadPtr)){
			idle = 0;
			currentEvent.event=1;
			currentEvent.task=dequeue(&aqheadPtr,&aqtailPtr);
			printf("\nTime: %d: serving task %d.\n", simTime,currentEvent.task.id);
			int qt;
			if(currentEvent.task.type==0)
				qt=QTRT;
			else
				qt=QTNRT;
			int min = currentEvent.task.bt-currentEvent.task.ts;
			if(min>qt)
				min=qt;
			currentEvent.time=simTime+min;
			currentEvent.task.ts+=min;
			enqueueevent(&eventsQheadPtr,&eventsQtailPtr,currentEvent);
		}			
	}

	printf("\nSimulation has finished.\n");
	calcTasksStat(fEQheadPtr);
	printf("\nResults:\n");
	printf("Total tasks served = %d, where %d are RT and %d are NRT tasks.\n", 			fTasks,fTasksRT,fTasksNRT);
	printf("Average wating time for all processes = %f\n",avrWT);
	printf("Average wating time for RT processes = %f\n",avrWTRT);
	printf("Average wating time for NRT processes = %f\n",avrWTNRT);
}

void enqueue(QnodePtr *headPtr, QnodePtr *tailPtr,Task task){
	QnodePtr newNodePtr = malloc( sizeof( Qnode));
	if(newNodePtr !=NULL){
		newNodePtr->data = task;
		newNodePtr->nextPtr = NULL;
		if(isEmpty(*headPtr)){
			*headPtr = newNodePtr;
		}
		else{
			(*tailPtr)->nextPtr = newNodePtr;
		}
		*tailPtr = newNodePtr;
	}
}
void enqueuepr(QnodePtr *headPtr, QnodePtr *tailPtr,Task task){
	QnodePtr newNodePtr = malloc( sizeof( Qnode));
	if(newNodePtr !=NULL){
		newNodePtr->data = task;
		newNodePtr->nextPtr = NULL;
	}
	QnodePtr current = *headPtr, prev = NULL;
	while(current!=NULL && task.pr>=(current->data).pr){
		prev = current;
		current = current->nextPtr;
	}
	if(prev==NULL){
		newNodePtr->nextPtr= *headPtr;
		*headPtr=newNodePtr;
	}
	else{
		newNodePtr->nextPtr=prev->nextPtr;		
		prev->nextPtr=newNodePtr;			
	}	
	if(newNodePtr->nextPtr==NULL){
		*tailPtr = newNodePtr;
	}
}

void enqueueevent(EventQnodePtr *headPtr, EventQnodePtr *tailPtr,SchedEvent se){
	EventQnodePtr newNodePtr = malloc( sizeof( EventQnode));
	if(newNodePtr !=NULL){
		newNodePtr->data = se;
		newNodePtr->nextPtr = NULL;
	}
	EventQnodePtr current = *headPtr, prev = NULL;
	while(current!=NULL && se.time>(current->data).time){
		prev = current;
		current = current->nextPtr;
	}
	while(current!=NULL && se.time==(current->data).time && se.event<(current->data).event){
		prev = current;
		current = current->nextPtr;
	}

	if(prev==NULL){
		newNodePtr->nextPtr= *headPtr;
		*headPtr=newNodePtr;
	}
	else{
		newNodePtr->nextPtr=prev->nextPtr;		
		prev->nextPtr=newNodePtr;			
	}	
	if(newNodePtr->nextPtr==NULL){
		*tailPtr = newNodePtr;
	}
}

Task dequeue(QnodePtr *headPtr, QnodePtr *tailPtr){
	Task value;
	QnodePtr tempPtr;
	value = (*headPtr)->data;
	tempPtr = *headPtr;
	*headPtr = (*headPtr)->nextPtr;
	if(*headPtr == NULL){
		*tailPtr = NULL;
	}
	free (tempPtr);
	return value;
}
SchedEvent dequeueevent(EventQnodePtr *headPtr, EventQnodePtr *tailPtr){
	SchedEvent value;
	EventQnodePtr tempPtr;
	value = (*headPtr)->data;
	tempPtr = *headPtr;
	*headPtr = (*headPtr)->nextPtr;
	if(*headPtr == NULL){
		*tailPtr = NULL;
	}
	free (tempPtr);
	return value;
}
int isEmpty(QnodePtr headPtr){
	return headPtr == NULL;
}
int isEmptyEQ(EventQnodePtr headPtr){
	return headPtr == NULL;
}
void displayEvents(EventQnodePtr currentPtr){
	if(currentPtr==NULL)
		printf("Queue is empty ...\n");
	else{
		printf("Queue is:\n");
		SchedEvent tempevent;
		while(currentPtr!=NULL){
			printf("\ntime: %d, event: %d\n",
			(currentPtr->data).time, (currentPtr->data).event);
			currentPtr=currentPtr->nextPtr;
		}
	}
}
void calcTasksStat(EventQnodePtr currentPtr){
	if(currentPtr==NULL)
		return;
	else{
		SchedEvent tempevent;
		while(currentPtr!=NULL){
			fTasks++;
			wt+=(currentPtr->data).time-
			    ((currentPtr->data).task.at+(currentPtr->data).task.bt);
			if((currentPtr->data).task.type==0){
				wtRT+=(currentPtr->data).time-
			    	((currentPtr->data).task.at+(currentPtr->data).task.bt);
				fTasksRT++;
			}
			else{
				wtNRT+=(currentPtr->data).time-
			    	((currentPtr->data).task.at+(currentPtr->data).task.bt);
				fTasksNRT++;		
			}
			currentPtr=currentPtr->nextPtr;
		}
		avrWT=(double)wt/fTasks;
		avrWTRT=(double)wtRT/fTasksRT;
		avrWTNRT=(double)wtNRT/fTasksNRT;
	}
}






















