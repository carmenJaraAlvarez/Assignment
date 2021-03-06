#include <stdio.h>
#include <string.h>
#include "../src/common/cadena.h"
#include "../src/assignment/aproblem.h"
#include "../src/assignment/task.h"
#include "../src/assignment/resource.h"


void testInitAProblem(PAproblem pa,PTask tasks,PResource resources,int numTasks, int numResources, double values[]);
void testInit5();
void testInit6();


void testInitAProblem(PAproblem pa,PTask tasks,PResource resources,int numTasks, int numResources, double *values){
	printf("*** testInitAProblem ***\n");
	int res=-1;
	res = init_aproblem(pa, tasks, resources, numTasks, numResources,values);
	if(res == -1) {
			printf("error initAProblem.\n");
		} else {
			printf("ok initAProblem.\n");
			printf("\n");
		}
}


void testInit5(){
	printf("\n5***********************\n");
	Aproblem ap;
	int numTasks=3;
	int numResources=3;
	Task t1;
	init_task(&t1,"T1");
	Task t2;
	init_task(&t2,"T2");
	Task t3;
	init_task(&t3,"T3");
	Task tasks[numTasks];
	tasks[0]=t1;
	tasks[1]=t2;
	tasks[2]=t3;

	Resource r1;
	init_resource(&r1,"R1",0);
	Resource r2;
	init_resource(&r2,"R2",1);
	Resource r3;
	init_resource(&r3,"R3",2);
	Resource resources[numResources];
	resources[0]=r1;
	resources[1]=r2;
	resources[2]=r3;
	double vector[9]={1.,200.,3.,0.,0.,1.,9.,8.,7.};
	testInitAProblem(&ap,tasks,resources, numTasks,numResources,vector);
	show_aproblem(&ap);
	delete_aproblem(&ap);
}
void testInit6(){
	printf("\n6***********************\n");
	Aproblem ap;
	int numTasks=3;
	int numResources=3;
	Cadena url="/home/practica/eclipse-workspace/c/files/data3.txt";
	read_aproblem_file(&ap, numTasks, numResources, url);
	printf("Reading problem...");
	show_aproblem(&ap);
	delete_aproblem(&ap);
}
void testInit6b(){
	Aproblem ap;
	int numTasks;
	int numResources;
	printf("\nNum tasks \n");
	scanf("%d",&numTasks);
	printf("\nNum tasks : %d\n", numTasks);
	printf("\nNum resources \n");
	scanf("%d",&numResources);
	Cadena tasksNames[numTasks];
	printf("\nName task \n");
	for(int i=0;i<numTasks;i++)
	{
		scanf("%s",tasksNames[i]);
	}
	Cadena resourcesNames[numResources];
	printf("\nName resource \n");
	for(int i=0;i<numTasks;i++)
	{
		scanf("%s",resourcesNames[i]);
	}
	double values[numTasks*numResources];
	printf("\nValues \n");
	for(int i=0;i<numTasks*numResources;i++)
	{
		scanf("%lf",&values[i]);
	}

	Task t;
	Task tasks[numTasks];
	for(int i=0;i<numTasks;i++)
	{
		init_task(&t,tasksNames[i]);
		tasks[i]=t;
	}

	Resource r;
	PResource resources;
	for(int i=0;i<numResources;i++)
	{
		init_resource(&r,resourcesNames[i],i);
		resources[i]=r;
	}
	testInitAProblem(&ap,tasks,resources, numTasks,numResources,values);
	show_aproblem(&ap);
	delete_aproblem(&ap);
}


