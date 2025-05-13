/*
 * LS.h
 *
 *  Created on: 18 Apr 2020
 *      Author: Peng
 */

#ifndef LS_H_
#define LS_H_
#include "Individual.h"
#include "read_data.h"
#include "RepairProcedure.h"
class LS {
public:
	// methods
	LS(read_data * data,RepairProcedure *repair);
	virtual ~LS();
	void local_search_run(Individual *s,double penaltyDepot, double penaltyVeh);
	/**********************************************************************/
	void removeRoutesAndLocations(Individual *s);

private:
	void check_ls_solution();
	void Comput_fitLS();
	void output_solution();
	//methods
	void srun();
	void loadSolution();
//	void updateRouteInfor(int &r);
	double compute(bool istrue, double value);
	double penaltyExcessLoad(int load);

	//moves
	bool Relocate_one();//  0-1 insert
	bool Relocate_two();//  2-0 insert
	bool Relocate_three();//0-2 insert
	bool Relocate_four();// 1-1 swap
	bool Relocate_five();// 2-1 swap
	bool Relocate_six();//  2-2 swap
	bool Relocate_seven();//2-2 swap R
	bool Relocate_eight();//2-opt
	bool Relocate_nine();//2-opt* R
	bool Relocate_ten();//2-opt*
	bool two_opt_starR();
	bool two_opt_star();
	//
	bool Relocate_nineDepot();
	bool Relocate_tenDepot();

	//methods for Moves
	void setLocalVariableRouteU();
	void setLocalVariableRouteV();
	void insertNode(Node *node1, Node *node2);
	void swapNode(Node *node1, Node *node2);
	//methods for split clients

	double distt;// used to debug
	/*******************************************Split operators*/
	bool Splitb();
	bool Splitf();
	/********************************************/
	read_data* data;
	Individual* s;
	RepairProcedure *repair;
	double penaltyDepot;
	double penaltyVeh;
	int numV;
	int numL;
	int numC;
	std::vector< int>orderNodes;
	Node *nodeU;
	Node *nodeX;
	Node *nodeV;
	Node *nodeY;
	Route *routeU;
	Route *routeV;
	NodeDepot *locU;
	NodeDepot *locV;
	int nodeUPrevIndex, nodeUIndex, nodeXIndex, nodeXNextIndex ;
	int nodeVPrevIndex, nodeVIndex, nodeYIndex, nodeYNextIndex ;
	int loadU, loadX, loadV, loadY;
	double PreU2U, X2NextX, PreV2V, Y2NextY;
	bool searchCompleted;
	int loopID;
	double costSuppU,costSuppV;
	double costTwoOpt;// used to record the move gain triggered by 2-opt and 2-opt*
	int nbMoves;
	double moveGain;
	bool searchComplete;

	void outputIntermediateSolution();
	////////////////////////////////////////////////////////////////////////////////////////
	void removeLocation(int removeLoc);
	void removeRoute(int removeRou);
	std::vector< int >listRemove;
	int *auxArray1;
};
#endif /* LS_H_ */

