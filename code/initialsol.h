/*
 * initialsol.h
 *
 *  Created on: 17 Apr 2020
 *      Author: Peng
 */

#ifndef INITIALSOL_H_
#define INITIALSOL_H_
#include "read_data.h"
#include "Individual.h"
#include "RepairProcedure.h"
class initial_sol {
public:
	initial_sol(read_data * data,RepairProcedure *repair);
	virtual ~initial_sol();
	void initial_solution(Individual *s, int rankPop);
private:
	Individual * s;
	read_data * data;
	RepairProcedure *repair;
	std::vector<int >sque;
	int *customer;
	int numV;
	int numL;
	int numC;
	int rankPop;

	//methods
	void greedyInsert();
	void insertCustomerToLocation(int &customer, int &loc, int newCus);
	double get_delta(int &in, Node *tem);
	void insertClient(Node *U, Node *V);
	// new initial solutions
	void newGreedyInsert();
	void insertCusToLoc(int loc);
	void remedyTheSolution(std::vector<int> location);


	//parameters
	int probDeterUsingLarLoc = 50;
	int remainDem = 0;
	bool *locUsed;
	bool *cusUsed;
	double penaltyDepot;
	double penaltyVeh;

};
#endif /* INITIALSOL_H_ */
