/*
 * solution.c
 *
 *  Created on: 24 dic. 2020
 *      Author: practica
 */

#include "solution.h"

int solutionToString(PSolution ps){
	int res=0;
	printf("Solution: %f\n",ps->acum);
	int l=ps->lengthArrays;
	for(int i=0;i<l; i++){
		printf("task %d - resource %s\n", i, ps->resources[i]->name);
	}
	return res;
}
int updateSolution(PSolution ps, PAlternative pa, double value){
	int res=0;
	ps->lengthArrays=ps->lengthArrays+1;
	ps->acum=(ps->acum)+value;
	Resource resource;
	initResource(&resource,"test");//TODO
	ps->resources[(ps->lengthArrays)-1]=&resource;
	return res;
}
