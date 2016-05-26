#include <stdlib.h>
#include <stdio.h>
// #include <gsl/gsl_rng.h>
// #include <gsl/gsl_randist.h>

struct task{
	int id;
	int bt;     //burst time
	int at;     //arrival time
	int pr;     //priority
	int status; // 0 is new task, 1 is old task
	int enteredQueue;
	int onCpu;  //which cpu it is executing on
	double avgST; //avg sleep time
	int totalST; //total sleep time
	int nsleep;  // track no of times sleep occurs
	int prevRT;    //for RT tasks keeps track of Response time
	int totalJitter;    //total jitter for RT task
	double jitter;  //jitter for RT tasks
	int type; //0: real time RT, 1: none real time NRT
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
    int prio;   //priority
    int numTask;//number of tasks in list
    QnodePtr pArrHeadPtr;
    QnodePtr pArrTailPtr;
};
typedef struct prioLinkList PrioLinkList;


struct prioArray{
    int nr_active;  //number of tasks
    int prevT;      //keeps track of how long the array length has been the same
    int avgQ;       //average Queue length before division
    PrioLinkList prioList[140]; //array of task prioritys
};
typedef struct prioArray PrioArray;

struct runQueue{
    int nr_active;          // number of tasks on queue
    PrioArray  activeQ;     //the active priorityarray
    PrioArray  expiredQ;    //the expired priority array
};
typedef struct runQueue RunQueue;

//definig all functions in simulation
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
void pullTask(int busy, int i);
void loadBalancing();

const int MAXTASKS = 1000;
const int MINBURSTTIME = 100;
const int MAXBURSTTIME = 1000;
const int IAT = 90;

int simTime;
int numCpus;
int *isIdle;    //CPUs, 1 is Idle, 0 is busy
RunQueue *runQueue; //run queue for each cpu
int *wt;            //cpu wait time
int *idletime;
int *previdletime;
int *taskPerCpu;
int *allbtPerCPU;
int *allatPerCPU;
int *rt;            //response time
int *minBT;
int *maxBT;
int *minIAT;
int *maxIAT;
int avgJitter=0;


FILE *fp;            //for saving data into trace

