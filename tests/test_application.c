#include <stdio.h>
#include <string.h>
#include "../src/PD/PD_algorithm.h"
#include "../src/common/cadena.h"
#include "../src/assignment/aproblem.h"
#include "../src/assignment/task.h"
#include "../src/assignment/resource.h"
#include "../src/assignment/PD/a_problem_PD.h"
#include "../src/assignment/solution.h"
#include "../src/assignment/aproblem_gui.h"
#include <stdlib.h>

extern int redistribution_rr;
extern int redistribution_rr_all;
extern GtkWidget *window;
extern Cadena test;
extern int var_test[7];
extern int fs;
extern int type_best;

MAX_L_BUFF=100;

NUM_TESTS=11;


//double expected[11]={2.,38.,51.,5.,1025.,46.,7.,7.,8.,10.,18.};//matrix size >=2	<=12


int test_set(int np);



int test_set(int np){
	if(print_all)
	{
		printf("\n*** test_set ***\n");
	}

	test_set_data(test,np);
	return 0;
}
int test_set_data(Cadena t, int np){
	if(print_all)
	{
		printf("\n*** test_set_data***\n");
	}
	printf("\n*******************************************\n");
	init_clock();
    int numt;
    int numr;
    int i=atoi(t);

////////////////////////////////////////////GUROBI RESULTs

    FILE *file  = fopen("/home/practica/eclipse-workspace/c/tests/expected", "r"); // read only
    char buff[MAX_L_BUFF];
    char clean_buff[MAX_L_BUFF];
    //  not existing.
    if (file == NULL )
      {
        printf("Error! Could not open file\n");
        exit(-1);
      }
	while (fgets(buff, MAX_L_BUFF, file))
	    {
			strtok(buff, "\n");
	        //printf(" %s\n", buff);
	    }
	fclose(file);
	for(int i=0;i<MAX_L_BUFF-1;i++)
	{
		clean_buff[i]=buff[i+1];
	}
	double double_aux;
	double expected[NUM_TESTS];
	char split=',';
	char *ptr_aux;
	Cadena aux;
	char *ptr=strtok(clean_buff, &split);
	int count=0;
	while (ptr !=NULL)
	{
		strcpy(aux,ptr);
		double_aux=strtod(aux,&ptr_aux);
		//printf("number->%f",double_aux);
		expected[count]=double_aux;
		//printf("%f",expected[count]);
		ptr=strtok(NULL, &split);
		count =count+1;
	}

/////////////////////////////////////////////////////////////////////////////////

	Cadena cadena_url_init="/home/practica/eclipse-workspace/c/files/data";
	Cadena cadena_url_end=".txt";
	Cadena url_center;
	numt=i;
	numr=i;

	sprintf(url_center, "%d",i);
	strcat(cadena_url_init,url_center);
	strcat(cadena_url_init,cadena_url_end);
	printf("\n%s",cadena_url_init);
	int e=read_aproblem_file(&pap_from_gui, numt, numr, cadena_url_init);

	prune=var_test[1];
	redistribution_rr=var_test[2];
	tuple_p=var_test[3];
	fs=var_test[4];
	type_best=var_test[5];
	redistribution_rr_all=var_test[6];
	if(print_all)
	{
		printf("\ntest_application.c		test_set_data()-> READED file  %d\n",e);
		printf("\ntest_application.c		test_set_data()		prune->%d",prune);
		printf("\ntest_application.c		test_set_data()		rr->%d",redistribution_rr);
		printf ("\ntest_application.c		test_set_data()		tuple->%d",tuple_p);
		printf ("\ntest_application.c		test_set_data()		fs->%d",fs);
		printf ("\ntest_application.c		test_set_data()		alg->%d",type_best);
		printf ("\ntest_application.c		test_set_data()		rr_all->%d",redistribution_rr_all);
	}
	if(redistribution_rr_all==1)//preference
	{
		redistribution_rr=1;
	}
	if(print_all)
	{
		printf("\ntest_application.c		test_set_data()-> pre resolve apd test");
	}
	resolve_aPD_test(&pap_from_gui, np,prune,redistribution_rr,tuple_p,fs,type_best,redistribution_rr_all);
	count=0;
	while(final_alg.best!=expected[i-2])//matrix size <=2
	{
		count++;
		if(count>=900000)//Timeout
		{
			printf("\ntest_application.c		test_set_data(%d)-> TIMEOUT expected:%f",i, expected[i-2]);
			printf("   final_alg.best->%f",final_alg.best);
			break;
		}

	}
	if(count<900000)
	{
		printf("\ntest_application.c		test_set_data(%d)-> OK: acum %f\n",i,final_alg.best);
	}

	return 0;

}

