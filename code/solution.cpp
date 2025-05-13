/*
 * solution.cpp
 *
 *  Created on: 17 Apr 2020
 *      Author: Peng
 */
#include "basic.h"
#include "solution.h"
#include "initialsol.h"
#include "LS.h"
#include "crossover.h"
using namespace std;
solution::solution(read_data * data, crossover *cross, initial_sol *initialSol,LS *ls,  popManager *popman, RepairProcedure *repair,mutation *mutationOperator) {
	this->data=data;
	this->cross=cross;
	this->initialSol=initialSol;
	this->ls=ls;
	this->popman=popman;
	this->repair=repair;
	this->mutationOperator=mutationOperator;
	//
	temIndiv=new Individual();
	temIndiv->define(data);
	global=new Individual ();
	global->define(data);
	global->initilization();
	loc_best = new Individual();
	loc_best->define(data);
	loc_best->initilization();
	bestIndivRestart=new Individual();
	bestIndivRestart->define(data);
	bestIndivRestart->initilization();

	popman->global=global;
	popman->bestIndivRestart = bestIndivRestart;
	int sizeSubPop=100;//
	subPop=new Individual[sizeSubPop];//
	for (int i=0;i<sizeSubPop;i++){
		subPop[i].define(data);
		subPop[i].initilization();
	}
	global->fit=Max;
	bestIndivRestart->fit=Max;
}
solution::~solution(){
	delete temIndiv;
	delete global;
	delete bestIndivRestart;
	delete [] subPop;
	delete loc_best;
}
void solution::preprocessingLocationConfiguration(){
	data->compactLocConfigure.clear();
	//int initalNumDepots = (int)data->LocConfigure.size();
	int initalNumDepots = std::min(500, (int)data->LocConfigure.size());      // the key parameters// 100
	if ((int)data->LocConfigure.size() < initalNumDepots)
		initalNumDepots = (int)data->LocConfigure.size();
	for (int i=0;i<initalNumDepots; i++)
		data->compactLocConfigure.push_back(data->LocConfigure[i].second);

	if (data->no_capacity_location){
		data->numLocConfiguration = 0;
		return;// if no depot configurations, jump
	}

	std::vector<std::pair<double, int>>averageCost;
	double avgCost,bestCost = Max;
	double global_cost = Max;
	for (int i = 0; i<initalNumDepots;i++){// an extra subPop
		avgCost = 0;bestCost = Max;
		for (int j = 0; j < numSubPopPara; j++){
			initialSol->initial_solution(temIndiv, i);
			ls->local_search_run(temIndiv,data->penaltyDepot, data->penaltyVeh);
			avgCost += temIndiv->fit;
			if (bestCost > temIndiv->fit)
				bestCost = temIndiv->fit;
			if (temIndiv->fit < global_cost){
				// to record the best solution given each depot configuration
				global_cost = temIndiv->fit;
				loc_best->initilization();
				popman->copyAllData(loc_best, temIndiv);// record the best solution given the current depot configuration.
			}
		}
		avgCost = avgCost / (double)(data->popMin);// in this place, we use the best solution
	//	averageCost.push_back({avgCost, i});
		averageCost.push_back({bestCost, i});
		std::cout<<i<<"   tries   "<< bestCost<<std::endl;

//		loc_best->evaluationDis();
	}
	std::sort(averageCost.begin(), averageCost.end());
	loc_best->evaluationDis();

	//
	data->numLocConfiguration = numSubPopPara;//5///////////////////////////////////////////////////
	data->compactLocConfigure.clear();
	for (int i=0;i < std::min(data->numLocConfiguration,initalNumDepots); i++)
		data->compactLocConfigure.push_back(data->LocConfigure[averageCost[i].second].second);
	data->numLocConfiguration = std::min(data->numLocConfiguration,initalNumDepots);
	// add the best solution of each depot configuration in the elite subpop.
//	loc_best->outputBestSolution();
	popman->addIndividual(loc_best, data->numLocConfiguration, true);// insert the local best solution to elite pop
	std::cout<<"Finishing initial filter"<<std::endl;
}
void solution::generation_population(int rankPop, bool isInitial){
	double bestFit=Max;
	/*generate a feasible population set*/
	for (int i = 0; i < data->popMin*4; i++){
		initialSol->initial_solution(temIndiv, rankPop);
		ls->local_search_run(temIndiv,data->penaltyDepot, data->penaltyVeh);
		popman->addIndividual(temIndiv, rankPop, isInitial);
		if (bestFit>temIndiv->fit)
			bestFit=temIndiv->fit;
//		if (i % 100 == 0)
//			std::cout<<i<<"  "<<bestFit<<std::endl;
	}
	std::cout<<"The best fit is    "<<bestFit<<"  The time consuming  "<<(double)clock()/(double)CLOCKS_PER_SEC<<std::endl;
}
void solution::outputprocess(Individual *indiv){
	std::cout<<"It: "<<data->iteration<<"--G-Fit: "<<std::setiosflags(ios::fixed)<<std::setprecision(2)<<float(global->fit);
	std::cout<<"  --R-Fit: "<<bestIndivRestart->fit;
//	std::cout<<"    Avg F "<<float(popman->averagePop())<<std::setprecision(2);
	std::cout<<"  Curr Sol--"<<float(indiv->fit)<<std::setprecision(2);
	std::cout<<"  IT NoImpro--"<<data->nbIterNonImpro;
	std::cout<<"  Time comsuming--"<<float((double)clock()/(double)CLOCKS_PER_SEC)<<std::setprecision(2);
//	std::cout<<"  used depots--"<<popman->usedDepots();
//	double ratio = data->loopp / 1000.;
//	std::cout<<"  overSat. Loc "<< ratio;
//	std::cout<<"  CostProb "<<data->probFixedCost<<std::endl;
	std::cout<<std::endl;
	data->loopp = 0;
}
void solution::getAverageValue(int rank){
	if (rank < 0){
		avgCostPop.clear();
		for (int i=0;i<data->numLocConfiguration + 1; i++){
			double cost = 0;
			for (int j=0;j<(int)popman->popula[i].size();j++){
				cost += popman->popula[i][j]->fit;
			}
			int ss = popman->popula[i].size();
		//	avgCostPop.push_back(cost);
			avgCostPop.push_back(cost / (double)ss);
		}
		//cut some pop whose obj is too worse.
/*		double min_avg = Max;
		for (int i=0;i <data->numLocConfiguration + 1; i++){
			if (min_avg > avgCostPop[i])
				min_avg = avgCostPop[i];
		}
		for (int i=0;i <data->numLocConfiguration + 1; i++){
			if (100*(avgCostPop[i] - min_avg) / min_avg > 5){
				for (Individual * indiv : popman->popula[i])
					delete indiv ;
				popman->popula[i].clear();
			}
		}
		*/
		//removing half subpop
		std::vector<std::pair<double,int>>rankOrder;
		for (int i=0;i<(int)avgCostPop.size();i++)
			rankOrder.push_back({-avgCostPop[i],i});
		sort(rankOrder.begin(),rankOrder.end());
		for (int i=0;i<data->numLocConfiguration / 2; i++){
			int subpop = rankOrder[i].second;
			for (Individual * indiv : popman->popula[subpop]) delete indiv ;
			popman->popula[subpop].clear();
		}
		//
		long long int var, var1, var2;;int seed;
		for (int i=0;i<data->numLocConfiguration;i++){
			if ((int)popman->popula[i].size() > 0){
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
	else{
		double cost = 0;
		for (int j=0;j<(int)popman->popula[rank].size();j++)
			cost += popman->popula[rank][j]->fit;
		int ss = popman->popula[rank].size();
		avgCostPop[rank] = cost / (double)ss;
	//	avgCostPop[rank] = cost;
	}
	// removing half population

}
int solution::selectsubPop(int subpop1){
	int rand1,rand2;
	int target;
	std::vector<int>locConf;
	for (int i=0;i<data->numLocConfiguration + 1; i++)// including the elite population
		if ((int)popman->popula[i].size() > 0)
			locConf.push_back(i);
	while(1){
		rand1 = rand() % (int)locConf.size();
		rand2 = rand() % (int)locConf.size();
		if ((int)locConf.size() <= 3 )return locConf[rand1];
		while(rand1 == rand2)
			rand2=rand() % (int)locConf.size();//data->numLocConfiguration;
		if (avgCostPop[locConf[rand1]] < avgCostPop[locConf[rand2]])
			target = locConf[rand1];
		else
			target=locConf[rand2];
		if (target == subpop1 && (int)popman->popula[target].size() > 2)break;
		if (target != subpop1)break;
		if ((int)locConf.size() == 2){
			if (locConf[0] == subpop1)
				target = locConf[1];
			else
				target = locConf[2];
			break;
		}
	}
	return target;
}
void solution::evolvePop(int rankPop, int &count){
	if ((int)popman->popula[rankPop].size() <= 2){
		count = data->maxIterationPerLoc;
		return;
	}
	int sizeSubPop;
	ma = NULL;
	fa = popman->selectionParentSolution(ma, rankPop);
	ma = popman->selectionParentSolution(fa, rankPop);
	cross->gEAX(fa,ma,subPop,sizeSubPop);
	for (int i =0; i<sizeSubPop; i++){
		repair->repairCapacityConstraint(subPop+i, data->penaltyDepot, data->penaltyVeh);
		if (subPop[i].isFeasible){
		//	if (rand()%100 > data->mutationProb)
		//		mutationOperator->mutationRun(subPop+i);
			ls->local_search_run(subPop+i, data->penaltyDepot, data->penaltyVeh);
			popman->addIndividual(subPop+i, rankPop, true);
		}
	//	if (data->iteration % 500 == 0)	outputprocess(subPop+i);
	//	data->iteration++;
		count++;
	}
}
void solution::initialPop(){
	//phase 1: to construct a number of populations
//	data->compactLocConfigure.clear();
//	if ((int)data->LocConfigure.size() < data->numLocConfiguration)
//		data->numLocConfiguration = (int)data->LocConfigure.size();
//	for (int i=0;i<data->numLocConfiguration; i++)
//		data->compactLocConfigure.push_back(data->LocConfigure[i].second);
	popman->popula.erase(popman->popula.begin()+data->numLocConfiguration +1, popman->popula.end());
	int i;
	for ( i = 0; i<data->numLocConfiguration +1 ;i++){// an extra subPop
		std::cout<<i<<" th ";
		std::cout<<(double)clock()/(double)CLOCKS_PER_SEC<<std::endl;
		generation_population(i, true);
		int count = 0;
		if ((int)popman->popula[i].size() <= 1)
			continue;
		while (count < data->maxIterationPerLoc)
			evolvePop(i, count);
	}
	std::cout<<"Finishing constructing number of initial population"<<std::endl;
}
void solution::main_loop(){
	std::cout<<"---- Start the search----"<<std::endl;
	preprocessingLocationConfiguration();
	int sizeSubPop;
	data->loopp = 0;
	data->nbIterNonImpro=1;
	int subPop1, subPop2;
	initialPop();

	getAverageValue(-1);// for using to select some locations
//	popman->removeEmptySubPop();

	data->iteration=0;
	int isGenerateNewSubPop;
	std::vector<int>depot;
	long long int var1, var2, var;
	int whichSubPop;
	while(1){
		// phase 2: to crossover individuals from different subpopulation;
		subPop1 = selectsubPop(-1);
		subPop2 = selectsubPop(subPop1);
		ma = NULL;
		fa = popman->selectionParentSolution(ma, subPop1);
		ma = popman->selectionParentSolution(fa, subPop2);
		cross->gEAX(fa,ma,subPop,sizeSubPop);
		for (int i =0; i<sizeSubPop; i++){
			repair->repairCapacityConstraint(subPop+i, data->penaltyDepot, data->penaltyVeh);
			if (subPop[i].isFeasible){
				if (rand()%100 > data->mutationProb)
					mutationOperator->mutationRun(subPop+i);
				ls->local_search_run(subPop+i, data->penaltyDepot, data->penaltyVeh);
				//deal with the new solution
				if (!data->no_capacity_location){
					depot.clear();
					for (int j=0; j<data->numL;j++){
						if (subPop[i].location[j].used)
							depot.push_back(j);
					}
					var1 = 0;var2 = 0;
					for (int j=0;j<(int)depot.size();j++){
						var = pow(2, depot[j]);
						var1 += var;
						var2 += var + 9527 * depot[j];
					}
					if (data->dimTabu1[var1 % data->dimension] == data->dimTabu2[var2 % data->dimension] && data->dimTabu2[var2 % data->dimension] > -1)
						whichSubPop = data->dimTabu2[var2 % data->dimension];
					else
						whichSubPop = data->numLocConfiguration;
					isGenerateNewSubPop = popman->addIndividual(subPop+i, whichSubPop, false);
					getAverageValue(whichSubPop);
					if (isGenerateNewSubPop > -1){
						generation_population(isGenerateNewSubPop, true);
						getAverageValue(isGenerateNewSubPop);
						data->dimTabu1[var1 % data->dimension] = isGenerateNewSubPop;
						data->dimTabu2[var2 % data->dimension] = isGenerateNewSubPop;
					}
				}
				else{
					popman->addIndividual(subPop+i, 0, false);
				}

				/////////////////////////////////////
				if (data->iteration % 1000 == 0)	outputprocess(subPop+i);
			//	if (data->iteration > 480000)
			//		outputprocess(subPop+i);
				data->iteration++;
				/*re-start procedure*/
				if (data->nbIterNonImpro >data->maxIterNonProd){
					popman->reStart();
					initialPop();
				}
				data->nbIterNonImpro++;
			}
			///////////////////
			if (data->iteration == data->iterLimit/2)
				popman->reduceHalfSubPop();

			if (data->iteration == data->iterLimit * 0.75)
				popman->reduceHalfSubPop();

			/////////
			if (data->stopCondition(global->fit)){
				global->isRight();
				global->outputBestSolution();
				std::cout<<global->fit<<std::endl;
				std::cout<<"----The memetic algorithm is finished, time spends: "<<(double)clock()/(double)CLOCKS_PER_SEC<<std::endl;
				return;
			}
		}
	}
}
