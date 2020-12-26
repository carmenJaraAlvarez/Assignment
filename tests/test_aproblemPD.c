#include <stdio.h>
#include <string.h>
#include "../src/common/cadena.h"
#include "../src/assignment/aproblem.h"
#include "../src/assignment/task.h"
#include "../src/assignment/resource.h"
#include "../src/assignment/PD/a_problem_PD.h"
#include "../src/assignment/solution.h"
void testInitAProblemPD(PAproblemPD, Aproblem);
void testInit7();
void testInit8();


void testInitAProblemPD(PAproblemPD papd, Aproblem a){
	printf("\n*** testInitAProblemPD ***\n");
	int res=-1;
	res = initAProblemPD(papd, a);

	if(res == -1) {
			perror("error initAProblemPD.\n");
		} else {
			printf("ok initAProblemPD.\n");
			printf("index: %d\n",papd->index);
		}
}


void testInit7(){
	Aproblem ap;
	ArrayPTasks tasks={};
	ArrayPResources resources={};
	int numTasks=3;
	int numResources=3;
	double vector[9]={1.,20000.,3.,0.,0.,1.,9.,8.,700.};
	testInitAProblem(&ap,tasks,resources, numTasks,numResources,vector);
	AproblemPD appd;
	testInitAProblemPD(&appd, ap);
	showAproblem(&(appd.aproblem));
}
void testInit8(){
	Aproblem ap;
	ArrayPTasks tasks={};
	ArrayPResources resources={};
	int numTasks=3;
	int numResources=3;
	double vector[9]={1.,20000.,3.,0.,0.,1.,9.,8.,700.};
	testInitAProblem(&ap,tasks,resources, numTasks,numResources,vector);
	AproblemPD appd;
	testInitAProblemPD(&appd, ap);
	showAproblem(&(appd.aproblem));
	Logico b=is_base_case(&appd);
	printf("is base case...%d\n", b);//?????????????
	//change to base case
	Solution sol;
	Alternative a;
	for(int i=0;i<2;i++){
		a.indexResource=i;
		updateSolution(&sol, &a, i);
	}
	appd.solution=sol;
	appd.index=2;

	b=is_base_case(&appd);
	printf("is base case...%d\n", b);
	if(b){
		printf("getting base case solution...\n");
		get_solution_base_case(&appd);
		solutionToString(appd.solution);

	}



}