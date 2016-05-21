            #include <stdlib.h>
#include <stdio.h>

struct task{
	int id;
	int bt;
	int at;
	int pr;
	int ts; //$$ 0 is new status
	int onCpu;
	int type;//0: real time RT 1: none real time NRT
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
	int type; //event type 0:arrival, 1: departure
	int time;
	Task task;
	int cpu;
};
typedef struct schedEvent SchedEvent;
typedef SchedEvent *SchedEventPtr;

struct eventQnode{
	SchedEvent data;
	struct eventQnode *nextPtr;
};
typedef struct eventQnode EventQnode;
typedef EventQnode *EventQnodePtr;


struct prioLinkList{
    int prio;
    int numTask;          //priority number
    QnodePtr pArrHeadPtr;
    QnodePtr pArrTailPtr;
};
typedef struct prioLinkList PrioLinkList;


struct prioArray{
    int nr_active;      /* number of tasks */
//    int type; //type=0 activeQ, type=1 expiaryQ
    PrioLinkList prioList[140];
};
typedef struct prioArray PrioArray;

struct runQueue{
    int nr_active; // which CPU this belongs too
    PrioArray  activeQ;       /* pointer to the active priorityarray */
    PrioArray  expiredQ;      /* pointer to the expiredpriority array */
};
typedef struct runQueue RunQueue;




SchedEvent serviceTask(int onCpu, SchedEvent currentEvent);
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
int simTime;

int *isIdle;
RunQueue *runQueue;

