/*
 * Indi.h
 *
 *  Created on: 18 Apr 2020
 *      Author: Peng
 */

#ifndef INDIVIDUAL_H_
#define INDIVIDUAL_H_
#include"read_data.h"
struct Node;
struct NodeDepot;
struct Route{
	bool used;
	int cour;
	int nbClients;
	int whenLastModified;				// "When" this route has been last modified
	Node *depot;
	Node *endDepot;
	NodeDepot *whichLocation;
	double dis;					// Total time on the route
	int load;						// Total load on the route
	double penalty;						// Current sum of load and duration penalties
};
struct Node{
	bool isDepot;//tells whether this node represent a depot or not
	int cour;//city index
	int position;
	int dem;
	int whenLastTestedRI;//When the MOVES evaluate this node
	Node *next;
	Node *pre;
	Route *route;
	int cumulatedLoad;// Cumulated load on this route until the customer (including itself)
	double cumulatedDis;	// Cumulated distance on this route until the customer (including itself)
	//
	int start;
	int end;
	bool iseum;
};
struct NodeDepot{
	bool used;
	int cour;
	int inventory;
	Node *depots;
	Node *endDepots;
	int splitTime;
	int maxVisit;
	int remainInven;
	bool iscapCon;
	double penalty;
	std::set< int > emptyRoutes;//used to index all empty routes
};
class Individual{
public:
	//methods
	Individual();
	virtual ~Individual();
	void define(read_data *data);
	void evaluationDis();
	void getSaturationDegree();
	void initilization();
	void updateRouteInfor(int &r,int nbMoves);
	void updateLocationInfor(int &ll);

	void outputSolution();
	void isRight();
	void decideFeasible();
	void outputBestSolution();
	/**********************************/
	double brokenPairsDistance(Individual * indiv2);
	double averageBrokenPairsDistanceClosest(int nbClosest);
	void removeProximity(Individual * indiv);	//variables
	Node * client;
	NodeDepot * location;
	Route * route;
//	std::vector<std::set< int > >emptyRoutes;//used to index all empty routes
	int inventory;// the total inventory carried by selected locations
	int inventoryVeh;// the total vehicles' capacity
	//
	double dis;
	int capExcess;
	double penalty;
	double fit;
	double biasedFit;
	bool isFeasible;
	std::multiset < std::pair < double, Individual* > > indivsPerProximity ;	// The other individuals in the population, ordered by increasing proximity (the set container follows a natural ordering based on the first value of the pair)
	int maxVnum;
	std::set<std::pair<double,int>> saturatLoc;// the first is degree, the second is the order of location.
	std::set<std::pair<double,int>> saturatRoute;// the first is degree, the second is the order of route.
private:
	read_data *data;
	int numV;
	int numL;
	int numC;

	int *checkArray;
};
#endif /* INDI_H_ */
