/*
 * PD_algorithm.c
 *
 *  Created on: 21 dic. 2020
 *      Author: practica
 */
#include "PD_algorithm.h"

  Logico is_min(const PalgorithmPD palg)
  {
	  Logico res=FALSE;
	  if(palg->ppd.aproblem.type==MIN)
	  {
		  res=TRUE;
	  }
	  return res;
  }
  Logico is_max(const PalgorithmPD palg)
  {
	  Logico res=FALSE;
	  if(palg->ppd.aproblem.type==MAX)
	  {
		  res=TRUE;
	  }
	  return res;
  }
  int init_algorithmPD(PalgorithmPD palg,AproblemPD ap)
  {
	  int res=0;
	  palg->isRandomize=FALSE;//we are not using
	  palg->sizeRef=100;//we are not using
	  if(is_min(palg))
	  {
		  palg->best=999999.;//TODO
	  }
	  else{
		  palg->best=-999999.;//TODO
	  }
	  int num=get_max_num_problems(&(ap));
	  palg->problems=(AproblemPD*)malloc(sizeof(AproblemPD)*num);

	  palg->ppd=ap;
	  copy_aproblem_PD(&(palg->problems[0]),ap);

	  palg->num_problems=1;//TODO anothers alternatives

	  //palg->problems[0]=palg->ppd;
	  palg->num_solved=0;

	  palg->solvedProblems=(AproblemPD*)malloc(sizeof(AproblemPD)*num);
	  return res;
  }

  int exec_algorithm(PalgorithmPD palg)
  {
	  int res=0;
	  Solution sol;
	  do
	  {
		  for(int i=0;i<get_num_subproblems();i++)
		  {
			  pD(palg);
		  }
	  }while(palg->isRandomize && get_PDsolution(palg,&sol)!=0);
	  return res;
  }

  int randomize(PalgorithmPD palg,PAlternative as)
  {
	  int res=0;
	  if(palg->isRandomize && get_size(&(palg->ppd))>palg->sizeRef)
	  {
		  //TODO change as to new smaller random array of alternatives
	  }
	  return res;
  }

  int update_best(PalgorithmPD palg)
  {
	  int res=0;
	  double target=get_target(&(palg->ppd));
	  if(is_min(palg) && target<palg->best)
	  {
		  palg->best=get_target(&(palg->ppd));
	  }
	  else if(is_max(palg) && target>palg->best)
	  {
		  palg->best=get_target(&(palg->ppd));
	  }
	  return res;
  }

  int get_PDsolution(PalgorithmPD palg, PSolution psol)
  {
	  int res=0;

	  if(palg->num_solved==0){
		  printf("there is not solution");
	  }
	  else if(palg->num_solved==1){
		  palg->ppd=palg->solvedProblems[0];
		  printf("there is only one solution");
	  }else{
		  for(int i=0;i<palg->num_solved;i++){
			  	  if(palg->solvedProblems[i].solution.acum>=palg->best){
			  		palg->ppd=palg->solvedProblems[i];
			  		palg->best=palg->solvedProblems[i].solution.acum;//TODO
			  	  }
			  }
		  printf("Selected one of the best solutions");//TODO
	  }

	  return res;;
  }

  int pD(PalgorithmPD palg)
  {
	  int res=0;
	  AproblemPD appd=palg->ppd;
	  int size=get_size(&(palg->ppd));

	  int lengthNewArrayAppd=0;
	  for(int i=0;i<size;i++){
		  AproblemPD newArrayAppd[get_max_num_problems(&appd)];
		  lengthNewArrayAppd=0;
		  //get previous problems
		  AproblemPD problems[get_max_num_problems(&appd)];
		  int numPreviousProblems=getPreviousProblems(palg, problems);
		  //if problems
		  if(numPreviousProblems>0)
		  {//TODO delete?
			  //for every previous problem
			  for(int m=0;m<numPreviousProblems;m++)
			  {

				  Alternative * as;
				  init_alternative_array(&as,get_max_num_alternatives(&appd));
				  //as=(Alternative*)malloc(sizeof(Alternative)*50);//TODO
				  int numAlternatives=get_alternatives(&(problems[m]), as);
				  //delete_alternatives(&as);
				  if(numAlternatives==0)
				  {
						  //TODO
						  printf("\n no alternatives\n");
				  }
				  else
				  {

					  if(is_base_case(&problems[m]))
					  {
							  SpPD sp;
							  get_solution_base_case(&(palg->problems[m]),&sp);
							  //TODO is better solution
							  if(problems[m].solution.acum>=palg->best)
							  {
								  copy_aproblem_PD( &(palg->solvedProblems[palg->num_solved]),palg->problems[m]);
								  palg->num_solved++;
								  update_best(palg);
							  }
							  printf("     is base case and takes alternative: %d\n", sp.alternative.indexResource);
					  }

					  else
					  {
						  //get new problems
						  printf("        Alternatives: ");
						  for(int k=0;k<numAlternatives;k++)
						  {
							  printf("%d ", as[k].indexResource);
						  }
						  printf("\n");
						  randomize(palg,as);//not using
						  Logico ismin;
						  Logico ismax;
						  for(int u=0;u<numAlternatives;u++)
						  {
							  //prune
							  ismin=is_min(palg);
							  ismax=is_max(palg);
							  double estimated=get_estimate(&problems[m]);
							  if((ismin && estimated<=palg->best)
							  					  || (ismax && estimated>=palg->best))
							  {
							  	  int numSubproblems=get_num_subproblems();
							  	  AproblemPD appdNew;
								  for(int j=0;j<numSubproblems;j++)
								  {
									  initAProblemPD(&appdNew,&(palg->ppd.aproblem));

									  get_subproblem(&problems[m], &appdNew, as[u],numSubproblems);
									  printf("\n     is NOT base case: last appdNew sol: %s\n",appdNew.solution.resources[appdNew.solution.lengthArrays-1].name);
									  printf("     i=%d of %d alternatives\n",u, numAlternatives);
									  //if problem//TODO
											  //add problem to new array
									  copy_aproblem_PD( &(newArrayAppd[lengthNewArrayAppd]),appdNew);
									 lengthNewArrayAppd++;

								  }//end for num subproblem=1
							  }//end if not prune
						   }//end for alternatives
					  }//end else (not base case)
					}//end else (exits alternative)
			  }//end for every previous problem

			  //change problems array and add num problems to array
			  palg->num_problems=lengthNewArrayAppd;
			  for(int w=0; w<lengthNewArrayAppd;w++)
			  {
				  palg->problems[w]=newArrayAppd[w];
			  }
		  }//end if num previous>0
	  }//end for size
	  return res;
  }


  int getPreviousProblems(PalgorithmPD palg,PAproblemPD problems){

	  for(int i=0;i<palg->num_problems;i++)
	  {
		  problems[i]=palg->problems[i];
	  }
	  return palg->num_problems;

  }

  int delete_algorithmPD(PalgorithmPD palg)
  {
	  free(palg->solvedProblems);
	  free(palg->problems);
	  delete_problem_PD(&(palg->ppd));
	  return 0;
  }


