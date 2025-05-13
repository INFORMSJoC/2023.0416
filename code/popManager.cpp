/*
 * popManager.cpp
 *
 *  Created on: 11 Aug 2021
 *      Author: Peng
 */

#include "popManager.h"

popManager::popManager(read_data *data) {
	// TODO Auto-generated constructor stub
	this->data=data;
	this->numV = data->numV;
	this->numL = data->numL;
	this->numC = data->numC;
	order = new int [numV];
	popula = std::vector <std::vector<Individual*>>(100);
}

popManager::~popManager() {
	// TODO Auto-generated destructor stub
	for (int i = 0; i < (int)feasibleSubpopulation.size(); i++)
		delete feasibleSubpopulation[i];
	delete [] order;
	for (int i=0;i<data->numLocConfiguration+1;i++){
		for (Individual * indiv : popula[i]) delete indiv;
		popula[i].clear();
	}
}
void popManager:: copyAllData(Individual * s,Individual *ss){
	Node *nodeU;
	s->saturatLoc.clear();
	s->saturatRoute.clear();
	/*initialize solution s*/
	for (int i=0;i<numV; i++){
		s->client[i].pre = NULL;
		s->client[i].next = NULL;
		s->client[i].route = NULL;
	}
	for (int i=0;i<numL;i++){
		s->location[i].emptyRoutes.clear();
		for (int j=0;j<s->location[i].maxVisit;j++){
			s->location[i].depots[j].next = &s->location[i].endDepots[j];
			s->location[i].endDepots[j].pre = &s->location[i].depots[j];
			s->updateRouteInfor(s->location[i].depots[j].route->cour, -1);
		}
	//	s->updateLocationInfor(i + numV);
		s->location[i].used=false;
		s->location[i].emptyRoutes.clear();
	}
	//
	int count;
	Route * routeU;
	for (int i=numV; i<numC;i++){
		if (!ss->location[i-numV].used)continue;
		for (int j=0;j<ss->location[i-numV].maxVisit;j++){
			if (ss->location[i-numV].depots[j].route->used){
				count = 0;
				nodeU = ss->location[i-numV].depots[j].next;
				while(!nodeU->isDepot){
					order[count] = nodeU->cour;
					count++;
					nodeU=nodeU->next;
				}
				s->location[i-numV].depots[j].route->used = true;
				routeU = s->location[i-numV].depots[j].route;
				nodeU = &s->client[order[0]];
				nodeU->pre = routeU->depot;
				nodeU->route = routeU;
				routeU->depot->next = nodeU;
				for (int k=1; k<count; k++){
					s->client[order[k]].pre = nodeU;
					nodeU->next = &s->client[order[k]];
					s->client[order[k]].route = routeU;
					nodeU = &s->client[order[k]];
				}
				nodeU->next = routeU->endDepot;
				routeU->endDepot->pre = nodeU;
				s->updateRouteInfor(routeU->cour,-1);
			}
			else{
				s->location[i-numV].depots[j].next = &s->location[i-numV].endDepots[j];
				s->location[i-numV].endDepots[j].pre = &s->location[i-numV].depots[j];
				s->updateRouteInfor(s->location[i-numV].depots[j].route->cour,-1);
			}
		}
		s->updateLocationInfor(i);
	}
	s->dis=ss->dis;
	s->fit=ss->fit;
	s->biasedFit=ss->biasedFit;
	s->isFeasible=ss->isFeasible;
}
int popManager::addIndividual(Individual *indiv, int rankPop, bool isInitial){
	if (isInitial){
		std::vector<int>depot;
		if (rankPop > -1 && rankPop < data->numLocConfiguration){
			for (int i=0;i<numL;i++){
				if (indiv->location[i].used)
					depot.push_back(i+numV);
			}
			auto it = data->compactLocConfigure[rankPop].begin();
			if ((int)depot.size() == (int)data->compactLocConfigure[rankPop].size()){
				for (int i=0;i<(int)depot.size();i++){
					if (*it != depot[i]){
						rankPop = data->numLocConfiguration;
					//	addInElitePop(indiv);
						break;
					}
					it ++;
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	SubPopulation & subpop = popula[rankPop];
	//
	for (Individual * myIndividual : subpop)// no clone individuals
		if (myIndividual->fit + Min > indiv->fit && myIndividual->fit - Min < indiv->fit)
			return -1;
	//
	Individual * myIndividual = new Individual();
	myIndividual->define(data);
	myIndividual->initilization();
	copyAllData(myIndividual, indiv);
	for (Individual * myIndividual2 : subpop){
		double myDistance = myIndividual->brokenPairsDistance(myIndividual2);
		myIndividual2->indivsPerProximity.insert({ myDistance, myIndividual });
		myIndividual->indivsPerProximity.insert({ myDistance, myIndividual2 });
	}

	int place = (int)subpop.size();
	while (place > 0 && subpop[place - 1]->fit > indiv->fit - Min) place--;
	subpop.emplace(subpop.begin() + place, myIndividual);
	//
	std::vector<int>depot;
	bool isObtainNewPop = false;int index = -1;

	if (indiv->fit + Min < bestIndivRestart->fit){

		indiv->isRight();
		bestIndivRestart->initilization();
		copyAllData(bestIndivRestart, indiv);
		data->meetBestTime = (double)clock()/(double)CLOCKS_PER_SEC;
		data->nbIterNonImpro=0;
		if (indiv->fit + Min < global->fit){
			global->initilization();
			copyAllData(global, indiv);
			data->meetBestTime = float((double)clock()/(double)CLOCKS_PER_SEC);
		}
		//to replace a subPop if the new global solution's not belong to any subpop.
		if (!data->no_capacity_location){
			depot.clear();
			for (int i=0;i<numL;i++){
				if (global->location[i].used)
					depot.push_back(i+numV);
			}
			if (rankPop == data->numLocConfiguration && !isInitial){

				std::vector<std::pair<double, int>>avgCostPop;
				for (int i=0;i<data->numLocConfiguration; i++){
					if((int)popula[i].size() == 0)continue;
					double cost = 0;int count = 0;
					SubPopulation & sub = popula[i];
					for (Individual * myIndiv : sub){
						cost += myIndiv->fit;
						count ++;
					}
					if (count > 0)
						avgCostPop.push_back({cost / (double)count, i});
				}
				if ((int)avgCostPop.size() == 0)
					index = 0;
				else{
					double max_avg = 0;
					for (int i=0;i <(int)avgCostPop.size(); i++){
						if (max_avg < avgCostPop[i].first){
							max_avg = avgCostPop[i].first;
							index = avgCostPop[i].second;
						}
					}
				}
				SubPopulation & sub = popula[index];
				for (Individual * indiv : sub) delete indiv;
				sub.clear();
				data->compactLocConfigure[index].clear();
				for (int i=0;i<(int)depot.size();i++)
					data->compactLocConfigure[index].insert(depot[i]);
				Individual * myIndivi = new Individual();
				myIndivi->define(data);
				myIndivi->initilization();
				copyAllData(myIndivi, myIndividual);

				sub.emplace(sub.begin(),myIndivi);
				// generate the initial pop for the promising depot configuration
				isObtainNewPop = true;
			}
		}
		////////////////////////////////////////////
	}
	if ((int)subpop.size() == data->popMax )
		while ((int)subpop.size() > data->popMin)
			removeWorstBiasedFitness(subpop);
	//
	if (isObtainNewPop)
		return index;
	else
		return -1;
}

void popManager::addInElitePop(Individual *indiv){
/*	//update the e-lite population
	SubPopulation & epop = eliteSubpopulation;
	for (Individual * myIndividual : epop)// no clone individuals
		if (myIndividual->fit + Min > indiv->fit && myIndividual->fit - Min < indiv->fit)
			return;
	if ((int)epop.size() < data->popMax){
		Individual * myIndiv = new Individual();
		myIndiv->define(data);
		copyAllData(myIndiv, indiv);
		int place = (int)epop.size();
		while (place > 0 && epop[place - 1]->fit > indiv->fit - Min)
			place--;
		epop.emplace(epop.begin() + place, myIndiv);
	}
	else{
		int place = (int)epop.size();
		while (place > 0 && epop[place - 1]->fit > indiv->fit - Min)
			place--;
		if (place < (int)epop.size()){
			Individual * myIndiv = new Individual();
			myIndiv->define(data);
			copyAllData(myIndiv, indiv);
			epop.emplace(epop.begin() + place, myIndiv);
			int worst = (int)epop.size()-1;
			Individual * worstIndividual = epop[worst];
			epop.erase(epop.begin() + worst); // Removing the individual from the population
			delete worstIndividual;
		}
	}
	*/
}
Individual * popManager::selectionParentSolution(Individual *exist, int rankPop){
	SubPopulation & subpop = popula[rankPop];
	updateBiasedFitnesses(subpop);

	int rand1,rand2;
	Individual *target;
	if (subpop.size() == 1){
		target = subpop[0];
		return target;
	}
	while(1){
		rand1=rand()%(int)subpop.size();
		rand2=rand()%(int)subpop.size();
		while(rand1 == rand2)
			rand2=rand()%(int)subpop.size();
		//if (subpop[rand1]->biasedFit < subpop[rand2]->biasedFit)
		if (subpop[rand1]->biasedFit < subpop[rand2]->biasedFit)
			target=subpop[rand1];
		else
			target=subpop[rand2];
		if (target != exist)break;
	}
	return target;
}

void popManager::reStart(){
	std::cout<<"----Restart and create a new population----"<<std::endl;
	for (int i=0;i<data->numLocConfiguration+1;i++){
		for (Individual * indiv : popula[i]) delete indiv ;
		popula[i].clear();
	}
	data->nbIterNonImpro=0;
	bestIndivRestart->fit = Max;
}
double popManager::averagePop(){
	double fit=0;
	for (Individual * indiv : feasibleSubpopulation)
		fit += indiv->fit;
	fit = fit/(double)feasibleSubpopulation.size();
	return fit;
}
double popManager::usedDepots(){
	std::set<int>locations;
	for (Individual * indiv : feasibleSubpopulation){
		for (int i=0;i<numL;i++){
			if (indiv->location[i].used)
				locations.insert(i);
		}
	}
	int ss = locations.size();
	return(double(ss) / double(numL));
}
void popManager::removeWorstBiasedFitness(SubPopulation & pop){
	updateBiasedFitnesses(pop);
	if (pop.size() <= 1) throw std::string("Eliminating the best individual: this should not occur in the algorithm");

	Individual * worstIndividual = NULL;
	int worstIndividualPosition = -1;
	bool isWorstIndividualClone = false;
	double worstIndividualBiasedFitness = -1.e30;
	for (int i = 0; i < (int)pop.size(); i++)
	{
		bool isClone = (pop[i]->averageBrokenPairsDistanceClosest(1) < Min); // A distance equal to 0 indicates that a clone exists
		if ((isClone && !isWorstIndividualClone) || (isClone == isWorstIndividualClone && pop[i]->biasedFit > worstIndividualBiasedFitness))
		{
			worstIndividualBiasedFitness = pop[i]->biasedFit;
			isWorstIndividualClone = isClone;
			worstIndividualPosition = i;
			worstIndividual = pop[i];
		}
	}

	pop.erase(pop.begin() + worstIndividualPosition); // Removing the individual from the population
	for (Individual * myIndividual2 : pop) myIndividual2->removeProximity(worstIndividual); // Cleaning its distances from the other individuals in the population
	delete worstIndividual; // Freeing memory
	worstIndividual=NULL;
}

void popManager::updateBiasedFitnesses(SubPopulation & pop){
	// Ranking the individuals based on their diversity contribution (decreasing order of distance)
	std::vector <std::pair <double, int> > ranking;
	for (int i = 0 ; i < (int)pop.size(); i++)
		ranking.push_back({-pop[i]->averageBrokenPairsDistanceClosest(data->nbClost),i});
	std::sort(ranking.begin(), ranking.end());

	// Updating the biased fitness values
	if (pop.size() == 1)
		pop[0]->biasedFit = 0;
	else{
		for (int i = 0; i < (int)pop.size(); i++){
			double divRank = (double)i / (double)(pop.size() - 1); // Ranking from 0 to 1
			double fitRank = (double)ranking[i].second / (double)(pop.size() - 1);
			if ((int)pop.size() <= data->nbElite) // Elite individuals cannot be smaller than population size
				pop[ranking[i].second]->biasedFit = fitRank;
			else
				pop[ranking[i].second]->biasedFit = fitRank + (1.0 - (double)data->nbElite / (double)pop.size()) * divRank;
		}
	}
}
void popManager::removeEmptySubPop(){
	for (int i = popula.size()-2;i >= 0; i--){
		if (popula[i].empty()){
			popula.erase(popula.begin() + i);
			data->numLocConfiguration--;
		}
	}
	for (int i=0;i<data->dimension;i++){
		data->dimTabu1 [i] = -1;
		data->dimTabu2 [i] = -1;
	}
	long long int var, var1, var2;;int seed;
	for (int i=0;i<data->numLocConfiguration;i++){
		if ((int)popula[i].size() > 0){
			var = 0;var1 = 0;
			auto it = data->compactLocConfigure[i].begin();
			for (;it != data->compactLocConfigure[i].end(); it++){
				seed = *it - data->numV;
				var2 = pow(2, seed);
				var += var2;
				var1 += var2 + 9527 * seed;
			}
			data->dimTabu1[var % data->dimension] = i;
			data->dimTabu2[var1 % data->dimension] = i;
		}
	}
}
void popManager::reduceHalfSubPop(){
	if ((int)popula.size() < 3)return;
	std::vector<std::pair<double, int>>averageCost;
	double avgCost;
	for (int i=0;i< data->numLocConfiguration;i++){
		avgCost = 0;
		for (int j=0;j<(int)popula[i].size();j++){
			avgCost += popula[i][j]->fit;
			if (std::abs(bestIndivRestart->fit - popula[i][j]->fit) < 0.000001)
				avgCost = -Max;
		}
		if ((int)popula[i].size() == 0)
			avgCost = Max;
		else
			avgCost = avgCost / (double)popula[i].size();
		averageCost.push_back({avgCost, i});
	}
	std::sort(averageCost.begin(), averageCost.end());
	int halfSize = (int)(data->numLocConfiguration/2.);
	std::vector<int> remove;
	for (int i=(int)averageCost.size()-1; i > halfSize; i--)
		remove.push_back(averageCost[i].second);
	std::sort(remove.begin(),remove.end());
	std::reverse(remove.begin(),remove.end());
	for (int i=0;i<(int)remove.size();i++){
		SubPopulation & sub = popula[remove[i]];
		for (Individual * indiv : sub) delete indiv;
		popula.erase(popula.begin()+remove[i]);
		data->compactLocConfigure.erase(data->compactLocConfigure.begin() + remove[i]);
	}
	data->numLocConfiguration = data->numLocConfiguration - remove.size();

	for (int i=0;i<data->dimension;i++){
		data->dimTabu1 [i] = -1;
		data->dimTabu2 [i] = -1;
	}
	long long int var, var1, var2;;int seed;
	for (int i=0;i<data->numLocConfiguration;i++){
		if ((int)popula[i].size() > 0){
			var = 0;var1 = 0;
			auto it = data->compactLocConfigure[i].begin();
			for (;it != data->compactLocConfigure[i].end(); it++){
				seed = *it - data->numV;
				var2 = pow(2, seed);
				var += var2;
				var1 += var2 + 9527 * seed;
			}
			data->dimTabu1[var % data->dimension] = i;
			data->dimTabu2[var1 % data->dimension] = i;
		}
	}


}
