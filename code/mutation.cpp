/*
 * mutation.cpp
 *
 *  Created on: Sep 14, 2021
 *      Author: pengfei
 */

#include "mutation.h"

mutation::mutation(read_data * data) {
	// TODO Auto-generated constructor stub
	this->data = data;
	this->numV = data->numV;
	this->numC = data->numC;
	this->numL = data->numL;
	auxArray1 = new int [numV];
	this->perturbLength = data->ratioPerturbLength*numV;
	cityRank = new int [numV];
}
mutation::~mutation() {
	// TODO Auto-generated destructor stub
	delete [] auxArray1;
	delete [] cityRank;
}
double mutation::compute(bool istrue, double value){
	if (istrue)	return 0.;
	else return value;
}
void mutation::shawRemoval(){
	// the first step is to select customers
	for (int i=0;i<numV;i++)auxArray1[i]=0;
	int seed=rand()%(numV);
	auxArray1[seed]=1;
	listRemove.push_back(seed);
	int iter;double y;int rank;
	while((int)listRemove.size() < perturbLength){//select more 1/3 cities and select randomly
		seed=rand()%(int)listRemove.size();
		iter=0;
		for (int i=0;i<numV-1;i++){// not the location
			if (data->nearCity[listRemove[seed]][i] < numV && auxArray1[data->nearCity[listRemove[seed]][i]]==0){
				cityRank[iter]=data->nearCity[listRemove[seed]][i];
				iter++;
			}
		}
		y=double(rand()%10000/10000.0);
		rank=int(pow(y,data->determine_para)*iter);
		listRemove.push_back(cityRank[rank]);
		auxArray1[cityRank[rank]]=1;
	}
	//the second step is to remove customers
	Node *p = NULL;
	Node *pre = NULL, *next = NULL;
	for (int i=0;i<(int)listRemove.size();i++){
		p = &s->client[listRemove[i]];
		pre = p->pre;
		next = p->next;
		pre->next = next;
		next->pre = pre;
		p->pre = NULL;
		p->next = NULL;
	}
	for (int i=0;i<numL;i++){
		for (int j=0;j<s->location[i].maxVisit;j++)
			s->updateRouteInfor(s->location[i].depots[j].route->cour, -1);
		int ll = i+numV;
		s->updateLocationInfor(ll);
	}
//	if (data->iteration == 313)
//		s->outputSolution();

	//the third step is to add customers
	random_shuffle(listRemove.begin(), listRemove.end());// randomize all removed customers
	double delta, min_delta;
	Node *node, *nodeU, *nodeX, *pos;
	while ((int)listRemove.size() != 0){
		min_delta = Max; //listRemove
		pos = NULL;
		node = &s->client[listRemove[0]];
		for (int i=0;i<(int)data->edgeNode[node->cour].size();i++){
			if (data->edgeNode[node->cour][i] >= numV) continue;
			if (auxArray1[data->edgeNode[node->cour][i]] == 0){
				nodeU = &s->client[data->edgeNode[node->cour][i]];
				if (nodeU->route->load + node->dem > data->vehCap || nodeU->route->whichLocation->remainInven + node->dem > nodeU->route->whichLocation->inventory)continue;
				if (nodeU->route->cour == node->route->cour)continue;
				delta = data->D[nodeU->cour][node->cour] + data->D[node->cour][nodeU->next->cour] - data->D[nodeU->cour][nodeU->next->cour];
				if (delta < min_delta){
					pos = nodeU;
					min_delta = delta;
				}
			}
		}
		for (int i=0; i<(int)data->customerToLoc[node->cour].size(); i++){
			if (s->location[data->customerToLoc[node->cour][i] - numV].remainInven + node->dem > s->location[data->customerToLoc[node->cour][i] - numV].inventory)continue;
			if (s->location[data->customerToLoc[node->cour][i] - numV].emptyRoutes.empty())continue;
			nodeU = s->route[*s->location[data->customerToLoc[node->cour][i] - numV].emptyRoutes.begin()].depot;
			nodeX = nodeU->next;
			delta = compute(nodeU->route->whichLocation->used, data->openCostLoc[nodeU->route->whichLocation->cour])
					+ compute(!(nodeU->isDepot && nodeX->isDepot), data->usingCostVeh)
					+ data->D[nodeU->cour][node->cour] * 2;
			if (delta < min_delta){
				pos = nodeU;
				min_delta = delta;
			}
		}
		// remedy strategy
		if (min_delta == Max){
			for (int i=0; i<numL; i++){
				if (s->location[i].used){
					if (s->location[i].remainInven + node->dem > s->location[i].inventory)continue;
					nodeU = s->route[*s->location[i].emptyRoutes.begin()].depot;
					nodeX = nodeU->next;
					delta = compute(nodeU->route->whichLocation->used, data->openCostLoc[nodeU->route->whichLocation->cour])
							+ compute(!(nodeU->isDepot && nodeX->isDepot), data->usingCostVeh)
							+ data->D[nodeU->cour][node->cour] * 2;
					if (delta < min_delta){
						pos = nodeU;
						min_delta = delta;
					}
				}
			}
			if (min_delta == Max){
				for (int i=0; i<numL; i++){
					if (!s->location[i].used){
						if (s->location[i].remainInven + node->dem > s->location[i].inventory)continue;
						nodeU = s->route[*s->location[i].emptyRoutes.begin()].depot;
						nodeX = nodeU->next;
						delta = compute(nodeU->route->whichLocation->used, data->openCostLoc[nodeU->route->whichLocation->cour])
								+ compute(!(nodeU->isDepot && nodeX->isDepot), data->usingCostVeh)
								+ data->D[nodeU->cour][node->cour] * 2;
						if (delta < min_delta){
							pos = nodeU;
							min_delta = delta;
						}
					}
				}
			}
		}

		insertNode(node,pos);
//		if (data->iteration == 313)
//			s->outputSolution();
		auxArray1[node->cour] = 0;
		listRemove.erase(listRemove.begin());
		s->updateRouteInfor(pos->route->cour,-1);
		s->updateLocationInfor(pos->route->whichLocation->cour);
	}
}
void mutation::insertNode(Node *U, Node *V){
	V->next->pre = U;
	U->next = V->next;
	V->next = U;
	U->pre = V;
	U->route = V->route;
}
void mutation::locationRemove(){// to exchange the most nearest locations
	for (int i=0;i<numV;i++)auxArray1[i]=0;
	std::vector< std::pair<double, int> >staDe;
	double temVar;
	for (int i=0;i<numL;i++){
		if (s->location[i].used){
			temVar = (double)s->location[i].remainInven / (double)s->location[i].inventory;
			staDe.push_back({temVar, i});
		}
	}
	std::sort(staDe.begin(),staDe.end());
	int removeLoc = staDe[0].second;
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
	Node *node, *nodeU, *nodeX, *pos;
	while ((int)listRemove.size() != 0){
		min_delta = Max; //listRemove
		pos = NULL;
		node = &s->client[listRemove[0]];
		for (int i=0;i<(int)data->edgeNode[node->cour].size();i++){
			if (data->edgeNode[node->cour][i] >= numV) continue;
			if (auxArray1[data->edgeNode[node->cour][i]] == 0){
				nodeU = &s->client[data->edgeNode[node->cour][i]];
				if (nodeU->route->load + node->dem > data->vehCap || nodeU->route->whichLocation->remainInven + node->dem > nodeU->route->whichLocation->inventory)continue;
				if (nodeU->route->cour == node->route->cour)continue;
				delta = data->D[nodeU->cour][node->cour] + data->D[node->cour][nodeU->next->cour] - data->D[nodeU->cour][nodeU->next->cour];
				if (delta < min_delta){
					pos = nodeU;
					min_delta = delta;
				}
			}
		}
		for (int i=0;i<numV;i++){
			if (auxArray1[i] == 0){
				nodeU = &s->client[i];
				if (nodeU->route->load + node->dem > data->vehCap || nodeU->route->whichLocation->remainInven + node->dem > nodeU->route->whichLocation->inventory)continue;
				if (nodeU->route->cour == node->route->cour)continue;
				delta = data->D[nodeU->cour][node->cour] + data->D[node->cour][nodeU->next->cour] - data->D[nodeU->cour][nodeU->next->cour];
				if (delta < min_delta){
					pos = nodeU;
					min_delta = delta;
					break;
				}
			}
		}

		for (int i=0; i<(int)data->customerToLoc[node->cour].size(); i++){
			if (data->customerToLoc[node->cour][i] - numV == removeLoc)continue;
			if (s->location[data->customerToLoc[node->cour][i] - numV].remainInven + node->dem > s->location[data->customerToLoc[node->cour][i] - numV].inventory)continue;
			if((int)s->location[data->customerToLoc[node->cour][i] - numV].emptyRoutes.size() == 0)continue;
			nodeU = s->route[*s->location[data->customerToLoc[node->cour][i] - numV].emptyRoutes.begin()].depot;

			nodeX = nodeU->next;
			delta = compute(nodeU->route->whichLocation->used, data->openCostLoc[nodeU->route->whichLocation->cour])
					+ compute(!(nodeU->isDepot && nodeX->isDepot), data->usingCostVeh)
					+ data->D[nodeU->cour][node->cour] * 2;
			if (delta < min_delta){
				pos = nodeU;
				min_delta = delta;
			}
		}
		if (min_delta == Max){
			for (int i=0; i<numL; i++){
				if (s->location[i].remainInven + node->dem > s->location[i].inventory)continue;
				if ((int) s->location[i].emptyRoutes.size() == 0)continue;
				nodeU = s->route[*s->location[i].emptyRoutes.begin()].depot;
				nodeX = nodeU->next;
				delta = compute(nodeU->route->whichLocation->used, data->openCostLoc[nodeU->route->whichLocation->cour])
						+ compute(!(nodeU->isDepot && nodeX->isDepot), data->usingCostVeh)
						+ data->D[nodeU->cour][node->cour] * 2;
				if (delta < min_delta){
					pos = nodeU;
					min_delta = delta;
				}
			}
		}

		insertNode(node,pos);
		auxArray1[node->cour] = 0;
		listRemove.erase(listRemove.begin());
		s->updateRouteInfor(pos->route->cour,-1);
		s->updateLocationInfor(pos->route->whichLocation->cour);
	}
}
void mutation::distanceRemove(){
	// the first step is to select customers
	for (int i=0;i<numV;i++)auxArray1[i]=0;
	std::set<std::pair<double, int>>distanceR;
	for (int i=0;i<numV;i++)//sort the distance
		distanceR.insert({data->D[i][s->client[i].route->whichLocation->cour], i});

	auto it = distanceR.begin();
	while((int)listRemove.size() < perturbLength){//select more 1/3 cities and select randomly
		listRemove.push_back(it->second);
		auxArray1[it->second]=1;
		it ++;
	}
	//the second step is to remove customers
	Node *p = NULL;
	Node *pre = NULL, *next = NULL;
	for (int i=0;i<(int)listRemove.size();i++){
		p = &s->client[listRemove[i]];
		pre = p->pre;
		next = p->next;
		pre->next = next;
		next->pre = pre;
		p->pre = NULL;
		p->next = NULL;
	}
	for (int i=0;i<numL;i++){
		for (int j=0;j<s->location[i].maxVisit;j++)
			s->updateRouteInfor(s->location[i].depots[j].route->cour, -1);
		int ll = i+numV;
		s->updateLocationInfor(ll);
	}

	//the third step is to add customers
	random_shuffle(listRemove.begin(), listRemove.end());// randomize all removed customers
	double delta, min_delta;
	Node *node, *nodeU, *nodeX, *pos;
	while ((int)listRemove.size() != 0){
		min_delta = Max; //listRemove
		pos = NULL;
		node = &s->client[listRemove[0]];
		for (int i=0;i<(int)data->edgeNode[node->cour].size();i++){
			if (data->edgeNode[node->cour][i] >= numV) continue;
			if (auxArray1[data->edgeNode[node->cour][i]] == 0){
				nodeU = &s->client[data->edgeNode[node->cour][i]];
				if (nodeU->route->load + node->dem > data->vehCap || nodeU->route->whichLocation->remainInven + node->dem > nodeU->route->whichLocation->inventory)continue;
				if (nodeU->route->cour == node->route->cour)continue;
				delta = data->D[nodeU->cour][node->cour] + data->D[node->cour][nodeU->next->cour] - data->D[nodeU->cour][nodeU->next->cour];
				if (delta < min_delta){
					pos = nodeU;
					min_delta = delta;
				}
			}
		}
		for (int i=0; i<(int)data->customerToLoc[node->cour].size(); i++){
			if (s->location[data->customerToLoc[node->cour][i] - numV].remainInven + node->dem > s->location[data->customerToLoc[node->cour][i] - numV].inventory)continue;
			if ((int)s->location[data->customerToLoc[node->cour][i] - numV].emptyRoutes.size() == 0)continue;
			nodeU = s->route[*s->location[data->customerToLoc[node->cour][i] - numV].emptyRoutes.begin()].depot;
			nodeX = nodeU->next;
			delta = compute(nodeU->route->whichLocation->used, data->openCostLoc[nodeU->route->whichLocation->cour])
					+ compute(!(nodeU->isDepot && nodeX->isDepot), data->usingCostVeh)
					+ data->D[nodeU->cour][node->cour] * 2;
			if (delta < min_delta){
				pos = nodeU;
				min_delta = delta;
			}
		}
		// remedy strategy
		if (min_delta == Max){
			for (int i=0; i<numL; i++){
				if (s->location[i].used){
					if (s->location[i].remainInven + node->dem > s->location[i].inventory)continue;
					if ((int)s->location[i].emptyRoutes.size() == 0)continue;
					nodeU = s->route[*s->location[i].emptyRoutes.begin()].depot;
					nodeX = nodeU->next;
					delta = compute(nodeU->route->whichLocation->used, data->openCostLoc[nodeU->route->whichLocation->cour])
							+ compute(!(nodeU->isDepot && nodeX->isDepot), data->usingCostVeh)
							+ data->D[nodeU->cour][node->cour] * 2;
					if (delta < min_delta){
						pos = nodeU;
						min_delta = delta;
					}
				}
			}
			if (min_delta == Max){
				for (int i=0; i<numL; i++){
					if (!s->location[i].used){
						if (s->location[i].remainInven + node->dem > s->location[i].inventory)continue;
						nodeU = s->route[*s->location[i].emptyRoutes.begin()].depot;
						nodeX = nodeU->next;
						delta = compute(nodeU->route->whichLocation->used, data->openCostLoc[nodeU->route->whichLocation->cour])
								+ compute(!(nodeU->isDepot && nodeX->isDepot), data->usingCostVeh)
								+ data->D[nodeU->cour][node->cour] * 2;
						if (delta < min_delta){
							pos = nodeU;
							min_delta = delta;
						}
					}
				}
			}
		}
		insertNode(node,pos);

		auxArray1[node->cour] = 0;
		listRemove.erase(listRemove.begin());
		s->updateRouteInfor(pos->route->cour,-1);
		s->updateLocationInfor(pos->route->whichLocation->cour);
	}
}
void mutation::mutationRun(Individual *s){
	this->s=s;
	if (rand()%1 == 0)
		shawRemoval();
		//distanceRemove();
	else
		locationRemove();

	s->evaluationDis();
	s->isRight();
}

