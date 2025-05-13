/*
 * solution.h
 *
 *  Created on: 17 Apr 2020
 *      Author: Peng
 */

#ifndef SOLUTION_H_
#define SOLUTION_H_
#include"initialsol.h"
#include "read_data.h"
#include "Individual.h"
#include "LS.h"
#include "crossover.h"
#include "popManager.h"
#include "RepairProcedure.h"
#include "mutation.h"
class solution {
public:
	solution(read_data * data, crossover *cross, initial_sol *initialSol,LS *ls,  popManager *popman, RepairProcedure *repair,mutation *mutationOperator);
	virtual ~solution();
	void main_loop();

private:
	void generation_population(int rankPop, bool isInitial);
	void evolvePop(int rankPop, int &count);
	void getAverageValue(int rank);
	int selectsubPop(int subpop1);
	void outputprocess(Individual *indiv);
	void initialPop();
	void preprocessingLocationConfiguration();
	//variables
	read_data * data;
	initial_sol * initialSol;
	LS * ls;
	crossover* cross;
	popManager * popman;
	RepairProcedure *repair;
	mutation *mutationOperator;
	///////////////////////////////////////////////////////////////////////////
	Individual * global;
	Individual * loc_best;
	Individual * temIndiv;// needs to be
	Individual * bestIndivRestart;
	Individual * subPop;//used to save offspring solutions                                              the size cannot exceed 100.
	//
	Individual * fa=NULL;
	Individual * ma=NULL;

	std::vector<double>avgCostPop;
};

#endif /* SOLUTION_H_ */
