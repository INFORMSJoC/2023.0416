/*
 * readdata.h
 *
 *  Created on: 17 Apr 2020
 *      Author: Peng
 */

#ifndef READ_DATA_H_
#define READ_DATA_H_
#include <time.h>
#include "basic.h"
class read_data {
public:
	//general variables for recording some information
	int loopp;
	double probFixedCost;
	int iteration;
	int nbIterNonImpro;
	int isFleetLimited;
	clock_t startTime;
	int timeLimit;
	int iterLimit;
	int totalDemand;
	double avgDemand;
	bool isConstraintLoc;
	// parameters used in population
	int nbClost=5;
	int nbElite=4;
	//parameters used in the shaw operator
	int nearest_pop_parameter=3;
	double determine_para=6;    // determinism parameter p_d suggested by Ropke and Pisinger (2006) TS
	/*parameters needed to be calibrated used in this paper*/
	/********************************************************************************************************/
	/********************************************************************************************************/
	int alpha=20;
	int popMin=30;//30
	int popMax=popMin+30;//30
	int popElit = 30;
	int mutationProb=85;
	double ratioPerturbLength=0.25;
	int maxIterNonProd=70000;
	/********************************************************************************************************/
	int numLocConfiguration = 1000;// if numV>400, it should be 2000.
	int maxIterationPerLoc = 500;
	/********************************************************************************************************/
	//strategies used in crossover
	int eSets_strategy = 3;
	double penaltyDepot = 1.;				// Penalty for one unit of capacity excess (adapted through the search)
	double penaltyVeh = 1.;				// Penalty for one unit of capacity excess (adapted through the search)
	double normalizaPara;
	/***********************************************************************************/
	//parameters for reading each instance
	std::vector < std::vector < int > > edgeNode;
	std::vector < std::vector < int > > customerToLoc;
	std::vector < std::vector < int > > nearCity;
	std::vector < std::vector < int > > nearCityLoca;
	std::vector < std::vector < int > > edgeLocVec;
	int numV;
	int numL;
	int numC;// the total number of vertices
	int vehCap;
	int maxVnum;
	double ** D;// distance between all cities
	double *x;// the coordinate
	double *y;// the coordinate
	int *demCus;// the demand of each customers
	int *invLoc;// the inventory of depots
	double *openCostLoc;
	double usingCostVeh;
	int realOrInteger = 0;
/****************************************************************************/
	read_data();
	virtual ~read_data();
	void define();// read information from .txt files including the number of customers
	void read_fun(std::string pathInstance, std::string pathDepotCon,std::string pathSolution, double bestDis, int isOptimal, int timeLimit,int iterLimit);
	std::vector< std::string > splitwithstr(std::string &str,char ch);
	bool equalTwoVar(double A, double B);
	bool stopCondition(double dis);
/************************************************************************************/
	std::string pathSolution;     // used to file name of output best solution
	std::string pathDepotCon;
	std::string pathInstance;		// the name of instance
	double bestDis;					// record the best result from literature.
	int isOptimal;
	bool meetOptimal=true;
	double timeToTarget=Max;		// record the time when the algorithm meets the best results in literature
	double meetBestTime;
/************************************************************************************/
	//variables for analyzing promising depot configurations
	std::vector<std::pair<double, int>>locRatio;
	std::vector<std::set<int>> compactLocConfigure;
	std::vector<std::pair<double, std::set<int>>> LocConfigure;
	int locCostSignificant;
	//record depot configurations---using binary method
	int dimension = 10000000;
	int *dimTabu1;
	int *dimTabu2;
	bool no_capacity_location = false;


private:
	int *solutionTabu;
	double maxDemand;										// Maximum demand of a client
	double maxDist;											// Maximum distance between two clients
	//these methods and variables are used to decide promising depot configurations
	int **conflictLoc;
	void analyzeLocation();
	void tabuSearchFindCompactConfiguration();
//	void solutionTabuSearchFindCompactConfiguration();void disturbation(int *sol, int &cusDem);
	void spanningTreeForLoc();
	void analyzeConflict();

	void constructedDepotConfigurations();
	std::vector<std::set<int>>locIncCus;//each location includes which customer.

	//
	void readDepotConfigurations();
	std::vector<std::vector<int>>depotCon;
	void decideUsingDepotConfigurations();
};