void main(){


    int ratioRT;
    int ratioNRT;
    int i,j;

    //initialize random number generator
    // const gsl_rng_type * T;
    // gsl_rng * r;

    // select random number generator
    // r = gsl_rng_alloc (gsl_rng_mt19937);

    //get inputs from user

    char *tfname = "trace.txt";
	fp = fopen(tfname, "w");
    if(fp == NULL)
       	perror("File opening failed");

   	fprintf(fp,"%s\n","Name: Wafa Waheeda Syed");
   	fprintf(fp,"%s\n","Name: Haya Al-Thani");
   	fprintf(fp,"%s\n\n","ID: ww1402824");
   	fprintf(fp,"%s\n\n","ID: 200662244");

   	fprintf(fp, "Linux Scheduler Simulator (1000 tasks)\n\n");
   	printf("\t Linux Scheduler Simulator (1000 tasks)\n\n");
    printf("\t Please enter the number of CPUs to simulate: ");
	scanf("%d", &numCpus);	
	fprintf(fp, "Number of CPUs entered by user to simulate: %d \n", numCpus);

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
    allatPerCPU = malloc(sizeof(int)*numCpus);
    if(allatPerCPU==NULL){
        printf("memory allocation error");
    }
    rt = malloc(sizeof(int)*numCpus);
    if(rt==NULL){
        printf("memory allocation error");
    }
    minBT = malloc(sizeof(int)*numCpus);
    if(minBT==NULL){
        printf("memory allocation error");
    }
    maxBT = malloc(sizeof(int)*numCpus);
    if(maxBT==NULL){
        printf("memory allocation error");
    }
    minIAT = malloc(sizeof(int)*numCpus);
    if(minIAT==NULL){
        printf("memory allocation error");
    }
    maxIAT = malloc(sizeof(int)*numCpus);
    if(maxIAT==NULL){
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
        allatPerCPU[i]=0;
        rt[i]=0;
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
	fprintf(fp, "the percentage of RT tasks to simulate: %d \n", ratioRT);
    fprintf(fp, "the percentage of NRT tasks to simulate: %d \n", ratioNRT);

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
	int allIAT=0; //initial arrival time
	int allbt=0; //total burst time
    int allminBT=0;
    int allmaxBT=0;
    int allminIAT=0;
    int allmaxIAT=0;
    int rnd; //random number based on the Uniform Distribution

	for(i=0;i<MAXTASKS;i++){

		task.id=i;
		task.at=rand()%IAT+prevAt;
		// task.at=gsl_rng_uniform(r)*IAT+prevAt;
		prevAt=task.at;
		task.bt=rand()%MAXBURSTTIME+MINBURSTTIME;
		// task.bt=gsl_rng_uniform(r)*MAXBURSTTIME+MINBURSTTIME;
		printf("AT is %d and BT is %d\n", task.at, task.bt);
		task.status=0;
		task.totalST=0;                    //new taskstatus
		task.nsleep =0;
		task.avgST = 0;   // assigning avg sleep time
        allbt+=task.bt; //&&
        allIAT+=task.at;
        task.prevRT=0;
        task.jitter=0;
        if(i==0){
            allminBT=task.bt;
            allmaxBT=task.bt;
            allminIAT=task.at;
            allmaxIAT=task.at;
        }
        allminBT=min(allminBT, task.bt);
        allmaxBT=max(allmaxBT, task.bt);
        allminIAT=min(allminIAT, task.at);
        allmaxIAT=max(allmaxIAT, task.at);

        type=rand()%2; 
		// type=gsl_rng_uniform(r)*2;                  //either 0 for RT, or 1 for NRT
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
	int starttime=(*eventsQheadPtr).data.time;
	SchedEvent currentEvent;

	printf("\nTime: %d: Simulation is started ...\n",simTime);

    i=0;        //start with fisrt CPU and divide tasks iteratively
    int highestPrio;
    int initType;
    PrioArray tempArray;
    int doLB=0;     //1: preform load balancing, 0: don't do load balancing
    int balanceEvery = 100; //preform load balancing after every 50 tasks
    int k=0;
    int maxQ=0;

    fprintf(fp,"%-25s%-25s%-25s \n","Time","Task ID","CPU Number");

	while(!isEmptyEQ(eventsQheadPtr)){

        k++;
		currentEvent = dequeueevent(&eventsQheadPtr,&eventsQtailPtr);
		simTime=currentEvent.time;
        initType=currentEvent.type;

		if(currentEvent.type==1){   //Departure

			isIdle[currentEvent.task.onCpu]=1;

			if(currentEvent.task.bt != 0){
                int bonus;
                int dp; //dynamic prio
                // fprintf(fp,"%-25s%-25s%-25s \n","Time","Task ID","CPU Number");
	
				printf("\nTime: %d: enqueu task %d in expired Q of CPU %d.\n", simTime, currentEvent.task.id, currentEvent.task.onCpu);
				fprintf(fp,"%-25d%-25d%-25d \n", simTime, currentEvent.task.id, currentEvent.task.onCpu);
		
				if(currentEvent.task.type==1){  //change priorty of NRT tasks
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
                if(currentEvent.task.type==0){
                    avgJitter+=currentEvent.task.jitter;
                }
                // fprintf(fp,"%-25s%-25s%-25s \n","Time","Task ID","CPU Number");
                printf("\nTime: %d: task %d has finished on CPU %d.\n", simTime, currentEvent.task.id, currentEvent.task.onCpu);
            	
            	fprintf(fp,"%-25d%-25d%-25d \n", simTime, currentEvent.task.id, currentEvent.task.onCpu);
		
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
            if(taskPerCpu[i]==0){
                minBT[i]=currentEvent.task.bt;
                maxBT[i]=currentEvent.task.bt;
                minIAT[i]=currentEvent.task.at;
                maxIAT[i]=currentEvent.task.at;
            }

            taskPerCpu[i]++;
            allbtPerCPU[i]+=currentEvent.task.bt;
            allatPerCPU[i]+=currentEvent.task.at;
			currentEvent.task.onCpu=i;
            currentEvent.task.enteredQueue=simTime;

			enqueue(&runQueue[i].activeQ.prioList[currentEvent.task.pr].pArrHeadPtr,
                    &runQueue[i].activeQ.prioList[currentEvent.task.pr].pArrTailPtr, currentEvent.task);

            runQueue[i].nr_active++;
            runQueue[i].activeQ.nr_active++;
            runQueue[i].activeQ.prioList[currentEvent.task.pr].numTask++;
            runQueue[i].activeQ.prevT=simTime;

		}//end Arrival

		if(isIdle[i]==1 && !isEmptyActiveQ(runQueue[i])){      //service tasks
			currentEvent = serviceTask(i, currentEvent);
			enqueueevent(&eventsQheadPtr,&eventsQtailPtr,currentEvent);
		}//end Service

        if(runQueue[i].activeQ.nr_active>maxQ){
            maxQ=runQueue[i].activeQ.nr_active;
        }
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
		if(doLB ==1 && k%balanceEvery==0){
            loadBalancing();
		}

	}//end while

	printf("\nSimulation has finished.\n");

    double avgWT=0;
    double avgRT=0;
    double avgCPU=0;
    double avgTP=0;
    double avgTT=0;
    double avgBT=0;
    double avgIAT=0;
    double avgQL=0;


    printf("\nSimulation statistics per CPU:\n");
    fprintf(fp, "\nSimulation statistics per CPU:\n");
	for(i=0;i<numCpus;i++){
        printf("CPU %d:\n",i);
        printf("\tNumber of Tasks served = %d\n", taskPerCpu[i]);
        printf("\tAverage Delay Time = %f\n", (double)wt[i]/taskPerCpu[i]);
        printf("\tAverage Response Time = %f\n", (double)rt[i]/taskPerCpu[i]);
        printf("\tCPU Utilization = %f\n", (double)((simTime-starttime)-idletime[i])/ (simTime-starttime));
        printf("\tThroughput per ms = %f\n", (double)MAXTASKS/simTime);
        printf("\tAverage Turnaround Time = %f\n", (double)(allbtPerCPU[i]+wt[i])/taskPerCpu[i]);
        printf("\tAverage Burst Time = %f\n", (double)allbtPerCPU[i]/taskPerCpu[i]);
        printf("\tMinimum Burst Time = %d\n", minBT[i]);
        printf("\tMaximum Burst Time = %d\n", maxBT[i]);
        printf("\tAverage IAT = %f\n", (double)allatPerCPU[i]/taskPerCpu[i]);
        printf("\tMinimum IAT = %d\n", minIAT[i]);
        printf("\tMaximum IAT = %d\n", maxIAT[i]);
        printf("\tAverage Queue Length = %f\n", (double)runQueue[i].activeQ.avgQ/simTime);
		
		fprintf(fp, "CPU %d:\n",i);
        fprintf(fp, "\tNumber of Tasks served = %d\n", taskPerCpu[i]);
        fprintf(fp, "\tAverage Delay Time = %f\n", (double)wt[i]/taskPerCpu[i]);
        fprintf(fp, "\tAverage Response Time = %f\n", (double)rt[i]/taskPerCpu[i]);
        fprintf(fp, "\tCPU Utilization = %f\n", (double)((simTime-starttime)-idletime[i])/ (simTime-starttime));
        fprintf(fp, "\tThroughput per ms = %f\n", (double)MAXTASKS/simTime);
        fprintf(fp, "\tAverage Turnaround Time = %f\n", (double)(allbtPerCPU[i]+wt[i])/taskPerCpu[i]);
        fprintf(fp, "\tAverage Burst Time = %f\n", (double)allbtPerCPU[i]/taskPerCpu[i]);
        fprintf(fp, "\tMinimum Burst Time = %d\n", minBT[i]);
        fprintf(fp, "\tMaximum Burst Time = %d\n", maxBT[i]);
        fprintf(fp, "\tAverage IAT = %f\n", (double)allatPerCPU[i]/taskPerCpu[i]);
        fprintf(fp, "\tMinimum IAT = %d\n", minIAT[i]);
        fprintf(fp, "\tMaximum IAT = %d\n", maxIAT[i]);
        fprintf(fp, "\tAverage Queue Length = %f\n", (double)runQueue[i].activeQ.avgQ/simTime);



        avgWT+=(double)wt[i]/taskPerCpu[i];
        avgRT+=(double)rt[i]/taskPerCpu[i];
        avgCPU+=(double)((simTime-starttime)-idletime[i])/ (simTime-starttime);
        avgTP+=(double)MAXTASKS/simTime;
        avgTT=(double)(allbtPerCPU[i]+wt[i])/taskPerCpu[i];
        avgBT+=(double)allbtPerCPU[i]/taskPerCpu[i];
        avgIAT+=(double)allatPerCPU[i]/taskPerCpu[i];
        avgQL+=(double)runQueue[i].activeQ.avgQ/simTime;
	}

	printf("\nOverall Simulation Statistics (all CPUs):\n");
	printf("\tMaximum Tasks in the system = %d\n", MAXTASKS);
    printf("\tAverage Delay Time = %f\n", avgWT/numCpus);
    printf("\tAverage Response Time = %f\n", avgRT/numCpus);
    printf("\tAverage CPU Utilization = %f\n", avgCPU/numCpus);
    printf("\tAverage Throughput per ms = %f\n", avgTP/numCpus);
    printf("\tAverage Turnaround Time = %f\n", avgTT/numCpus);
    printf("\tAverage Burst Time = %f\n", avgBT/numCpus);
    printf("\tMinimum Burst Time = %d\n", allminBT);
    printf("\tMaximum Burst Time = %d\n", allmaxBT);
    printf("\tAverage IAT = %f\n", avgIAT/numCpus);
    printf("\tMinimum IAT = %d\n", allminIAT);
    printf("\tMaximum IAT = %d\n", allmaxIAT);
    printf("\tAverage Queue Length = %f\n", avgQL/numCpus);
    printf("\tMaximum Queue Length = %d\n", maxQ);


    fprintf(fp, "\nOverall Simulation Statistics (all CPUs):\n");
	fprintf(fp, "\tMaximum Tasks in the system = %d\n", MAXTASKS);
    fprintf(fp, "\tAverage Delay Time = %f\n", avgWT/numCpus);
    fprintf(fp, "\tAverage Response Time = %f\n", avgRT/numCpus);
    fprintf(fp, "\tAverage CPU Utilization = %f\n", avgCPU/numCpus);
    fprintf(fp, "\tAverage Throughput per ms = %f\n", avgTP/numCpus);
    fprintf(fp, "\tAverage Turnaround Time = %f\n", avgTT/numCpus);
    fprintf(fp, "\tAverage Burst Time = %f\n", avgBT/numCpus);
    fprintf(fp, "\tMinimum Burst Time = %d\n", allminBT);
    fprintf(fp, "\tMaximum Burst Time = %d\n", allmaxBT);
    fprintf(fp, "\tAverage IAT = %f\n", avgIAT/numCpus);
    fprintf(fp, "\tMinimum IAT = %d\n", allminIAT);
    fprintf(fp, "\tMaximum IAT = %d\n", allmaxIAT);
    fprintf(fp, "\tAverage Queue Length = %f\n", avgQL/numCpus);
    fprintf(fp, "\tMaximum Queue Length = %d\n", maxQ);

    numRT=(ratioRT/100.0)*MAXTASKS;
    printf("\tJitter for Real Time Tasks = %f\n", (double)avgJitter/numRT);
    fprintf(fp, "\tJitter for Real Time Tasks = %f\n", (double)avgJitter/numRT);
}//end main

SchedEvent serviceTask(int onCpu, SchedEvent currentEvent){

			SchedEvent newEvent;
            newEvent = currentEvent;
			isIdle[onCpu] = 0;
			newEvent.type=1; //departure
			int highestPrio = getHighestPriority(onCpu);
            runQueue[onCpu].activeQ.avgQ+=(simTime-runQueue[onCpu].activeQ.prevT)*runQueue[onCpu].activeQ.nr_active;

			newEvent.task=dequeue(&runQueue[onCpu].activeQ.prioList[highestPrio].pArrHeadPtr,
                                  &runQueue[onCpu].activeQ.prioList[highestPrio].pArrTailPtr);

            runQueue[onCpu].activeQ.prevT=simTime;
            runQueue[onCpu].nr_active--;
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

            int currRT; //current response time

            if(newEvent.task.status==0){
                rt[onCpu]+=simTime-newEvent.task.at;
                minBT[onCpu]=min(minBT[onCpu], newEvent.task.bt);
                maxBT[onCpu]=max(maxBT[onCpu], newEvent.task.bt);
                minIAT[onCpu]=min(minIAT[onCpu], newEvent.task.at);
                maxIAT[onCpu]=max(maxIAT[onCpu], newEvent.task.at);
                if(newEvent.task.type==0){ //RT task
                   newEvent.task.prevRT=simTime-newEvent.task.enteredQueue;
                }
                newEvent.task.status=1;// not new
            }
            else{
                if(newEvent.task.type==0){ //RT task
                currRT=simTime-newEvent.task.enteredQueue;
                newEvent.task.totalJitter+=abs(currRT-newEvent.task.prevRT);
                newEvent.task.jitter=(double)newEvent.task.totalJitter/newEvent.task.nsleep;
                newEvent.task.prevRT=currRT;
                //printf("nsleep %d, total jitter %d, jitter %f, currRT %d, prevRT %d",
               // newEvent.task.nsleep,   newEvent.task.totalJitter,newEvent.task.jitter, currRT, newEvent.task.prevRT );
                }
            }
            wt[onCpu]+=(simTime-newEvent.task.enteredQueue);
			return newEvent;
}

void loadBalancing(){

    printf("\nChecking to see if load balancing is required...\n");

    int busy = findBusiestQueue();
    int i = checkRatio(busy);
    int prio;

    while (i != -1){
        printf("\nLoad balancing from CPU %d to CPU %d...\n", busy, i);
        //prio = getHighestPriorityEA(busy);
        pullTask(busy, i);
        i=checkRatio(busy);
    }

}
void pullTask(int busy, int i){
    int prio;
    Task task;

    prio = getHighestPriorityEA(busy);

    if(prio != -1){
        task=dequeue(&runQueue[busy].expiredQ.prioList[prio].pArrHeadPtr,
                     &runQueue[busy].expiredQ.prioList[prio].pArrTailPtr);

        runQueue[busy].nr_active--;
        runQueue[busy].expiredQ.nr_active--;
        runQueue[busy].expiredQ.prioList[prio].numTask--;
        runQueue[busy].expiredQ.prevT=simTime;
    }
    else{
        prio = getHighestPriority(busy);
        task=dequeue(&runQueue[busy].activeQ.prioList[prio].pArrHeadPtr,
                 &runQueue[busy].activeQ.prioList[prio].pArrTailPtr);
        runQueue[busy].nr_active--;
        runQueue[busy].activeQ.nr_active--;
        runQueue[busy].activeQ.prioList[prio].numTask--;
        runQueue[busy].activeQ.prevT=simTime;
    }

    enqueue(&runQueue[i].activeQ.prioList[prio].pArrHeadPtr,
            &runQueue[i].activeQ.prioList[prio].pArrTailPtr, task);
    runQueue[i].nr_active++;
    runQueue[i].activeQ.nr_active++;
    runQueue[i].activeQ.prioList[prio].numTask++;
    runQueue[i].activeQ.prevT=simTime;

    printf("Pulled task id %d of priority %d from expired queue of CPU %d to CPU %d.\n",
            task.id, prio, busy, i);

}
int checkRatio(int busy){
    int i=0;
    double ratio;
    //if the busy Q has 25% more processs than the other task preform load balancing
    for(i=0;i<numCpus;i++){
        ratio=(double) (runQueue[busy].nr_active-runQueue[i].nr_active)/runQueue[busy].nr_active;
        if( ratio >= 0.25 && busy!=i ){
            return i;
        }
    }
    return -1;
}

int findBusiestQueue(){
    int max=0;
    int i=0;

    for(i=0;i<numCpus;i++){
        if(runQueue[i].nr_active>runQueue[max].nr_active){
            max=i;
        }
    }
    return max;
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

int getHighestPriorityEA(int onCpu){

    int i;
    int prio=-1;

    for(i=0;i<140;i++){
        if(runQueue[onCpu].expiredQ.prioList[i].numTask!=0){
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






