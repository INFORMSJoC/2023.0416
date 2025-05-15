/*
 * LS.cpp
 *
 *  Created on: 18 Apr 2020
 *      Author: Peng
 */

#include "LS.h"
#include "basic.h"
#include "Individual.h"
LS::LS(read_data * data, RepairProcedure *repair) {
	this->data=data;
	this->repair = repair;
	this->numV = data->numV;
	this->numC = data->numC;
	this->numL = data->numL;
	for (int i=0;i<numV;i++)
		orderNodes.push_back(i);
	auxArray1 = new int [numV];
}
LS::~LS() {
	delete [] auxArray1;
}
double LS::compute(bool istrue, double value){
	if (istrue)	return 0.;
	else return value;
}
void LS::srun(){
	random_shuffle(orderNodes.begin(),orderNodes.end());
	for (int i=0; i<numV; i++)
		if (rand()%data->alpha==0)
			random_shuffle(data->edgeNode[i].begin(),data->edgeNode[i].end());
	searchComplete=false;
	for (loopID=0;!searchComplete;loopID++){
		searchComplete=true;
		for (int posU=0;posU<numV;posU++){// only num_v-1 clients
			nodeU=&s->client[orderNodes[posU]];
			int lastTestRINodeU = nodeU->whenLastTestedRI;
			nodeU->whenLastTestedRI=nbMoves;
			for (int posV = 0; posV<(int)data->edgeNode[nodeU->cour].size(); posV++){
				if (data->edgeNode[nodeU->cour][posV] >= numV)continue;// not consider locations
				nodeV=&s->client[data->edgeNode[nodeU->cour][posV]];
				if (loopID==0 || std::max<int>(nodeU->route->whenLastModified,nodeV->route->whenLastModified) > lastTestRINodeU){
					setLocalVariableRouteU();
					setLocalVariableRouteV();
					//
					if (Relocate_one())continue;//10 insert operator
					if (Relocate_two())continue;//20 insert operator
					if (Relocate_three())continue;//02 insert operator
					if (nodeUIndex < nodeVIndex && Relocate_four())continue;//11 swap operator
					if (Relocate_five())continue;//21 swap
					if (nodeUIndex <= nodeVIndex && Relocate_six()) continue; // SWAP 22
					if (nodeUIndex <= nodeVIndex && Relocate_seven()) continue;; // SWAP 2-2 reverse
					if (routeU == routeV && Relocate_eight()) continue; // 2-OPT
					if (two_opt_star())continue;
					if (two_opt_starR())continue;
		//			if (routeU != routeV && Relocate_nine()) continue; // 2-OPT*
		//			if (routeU != routeV && Relocate_ten()) continue; // 2-OPT*
					// Trying moves that insert nodeU directly after the depot
					if (nodeV->pre->isDepot){
						nodeV = nodeV->pre;
						setLocalVariableRouteV();
						if (Relocate_one()) continue; // RELOCATE
						if (Relocate_two()) continue; // RELOCATE
						if (Relocate_three()) continue; // RELOCATE
						if (routeU != routeV && Relocate_nine()) continue; // 2-OPT*
						if (routeU != routeV && Relocate_ten()) continue; // 2-OPT*
					}
				}
			}
			if (loopID > 0){
				for (int posV = 0; posV < (int)data->customerToLoc[nodeU->cour].size();posV++){
					if (!s->location[data->customerToLoc[nodeU->cour][posV] - numV].used)continue;
					if (!s->location[data->customerToLoc[nodeU->cour][posV] - numV].emptyRoutes.empty()){
						nodeV = s->route[*s->location[data->customerToLoc[nodeU->cour][posV] - numV].emptyRoutes.begin()].depot;
						setLocalVariableRouteU();
						setLocalVariableRouteV();
						if (Relocate_one()) continue; // RELOCATE
						if (Relocate_two()) continue; // RELOCATE
						if (Relocate_nine()) continue; // RELOCATE
						if (Relocate_ten()) continue; // 2-OPT*
					}
				}
			}
			//begin split operators// the main function is to add routes
			setLocalVariableRouteU();
			if (Splitb())continue;
			if (Splitf())continue;
		}/// end the CVRP neighborhoods search
	}
}
bool LS::Splitf(){
	for (int i=0;i<(int)data->customerToLoc[nodeUIndex].size(); i++){
		int loc = data->customerToLoc[nodeUIndex][i];
		if (loc != nodeU->route->whichLocation->cour){
			Node * beginU = nodeU->route->depot->next;
		//	if (nodeU == lastU)return false;
			if (s->location[loc-numV].remainInven + nodeU->cumulatedLoad > s->location[loc-numV].inventory)return false;
			if (s->location[loc - numV].emptyRoutes.empty())return false;
			moveGain = compute(s->location[loc-numV].used, data->openCostLoc[loc])
					- compute(!(nodeU->route->whichLocation->splitTime == 1 && nodeU->next->isDepot), data->openCostLoc[nodeU->route->whichLocation->cour])
					- compute(!nodeU->next->isDepot, data->usingCostVeh) + data->usingCostVeh
					+ data->D[nodeUIndex][loc] + data->D[loc][beginU->cour] + data->D[nodeXIndex][beginU->pre->cour]
					- data->D[nodeXIndex][nodeUIndex] - data->D[beginU->cour][beginU->pre->cour];

			if (moveGain > -Min)return false;

			Node *xx = nodeU;
			Route *rr = &s->route[*s->location[loc-numV].emptyRoutes.begin()];

			while(!xx->isDepot){
				xx->route = rr;//route
				xx=xx->pre;
			}
			Node *pre, *next;
			pre=beginU->pre;
			next = nodeX;
			pre->next = next;
			next->pre = pre;
			//
			beginU->pre = rr->depot;
			rr->depot->next = beginU;
			nodeU->next= rr->endDepot;
			rr->endDepot->pre = nodeU;
			s->updateRouteInfor(routeU->cour,nbMoves);
			s->updateRouteInfor(rr->cour, nbMoves);

			s->updateLocationInfor(routeU->whichLocation->cour);
			s->updateLocationInfor(nodeU->route->whichLocation->cour);
/*			s->outputSolution();
			s->isRight();
			distt += moveGain;
			s->evaluationDis();
			if (!data->equalTwoVar(distt,s->fit)){
				std::cout<<"the distance is wrong"<<std::endl;
				exit(0);
			}
*/
			return true;
		}
	}
	return false;
}
bool LS::Splitb(){
	for (int i=0;i<(int)data->customerToLoc[nodeUIndex].size(); i++){
		int loc = data->customerToLoc[nodeUIndex][i];
		if (loc != nodeU->route->whichLocation->cour){
			Node * lastU = nodeU->route->endDepot->pre;
		//	if (nodeU == lastU)return false;
			if (s->location[loc-numV].remainInven + (nodeU->route->load - nodeU->pre->cumulatedLoad) > s->location[loc-numV].inventory)return false;
			if (s->location[loc - numV].emptyRoutes.empty())return false;
			moveGain = compute(s->location[loc-numV].used, data->openCostLoc[loc])
					- compute(!(nodeU->route->whichLocation->splitTime == 1 && nodeU->pre->isDepot), data->openCostLoc[nodeU->route->whichLocation->cour])
					- compute(!nodeU->pre->isDepot, data->usingCostVeh) + data->usingCostVeh
					+ data->D[nodeUIndex][loc] + data->D[loc][lastU->cour] + data->D[nodeUPrevIndex][lastU->next->cour]
					- data->D[nodeUPrevIndex][nodeUIndex] - data->D[lastU->cour][lastU->next->cour];

			if (moveGain > -Min)return false;
			Node *xx = nodeU;
			Route *rr = &s->route[*s->location[loc-numV].emptyRoutes.begin()];

			while(!xx->isDepot){
				xx->route = rr;
				xx=xx->next;
			}
			Node *pre, *next;
			pre=nodeU->pre;
			next = lastU->next;
			pre->next = next;
			next->pre = pre;
			//
			nodeU->pre = rr->depot;
			rr->depot->next = nodeU;
			lastU->next= rr->endDepot;
			rr->endDepot->pre = lastU;
			s->updateRouteInfor(routeU->cour,nbMoves);
			s->updateRouteInfor(rr->cour,nbMoves);

			s->updateLocationInfor(routeU->whichLocation->cour);
			s->updateLocationInfor(nodeU->route->whichLocation->cour);
/*			s->outputSolution();
			s->isRight();
			distt += moveGain;
			s->evaluationDis();
			if (!data->equalTwoVar(distt,s->fit)){
				std::cout<<"the distance is wrong"<<std::endl;
				exit(0);
			}
*/
			return true;
		}
	}
	return false;
}
bool LS::Relocate_one(){
	if (nodeUIndex == nodeYIndex)return false;
	if (!(nodeV->route->whichLocation->remainInven + nodeU->dem <= nodeV->route->whichLocation->inventory))// depot feasible
		return false;
	if(nodeV->route != nodeU->route && nodeV->route->load + nodeU->dem > data->vehCap)return false;// route feasible

	moveGain = compute(nodeV->route->whichLocation->used, data->openCostLoc[nodeV->route->whichLocation->cour])
			+ compute(!(nodeV->isDepot && nodeY->isDepot), data->usingCostVeh)
			- compute(!(nodeU->route->whichLocation->splitTime == 1 && nodeU->route->nbClients == 1 && nodeU->route->whichLocation != nodeV->route->whichLocation), data->openCostLoc[nodeU->route->whichLocation->cour])
			- compute(nodeU->route->nbClients != 1, data->usingCostVeh)
			+ data->D[nodeUPrevIndex][nodeXIndex]-data->D[nodeUPrevIndex][nodeUIndex]-data->D[nodeUIndex][nodeXIndex]
			+ data->D[nodeVIndex][nodeUIndex]+data->D[nodeUIndex][nodeYIndex]-data->D[nodeVIndex][nodeYIndex];
	//
	if (moveGain > -Min)return false;
	insertNode(nodeU,nodeV);

	nbMoves++;
	searchComplete=false;
	s->updateRouteInfor(routeU->cour,nbMoves);
	if (routeU != routeV) s->updateRouteInfor(routeV->cour,nbMoves);
	s->updateLocationInfor(routeU->whichLocation->cour);
	if (routeU->whichLocation != routeV->whichLocation)
		s->updateLocationInfor(routeV->whichLocation->cour);
/*	s->outputSolution();
	s->isRight();
	s->evaluationDis();
	distt += moveGain;
	if (!data->equalTwoVar(distt,s->fit))std::cout<<"the distance is wrong"<<std::endl;
	*/
	return true;
}
bool LS::Relocate_two(){
	if (nodeUIndex == nodeYIndex || nodeXIndex == nodeVIndex || nodeX->isDepot) return false;

	if (!(nodeV->route->whichLocation->remainInven + nodeU->dem + nodeX->dem <= nodeV->route->whichLocation->inventory))// depot feasible
		return false;
	if(nodeV->route != nodeU->route && nodeV->route->load + nodeU->dem + nodeX->dem > data->vehCap)return false;// route feasible
	//
	moveGain = compute(nodeV->route->whichLocation->used, data->openCostLoc[nodeV->route->whichLocation->cour])
			+ compute(!(nodeV->isDepot && nodeY->isDepot), data->usingCostVeh)
			- compute(!(nodeU->route->whichLocation->splitTime == 1 && nodeU->route->nbClients == 2 && nodeU->route->whichLocation != nodeV->route->whichLocation), data->openCostLoc[nodeU->route->whichLocation->cour])
			- compute(nodeU->route->nbClients != 2, data->usingCostVeh)
			+ data->D[nodeUPrevIndex][nodeXNextIndex]-data->D[nodeUPrevIndex][nodeUIndex]-data->D[nodeXIndex][nodeXNextIndex]
			+ data->D[nodeVIndex][nodeXIndex]+data->D[nodeUIndex][nodeYIndex]-data->D[nodeVIndex][nodeYIndex];
	//
	if (moveGain>-Min)return false;
	insertNode(nodeX,nodeV);
	insertNode(nodeU,nodeX);

	nbMoves++;
	searchComplete=false;
	s->updateRouteInfor(routeU->cour,nbMoves);
	if (routeU != routeV) s->updateRouteInfor(routeV->cour,nbMoves);
	s->updateLocationInfor(routeU->whichLocation->cour);
	if (routeU->whichLocation != routeV->whichLocation)
		s->updateLocationInfor(routeV->whichLocation->cour);
/*	s->outputSolution();
	s->isRight();
	s->evaluationDis();
	distt += moveGain;
	if (!data->equalTwoVar(distt,s->fit))std::cout<<"the distance is wrong"<<std::endl;
	*/
	return true;
}
bool LS::Relocate_three(){
	if (nodeUIndex == nodeYIndex || nodeXIndex == nodeVIndex || nodeX->isDepot) return false;
	if (!(nodeV->route->whichLocation->remainInven + nodeU->dem + nodeX->dem <= nodeV->route->whichLocation->inventory))// depot feasible
		return false;
	if(nodeV->route != nodeU->route && nodeV->route->load + nodeU->dem + nodeX->dem > data->vehCap)return false;// route feasible
	//
	if (nodeU->route->whichLocation->splitTime == 1 && nodeU->route->nbClients == 1){//a depot is removed and a route is also removed.
		moveGain = -data->openCostLoc[nodeU->route->whichLocation->cour] - data->usingCostVeh;
		moveGain += data->D[nodeUPrevIndex][nodeXNextIndex]-data->D[nodeUPrevIndex][nodeUIndex]-data->D[nodeXIndex][nodeXNextIndex];
		moveGain += data->D[nodeVIndex][nodeXIndex]+data->D[nodeUIndex][nodeYIndex]-data->D[nodeVIndex][nodeYIndex];
	}
	else if (nodeU->route->nbClients == 2){
		moveGain = -data->usingCostVeh;
		moveGain += data->D[nodeUPrevIndex][nodeXNextIndex]-data->D[nodeUPrevIndex][nodeUIndex]-data->D[nodeXIndex][nodeXNextIndex];
		moveGain += data->D[nodeVIndex][nodeXIndex]+data->D[nodeUIndex][nodeYIndex]-data->D[nodeVIndex][nodeYIndex];
	}
	else{
		moveGain += data->D[nodeUPrevIndex][nodeXNextIndex]-data->D[nodeUPrevIndex][nodeUIndex]-data->D[nodeXIndex][nodeXNextIndex];
		moveGain += data->D[nodeVIndex][nodeXIndex]+data->D[nodeUIndex][nodeYIndex]-data->D[nodeVIndex][nodeYIndex];
	}
	if (moveGain>-Min)return false;
	insertNode(nodeX,nodeV);
	insertNode(nodeU,nodeX);

	nbMoves++;
	searchComplete=false;
	s->updateRouteInfor(routeU->cour,nbMoves);
	if (routeU != routeV) s->updateRouteInfor(routeV->cour,nbMoves);
	s->updateLocationInfor(routeU->whichLocation->cour);
	if (routeU->whichLocation != routeV->whichLocation)
		s->updateLocationInfor(routeV->whichLocation->cour);
/*	s->outputSolution();
	s->isRight();
	s->evaluationDis();
	distt += moveGain;
	if (!data->equalTwoVar(distt,s->fit))std::cout<<"the distance is wrong"<<std::endl;
	*/
	return true;
}
bool LS::Relocate_four(){//
	if (nodeUIndex == nodeVPrevIndex || nodeUIndex == nodeYIndex)return false;
	if (nodeU->route->whichLocation != nodeV->route->whichLocation)
		if ((nodeV->route->whichLocation->remainInven + nodeU->dem - nodeV->dem > nodeV->route->whichLocation->inventory) || (nodeU->route->whichLocation->remainInven + nodeV->dem - nodeU->dem > nodeU->route->whichLocation->inventory))return false;
	if (nodeU->route->load - nodeU->dem + nodeV->dem > data->vehCap || nodeV->route->load - nodeV->dem + nodeU->dem > data->vehCap)return false;

	moveGain=data->D[nodeUPrevIndex][nodeVIndex]+data->D[nodeVIndex][nodeXIndex]-data->D[nodeUPrevIndex][nodeUIndex]-data->D[nodeUIndex][nodeXIndex];
	moveGain+=data->D[nodeVPrevIndex][nodeUIndex]+data->D[nodeUIndex][nodeYIndex]-data->D[nodeVPrevIndex][nodeVIndex]-data->D[nodeVIndex][nodeYIndex];
	//
	if (moveGain > -Min)return false;
	swapNode(nodeU,nodeV);

	nbMoves++;
	searchComplete=false;
	s->updateRouteInfor(routeU->cour,nbMoves);
	if (routeU != routeV) s->updateRouteInfor(routeV->cour,nbMoves);
	s->updateLocationInfor(routeU->whichLocation->cour);
	if (routeU->whichLocation != routeV->whichLocation)
		s->updateLocationInfor(routeV->whichLocation->cour);
/*	s->outputSolution();
	s->isRight();
	s->evaluationDis();
	distt += moveGain;
	if (!data->equalTwoVar(distt,s->fit))std::cout<<"the distance is wrong"<<std::endl;
	*/
	return true;
}
bool LS::Relocate_five(){
	if (nodeU == nodeV->pre || nodeX == nodeV->pre || nodeU == nodeY || nodeX->isDepot) return false;
	if (nodeU->route->whichLocation != nodeV->route->whichLocation){
		if (nodeU->route->whichLocation->remainInven + nodeV->dem - nodeU->dem - nodeX->dem > nodeU->route->whichLocation->inventory)return false;
		if (nodeV->route->whichLocation->remainInven + nodeU->dem + nodeX->dem - nodeV->dem >nodeV->route->whichLocation->inventory)return false;
	}
	if (nodeU->route->load - nodeU->dem - nodeX->dem + nodeV->dem > data->vehCap || nodeV->route->load + nodeU->dem + nodeX->dem - nodeV->dem > data->vehCap)return false;

	moveGain=data->D[nodeUPrevIndex][nodeVIndex]+data->D[nodeVIndex][nodeXNextIndex]-data->D[nodeUPrevIndex][nodeUIndex]-data->D[nodeXIndex][nodeXNextIndex];
	moveGain+=data->D[nodeVPrevIndex][nodeUIndex]+data->D[nodeXIndex][nodeYIndex]-data->D[nodeVPrevIndex][nodeVIndex]-data->D[nodeVIndex][nodeYIndex];

	if (moveGain >-Min)return false;

	swapNode(nodeU, nodeV);
	insertNode(nodeX, nodeU);

	nbMoves++;
	searchComplete=false;
	s->updateRouteInfor(routeU->cour,nbMoves);
	if (routeU != routeV) s->updateRouteInfor(routeV->cour,nbMoves);
	s->updateLocationInfor(routeU->whichLocation->cour);
	if (routeU->whichLocation != routeV->whichLocation)
		s->updateLocationInfor(routeV->whichLocation->cour);
/*	s->outputSolution();
	s->isRight();
	s->evaluationDis();
	distt += moveGain;
	if (!data->equalTwoVar(distt,s->fit))std::cout<<"the distance is wrong"<<std::endl;
	*/
	return true;
}
bool LS::Relocate_six(){
	if (nodeX->isDepot || nodeY->isDepot || nodeY == nodeU->pre || nodeU == nodeY || nodeX == nodeV || nodeV == nodeX->next) return false;
	if (nodeU->route->whichLocation != nodeV->route->whichLocation){
		if (nodeU->route->whichLocation->remainInven + nodeV->dem + nodeY->dem - nodeU->dem - nodeX->dem > nodeU->route->whichLocation->remainInven)return false;
		if (nodeV->route->whichLocation->remainInven + nodeU->dem + nodeX->dem - nodeV->dem - nodeY->dem > nodeV->route->whichLocation->remainInven)return false;
	}
	if (nodeU->route->load - nodeU->dem - nodeX->dem + nodeV->dem + nodeY->dem > data->vehCap || nodeV->route->load - nodeV->dem - nodeY->dem + nodeU->dem + nodeX->dem > data->vehCap)return false;

	moveGain=data->D[nodeUPrevIndex][nodeVIndex]+data->D[nodeYIndex][nodeXNextIndex]-data->D[nodeUPrevIndex][nodeUIndex]-data->D[nodeXIndex][nodeXNextIndex];
	moveGain+=data->D[nodeVPrevIndex][nodeUIndex]+data->D[nodeXIndex][nodeYNextIndex]-data->D[nodeVPrevIndex][nodeVIndex]-data->D[nodeYIndex][nodeYNextIndex];

	if (moveGain > -Min)return false;
	swapNode(nodeU,nodeV);
	swapNode(nodeX,nodeY);

	nbMoves++;
	searchComplete=false;
	s->updateRouteInfor(routeU->cour,nbMoves);
	if (routeU != routeV) s->updateRouteInfor(routeV->cour,nbMoves);
	s->updateLocationInfor(routeU->whichLocation->cour);
	if (routeU->whichLocation != routeV->whichLocation)
		s->updateLocationInfor(routeV->whichLocation->cour);
/*	s->outputSolution();
	s->isRight();
	s->evaluationDis();
	distt += moveGain;
	if (!data->equalTwoVar(distt,s->fit))std::cout<<"the distance is wrong"<<std::endl;
	*/
	return true;
}
bool LS::Relocate_seven(){
	if (nodeX->isDepot || nodeY->isDepot || nodeY == nodeU->pre || nodeU == nodeY || nodeX == nodeV || nodeV == nodeX->next) return false;
	if (nodeU->route->whichLocation != nodeV->route->whichLocation){
		if (nodeU->route->whichLocation->remainInven + nodeV->dem + nodeY->dem - nodeU->dem - nodeX->dem > nodeU->route->whichLocation->remainInven)return false;
		if (nodeV->route->whichLocation->remainInven + nodeU->dem + nodeX->dem - nodeV->dem - nodeY->dem > nodeV->route->whichLocation->remainInven)return false;
	}
	if (nodeU->route->load - nodeU->dem - nodeX->dem + nodeV->dem + nodeY->dem > data->vehCap || nodeV->route->load - nodeV->dem - nodeY->dem + nodeU->dem + nodeX->dem > data->vehCap)return false;

	moveGain=data->D[nodeUPrevIndex][nodeYIndex]+data->D[nodeVIndex][nodeXNextIndex]-data->D[nodeUPrevIndex][nodeUIndex]-data->D[nodeXIndex][nodeXNextIndex];
	moveGain+=data->D[nodeVPrevIndex][nodeXIndex]+data->D[nodeUIndex][nodeYNextIndex]-data->D[nodeVPrevIndex][nodeVIndex]-data->D[nodeYIndex][nodeYNextIndex];

	if (moveGain > -Min)return false;

	swapNode(nodeX,nodeV);
	swapNode(nodeU,nodeY);

	nbMoves++;
	searchComplete=false;
	s->updateRouteInfor(routeU->cour,nbMoves);
	if (routeU != routeV) s->updateRouteInfor(routeV->cour,nbMoves);
	s->updateLocationInfor(routeU->whichLocation->cour);
	if (routeU->whichLocation != routeV->whichLocation)
		s->updateLocationInfor(routeV->whichLocation->cour);
/*	s->outputSolution();
	s->isRight();
	s->evaluationDis();
	distt += moveGain;
	if (!data->equalTwoVar(distt,s->fit))std::cout<<"the distance is wrong"<<std::endl;
	*/
	return true;
}
bool LS:: Relocate_eight(){
	if (nodeU->position >nodeV->position)return false;
	if (nodeU->next == nodeV)return false;
	moveGain=data->D[nodeUIndex][nodeVIndex]+data->D[nodeYIndex][nodeXIndex]-data->D[nodeUIndex][nodeXIndex]-data->D[nodeVIndex][nodeYIndex];
	if (moveGain > -Min)return false;
	Node * nodeNum = nodeX->next;
	nodeX->pre = nodeNum;
	nodeX->next = nodeY;

	while (nodeNum != nodeV){
		Node * temp = nodeNum->next;
		nodeNum->next = nodeNum->pre;
		nodeNum->pre = temp;
		nodeNum = temp;
	}

	nodeV->next = nodeV->pre;
	nodeV->pre = nodeU;
	nodeU->next = nodeV;
	nodeY->pre = nodeX;

	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	s->updateRouteInfor(routeU->cour,nbMoves);

/*	s->outputSolution();
	s->isRight();
	s->evaluationDis();
	distt += moveGain;
	if (!data->equalTwoVar(distt,s->fit))std::cout<<"the distance is wrong"<<std::endl;
	*/
	return true;
}
bool LS::Relocate_nine(){
	if (routeU->whichLocation != routeV->whichLocation){
		if (routeU->whichLocation->remainInven + nodeV->cumulatedLoad - (routeU->load - nodeU->cumulatedLoad) > routeU->whichLocation->inventory)
			return false;
		if (routeV->whichLocation->remainInven + (routeU->load - nodeU->cumulatedLoad) - nodeV->cumulatedLoad > routeV->whichLocation->inventory)
			return false;
	}
	if (nodeU->cumulatedLoad + nodeV->cumulatedLoad >= data->vehCap)return false;
	if (routeU->load - nodeU->cumulatedLoad + routeV->load - nodeV->cumulatedLoad >= data->vehCap)return false;
//	if (nodeX->isDepot && nodeV->isDepot)return false;
	Node *lastU = nodeU->route->endDepot->pre;
	Node *beginV =nodeV->route->depot->next;
	moveGain = compute(nodeV->route->whichLocation->used, data->openCostLoc[nodeV->route->whichLocation->cour])
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

	moveGain += distanceMoveGain;

	if (moveGain > -Min)return false;

//	s->outputSolution();

	Node * depotV = routeV->depot;

	Node * temp;
	Node * xx = nodeX;
	Node * vv = nodeV;

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

	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	s->updateRouteInfor(routeU->cour,nbMoves);
	s->updateRouteInfor(routeV->cour,nbMoves);
	s->updateLocationInfor(routeU->whichLocation->cour);
	if (routeU->whichLocation != routeV->whichLocation)
		s->updateLocationInfor(routeV->whichLocation->cour);
/*	s->outputSolution();
	s->isRight();
	s->evaluationDis();
	distt += moveGain;
	if (!data->equalTwoVar(distt,s->fit)){
		std::cout<<"the distance is wrong"<<std::endl;
		exit(0);
	}
	*/
	return true;
}
bool LS::Relocate_ten(){
	if (routeU->whichLocation != routeV->whichLocation){
		if (routeU->whichLocation->remainInven + (routeV->load - nodeV->cumulatedLoad) - (routeU->load - nodeU->cumulatedLoad) > routeU->whichLocation->inventory)
			return false;
		if (routeV->whichLocation->remainInven + (routeU->load - nodeU->cumulatedLoad) - (routeV->load - nodeV->cumulatedLoad) > routeV->whichLocation->inventory)
			return false;
	}
	if (nodeU->cumulatedLoad + routeV->load - nodeV->cumulatedLoad > data->vehCap)return false;
	if (routeU->load - nodeU->cumulatedLoad + nodeV->cumulatedLoad > data->vehCap)return false;
//	if (nodeX->isDepot && nodeY->isDepot)return false;
	moveGain = compute(nodeV->route->whichLocation->used, data->openCostLoc[nodeV->route->whichLocation->cour])
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

	moveGain += distanceMoveGain;
	if (moveGain > -Min)return false;
//	s->outputSolution();

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

	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	s->updateRouteInfor(routeU->cour,nbMoves);
	s->updateRouteInfor(routeV->cour,nbMoves);
	s->updateLocationInfor(routeU->whichLocation->cour);
	if (routeU->whichLocation != routeV->whichLocation)
		s->updateLocationInfor(routeV->whichLocation->cour);
/*	s->outputSolution();
	s->isRight();
	s->evaluationDis();
	distt += moveGain;
	if (!data->equalTwoVar(distt,s->fit)){
		std::cout<<"the distance is wrong"<<std::endl;
		exit(0);
	}
	*/
	return true;
}
bool LS::two_opt_starR(){// distinguish all cases
	if (routeU == routeV) return false;
	locU = routeU->whichLocation;
	locV = routeV->whichLocation;
	if (locU == locV){// two routes come from the same location
		double c1 = data->D[nodeXIndex][nodeYIndex] - data->D[nodeUIndex][nodeXIndex];
	//	if (c1 > 0 && (!nodeX->isDepot && !nodeY->isDepot)) return false;// the second condition is used to decide whether a route is eliminated
		if (routeU->load + nodeV->cumulatedLoad - (routeU->load - nodeU->cumulatedLoad) > data->vehCap) return false;
		if (routeV->load - nodeV->cumulatedLoad + (routeU->load - nodeU->cumulatedLoad) > data->vehCap) return false;
		double c2 = data->D[nodeUIndex][nodeVIndex] - data->D[nodeVIndex][nodeYIndex];
		if (nodeX->isDepot && nodeY->isDepot)
			c2 = c2 - data->usingCostVeh;// a route is removed
		moveGain = c1 + c2;
		if (moveGain < 0){
			Node * temp;
			Node * xx = nodeX;
			Node * vv = nodeV;
			Node * endU = routeU->endDepot->pre;
			Node * beginV = routeV->depot->next;// there is definitely a beginV;
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
			nodeU->next = nodeV;
			nodeV->pre = nodeU;
			beginV->next = routeU->endDepot;
			routeU->endDepot->pre = beginV;
			//
			if (!nodeX->isDepot){// two cases
				routeV->depot->next = endU;
				endU->pre = routeV->depot;
				nodeX->next = nodeY;
				nodeY->pre = nodeX;
			}
			else{
				routeV->depot->next = nodeY;
				nodeY->pre = routeV->depot;
			}
		}
		else return false;
	}
	else{
		Node * nodeVPre = nodeV->pre;
		while(1){
			if (!nodeX->isDepot && !nodeV->isDepot){
				Node *nodeUE = routeU->endDepot->pre;
				Node *nodeVB = routeV->depot->next;
				double c1 = data->D[nodeUIndex][nodeVIndex] - data->D[nodeUIndex][nodeXIndex]
					+ data->D[nodeVB->cour][routeU->depot->cour] - data->D[routeV->depot->cour][nodeVB->cour];
	//			if (c1 > 0) return false;
				if (nodeU->cumulatedLoad + nodeV->cumulatedLoad > data->vehCap)return false;
				if (routeU->load - nodeU->cumulatedLoad + routeV->load - nodeV->cumulatedLoad > data->vehCap)return false;
				if (locU->remainInven + nodeV->cumulatedLoad - routeU->load - nodeU->cumulatedLoad > locU->inventory)return false;
				if (locV->remainInven + (routeU->load - nodeU->cumulatedLoad) - nodeV->cumulatedLoad) return false;
				double c2 = data->D[nodeXIndex][nodeYIndex] - data->D[nodeVIndex][nodeYIndex]
					+ data->D[nodeVB->cour][routeU->depot->cour] - data->D[nodeUE->cour][routeU->depot->cour];
				moveGain = c1 + c2;
				if (moveGain < 0){
					Node * temp;
					Node * xx = nodeX;
					Node * vv = nodeV;

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
					nodeU->next = nodeV;
					nodeV->pre = nodeU;
					nodeX->next = nodeY;
					nodeY->pre = nodeX;
					routeV->depot->next = nodeUE;
					nodeUE->pre = routeV->depot;
					nodeVB->next = routeU->endDepot;
					routeU->endDepot->pre = nodeVB;
				}
				break;
			}
			if (!nodeU->isDepot && !nodeY->isDepot){
				Node * nodeVE = routeV->endDepot->pre;
				Node * nodeUB = routeU->depot->next;
				double c1 = data->D[nodeUIndex][nodeVIndex] - data->D[nodeUIndex][nodeXIndex]
					+ data->D[nodeUB->cour][routeV->depot->cour] - data->D[nodeVE->cour][routeV->depot->cour];
	//			if (c1 > 0) return false;
				if (nodeU->cumulatedLoad + nodeV->cumulatedLoad > data->vehCap)return false;
				if (routeU->load - nodeU->cumulatedLoad + routeV->load - nodeV->cumulatedLoad > data->vehCap)return false;
				if (locU->remainInven + routeV->load - nodeV->cumulatedLoad - nodeU->cumulatedLoad > locU->inventory)return false;
				if (locV->remainInven + nodeU->cumulatedLoad -(routeV->load - nodeV->cumulatedLoad)) return false;
				double c2 = data->D[nodeXIndex][nodeYIndex] - data->D[nodeVIndex][nodeYIndex]
					+ data->D[nodeVE->cour][routeU->depot->cour] - data->D[nodeUB->cour][routeU->depot->cour];
				moveGain = c1 + c2;
				if (moveGain < 0){
					Node * temp;
					Node * xx = nodeX;
					Node * vv = nodeU;
					//
					while (!xx->isDepot){
						temp = xx->next;
						xx->next = xx->pre;
						xx->pre = temp;
						xx = temp;
					}
					xx = nodeY;
					while(!xx->isDepot){
						xx->route = routeU;
						xx = xx->next;
					}
					while (!vv->isDepot){
						temp = vv->pre;
						vv->pre = vv->next;
						vv->next = temp;
						vv->route = routeV;
						vv = temp;
					}
					nodeU->pre = nodeV;
					nodeV->next = nodeU;
					nodeUB->next = routeV->endDepot;
					routeV->endDepot->pre = nodeUB;


					nodeX->next = nodeY;
					nodeY->pre = nodeX;
					nodeVE->pre = routeU->depot;
					routeU->depot->next = nodeVE;
				}
				break;
			}
			if (nodeY->isDepot && !nodeU->isDepot){
				Node *nodeUB = routeU->depot->next;
				double c1 = data->D[nodeUIndex][nodeVIndex] - data->D[nodeUIndex][nodeXIndex]
					+ data->D[routeU->depot->cour][nodeXIndex] - data->D[routeU->depot->cour][nodeUB->cour];
				if (c1 > 0 && !nodeX->isDepot)return false;
				if (routeV->load + nodeU->cumulatedLoad > data->vehCap)return false;
				if (locV->remainInven + nodeU->cumulatedLoad > locV->inventory)return false;
				double c2 = data->D[nodeUB->cour][routeV->depot->cour] - data->D[nodeV->cour][routeV->depot->cour];
				if (nodeX->isDepot)
					c2 = c2 - data->usingCostVeh;
				if (locU->splitTime == 1 && nodeX->isDepot)
					c2 = c2 - data->openCostLoc[locU->cour];
				moveGain = c1 + c2;
				if (moveGain < 0){
					Node * temp;
					Node * vv = nodeU;
					while (!vv->isDepot){
						temp = vv->pre;
						vv->pre = vv->next;
						vv->next = temp;
						vv->route = routeV;
						vv = temp;
					}
					nodeV->next = nodeU;
					nodeU->pre = nodeV;
					nodeUB->next = routeV->endDepot;
					routeV->endDepot->pre = nodeUB;
					routeU->depot->next = nodeX;
					nodeX->pre = routeU->depot;
				}
				break;
			}
			if (nodeVPre->isDepot && !nodeU->isDepot){
				Node *nodeUE = routeU->endDepot->pre;
				double c1 = data->D[nodeUIndex][nodeVIndex] - data->D[nodeUIndex][nodeUPrevIndex]
					+ data->D[routeU->depot->cour][nodeU->pre->cour] - data->D[routeU->depot->cour][nodeUE->cour];
				if (c1 > 0 && !nodeU->pre->isDepot)return false;
				if (routeV->load + routeU->load - nodeU->pre->cumulatedLoad > data->vehCap)return false;
				if (locV->remainInven + routeU->load - nodeU->pre->cumulatedLoad > locV->inventory)return false;
				double c2 = data->D[nodeUE->cour][routeV->depot->cour] - data->D[nodeV->cour][routeV->depot->cour];
				if (nodeU->pre->isDepot)
					c2 = c2 - data->usingCostVeh;
				if (locU->splitTime == 1 && nodeU->pre->isDepot)
					c2 = c2 - data->openCostLoc[locU->cour];
				moveGain = c1 + c2;
				if (moveGain < 0){
					Node * temp;
					Node * vv = nodeU;
					while (!vv->isDepot){
						temp = vv->next;
						vv->pre = vv->next;
						vv->next = temp;
						vv->route = routeV;
						vv = temp;
					}
					Node * nodeUPre = nodeU->pre;
					nodeU->next = nodeV;
					nodeV->pre = nodeU;
					nodeUPre->next = routeU->endDepot;
					routeU->endDepot->pre = nodeUPre;
					routeV->depot->next = nodeUE;
					nodeUE->pre = routeV->depot;
				}
				break;
			}
		}
	}
	if (moveGain >= 0)return false;
	//
	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	s->updateRouteInfor(routeU->cour,nbMoves);
	s->updateRouteInfor(routeV->cour,nbMoves);
	s->updateLocationInfor(routeU->whichLocation->cour);
	if (locU->cour != locV->cour)
		s->updateLocationInfor(routeV->whichLocation->cour);
//	s->outputSolution();
	s->evaluationDis();
	s->isRight();
	return true;
}
bool LS::two_opt_star(){// distinguish all cases, there are four cases.
	if (routeU == routeV || (nodeX->isDepot && nodeY->isDepot))return false;
	locU = routeU->whichLocation;
	locV = routeV->whichLocation;
	if (locU == locV){// two routes come from the same location
		double c1 = data->D[nodeVPrevIndex][nodeXIndex] - data->D[nodeUIndex][nodeXIndex];
	//	if (c1 > 0 && (!nodeX->isDepot || !nodeV->pre->isDepot))return false;// if there is no route eliminated
		// decide the feasibility
		if (routeU->load - (routeU->load - nodeU->cumulatedLoad) + (routeV->load - nodeV->pre->cumulatedLoad) > data->vehCap)return false;
		if (routeV->load - (routeV->load - nodeV->pre->cumulatedLoad) + (routeU->load - nodeU->cumulatedLoad) > data->vehCap)return false;
		double c2 = data->D[nodeUIndex][nodeVIndex] - data->D[nodeVPrevIndex][nodeVIndex];
		if (nodeX->isDepot && nodeV->pre->isDepot)
			c2 = c2 - data->usingCostVeh;// a route is removed
		moveGain = c1 + c2;
		if (moveGain < 0){// to exchange two substrings
			Node * nodeVP = nodeV->pre;
			nodeU->next = nodeV;
			nodeV->pre = nodeU;
			Node *Uend = routeU->endDepot->pre;
			Node *Vend = routeV->endDepot->pre;
			if (nodeX->isDepot){
				nodeVP->next = routeV->endDepot;
				routeV->endDepot->pre = nodeVP;
			}
			else{
				nodeVP->next = nodeX;
				nodeX->pre = nodeVP;
				Uend->next = routeV->endDepot;
				routeV->endDepot->pre = Uend;
			}
			Vend->next = routeU->endDepot;
			routeU->endDepot->pre = Vend;
			//
			Node *p = nodeX;
			while(!p->isDepot){
				p->route = routeV;
				p = p->next;
			}
			p = nodeV;
			while(!p->isDepot){
				p->route = routeU;
				p = p->next;
			}
		}
		else return false;
	}
	else{// two opt and there are two many cases and we can list it,,,,,,,,,,the costs of routes and locations
		Node * nodeVPre = nodeV->pre;
		while(1){
			if (!nodeU->isDepot && !nodeVPre->isDepot && !nodeX->isDepot){//the first case
				Node * nodeUB = routeU->depot->next;// the begin node in route U
				Node * nodeVB = routeV->depot->next;// the begin node in route V
				double c1 = data->D[nodeXIndex][nodeVPrevIndex] - data->D[nodeUIndex][nodeXIndex]
					+ data->D[routeV->depot->cour][nodeUB->cour] - data->D[routeU->depot->cour][nodeUB->cour];
	//			if (c1 > 0)return false;// refuse the operation
				if (routeU->load + nodeV->pre->cumulatedLoad - nodeU->cumulatedLoad > data->vehCap) return false;// route capacity
				if (routeV->load - nodeV->pre->cumulatedLoad + nodeU->cumulatedLoad > data->vehCap) return false;// location capacity
				if (locU->remainInven + nodeV->pre->cumulatedLoad - nodeU->cumulatedLoad > locU->inventory) return false;
				if (locV->remainInven - nodeV->pre->cumulatedLoad + nodeU->cumulatedLoad > locV->inventory) return false;
				double c2 = data->D[nodeUIndex][nodeVIndex] - data->D[nodeVPrevIndex][nodeVIndex]
					+ data->D[routeU->depot->cour][nodeVB->cour] - data->D[routeV->depot->cour][nodeVB->cour];
				moveGain = c1 + c2;
				if (moveGain < 0){
					Node *p = nodeUB;
					while (p != nodeX){
						p->route = routeV;
						p = p->next;
					}
					p = nodeVB;
					while (p != nodeV){
						p->route = routeU;
						p = p->next;
					}
					//
					routeU->depot->next = nodeVB;
					nodeVB->pre = routeU->depot;
					routeV->depot->next = nodeUB;
					nodeUB->pre = routeV->depot;
					nodeU->next = nodeV;
					nodeV->pre = nodeU;
					nodeX->pre = nodeVPre;
					nodeVPre->next = nodeX;
				}
				break;
			}
			if (!nodeX->isDepot && !nodeV->isDepot && !nodeVPre->isDepot){
				Node * nodeUE = routeU->endDepot->pre;// the end node in route U
				Node * nodeVE = routeV->endDepot->pre;// the end node in route V
				double c1 = data->D[nodeXIndex][nodeVPrevIndex] - data->D[nodeUIndex][nodeXIndex]
					+ data->D[nodeUE->cour][routeV->depot->cour] - data->D[nodeUE->cour][routeU->depot->cour];
	//			if (c1 > 0)return false;// refuse the operation
				if (routeU->load + (routeV->load - nodeV->pre->cumulatedLoad) - (routeU->load - nodeU->cumulatedLoad) > data->vehCap) return false;// route capacity
				if (routeV->load - (routeV->load - nodeV->pre->cumulatedLoad) + (routeU->load - nodeU->cumulatedLoad) > data->vehCap) return false;// location capacity
				if (locU->remainInven + (routeV->load - nodeV->pre->cumulatedLoad) - (routeU->load - nodeU->cumulatedLoad) > locU->inventory) return false;
				if (locV->remainInven - (routeV->load - nodeV->pre->cumulatedLoad) + (routeU->load - nodeU->cumulatedLoad) > locV->inventory) return false;
				double c2 = data->D[nodeUIndex][nodeVIndex] - data->D[nodeVPrevIndex][nodeVIndex]
					+ data->D[routeU->depot->cour][nodeVE->cour] - data->D[routeV->depot->cour][nodeVE->cour];
				moveGain = c1 + c2;
				if (moveGain < 0){
					Node *p = nodeX;
					while (!p->isDepot){
						p->route = routeV;
						p = p->next;
					}
					p = nodeV;
					while (p->isDepot){
						p->route = routeU;
						p = p->next;
					}
					routeU->depot->pre = nodeVE;
					nodeVE->pre = routeU->depot;
					routeV->depot->next = nodeUE;
					nodeUE->pre = routeV->depot;
					nodeU->next = nodeV;
					nodeV->pre = nodeU;
					nodeX->pre = nodeVPre;
					nodeVPre->next = nodeX;
				}
				break;
			}
			if (!nodeU->isDepot && nodeVPre->isDepot){
				Node * nodeUB = routeU->depot->next;// the begin node in route V
				double c1 = data->D[nodeUIndex][nodeVIndex] - data->D[nodeUIndex][nodeXIndex];
				if (c1 > 0 && !nodeX->isDepot)return false;// refuse the operation
				if (routeV->load + nodeU->cumulatedLoad > data->vehCap) return false;// location capacity
				if (locV->remainInven + nodeU->cumulatedLoad > locV->inventory)return false;
				double c2 = data->D[routeU->depot->cour][nodeXIndex] - data->D[routeU->depot->cour][nodeUB->cour]
					+ data->D[routeV->depot->cour][nodeUB->cour] - data->D[routeV->depot->cour][nodeV->cour];
				if (nodeX->isDepot)
					c2 = c2 - data->usingCostVeh;
				if (locU->splitTime == 1 && nodeX->isDepot)
					c2 = c2 - data->openCostLoc[locU->cour];
				moveGain = c1 + c2;
				if (moveGain < 0){
					routeU->depot->next = nodeX;
					nodeX->pre = routeU->depot;
					routeV->depot->next = nodeUB;
					nodeUB->pre = routeV->depot;
					nodeU->next = nodeV;
					nodeV->pre = nodeU;
					//
					Node *p = nodeUB;
					while (p != nodeV){
						p->route = routeV;
						p = p->next;
					}
				}
				break;
			}
			if (nodeX->isDepot && !nodeV->isDepot){
				Node * nodeVE = routeV->endDepot->pre;// the begin node in route V
				double c1 = data->D[nodeVIndex][nodeUIndex] - data->D[nodeVPrevIndex][nodeVIndex];
				if (c1 > 0 && !nodeV->pre->isDepot)return false;// refuse the operation
				if (routeU->load + routeV->load - nodeVPre->cumulatedLoad > data->vehCap) return false;// location capacity
				if (locU->remainInven + routeV->load - nodeVPre->cumulatedLoad > locU->inventory) return false;
				double c2 = data->D[nodeVPrevIndex][routeV->depot->cour] - data->D[routeV->depot->cour][nodeVE->cour]
					+ data->D[nodeVE->cour][routeU->depot->cour] - data->D[nodeUIndex][routeU->depot->cour];
				if (nodeV->pre->isDepot)
					c2 = c2 - data->usingCostVeh;
				if (locU->splitTime == 1 && nodeV->pre->isDepot)
					c2 = c2 - data->openCostLoc[locU->cour];
				moveGain = c1 + c2;
				if (moveGain < 0){
		//			s->outputSolution();
					nodeVPre->next = routeV->endDepot;
					routeV->endDepot->pre = nodeVPre;
					nodeU->next = nodeV;
					nodeV->pre = nodeU;
					nodeVE->next = routeU->endDepot;
					routeU->endDepot->pre = nodeVE;
					//
					Node *p = nodeV;
					while (!p->isDepot){
						p->route = routeU;
						p = p->next;
					}
		//			s->outputSolution();
				}
				break;
			}
		}
	}
	if (moveGain >= 0)return false;
	//
	s->updateRouteInfor(routeU->cour,nbMoves);
	s->updateRouteInfor(routeV->cour,nbMoves);
	s->updateLocationInfor(locU->cour);
	if (locV != locU)
		s->updateLocationInfor(locV->cour);
	nbMoves++;
	searchComplete=false;
//	s->outputSolution();
	s->evaluationDis();
	s->isRight();
	return true;
}
void LS::setLocalVariableRouteU(){
	routeU=nodeU->route;
	nodeX=nodeU->next;
	nodeXNextIndex=nodeX->next->cour;
	nodeUIndex = nodeU->cour;
	nodeUPrevIndex = nodeU->pre->cour;
	nodeXIndex = nodeX->cour;
	loadU=nodeU->dem;
	loadX=nodeX->dem;
}
void LS::setLocalVariableRouteV(){
	routeV=nodeV->route;
	nodeY=nodeV->next;
	nodeYNextIndex=nodeY->next->cour;//the next two city.  U-X-this
	nodeVIndex=nodeV->cour;
	nodeVPrevIndex=nodeV->pre->cour;
	nodeYIndex=nodeY->cour;
	loadV=nodeV->dem;
	loadY=nodeY->dem;
}
void LS::insertNode(Node *U, Node *V){
	U->pre->next = U->next;
	U->next->pre = U->pre;
	V->next->pre = U;
	U->pre = V;
	U->next = V->next;
	V->next = U;
	U->route = V->route;
}
void LS::swapNode(Node *U, Node *V){
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
void LS::loadSolution(){
	nbMoves = 0;
	for (int i=0;i<numV;i++)
		s->client[i].whenLastTestedRI = -1;
	for (int i=numV;i<numC;i++){
		for (int j=0;j<s->location[i-numV].maxVisit; j++){
			s->location[i-numV].depots[j].route->whenLastModified = -1;
		}
	}
}
void LS::local_search_run(Individual *s,double penaltyDepot, double penaltyVeh){
	this->penaltyDepot = penaltyDepot;
	this->penaltyDepot=Max;// no infeasible
	this->penaltyVeh = penaltyVeh;
	this->penaltyVeh = Max;
	this->s=s;
//	s->isRight();
	loadSolution();
	srun();
	s->evaluationDis();
	s->isRight();

//	s->outputSolution();



	int totalInv = 0;
	std::set<std::pair<int, int>>order;

	for (int i=0;i<numL;i++){
		if (s->location[i].used){
			order.insert({s->location[i].inventory,i});
			totalInv += s->location[i].inventory;
		}
	}
	if (totalInv > data->totalDemand){
		auto it = order.begin();
		if (totalInv - it->first > data->totalDemand)
			data->loopp++;
	}


}

/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
void LS::removeLocation(int removeLoc){
	for (int i=0;i<numV;i++)auxArray1[i]=0;
	Node *p;
	for (int i=0;i<s->location[removeLoc].maxVisit;i++){
		if (s->location[removeLoc].depots[i].route->used){
			p = s->location[removeLoc].depots[i].next;
			while(!p->isDepot){
				listRemove.push_back(p->cour);
				p = p->next;
			}
			s->location[removeLoc].depots[i].next = &s->location[removeLoc].endDepots[i];
			s->location[removeLoc].endDepots[i].pre = &s->location[removeLoc].depots[i];
			s->updateRouteInfor(s->location[removeLoc].depots[i].route->cour, -1);
		}
	}
	int ll = removeLoc + numV;
	s->updateLocationInfor(ll);
	//
	for (int i=0;i<(int)listRemove.size();i++){
		p = &s->client[listRemove[i]];
		p->pre = NULL;
		p->next = NULL;
		auxArray1[p->cour] = 1;
	}
	//
	random_shuffle(listRemove.begin(), listRemove.end());// randomize all removed customers
	double delta, min_delta;
	Node *node, *nodeU, *pos;bool isFirst;
	while ((int)listRemove.size() != 0){
		min_delta = Max; //listRemove
		pos = NULL;
		node = &s->client[listRemove[0]];
		for (int i=0;i<numL;i++){
			if (s->location[i].used){
				for (int j=0;j<s->location[i].maxVisit;j++){
					if (s->location[i].depots[j].route->used && (s->location[i].depots[j].route->load + node->dem <= data->vehCap) && (s->location[i].remainInven + node->dem <= s->location[i].inventory)){
						nodeU = &s->location[i].depots[j]; isFirst = true;
						while(isFirst || !nodeU->isDepot){
							delta = data->D[nodeU->cour][node->cour] + data->D[node->cour][nodeU->next->cour] - data->D[nodeU->cour][nodeU->next->cour];
							if (min_delta > delta){
								min_delta = delta;
								pos = nodeU;
							}
							nodeU = nodeU->next;
							isFirst = false;
						}
					}
				}
			}
		}
		//
		pos->next->pre = node;
		node->next = pos->next;
		pos->next = node;
		node->pre = pos;
		node->route = pos->route;
		//
		auxArray1[node->cour] = 0;
		listRemove.erase(listRemove.begin());
		s->updateRouteInfor(pos->route->cour,-1);
		s->updateLocationInfor(pos->route->whichLocation->cour);
	}
}
void LS::removeRoute(int removeRou){
	for (int i=0;i<numV;i++)auxArray1[i] = 0;
	Node *p = s->route[removeRou].depot->next;
	while(!p->isDepot){
		listRemove.push_back(p->cour);
		p = p->next;
	}
	s->route[removeRou].depot->next = s->route[removeRou].endDepot;
	s->route[removeRou].endDepot->pre = s->route[removeRou].depot;
	s->updateRouteInfor(removeRou, -1);
	int ll = s->route[removeRou].whichLocation->cour;
	s->updateLocationInfor(ll);
	//
	for (int i=0;i<(int)listRemove.size();i++){
		p = &s->client[listRemove[i]];
		p->pre = NULL;
		p->next = NULL;
		p->route = NULL;
		auxArray1[p->cour] = 1;
	}
	random_shuffle(listRemove.begin(), listRemove.end());// randomize all removed customers
	double delta, min_delta;
	Node *node, *nodeU, *pos;
	while ((int)listRemove.size() != 0){
		min_delta = Max; //listRemove
		pos = NULL;
		node = &s->client[listRemove[0]];
		for (int i=0;i<(int)data->edgeNode[node->cour].size();i++){
			if (data->edgeNode[node->cour][i] < numV){
				nodeU = &s->client[data->edgeNode[node->cour][i]];
				if (nodeU->route != NULL){
					if (nodeU->route->load + node->dem <= data->vehCap && nodeU->route->whichLocation->remainInven + node->dem <= nodeU->route->whichLocation->inventory){
						delta = data->D[nodeU->cour][node->cour] + data->D[node->cour][nodeU->next->cour] - data->D[nodeU->cour][nodeU->next->cour];
						if (delta < min_delta){
							min_delta = delta;
							pos = nodeU;
						}
					}
				}
			}
		}
		if (min_delta == Max){
			for (int i=0;i<(int)data->edgeNode[node->cour].size();i++){
				if (data->edgeNode[node->cour][i] < numV){
					nodeU = &s->client[data->edgeNode[node->cour][i]];
					if (nodeU->route != NULL){
						delta = data->D[nodeU->cour][node->cour] + data->D[node->cour][nodeU->next->cour] - data->D[nodeU->cour][nodeU->next->cour];
						if (delta < min_delta){
							min_delta = delta;
							pos = nodeU;
						}
					}
				}
			}
			if (min_delta == Max){
				for (int i=0;i<numV;i++){
					if (s->client[i].route != NULL){
						nodeU = &s->client[i];
						delta = data->D[nodeU->cour][node->cour] + data->D[node->cour][nodeU->next->cour] - data->D[nodeU->cour][nodeU->next->cour];
						if (delta < min_delta){
							min_delta = delta;
							pos = nodeU;
						}
					}
				}
			}
		}
		//
		if (min_delta == Max)std::cout<<"report a bug at removing routes"<<std::endl;
//		s->outputSolution();
		pos->next->pre = node;
		node->next = pos->next;
		pos->next = node;
		node->pre = pos;
		node->route = pos->route;
		//
		auxArray1[node->cour] = 0;
		listRemove.erase(listRemove.begin());
		s->updateRouteInfor(pos->route->cour,-1);
		s->updateLocationInfor(pos->route->whichLocation->cour);
	}
	/***************************/
	//repair the infeasible solution
	s->evaluationDis();
	repair->repairForInitialSolution(s);
}
void LS::removeRoutesAndLocations(Individual *s){
	//
	this->s = s;
	s->getSaturationDegree();
//	s->outputSolution();
/*	int sumLoadLoc;
	while(1){
		sumLoadLoc = 0;locSat.clear();
		for (int i=0;i<numL;i++){
			if (s->location[i].used){
				sumLoadLoc += s->location[i].inventory;
				locSat.push_back({(double)s->location[i].remainInven / (double)s->location[i].inventory, i});
			}
		}
		std::sort(locSat.begin(),locSat.end());
		//
		if (sumLoadLoc - s->location[locSat[0].second].inventory >= data->totalDemand){// less saturated
			s->outputSolution();
			removeLocation(locSat[0].second);
		}
		else break;
	}
	*/
	int tarRoute;
	int sumLoadRou;
	while(1){// existing non-saturated routes
		auto it = s->saturatRoute.begin();
		if (it->first < 0.7){// saturated parameter
			tarRoute = it->second;// the order of the route.
			sumLoadRou = ((int)s->saturatRoute.size() - 1) * data->vehCap;
			if (sumLoadRou > 1. * data->totalDemand)
				removeRoute(tarRoute);
		}
		else
			break;
		break;
		s->getSaturationDegree();
	}
	s->decideFeasible();
	if (!s->isFeasible)return;
	s->evaluationDis();
	s->isRight();

}
