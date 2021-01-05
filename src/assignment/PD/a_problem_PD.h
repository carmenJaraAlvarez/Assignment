/*
 * a_problem_PD.h
 *
 *  Created on: 22 dic. 2020
 *      Author: practica
 */

#ifndef ASSIGNMENT_PD_A_PROBLEM_PD_H_
#define ASSIGNMENT_PD_A_PROBLEM_PD_H_
#include "../../common/logico.h"
#include "../aproblem.h"
#include "../alternative.h"
#include "../solution.h"
#include "../../PD/Sp_PD.h"

#define GREAT 999999.
#define SMALL -999999.
typedef struct
  {
	Aproblem aproblem;
	int index;
	Solution solution;

  } AproblemPD;

typedef AproblemPD* PAproblemPD;
typedef AproblemPD ArrayAproblemPD[TAM_ARRAY_PROBLEM];

  int initAProblemPD(PAproblemPD,PAproblem);
  int size(AproblemPD);
  Type get_type(PAproblemPD);
  Logico is_base_case(PAproblemPD);
  int get_alternatives(PAproblemPD, PAlternative);//PAlternative is a pointer to a dinamic array
  Logico is_alternative(PAproblemPD, int);
  int select_alternative(PAproblemPD,PAlternative, int,double*);//PAlternative is a pointer to a dinamic array
  int get_solution_base_case(PAproblemPD,PSpPD);
  int combine_solutions(AproblemPD, PSolution,PSpPD);
  int get_num_subproblems();//here is 1
  int get_subproblem(const PAproblemPD, PAproblemPD,Alternative, int);
  double get_estimate(AproblemPD);
  int get_solution(AproblemPD);
  int get_target(AproblemPD);
  int get_size(const PAproblemPD);
//*************************************AUX
  int updateSolution(PSolution, PAlternative, double ,Aproblem);
  int delete_problem_PD(PAproblemPD);//free memory
  int copy_AproblemPD(PAproblemPD,AproblemPD);




#endif /* ASSIGNMENT_PD_A_PROBLEM_PD_H_ */