#endif /* READ_DATA_H_ */
/*
 *
void read_data::solutionTabuSearchFindCompactConfiguration(){
	int *sol,*best_sol;
	double *dem;
	int *solutionTabu1;
	int *solutionTabu2;
	int dimension = 1000000;
	solutionTabu1 = new int [dimension];
	solutionTabu2 = new int [dimension];
	for (int i=0;i<dimension;i++){
		solutionTabu1[i] = 0;   	// binary
		solutionTabu2[i] = 0;		// binary plus the loc
	}
	sol = new int [numL];
	best_sol = new int [numL];
	dem = new double [numL];
	double obj=0,best_obj;
	int cusDem = 0;
	for (int i=0;i<numL;i++){
		sol[i] = 1;
		dem[i] = locRatio[i].first;
		cusDem += invLoc[i+numV];
		obj += dem[i];
	}
	best_obj = obj;
	double delta,min_delta;
	long long int leap = 0, leap2 = 0;
	long long int leap_delta;
	for (int i=0;i<numL;i++){
		if (sol[i]){
			leap += pow(2, i);
			leap2 += leap + i * 9527;
		}
	}
	solutionTabu1[leap % dimension] = 1;
	solutionTabu1[leap2 % dimension] = 1;
	long long int var1 = 0, var2 = 0;
	//
	int no_improve = 0;
	int loc1, loc2;
	int step = 1;
	while (1){
		min_delta = Max;loc1 = -1;loc2 = -1;
		for (int i=0;i<numL;i++){
			if (sol[i] == 1){// remove a location
				if (cusDem - invLoc[i+numV] >= totalDemand){
					delta = -dem[i];
					leap_delta = pow(2,i);
					var1 = leap - leap_delta;
					var2 = leap2 - leap_delta - i * 9527;
					if (solutionTabu1[var1 % dimension] == 0 && solutionTabu2[var2 % dimension] == 0){// the first condition to decide non tabu.
						if (delta < min_delta){
							loc1 = i;
							loc2 = -1;
							min_delta = delta;
						}
					}
				}
				for (int j=0; j<numL;j++){
					if (i == j || sol[j] == 1)continue;
					if (cusDem - invLoc[i+numV] + invLoc[j+numV] < totalDemand)continue;
					delta = dem[j] - dem[i];
					//
					leap_delta = pow(2,j) - pow(2,i);
					var1 += leap_delta;
					var2 = leap2 + leap_delta + j * 9527 - i * 9527;

					if (solutionTabu1[var1 % dimension] == 0 && solutionTabu2[var2 % dimension] == 0){
						if (delta < min_delta){
							loc1 = i;
							loc2 = j;
							min_delta = delta;
						}
					}
				}
			}
			else{//allowing adding a location
				if (cusDem + invLoc[i+numV] >= totalDemand){
					delta = dem[i];
					leap_delta = pow(2,i);
					var1 += leap_delta;
					var2 += leap_delta + i * 9527;

					if (solutionTabu1[leap % dimension] == 0 && solutionTabu2[leap2 % dimension] == 0){
						if (delta < min_delta){
							loc1 = -1;
							loc2 = i;
							min_delta = delta;
						}
					}
				}
			}
		}
		if (min_delta == Max){
			disturbation(sol, cusDem);
			leap = 0; leap2 = 0;
			for (int i=0;i<numL;i++){
				if (sol[i]){
					leap += pow(2, i);
					leap2 += leap + i * 9527;
				}
			}
			solutionTabu1[leap % dimension] = 1;
			solutionTabu1[leap2 % dimension] = 1;
			continue;
		}
		// carry out operations
		if (loc2 > -1 && loc1 > -1){
			sol[loc1] = 0;
			sol[loc2] = 1;
			cusDem += invLoc[loc2+numV];
			cusDem = cusDem - invLoc[loc1+numV];
			leap_delta = pow(2,loc2) - pow(2,loc1);
			leap += leap_delta;
			leap2 += leap_delta + loc2 * 9527 - loc1 * 9527;
			solutionTabu1[leap % dimension] = 1;
			solutionTabu2[leap2 % dimension] = 1;
		}
		else if (loc2 == -1 && loc1 > -1){
			sol[loc1] = 0;
			cusDem = cusDem - invLoc[loc1+numV];
			leap_delta = -pow(2,loc1);
			leap += leap_delta;
			leap2 += leap_delta - loc1 * 9527;
			solutionTabu1[leap % dimension] = 1;
			solutionTabu2[leap2 % dimension] = 1;
		}
		else if (loc2 > -1 && loc1 == -1){
			sol[loc2] = 1;
			cusDem += invLoc[loc2+numV];
			leap_delta = pow(2, loc2);
			leap += leap_delta;
			leap2 += leap_delta + loc2 * 9527;
			solutionTabu1[leap % dimension] = 1;
			solutionTabu2[leap2 % dimension] = 1;
		}

		obj += min_delta;

		for (int i=0;i<numL;i++)
			std::cout<<i<<" ";
		std::cout<<std::endl;
		for (int i=0;i<numL;i++)
			std::cout<<sol[i]<<" ";
		std::cout<<std::endl;

		//
		if (best_obj > obj){
			best_obj = obj;
			for (int i=0;i<numL;i++)
				best_sol[i] = sol[i];
		}
		else	no_improve ++;
		step++;
		if (no_improve == 1000){
			disturbation(sol, cusDem);
			leap = 0; leap2 = 0;
			for (int i=0;i<numL;i++){
				if (sol[i]){
					leap += pow(2, i);
					leap2 += leap + i * 9527;
				}
			}
			solutionTabu1[leap % dimension] = 1;
			solutionTabu1[leap2 % dimension] = 1;
			continue;
		}
		////////////////////////////////////////////////////////
		std::set<int>depot;
		double cost = 0;
		for (int i=0;i<numL;i++)
			if (sol[i] == 1){
				depot.insert(i+numV);
				cost += dem[i];
			}
		////////////////////////////
		int sumDem = 0;
		for (int i=0;i<numL;i++){
			if (sol[i] == 1)
				sumDem += invLoc[i+numV];
		}
		bool isContinue = false;
		for (int i=0;i<numL;i++){
			if (sol[i] == 1)
				if (sumDem - invLoc[i+numV] >= totalDemand){
					isContinue = true;
					break;
				}
		}
		if (isContinue)continue;
		/////////////////////////////

		bool same = true;
		for (int i=0;i<(int)LocConfigure.size();i++){
			if ((int)depot.size() == (int)LocConfigure[i].second.size()){
				auto it1 = depot.begin();
				auto it2 = LocConfigure[i].second.begin();
				bool theSame = true;
				while(it1 != depot.end()){
					if (*it1 != *it2){
						theSame = false;break;
					}
					it1 ++;
					it2 ++;
				}
				if (theSame)same = false;
			}
		}
		if (same){
			if ((int)LocConfigure.size() > numLocConfiguration * 3){
				std::sort(LocConfigure.begin(), LocConfigure.end());
				LocConfigure.pop_back();
			}
			LocConfigure.push_back({cost,depot});
		}
		if ((int)LocConfigure.size() >= 300)break;
	}
	delete [] sol;
	delete [] best_sol;
	delete [] dem;
	delete [] solutionTabu1;
	delete [] solutionTabu2;
	//
	std::sort(LocConfigure.begin(), LocConfigure.end());
}
void read_data::disturbation(int *sol, int &cusDem){
	std::vector<int>loc_in;
	std::vector<int>loc_out;
	for (int i=0;i<numL;i++){
		if (sol[i])
			loc_in.push_back(i);
		else
			loc_out.push_back(i);
	}
	random_shuffle(loc_in.begin(),loc_in.end());
	random_shuffle(loc_out.begin(),loc_out.end());
	for (int i=0;i<numL;i++){
		int randNum = rand()%3;
		if (randNum == 0 && (int)loc_out.size() > 1){// adding
			int index = rand()%(int)loc_out.size();
			int seed = loc_out[index];
			loc_out.erase(loc_out.begin()+index);
			loc_in.push_back(seed);
			cusDem += invLoc[seed+ numL];
		}
		if (randNum == 1 && (int)loc_out.size() > 1 && (int)loc_in.size() > 1){// swaping
			int index1 = rand() % (int)loc_out.size();
			int seed1 = loc_out[index1];
			int index2 = rand() % (int)loc_in.size();
			int seed2 = loc_in[index2];
			if (cusDem + invLoc[seed1 + numL] - invLoc[seed2 + numL] >= totalDemand){
				loc_out.erase(loc_out.begin()+index1);
				loc_in.erase(loc_in.begin()+index2);
				loc_out.push_back(seed2);
				loc_in.push_back(seed1);
				cusDem = cusDem + invLoc[seed1 + numL] - invLoc[seed2 + numL];
			}
		}
		if (randNum == 2 && (int)loc_in.size() > 1){
			int index = rand() % (int)loc_in.size();
			int seed = loc_in[index];
			if (cusDem - invLoc[seed + numL] >= totalDemand){
				loc_in.erase(loc_in.begin()+index);
				loc_out.push_back(seed);
				cusDem = cusDem - invLoc[seed + numL];
			}
		}
	}
}
*/
