#include <stdlib.h>
#include <stdio.h>

struct task{
	int id;
	int bt;
	int at;
	int pr; //  priority
	int status;  //$$ 0 is new status
	int enteredQueue;
	int onCpu;
	double avgST; // avg sleep time
	int totalST; //total sleep time
	int nsleep;  // track no of times sleep occurs
	//int interactivity;
	int type; //0: real time RT 1: none real time NRT
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
    int prevT;
    int avgQ;
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
int max(int a , int b);
int min(int a , int b);

const int MAXQUEUES = 2;
const int MAXTASKS = 1000;
const int MINBURSTTIME = 100;
const int MAXBURSTTIME = 1000;
const int IAT = 90;
const int QTRT = 30;
const int QTNRT = 10;

/*double avrWT=0, avrWTRT=0, avrWTNRT=0;
int wt=0,wtRT=0,wtNRT=0;
int fTasks=0, fTasksRT=0, fTasksNRT=0;*/
int simTime;

int *isIdle;
RunQueue *runQueue;
int *wt; //cpu wait time
int *idletime;
int *previdletime;
int *taskPerCpu;
int *allbtPerCPU;
int *rt; //response time
int *avgQL;
int *preSimT;
int *minBT;
int *maxBT;

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

    wt = malloc(sizeof(int)*numCpus);
    if(wt==NULL){
        printf("memory allocation error");
    }

    idletime = malloc(sizeof(int)*numCpus);
    if(idletime==NULL){
        printf("memory allocation error");
    }

    previdletime = malloc(sizeof(int)*numCpus);
    if(previdletime==NULL){
        printf("memory allocation error");
    }