void main(){

    int numCpus;
    int ratioRT;
    int ratioNRT;
    int i,j;


    printf("Linux Scheduler Simulator (1000 tasks)\n");

    printf("Please enter the number of CPUs to simulate: ");
    scanf("%d", &numCpus);

    //allocate isIdle and runQueue their sizes
    isIdle = malloc(sizeof(int)*numCpus);
    if(isIdle==NULL){
        printf("memory allocation error");
    }

    runQueue = malloc(sizeof(RunQueue)*numCpus);
    if(runQueue==NULL){
        printf("memory allocation error");
    }

    //initilize isIdle and runQueue
    for(i=0;i<numCpus;i++){

        isIdle[i]=1;
        runQueue[i].nr_active=0;
        runQueue[i].activeQ.nr_active=0;
        runQueue[i].expiredQ.nr_active=0;

        for(j=0;j<140;j++){
            runQueue[i].activeQ.prioList[j].numTask=0;
            runQueue[i].activeQ.prioList[j].pArrHeadPtr=NULL;
            runQueue[i].activeQ.prioList[j].pArrTailPtr=NULL;

            runQueue[i].expiredQ.prioList[j].numTask=0;
            runQueue[i].expiredQ.prioList[j].pArrHeadPtr=NULL;
            runQueue[i].expiredQ.prioList[j].pArrTailPtr=NULL;
        }
    }//end for

    printf("Please enter the precentage of RT tasks to simulate: ");
    scanf("%d", &ratioRT);
    printf("Please enter the precentage of NRT tasks to simulate: ");
    scanf("%d", &ratioNRT);

    while((ratioNRT+ratioRT)!=100){
        printf("the sum of the RT and NRT presentages must equal 100.\n");
        printf("please enter the values again.\n");

        printf("Please enter the precentage of RT tasks to simulate: ");
        scanf("%d", &ratioRT);
        printf("Please enter the precentage of NRT tasks to simulate: ");
        scanf("%d", &ratioNRT);
    }//end while


    float numRT;
    float numNRT;

    //get number of RT and NRT tasks to be generated
    numRT=(ratioRT/100.0)*MAXTASKS;
    numNRT=(ratioNRT/100.0)*MAXTASKS;


	EventQnodePtr eventsQheadPtr=NULL, eventsQtailPtr=NULL;
	Task task;
	SchedEvent sysevent;
	int prevAt = 0;
	int type;

	for(i=0;i<MAXTASKS;i++){

		task.id=i;
		task.at=rand()%IAT+prevAt;
		prevAt=task.at;
		task.bt=rand()%MAXBURSTTIME+1;
		task.ts=0;                      //new taskstatus
		type=rand()%2;                  //either 0 for RT, or 1 for NRT

		if(type==0 && numRT!=0){
            numRT--;
            task.type=0;
			task.pr=rand()%(100);          //RT prio from 0 to 99
        }
		else if(type==0 && numRT==0){
            numNRT--;
            task.type=1;
			task.pr=rand()%40 + 100;       //NRT prio from 100 to 139
        }
        else if(type==1 && numNRT!=0){
            numNRT--;
            task.type=1;
			task.pr=rand()%40 + 100;       //NRT prio from 100 to 139
        }
        else if(type==1 && numNRT==0){
            numRT--;
            task.type=0;
			task.pr=rand()%(100);           //RT prio from 0 to 99
        }

        sysevent.type=0;                    //arival
		sysevent.time=task.at;
		sysevent.task=task;
		enqueueevent(&eventsQheadPtr,&eventsQtailPtr,sysevent);
    }

    //start simulation
	simTime=0;
	SchedEvent currentEvent;

	printf("\nTime: %d: Simulation is started ...\n",simTime);

    i=0;                                    //start with fisrt CPU and divide tasks iteratively
    int highestPrio;
    int initType;
    PrioArray tempArray;

	while(!isEmptyEQ(eventsQheadPtr)){

		currentEvent = dequeueevent(&eventsQheadPtr,&eventsQtailPtr);

		simTime=currentEvent.time;
        initType=currentEvent.type;

		if(currentEvent.type==1){   //Departure

			isIdle[currentEvent.task.onCpu]=1;

			if(currentEvent.task.ts < currentEvent.task.bt){

				printf("\nTime: %d: enqueu task %d in expired Q of CPU %d.\n", simTime, currentEvent.task.id, currentEvent.task.onCpu);

				if(currentEvent.task.type==0)//NRT
					currentEvent.task.pr+=5;

                enqueue(&runQueue[currentEvent.task.onCpu].expiredQ.prioList[currentEvent.task.pr].pArrHeadPtr,
                        &runQueue[currentEvent.task.onCpu].expiredQ.prioList[currentEvent.task.pr].pArrTailPtr, currentEvent.task);

                runQueue[currentEvent.task.onCpu].nr_active++;
                runQueue[currentEvent.task.onCpu].expiredQ.nr_active++;
                runQueue[currentEvent.task.onCpu].expiredQ.prioList[currentEvent.task.pr].numTask++;

            }
            else{
                printf("\nTime: %d: task %d has finished on CPU %d.\n", simTime, currentEvent.task.id, currentEvent.task.onCpu);
            }
            //switch Queues if active queue is empty
			if(isEmptyActiveQ(runQueue[currentEvent.task.onCpu]) && !isEmptyExpireQ(runQueue[currentEvent.task.onCpu])){

				printf("\nTime: %d:...switching queues on CPU %d...\n",simTime, currentEvent.task.onCpu);

                tempArray = runQueue[currentEvent.task.onCpu].activeQ;
				runQueue[currentEvent.task.onCpu].activeQ = runQueue[currentEvent.task.onCpu].expiredQ;
				runQueue[currentEvent.task.onCpu].expiredQ = tempArray;
			}
            //service next task
			if( isIdle[currentEvent.task.onCpu] && !isEmptyActiveQ(runQueue[currentEvent.task.onCpu]))
			{
                currentEvent = serviceTask(currentEvent.task.onCpu, currentEvent);
                enqueueevent(&eventsQheadPtr,&eventsQtailPtr,currentEvent);
			}
		}//end Departure

        if(currentEvent.type==0){   //arrival

			printf("\nTime: %d: enqueu task id %d in active Q of CPU %d.\n", simTime, currentEvent.task.id, i);

			currentEvent.task.onCpu=i;

			enqueue(&runQueue[i].activeQ.prioList[currentEvent.task.pr].pArrHeadPtr,
                    &runQueue[i].activeQ.prioList[currentEvent.task.pr].pArrTailPtr, currentEvent.task);

            runQueue[i].nr_active++;
            runQueue[i].activeQ.nr_active++;
            runQueue[i].activeQ.prioList[currentEvent.task.pr].numTask++;
		}//end Arrival

		if(isIdle[i]==1 && !isEmptyActiveQ(runQueue[i])){      //service tasks

			currentEvent = serviceTask(i, currentEvent);
			enqueueevent(&eventsQheadPtr,&eventsQtailPtr,currentEvent);
		}//end Service

		//increment Cpu counter only after processing arrivals
		if(initType==0){
            i=(i+1)%numCpus;
		}
	}//end while

	printf("\nSimulation has finished.\n");
	/*calcTasksStat(fEQheadPtr);
	printf("\nResults:\n");
	printf("Total tasks served = %d, where %d are RT and %d are NRT tasks.\n", 			fTasks,fTasksRT,fTasksNRT);
	printf("Average wating time for all processes = %f\n",avrWT);
	printf("Average wating time for RT processes = %f\n",avrWTRT);
	printf("Average wating time for NRT processes = %f\n",avrWTNRT);*/

}

