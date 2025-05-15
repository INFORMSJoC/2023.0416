/*
 * Indi.cpp
 *
 *  Created on: 18 Apr 2020
 *      Author: Peng
 */

#include "Individual.h"

Individual::Individual() {
}
Individual::~Individual() {
	delete [] client;
	for (int i=0; i< numL;i++){
		delete [] location[i].depots;
		delete [] location[i].endDepots;
	}
	delete [] location;
	delete [] route;
	delete [] checkArray;
}
void Individual::define(read_data *data){
	this->data=data;
	this->numV=data->numV;
	this->numL = data->numL;
	this->numC = data->numC;
	client=new Node [numV];
	for (int i=0;i<numV;i++){
		client[i].isDepot=false;
		client[i].cour=i;
		client[i].pre=NULL;
		client[i].next=NULL;
		client[i].route=NULL;
		client[i].dem=data->demCus[i];
		client[i].position=0;
		client[i].cumulatedLoad=0;
		client[i].cumulatedDis=0;
		client[i].whenLastTestedRI=-1;
	}
	location = new NodeDepot [numL];
	maxVnum=0;
	for (int i=0; i<numL; i++){
		location[i].used = false;
		location[i].cour = i+numV;
		location[i].inventory = data->invLoc[i+numV];
		location[i].remainInven = 0;
		location[i].splitTime = 0;
		int numOfVeh;
	//	if (data->totalDemand < location[i].inventory){
			numOfVeh =ceil(1.2*data->totalDemand/data->vehCap)+5;
	//	}
		if (location[i].inventory > data->totalDemand) location[i].iscapCon = false;// to decide the constraint in locations
		else location[i].iscapCon = true;
		location[i].maxVisit = std::min((double)numOfVeh, 3*(double)ceil((double)location[i].inventory/(double)data->vehCap));
		maxVnum += location[i].maxVisit;
		location[i].depots = new Node [location[i].maxVisit];
		location[i].endDepots = new Node [location[i].maxVisit];
		for (int j=0; j<location[i].maxVisit;j++){
			location[i].depots[j].isDepot=true;
			location[i].depots[j].cour = i+numV;
			location[i].depots[j].position = 0;
			location[i].depots[j].dem = 0;
			location[i].depots[j].cumulatedLoad = 0;
			location[i].depots[j].cumulatedDis = 0;
			location[i].depots[j].pre = &location[i].endDepots[j];
			location[i].depots[j].next = &location[i].endDepots[j];
			location[i].endDepots[j].isDepot=true;
			location[i].endDepots[j].cour = i+numV;
			location[i].endDepots[j].position = 0;
			location[i].endDepots[j].dem = 0;
			location[i].endDepots[j].cumulatedLoad = 0;
			location[i].endDepots[j].cumulatedDis = 0;
			location[i].endDepots[j].pre = &location[i].depots[j];
			location[i].endDepots[j].next = &location[i].depots[j];
		}
	}
//	data->maxVnum = maxVnum;
	route=new Route [maxVnum];// the maxVnum is the maximum vehicles;
	for (int i=0; i<maxVnum; i++){
		route[i].used=false;
		route[i].cour=i;
		route[i].dis=0;
		route[i].load=0;
		route[i].nbClients=0;
		route[i].penalty=0;
	}
	int rankRoute=0;
	for (int i=0;i<numL;i++){
		for (int j=0;j<location[i].maxVisit; j++){
			location[i].depots[j].route = &route[rankRoute];
			location[i].endDepots[j].route = &route[rankRoute];
			route[rankRoute].depot = &location[i].depots[j];
			route[rankRoute].endDepot = &location[i].endDepots[j];
			route[rankRoute].whichLocation = & location[i];
			rankRoute++;
		}
	}
	checkArray = new int [numV];
}
void Individual::initilization(){
	saturatLoc.clear();
	saturatRoute.clear();
	for (int i=0;i<numV;i++){
		client[i].pre=NULL;
		client[i].next=NULL;
		client[i].route=NULL;
		client[i].position=0;
		client[i].cumulatedLoad=0;
		client[i].cumulatedDis=0;
		client[i].whenLastTestedRI=-1;
	}
	for (int i=0; i<numL; i++){
		location[i].used = false;
		location[i].inventory = data->invLoc[i+numV];
		location[i].remainInven = 0;
		location[i].splitTime = 0;
		for (int j=0; j<location[i].maxVisit;j++){
			location[i].depots[j].cour = i+numV;
			location[i].depots[j].dem = 0;
			location[i].depots[j].cumulatedLoad = 0;
			location[i].depots[j].cumulatedDis = 0;
			location[i].depots[j].pre = &location[i].endDepots[j];
			location[i].depots[j].next = &location[i].endDepots[j];
			location[i].endDepots[j].isDepot=true;
			location[i].endDepots[j].cour = i+numV;
			location[i].endDepots[j].position = 0;
			location[i].endDepots[j].dem = 0;
			location[i].endDepots[j].cumulatedLoad = 0;
			location[i].endDepots[j].cumulatedDis = 0;
			location[i].endDepots[j].pre = &location[i].depots[j];
			location[i].endDepots[j].next = &location[i].depots[j];
		}
	}
	for (int i=0; i<maxVnum; i++){
		route[i].used=false;
		route[i].cour=i;
		route[i].dis=0;
		route[i].load=0;
		route[i].nbClients=0;
		route[i].penalty=0;
	}
	for (int i=numV;i<numC;i++){
		location[i-numV].emptyRoutes.clear();
		for (int j=0;j<location[i-numV].maxVisit;j++){
			int routee = location[i-numV].depots[j].route->cour;
			location[i-numV].emptyRoutes.insert(routee);
		}
	}
}
void Individual::evaluationDis(){
	double totalDis=0;
	double totalLoc=0;
	double totalVeh=0;
	Node *p;
	for (int i=numV; i < numC ;i++){
		if (location[i - numV].used){
			totalLoc += data->openCostLoc[location[i - numV].cour];
			totalVeh += data->usingCostVeh * location[i - numV].splitTime;
			for (int j=0; j < location[i - numV].maxVisit; j++){
				if (location[i-numV].depots[j].route->used){
					p = &location[i - numV].depots[j];
					totalDis += data->D[p->next->cour][p->cour];
					p=p->next;
					while(1){
						p=p->next;
						if (p->isDepot){
							totalDis += data->D[p->pre->cour][p->cour];
							break;
						}
						totalDis += data->D[p->cour][p->pre->cour];
					}
				}
			}
		}
	}

	fit = totalLoc + totalVeh + totalDis;
	dis = totalDis;

	data->probFixedCost = totalLoc / fit;
}
void Individual::getSaturationDegree(){
	saturatLoc.clear();
	saturatRoute.clear();
	for (int i=0;i<numL;i++){
		if (location[i].used){
			if (!data->isConstraintLoc)/////////////////////////the other cases and considered in other places
				saturatLoc.insert({double(location[i].remainInven) / double(location[i].inventory), i});
			for (int j=0;j<location[i].maxVisit;j++){
				if (location[i].depots[j].route->used){
					saturatRoute.insert({double(location[i].depots[j].route->load) / double(data->vehCap), location[i].depots[j].route->cour});
				}
			}
		}
	}
}


