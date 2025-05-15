/*
 * initialsol.cpp
 *
 *  Created on: 17 Apr 2020
 *      Author: Pengfei He
 */
#include "basic.h"
#include "initialsol.h"
initial_sol::initial_sol(read_data * data, RepairProcedure *repair) {
	this->repair = repair;
	this->data=data;
	this->numV = data->numV;
	this->numL = data->numL;
	this->numC = data->numC;
	customer=new int [numV];
	for (int i=0;i<numV;i++)
		sque.push_back(i);
	locUsed = new bool [numC];
	cusUsed = new bool [numV];
}
initial_sol::~initial_sol() {
	delete [] customer;
	delete [] locUsed;
	delete [] cusUsed;
}
void initial_sol::insertCustomerToLocation(int &customer, int &loc, int newCus){
	Node *U = &s->client[customer];
	if (newCus > 0){
		Node *V = &s->client[newCus];
		insertClient(U, V);
		s->updateRouteInfor(U->route->cour,-1);
		s->updateLocationInfor(U->route->whichLocation->cour);
		return;
	}
	for (int i=0;i<s->location[loc].maxVisit; i++){
		if (!(s->location[loc].depots[i].route->load + data->demCus[customer] > data->vehCap)){
			Node *p= s->location[loc].depots[i].next;
			if (p != NULL){
				Node *V = NULL;
				double delta=Max;
				double tem =  data->D[customer][s->location[loc].cour] + data->D[customer][p->cour] - data->D[p->cour][s->location[loc].cour];
				if ( tem < delta){
					delta = tem;
					V=&s->location[loc].depots[i];
				}
				while(1){
					p=p->next;
					if (p->next->isDepot)break;
					tem = data->D[p->pre->cour][customer] + data->D[customer][p->cour] - data->D[p->pre->cour][p->cour];
					if (tem < delta){
						delta = tem;
						V = p->pre;
					}
				}
		//		tem = data->D[p->cour][customer] + data->D[customer][s->location[loc].cour] - data->D[p->cour][s->location[loc].cour];
		//		if (tem < delta){
		//			delta = tem;
		//			V = p;
		//		}
				insertClient(U, V);
				s->updateRouteInfor(U->route->cour,-1);
				s->updateLocationInfor(U->route->whichLocation->cour);
			}
			else{// open a new route
				Node *V = &s->location[loc].depots[i];
				insertClient(U, V);
				U->route->used = true;
				s->location[loc].used = true;
				s->location[loc].splitTime++;
				s->updateRouteInfor(U->route->cour,-1);
				s->updateLocationInfor(U->route->whichLocation->cour);
			}
			return;
		}
	}
}
void initial_sol::greedyInsert(){
	s->initilization();
	for (int i=0; i<numV; i++)	customer[i]=0;
	std::random_shuffle(sque.begin(),sque.end());// to random all customers
	int cus;
	for (int i=0;i<numV;i++){
		cus=sque[i];
		if (! customer[cus]){
			int whichLocation = -1;// three conditions: if the location is open and has enough free capacity, if all locations are closed.
			if ((int)data->customerToLoc[cus].size() > 0){
				// find the most near depot.
				for (int j = 0; j< (int)data->customerToLoc[cus].size(); j++){
					int location = data->customerToLoc[cus][j] - numV;
					if (!s->location[location].used && whichLocation < 0){
						whichLocation = location;
						break;
					}
					if (s->location[location].used){
						if (s->location[location].remainInven + data->demCus[cus] > s->location[location].inventory)
							continue;
						else{
							whichLocation = location;
							break;
						}
					}
				}
				insertCustomerToLocation(cus, whichLocation, -1);
				customer[cus] = 1;
				int currentCus = cus;
				int targetCus;
				while(1){
					int j;
					for (j=0; j<(int)data->edgeNode[currentCus].size();j++){
						targetCus = data->edgeNode[currentCus][j];
						if (targetCus >= numV) continue;// target the depot
						if (customer[targetCus] == 0){
							if (s->client[currentCus].route->whichLocation->remainInven + data->demCus[targetCus] < s->client[currentCus].route->whichLocation->inventory){
								if (s->client[currentCus].route->load + data->demCus[targetCus] < data->vehCap){
									insertCustomerToLocation(targetCus, whichLocation, currentCus);
									customer[targetCus] = 1;
									currentCus = targetCus;
									break;
								}
								else{
									if (std::find(data->customerToLoc[targetCus].begin(), data->customerToLoc[targetCus].end(), whichLocation+numV) != data->customerToLoc[targetCus].end()){
										insertCustomerToLocation(targetCus, whichLocation, -1);
										customer [targetCus] = 1;
										currentCus = targetCus;
										break;
									}
									else{
										whichLocation =-1;
										for (int k = 0; k< (int)data->customerToLoc[targetCus].size(); k++){
											int location = data->customerToLoc[targetCus][k];
											if (!s->location[location-numV].used ){
												whichLocation = location - numV;
												break;
											}
											else{
												if (s->location[location-numV].remainInven + data->demCus[targetCus] > s->location[location-numV].inventory)
													continue;
												else{
													whichLocation = location - numV;
													break;
												}
											}
										}
										if (whichLocation == -1)break;
										insertCustomerToLocation(targetCus, whichLocation, -1);
										customer [targetCus] = 1;
										currentCus = targetCus;
										break;
									}
								}
							}
						}
					}
					if (whichLocation ==-1 || j == (int)data->edgeNode[currentCus].size())break;
				}
			}
		}
	}
	//to save all remaining unrouted customers
	double moveGain,minGain;
	Node *nodeV;
	for (int i=0;i<numV;i++){
		if (customer[i] == 0){
			minGain = Max;
			for (int j=0;j<(int)data->edgeNode[i].size();j++){
				if (data->edgeNode[i][j] >= numV || customer[data->edgeNode[i][j]])continue;
				if (s->client[data->edgeNode[i][j]].route->whichLocation->remainInven + data->demCus[i] > s->client[data->edgeNode[i][j]].route->whichLocation->inventory
						|| s->client[data->edgeNode[i][j]].route->load + data->demCus[i] > data->vehCap) continue;
				if (customer[data->edgeNode[i][j]] != 0){
					if (s->client[data->edgeNode[i][j]].next->isDepot)
						moveGain = data->D[s->client[data->edgeNode[i][j]].next->cour][i] + data->D[i][data->edgeNode[i][j]] - data->D[data->edgeNode[i][j]][s->client[data->edgeNode[i][j]].next->cour];
					else
						moveGain = data->D[data->edgeNode[i][j]][i] + data->D[i][s->client[data->edgeNode[i][j]].next->cour] - data->D[data->edgeNode[i][j]][s->client[data->edgeNode[i][j]].next->cour];
				}
				if (moveGain < minGain){
					minGain = moveGain;
					nodeV = &s->client[data->edgeNode[i][j]];
				}
			}
			Node * nodeU = &s->client[i];
			insertClient(nodeU, nodeV);
			s->updateRouteInfor(nodeU->route->cour,-1);
			s->updateLocationInfor(nodeU->route->whichLocation->cour);
		}
	}
}
void initial_sol::insertClient(Node *U, Node *V){
	if (!V->route->used){// insert into a empty route;
		V->pre->pre = U;// to locate the endDepot
		V->next = U;
		U->pre = V;
		U->next = V->pre;
	}
	else{
		V->next->pre = U;
		U->pre = V;
		U->next = V->next;
		V->next = U;
	}
	U->route = V->route;
	remainDem = remainDem - U->dem;
}
void initial_sol::newGreedyInsert(){
	s->initilization();

	bool isTotalSame = true;
	for (int i=1;i<numL;i++)
		if (data->invLoc[numV] != data->invLoc[i+numV])
			isTotalSame = false;
	if (isTotalSame && data->invLoc[numV] > data->totalDemand)
		random_shuffle(data->locRatio.begin(),data->locRatio.end());

	remainDem = data->totalDemand;
	for (int i=0; i<numV; i++)
		random_shuffle(data->edgeNode[i].begin(),data->edgeNode[i].end());
	for (int i=numV; i<numC;i++) locUsed[i] = false;
	// if the inventory of a location can meet all demands, we choose this location in first step;
	int tarLoc = -1;
	for (int i=numV;i<numC;i++){// if there is unlimited capacity location
		if (data->invLoc[i] >= data->totalDemand){
			tarLoc = i;break;
		}
	}
	//// existing perfect compact location configurations
	for (int i=0;i<numV;i++) cusUsed[i] = false;
	double y;int rank;
	if (rankPop > -1 && rankPop < data->numLocConfiguration && (int)data->compactLocConfigure[rankPop].size() > 0){
	//	y=double(rand()%10000/10000.0);
	//	rank=int(pow(y,data->determine_para)*(int)data->compactLocConfigure.size());// this location configuration
		auto it = data->compactLocConfigure[rankPop].begin();
		std::vector<int>location;
		while (it != data->compactLocConfigure[rankPop].end()){
			location.push_back(*it);
			it ++;
		}
		random_shuffle(location.begin(), location.end());
		for (int i=0; i<(int)location.size();i++)
			insertCusToLoc(location[i]);
		int usedInventory = 0;
		for (int i=0;i<numL;i++)if (locUsed[i+numV]) usedInventory += s->location[i].inventory;

		if (usedInventory >= data->totalDemand && remainDem != 0)// the most key method to guarantee the feasible
			remedyTheSolution(location);
	}
	// without the perfect compact location configuration
	int rand_num;
	if (rand()%3 == 1)rand_num =1;
	else rand_num = 0;

	std::vector<int>listLoc1;
	while(remainDem != 0){
		for (int i=0;i<numL;i++)
			if (!locUsed[data->locRatio[i].second] && remainDem == s->location[data->locRatio[i].second - numV].inventory)
				listLoc1.push_back(data->locRatio[i].second);
		if ((int)listLoc1.size() == 0){
			for (int i=0;i<numL;i++)
				if (!locUsed[data->locRatio[i].second])listLoc1.push_back(data->locRatio[i].second);
			//
	//		if (rand_num)// add the randomness
	//			rank = rand()%(int)listLoc1.size();
	//		else{
				y=double(rand()%10000/10000.0);
				rank=int(pow(y,data->determine_para)*(int)listLoc1.size());
	//		}
			tarLoc = listLoc1[rank];
			insertCusToLoc(tarLoc);
		}
		else{
			y=double(rand()%10000/10000.0);
			rank=int(pow(y,data->determine_para)*(int)listLoc1.size());
			tarLoc = listLoc1[rank];
			insertCusToLoc(tarLoc);
		}
		int usedInventory = 0;
		for (int i=0;i<numL;i++)
			if (locUsed[i+numV]) usedInventory += s->location[i].inventory;


		if (usedInventory >= data->totalDemand && remainDem != 0){// the most key method to guarantee the feasible
			std::vector<int>location;
			for (int i=0;i<numL;i++)
				if (s->location[i].used)
					location.push_back(i + numV);
			remedyTheSolution(location);
		}
		listLoc1.clear();
	}
}
void initial_sol::insertCusToLoc(int loc){
	locUsed [loc] = true;
	random_shuffle(data->edgeLocVec[loc-numV].begin(),data->edgeLocVec[loc-numV].end());
	Node *p, *nodeU, *nodeV, *pos; bool isFirst,consider;
	double delta, min_delta;
	for (int i=0;i<(int)data->edgeLocVec[loc-numV].size();i++){
		p = &s->client[data->edgeLocVec[loc-numV][i]];
		if (! cusUsed[p->cour]){
			min_delta = Max;
			for (int j=0;j<s->location[loc-numV].maxVisit;j++){
				if (s->location[loc-numV].depots[j].route->load + p->dem <= data->vehCap && s->location[loc-numV].remainInven + p->dem <= s->location[loc-numV].inventory){
					nodeU = &s->location[loc-numV].depots[j];
					isFirst = true;
					while(isFirst || !nodeU->isDepot){
						delta = data->D[nodeU->cour][p->cour] + data->D[p->cour][nodeU->next->cour] - data->D[nodeU->cour][nodeU->next->cour];
						if (delta < min_delta){
							min_delta = delta;
							pos = nodeU;
						}
						isFirst = false;
						nodeU = nodeU->next;
					}
				}
			}
			if (min_delta == Max)return;// the customer cannot be inserted into the solution
			insertClient(p, pos);
			s->updateRouteInfor(pos->route->cour,-1);
			s->updateLocationInfor(pos->route->whichLocation->cour);
			cusUsed[p->cour] = true;
			//
			while(1){
				consider = false;
				for (int j=0;j<(int)data->edgeNode[p->cour].size();j++){
					if (data->edgeNode[p->cour][j] >= numV)continue;
					nodeV = &s->client[data->edgeNode[p->cour][j]];
					if (!cusUsed[nodeV->cour]){
						if (p->route->load + nodeV->dem <= data->vehCap && p->route->whichLocation->remainInven + nodeV->dem <= p->route->whichLocation->inventory){
							consider = true;
							insertClient(nodeV, p);
							s->updateRouteInfor(p->route->cour,-1);
							s->updateLocationInfor(p->route->whichLocation->cour);
							cusUsed[nodeV->cour] = true;
							p = nodeV;
							break;
						}
						else if (p->route->whichLocation->remainInven + nodeV->dem <= p->route->whichLocation->inventory){
							consider = true;
							nodeU = &s->location[loc-numV].depots[s->location[loc-numV].splitTime];
							insertClient(nodeV, nodeU);
							s->updateRouteInfor(nodeU->route->cour,-1);
							s->updateLocationInfor(nodeU->route->whichLocation->cour);
							cusUsed[nodeV->cour] = true;
							p = nodeV;
							break;
						}
					}
				}
				if (! consider) break;
			}
		}
	}
	// is there any customer can be inserted into the location
	for (int i=0;i<(int)data->nearCityLoca[loc-numV].size();i++){
		p = &s->client[data->nearCityLoca[loc-numV][i]];
		if (cusUsed[p->cour])continue;
		if (s->location[loc-numV].remainInven + p->dem > s->location[loc-numV].inventory)continue;
		min_delta = Max;
		for (int j=0;j<s->location[loc-numV].maxVisit;j++){
			if (s->location[loc-numV].depots[j].route->load + p->dem <= data->vehCap && s->location[loc-numV].remainInven + p->dem <= s->location[loc-numV].inventory){
				nodeU = &s->location[loc-numV].depots[j];
				isFirst = true;
				while(isFirst || !nodeU->isDepot){
					delta = data->D[nodeU->cour][p->cour] + data->D[p->cour][nodeU->next->cour] - data->D[nodeU->cour][nodeU->next->cour];
					if (delta < min_delta){
						min_delta = delta;
						pos = nodeU;
					}
					isFirst = false;
					nodeU = nodeU->next;
				}
			}
		}
		if (min_delta == Max)return;// the customer cannot be inserted into the solution
		insertClient(p, pos);
		s->updateRouteInfor(pos->route->cour,-1);
		s->updateLocationInfor(pos->route->whichLocation->cour);
		cusUsed[p->cour] = true;
	}
}
void initial_sol::remedyTheSolution(std::vector<int> location){// to use the infeasible framework to restore the solution.
	int remainCap, tarLoc;
	Node *p, *pos;
	double delta, min_delta;
	bool isFirst;
	for (int i=0;i<numV;i++){
		if (!cusUsed[i]){// adjust customers in locations and insert the customer i in targeted locations.
			remainCap = 0;
			tarLoc = -1;
			for (int i=0; i<(int)location.size();i++){
				if(std::max(s->location[location[i]-numV].inventory - s->location[location[i]-numV].remainInven, 0) > remainCap){
					remainCap = std::max(s->location[location[i]-numV].inventory - s->location[location[i]-numV].remainInven, 0);
					tarLoc = location[i] - numV;
				}
			}
			if (tarLoc == -1){std::cout<<"there is a wrong problem"<<std::endl;exit(0);}
			min_delta = Max;
			for (int j=0; j<s->location[tarLoc].maxVisit;j++){
				p = &s->location[tarLoc].depots[j];
				isFirst = true;
				while(isFirst || !p->isDepot){
					delta = data->D[p->cour][i] + data->D[i][p->next->cour] - data->D[p->cour][p->next->cour];
					if (delta < min_delta){
						min_delta = delta;
						pos = p;
					}
					p = p->next;
					isFirst = false;
				}
			}
			p = &s->client[i];
			insertClient(p, pos);

			s->updateRouteInfor(pos->route->cour,-1);
			s->updateLocationInfor(pos->route->whichLocation->cour);
			cusUsed[p->cour] = true;
		}
	}
	//repair the infeasible solution
	s->evaluationDis();
//	s->outputSolution();
	repair->repairForInitialSolution(s);
//	s->outputSolution();
}
void initial_sol::initial_solution(Individual *s, int rankPop){
	this->s=s;
	this->rankPop = rankPop;
//	if ((int)data->compactLocConfigure.size() == 0)
//	greedyInsert();
//	else
	newGreedyInsert();

/*	for (int i=0;i<numL;i++){
		if (s->location[i].used){
			std::cout<<i<<" ";
		}
	}
	std::cout<<std::endl;
*/
	s->evaluationDis();
	s->isRight();
}