SchedEvent serviceTask(int onCpu, SchedEvent currentEvent){

          //  printf("enter service");
			SchedEvent newEvent;

            newEvent = currentEvent;

			isIdle[onCpu] = 0;

			newEvent.type=1; //departure

           // printf("getting high prio");
			int highestPrio = getHighestPriority(onCpu);

           // printf("dequeue task");
			newEvent.task=dequeue(&runQueue[onCpu].activeQ.prioList[highestPrio].pArrHeadPtr,
                                  &runQueue[onCpu].activeQ.prioList[highestPrio].pArrTailPtr);

//printf("decre active Q");
            runQueue[onCpu].activeQ.nr_active--;
            runQueue[onCpu].activeQ.prioList[highestPrio].numTask--;

			printf("\nTime: %d: serving task %d on CPU %d.\n", simTime,newEvent.task.id, onCpu);

			int qt; //quantum time
			if(newEvent.task.type==0) // RT task
				qt=QTRT;
			else
				qt=QTNRT;
			int min = newEvent.task.bt-newEvent.task.ts;
			if(min>qt)
				min=qt;
			newEvent.time=simTime+min;
			newEvent.task.ts+=min;

			return newEvent;
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

int isEmptyActiveQ(RunQueue runQueue){
    if(runQueue.activeQ.nr_active!=0)
        return 0;
    else
        return 1;
}

int isEmptyExpireQ(RunQueue runQueue){
    if(runQueue.expiredQ.nr_active!=0)
        return 0;
    else
        return 1;
}

int getHighestPriority(int onCpu){

    int i;
    int prio=-1;

    for(i=0;i<140;i++){
        if(runQueue[onCpu].activeQ.prioList[i].numTask!=0){
            prio=i;
            break;
        }
    }
    return prio;

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
	while(current!=NULL && se.time==(current->data).time && se.type<(current->data).type){
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
			(currentPtr->data).time, (currentPtr->data).type);
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



/*printf(" task of prio %d, i is %d,\n", currentEvent.task.pr,  i);
        for(j=0;j<numCpus;j++){
        printf("isIdle of %d is %d \n", j, isIdle[j]) ;

        printf("run Q tasks of cpu %d is %d \n", j , runQueue[j].nr_active);
        printf("active Q tasks of cpu %d is %d \n", j , runQueue[j].activeQ.nr_active);
        printf("expired Q tasks of cpu %d is %d \n", j , runQueue[j].expiredQ.nr_active);


        for(k=0;k<140;k++){
            printf("num of task in active Q tasks of prio %d is %d \n", k, runQueue[j].activeQ.prioList[k].numTask);
        printf("num of task in expired Q tasks of prio %d is %d \n", k, runQueue[j].expiredQ.prioList[k].numTask);
        }
        }*/













            /*
			isIdle[i] = 0;

			currentEvent.type=1;    //departure
			highestPrio = getHighestPriority(runQueue[i]);

			currentEvent.task=dequeue(&runQueue[i].activeQ.prioList[highestPrio].pArrHeadPtr,
                                        &runQueue[i].activeQ.prioList[highestPrio].pArrTailPtr);

            runQueue[i].activeQ.nr_active--;
            runQueue[i].activeQ.prioList[highestPrio].numTask--;

			printf("\nTime: %d: serving task %d.\n", simTime,currentEvent.task.id);
			int qt; //quantum time
			if(currentEvent.task.type==0) // RT task
				qt=QTRT;
			else
				qt=QTNRT;
			int min = currentEvent.task.bt-currentEvent.task.ts;
			if(min>qt)
				min=qt;
			currentEvent.time=simTime+min;
			currentEvent.task.ts+=min;
			*/