void Individual::updateRouteInfor(int &r, int nbMoves){
	Node *mynode=route[r].depot;
	mynode->cumulatedLoad=0;
	mynode->position=0;
	mynode->cumulatedDis = 0;
		//
	bool isFirst = true;
	while(!mynode->isDepot || isFirst){
		mynode=mynode->next;
		mynode->position=mynode->pre->position+1;
		mynode->cumulatedLoad=mynode->dem+mynode->pre->cumulatedLoad;
		mynode->cumulatedDis=data->D[mynode->cour][mynode->pre->cour]+mynode->pre->cumulatedDis;
		isFirst = false;
	}
	route[r].dis=mynode->cumulatedDis + data->D[mynode->cour][mynode->next->cour];
	route[r].load=mynode->cumulatedLoad;
	route[r].nbClients=mynode->position;

//		NodeDepot *loc = route[r].whichLocation;// update the location inventory
//		loc->remainInven=0;
//		for (int i=0; i<loc->splitTime;i++)
//			loc->remainInven += loc->depots[i].route->load;
	route[r].whenLastModified = nbMoves;
	if (route[r].load == 0)
		route[r].whichLocation->emptyRoutes.insert(route[r].cour);
	else
		route[r].whichLocation->emptyRoutes.erase(route[r].cour);
}
void Individual:: updateLocationInfor(int &ll){
	int l = ll - numV;
	location[l].splitTime = 0;
	location[l].remainInven = 0;
	for (int i=0; i<location[l].maxVisit; i++){
		if (location[l].depots[i].route->load > 0){
			location[l].remainInven += location[l].depots[i].route->load;
			location[l].depots[i].route->used = true;
			location[l].splitTime++;
		}
		else location[l].depots[i].route->used = false;
	}
//	if (location[l].remainInven > location[l].inventory)std::cout<<"report a wrong "
	if (location[l].remainInven == 0){
		location[l].used = false;
		location[l].splitTime=0;
	}
	else location[l].used = true;
}
void Individual::outputSolution(){
	std::cout<<"------------------------------------The solution display is as follows-------------------------------------------"<<std::endl;
	Node *p;
	for (int loc=numV; loc < numC; loc++){
		if (location[loc - numV].used){
			std::cout<<"Loc "<<loc-numV<<" inventory =  "<<location[loc-numV].inventory<<"  used: "<<location[loc-numV].remainInven<<std::endl;
			for (int i=0;i<location[loc - numV].maxVisit;i++){
				if (!location[loc - numV].depots[i].route->used)continue;
				std::cout<<"Loc "<< location[loc - numV].cour<<" and route ";
				std::cout<< location[loc - numV].depots[i].route->cour<<" (cap "<<location[loc - numV].depots[i].route->load<<"): ";
				p = &location[loc - numV].depots[i];
				p=p->next;
				while(! p->next->isDepot){
					std::cout<<p->cour<<" ";
					p=p->next;
				}
				std::cout<<p->cour<<std::endl;
			}
		}
	}
}
void Individual::isRight(){// check the solution
	double totalDis=0,totalFit=0;
	Node *p;int capLoc, capVeh;
	for (int i=0;i<numV;i++)
		checkArray[i] = 0;
	for (int loc=numV; loc < numC; loc++){
		if (location[loc - numV].used){
			capLoc = 0;
			totalFit += data->openCostLoc[loc];
			totalFit += data->usingCostVeh * location[loc - numV].splitTime;
			for (int i=0;i<location[loc - numV].maxVisit;i++){
				if (!location[loc - numV].depots[i].route->used)continue;
				capVeh = 0;
				p = &location[loc - numV].depots[i];
				totalDis += data->D[p->next->cour][p->cour];
				p=p->next;
				while(! p->next->isDepot){
					checkArray[p->cour] ++;
					capVeh += data->demCus[p->cour];
					totalDis += data->D[p->cour][p->next->cour];
					p=p->next;
				}
				checkArray[p->cour] ++;
				capVeh += data->demCus[p->cour];
				if (capVeh != p->route->load || capVeh > data->vehCap){
					std::cout<<"The route "<<p->route->cour<<" capacity is wrong"<<std::endl;
					exit(0);
				}
				capLoc += capVeh;
				totalDis += data->D[p->cour][p->next->cour];
			}
			if (capLoc != p->route->whichLocation->remainInven || capLoc > location[loc-numV].inventory){
				std::cout<<"The location "<<p->route->whichLocation->cour<<" capacity is wrong"<<std::endl;
				exit(0);
			}
		}
	}
	for (int i=0;i<numV;i++)
		if (checkArray[i] != 1){
			std::cout<<"The vertex "<<i<<"  is wrong"<<std::endl;
			exit(0);
		}
	if (! data->equalTwoVar(totalDis, dis)){
		std::cout<<"The traveling distance is wrong"<<std::endl;
		exit(0);
	}
	if (! data->equalTwoVar(totalFit+ totalDis, fit)){
		std::cout<<"The fitness is wrong"<<std::endl;
		exit(0);
	}
}
void Individual::decideFeasible(){
	for (int i=0;i<numL;i++){
		if (location[i].used){
			for (int j=0; j<location[i].maxVisit; j++){
				if (location[i].depots[j].route->used){
					if (location[i].depots[j].route->load > data->vehCap){
						isFeasible = false;
						return;
					}
				}
			}
			if (location[i].remainInven > location[i].inventory){
				isFeasible = false;
				return;
			}
		}
	}
	isFeasible = true;
}
void Individual::outputBestSolution(){
	std::cout<<"----------Write Solution With Value  "<<fit<<" In: "<<data->pathSolution<<std::endl;
	std::ofstream myfile(data->pathSolution);
	//double Distance=0;
	if (myfile.is_open()){
		myfile<<"The objective is: "<<std::endl;
		myfile<<fit<<std::endl;
		myfile<<"The running time is: "<<std::endl;
		myfile<<(double)clock()/(double)CLOCKS_PER_SEC<<std::endl;
		myfile<<"The time meeting best is: "<<std::endl;
		myfile<<data->meetBestTime<<std::endl;
		Node *p;
		for (int loc=numV; loc < numC; loc++){
			if (location[loc - numV].used){
				int count=0;
				myfile<<"Location: "<<loc-numV<<"(load: "<<location[loc-numV].remainInven<<") "<<std::endl;
				for (int i=0;i<location[loc - numV].maxVisit;i++){
					if (!location[loc - numV].depots[i].route->used)continue;
					myfile<<" Route "<<count<<"(load: "<<location[loc - numV].depots[i].route->load<<") ";
					count++;
					p = &location[loc - numV].depots[i];
				//	myfile<<p->cour<<" ";
					p=p->next;
					while(! p->next->isDepot){
						myfile<<p->cour<<" ";
						p=p->next;
					}
				//	myfile<<p->cour<<" "<<p->next->cour<<std::endl;
					myfile<<p->cour<<" "<<std::endl;
				}
			}
		}
		// computing the actual cost
		double totalDis=0,totalFit=0;
		int capLoc, capVeh;
		for (int loc=numV; loc < numC; loc++){
			if (location[loc - numV].used){
				capLoc = 0;
				totalFit += data->openCostLoc[loc];
				totalFit += data->usingCostVeh * location[loc - numV].splitTime;
				for (int i=0;i<location[loc - numV].maxVisit;i++){
					if (!location[loc - numV].depots[i].route->used)continue;
					capVeh = 0;
					p = &location[loc - numV].depots[i];
					totalDis += data->D[p->next->cour][p->cour];
					p=p->next;
					while(! p->next->isDepot){
						checkArray[p->cour] ++;
						capVeh += data->demCus[p->cour];
						totalDis += data->D[p->cour][p->next->cour];
						p=p->next;
					}
					checkArray[p->cour] ++;
					capVeh += data->demCus[p->cour];
					if (capVeh != p->route->load || capVeh > data->vehCap){
						std::cout<<"The route "<<p->route->cour<<" capacity is wrong"<<std::endl;
						exit(0);
					}
					capLoc += capVeh;
					totalDis += data->D[p->cour][p->next->cour];
				}
			}
		}
		myfile<<"The actual cost is "<<totalFit + totalDis<<std::endl;


	/*	myfile<<numC<<std::endl;
		for (int i=0;i<numC;i++){
			myfile<<data->x[i]<<" "<<data->y[i]<<std::endl;
		}
		Node *p;
		for (int loc=numV; loc < numC; loc++){
			if (location[loc - numV].used){
				int count=0;
				for (int i=0;i<location[loc - numV].maxVisit;i++){
					if (!location[loc - numV].depots[i].route->used)continue;
					count++;
					p = &location[loc - numV].depots[i];
					myfile<<p->cour<<" ";
					p=p->next;
					while(! p->isDepot){
						myfile<<p->cour<<" ";
						p=p->next;
					}
				//	myfile<<p->cour<<" "<<p->next->cour<<std::endl;
					myfile<<p->cour<<" "<<std::endl;
				}
			}
		}
*/
	}
	else std::cout << "----- IMPOSSIBLE TO OPEN: " << data->pathSolution << std::endl;
	myfile.close();

}
void Individual::removeProximity(Individual * indiv){
	auto it = indivsPerProximity.begin();
	while (it->second != indiv) ++it;
	indivsPerProximity.erase(it);
}
double Individual::averageBrokenPairsDistanceClosest(int nbClosest){
	double result = 0 ;
	int maxSize = std::min<int>(nbClosest, indivsPerProximity.size());
	auto it = indivsPerProximity.begin();
	for (int i=0 ; i < maxSize; i++){
		result += it->first ;
		++it ;
	}
	return result/(double)maxSize ;
}
double Individual::brokenPairsDistance(Individual * indiv2){
	Node *p,*p1;bool isFirst;
	double hammingDis = 0;
	for (int i=numV; i<numC; i++){
		if (location[i-numV].used){
			for (int j=0; j<location[i-numV].maxVisit; j++){
				if (location[i-numV].depots[j].route->used){
					p = &location[i-numV].depots[j];
					p1=p->next;
					if (indiv2->client[p1->cour].route->whichLocation->cour != p->route->whichLocation->cour)hammingDis += 10;
					isFirst = true;
					while (isFirst || !p->next->isDepot){
						if (indiv2->client[p->next->cour].pre->cour == p->cour || indiv2->client[p->next->cour].next->cour == p->cour)
							hammingDis += 1;
						else if (indiv2->client[p->next->cour].route->cour == p->route->cour){
							hammingDis += 3;
						}
						else if (indiv2->client[p->next->cour].route->whichLocation->cour != p->route->whichLocation->cour){
							hammingDis += 10;
						}
						p = p->next;
						isFirst = false;
					}
					if (indiv2->client[p->cour].pre->cour == p->next->cour || indiv2->client[p->cour].next->cour == p->next->cour)
						hammingDis += 1;
					else if (indiv2->client[p->cour].route->cour == p->next->route->cour){
						hammingDis += 3;
					}
					else if (indiv2->client[p->cour].route->whichLocation->cour != p->next->route->whichLocation->cour){
						hammingDis += 10;
					}
				}
			}
		}
	}
	return -hammingDis;
}




