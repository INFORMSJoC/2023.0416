/*
 * RepairProcedure.h
 *
 *  Created on: Aug 25, 2021
 *      Author: pengfei
 */

#ifndef REPAIRPROCEDURE_H_
#define REPAIRPROCEDURE_H_
#include "Individual.h"
#include "popManager.h"
class RepairProcedure {
public:
	RepairProcedure(read_data * data, popManager * popman);
	virtual ~RepairProcedure();
	void repairForInitialSolution(Individual *s);
	void repairCapacityConstraint(Individual *subPop,double penaltyDepot, double penaltyVeh);
	double penaltyLocationFunction(Node *p, int loc);
	double penaltyExcessLocation(int loc, int inventory);
	double penaltyExcessRoute(int load);
	void updateThePenalty(int iden, Route *rou);
	//
	void setLocalVariableRouteU();
	void setLocalVariableRouteV();
	double compute(bool istrue, double value);
	void Relocate_one();//  0-1 insert
	void Relocate_four();
	void Relocate_nine();
	void Relocate_ten();
	void updateTheMove();
	void swapNode(Node *U, Node *V);
	void insertNode(Node *U, Node *V);
	//

	void localRepair();
	bool checkInfeasible();
	void loadInventory();
	void loadSolution();


	//
	void newRepair();


private:
	int numV;
	int numL;
	int numC;
	read_data *data;
	Individual *s;
	int sizeSubPop;
	double penaltyDepot;
	double penaltyVeh;
	//
	int *checkCus;
	int **tabuTable;
	int iteration;
	Node *nodeU;
	Node *nodeX;
	Node *nodeUb, *nodeVb;
	Node *nodeV;
	Node *nodeY;
	Route *routeU;
	Route *routeV;
	int nodeUPrevIndex, nodeUIndex, nodeXIndex, nodeXNextIndex ;
	int nodeVPrevIndex, nodeVIndex, nodeYIndex, nodeYNextIndex ;
	int loadU, loadX, loadV, loadY;
	double min_delta, delta;
	int whichMove;
	//
	int tabu_length = 30;


	popManager * popman;
};

#endif /* REPAIRPROCEDURE_H_ */
