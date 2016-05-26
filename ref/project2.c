#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>



struct task{
	int id;    //id
	int bt;    //burst time
	int at;	   //arrival time
	int pr;    //priority
	int ts;    //time service
	//int cs;    //completiontime
	int tat;   //turnAroundTime
	int type;  //0: real time 1: none real time
};

typedef struct task Task;
typedef Task *TaskPtr;

struct qnode{
	Task data;
	struct qnode *nextPtr;
	int qLength;
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

void enqueue(QnodePtr *headPtr, QnodePtr *tailPtr, Task task);
void enqueuepr(QnodePtr *headPtr, QnodePtr *tailPtr, Task task);
void enqueueevent(EventQnodePtr *headPtr, EventQnodePtr *tailPtr, SchedEvent e);
Task dequeue(QnodePtr *headPtr, QnodePtr *tailPtr);
SchedEvent dequeueevent(EventQnodePtr *headPtr, EventQnodePtr *tailPtr);
int isEmpty(QnodePtr headPtr);
int isEmptyEQ(EventQnodePtr headPtr);
void displayEvents(EventQnodePtr currentPtr);
void over_all_system_stats(EventQnodePtr currentPtr);
int total_no_task_in_system(QnodePtr *aqheadPtr, QnodePtr *eqheadPtr, int cpu[]);
int find_list_len(QnodePtr *headPtr);

int NO_OF_CPU = 4;
int MAXTASKS = 400;
int MAXBURSTTIME = 400;
int IAT = 70; //time bn the  processes in the event queue
int QTRT = 30;
int QTNRT = 10;


int RT_NO = 0;
int NRT_NO = 0;

int MAX_Q_LENGTH = 0;
int NO_OF_RUNS = 1;

int avg = 0;
int total_Max_Q_Length = 0;
int max_task_in_system = 0;
int task_in_system = 0;
double MAX_IAT = 0;         //Maximum Burst Time assigned so task
double MAX_BT = 0;			//maximum IAT assigned to task


FILE *fp;            //This is the Trace File saver
FILE *IAT_NRTvsRT, *fp_waiting, *fp_max;


double *avrWT, *avrWTRT, *avrWTNRT;        //Average Waiting time , Average Waiting time for Real TIme ,Average Waiting time for Non Real Time
double *avrRST, *avrRSTRT, *avrRSTNRT;      //Average Response Time


int *wt, *wtRT, *wtNRT;          //waiting time , waiting time of real time , waiting time of Non Real Time
int *fTasks, *fTasksRT, *fTasksNRT;   //Finished Task  , Finished Task Real Time , Finished Task Non Real Time

double *avrTAT;   //Average turn arround time 

#pragma warning (disable : 4996)

//Over All System Performance

double sys_avrWT, sys_avrWTRT, sys_avrWTNRT;        //Average Waiting time , Average Waiting time for Real TIme ,Average Waiting time for Non Real Time
int sys_wt, sys_wtRT, sys_wtNRT;          //waiting time , waiting time of real time , waiting time of Non Real Time
int sys_fTasks, sys_fTasksRT, sys_fTasksNRT;   //Finished Task  , Finished Task Real Time , Finished Task Non Real Time
int qSize = 0;
double total_WQsize = 0;
double total_IAT_RT = 0;
double total_IAT_NRT = 0;
double total_IAT = 0;
double total_sys_WTime = 0;
double responseTime=0;
double total_BT = 0;
double total_QLength=0;
int no_of_q_count=0;
double total_turn_time;
double over_task_in_system = 0;

void initializeAllPerformaceParameters(int size){

	avrWT = malloc(size * sizeof(double));
	avrWTRT = malloc(size * sizeof(double));
	avrWTNRT = malloc(size * sizeof(double));
	avrTAT = malloc(size * sizeof(double));

	wt = malloc(size * sizeof(int));
	wtRT = malloc(size * sizeof(int));
	wtNRT = malloc(size * sizeof(int));
	fTasks = malloc(size * sizeof(int));
	fTasksRT = malloc(size * sizeof(int));
	fTasksNRT = malloc(size * sizeof(int));

	int i;
	for (i = 0; i < size; i++)
		avrWT[i] = avrWTRT[i] = avrWTNRT[i] = avrTAT[i] = wt[i] = wtRT[i] = wtNRT[i] = fTasks[i] = fTasksRT[i] = fTasksNRT[i] = 0;



	sys_avrWT = sys_avrWTRT = sys_avrWTNRT = sys_wt = sys_wtRT = sys_wtNRT = sys_fTasks = sys_fTasksRT = sys_fTasksNRT = 0;
	avg = total_Max_Q_Length = max_task_in_system = task_in_system = 0;
	MAX_IAT = MAX_BT = RT_NO = NRT_NO = no_of_q_count=0;
	total_IAT_RT = total_IAT_NRT = total_IAT = total_sys_WTime = responseTime = total_BT = total_QLength = total_turn_time ,over_task_in_system = 0;

	//sys_fTasks, sys_fTasksRT, sys_fTasksNRT = 0;

}
void main(){

	int runs = 0;
	int useChoice = 0;

	char *filename = "home/wafa/simulation-linux-scheduler/tracedFile.txt";
	
	char *NRTvsRT = "home/wafa/simulation-linux-scheduler/output_NRTvsRT.txt";
	char *avgWTT = "home/wafa/simulation-linux-scheduler/output_avgWTT.txt";
	char *AVQSize = "home/wafa/simulation-linux-scheduler/output_AVQSize.txt";
	
	fp = fopen(filename, "w");
	IAT_NRTvsRT = fopen(NRTvsRT, "w");
	fp_waiting = fopen(avgWTT, "w");
	fp_max = fopen(AVQSize, "w");

	
	// fprintf(fp_waiting, "#'total_sys_WTime\tAverage Q_WaitingTime\tsys_avrWTRT\tsys_avrWTNRT\n" );


	int errno = 10;
	if (fp == NULL) {
		printf("File not created, errno = %d\n", errno);
	}

	int Tcounter = 1;
	
	
	while (Tcounter < 10){
		

		if (useChoice != 2){
			printf("\t   What Do You like to Do\t \n");
			printf("\t---------------------------\n");
			printf("\t| 1. Run Simulation with New Settings \n\t| 2. Run With Exisiting default Setup\n\t| 3. Exit\n ");
			printf("\t---------------------------\n");
			scanf_s("%d", &useChoice);
		}

		if (useChoice == 3)
			exit(0);

		else if (useChoice == 1){
			printf("\tEnter the Parametes separated by space in the same order\n\n");
			printf("\t~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
			printf("No CPU | No Tasks | Max Burst Time | IAT | QTRT | QTNRT | RT | NRT \n\n");
			printf("-----------------------------------------------------------------------\n\n");
			scanf_s("%d,%d,%d,%d,%d ", &NO_OF_CPU, &MAXTASKS, &MAXBURSTTIME, &IAT, &QTRT, &QTNRT, &RT_NO, &NRT_NO);
		}


		initializeAllPerformaceParameters(NO_OF_CPU);

		int size = NO_OF_CPU;


		int MAXQUEUES = 2 * NO_OF_CPU;

		EventQnodePtr *eventsQheadPtr = NULL, eventsQtailPtr = NULL;
		EventQnodePtr *fEQheadPtr = malloc(MAXQUEUES * sizeof(int)), *fEQtailPtr = malloc(MAXQUEUES * sizeof(int));
		EventQnodePtr *sys_fEQheadPtr = NULL, *sys_fEQtailPtr = NULL;

		Task *tasks = malloc(MAXTASKS * sizeof(int));
		SchedEvent *sysevents = malloc(MAXTASKS * sizeof(int));

		int i = 0;
		int prevAt = 0;
		Task task;
		SchedEvent sysevent;
		int temp_prev_AT;



		int percentage_RT = (((double)Tcounter / 10)*MAXTASKS);
		int percentage_NRT = MAXTASKS - percentage_RT;
		printf(" RT : %d  :: NRT%d \n", percentage_RT, percentage_NRT);
		Tcounter++;


		for (i = 0; i < MAXTASKS; i++){
			
			//srand(time());     // this produces a negetive number , needs little bit of fixing
			
			task.id = i;
			task.at = rand() % IAT + prevAt;
			temp_prev_AT = prevAt;
			prevAt = task.at;
			task.bt = rand() % MAXBURSTTIME + 1;
			task.ts = 0;

			if (RT_NO < percentage_RT && NRT_NO < percentage_NRT)
				task.type = rand() % 2;
			
			else if (RT_NO == percentage_RT)
				task.type = 1;
			else
				task.type = 0;

			if (task.type == 0){
				task.pr = rand() % (100);//RT
				total_IAT_RT += abs(task.at - temp_prev_AT);
				RT_NO++;

			}
			else{
				task.pr = rand() % 41 + 100; //NRT
				total_IAT_NRT += abs(task.at - temp_prev_AT);
				NRT_NO++;
			}

			sysevent.event = 0;
			sysevent.time = task.at;
			sysevent.task = task;

			//All Averages can be calculated for this two Burst Time and IAT can be calculated
			if (MAX_BT < task.bt)
				MAX_BT = task.bt;

			if (abs(task.at - temp_prev_AT) > MAX_IAT)
				MAX_IAT = abs(task.at - temp_prev_AT);

			total_IAT += abs(task.at - temp_prev_AT);
			total_BT += task.bt;

			enqueueevent(&eventsQheadPtr, &eventsQtailPtr, sysevent);
		}

		int simTime = 0;
		
		SchedEvent currentEvent;
		QnodePtr *aqheadPtr = malloc(NO_OF_CPU *sizeof(int)), *aqtailPtr = malloc(NO_OF_CPU *sizeof(int));
		QnodePtr *eqheadPtr = malloc(NO_OF_CPU *sizeof(int)), *eqtailPtr = malloc(NO_OF_CPU *sizeof(int));

	

		//The Size counters
		//Active_queue_size = malloc(NO_OF_CPU *sizeof(int)), expired_queue_size = malloc(NO_OF_CPU *sizeof(int));

		void *tqPtr;
		bool *cpu_idle = malloc(NO_OF_CPU *sizeof(int));


		//initialize all the queues to be empty and the cpus to be idle 
		for (i = 0; i < NO_OF_CPU; i++)
		{
			aqheadPtr[i] = NULL;
			aqtailPtr[i] = NULL;

			eqheadPtr[i] = NULL;
			eqtailPtr[i] = NULL;

			fEQheadPtr[i] = NULL;
			fEQtailPtr[i] = NULL;

			fEQheadPtr[i] = NULL;
			fEQtailPtr[i] = NULL;

			//active_queue_size[i] = 0;
			//expired_queue_size[i] = 0;

			cpu_idle[i] = true;				//Initialize all CPU to be Iddle

		}


		if (Tcounter % 100 == 0.0 || Tcounter == 1.0)
			printf("\n\nTime: %d: Simulation is started ...\n", simTime);
	
		char *event_type = "", *state_change_descr = "", *stat_of_queue = "", *stat_of_process = "", *stat_of_CPU = "";
	

		fprintf(fp, "\n Simulation Parameters %d\n", Tcounter);
		fprintf(fp, "No CPU = %d, No Tasks= %d, Max Burst Time= %d, IAT= %d, QTRT = %d , QTNRT=%d", NO_OF_CPU, MAXTASKS, MAXBURSTTIME, IAT, QTRT, QTNRT);
		fprintf(fp, "\n______________________________________________________________________________\n");

		fprintf(fp, "Legend\n ->Status {S-Switichg :DA-Dequeue_Active : EA:Enqueue_Active : DE-Dequeue_Expired : EE:Enqueue_Expired}\nCPU {I=Idle : B = Busy }");
		fprintf(fp, "\nTime | Event-Type | Q# : Q-Size :State | CPU# : Status : Task# : TS | #FinishedTask\n");
		fprintf(fp, "\n______________________________________________________________________________\n");


		//printf("\n Simulation Parameters %d\n", Tcounter);
		//printf("REAL TIME = %d, NoN REAL TIME= %d", RT_NO , NRT_NO);

		int current_cpu = 0;
		//sys_fTasks = 0;
		int serviceTime = 0;

		int prev_Time = 0;
		int currentTasksInSystem = 0;

		QnodePtr currentQlengthheadPtr = NULL;

		while (!isEmptyEQ(eventsQheadPtr)){

			currentEvent = dequeueevent(&eventsQheadPtr, &eventsQtailPtr);

			simTime = currentEvent.time;
			serviceTime = currentEvent.task.ts;
			
			if (currentEvent.event == 1){      //the event is a departure event 
				cpu_idle[currentEvent.processor] = true;     //Because its a departure that means the CPU should turn to be free
				task_in_system--;

				stat_of_CPU = "Idle";             //Idle
				event_type = "D";			  //Departure


				if (currentEvent.task.ts < currentEvent.task.bt){

					if (currentEvent.task.type == 1)//NRT
						currentEvent.task.pr += 5;

					stat_of_queue = "E-E";
					enqueuepr(&eqheadPtr[currentEvent.processor], &eqtailPtr[currentEvent.processor], currentEvent.task);
				}
				else{


					sys_fTasks++;
					stat_of_queue = "E-F";
					
					currentEvent.task.tat = simTime - currentEvent.task.at;
					total_turn_time += currentEvent.task.tat;

					//enqueueevent(&fEQheadPtr[currentEvent.processor], &fEQtailPtr[currentEvent.processor], currentEvent);
					enqueueevent(&sys_fEQheadPtr, &sys_fEQtailPtr, currentEvent);
				}
			
				if (isEmpty(aqheadPtr[currentEvent.processor]) && !isEmpty(eqheadPtr[currentEvent.processor])){
					
					qSize = find_list_len(aqheadPtr[currentEvent.processor]);
					fprintf(fp, "%d\t%s\t%d:%d:%s\t\t%d:%s:%d:%d\t%d\n", simTime, event_type, currentEvent.processor * 2,
						qSize, stat_of_queue, currentEvent.processor, stat_of_CPU, currentEvent.task.id, serviceTime, sys_fTasks);

					stat_of_queue = "S";   //Switching
			
					tqPtr = aqheadPtr[currentEvent.processor];
					aqheadPtr[currentEvent.processor] = eqheadPtr[currentEvent.processor];
					eqheadPtr[currentEvent.processor] = tqPtr;

					tqPtr = aqtailPtr[currentEvent.processor];
					aqtailPtr[currentEvent.processor] = eqtailPtr[currentEvent.processor];
					eqtailPtr[currentEvent.processor] = tqPtr;
				
					
					qSize = find_list_len(aqheadPtr[currentEvent.processor]);
					fprintf(fp, "%d\t%s\t%d:%d:%s\t\t%d:%s:%d:%d\t%d\n", simTime, event_type, currentEvent.processor * 2,
						qSize, stat_of_queue, currentEvent.processor, stat_of_CPU, currentEvent.task.id, serviceTime, sys_fTasks);
				
				
				}

			
			}

			if (currentEvent.event == 0){
				event_type = "A";
				currentEvent.processor = current_cpu++%NO_OF_CPU;
				stat_of_queue = "E-A";
				enqueuepr(&aqheadPtr[currentEvent.processor], &aqtailPtr[currentEvent.processor], currentEvent.task);
				
			}

			if (cpu_idle[currentEvent.processor] == true && !isEmpty(aqheadPtr[currentEvent.processor])){

				task_in_system++;
				stat_of_CPU = "Busy";       //Busy
			
				cpu_idle[currentEvent.processor] = false;
				currentEvent.event = 1;

				stat_of_queue = "D-A";
				currentEvent.task = dequeue(&aqheadPtr[currentEvent.processor], &aqtailPtr[currentEvent.processor]);

				if (currentEvent.task.ts == 0)
					responseTime += simTime-currentEvent.task.at;

				int qt;

				if (currentEvent.task.type == 0)
					qt = QTRT;
				else
					qt = QTNRT;

				int min = currentEvent.task.bt - currentEvent.task.ts;

				if (min > qt)
					min = qt;

				currentEvent.time = simTime + min;
				currentEvent.task.ts += min;


				enqueueevent(&eventsQheadPtr, &eventsQtailPtr, currentEvent);

				
			}
			
			 currentTasksInSystem = total_no_task_in_system(aqheadPtr, eqheadPtr, cpu_idle);
			 over_task_in_system += currentTasksInSystem;

			 if (currentTasksInSystem > max_task_in_system)
				 max_task_in_system = currentTasksInSystem;


			 qSize = find_list_len(aqheadPtr[currentEvent.processor]);
			 total_QLength += qSize;
			 no_of_q_count++;


			 total_WQsize += (simTime - prev_Time)* qSize;         //Average Queue Waiting 
			 prev_Time = simTime;

			 if (qSize > MAX_Q_LENGTH)
				 MAX_Q_LENGTH = qSize;

			fprintf(fp,"%d\t%s\t%d:%d:%s\t\t%d:%s:%d:%d\t%d\n", simTime, event_type, currentEvent.processor*2,
			qSize, stat_of_queue, currentEvent.processor, stat_of_CPU, currentEvent.task.id, serviceTime, sys_fTasks);
		}
		
		over_all_system_stats(sys_fEQheadPtr);
		
		int per = (Tcounter - 1) * 10;
		
		fprintf(fp,"\n\n\n\tSystem Performance Summary for RUN # : %d\n", Tcounter);
		fprintf(fp,"\n\t______________________________________________________\n\n");

		fprintf(fp,"\n Simulation Parameters %d\n", Tcounter);
		fprintf(fp,"No CPU = %d, No Tasks= %d, Max Burst Time= %d, IAT= %d, QTRT = %d , QTNRT=%d , Pecentage of RT = %d , Percentage of NRT = %d ", 
			NO_OF_CPU, MAXTASKS, MAXBURSTTIME, IAT, QTRT, QTNRT, per, (100-per));
		fprintf(fp, "\n______________________________________________________________________________\n");

		fprintf(fp,"\n\n");
		fprintf(fp,"\tTotal tasks served = %d, where %d are RT and %d are NRT tasks.\n", sys_fTasks, sys_fTasksRT, sys_fTasksNRT);
		
		fprintf(fp,"\n\n");

		


		fprintf(fp, "\tAverage wating time in the system = %f\n", sys_avrWT );
		fprintf(fp,"\tAverage wating time for RT in the system = %f\n", sys_avrWTRT);
		fprintf(fp,"\tAverage wating time for NRT in the system = %f\n", sys_avrWTNRT);
		
		fprintf(fp,"\n\n");

		fprintf(fp,"\tThe total waiting time in queue : %f \n", total_sys_WTime);
		fprintf(fp,"\tThe average waiting time of the Queue : %f \n", total_WQsize / simTime);
		fprintf(fp,"\n\n");

		fprintf(fp,"\tTotal turn arround time : %f \n", total_turn_time);
		fprintf(fp,"\tAverage turn arround time : %f \n",total_turn_time/MAXTASKS );
		fprintf(fp,"\n\n");
		fprintf(fp,"\tMaximum IAT : %f \n", MAX_IAT);
		
		
		fprintf(fp,"\tMaximum Burst time : %f \n", MAX_BT);
		fprintf(fp,"\n\n");


		fprintf(fp, "\tAverage IAT : %f \n", total_IAT / MAXTASKS);
		fprintf(fp, "\tAverage IAT_RT : %f \n", total_IAT_RT / RT_NO);
		fprintf(fp, "\tAverage IAT_NRT : %f \n", total_IAT_NRT / NRT_NO);


		fprintf(fp,"\tAverage Burst Time : %f \n ",total_BT/MAXTASKS);
		fprintf(fp,"\tAverage Response Time : %f \n ", responseTime/MAXTASKS);
		fprintf(fp,"\n\n");


		fprintf(fp,"\tMaximum Queue Size was : %d \n", MAX_Q_LENGTH);

		fprintf(fp,"\tAverage Queue Length : %f \n", total_QLength / no_of_q_count);

		fprintf(fp,"\tMaximum Task in System : %d \n", max_task_in_system);
		fprintf(fp,"\tAverage task in System : %f \n", over_task_in_system / no_of_q_count);


		//=============================================================================================================================

		fprintf(fp, "\tAverage wating time in the system = %f\n", sys_avrWT);
		fprintf(fp, "\tAverage wating time for RT in the system = %f\n", sys_avrWTRT);
		fprintf(fp, "\tAverage wating time for NRT in the system = %f\n", sys_avrWTNRT);

		fprintf(fp, "\n\n");

		fprintf(fp, "\tThe total waiting time in queue : %f \n", total_sys_WTime);
		fprintf(fp, "\tThe average waiting time of the Queue : %f \n", total_WQsize / simTime);
		fprintf(fp, "\n\n");

		fprintf(fp, "\tTotal turn arround time : %f \n", total_turn_time);
		fprintf(fp, "\tAverage turn arround time : %f \n", total_turn_time / MAXTASKS);
		fprintf(fp, "\n\n");
		fprintf(fp, "\tMaximum IAT : %f \n", MAX_IAT);


		fprintf(fp, "\tMaximum Burst time : %f \n", MAX_BT);
		fprintf(fp, "\n\n");


		fprintf(fp, "\tAverage IAT : %f \n", total_IAT / MAXTASKS);
		fprintf(fp, "\tAverage IAT_RT : %f \n", total_IAT_RT / RT_NO);
		fprintf(fp, "\tAverage IAT_NRT : %f \n", total_IAT_NRT / NRT_NO);


		fprintf(fp, "\tAverage Burst Time : %f \n ", total_BT / MAXTASKS);
		fprintf(fp, "\tAverage Response Time : %f \n ", responseTime / MAXTASKS);
		fprintf(fp, "\n\n");


		fprintf(fp, "\tMaximum Queue Size was : %d \n", MAX_Q_LENGTH);

		fprintf(fp, "\tAverage Queue Length : %f \n", total_QLength / no_of_q_count);

		fprintf(fp, "\tMaximum Task in System : %d \n", max_task_in_system);
		fprintf(fp, "\tAverage task in System : %f \n", over_task_in_system / no_of_q_count);


		fprintf(fp_waiting, "%f\n", total_QLength / no_of_q_count);
		fprintf(IAT_NRTvsRT,"%f\t%f\t%f\n", total_IAT / MAXTASKS , total_IAT_RT / RT_NO , total_IAT_NRT / NRT_NO);


		fprintf(fp, "\n\nSimulation # %d has finished.\n",Tcounter);
		
	}

	fclose(IAT_NRTvsRT);
	fclose(fp_waiting);
	fclose(fp_max);
	fclose(fp);

	printf("\n\nSimulation has finished.\n");
	getch();
}
int total_no_task_in_system(QnodePtr *aqheadPtr, QnodePtr *eqheadPtr , int cpu[]){
	int total = 0, i = 0;

	for (i = 0; i < NO_OF_CPU; i++){
		total += find_list_len(aqheadPtr[i]) + find_list_len(eqheadPtr[i]);
		if (!cpu[i])      //this means one task in CPU
			total++;
	}

	return total;
}
int find_list_len(QnodePtr headPtr){
	int len = 0;
	QnodePtr t;
	if (headPtr == NULL)
		return(0);
	t = headPtr;
	while (t != NULL)
	{
		len++;
		t = t->nextPtr;
	}
	return(len);
}
void enqueue(QnodePtr *headPtr, QnodePtr *tailPtr, Task task){
	QnodePtr newNodePtr = malloc(sizeof(Qnode));
	if (newNodePtr != NULL){
		newNodePtr->data = task;
		newNodePtr->nextPtr = NULL;
		if (isEmpty(*headPtr)){
			*headPtr = newNodePtr;

		}
		else{
			(*tailPtr)->nextPtr = newNodePtr;
		}
		*tailPtr = newNodePtr;
	}
}
void enqueuepr(QnodePtr *headPtr, QnodePtr *tailPtr, Task task){
	QnodePtr newNodePtr = malloc(sizeof(Qnode));

	QnodePtr current = *headPtr, prev = NULL;

	if (newNodePtr != NULL){
		newNodePtr->data = task;
		newNodePtr->nextPtr = NULL;
		newNodePtr->qLength = 0;
	}
		
	while (current != NULL && task.pr >= (current->data).pr){
		prev = current;
		current = current->nextPtr;
	}

	if (prev == NULL){
		newNodePtr->nextPtr = *headPtr;
		//newNodePtr->qLength = (*headPtr)->qLength;
		*headPtr = newNodePtr;
	}
	else{
		newNodePtr->nextPtr = prev->nextPtr;
		prev->nextPtr = newNodePtr;
	}

	if (newNodePtr->nextPtr == NULL){
		*tailPtr = newNodePtr;
	}

	
}
void enqueueevent(EventQnodePtr *headPtr, EventQnodePtr *tailPtr, SchedEvent se){

	//printf("%d QUEUE SIZE %d" , )

	EventQnodePtr newNodePtr = malloc(sizeof(EventQnode));
	if (newNodePtr != NULL){
		newNodePtr->data = se;
		newNodePtr->nextPtr = NULL;

		
	}
	EventQnodePtr current = *headPtr, prev = NULL;
	while (current != NULL && se.time>(current->data).time){
		prev = current;
		current = current->nextPtr;
	}
	while (current != NULL && se.time == (current->data).time && se.event<(current->data).event){
		prev = current;
		current = current->nextPtr;
	}

	if (prev == NULL){
		newNodePtr->nextPtr = *headPtr;
		*headPtr = newNodePtr;
		
	}
	else{
		newNodePtr->nextPtr = prev->nextPtr;
		
		prev->nextPtr = newNodePtr;
	}
	if (newNodePtr->nextPtr == NULL){
		*tailPtr = newNodePtr;
	}
}
Task dequeue(QnodePtr *headPtr, QnodePtr *tailPtr){
	Task value;
	QnodePtr tempPtr;
	value = (*headPtr)->data;

	tempPtr = *headPtr;
	*headPtr = (*headPtr)->nextPtr;
	if (*headPtr == NULL){
		*tailPtr = NULL;
		 qSize = 0;
	}

	free(tempPtr);
	return value;
}
SchedEvent dequeueevent(EventQnodePtr *headPtr, EventQnodePtr *tailPtr){
	SchedEvent value;
	EventQnodePtr tempPtr;
	value = (*headPtr)->data;
	tempPtr = *headPtr;
	*headPtr = (*headPtr)->nextPtr;
	

	if (*headPtr == NULL){
		*tailPtr = NULL;
	}
	free(tempPtr);
	return value;
}
int isEmpty(QnodePtr headPtr){
	return headPtr == NULL;
}
int isEmptyEQ(EventQnodePtr headPtr){
	return headPtr == NULL;
}
void displayEvents(EventQnodePtr currentPtr){
	if (currentPtr == NULL)
		printf("Queue is empty ...\n");
	else{
		printf("Queue is:\n");
		SchedEvent tempevent;
		while (currentPtr != NULL){
			printf("\ntime: %d, event: %d\n",
				(currentPtr->data).time, (currentPtr->data).event);
			currentPtr = currentPtr->nextPtr;
		}
	}
}
void over_all_system_stats(EventQnodePtr currentPtr){
	
	sys_fTasks = sys_wt = sys_avrWT = sys_avrWTRT = sys_avrWTNRT = 0;

	if (currentPtr == NULL)
		return;
	else{
		SchedEvent tempevent;
		while (currentPtr != NULL){
			sys_fTasks++;
			sys_wt += (currentPtr->data).time -
				((currentPtr->data).task.at + (currentPtr->data).task.bt);
			if ((currentPtr->data).task.type == 0){
				sys_wtRT += (currentPtr->data).time -
					((currentPtr->data).task.at + (currentPtr->data).task.bt);
				sys_fTasksRT++;
			}
			else{
				sys_wtNRT += (currentPtr->data).time -
					((currentPtr->data).task.at + (currentPtr->data).task.bt);
				sys_fTasksNRT++;
			}
			total_sys_WTime += sys_wtNRT + sys_wtNRT;
			currentPtr = currentPtr->nextPtr;
		}

		sys_avrWT = (double)sys_wt / sys_fTasks;
		sys_avrWTRT = (double)sys_wtRT / sys_fTasksRT;
		sys_avrWTNRT = (double)sys_wtNRT / sys_fTasksNRT;
	
	}
}
























