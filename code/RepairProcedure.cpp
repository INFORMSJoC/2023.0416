/*
 * RepairProcedure.cpp
 *
 *  Created on: Aug 25, 2021
 *      Author: pengfei
 */

#include "RepairProcedure.h"

RepairProcedure::RepairProcedure(read_data *data, popManager * popman) {
	// TODO Auto-generated constructor stub
	this->data = data;
	this->numV = data->numV;
	this->numL = data->numL;
	this->numC = data->numC;
	tabuTable = new int *[data->maxVnum];
	for (int i=0;i<data->maxVnum; i++)
		tabuTable[i] = new int [numC];
	checkCus = new int [numV];

	this->popman = popman;
}
RepairProcedure::~RepairProcedure(){
	// TODO Auto-generated destructor stub
	for (int i=0;i<data->maxVnum;i++)
		delete [] tabuTable[i];
	delete [] tabuTable;
	delete [] checkCus;
}
bool RepairProcedure::checkInfeasible(){
	for (int i=0;i<numL;i++){
		if (s->location[i].used){
			for (int j=0; j<s->location[i].maxVisit; j++){
				if (s->location[i].depots[j].route->used){
					if (s->location[i].depots[j].route->load > data->vehCap)
						return false;
				}
			}
			if (s->location[i].remainInven > s->location[i].inventory)
				return false;
		}
	}
	return true;
}
double RepairProcedure::penaltyLocationFunction(Node *p, int loc){
	double penalty = 0.;
	penalty += penaltyExcessLocation(loc-numV, s->location[loc-numV].remainInven + p->route->load) - s->location[loc-numV].penalty
			+ penaltyExcessLocation(p->route->whichLocation->cour-numV, p->route->whichLocation->remainInven - p->route->load) - p->route->whichLocation->penalty;
	return penalty;
}
double RepairProcedure::penaltyExcessLocation(int loc, int inventory){
	return (std::max<double>(0.,inventory - s->location[loc].inventory) * penaltyDepot * (data->normalizaPara * s->dis));
}
double RepairProcedure::penaltyExcessRoute(int load){
	return (std::max<double>(0.,load - data->vehCap) * penaltyVeh * (data->normalizaPara * s->dis));
}
void RepairProcedure::updateThePenalty(int iden, Route *rou){
	if (iden < 0){
		for (int i=0;i<numL;i++){
			s->location[i].penalty = penaltyExcessLocation(i, s->location[i].remainInven);
			for (int j=0; j<s->location[i].maxVisit; j++)
				s->location[i].depots[j].route->penalty = penaltyExcessRoute(s->location[i].depots[j].route->load);
		}
	}
	else{
		s->location[iden].penalty = penaltyExcessLocation(iden, s->location[iden].remainInven);
		rou->penalty = penaltyExcessRoute(rou->load);
	}
}
void RepairProcedure::localRepair(){
	if(checkInfeasible())return;// check the in-feasibility
	int loopID = 1;
	updateThePenalty(-1, NULL);
	Node *p; int whichLoc;

	for (int i=0; i<numL; i++){//to adjust some routes
		if (s->location[i].remainInven > s->location[i].inventory){
			min_delta = Max; nodeU = NULL;
			for (int j=0;j<s->location[i].maxVisit; j++){
				if (s->location[i].splitTime == 1)break;// if the capacity of a route exceeds the location capacity, then continue;
				if (s->location[i].depots[j].route->load == 0)continue;
				p = s->location[i].depots[j].next;
				for (int k =0; k<(int)data->customerToLoc[p->cour].size(); k++){
					if (data->customerToLoc[p->cour][k] == i+numV)continue;
					delta = data->D[p->cour][data->customerToLoc[p->cour][k]] + data->D[s->location[i].endDepots[j].pre->cour][data->customerToLoc[p->cour][k]]
							- data->D[p->pre->cour][p->cour] - data->D[s->location[i].endDepots[j].pre->cour][p->pre->cour];
					delta +=  penaltyLocationFunction(p,data->customerToLoc[p->cour][k])
							+ compute(s->location[data->customerToLoc[p->cour][k]-numV].used,data->openCostLoc[data->customerToLoc[p->cour][k]]);
						//	+ compute(false, data->usingCostVeh);

					if (delta < min_delta){
						min_delta = delta;
						nodeU = p;
						whichLoc = data->customerToLoc[p->cour][k];
					}
				}
				while (! p->isDepot){
					for (int k =0; k<(int)data->customerToLoc[p->cour].size(); k++){
						if (data->customerToLoc[p->cour][k] == i+numV)continue;
						delta = data->D[p->cour][data->customerToLoc[p->cour][k]] + data->D[s->location[i].endDepots[j].pre->cour][data->customerToLoc[p->cour][k]]
								- data->D[p->pre->cour][p->cour] - data->D[s->location[i].endDepots[j].pre->cour][p->pre->cour];
						delta += penaltyLocationFunction(p,data->customerToLoc[p->cour][k])
								+ compute(s->location[data->customerToLoc[p->cour][k]-numV].used,data->openCostLoc[data->customerToLoc[p->cour][k]]);
							//	+ compute(false, data->usingCostVeh);
						if (delta < min_delta){
							min_delta = delta;
							nodeU = p;
							whichLoc = data->customerToLoc[p->cour][k];
						}
					}
					p = p->next;
				}
			}
			// carry out the operation'
			if (nodeU == NULL)continue;
			whichLoc = whichLoc - numV;
			if (nodeU->pre->cour >= numV){// is the depot;
				routeU = nodeU->route;
				routeV = &s->route[*s->location[whichLoc].emptyRoutes.begin()];
				Node *end = nodeU->route->endDepot->pre;
				nodeU->route->depot->next = nodeU->route->endDepot;
				nodeU->route->endDepot->pre = nodeU->route->depot;
				nodeU->pre = routeV->depot;
				routeV->depot->next = nodeU;
				routeV->endDepot->pre = end;
				end->next = routeV->endDepot;
				nodeU->route = routeV;
				s->location[whichLoc].splitTime++;
				while(!nodeU->isDepot){
					nodeU->route = routeV;
					nodeU = nodeU->next;
				}
			}
			else{
				routeU = nodeU->route;
				routeV = &s->route[*s->location[whichLoc].emptyRoutes.begin()];
				Node *begin = nodeU->route->depot->next;
				Node *end = nodeU->route->endDepot->pre;
				nodeU->route->depot->next = nodeU->route->endDepot;
				nodeU->route->endDepot->pre = nodeU->route->depot;
				begin->pre = end;
				end->next = begin;
				Node *nodeX = nodeU->pre;
				nodeU->pre = routeV->depot;
				routeV->depot->next = nodeU;
				routeV->endDepot->pre = nodeX;
				nodeX->next =routeV->endDepot;
				nodeU->route = routeV;
				s->location[whichLoc].splitTime++;
				while(!nodeU->isDepot){
					nodeU->route = routeV;
					nodeU = nodeU->next;
				}
			}
			s->updateRouteInfor(nodeU->route->cour, -1);
			whichLoc += numV;
			s->updateLocationInfor(whichLoc);
			updateThePenalty(whichLoc-numV, routeV);
			whichLoc = i+ numV;
			s->updateRouteInfor(routeU->cour, -1);
			s->updateLocationInfor(whichLoc);
			updateThePenalty(whichLoc-numV, routeU);

		}
	}

	int usedInventory = 0;
	for (int i =0;i<numL;i++)
		if (s->location[i].used)
			usedInventory += s->location[i].inventory;

	int looppID = 0;
	while (usedInventory < data->totalDemand){
		min_delta = Max;
		for (int i=0; i<numL;i++){
			if (s->location[i].used && s->location[i].remainInven > s->location[i].inventory){
				for (int j=0;j<s->location[i].maxVisit; j++){
					if (s->location[i].depots[j].route->load == 0)continue;
					p = s->location[i].depots[j].next;
					while (! p->isDepot){
						//
						if (p->route->whichLocation->splitTime == 1 && p->route->nbClients == 1)continue;
						for (int k =0;k < (int)data->customerToLoc[p->cour].size();k++){
							nodeV = &s->location[data->customerToLoc[p->cour][k] - numV].depots[0];
						//	nodeV = s->route[*s->location[data->customerToLoc[p->cour][k] - numV].emptyRoutes.begin()].depot;


							delta = compute(nodeV->route->whichLocation->used, data->openCostLoc[nodeV->route->whichLocation->cour])
									+ compute(!(nodeV->isDepot), data->usingCostVeh)
									- compute(p->route->nbClients != 1, data->usingCostVeh);
							delta += data->D[p->pre->cour][p->next->cour]-data->D[p->pre->cour][p->cour]-data->D[p->cour][p->next->cour]
									+ data->D[nodeV->cour][p->cour]+data->D[p->cour][nodeV->cour];
							routeU = p->route;
							routeV = nodeV->route;
							delta += penaltyExcessLocation(routeU->whichLocation->cour-numV, routeU->whichLocation->remainInven - p->dem) - routeU->whichLocation->penalty
									+ penaltyExcessLocation(routeV->whichLocation->cour-numV, routeV->whichLocation->remainInven + p->dem) - routeV->whichLocation->penalty
									+ penaltyExcessRoute(routeU->load - p->dem) - routeU->penalty
									+ penaltyExcessRoute(routeV->load + p->dem) - routeV->penalty;
							//
							if (delta < min_delta){
								whichMove = 1;
								nodeUb = p;
								nodeVb = nodeV;
								min_delta = delta;
							}
						}
						p = p->next;
					}
				}
			}
			// carry out the operation'
		}
		routeU = nodeUb->route;
		routeV = nodeVb->route;
		insertNode(nodeUb,nodeVb);
		//
		s->updateRouteInfor(routeU->cour, -1);
		s->updateLocationInfor(routeU->whichLocation->cour);
		s->updateRouteInfor(routeV->cour, -1);
		s->updateLocationInfor(routeV->whichLocation->cour);
		updateThePenalty(whichLoc-numV, routeU);
		looppID ++;
		if (looppID > 100) return;
		//
		usedInventory = 0;
		for (int i =0;i<numL;i++)
			if (s->location[i].used)
				usedInventory += s->location[i].inventory;
	}
////////////////////////////////////////////////////////////////////////////////
	//the second phase to restore the feasibility
	for (int i=0;i<data->maxVnum;i++)
		for (int j=0;j<numC; j++)
			tabuTable[i][j] = 0;
	iteration = 0;
//	if (data->iteration == 1)
//		s->outputSolution();
	loadInventory();
	while(1){
		min_delta = Max;
		for (int i=0;i<numL; i++){// to adjust some customers
			for (int j=0; j<s->location[i].maxVisit; j++){
				if (s->location[i].penalty > 0 || s->location[i].depots[j].route->penalty > 0){
					nodeU = s->location[i].depots[j].next;
					while(!nodeU->isDepot){
						for (int posV = 0; posV<(int)data->edgeNode[nodeU->cour].size(); posV++){
							if (data->edgeNode[nodeU->cour][posV] < numV){
								nodeV = &s->client[data->edgeNode[nodeU->cour][posV]];
								if (s->location[i].penalty > 0 && s->location[i].depots[j].route->penalty <= 0)
									if (nodeV->route->whichLocation == nodeU->route->whichLocation)continue;

								if (nodeV->route == nodeU->route)continue;
								setLocalVariableRouteU();
								setLocalVariableRouteV();
								//
								Relocate_one();
								Relocate_four();
								Relocate_nine();
								Relocate_ten();
								// Trying moves that insert nodeU directly after the depot
								if (nodeV->pre->isDepot){
									nodeV = nodeV->pre;
									setLocalVariableRouteV();
									Relocate_one();
									Relocate_nine();
									Relocate_ten();
								}
							}
						//	else{
						//		nodeV = s->route[*s->location[data->edgeNode[nodeU->cour][posV]-numV].emptyRoutes.begin()].depot;
						//		if (s->location[i].penalty > 0 && s->location[i].depots[j].route->penalty <= 0)
						//			if (nodeV->route->whichLocation == nodeU->route->whichLocation)continue;

						//		setLocalVariableRouteU();
						//		setLocalVariableRouteV();
						//		Relocate_one();
						//		Relocate_nine();
						//		Relocate_ten();
						//	}
						}
						// adding the new route.
						if (!s->location[i].emptyRoutes.empty()){
							nodeV = s->route[*s->location[i].emptyRoutes.begin()].depot;
							setLocalVariableRouteU();
							setLocalVariableRouteV();
							Relocate_one();
							Relocate_nine();
							Relocate_ten();
						}
						//
						nodeU = nodeU->next;
					}
					// inserted the cities into some locations

				}
			}
		}
		if (min_delta == Max )
			return;
		updateTheMove();

	//	if (iteration > 100) break;// using the last remedy strategy to save the solution
		if (iteration > 100) return;// if the solution cannot repair within fixed steps, discard it.

		iteration++;
		if (iteration > loopID*numV/10. && (s->inventory < data->totalDemand || s->inventoryVeh < data->totalDemand)){
			penaltyVeh = penaltyVeh*10;
			penaltyDepot = penaltyDepot*10;
			updateThePenalty(-1, NULL);
			loopID ++;
		}
	}
	/////////////////////////////the finall remedy
	for (int i=0;i<data->maxVnum;i++)
		for (int j=0;j<numC; j++)
			tabuTable[i][j] = 0;
	iteration = 0;
	while(1){
		min_delta = Max;
		for (int i=0;i<numL; i++){// to adjust some customers
			for (int j=0; j<s->location[i].maxVisit; j++){
				if (s->location[i].penalty > 0 || s->location[i].depots[j].route->penalty > 0){
					nodeU = s->location[i].depots[j].next;
					while(!nodeU->isDepot){
						for (int posV = 0; posV<numV;posV++){
							if (s->client[posV].route == nodeU->route)continue;
							if (s->client[posV].route->whichLocation->penalty || s->client[posV].route->penalty > 0)continue;
							nodeV = &s->client[posV];
							setLocalVariableRouteU();
							setLocalVariableRouteV();
							//
							Relocate_one();
							Relocate_four();
							Relocate_nine();
							Relocate_ten();
							// Trying moves that insert nodeU directly after the depot
							if (nodeV->pre->isDepot){
								nodeV = nodeV->pre;
								setLocalVariableRouteV();
								Relocate_one();
								Relocate_nine();
								Relocate_ten();
							}
						}
						// adding the new route.
						if (!s->location[i].emptyRoutes.empty()){
							nodeV = s->route[*s->location[i].emptyRoutes.begin()].depot;
							setLocalVariableRouteU();
							setLocalVariableRouteV();
							Relocate_one();
							Relocate_nine();
							Relocate_ten();
						}
						//
						nodeU = nodeU->next;
					}
				}
			}
		}
		if (min_delta == Max )
			return;
		updateTheMove();
		iteration++;
		if (iteration > loopID*numV/10. && (s->inventory < data->totalDemand || s->inventoryVeh < data->totalDemand)){
			penaltyVeh = penaltyVeh*10;
			penaltyDepot = penaltyDepot*10;
			updateThePenalty(-1, NULL);
			loopID ++;
		}
	}
}
void RepairProcedure::loadInventory(){
	s->inventory = 0;
	int numRoute = 0;
	for (int i=0;i<numL;i++){// check the total inventory
		if (s->location[i].used){
			s->inventory += s->location[i].inventory;
			numRoute += s->location[i].splitTime;
		}
		s->inventoryVeh = numRoute * data->vehCap;
	}
}
//void RepairProcedure::createNewRoute(){




