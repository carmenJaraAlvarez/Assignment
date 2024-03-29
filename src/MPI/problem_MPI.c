
#include "problem_MPI.h"
#include <stddef.h>

static int pD_distribution(PalgorithmPD palg);
static PResolved resolved_from_slaves;
static Distribution_rr rr;

static int index_rr=0;
static int init_rr();
static int init_resolved;
static int send_more_work(int process);
static MPI_Request request_w[100];//TODO
static int buffer_work;

static int send_more_work(int process)
{
	if(print_all)
	{
		printf("\nproblem_MPI.c		send_more_work()		to %d",process);
	}


	if(0)//TODO batch
	{
		int n;
		MPI_Request request_rcv;
		MPI_Datatype node_mpi_datatype;
		int blocklengths[3] = {1,100,1};
		MPI_Datatype types[3] = {MPI_INT, MPI_INT,MPI_DOUBLE};
		const MPI_Aint offsets[3]= { 0, sizeof(int),sizeof(int)*101};
		MPI_Type_create_struct(3, blocklengths, offsets, types,  &node_mpi_datatype);
		MPI_Type_commit ( &node_mpi_datatype );
		MPI_Status status;
		MPI_Isend(&n, 1, MPI_INT, process, tag_give_work, MPI_COMM_WORLD, &request_w);
		Node_work node_w;
		MPI_Irecv(&node_w, 1, node_mpi_datatype, process, tag_give_work, MPI_COMM_WORLD, &request_rcv);
	}
	else
	{//Boxy master
		int sender;
		int n=process;
//		if(rr.len_receivers_plus>=rr.index &&rr.len_receivers_plus>0)
//		{
//			sender=rr.receivers_plus[rr.index];
//			if(print_all)
//			{
//				//problem_MPI.c		send_more_work() to 3.	sender plus 219400.000000
//				printf("\nproblem_MPI.c		send_more_work() to %d.	sender plus %d",process,sender);
//			}
//			MPI_Isend(&n, 1, MPI_INT, sender, tag_give_work, MPI_COMM_WORLD, &request_w[sender]);
//			rr.index++;
//		}
//		else if((rr.len_receivers_plus+rr.len_receivers_less)>(rr.index))
//		{
//			sender=rr.receivers_less[rr.index-rr.len_receivers_plus];
//			if(print_all)
//			{
//				printf("\nproblem_MPI.c		send_more_work() to %d.	sender less %d",process,sender);
//			}
//			MPI_Isend(&n, 1, MPI_INT, sender, tag_give_work, MPI_COMM_WORLD, &request_w[sender]);
//			rr.index++;
//		}
		if(index_rr+1==n){
			index_rr++;
		}
		if(index_rr+1<numprocs)
		{
			sender=index_rr+1;
			index_rr++;
		}
		else
		{
			index_rr=0;
			sender=index_rr+1;
			index_rr++;
		}

		MPI_Isend(&n, 1, MPI_INT, sender, tag_give_work, MPI_COMM_WORLD, &request_w[sender]);

//		else
//		{
//			//TODO
//			if(print_all)
//			{
//				printf("\nproblem_MPI.c		send_more_work()		NO more work to %d",process);
//			}
//
//		}


	}

	return 0;
}

static int init_rr()
{
	rr.index=0;
	rr.len_receivers_empty=0;
	rr.len_receivers_less=0;
	rr.len_receivers_plus=0;
	// memory
	int memoryArrayValues=sizeof(int)*numprocs;
	rr.receivers_plus=(int*)malloc(memoryArrayValues);
	rr.receivers_less=(int*)malloc(memoryArrayValues);
	rr.receivers_empty=(int*)malloc(memoryArrayValues);
	return 0;
}
int init_redistribution(MPI_Request * request_work){
//	int memoryArrayValues=sizeof(MPI_Request)*numprocs;
//	request_w=(double*)malloc(memoryArrayValues);
	for(int i=0;i<numprocs;i++)
	{
		request_w[i]=request_work[i];
	}

	return 0;
}
int distribution(PalgorithmPD palg, int prune, int r_rr, int tuple_p, int fs, int rr_all, int alg_best)
{
	int res=-1;
	Solution sol;
	int num_slaves=numprocs-1;
    MPI_Request request;
    MPI_Status status;
    int flag;

    if(print_all)
    {
    	printf("\n num problems-num slaves: %d-%d",palg->num_problems,num_slaves);
    }

    if(r_rr)
    {
    	init_rr();
    }

	do
	{
		if(print_all)
		{
			printf("\n num subpr: %d",get_num_subproblems());
		}

		for(int i=0;i<get_num_subproblems();i++)
		{
			pD_distribution(palg);
		}
	}while(palg->isRandomize && get_PDsolution(palg,&sol)!=0);


	if(palg->num_problems==num_slaves)
	{
		if(print_all)
		{
			printf("\nproblem_MPI.c		distribution()		if num problems=num slaves");
		}


		for(int i=1; i<(num_slaves+1);i++)
		{
			if(print_all)
			{
				printf("\nproblem_MPI.c		distribution()		inside for");
			}
			int alternatives[1];
			alternatives[0]=i-1;
			if(print_all)
			{
				printf("\nproblem_MPI.c		distribution()		sending best=%f",final_alg.best);
				printf("\nproblem_MPI.c		distribution()		num problems==num slaves: alternative->%d",alternatives[0]);
			}

			send_work(palg,&alternatives,1,i,prune,r_rr, tuple_p,fs,rr_all,final_alg.best);//sending 1 alternative to process i
			if(r_rr)
			{
				rr.receivers_less[i-1]=i;
				rr.len_receivers_less++;
			}

		}
	}
	else if(palg->num_problems<num_slaves)
	{
		if(print_all)
		{
			printf("\nproblem_MPI.c		distribution()		else if");
		}

		int i;
		for(i=1; i<(palg->num_problems+1);i++)
		{
			int alternatives[1];
			alternatives[0]=i-1;
			if(print_all)
			{
				printf("\n num problems<num slaves: alternative->%d",alternatives[0]);
			}

			send_work(palg,&alternatives,1,i,prune,r_rr, tuple_p,fs,rr_all,final_alg.best);
			if(r_rr)
			{
				rr.receivers_less[i-1]=i;
				rr.len_receivers_less++;
			}
			if(print_all)
			{
				printf("\nproblem_MPI		distribution()		rcv_less %d, len %d",rr.receivers_less[i-1],rr.len_receivers_less);
			}

		}
		for(int j=palg->num_problems+1;j<num_slaves+1;j++)
		{
			int alternatives[0];
			send_work(palg,&alternatives,0,j,prune,r_rr, tuple_p,fs,rr_all,final_alg.best);
		}
	}
	else//num problems >num processes
	{

		int rounds=(palg->num_problems)/num_slaves;
		int more_round=(palg->num_problems)-num_slaves*rounds;

		for(int i=1; i<(more_round+1);i++)
		{
			if(print_all)
			{
				printf("\nproblem_MPI		distribution()		sending case num problems >num processes. resource 0: %s",palg->ppd.aproblem.resources[0].name);

			}
			int alternatives[100];//TODO
			for(int j=0;j<rounds+1;j++)
			{
				if(print_all)
				{
					printf("\nproblem_MPI		distribution()		sending case num problems >num processes. resource 0: %s",palg->ppd.aproblem.resources[0].name);
				}

				int a=(i-1)+(j*num_slaves);
				if(print_all)
				{
					printf("\"\nproblem_MPI.c		distribution()		alternative %d=%d",j,a);
				}

				alternatives[j]=a;//process i, distribution round j
			}
			send_work(palg,&alternatives,rounds+1,i,prune,r_rr, tuple_p,fs,rr_all,final_alg.best);
			if(r_rr)
			{
				rr.receivers_plus[i-1]=i;
				rr.len_receivers_plus++;
				if(print_all)
				{
					printf("\nproblem_MPI		distribution()		rcv_plus %d, len %d",rr.receivers_plus[i-1],rr.len_receivers_plus);
				}
			}
		}
		for(int i=more_round+1;i<num_slaves+1;i++)
		{
			int alternatives[MAX_NUM_ALT];
			for(int j=0;j<rounds;j++)
			{
				alternatives[j]=(i-1)+(j*num_slaves);//process i, distribution round j
			}
			send_work(palg,alternatives,rounds,i,prune,r_rr, tuple_p,fs,rr_all,final_alg.best);
			if(r_rr)
			{
				rr.receivers_less[i-1]=i;
				rr.len_receivers_less++;
			}
		}

	}
	if(print_all)
	{
		printf("\nproblem_MPI.c		distribution()		outgoing");
	}

	return res;


}

