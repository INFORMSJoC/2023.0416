/*
 * main.cpp
 *
 *  Created on: 27 Apr 2021
 *      Author: Pengfei He
 */
#include"basic.h"
#include"commandline.h"
#include"solution.h"
int main(int argc, char*argv[]){
	CommandLine comLin(argc,argv);
	srand(comLin.seed);
	read_data *data=new read_data();
	data->read_fun(comLin.pathInstance, comLin.pathDepotCon, comLin.pathSolution, comLin.bestDis, comLin.isOptimal, comLin.timeLimit,comLin.iterLimit);
	data->startTime=clock();
	/***********************/

	popManager *popman=new popManager(data);
	crossover *cross=new crossover (data,popman);
	RepairProcedure *repair=new RepairProcedure(data, popman);
	LS * ls=new LS(data, repair);// local search phase
	initial_sol *initialSol=new initial_sol (data, repair);
	mutation *mutationOperator=new mutation(data);
	solution *solver=new solution(data,cross,initialSol,ls,popman,repair,mutationOperator);
	//
	solver->main_loop();
	/*************************************************/
	delete solver;
	delete repair;
	delete cross;
	delete popman;
	delete mutationOperator;
	delete ls;
	delete initialSol;
	delete data;
	return 0;
}
