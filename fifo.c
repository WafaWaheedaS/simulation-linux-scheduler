#include <stdlib.h>
#include <stdio.h>

struct task{//process
	int id;//pid
	int bt;//burst time
	int at;//arrival time
	int pr;//$$ priority
	int status;//$$ 0 is new status
};
typedef struct task Task;
typedef Task *TaskPtr;

struct qnode{//a node in the run/ready queue
	Task data;//process
	struct qnode *nextPtr;
};
typedef struct qnode Qnode;
typedef Qnode *QnodePtr;

void enqueue(QnodePtr *headPtr, QnodePtr *tailPtr,Task task);
Task dequeue(QnodePtr *headPtr, QnodePtr *tailPtr);
int isEmpty(QnodePtr headPtr);

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

int isEmpty(QnodePtr headPtr){
	return headPtr == NULL;
}

///////////////////////////////////////////////

struct event{//an event event
	int type;//event type 0:arrival, 1: departure
	int time;//event time
	Task task;//the process
};
typedef struct event Event;
typedef Event *EventPtr;

struct eventQnode{//an node in the events list
	Event data;//the event
	struct eventQnode *nextPtr;
};
typedef struct eventQnode EventQnode;
typedef EventQnode *EventQnodePtr;


void enqueueevent(EventQnodePtr *headPtr, EventQnodePtr *tailPtr,Event e);
Event dequeueevent(EventQnodePtr *headPtr, EventQnodePtr *tailPtr);
int isEmptyEQ(EventQnodePtr headPtr);
void displayEvents(EventQnodePtr currentPtr, FILE *fp);