//}
void RepairProcedure::updateTheMove(){
	nodeU=nodeUb;
	nodeV=nodeVb;
	setLocalVariableRouteU();
	setLocalVariableRouteV();

	if (whichMove == 1){
		tabuTable[routeU->cour][nodeU->cour] = iteration + tabu_length + rand()%tabu_length;
		insertNode(nodeU,nodeV);
	}
	else if (whichMove == 4){
		tabuTable[routeU->cour][nodeU->cour] = iteration + tabu_length + rand()%tabu_length;
		tabuTable[routeV->cour][nodeV->cour] = iteration + tabu_length + rand()%tabu_length;
		swapNode(nodeU,nodeV);
	}
	else if (whichMove == 9){
		tabuTable[routeU->cour][nodeU->cour] = iteration + tabu_length + rand()%tabu_length;
		tabuTable[routeV->cour][nodeV->cour] = iteration + tabu_length + rand()%tabu_length;
		Node * depotV = routeV->depot;
		Node * temp = NULL;
		Node * xx = nodeX;
		Node * vv = nodeV;
		Node * lastU = nodeU->route->endDepot->pre;
		Node * beginV =nodeV->route->depot->next;

		while (!xx->isDepot){
			temp = xx->next;
			xx->next = xx->pre;
			xx->pre = temp;
			xx->route = routeV;
			xx = temp;
		}
		while (!vv->isDepot){
			temp = vv->pre;
			vv->pre = vv->next;
			vv->next = temp;
			vv->route = routeU;
			vv = temp;
		}
		if (nodeV->isDepot){
			nodeU->next = routeU->endDepot;
			routeU->endDepot->pre = nodeU;
			if (nodeX->isDepot){
				nodeY->pre = depotV;
				depotV->next = nodeY;
			}
			else{
				nodeY->pre = nodeX;
				nodeX->next = nodeY;
				lastU->pre = depotV;
				depotV->next = lastU;
			}
		}
		else{
			nodeU->next= nodeV;
			nodeV->pre = nodeU;
			beginV->next = routeU->endDepot;
			routeU->endDepot->pre = beginV;
			if (nodeX->isDepot){
				nodeY->pre = depotV;
				depotV->next = nodeY;
			}
			else{
				nodeY->pre = nodeX;
				nodeX->next = nodeY;
				lastU->pre = depotV;
				depotV->next = lastU;
			}
		}
	}
	else if (whichMove == 10){
		tabuTable[routeU->cour][nodeU->cour] = iteration + tabu_length + rand()%tabu_length;
		tabuTable[routeV->cour][nodeV->cour] = iteration + tabu_length + rand()%tabu_length;
		Node *lastU = nodeU->route->endDepot->pre;
		Node *lastV =nodeV->route->endDepot->pre;
		Node * count = nodeY;
		while (!count->isDepot){
			count->route = routeU;
			count = count->next;
		}
		count = nodeX;
		while (!count->isDepot){
			count->route = routeV;
			count = count->next;
		}

		if (nodeY->isDepot){
			nodeU->next = routeU->endDepot;
			routeU->endDepot->pre = nodeU;
			if (nodeX->isDepot){
				nodeV->next = routeV->endDepot;
				routeV->endDepot->pre = nodeV;
			}
			else{
				lastU->next = routeV->endDepot;
				routeV->endDepot->pre = lastU;
				nodeV->next = nodeX;
				nodeX->pre = nodeV;
			}
		}
		else{
			nodeU->next = nodeY;
			nodeY->pre = nodeU;
			lastV->next = routeU->endDepot;
			routeU->endDepot->pre = lastV;
			if (nodeX->isDepot){
				nodeV->next = routeV->endDepot;
				routeV->endDepot->pre = nodeV;
			}
			else{
				lastU->next = routeV->endDepot;
				routeV->endDepot->pre = lastU;
				nodeV->next = nodeX;
				nodeX->pre = nodeV;
			}
		}
	}
	//
	s->updateRouteInfor(routeU->cour,-1);
	s->updateRouteInfor(routeV->cour,-1);
	s->updateLocationInfor(routeU->whichLocation->cour);
	if (routeU->whichLocation != routeV->whichLocation)
		s->updateLocationInfor(routeV->whichLocation->cour);
	updateThePenalty(routeU->whichLocation->cour-numV, routeU);
	updateThePenalty(routeV->whichLocation->cour-numV, routeV);
	loadInventory();
}
void RepairProcedure::insertNode(Node *U, Node *V){
	U->pre->next = U->next;
	U->next->pre = U->pre;
	V->next->pre = U;
	U->pre = V;
	U->next = V->next;
	V->next = U;
	U->route = V->route;
}
void RepairProcedure::swapNode(Node *U, Node *V){
	Node * myVPred = V->pre;
	Node * myVSuiv = V->next;
	Node * myUPred = U->pre;
	Node * myUSuiv = U->next;
	Route * myRouteU = U->route;
	Route * myRouteV = V->route;

	myUPred->next = V;
	myUSuiv->pre = V;
	myVPred->next = U;
	myVSuiv->pre = U;

	U->pre = myVPred;
	U->next = myVSuiv;
	V->pre = myUPred;
	V->next = myUSuiv;

	U->route = myRouteV;
	V->route = myRouteU;
}
double RepairProcedure::compute(bool istrue, double value){
	if (istrue)	return 0.;
	else return value;
}
void RepairProcedure::Relocate_one(){
	if (nodeU->route->whichLocation->splitTime == 1 && nodeU->route->nbClients == 1 && s->inventory - nodeU->route->whichLocation->inventory < data->totalDemand)return;

	if (tabuTable[routeU->cour][nodeU->cour] > iteration) return;
	delta = compute(nodeV->route->whichLocation->used, data->openCostLoc[nodeV->route->whichLocation->cour])
			+ compute(!(nodeV->isDepot && nodeY->isDepot), data->usingCostVeh)
			- compute(!(nodeU->route->whichLocation->splitTime == 1 && nodeU->route->nbClients == 1 && nodeU->route->whichLocation != nodeV->route->whichLocation), data->openCostLoc[nodeU->route->whichLocation->cour])
			- compute(nodeU->route->nbClients != 1, data->usingCostVeh);
	delta += data->D[nodeUPrevIndex][nodeXIndex]-data->D[nodeUPrevIndex][nodeUIndex]-data->D[nodeUIndex][nodeXIndex]
			+ data->D[nodeVIndex][nodeUIndex]+data->D[nodeUIndex][nodeYIndex]-data->D[nodeVIndex][nodeYIndex];
	delta += penaltyExcessLocation(routeU->whichLocation->cour-numV, routeU->whichLocation->remainInven - nodeU->dem) - routeU->whichLocation->penalty
			+ penaltyExcessLocation(routeV->whichLocation->cour-numV, routeV->whichLocation->remainInven + nodeU->dem) - routeV->whichLocation->penalty
			+ penaltyExcessRoute(routeU->load - nodeU->dem) - routeU->penalty
			+ penaltyExcessRoute(routeV->load + nodeU->dem) - routeV->penalty;
	//
	if (delta < min_delta){
		whichMove = 1;
		nodeUb = nodeU;
		nodeVb = nodeV;
		min_delta = delta;
	}
}
void RepairProcedure::Relocate_four(){//
	if (tabuTable[routeU->cour][nodeU->cour] > iteration && tabuTable[routeV->cour][nodeV->cour] > iteration) return;
	delta = data->D[nodeUPrevIndex][nodeVIndex]+data->D[nodeVIndex][nodeXIndex]-data->D[nodeUPrevIndex][nodeUIndex]-data->D[nodeUIndex][nodeXIndex]
			+ data->D[nodeVPrevIndex][nodeUIndex]+data->D[nodeUIndex][nodeYIndex]-data->D[nodeVPrevIndex][nodeVIndex]-data->D[nodeVIndex][nodeYIndex]
			+ penaltyExcessLocation(routeU->whichLocation->cour-numV, routeU->whichLocation->remainInven - nodeU->dem + nodeV->dem) - routeU->whichLocation->penalty
			+ penaltyExcessLocation(routeV->whichLocation->cour-numV, routeV->whichLocation->remainInven + nodeU->dem - nodeV->dem) - routeV->whichLocation->penalty
			+ penaltyExcessRoute(routeU->load - nodeU->dem + nodeV->dem) - routeU->penalty
			+ penaltyExcessRoute(routeV->load + nodeU->dem - nodeV->dem) - routeV->penalty;
	//
	if (delta < min_delta){
		whichMove = 4;
		min_delta = delta;
		nodeUb = nodeU;
		nodeVb = nodeV;
	}
}
void RepairProcedure::Relocate_nine(){
	if (tabuTable[routeU->cour][nodeU->cour] > iteration && tabuTable[routeV->cour][nodeV->cour] > iteration) return;
	if (nodeV->route->whichLocation->splitTime == 1 && nodeV->next->isDepot && s->inventory - nodeV->route->whichLocation->inventory < data->totalDemand)return;
	if (nodeU->next->isDepot && nodeV->isDepot)return;
	Node *lastU = nodeU->route->endDepot->pre;
	Node *beginV =nodeV->route->depot->next;
	delta = compute(nodeV->route->whichLocation->used, data->openCostLoc[nodeV->route->whichLocation->cour])
			- compute(!(nodeV->route->whichLocation->splitTime == 1 &&routeU->whichLocation != routeV->whichLocation&& nodeY->isDepot && nodeX->isDepot), data->openCostLoc[nodeV->route->whichLocation->cour])
			+ compute(!(nodeV->isDepot && nodeY->isDepot && !nodeX->isDepot), data->usingCostVeh)
			- compute(!(nodeX->isDepot && nodeY->isDepot), data->usingCostVeh);
	double distanceMoveGain = 0;
	if (nodeV->isDepot){
		distanceMoveGain += data->D[nodeUIndex][nodeU->route->endDepot->cour];
		if (nodeX->isDepot)
			distanceMoveGain += data->D[routeV->depot->cour][nodeYIndex];
		else{
			distanceMoveGain += data->D[nodeX->cour][nodeYIndex];
			distanceMoveGain += data->D[lastU->cour][routeV->depot->cour];
			distanceMoveGain = distanceMoveGain - data->D[lastU->cour][routeU->depot->cour];
		}
	}
	else{
		distanceMoveGain += data->D[nodeUIndex][nodeVIndex];
		distanceMoveGain += data->D[routeU->depot->cour][beginV->cour];
		distanceMoveGain = distanceMoveGain - data->D[routeV->depot->cour][beginV->cour];
		if (nodeX->isDepot)
			distanceMoveGain += data->D[routeV->depot->cour][nodeYIndex];
		else{
			distanceMoveGain += data->D[nodeX->cour][nodeYIndex];
			distanceMoveGain += data->D[lastU->cour][routeV->depot->cour];
			distanceMoveGain = distanceMoveGain - data->D[lastU->cour][routeU->depot->cour];
		}
	}
	distanceMoveGain = distanceMoveGain - data->D[nodeUIndex][nodeXIndex] - data->D[nodeVIndex][nodeYIndex];

	delta += distanceMoveGain
			+ penaltyExcessLocation(routeU->whichLocation->cour-numV,routeU->whichLocation->remainInven + nodeV->cumulatedLoad - (routeU->load - nodeU->cumulatedLoad)) - routeU->whichLocation->penalty
			+ penaltyExcessLocation(routeV->whichLocation->cour-numV,routeV->whichLocation->remainInven - nodeV->cumulatedLoad + (routeU->load - nodeU->cumulatedLoad)) - routeV->whichLocation->penalty
			+ penaltyExcessRoute(nodeU->cumulatedLoad + nodeV->cumulatedLoad) - routeU->penalty
			+ penaltyExcessRoute(routeV->load - nodeV->cumulatedLoad + routeU->load - nodeU->cumulatedLoad) - routeV->penalty;

	if (delta < min_delta){
		whichMove = 9;
		min_delta = delta;
		nodeUb = nodeU;
		nodeVb = nodeV;
	}
}
void RepairProcedure::Relocate_ten(){
	if (tabuTable[routeU->cour][nodeU->cour] > iteration && tabuTable[routeV->cour][nodeV->cour]>iteration) return;
	// if the maximum inventory in location is less than all demands
	if (nodeV->route->whichLocation->splitTime == 1 && nodeV->isDepot && s->inventory - nodeV->route->whichLocation->inventory < data->totalDemand)return;
	if (nodeU->next->isDepot && nodeV->next->isDepot)return;
	delta = compute(nodeV->route->whichLocation->used, data->openCostLoc[nodeV->route->whichLocation->cour])
			- compute(!(nodeV->route->whichLocation->splitTime == 1 &&routeU->whichLocation != routeV->whichLocation && nodeV->isDepot && nodeX->isDepot), data->openCostLoc[nodeV->route->whichLocation->cour])
			+ compute(!(nodeV->isDepot && nodeY->isDepot && !nodeX->isDepot), data->usingCostVeh)
			- compute(!(nodeX->isDepot && nodeV->isDepot), data->usingCostVeh);
	double distanceMoveGain = 0;
	Node *lastU = nodeU->route->endDepot->pre;
	Node *lastV =nodeV->route->endDepot->pre;
	if (nodeY->isDepot){
		distanceMoveGain += data->D[nodeUIndex][routeU->endDepot->cour];
		if (nodeX->isDepot)
			distanceMoveGain += data->D[routeV->depot->cour][nodeVIndex];
		else{
			distanceMoveGain += data->D[nodeX->cour][nodeVIndex];
			distanceMoveGain += data->D[routeV->depot->cour][lastU->cour];
			distanceMoveGain = distanceMoveGain - data->D[routeU->depot->cour][lastU->cour];
		}
	}
	else{
		distanceMoveGain += data->D[nodeUIndex][nodeYIndex];
		distanceMoveGain += data->D[routeU->depot->cour][lastV->cour];
		distanceMoveGain = distanceMoveGain - data->D[routeV->depot->cour][lastV->cour];
		if (nodeX->isDepot)
			distanceMoveGain += data->D[routeV->depot->cour][nodeVIndex];
		else{
			distanceMoveGain += data->D[nodeX->cour][nodeVIndex];
			distanceMoveGain += data->D[routeV->depot->cour][lastU->cour];
			distanceMoveGain = distanceMoveGain - data->D[routeU->depot->cour][lastU->cour];
		}
	}
	distanceMoveGain = distanceMoveGain - data->D[nodeUIndex][nodeXIndex] - data->D[nodeVIndex][nodeYIndex];

	delta += distanceMoveGain
			+ penaltyExcessLocation(routeU->whichLocation->cour-numV, routeU->whichLocation->remainInven - (routeU->load - nodeU->cumulatedLoad) + routeV->load - nodeV->cumulatedLoad) - routeU->whichLocation->penalty
			+ penaltyExcessLocation(routeV->whichLocation->cour-numV, routeV->whichLocation->remainInven - (routeV->load - nodeV->cumulatedLoad) + routeU->load - nodeU->cumulatedLoad) - routeV->whichLocation->penalty
			+ penaltyExcessRoute(nodeU->cumulatedLoad + routeV->load - nodeV->cumulatedLoad) - routeU->penalty
			+ penaltyExcessRoute(nodeV->cumulatedLoad + routeU->load - nodeU->cumulatedLoad) - routeV->penalty;
	if (delta < min_delta){
		whichMove = 10;
		min_delta = delta;
		nodeUb = nodeU;
		nodeVb = nodeV;
	}
}
void RepairProcedure::setLocalVariableRouteU(){
	routeU=nodeU->route;
	nodeX=nodeU->next;
	nodeXNextIndex=nodeX->next->cour;
	nodeUIndex = nodeU->cour;
	nodeUPrevIndex = nodeU->pre->cour;
	nodeXIndex = nodeX->cour;
	loadU=nodeU->dem;
	loadX=nodeX->dem;
}
void RepairProcedure::setLocalVariableRouteV(){
	routeV=nodeV->route;
	nodeY=nodeV->next;
	nodeYNextIndex=nodeY->next->cour;//the next two city.  U-X-this
	nodeVIndex=nodeV->cour;
	nodeVPrevIndex=nodeV->pre->cour;
	nodeYIndex=nodeY->cour;
	loadV=nodeV->dem;
	loadY=nodeY->dem;
}
void RepairProcedure::loadSolution(){
	for (int i=0; i<numL;i++){
		for (int j=0;j<s->location[i].maxVisit; j++)
			s->updateRouteInfor(s->location[i].depots[j].route->cour, -1);
		int tem = i+numV;
		s->updateLocationInfor(tem);
	}
}
/***************************************************************************************************************/
/***************************************************************************************************************/
/***************************************************************************************************************/
/***************************************************************************************************************/
void RepairProcedure::newRepair(){
	if(checkInfeasible())return;// check the in-feasibility
	int loopID = 1;
	updateThePenalty(-1, NULL);
	for (int i=0;i<data->maxVnum;i++)
		for (int j=0;j<numC; j++)
			tabuTable[i][j] = 0;
	iteration = 0;
	while(1){
		min_delta = Max;
		for (int i=0;i<numL; i++){// to adjust some customers
			for (int j=0; j<s->location[i].maxVisit; j++){
				if (s->location[i].penalty > 0 || s->location[i].depots[j].route->penalty > 0){
					nodeU = s->location[i].depots[j].next;
					while(!nodeU->isDepot){
						for (int posV = 0; posV<(int)data->edgeNode[nodeU->cour].size(); posV++){
							if (data->edgeNode[nodeU->cour][posV] < numV){
								nodeV = &s->client[data->edgeNode[nodeU->cour][posV]];
								if (s->location[i].penalty > 0 && s->location[i].depots[j].route->penalty <= 0)
									if (nodeV->route->whichLocation == nodeU->route->whichLocation)continue;

								if (nodeV->route == nodeU->route)continue;
								setLocalVariableRouteU();
								setLocalVariableRouteV();
								//
								Relocate_one();
								Relocate_four();
								Relocate_nine();
								Relocate_ten();
								// Trying moves that insert nodeU directly after the depot
								if (nodeV->pre->isDepot){
									nodeV = nodeV->pre;
									setLocalVariableRouteV();
									Relocate_one();
									Relocate_nine();
									Relocate_ten();
								}
							}
							else{
								nodeV = s->route[*s->location[data->edgeNode[nodeU->cour][posV]-numV].emptyRoutes.begin()].depot;
								if (s->location[i].penalty > 0 && s->location[i].depots[j].route->penalty <= 0)
									if (nodeV->route->whichLocation == nodeU->route->whichLocation)continue;

								setLocalVariableRouteU();
								setLocalVariableRouteV();
								Relocate_one();
								Relocate_nine();
								Relocate_ten();
							}
						}
			//			createNewRoute();
						nodeU = nodeU->next;
					}
				}
			}
		}
		if (min_delta == Max )
			return;
		updateTheMove();
		iteration++;
		if (iteration > loopID*numV/10. && (s->inventory < data->totalDemand || s->inventoryVeh < data->totalDemand)){
			penaltyVeh = penaltyVeh*10;
			penaltyDepot = penaltyDepot*10;
			updateThePenalty(-1, NULL);
			loopID ++;
		}
	}
}
void RepairProcedure::repairForInitialSolution(Individual *s){
	this->s = s;
	loadSolution();
	loadInventory();
	this->penaltyDepot = data->penaltyDepot;
	this->penaltyVeh = data->penaltyVeh;
	if(checkInfeasible())return;// check the in-feasibility
	int loopID = 1;
	updateThePenalty(-1, NULL);
	for (int i=0;i<data->maxVnum;i++)
		for (int j=0;j<numC; j++)
			tabuTable[i][j] = 0;
	iteration = 0;
	while(1){
		min_delta = Max;
		for (int i=0;i<numL; i++){// to adjust some customers
			if (data->equalTwoVar(s->location[i].penalty, 0))continue;
			for (int j=0; j<s->location[i].maxVisit; j++){
				nodeU = s->location[i].depots[j].next;
				while(!nodeU->isDepot){
					for (int posV = 0; posV<(int)data->edgeNode[nodeU->cour].size(); posV++){
						if (data->edgeNode[nodeU->cour][posV] < numV){
							nodeV = &s->client[data->edgeNode[nodeU->cour][posV]];
							if (nodeU->route->whichLocation == nodeV->route->whichLocation)continue;
								if (nodeV->route == nodeU->route)continue;
							setLocalVariableRouteU();
							setLocalVariableRouteV();
							//
							Relocate_four();
							Relocate_nine();
							Relocate_ten();
							// Trying moves that insert nodeU directly after the depot
							if (nodeV->pre->isDepot){
								nodeV = nodeV->pre;
								setLocalVariableRouteV();
								Relocate_nine();
								Relocate_ten();
							}
						}
						else{
							nodeV = s->route[*s->location[data->edgeNode[nodeU->cour][posV]-numV].emptyRoutes.begin()].depot;
							if (s->location[i].penalty > 0 && s->location[i].depots[j].route->penalty <= 0)
								if (nodeV->route->whichLocation == nodeU->route->whichLocation) continue;
							setLocalVariableRouteU();
							setLocalVariableRouteV();
							Relocate_nine();
							Relocate_ten();
						}
					}
					nodeU = nodeU->next;
				}
			}
		}
		if (min_delta == Max ) break;
		updateTheMove();

		iteration++;
		if (iteration > loopID*numV/10. && (s->inventory < data->totalDemand || s->inventoryVeh < data->totalDemand)){
			penaltyVeh = penaltyVeh*10;
			penaltyDepot = penaltyDepot*10;
			updateThePenalty(-1, NULL);
			loopID ++;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////
//	s->outputSolution();
	updateThePenalty(-1, NULL);
	for (int i=0;i<data->maxVnum;i++)
		for (int j=0;j<numC; j++)
			tabuTable[i][j] = 0;
	iteration = 0;
	bool inFeasible = false;
	while(1){
		min_delta = Max;
		for (int i=0;i<numL; i++){// to adjust some customers
			for (int j=0; j<s->location[i].maxVisit; j++){
				if (s->location[i].depots[j].route->load <= data->vehCap)continue;
				inFeasible = true;
				nodeU = s->location[i].depots[j].next;
				while(!nodeU->isDepot){
					for (int posV = 0; posV<(int)data->edgeNode[nodeU->cour].size(); posV++){
						if (data->edgeNode[nodeU->cour][posV] < numV){
							nodeV = &s->client[data->edgeNode[nodeU->cour][posV]];
							if (nodeU->route->whichLocation != nodeV->route->whichLocation)continue;
							if (nodeV->route == nodeU->route)continue;
							setLocalVariableRouteU();
							setLocalVariableRouteV();
							//
							Relocate_one();
							Relocate_four();
							Relocate_nine();
							Relocate_ten();
							// Trying moves that insert nodeU directly after the depot
							if (nodeV->pre->isDepot){
								nodeV = nodeV->pre;
								setLocalVariableRouteV();
								Relocate_one();
								Relocate_nine();
								Relocate_ten();
							}
						}
						else{
							nodeV = s->route[*nodeU->route->whichLocation->emptyRoutes.begin()].depot;
							if (s->location[i].penalty > 0 && s->location[i].depots[j].route->penalty <= 0)
								if (nodeV->route->whichLocation == nodeU->route->whichLocation) continue;
							setLocalVariableRouteU();
							setLocalVariableRouteV();
							Relocate_one();
							Relocate_nine();
							Relocate_ten();
						}


					}
					// adding a new route
					nodeV = s->route[*nodeU->route->whichLocation->emptyRoutes.begin()].depot;
					setLocalVariableRouteU();
					setLocalVariableRouteV();
					Relocate_one();
					Relocate_nine();
					Relocate_ten();


					nodeU = nodeU->next;
				}
			}
		}
		if (inFeasible && min_delta == Max){
			for (int i=0;i<numL; i++){// to adjust some customers
				for (int j=0; j<s->location[i].maxVisit; j++){
					if (s->location[i].depots[j].route->load <= data->vehCap)continue;
					inFeasible = true;
					nodeU = s->location[i].depots[j].next;
					while(!nodeU->isDepot){
						for (int posV = 0; posV< numL; posV++){
							nodeV = s->route[*s->location[posV].emptyRoutes.begin()].depot;
							if (s->location[i].penalty > 0 && s->location[i].depots[j].route->penalty <= 0)
								if (nodeV->route->whichLocation == nodeU->route->whichLocation) continue;
							setLocalVariableRouteU();
							setLocalVariableRouteV();
							Relocate_one();
							Relocate_nine();
							Relocate_ten();
						}
						nodeU = nodeU->next;
					}
				}
			}
		}
		if (min_delta == Max )
			return;
//		s->outputSolution();
		updateTheMove();

		iteration++;
		if (iteration > loopID*numV/10. && (s->inventory < data->totalDemand || s->inventoryVeh < data->totalDemand)){
			penaltyVeh = penaltyVeh*10;
			penaltyDepot = penaltyDepot*10;
			updateThePenalty(-1, NULL);
			loopID ++;
		}
	}
}
void RepairProcedure::repairCapacityConstraint(Individual *subPop,double penaltyDepot, double penaltyVeh){
	this->penaltyDepot=penaltyDepot;
	this->penaltyVeh=penaltyVeh;
	this->s=subPop;
	loadSolution();

	localRepair();

//	newRepair();
	s->decideFeasible();
	if (!s->isFeasible)return;

	s->evaluationDis();
	s->isRight();


}