int rcv_work(double * buffer,MPI_Request * request_b,int * buffer_w)
{
	MPE_Log_event(event2a, 0, "start receive work");
	int res=0;
	int id;
	MPI_Comm_rank(MPI_COMM_WORLD,&id);
	if(print_all)
	{
		printf("\nproblem_MPI.c	rcv_work()	init. I am %d\n",id);
	}

	struct Work work;

	//getting work

	MPI_Datatype work_mpi_datatype;
	int blocklengths[10] = {1,1,1,1,1,1,1,1,1,1};
	MPI_Datatype types[10] = {MPI_INT, MPI_INT,MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT,MPI_DOUBLE};
	const MPI_Aint offsets[10]= { 0, sizeof(int),sizeof(int)*2,sizeof(int)*3,sizeof(int)*4,sizeof(int)*5,sizeof(int)*6,sizeof(int)*7,sizeof(int)*8,sizeof(int)*9};
	MPI_Type_create_struct(10, blocklengths, offsets, types,  &work_mpi_datatype);
	MPI_Type_commit ( &work_mpi_datatype );
	MPI_Status status;

    MPI_Recv(&work, 1, work_mpi_datatype , 0, tag_work, MPI_COMM_WORLD, &status);

    if(print_all)
    {
        printf("\nproblem_MPI.c		rcv_work()		p%d. received num task: %d",id,work.num_tasks);
    	printf("\nproblem_MPI.c		rcv_work()		p%d. received num resources: %d\n",id,work.num_resources);
    	printf("\nproblem_MPI.c		rcv_work()		p%d. received prune: %d",id,work.prune);
    	printf("\nproblem_MPI.c		rcv_work()		p%d. received rr_all: %d",id,work.rrall);
    	printf("\nproblem_MPI.c		rcv_work()		p%d. received best: %f",id,work.best);
    }
    prune=work.prune;
    tuple_p=work.tuple_prune;
    fs=work.first_search;
    redistribution_rr=work.redistribution;
    redistribution_rr_all=work.rrall;
	int size_values=work.num_tasks*work.num_resources;

	//getting values

	double values[size_values];

	MPI_Recv ( &values, size_values, MPI_DOUBLE, MPI_ANY_SOURCE, tag_values, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	if(print_all)
	{
		printf("\nproblem_MPI.c	rcv_work()	p%d. num alternatives:%d\n",id,work.num_alternatives);
		printf("%d resources\n",work.num_resources);
		printf("%d received size values\n",size_values);
		printf("%d received type\n",work.type);
		printf("%f received first\n",values[0]);
		printf("%f received second\n",values[1]);

	}


	//getting tasks and resources names

	char serialized[MAX_TAM_STRING];
	Task tasks[work.num_tasks];

	char serialized_resources[MAX_TAM_STRING];
	Resource resources[work.num_resources];

	MPI_Recv( &serialized, 1000, MPI_CHAR, MPI_ANY_SOURCE, tag_tasks, MPI_COMM_WORLD,MPI_STATUS_IGNORE );

	deserializer_tasks(&serialized,work.num_tasks,tasks);
	if(print_all)
	{
		printf("\nproblem_MPI.c	rcv_work()	p%d. POST RCVE TASKS name 3º task\n%s\n",id,tasks[2].name);
	}

	MPI_Recv( &serialized_resources, MAX_TAM_STRING, MPI_CHAR, MPI_ANY_SOURCE, tag_resources, MPI_COMM_WORLD,MPI_STATUS_IGNORE );
	deserializer_resources(&serialized_resources,work.num_resources,resources);
	if(print_all)
	{
		printf("\nproblem_MPI.c		rcv_work()	p%d. POST RCVE RESOURCES name 3º resource\n%s\n",id,resources[2].name);

	}

    int alternatives[work.num_alternatives];
    if(work.num_alternatives>0)
    {
    	MPI_Recv ( &alternatives, work.num_alternatives, MPI_INT, MPI_ANY_SOURCE, tag_alternatives, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    }


	MPE_Log_event(event2b, 0, "end receive work");
	Aproblem a;
	init_aproblem(&a,tasks,resources,work.num_tasks, work.num_resources, values);
	if(print_all)
	{
		printf("\n\nproblem_MPI.c	rcv_work()	p%d",id);
		show_aproblem(&a);
		printf("\n\nproblem_MPI.c	rcv_work()	p%d end show problem",id);
	}

	MPI_Type_free ( &work_mpi_datatype);
	Resolved resolved;
    init_work(&a, work.num_alternatives, &alternatives,buffer,request_b,buffer_w,work.best,&resolved,fs);
    if(print_all)
    {
        int id;
        MPI_Comm_rank(MPI_COMM_WORLD,&id);
        printf("\nproblem_MPI.c	rcv_work()	outgoing process %d. num resolved:%d\n",id,resolved.num_resolved);
        show_aproblem(&a);
    }
    delete_aproblem(&a);
	return res;
}

int ask_work()
{
	MPE_Log_event(event1, 0, "ask work");
	int n=0;
	int master=0;
	MPI_Request request;
	MPI_Isend(&n, 1, MPI_INT, master, tag_ask_work, MPI_COMM_WORLD,&request);
	return 0;
}

int rcv_resolved()
{
		MPE_Log_event(event6a, 0, "start receive resolved");
	if(print_all)
	{
		printf("+\nproblem_MPI.c		rcv_resolved()");

	}
	int res=0;

	 int memoryArrayValues=sizeof(Resolved)*numprocs;
	 resolved_from_slaves=(Resolved*)malloc(memoryArrayValues);

	//getting

	MPI_Datatype resolved_mpi_datatype;
	int blocklengths[3] = {1,1,100};
	MPI_Datatype types[3] = {MPI_DOUBLE,MPI_DOUBLE,MPI_INT};
	const MPI_Aint offsets[3]= { 0, sizeof(double),sizeof(double)*2};

	MPI_Type_create_struct(3, blocklengths, offsets, types,  &resolved_mpi_datatype);
	MPI_Type_commit ( &resolved_mpi_datatype );
	MPI_Status status;
	//rcve every process
	for(int p=1;p<numprocs;p++)
	{

		if(print_all)
		{
			printf("+\nproblem_MPI.c		rcv_resolved()		pre mpi_rcev\n");
		}


		MPI_Recv(&(resolved_from_slaves[p]), 1, resolved_mpi_datatype , p, tag_resolved, MPI_COMM_WORLD, &status);

		if(resolved_from_slaves[p].num_resolved>0)//TODO
		{
			//palg->best=resolved.value;
			if(print_all)
			{
				printf("\nproblem_MPI.c		rcv_resolved()	 BEST IN -> %f",resolved_from_slaves[p].value);
				printf("\nproblem_MPI.c		rcv_resolved()	 NUM SOLVED IN-> %d",resolved_from_slaves[p].num_resolved);
			}

			for(int i=0; i<resolved_from_slaves[p].num_resolved;i++)
			{
				if(print_all)
				{
					printf("\nproblem_MPI.c		rcv_resolved()	 Inside for EACH resolved of process %d",p);
				}
				if(print_all)
				{
					printf("\nproblem_MPI.c		rcv_resolved()	 resolved_from_slaves[p].value: %f",resolved_from_slaves[p].value);
				}
				if(resolved_from_slaves[p].value>=final_alg.best)//TODO
				{
					if(print_all)
					{
						printf("\nproblem_MPI.c		rcv_resolved()	 Inside if received:%f >=final:%f",resolved_from_slaves[p].value,final_alg.best);
					}
					final_alg.best=resolved_from_slaves[p].value;
					for(int j=0;j<final_alg.ppd.aproblem.numTask;j++)//TODO
					{

		//				final_alg.solvedProblems[i].solution.resources[j].position=resolved.resources[j];
						final_sol[j]=resolved_from_slaves[p].resources[j];//TODO create resolved problem
						if(print_all)
							{
							printf("\nproblem_MPI.c		rcv_resolved()		received t%d->%d",j, resolved_from_slaves[p].resources[j]);
							printf("\nproblem_MPI.c		rcv_resolved()		final_alg from process %d t%d->",i, j);

							}
					}
					final_alg.num_solved=resolved_from_slaves[p].num_resolved;


				}
				}


		}
		if(print_all)
		{
			printf("+\nproble_MPI.c		rcv_resolved()		received resolved from %d\n ",p);
		}

	}



    MPI_Type_free ( &resolved_mpi_datatype);
    free(resolved_from_slaves);
    MPE_Log_event(event6b, 0, "end receive resolved");
    if(print_all)
    {
    	  printf("+\n+++++++++++++++++++++++++++++++++++++++++++++++++++\nEND RCV RESOLVED\n");

    }
    return res;

}
int confirming_work(int sender)
{
	int res=0;
	int myid;
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	int n=1;
	int count = 1;
//	MPE_Log_event(event3a, 0, "start send best");
	MPI_Request request_c;
	if(print_all)
	{
		printf("\nproblem_MPI.c		confirming_work()");
	}

	MPI_Isend(&n, count, MPI_INT, sender, tag_redistribution+myid, MPI_COMM_WORLD, &request_c);
	int ready=0;
	return res;

}
int init_serial(
		PalgorithmPD final_alg,
		int prune,
		int redistribution_rr,
		int tuple_p,
		int fs)
{

	double buffer=0.0;//not using in serial
	MPI_Request request_work=MPI_REQUEST_NULL;//not using in serial
	int buffer_w=1;//not using in serial
	AlgorithmPD alg;
	init_algorithmPD(&alg, final_alg->ppd, fs);
	exec_algorithm(&alg,&buffer,request_b, &buffer_w, &request_work, fs);

	for(int j=0;j<final_alg->ppd.aproblem.numTask;j++)
	{

		//final_sol[j]=resolved_from_slaves[p].resources[j];//TODO create resolved problem
		final_sol[j]=alg.solvedProblems[0].solution.resources[j].position;
		if(print_all)
			{
			printf("\nproblem_MPI.c		init_serial()		task %d", j);

			}
	}
	final_alg->best=alg.solvedProblems[0].solution.acum;
//	free(alg.problems);
//	free(alg.solvedProblems);
return 0;

}
int init_work(
		PAproblem pa,
		int num_alternatives,
		int * alternatives,
		double * buffer,
		MPI_Request * request_b,
		int * buffer_w,
		double best,
		Resolved *resolved,
		int search)
{
	MPI_Request request_work=MPI_REQUEST_NULL;//to listen to masterś order
	int res=0;
	AproblemPD appd;
	initAProblemPD(&appd, pa);
	AlgorithmPD alg;
	init_algorithmPD(&alg, appd,search);
	alg.best=best;
	int id;
	MPI_Comm_rank(MPI_COMM_WORLD,&id);
	if(print_all)
	{
		printf("\n problem_MPI.c		init_work()		p%d best post init: %f",id,alg.best);
		//show_aproblem(&(alg.ppd.aproblem));
		printf("\n*******************************");
	}

	new_best=alg.best;
	MPE_Log_event(event5, 0, "init listening");
	MPI_Irecv(&new_best, 1, MPI_DOUBLE, master, tag_best, MPI_COMM_WORLD, &request_bcast);

	if (num_alternatives==0){
		if(print_all)
		{
			printf("\nproblem_MPI.c		init_work() p%d -0 alternative rr:%d",id,redistribution_rr);
		}

		if(!redistribution_rr)
		{
			alg.num_solved=0;
			send_resolved(&alg);
			delete_algorithmPD(&alg);
			if(print_all)
			{
				printf("\nproblem_MPI.c		init_work() no rr. alg.num solved:%d",alg.num_solved);
			}

		}
		else
		{
			if(print_all)
			{
				printf("\nproblem_MPI.c		init_work() asking work p%d",id);
			}
			ask_work();
			Node_work node;
			MPI_Request r=MPI_REQUEST_NULL;
			MPI_Status s;
			int ready=0;

			int myid;
			MPI_Comm_rank(MPI_COMM_WORLD,&myid);

			MPI_Datatype node_mpi_datatype;
			int blocklengths[3] = {1,1,100};
			MPI_Datatype types[3] = {MPI_INT, MPI_DOUBLE,MPI_INT};
			const MPI_Aint offsets[3]= { 0, sizeof(int),sizeof(double)+sizeof(int)};
			MPI_Type_create_struct(3, blocklengths, offsets, types,  &node_mpi_datatype);
			MPI_Type_commit ( &node_mpi_datatype);


			MPI_Irecv(&node, 1, node_mpi_datatype, MPI_ANY_SOURCE, tag_redistribution+myid, MPI_COMM_WORLD, &r);

			int i=0;
			while(!ready && i<90000)//TODO
			{
				MPI_Test(&r,&ready,&s);
				i++;
			}
			if (!ready) {
				MPE_Log_event(event8, 0, "TIMEOUT");
				if(print_all)
				{
					printf("\nproblem_MPI.c		init_work()		p%d waiting work TIMEOUT",id);
				}
				if(redistribution_rr_all)
				{
					send_resolved_data(1,resolved->value,pa->numTask,resolved->resources);//TODO num resolved
					if(print_all)
					{
						printf("\nproblem_MPI.c		init_work()		p%d TIMEOUT rr_all send resolved data",id);
						for(int d=0;d<pa->numTask;d++)
						{
							printf("\nproblem_MPI.c		init_work()		p%d send %d:%d",id,d,resolved->resources[d]);
						}
					}

					delete_algorithmPD(&alg);

				}
				else
				{
					alg.num_solved=0;
					send_resolved(&alg);
					delete_algorithmPD(&alg);
					if(print_all)
					{
						printf("\nproblem_MPI.c		init_work()		p%d TIMEOUT NO_rr_all send resolved(alg)",id);
					}
				}

			}
			else
			{
				int sender=s.MPI_SOURCE;
				confirming_work(sender);
				if(print_all)
				{
					printf("\nproblem_MPI.c		init_work()		ready. Index= %d in p%d", node.index,id);
				}

				alg.ppd.index=node.index;
				alg.ppd.solution.lengthArrays=node.index;
				alg.ppd.solution.acum=node.value;
				for(int i=0;i<node.index;i++)
				{
					strcpy(alg.ppd.solution.resources[i].name, pa->resources[node.solution[i]].name);
					int aux=node.solution[i];
					Resource aux_r;

					Cadena aux_name;
					strcpy(&aux_name,pa->resources[aux].name);
					init_resource(&aux_r,aux_name,aux);
					alg.ppd.solution.resources[i].position=aux;


					if(print_all)
					{

						printf("\nproblem_MPI.c		init_work()		rcv sol resource %d: %d",i,aux);
						printf("\nproblem_MPI.c		init_work()		copied in ppd.solution %d: %d",i,alg.ppd.solution.resources[i].position);
						printf("\nproblem_MPI.c		init_work()		copied name resource %d: %s",i,aux_name);
						printf("\nproblem_MPI.c		init_work()		copied name in aux_r %d: %s",i,aux_name);

						printf("\nproblem_MPI.c		init_work()		copied num resource in aux_r %d: %d",i,aux_r.position);

					}
				}
				alg.problems[0]=alg.ppd;
				if(print_all)
				{
					for(int i=0;i<node.index;i++)
					{
						printf("\nproblem_MPI.c		init_work()		p%d rcv sol proble[0] %d: %d",id,i,alg.problems[0].solution.resources[i].position);
						printf("\nproblem_MPI.c		init_work()		p%d copied in ppd.solution %d: %d",id,i,alg.ppd.solution.resources[i].position);

					}

				}
				exec_algorithm(&alg,buffer,request_b, buffer_w, &request_work, fs);
				if(print_all)
				{
					printf("\nAfter exec algoritm p%d \n",id);
				}
				Solution sol;
				get_PDsolution(&alg, &sol);
				if(print_all)
				{
					for(int i=0;i<alg.ppd.solution.lengthArrays;i++){
						printf("\nResources: \n*%s\n",alg.ppd.solution.resources[i].name);
					}
					printf("Solution value: %f", alg.ppd.solution.acum);
				}

				if(redistribution_rr_all)
				{
					if(print_all)
					{
						printf("\nproblem_MPI.c		init_work()		p%d  inside redistribution_rr_all, no timeout",id);
					}
					//select resolved
					if(init_resolved==0)
					{
						//init resolved
						if(print_all)
						{
							printf("\nproblem_MPI.c		init_work()		init_resolved=0");
						}
						if(alg.num_solved>0)
						{
							resolved->num_resolved=alg.num_solved;
							resolved->value=alg.solvedProblems[0].solution.acum;
							for(int i=0;i<alg.ppd.aproblem.numTask;i++)
							{
								resolved->resources[i]=alg.solvedProblems[0].solution.resources[i].position;
							}
							init_resolved=1;
							if(print_all)
							{
								printf("\nproblem_MPI.c		init_work()		saved resolved:");
								for(int i=0;i<alg.ppd.aproblem.numTask;i++)
								{
									printf("\n 		%d",resolved->resources[i]);
								}
							}
						}

					}
					else//if init resolved !=0
					{
						if(print_all)
						{
							printf("\nproblem_MPI.c		init_work()		init_resolved=1 so we compare");
						}
						if(alg.num_solved>0)
						{
							//compare
							if(alg.solvedProblems[0].solution.acum>resolved->value)//new resolved is better
							{
								if(print_all)
								{
									printf("\nproblem_MPI.c		init_work()		new resolved is better");
								}
								resolved->num_resolved=alg.num_solved;
								resolved->value=alg.solvedProblems[0].solution.acum;
								for(int i=0;i<alg.ppd.aproblem.numTask;i++)
								{
									resolved->resources[i]=alg.solvedProblems[0].solution.resources[i].position;
								}

							 }

						 }
					 }
					delete_algorithmPD(&alg);
					init_work(
							pa,
							0,
							alternatives,
							buffer,
							request_b,
							buffer_w,
							best,
							resolved,
							search);
				}
				else
				{
					if(print_all)
					{
						printf("\nproblem_MPI.c		no rr_all");
					}
					send_resolved(&alg);
					delete_algorithmPD(&alg);
					if(print_all)
					{
						printf("\nSEND RESOLVED TO MASTER");
					}
				}


			}
		}//end redistribution rr

	}//end num alternatives==0
	else//more than 0 alternatives
	{

		MPI_Irecv(&buffer_work,1,MPI_INT,master,tag_give_work,MPI_COMM_WORLD, &request_work);
		if(print_all)
		{
			printf("\nproblem_MPI.c		init_work()		p%d more than 0 alt",id);
		}
		if(num_alternatives==1)
		{
			if(print_all)
			{
				printf("\nproblem_MPI.c		init_work()	===========p%d only 1 alternative",id);
			}

			alg.ppd.index=1;
			strcpy(alg.ppd.solution.resources[alg.ppd.solution.lengthArrays].name, pa->resources[alternatives[0]].name);
			alg.ppd.solution.resources[alg.ppd.solution.lengthArrays].position=alternatives[0];

			alg.ppd.solution.lengthArrays=alg.ppd.solution.lengthArrays+1;
			alg.ppd.solution.acum=pa->values[0+alternatives[0]*alg.ppd.aproblem.numTask];
			alg.problems[0]=alg.ppd;
			if(print_all)
			{
				printf("Assert index =1 in problems post work:%d ",alg.problems[0].index);
			}

		}
		else//num_alternatives>1
		{
			// we need to add subproblems
			//int get_subproblem(const PAproblemPD, PAproblemPD,Alternative, int);
			//int init_alternative(PAlternative a,const int i)
			AproblemPD subproblems[num_alternatives];
			Alternative alt[num_alternatives];

			for(int i=0;i<num_alternatives;i++)
			{
				alg.ppd.index=0;
				init_alternative(&alt[i],alternatives[i]);
				if(print_all)
				{
					printf("\nproblem_MPI.c		init_work()	===============p%d Alternative: %d\n",id,alt[i].indexResource);
					printf("\n PPD index: %d\n",alg.ppd.index);
				}

				get_subproblem(&alg.ppd, &subproblems[i],alt[i], get_num_subproblems());
				if(print_all)
				{
					printf("\nproblem_MPI.c		init_work() "
							"========================p%d Inside init work more than 1 alternative",id);
					show_aproblem_PD(&subproblems[i]);
					printf("\n post get subproblems index: %d",subproblems[i].index);
					printf("\n post get subproblems solution.len: %d",subproblems[i].solution.lengthArrays);
					printf("\n post get subproblems solution.acum: %f",subproblems[i].solution.acum);
					printf("\n post get subproblems first resource: %s",subproblems[i].solution.resources[0].name);
				}

				alg.problems[i]=subproblems[i];
				if(print_all)
				{
					printf("\n post get subproblems p%d",id);
					printf("\n post get subproblems in alg solution.len: %d",alg.problems[i].solution.lengthArrays);
					printf("\n post get subproblems in alg solution.acum: %f",alg.problems[i].solution.acum);
					printf("\n post get subproblems in algfirst resource: %s",alg.problems[i].solution.resources[0].name);

				}

				if(i>0)
				{
					if(print_all)
					{
						printf("\nProblem i-1:\n");
						show_aproblem_PD(&(alg.problems[i-1]));
					}

				}
				//adding alt to solution
				strcpy(alg.problems[i].solution.resources[alg.problems[i].solution.lengthArrays].name, pa->resources[alternatives[i]].name);
				alg.problems[i].solution.resources[alg.problems[i].solution.lengthArrays].position=pa->resources[alternatives[i]].position;
				if(print_all)
				{
					printf("\nproblem_MPI.c	init_work() p%d. len solution %d ccccccccccccccccccccccccccccc",id,alg.problems[i].solution.lengthArrays);
					printf("\nacum solution %f ccccccccccccccccccccccccccccc",alg.problems[i].solution.acum);
					printf("Assert index =1 in problems post work:%d \n",alg.problems[i].index);

				}

			}
			alg.ppd.index=1;
			alg.num_problems=num_alternatives;//TODO
		}
		if(print_all)
		{
			for(int i=0;i<alg.num_problems;i++){
				show_aproblem_PD(&(alg.problems[i]));
			}
			printf("\nBefore exec algoritm\n");
		}

		exec_algorithm(&alg,buffer,request_b, buffer_w, &request_work,fs);
		if(print_all)
		{
			printf("\nproblem_MPI.c	init_work()	p%d After exec algoritm\n",id);
		}
		Solution sol;
		get_PDsolution(&alg, &sol);

		if(print_all)
		{
			for(int i=0;i<alg.ppd.solution.lengthArrays;i++){
				printf("\nResources: \n*%s\n",alg.ppd.solution.resources[i].name);
			}
			printf("Solution value: %f", alg.ppd.solution.acum);
		}
		if(redistribution_rr_all)
		{
			//select resolved
			if(init_resolved==0)
			{
				if(print_all)
				{
					printf("\nproblem_MPI.c		init_work()		rrall and init resolved==0");
				}
				//init resolved
				if(alg.num_solved>0)
				{
					resolved->num_resolved=alg.num_solved;
					resolved->value=alg.solvedProblems[0].solution.acum;
					for(int i=0;i<alg.ppd.aproblem.numTask;i++)
					{
						resolved->resources[i]=alg.solvedProblems[0].solution.resources[i].position;
						if(print_all)
						{
							printf("\nproblem_MPI.c		init_work()		resource: %d",resolved->resources[i]);
						}
					}
					init_resolved=1;
				}

			}
			else{
				if(alg.num_solved>0){
					//compare
					if(print_all)
					{
						printf("\nproblem_MPI.c		init_work()		rrall and init resolved>0 so compare");
					}
					if(alg.solvedProblems[0].solution.acum>resolved->value)//new resolved is better
					{
						if(print_all)
						{
							printf("\nproblem_MPI.c		init_work()		new resolved %d is better %d",alg.solvedProblems[0].solution.acum,resolved->value);
						}

						resolved->num_resolved=alg.num_solved;
						resolved->value=alg.solvedProblems[0].solution.acum;
						for(int i=0;i<alg.ppd.aproblem.numTask;i++)
						{
							resolved->resources[i]=alg.solvedProblems[0].solution.resources[i].position;
						}

					}

				}
			}

			delete_algorithmPD(&alg);
			init_work(
					pa,
					0,
					alternatives,
					buffer,
					request_b,
					buffer_w,
					best,
					resolved,
					search);
		}
		else
		{
			send_resolved(&alg);
		}

		if(print_all)
		{
			printf("\nSEND RESOLVED TO MASTER");
		}


	}//end else (alternative>0)
	if(print_all)
	{
		printf("\nproblem_MPI.c		outgoing initwork");
	}

	return res;
}
int send_work(
		const PalgorithmPD palg,
		int *alternatives,
		int num_alternatives,
		int num_process,
		int prune,
		int r_rr,//redistribution round robin
		int tuple_prune,
		int first_search,
		int r_rr_all,
		double best

		)
{
	MPE_Log_event(event1a, 0, "start send");
	if(print_all)
	{
		printf("\nproblem_MPI.c		send_work()		sending  %d  alternatives to process %d\n",num_alternatives,num_process);
		show_aproblem(&(palg->ppd.aproblem));
	}


	int res=0;

	//process 0 resolves 0. Var for Not blocking solution
	MPI_Request request;
	MPI_Status  status;
	int request_complete = 0;

	if(num_process==0){
		return 0;
	}

	struct Work work;
	char serialized_tasks[MAX_TAM_STRING];
	int len=serializer_tasks(palg, &serialized_tasks);
	char serialized_resources[MAX_TAM_STRING];

	int len_resources=serializer_resources(palg, &serialized_resources);

	work.num_resources=palg->ppd.aproblem.numResources;
	work.num_tasks=palg->ppd.aproblem.numTask;
	work.num_alternatives=num_alternatives;//TODO
	work.type=palg->ppd.aproblem.type;
	work.prune=prune;
	work.tuple_prune=tuple_prune;
	work.redistribution=r_rr;
	work.first_search=first_search;
	work.rrall=r_rr_all;
	work.best=best;
	int num_values=work.num_resources*work.num_tasks;
	if(print_all)
	{
		printf("\n In work task: %d",work.num_tasks);
		printf("\n In work resources: %d",work.num_resources);
	}


	double values[num_values];
	int alternatives_to_work[num_alternatives];
	Cadena tasks[num_values];
	Cadena resources[num_values];

	for(int i=0;i<num_values;i++)
	{
		values[i]=palg->ppd.aproblem.values[i];
		if(print_all)
		{
			printf("inside send. value[%d]=%f\n",i,values[i]);
		}

	}
	for(int i=0;i<num_alternatives;i++)
	{
		alternatives_to_work[i]=alternatives[i];//TODO
		if(print_all)
		{
			printf("inside send. alternative[%d]=%d\n",i,alternatives[i]);
		}

	}
	for(int i=0;i<work.num_resources;i++)
	{
		strcpy(resources[i],palg->ppd.aproblem.resources[i].name);
		if(print_all)
		{
			printf("inside send. resources[%d]=%s\n",i,resources[i]);
		}

	}
	for(int i=0;i<work.num_tasks;i++)
	{
		strcpy(tasks[i],palg->ppd.aproblem.tasks[i].name);
		if(print_all)
		{
			printf("inside send. resources[%d]=%s\n",i,tasks[i]);
		}

	}

	//sending
	MPI_Datatype work_mpi_datatype;
	int blocklengths[10] = {1,1,1,1,1,1,1,1,1,1};
	MPI_Datatype types[10] = {MPI_INT, MPI_INT,MPI_INT,MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT,MPI_INT,MPI_DOUBLE};
    const MPI_Aint offsets[10]= { 0, sizeof(int),sizeof(int)*2, sizeof(int)*3, sizeof(int)*4,sizeof(int)*5,sizeof(int)*6,sizeof(int)*7,sizeof(int)*8,sizeof(int)*9};
	MPI_Type_create_struct(10, blocklengths, offsets, types,  &work_mpi_datatype);
	MPI_Type_commit ( &work_mpi_datatype);

	if(print_all)
	{
		printf("\nproblem_MPI.c		send_work()		  sending num task: %d",work.num_tasks);
		printf("\nproblem_MPI.c		send_work()		  sending num resources: %d\n",work.num_resources);
		printf("\nproblem_MPI.c		send_work()		  sending type: %d\n",work.type);
		printf("\nproblem_MPI.c		send_work()		  sending prune: %d\n",work.prune);
		printf("\nproblem_MPI.c		send_work()		  sending tuple_prune: %d\n",work.tuple_prune);
		printf("\nproblem_MPI.c		send_work()		  sending first_search: %d\n",work.first_search);
		printf("\nproblem_MPI.c		send_work()		  sending rr_all: %d\n",work.rrall);
		printf("\nproblem_MPI.c		send_work()		  sending alternatives: %d\n",work.num_alternatives);
		printf("\nproblem_MPI.c		send_work()		  sending best: %f\n",work.best);
	}

    MPI_Send(&work, 1, work_mpi_datatype, num_process, tag_work, MPI_COMM_WORLD);
    MPI_Type_free( &work_mpi_datatype);
	//sending dinamic arrays
	MPI_Send( &values, num_values, MPI_DOUBLE, num_process, tag_values, MPI_COMM_WORLD );
	MPI_Send( &serialized_tasks, len, MPI_CHAR, num_process, tag_tasks, MPI_COMM_WORLD );
	MPI_Send( &serialized_resources, len_resources, MPI_CHAR, num_process, tag_resources, MPI_COMM_WORLD );

	if(num_alternatives>0)
    {
    	MPI_Send( &alternatives_to_work, num_alternatives, MPI_INT, num_process, tag_alternatives, MPI_COMM_WORLD );
    }
	MPE_Log_event(event1b, 0, "end send");
	return res;
}

int send_resolved(const PalgorithmPD palg)
{
	MPE_Log_event(event5a, 0, "start send resolved");
	int master=0;//to master
	int myid;
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	if(print_all)
	{
		printf("\nPROCESS %d SENDING RESOLVED TO MASTER",myid);
	}


	Resolved resolved;
	int num_resolved=palg->num_solved;
	int numT=palg->ppd.aproblem.numTask;
	if(print_all)
	{
		printf("\nproblem_MPI.c		send_resolved()	----------NUM SOLVED %d",num_resolved);
		printf("\nproblem_MPI.c		send_resolved()	----------NUM TASK in palg ppd %d",numT);
	}
	if(num_resolved==0)
	{
		if(print_all)
		{
			printf("\nproblem_MPI.c		send_resolved()	 SENDING NO SOLUTION");
		}

	}
	else
	{
		num_resolved=1;//TODO select num resolved to see (in pd limit 10 array but not num resolved)
		resolved.value=palg->solvedProblems[0].solution.acum;
		for(int i=0;i<num_resolved;i++){
			if(print_all)
			{
				printf("\nproblem_MPI.c		send_resolved()	----------RESOLVED %d",i);
				}
			for(int j=0;j<numT;j++){
				resolved.resources[i*numT+j]=palg->solvedProblems[i].solution.resources[j].position;
				if(print_all)
				{
					printf("\nproblem_MPI.c		send_resolved()		resource%d:%d",i*numT+j,palg->solvedProblems[i].solution.resources[j].position);
					printf("\nproblem_MPI.c		send_resolved()		send%d:%d",i*numT+j,resolved.resources[i*numT+j]);

				}
			}

		}

	}
	resolved.num_resolved=num_resolved;
	if(print_all)
	{
		printf("\nproblem_MPI.c		send_resolved()			SEND");
	}

	MPI_Datatype resolved_mpi_datatype;
	int blocklengths[3] = {1,1,100};
	MPI_Datatype types[3] = {MPI_DOUBLE,MPI_DOUBLE,MPI_INT};
	const MPI_Aint offsets[3]= { 0, sizeof(double),sizeof(double)*2};

	MPI_Type_create_struct(3, blocklengths, offsets, types,  &resolved_mpi_datatype);
	MPI_Type_commit ( &resolved_mpi_datatype);
	if(print_all)
	{
		printf("\nproblem_MPI.c		send_resolved()			pre send mpi");
	}

	MPE_Init_log();

	MPI_Send( &resolved, 1, resolved_mpi_datatype, master, tag_resolved, MPI_COMM_WORLD );
	if(print_all)
	{
		printf("\nproblem_MPI.c		send_resolved()			post send mpi %f",resolved.value);
	}

	MPI_Type_free( &resolved_mpi_datatype);
	MPE_Log_event(event5b, 0, "end send resolved");

//	free(transfered.receivers);
//	free(transfered.transfered);
//	free(tuple_prune_data.solution_tuples);
//	free(tuple_prune_data.solution_values);
	return 0;
}
int send_resolved_data(int num_resolved, double acum,int num_tasks, int position[])
{
	//MPE_Log_event(event5a, 0, "start send resolved");
	int master=0;//to master
	int myid;
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	if(print_all)
	{
		printf("\nPROCESS %d SENDING RESOLVED data TO MASTER. first position:%d",myid,*position);
	}


	Resolved resolved;

	if(print_all)
	{
		printf("\nproblem_MPI.c		send_resolved_data()	----------NUM SOLVED %d",num_resolved);

	}
	if(num_resolved==0)
	{
		if(print_all)
		{
			printf("\nproblem_MPI.c		send_resolved_data()	 SENDING NO SOLUTION");
		}

	}
	else
	{
		num_resolved=1;//TODO select num resolved to see (in pd limit 10 array but not num resolved)
		resolved.value=acum;
		for(int i=0;i<num_resolved;i++){
			if(print_all)
			{
				printf("\nproblem_MPI.c		send_resolved()	----------RESOLVED num %d",i+1);
				}
			for(int j=0;j<num_tasks;j++){
				resolved.resources[i*num_tasks+j]=position[j];
				if(print_all)
				{

					printf("\nproblem_MPI.c		send_resolved()		send%d:%d",i*num_tasks+j,resolved.resources[i*num_tasks+j]);

				}
			}

		}

	}
	resolved.num_resolved=num_resolved;
	if(print_all)
	{
		printf("\nproblem_MPI.c		send_resolved()			SEND num res %d",resolved.num_resolved);
	}

	MPI_Datatype resolved_mpi_datatype;
	int blocklengths[3] = {1,1,100};
	MPI_Datatype types[3] = {MPI_DOUBLE,MPI_DOUBLE,MPI_INT};
	const MPI_Aint offsets[3]= { 0, sizeof(double),sizeof(double)*2};

	MPI_Type_create_struct(3, blocklengths, offsets, types,  &resolved_mpi_datatype);
	MPI_Type_commit ( &resolved_mpi_datatype);
	if(print_all)
	{
		printf("\nproblem_MPI.c		send_resolved()			pre send mpi");
	}

	//MPE_Init_log();

	MPI_Send( &resolved, 1, resolved_mpi_datatype, master, tag_resolved, MPI_COMM_WORLD );
	if(print_all)
	{
		printf("\nproblem_MPI.c		send_resolved()			post send mpi %f",resolved.value);
	}

	MPI_Type_free( &resolved_mpi_datatype);
	//MPE_Log_event(event5b, 0, "end send resolved");
	return 0;
}
int rcv_best(double rcvd_best, MPI_Request *request_bcast)
{
	if((rcvd_best>final_alg.best && final_alg.ppd.aproblem.type==MAX) ||
			(rcvd_best<final_alg.best && final_alg.ppd.aproblem.type==MIN))
	{
		final_alg.best=rcvd_best;
		MPE_Log_event(event5, 0, "start broadcast");
		broadcast_best(rcvd_best);
		if(print_all)
		{
			printf("\nproblem_MPI.c		rcv_best()		BETTER %f",rcvd_best);
		}

	}
	return 0;
}
int send_best(PalgorithmPD palg)
{
	int res=0;
	if(numprocs>1)
	{

		double best=palg->best;
		int count = 1;
		MPE_Log_event(event3a, 0, "start send best");
		MPI_Request request_b;
		if(print_all)
		{
			printf("\n sending best: %f to %d",best,master);
		}

		MPI_Isend(&best, count, MPI_DOUBLE, master, tag_best, MPI_COMM_WORLD, &request_b);
		if(print_all)
		{
			printf("\n send best: %f to %d",best,master);
		}

		MPE_Log_event(event3b, 0, "end send best");
	}

	return res;
}
int broadcast_best(double better){
	if(print_all)
	{
		printf("\nproblem_MPI.c		broadcast_best()		better IN %f",better);
	}
	int res=0;
	double best_to_broadcast;
	int count = 1;
	int rank;
	MPE_Log_event(event5, 0, "broadcast");
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if(rank==master){
		best_to_broadcast=better;
		if(print_all){
			printf("\n I am %d, and the best to broadcast is %f",rank, best_to_broadcast);
		}

		MPE_Log_event(event5, 0, "broadcast best");
		//TODO control buffer free
		MPI_Ibcast(&best_to_broadcast, count, MPI_DOUBLE, master, world, &request_b);
	}

	return res;
}
int init_waiting_best(double * buffer, MPI_Request * request_b)
{
//	MPI_Ibcast(buffer,1,MPI_DOUBLE,0,world, request_b);
	return 0;
}
void waiting_best(double * buffer, MPI_Request* request_b)
{
	if(print_all)
	{
		printf("\nproblem_MPI.c		waiting_best()");
	}
	int ready=0;
	double d;
	MPI_Test(request_b, &ready, MPI_STATUS_IGNORE);
	if(ready)
	{
		if(print_all)
		{
			printf("\nproblem_MPI.c		waiting_best()		Master has send best result %f", *buffer);
		}
//				if((buffer>palg->best && palg->ppd.aproblem.type==MAX) ||
//						(buffer<palg->best && palg->ppd.aproblem.type==MIN)){
//					palg->best=buffer;
//				}
				if((*buffer>final_alg.best && final_alg.ppd.aproblem.type==MAX) ||
						(*buffer>final_alg.best && final_alg.ppd.aproblem.type==MIN))
				{
					if(print_all)
					{
						printf("\nproblem_MPI.c		waiting_best()		buffer rcv better");
					}
					final_alg.best=*buffer;
				}
		MPE_Log_event(event5, 0, "broadcast best");
		MPI_Ibcast(buffer, 1, MPI_DOUBLE, master, world, request_b);//TODO d

	}
}
int sending_my_work(int recved,PalgorithmPD palg,int m)
{
	Node_work to_send;
	MPI_Request r;

	to_send.index=palg->problems[m].index;
	to_send.value=palg->problems[m].solution.acum;
	for(int i=0;i<to_send.index;i++)
	{
		to_send.solution[i]=palg->problems[m].solution.resources[i].position;
	}

	MPI_Datatype node_mpi_datatype;
	int blocklengths[3] = {1,1,100};
	MPI_Datatype types[3] = {MPI_INT, MPI_DOUBLE,MPI_INT};
	const MPI_Aint offsets[3]= { 0, sizeof(int),sizeof(double)+sizeof(int)};
	MPI_Type_create_struct(3, blocklengths, offsets, types,  &node_mpi_datatype);
	MPI_Type_commit ( &node_mpi_datatype);

	if(print_all)
	{
		printf("\nproblem_MPI.c		sending_my_work()		index: %d",to_send.index);
		for(int i=0;i<to_send.index;i++)
		{
			printf("\nproblem_MPI.c		sending_my_work()		solution.resources[%d]= %d",i, to_send.solution[i]);
		}

	}

	MPI_Isend(&to_send, 1, node_mpi_datatype, recved, tag_redistribution+recved, MPI_COMM_WORLD,&r);
	int ready=0;
	while (!ready){
		MPI_Test(&r,&ready,MPI_STATUS_IGNORE);
	}
    MPI_Type_free( &node_mpi_datatype);

	return 0;
}
int waiting_confirming(Transfered_nodes * transf)
{
	int res=0;
	int n;
	int ready;
	MPI_Status s;
	MPI_Request r;


	if(print_all)
	{
		printf("\nproblem_MPI.c		waiting_confirming() leng transfered: %d",transf->len_transfered);
	}
	for(int i=0;i<transf->len_transfered;i++)
	{
		int tag=tag_redistribution+(transf->receivers[i]);
		int sender=transf->receivers[i];
		if(print_all)
		{
			printf("\nproblem_MPI.c		waiting_confirming()		sender and tag: %d %d",sender,tag);
		}
		MPI_Iprobe(sender,tag,MPI_COMM_WORLD,&ready,&s);
		if(ready)
		{
			MPI_Irecv(&n,1,MPI_INT,sender,tag,MPI_COMM_WORLD, &r);
			if(print_all)
			{
				printf("\nproblem_MPI.c		waiting_confirming()		iprobe ready. Transf previous:");
				show_transfered(transf);
			}
			MPE_Log_event(event7, 0, "confirmed");
			delete_transfered(transf,sender);
			if(print_all)
			{
				printf("\nproblem_MPI.c		waiting_confirming()		iprobe ready. Transf POST. Sender %d:",sender);
				show_transfered(transf);
			}
			ready=0;
		}
	}

	return res;
}


int waiting_petition(int * buffer_w, MPI_Request* r_w,PalgorithmPD palg,int m, int * rcvd)
{
	int res=0;
	MPI_Status s;
	int ready=0;
	//sleep(2);
	MPI_Test(r_w, &ready, &s);
	if(print_all)
	{
		printf("\nproblem_MPI.c		waiting_petition() ready=%d",ready);
	}
	if(ready && buffer_work!=0)//TODO work around
	{
		MPE_Log_event(event8a, 0, "rcv petition work");
		if(print_all)
		{

			printf("\nproblem_MPI.c		waiting_petition()		Ready. Master has send work petition from %d",buffer_work);
			//printf("\nproblem_MPI.c		waiting_petition()		error %d, source %d, tag %d, hi %d, lo %d",s.MPI_ERROR,s.MPI_SOURCE,s.MPI_TAG,s.count_hi_and_cancelled,s.count_lo);
		}
		int rcved=buffer_work;
		*rcvd=rcved;
		sending_my_work(rcved,palg,m);
		MPI_Irecv(&buffer_work,1,MPI_INT,master,tag_give_work,MPI_COMM_WORLD, r_w);
		MPE_Log_event(event8b, 0, "rcv petition work");
		res=1;
	}
	return res;
}

int scan_petition(MPI_Request *request_ask_work, MPI_Request *request_best, MPI_Request *request_bcast)
{
	//MPE_Log_event(event2, 0, "listen");
	MPI_Status status;
	MPI_Status status_best;
	int flag=0;
	int flag_b=0;
	int sender;
//	printf("\n 				PRE TEST");
	MPI_Test(request_best, &flag_b, &status_best);
	if(flag_b)
	{
		sender=status_best.MPI_SOURCE;
		if(print_all)
		{
			printf("\nproblem_MPI.c		scan_petition()			SENDER %d Best %f", sender, b);
		}

		MPE_Log_event(event4, 0, "rcve best");
		double new_best=b;
		rcv_best(new_best, request_bcast);
		MPI_Irecv(&b, 1, MPI_DOUBLE, MPI_ANY_SOURCE, tag_best, MPI_COMM_WORLD, request_best);
		flag_b=0;
	}
	MPI_Test(request_ask_work, &flag, &status);
	if(flag)
	{
		sender=status.MPI_SOURCE;
		if(print_all)
		{
			printf("\n 				SENDER PETITION %d\n"
					"				PETITION %d", sender, n);
		}

		MPE_Log_event(event3, 0, "rcve petition");

		send_more_work(sender);
		MPI_Irecv(&n, 1, MPI_INT, MPI_ANY_SOURCE, tag_ask_work, MPI_COMM_WORLD, request_ask_work);

	}



	return n;
}


void init_best(MPI_Request * request_b, MPI_Comm * world, int np){
	  ////////////////////
	  best=final_alg.best;

	  if(print_all)
	  {
		  int myid;
		 MPI_Comm_rank(MPI_COMM_WORLD,&myid);
		 printf("\n problem_MPI.c		init best()-------------------%f",best);
	 	 printf( "\nproblem_MPI.c		init best()		Message from process %d : best %f\n", myid, best);
	  }
	  if(np>1){//no serial
		  MPI_Ibcast(&best,1,MPI_DOUBLE,0,*world, request_b);
	  }

}

int init_listening(MPI_Request *request_petition,MPI_Request *request_best )
{

	MPI_Irecv(&n, 1, MPI_INT, MPI_ANY_SOURCE, tag_ask_work, MPI_COMM_WORLD, request_petition);
	MPI_Irecv(&b, 1, MPI_DOUBLE, MPI_ANY_SOURCE, tag_best, MPI_COMM_WORLD, request_best);
	return 0;
}

int finish_work()
{
	rcv_resolved();
	//TODO
	if(print_all)
	{
		printf("\n END OF FINISH WORK ON MASTER\n ");
		printf("\n BEST: %f\n",final_alg.best);
	}

	return 0;
}

int serializer_tasks(PalgorithmPD palg, char* all)
{
	Cadena temp="";
	Cadena divisor=";";

	for(int i=0;i<palg->ppd.aproblem.numTask;i++)
	{
		strcat(temp,palg->ppd.aproblem.tasks[i].name);
		strcat(temp,divisor);
		if(print_all)
		{
			printf("\n----%s\n",temp);
		}

	}
	strcpy(all,temp);

	int len;
	for (len = 0; all[len] != '\0'; ++len);
	if(print_all)
	{
		printf("\n---->>>>%s\n",all);
		printf("Length of Str tasks is %d", len);
	}


	return len;
}
int deserializer_tasks(char* all, int len, PTask tasks)
{

	Cadena aux="";
	char divisor=';';
	if(print_all)
	{
		printf("\nINSIDE DESERIALIZED---->>>>%s\n",all);
	}

	int res=0;
	int count=0;
	for(int i=0;i<1000;i++)//TODO
	{
		if(all[i]==divisor)
		{
			strcpy(tasks[count].name,aux);
			count++;
			strcpy(aux,"");
			if(count==len)
			{
				if(print_all)
				{
					printf("\nfound %d *\n",count);
				}

				break;
			}

		}
		else{
			strncat(aux, &all[i], 1);

		}

	}
	return res;
}
int serializer_resources(PalgorithmPD palg, char* all)
{
	Cadena temp="";
	Cadena divisor=";";

	for(int i=0;i<palg->ppd.aproblem.numResources;i++)
	{
		if(print_all)
		{
			printf("\n*************************************************************************"
							"name resource %d in serializer: %s",i,palg->ppd.aproblem.resources[i].name);

		}
		strcat(temp,palg->ppd.aproblem.resources[i].name);
		strcat(temp,divisor);
	}
	strcpy(all,temp);
	int len;
	for (len = 0; all[len] != '\0'; ++len);
	if(print_all)
	{
		printf("\ncopied RESOURCES---->>>>%s\n",all);
		printf("Length of Str resources is %d", len);
	}

	return len;
}
int deserializer_resources(char* all, int len, PResource resources)
{

	Cadena aux="";
	char divisor=';';
	if(print_all)
	{
		printf("\nINSIDE DESERIALIZED  resources---->>>>%s\n",all);
	}

	int res=0;
	int count=0;
	for(int i=0;i<1000;i++)//TODO
	{
		if(all[i]==divisor)
		{
			strcpy(resources[count].name,aux);
			count++;
			strcpy(aux,"");
			if(count==len)
			{
				if(print_all)
				{
					printf("\nfound resources %d *\n",count);
				}

				break;
			}

		}
		else{
			strncat(aux, &all[i], 1);

		}

	}
	return res;
}
void waitting_answer()
{

}

int pD_distribution(PalgorithmPD palg)
{
	 	  if(print_all)
	 	  {
	 		  printf("\nproblem_MPI.c		pD_distribution()");
	 	  }
		  int res=0;
		  AproblemPD appd=palg->ppd;
		  int lengthNewArrayAppd=0;
		  int max_num_problem;
		  if(fs)
		  {
			  max_num_problem=get_max_num_problems_deep(&appd);
		  }
		  else
		  {
			  max_num_problem=get_max_num_problems(&appd);
		  }

	 	  if(print_all)
	 	  {
	 		 printf("\n**%d",fs);
	 		 printf("\nproblem_MPI.c		pD_distribution()		max num %d",max_num_problem);
	 	  }
//	 	 int memoryArrayValues=sizeof(double)*numTasks*numResources;
//	 	 pa->values=(double*)malloc(memoryArrayValues);
	 	 newArrayAppd = (AproblemPD*)malloc(max_num_problem * sizeof(AproblemPD));
	 	 problems= (AproblemPD*)malloc(max_num_problem * sizeof(AproblemPD));
//	 	  AproblemPD newArrayAppd[max_num_problem];
//		  AproblemPD problems[max_num_problem];
		  int numPreviousProblems=getPreviousProblems(palg, problems);
		  if(print_all)
		 	  {
		 		  printf("\nproblem_MPI.c		pD_distribution()		num previous problems %d",numPreviousProblems);
		 	  }
		  Alternative * as;
		  init_alternative_array(&as,get_max_num_alternatives(&appd));
		  //as=(Alternative*)malloc(sizeof(Alternative)*50);//TODO
		  int numAlternatives=get_alternatives(&appd, as);
		  //delete_alternatives(&as);
		  if(numAlternatives==0)
		  {
				  //TODO
				  printf("\n no alternatives\n");
		  }
		  else
		  {
		 	  if(print_all)
		 	  {
		 		  printf("\nproblem_MPI.c		pD_distribution()		num alt %d",numAlternatives);
		 	  }


			  if(is_base_case(&problems[0]))
			  {
				  //TODO
				  				  printf("\n too little\n");
			  }
			  else
			  {
				  //get new problems
				  if(print_all)
				  {
					  printf("        Alternatives: ");
					  for(int k=0;k<numAlternatives;k++)
					  {
						  printf("%d ", as[k].indexResource);
					  }
					  printf("\n");

				  }

				  randomize(palg,as);//not using
				  Logico ismin;
				  Logico ismax;
				  double w_estimated;

				   /////////looking for the first best
				  if(type_best==1 || type_best==0)//dummy best
				  {
					  w_estimated=get_worst_estimate(&problems[0]);
				  }
				  else if(type_best==2)//diagonal best
				  {
					  w_estimated=best_diagonal(&(problems[0].aproblem));
				  }
				  else if(type_best==3)//greedy
				  {
					  w_estimated=greedy(&(problems[0].aproblem));
					  if(print_all)
					  {
						  printf("\nproblem_MPI.c		pD_distribution()		w estimated=%f",w_estimated);
					  }
				  }
				  if((w_estimated>final_alg.best && ismax) ||
				 								  (w_estimated<final_alg.best && ismin)  )
				  {
					  final_alg.best=w_estimated;
					  if(print_all)
					  {
						  printf("\nproblem_MPI.c		pD_distribution()		final_alg best is now=%f",final_alg.best);
					  }
				  }

				  for(int u=0;u<numAlternatives;u++)
				  {
					  //prune
					  ismin=is_min(palg);
					  ismax=is_max(palg);
					  double b_estimated;
					  b_estimated=get_best_estimate(&problems[0]);

					  if((ismin && b_estimated<=palg->best)
										  || (ismax && b_estimated>=palg->best))
					  {

						  int numSubproblems=get_num_subproblems();
						  AproblemPD appdNew;
						  for(int j=0;j<numSubproblems;j++)
						  {
							  initAProblemPD(&appdNew,&(palg->ppd.aproblem));

							  get_subproblem(&problems[0], &appdNew, as[u],numSubproblems);
							  if(print_all)
							  {
								  printf("\nproblem_MPI.c		pD_distribution()     is NOT base case: last appdNew sol: %s\n",appdNew.solution.resources[appdNew.solution.lengthArrays-1].name);
								  printf("\nproblem_MPI.c		pD_distribution()     i=%d of %d alternatives\n",u, numAlternatives);

							  }
							   //if problem//TODO
									  //add problem to new array
							  copy_aproblem_PD( &(newArrayAppd[lengthNewArrayAppd]),appdNew);
							 lengthNewArrayAppd++;

						  }//end for num subproblem=1
					  }//end if not prune
					  else
					  {
						  MPE_Log_event(event6, 0, "prune in distribution");
					  }
				   }//end for alternatives
			  }//end else (not base case)
			}//end else (exits alternative)

		  for(int w=0; w<lengthNewArrayAppd;w++)
		  {
			  palg->problems[w]=newArrayAppd[w];
			  if(print_all)
			  {
				  printf("\nproblem_MPI.c		pD_distribution()----------------------- ");
				  printf("\nproblem_MPI.c		pD_distribution() problem %d of %d problems in alg",w,lengthNewArrayAppd);
				  show_aproblem_PD(&(palg->problems[w]));
			  }


		  }
		  palg->num_problems=lengthNewArrayAppd;
		  if(print_all)
		  {
			  printf("\nproblem_MPI.c		pD_distribution() finish ");
		  }
//		  free(newArrayAppd);
//		  free(problems);

  return res;
}