void enqueueevent(EventQnodePtr *headPtr, EventQnodePtr *tailPtr,Event se){
	EventQnodePtr newNodePtr = malloc( sizeof( EventQnode));
	if(newNodePtr !=NULL){
		newNodePtr->data = se;
		newNodePtr->nextPtr = NULL;
	}
	EventQnodePtr current = *headPtr, prev = NULL;
	while(current!=NULL && se.time>(current->data).time){//find the insert position in order of time
		prev = current;
		current = current->nextPtr;
	}
	while(current!=NULL && se.time==(current->data).time && se.type<(current->data).type){//then find the insert position in order of event's type
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

Event dequeueevent(EventQnodePtr *headPtr, EventQnodePtr *tailPtr){
	Event value;
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

int isEmptyEQ(EventQnodePtr headPtr){
	return headPtr == NULL;
}

void displayEvents(EventQnodePtr currentPtr,FILE *fp){
	if(currentPtr==NULL)
		fprintf(fp,"%s\n","The event list is empty ...");
	else{
	//	fprintf(fp,"%s\n","The event list is:");
		fprintf(fp,"%-25s%-25s%-25s%-25s%-25s%s\n","Time","Event","PID","Arrival Time","Burst Time", "Priority");
		Event tempevent;
		while(currentPtr!=NULL){
			fprintf(fp,"%010d%15s%-1d%-24s%-10d%-15s%-15d%-10s%-10d%-15s%-10d\n",(currentPtr->data).time, "",(currentPtr->data).type,"",(currentPtr->data).task.id,"",(currentPtr->data).task.at,"",(currentPtr->data).task.bt,"",(currentPtr->data).task.pr);
			currentPtr=currentPtr->nextPtr;
		}
	}
}


///////////////////////////////////////////////

int num_tasks = 1000, max_bt = 100, max_iat = 120, qt=20; //$$
const int max_pr=141; //$$
int qsize=0, finished=0;


FILE*  fp;

void setSim(){
    printf("Number of Tasks: ");
    scanf("%d",&num_tasks);
    printf("Maximum burst time: ");
    scanf("%d",&max_bt);
    printf("Maximum inter-arrival time: ");
    scanf("%d",&max_iat);
    printf("Quantum time: ");//only for rr and prr
    scanf("%d",&qt);//only for rr and prr
}

void trace(){
    fp = fopen("fifo.trace", "w");
    if(!fp)
       	perror("File opening failed");
   	fprintf(fp,"%s\n","Name: Mohammad Saleh");
   	fprintf(fp,"%s\n\n","ID: 20041111");
   	fprintf(fp, "%s", "Simulation of CPU scheduling using FIFO algorithm\n\n");
   	fprintf(fp,"%-20s%-10d%-10s%-10d%-10s%-10d%-10s%-10d\n\n","Number of tasks : ", num_tasks, "Max IAT: ", max_iat, "Max bt: ", max_bt, "QT: ", qt);
}

void main(){
    setSim();
    trace();

	QnodePtr rqheadPtr=NULL, rqtailPtr=NULL;//the run/ready queue
	EventQnodePtr eventsQheadPtr=NULL, eventsQtailPtr=NULL;//the event queue/list

	Task task;//the process structure
	Event event;//the event structure
	int prevat = 0, i;//set the previous arrival time to zero
	int allbt=0;
	int allmax_iat=0;
	srand(555);
	for(i=0;i<num_tasks;i++){//generate all arrivals and insert them in the event list
		//fill up the info of the process structure
		task.id=i;

		task.bt=rand()%max_bt+1;
		allbt+=task.bt; //&&
		task.at=rand()%max_iat+prevat;
		allmax_iat+=task.at-prevat; //&&
		prevat=task.at;
		task.pr=rand()%max_pr;//$$
		task.status=0; //$$
		//fill up the info of the event structure
		event.type=0;//event type is 0:arrival
		event.time=task.at;//event time
		event.task=task;//note that the process is encapsulated in an event structure
		enqueueevent(&eventsQheadPtr,&eventsQtailPtr,event);//insert the event in the events list
	}
//	displayEvents(eventsQheadPtr, fp);//Display the events list of all arrivals before starting the simulation


	int clock=0;//the sim clock time is currently 0
	int idle=1;//CPU is initially idle


	Event currentEvent;//to hold the current event
	int wt=0;//waiting time
	int rt=0;
	int idletime=0;//CPU idel time
	int idletime2=0;
	int previdletime=-1; //&& -1 means CPU was not idel before otherwise, it was idle since that time.

	/*
	currentEvent = dequeueevent(&eventsQheadPtr,&eventsQtailPtr);
	int starttime= currentEvent.time;
	enqueueevent(&eventsQheadPtr,&eventsQtailPtr,currentEvent);
	*/
	int starttime = (*eventsQheadPtr).data.time;

//	fprintf(fp,"\n%s\n","Simulation is started ...");
	fprintf(fp,"%-25s%-25s%-25s%-25s%-25s\n","Time","Event","RQ Size", "Task in CPU", "Number of Finished Tasks");
	while(!isEmptyEQ(eventsQheadPtr)){
		currentEvent = dequeueevent(&eventsQheadPtr,&eventsQtailPtr);//get an event from the events list
		clock=currentEvent.time;//
		if(currentEvent.type==1){//Departure logic
			idle=1;
			task = currentEvent.task;
			++finished;
			fprintf(fp,"%010d%15s%-1d%-24s%-10d%-15sP%-10d%-14s%-10d\n",clock,"",1,"",qsize,"",task.id,"",finished);
		}
		if(currentEvent.type==0){//Arrival logic
			task = currentEvent.task;
			enqueue(&rqheadPtr,&rqtailPtr,task);
			++qsize;
			fprintf(fp,"%010d%15s%-1d%-24s%-10d%-15sP%-10d%-14s%-10d\n",clock,"",0,"",qsize,"",task.id,"",finished);
		}

		if(idle==1 && !isEmpty(rqheadPtr)){//Service logic
		  idle = 0;
		  currentEvent.task=dequeue(&rqheadPtr,&rqtailPtr);
		  --qsize;
		  wt+=clock-currentEvent.task.at;
		  // $$ calculate response time
		  if(currentEvent.task.status==0){
		  	rt+=clock-currentEvent.task.at;
		  	currentEvent.task.status=1;// not new
		  }

		  currentEvent.type=1;
		  currentEvent.time=clock+currentEvent.task.bt;
		  enqueueevent(&eventsQheadPtr,&eventsQtailPtr,currentEvent);
		  task = currentEvent.task;
		  fprintf(fp,"%010d%15s%-1d%-24s%-10d%-15sP%-10d%-14s%-10d\n",clock,"",2,"",qsize,"",task.id,"",finished);
		}
		//$$ calculate idle time
		if(previdletime>=0){
		 	idletime+=clock-previdletime;
		  	previdletime=-1;
		 }
		//$$ check if it is a start of an idel time
		if(idle==1)
			previdletime=clock;

		if(idle==1 && !isEmptyEQ(eventsQheadPtr))
			idletime2+=(*eventsQheadPtr).data.time-clock;
	}

//	fprintf(fp,"\n%s\n","Simulation has finished.");
	fprintf(fp,"\nPerformance measures\n");

	fprintf(fp,"%s%f\n","Average delay time     :",(double)wt/num_tasks);
	fprintf(fp,"%s%f\n","Average response time   :",(double)rt/num_tasks); //$$
	fprintf(fp,"%s%f\n","Average turarround time :",(double)(allbt+wt)/num_tasks); //$$
	fprintf(fp,"%s%f\n","CPU utilization         :", (double)((clock-starttime)-idletime)/(clock-starttime)); //$$
//	fprintf(fp,"%s%f\n","CPU utilization2        :", (double)((clock-starttime)-idletime2)/(clock-starttime)); //$$
	fprintf(fp,"%s%f\n","Average burst time      :",(double)allbt/num_tasks); //$$
	fprintf(fp,"%s%f\n","Average max_iat         :",(double)allmax_iat/num_tasks); //$$

	fclose(fp);
}













