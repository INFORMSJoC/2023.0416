/*
 * popManager.h
 *
 *  Created on: 11 Aug 2021
 *      Author: Peng
 */

#ifndef POPMANAGER_H_
#define POPMANAGER_H_
#include "read_data.h"
#include "Individual.h"
typedef std::vector <Individual*> SubPopulation ;
typedef std::vector <std::vector<Individual*>> Population ;
class popManager {
public:
	popManager(read_data * data);
	virtual ~popManager();
	void reduceHalfSubPop();
	void removeEmptySubPop();

	//
	void copyAllData(Individual * s,Individual *ss);
	int addIndividual(Individual *indiv, int rankPop, bool isInitial);
	void addInElitePop(Individual *indiv);
	void reStart();
	//
	void removeWorstBiasedFitness(SubPopulation & pop);
	void updateBiasedFitnesses(SubPopulation & pop);
	/////////////
	Individual * selectionParentSolution(Individual *exist, int rankPop);
	Individual * selectionMotherSolution(Individual *exist, int rankPop);
	double averagePop();
	double usedDepots();
	//variables;
	Individual *global;
	Individual *bestIndivRestart;
	std::vector <std::vector<Individual*>> popula;


private:
	//variables
	read_data *data;
	SubPopulation feasibleSubpopulation;			// Feasible subpopulation, kept ordered by increasing penalized cost

	int numV;
	int numL;
	int numC;
	int *order;
};
#endif /* POPMANAGER_H_ */
