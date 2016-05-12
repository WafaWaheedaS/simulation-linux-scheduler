//#include <stdlib.h>
//#include <shared.h>
//#include <stdio.h>
//#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
//#include <sys/ipc.h>



int cpus=3;
int MAXQUEUES=6;
//MAXQUEUES = 2*cpus;
int MAXTASKS = 1000;
int MAXBURSTTIME = 70;
int IAT = 90;
int QTRT = 30;
int QTNRT = 10;

struct task{
	int id;
	int bt;
	int at;
	int pr;
	int ts;
	int type;//0: real time 1: none real time
	int rt, rf; //responce time and reponce time flag
	int tt;  //turn around time;
	int wt, wf;	//waiting time, and waiting time flag
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
	//int processor
	int size;
	//size=  MAXTASKS;
	//in process[1000] = {0};
	//fill process with 0s.
	//int i;
	//for (i=0; i<1000; i++)
		//process[i]=0;
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

double avrWT=0, avrWTRT=0, avrWTNRT=0;
int wt=0,wtRT=0,wtNRT=0;
int fTasks=0, fTasksRT=0, fTasksNRT=0;

void main(){
	QnodePtr headPtr[MAXQUEUES], tailPtr[MAXQUEUES];
	EventQnodePtr eventsQheadPtr=NULL, eventsQtailPtr=NULL;
	EventQnodePtr fEQheadPtr=NULL, fEQtailPtr=NULL;
	Task tasks[MAXTASKS];
	SchedEvent sysevents[MAXTASKS];
	int naQi = MAXQUEUES/2; //active Queue index = #of cpus
	int capacity = MAXTASKS/naQi; // the capacity of each active and expired Queue in ecah cpu

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
		//set the responce time and responce flag to zero
		task.rt=0;
		task.rf=0;
		//set the turn around time to zero
		task.tt=0;
		//set the waiting time and it's flag to zero
		task.wt=0;
		task.wf=0;
		sysevent.event=0;
		sysevent.time=task.at;
		//sysevent.processor=-1;
		sysevent.task=task;
		//assighine a task to process
		int s;
		s = sysevent.size;
		/*int pi;
		for(pi=0; pi<s; pi++)
		{
			if(sysevent.process[pi]==0)
			{
				sysevent.process[i]= task.id;
				break;
			}
		}*/
		enqueueevent(&eventsQheadPtr,&eventsQtailPtr,sysevent);
	}

	int simTime[naQi];
	int idle[naQi];
	int ni;
	for(ni=0; ni<naQi; ni++)
	{
		simTime[ni]=0;
		idle[ni]=1;
	}

	SchedEvent currentEvent[naQi];

	//QnodePtr aqheadPtr=headPtr[0], aqtailPtr=tailPtr[0];
	//QnodePtr eqheadPtr=headPtr[1], eqtailPtr=tailPtr[1];
	QnodePtr aqheadPtr[naQi];
	QnodePtr aqtailPtr[naQi];
	QnodePtr eqheadPtr[naQi];
	QnodePtr eqtailPtr[naQi];
	//fill them
	int aindex = 0; //index counter of active and expired queues
	int q;
	for(q=0; q<naQi; q++)
	{
		aqheadPtr[q]= headPtr[aindex];
		aqtailPtr[q]= tailPtr[aindex];
		aindex++;
		eqheadPtr[q]= headPtr[aindex];
		eqtailPtr[q]= tailPtr[aindex];
		aindex++;
	}

	QnodePtr tqPtr;
	int  aQc = 0; //the queue capacity counter
	int evei;
	for(evei=0; evei<naQi; evei++, aQc=0){

		printf("\nTime: %d: Simulation is started from cpu # %d ...\n",simTime[evei],evei);
	while(!isEmptyEQ(eventsQheadPtr)){
		currentEvent[evei] = dequeueevent(&eventsQheadPtr,&eventsQtailPtr);
		simTime[evei]=currentEvent[evei].time;
		if(currentEvent[evei].event==1){
			idle[evei]=1;
			if(currentEvent[evei].task.ts < currentEvent[evei].task.bt){
				printf("\nTime: %d: enqueu task %d in expired Q in CPU # %d.\n",
					simTime[evei], currentEvent[evei].task.id,evei);
				if(currentEvent[evei].task.type==1)//NRT
					currentEvent[evei].task.pr+=5;
				enqueuepr(&eqheadPtr[evei],&eqtailPtr[evei],currentEvent[evei].task);
			}
			else{
				printf("\nTime: %d: task %d has finished in CPU# %d.\n",
					simTime[evei], currentEvent[evei].task.id,evei);
				enqueueevent(&fEQheadPtr,&fEQtailPtr,currentEvent[evei]);
			}
			if(isEmpty(aqheadPtr[evei])&&!isEmpty(eqheadPtr[evei])){
				printf("\nTime: %d:...switching queues in cpu %d...\n",simTime[evei],evei);
				tqPtr = aqheadPtr[evei];
				aqheadPtr[evei] = eqheadPtr[evei];
				eqheadPtr[evei] = tqPtr;
				tqPtr = aqtailPtr[evei];
				aqtailPtr[evei] = eqtailPtr[evei];
				eqtailPtr[evei] = tqPtr;
			}
		} //end event = 1
		if(currentEvent[evei].event==0){
			printf("\nTime: %d: enqueu task %d in active Q in cpu # %d.\n", 
				simTime[evei], currentEvent[evei].task.id, evei);
			if(aQc>= capacity)
				break;
			enqueuepr(&aqheadPtr[evei],&aqtailPtr[evei],currentEvent[evei].task);
			aQc++;
			
		}
		if(idle[evei]==1 && !isEmpty(aqheadPtr[evei])){
			idle[evei] = 0;
			currentEvent[evei].event=1;
			currentEvent[evei].task=dequeue(&aqheadPtr[evei],&aqtailPtr[evei]);
			printf("\nTime: %d: serving task %d from CPU # %d. \n", simTime[evei],currentEvent[evei].task.id, evei);
			//count the responce time:
			if(currentEvent[evei].task.rf==0) { //check the responce time flag 
				currentEvent[evei].task.rf=1; 
				currentEvent[evei].task.rt= simTime[evei]- currentEvent[evei].task.at;
			}
			//count the waiting time 
			if(currentEvent[evei].task.wf==0){ //check if it the first time you count the waitting time
				currentEvent[evei].task.wf=1;
				currentEvent[evei].task.wt= simTime[evei]- currentEvent[evei].task.bt- currentEvent[evei].task.at;
			}
			else { //not the first waiting time
				currentEvent[evei].task.wt += simTime[evei];
				currentEvent[evei].task.wt -= currentEvent[evei].task.bt;
			}
			int qt;
			if(currentEvent[evei].task.type==0)
				qt=QTRT;
			else
				qt=QTNRT;
			int min = currentEvent[evei].task.bt-currentEvent[evei].task.ts;
			if(min>qt)
				min=qt;
			currentEvent[evei].time=simTime[evei]+min;
			currentEvent[evei].task.ts+=min;
			enqueueevent(&eventsQheadPtr,&eventsQtailPtr,currentEvent[evei]);
		}			
	}//end while
	} //end for evei

	printf("\nSimulation has finished.\n");
	calcTasksStat(fEQheadPtr);
	printf("\nResults:\n");
	printf("Total tasks served = %d, where %d are RT and %d are NRT tasks.\n",fTasks,fTasksRT,fTasksNRT);
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