    taskPerCpu = malloc(sizeof(int)*numCpus);
    if(taskPerCpu==NULL){
        printf("memory allocation error");
    }
    allbtPerCPU = malloc(sizeof(int)*numCpus);
    if(allbtPerCPU==NULL){
        printf("memory allocation error");
    }
    rt = malloc(sizeof(int)*numCpus);
    if(rt==NULL){
        printf("memory allocation error");
    }
    avgQL = malloc(sizeof(int)*numCpus);
    if(avgQL==NULL){
        printf("memory allocation error");
    }
    preSimT = malloc(sizeof(int)*numCpus);
    if(preSimT==NULL){
        printf("memory allocation error");
    }
    //initilize isIdle and runQueue
    for(i=0;i<numCpus;i++){

        isIdle[i]=1;
        runQueue[i].nr_active=0;
        runQueue[i].activeQ.nr_active=0;
        runQueue[i].expiredQ.nr_active=0;

        runQueue[i].activeQ.avgQ=0;
        runQueue[i].activeQ.prevT=0;
        runQueue[i].expiredQ.avgQ=0;
        runQueue[i].expiredQ.prevT=0;

        for(j=0;j<140;j++){
            runQueue[i].activeQ.prioList[j].numTask=0;
            runQueue[i].activeQ.prioList[j].pArrHeadPtr=NULL;
            runQueue[i].activeQ.prioList[j].pArrTailPtr=NULL;

            runQueue[i].expiredQ.prioList[j].numTask=0;
            runQueue[i].expiredQ.prioList[j].pArrHeadPtr=NULL;
            runQueue[i].expiredQ.prioList[j].pArrTailPtr=NULL;
        }
        wt[i]=0;
        idletime[i]=0;
        previdletime[i]=0;
        taskPerCpu[i]=0;
        allbtPerCPU[i]=0;
        rt[i]=0;
        avgQL[i]=0;
        preSimT[i]=0;
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
//printf("1test\n");

	EventQnodePtr eventsQheadPtr=NULL, eventsQtailPtr=NULL;
	Task task;
	SchedEvent sysevent;
	int prevAt = 0;
	int type;
	int allIAT=0; //initial arrival time
	int allbt=0; //total burst time

//printf("2test\n");

	for(i=0;i<MAXTASKS;i++){

		task.id=i;
		task.at=rand()%IAT+prevAt;
		prevAt=task.at;
		task.bt=rand()%MAXBURSTTIME+MINBURSTTIME;
		task.status=0;
		task.totalST=0;                    //new taskstatus
		task.nsleep =0;
		task.avgST = 0;   // assigning avg sleep time
        allbt+=task.bt; //&&
        allIAT+=task.at;
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
//printf("3test\n");

    //start simulation
	simTime=0;
	int starttime=(*eventsQheadPtr).data.time;
	SchedEvent currentEvent;

	printf("\nTime: %d: Simulation is started ...\n",simTime);

    i=0;                                    //start with fisrt CPU and divide tasks iteratively
    int highestPrio;
    int initType;
    PrioArray tempArray;
    //int prevT;
    int k=0;

	while(!isEmptyEQ(eventsQheadPtr) && k<50 ){

        //prevT=simTime;

		currentEvent = dequeueevent(&eventsQheadPtr,&eventsQtailPtr);
		simTime=currentEvent.time;


        initType=currentEvent.type;

		if(currentEvent.type==1){   //Departure

			isIdle[currentEvent.task.onCpu]=1;

			if(currentEvent.task.bt != 0){
                int bonus;
                int dp; //dynamic prio

				printf("\nTime: %d: enqueu task %d in expired Q of CPU %d.\n", simTime, currentEvent.task.id, currentEvent.task.onCpu);

				if(currentEvent.task.type==0){
                    bonus=getBonus(currentEvent.task.avgST);
                    dp=max(100,min(currentEvent.task.pr-bonus+5, 139));
                    currentEvent.task.pr=dp;
				}//NRT

                currentEvent.task.enteredQueue=simTime;
                enqueue(&runQueue[currentEvent.task.onCpu].expiredQ.prioList[currentEvent.task.pr].pArrHeadPtr,
                        &runQueue[currentEvent.task.onCpu].expiredQ.prioList[currentEvent.task.pr].pArrTailPtr, currentEvent.task);

                runQueue[currentEvent.task.onCpu].nr_active++;
                runQueue[currentEvent.task.onCpu].expiredQ.nr_active++;
                runQueue[currentEvent.task.onCpu].expiredQ.prioList[currentEvent.task.pr].numTask++;
                runQueue[currentEvent.task.onCpu].expiredQ.prevT=simTime;

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
                preSimT[i]=simTime;

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

            taskPerCpu[i]++;
            allbtPerCPU[i]+=currentEvent.task.bt;
			currentEvent.task.onCpu=i;
            currentEvent.task.enteredQueue=simTime;

            preSimT[i]=simTime;

			enqueue(&runQueue[i].activeQ.prioList[currentEvent.task.pr].pArrHeadPtr,
                    &runQueue[i].activeQ.prioList[currentEvent.task.pr].pArrTailPtr, currentEvent.task);

            runQueue[i].nr_active++;
            runQueue[i].activeQ.nr_active++;
            runQueue[i].activeQ.prioList[currentEvent.task.pr].numTask++;
            runQueue[i].activeQ.prevT=simTime;

            //avgQL[i]+=(runQueue[i].activeQ.nr_active++);
		}//end Arrival

		if(isIdle[i]==1 && !isEmptyActiveQ(runQueue[i])){      //service tasks

			currentEvent = serviceTask(i, currentEvent);
			enqueueevent(&eventsQheadPtr,&eventsQtailPtr,currentEvent);
		}//end Service

        if(previdletime[i]>=0){
		 	idletime[i]+=simTime-previdletime[i];
		  	previdletime[i]=-1;
		 }

        if(isIdle[i]==1)
			previdletime[i]=simTime;
		//increment Cpu counter only after processing arrivals
		if(initType==0){
            i=(i+1)%numCpus;
		}

		printf("Q length %d, avgQL %d", runQueue[i].activeQ.nr_active, runQueue[i].activeQ.avgQ);
		k++;
	}//end while

	printf("\nSimulation has finished.\n");

    int sumWT=0;
    int sumIdleTime=0;

    printf("\nSimulation statistics per CPU:\n");

	for(i=0;i<numCpus;i++){
        sumWT+=wt[i];
        sumIdleTime+=idletime[i];
       // printf("%d\n",taskPerCpu[i]);
        printf("CPU %d:\n",i);
        printf("\tAverage Delay Time = %f\n", (double)wt[i]/taskPerCpu[i]);
        printf("\tAverage Response Time = %f\n", (double)rt[i]/taskPerCpu[i]);
        printf("\tCPU Utilization = %f\n", (double)((simTime-starttime)-idletime[i])/ (simTime-starttime));
        printf("\tAverage Turnaround Time = %f\n", (double)(allbtPerCPU[i]+wt[i])/taskPerCpu[i]);
        printf("\tAverage Burst Time = %f\n", (double)allbtPerCPU[i]/taskPerCpu[i]);
        printf("\tAverage Queue Length = %f\n", (double)runQueue[i].activeQ.avgQ/simTime);

	}

	printf("\nOverall Simulation statistics:\n");
	printf("\tAverage wating time for all CPUs = %f\n",(double)sumWT/numCpus);
    printf("\tAverage idle time for all CPUs = %f\n",(double)sumIdleTime/numCpus);

	/*calcTasksStat(fEQheadPtr);
	printf("\nResults:\n");
	printf("Total tasks served = %d, where %d are RT and %d are NRT tasks.\n", 			fTasks,fTasksRT,fTasksNRT);
	printf("Average wating time for all processes = %f\n",avrWT);
	printf("Average wating time for RT processes = %f\n",avrWTRT);
	printf("Average wating time for NRT processes = %f\n",avrWTNRT);*/

}

SchedEvent serviceTask(int onCpu, SchedEvent currentEvent){

			SchedEvent newEvent;
            newEvent = currentEvent;
			isIdle[onCpu] = 0;
			newEvent.type=1; //departure

            if(newEvent.task.status==0){
                rt[onCpu]+=simTime-newEvent.task.at;
                newEvent.task.status=1;// not new
            }

			int highestPrio = getHighestPriority(onCpu);

            avgQL[onCpu]+=(simTime-preSimT[onCpu])*runQueue[onCpu].activeQ.nr_active;

            runQueue[onCpu].activeQ.avgQ+=(simTime-runQueue[onCpu].activeQ.prevT)*runQueue[onCpu].activeQ.nr_active;

			newEvent.task=dequeue(&runQueue[onCpu].activeQ.prioList[highestPrio].pArrHeadPtr,
                                  &runQueue[onCpu].activeQ.prioList[highestPrio].pArrTailPtr);
            runQueue[onCpu].activeQ.nr_active--;
            runQueue[onCpu].activeQ.prioList[highestPrio].numTask--;

			printf("\nTime: %d: serving task %d with bt %d on CPU %d.\n",
                simTime,newEvent.task.id,newEvent.task.bt, newEvent.task.onCpu);

			int qt; //quantum time

			if(newEvent.task.pr<120){
				qt=(140-newEvent.task.pr)*20;
			}
			else{
				qt=(140-newEvent.task.pr)*5;
			}
            //printf("simtime %d, time %d, task time %d\n", simTime,newEvent.time, newEvent.task.at);

			if(newEvent.task.bt<=qt){
				newEvent.time=simTime+newEvent.task.bt;
				newEvent.task.bt = 0;
			}else{
                newEvent.task.nsleep++;
				newEvent.task.totalST+=(simTime-newEvent.task.enteredQueue);
				newEvent.task.avgST= ((double)newEvent.task.totalST)/newEvent.task.nsleep;
				newEvent.task.avgST-=qt;
                newEvent.task.bt-=qt;
                newEvent.time=simTime+qt;
			}

            wt[onCpu]+=(simTime-newEvent.task.enteredQueue);

           // printf("total sleep %d, n sleep %d, task pr %d, avg sleep time: %lf\n"
            //, newEvent.task.totalST, newEvent.task.nsleep, newEvent.task.pr, newEvent.task.avgST);
			//printf("bt %d, , qt %d, bonus %d, dp %d \n",newEvent.task.bt, newEvent.task.avgST,qt, bonus,dp );
			//newEvent.task.ts+=min;
			return newEvent;
}

int max(int a , int b){
    if(a>b)
        return a;
    else
        return b;
}

int min(int a , int b){
    if(a<b)
        return a;
    else
        return b;
}

int getBonus(double avgST){
        //double avgST = aST;

		if(avgST>=0 && avgST<100){
            return 0;
		}
		else if (avgST>=100 && avgST<200){
			return 1;
		}
		else if (avgST>=200 && avgST<300){
			return 2;
		}
		else if (avgST>=300 && avgST<400){
			return 3;
		}
		else if (avgST>=400 && avgST<500){
			return 4;
		}
		else if (avgST>=500 && avgST<600){
			return 5;
		}
		else if (avgST>=600 && avgST<700){
			return 6;
		}else if (avgST>=700 && avgST<800){
			return 7;
		}else if (avgST>=800 && avgST<900){
			return 8;
		}else if (avgST>=900 && avgST<1000){
			return 9;
		}else {
			return 10;
		}
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

//remove!!
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
/*
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
}*/





