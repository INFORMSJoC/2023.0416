/*
 * readdata.cpp
 *
 *  Created on: 17 Apr 2020
 *      Author: Peng
 */

#include "read_data.h"
#include "basic.h"
#include "Individual.h"
read_data::read_data() {

}
read_data::~read_data() {
	delete [] dimTabu1;
	delete [] dimTabu2;
	for (int i=0;i<numC;i++){
		delete [] D[i];
	}
	delete [] D;
	delete [] x;
	delete [] y;
	delete [] demCus;
	delete [] invLoc;
	delete [] openCostLoc;
	for (int i=0;i<numL;i++) delete [] conflictLoc[i];
	delete [] conflictLoc;
}
void read_data::define(){
	dimTabu1 = new int [dimension];
	dimTabu2 = new int [dimension];
	for (int i=0;i<dimension;i++){
		dimTabu1 [i] = -1;
		dimTabu2 [i] = -1;
	}

	D=new double *[numC];
	for (int i=0;i<numC;i++) D[i]=new double [numC];
	x=new double [numC];
	y=new double [numC];
	demCus=new int [numV];
	invLoc = new int [numC];
	openCostLoc = new double [numC];
	edgeNode = std::vector< std::vector < int > >(numV);
	customerToLoc = std::vector< std::vector < int > >(numV);
	edgeLocVec = std::vector < std::vector < int > > (numL);
	nearCity = std::vector< std::vector < int > >(numV);
	nearCityLoca = std::vector< std::vector < int > >(numL);
	conflictLoc = new int *[numL];
	for(int i=0;i<numL;i++) conflictLoc[i] = new int [numL];
	for (int i=0;i<numL;i++)
		for (int j=0;j<numL;j++)
			conflictLoc[i][j] = 0;
	locIncCus = std::vector<std::set<int>>(numL);
}
void read_data::read_fun(std::string pathInstance,std::string pathDepotCon,std::string pathSolution, double bestDis, int isOptimal, int timeLimit,int iterLimit){
	/****imput information*******/
	this->timeLimit=timeLimit;
	this->iterLimit=iterLimit;
	this->bestDis = bestDis;
	this->isOptimal=isOptimal;
	this->pathSolution=pathSolution;
	this->pathDepotCon=pathDepotCon;
	this->pathInstance=pathInstance;
	this->isFleetLimited = isFleetLimited;
	/*********************************************************/
	std::ifstream inputFile(pathInstance);

	std::vector<std::string> temStr2=splitwithstr(pathInstance,'.');
	int sss = (int)temStr2.size();
//	std::cout<<temStr2[sss-1]<<std::endl;

	if (inputFile.is_open()){
		std::string instanceType;
		std::string name1 = "dat";
		std::string name2 = "json";
		if (!temStr2[sss-1].compare(name1)){
			inputFile >> numV;
			inputFile >> numL;
			numC = numV + numL;
			define();
			for (int i=numV; i<numC; i++){
				inputFile >> x[i];
				inputFile >> y[i];
			}
			for (int i=0; i< numV; i++){
				inputFile >> x[i];
				inputFile >> y[i];
			}
			inputFile >> vehCap;
			for (int i=numV; i< numC; i++)
				inputFile >> invLoc[i];
			for (int i=0; i< numV; i++)
				inputFile >> demCus[i];
			for (int i=numV; i< numC; i++)
				inputFile >> openCostLoc[i];
			inputFile >> usingCostVeh;
			inputFile >> realOrInteger;
		}
		else if (!temStr2[sss-1].compare(name2)){
			numV = 0; numL = 0;
			std::string var1;
			inputFile >> var1;
			while (var1 != "\"depots\":"){
				inputFile >> var1;
				if (var1 == "\"y\":"){
					inputFile >> var1;
					std::vector<std::string> temStr2=splitwithstr(var1,',');
					numV++;
				}
			}
			while (var1 != "\"name\":"){
				inputFile >> var1;
				if (var1 == "\"y\":"){
					inputFile >> var1;
					std::vector<std::string> temStr2=splitwithstr(var1,',');
					numL++;
				}
			}
			numC = numV + numL;
			define();
			inputFile.close();
			//
			std::ifstream inputFile(pathInstance);
			inputFile >> var1;
			numV = 0;
			numL = 0;
			while (var1 != "\"depots\":"){
				inputFile >> var1;
				if (var1 == "\"demand\":"){
					inputFile >> var1;
					std::vector<std::string> temStr2=splitwithstr(var1,',');
					demCus[numV] = int(std::stoi(temStr2[0]));
				}
				if (var1 == "\"x\":"){
					inputFile >> var1;
					std::vector<std::string> temStr2=splitwithstr(var1,',');
					x[numV] = std::stoi(temStr2[0]);
				}
				if (var1 == "\"y\":"){
					inputFile >> var1;
					std::vector<std::string> temStr2=splitwithstr(var1,',');
					y[numV] = std::stoi(temStr2[0]);
					numV++;
				}
			}
			while (var1 != "\"name\":"){
				inputFile >> var1;
				if (var1 == "\"capacity\":"){
					inputFile >> var1;
					std::vector<std::string> temStr2=splitwithstr(var1,',');
					invLoc[numL+numV] = std::stoi(temStr2[0]);
				}
				if (var1 == "\"costs\":"){
					inputFile >> var1;
					std::vector<std::string> temStr2=splitwithstr(var1,',');
					openCostLoc[numL+numV] = std::stoi(temStr2[0]);
				}
				if (var1 == "\"x\":"){
					inputFile >> var1;
					std::vector<std::string> temStr2=splitwithstr(var1,',');
					x[numV+numL] = std::stoi(temStr2[0]);
				}
				if (var1 == "\"y\":"){
					inputFile >> var1;
					std::vector<std::string> temStr2=splitwithstr(var1,',');
					y[numV+numL] = std::stoi(temStr2[0]);
					numL++;
				}
			}
			while(1){
				inputFile >> var1;
				if (var1 == "\"vehicle_capacity\":"){
					inputFile >> var1;
					std::vector<std::string> temStr2=splitwithstr(var1,',');
					vehCap = std::stoi(temStr2[0]);
				}
				if (var1 == "\"vehicle_costs\":"){
					inputFile >> var1;
					std::vector<std::string> temStr2=splitwithstr(var1,',');
					usingCostVeh = std::stoi(temStr2[0]);
					break;
				}
			}
		}
	}
	else {
		std::cout<<"Wrong when opening file"<<std::endl;
		exit(0);}
	inputFile.close();
	//
	readDepotConfigurations();


	totalDemand=0;
	maxDemand=0;
	for (int i=0; i < numV; i++){
		if (demCus[i] > maxDemand)
			maxDemand=demCus[i];
		totalDemand+=demCus[i];
	}
	normalizaPara = double(numV) / double(totalDemand);
	avgDemand = double(totalDemand) / double(numV);
	//
	int numOfVeh;
	maxVnum=0;
	for (int i=0;i<numL;i++){
		numOfVeh =ceil(1.2*totalDemand/vehCap)+5;
		maxVnum += std::min((double)numOfVeh, 3*(double)ceil((double)invLoc[i+numV]/(double)vehCap));
	}
	// process the initial data

	isConstraintLoc = false;
	for (int i=numV;i<numC;i++){
		if (invLoc[i] <= totalDemand){
			isConstraintLoc = true;
			break;
		}
	}
	/***********************************************************************************************/
	/*give the distance matrix*/
	maxDist=0;
	for (int i=0;i<numV;i++){
		for (int j=0;j<numV;j++){
			D[i][j]=(sqrt((x[i]-x[j])*(x[i]-x[j])+(y[i]-y[j])*(y[i]-y[j])));
			if (D[i][j]>maxDist)
				maxDist=D[i][j];
		}
	}
	for (int i=0; i < numV;i++){
		for (int j=numV; j<numC; j++){
			D[i][j] = (sqrt((x[i]-x[j])*(x[i]-x[j])+(y[i]-y[j])*(y[i]-y[j])));
			D[j][i] = D[i][j];
		}
	}
	for (int i=numV; i<numC; i++)
		for (int j=numV; j<numC; j++)
			D[i][j] = 0;
	if (realOrInteger == 0)
		for (int i=0;i<numC;i++)
			for (int j=0;j<numC;j++)
				D[i][j] = (int)(ceil(D[i][j]*100));
	// the most key element to decide promising depot configurations.
	/****************************************************************************/

	/****************************************************************************/
	/* construct sparse graph****************/
	std::vector < std::set < int > > setCorrelatedVertices = std::vector < std::set <int> >(numC);
	std::vector < std::pair <double, int> > orderProximity;
	for (int i = 0; i <numV; i++){
		orderProximity.clear();
		for (int j = 0; j <numC; j++)
			if (i != j) orderProximity.push_back(std::pair <double, int>(D[i][j], j));

		sort(orderProximity.begin(), orderProximity.end());
		for (int j = 0; j < std::min<int>(alpha, numV - 2);  j++){
			setCorrelatedVertices[i].insert(orderProximity[j].second);
			setCorrelatedVertices[orderProximity[j].second].insert(i);
		}
		for (int j=0;j < (int)orderProximity.size();j++)
			nearCity[i].push_back(orderProximity[j].second);
	}
	// Filling the vector of correlated vertices
	for (int i = 0; i <numV; i++)
		for (int x : setCorrelatedVertices[i])
			edgeNode[i].push_back(x);
	// to control the number of locations closed to customers.
	for (int i = 0; i< numV; i++){
		for (int j=0; j<(int)nearCity[i].size(); j++){
			if (nearCity[i][j] >= numV){
				customerToLoc[i].push_back(nearCity[i][j]);
				if ((int)customerToLoc[i].size() > numL/2.)
					break;
			}
		}
	}
	//
	std::vector < std::set < int > > setCorrelatedLocation = std::vector < std::set <int> >(numL);
	for (int i=0; i < numL; i++){
		orderProximity.clear();
		for (int j = 0; j <numV; j++)
			orderProximity.push_back(std::pair <double, int>(D[j][i+numV], j));
		sort(orderProximity.begin(), orderProximity.end());

		for (int j = 0; j < std::min<int>(alpha, numV - 2);  j++)
				setCorrelatedLocation[i].insert(orderProximity[j].second);

		for (int j=0;j < (int)orderProximity.size();j++)
			nearCityLoca[i].push_back(orderProximity[j].second);
	}
	for (int i = 0; i <numL; i++)
		for (int x : setCorrelatedLocation[i])
			edgeLocVec[i].push_back(x);
	/****************************************************///


	//decide whether used extra depot configurations
	decideUsingDepotConfigurations();

}
void read_data::decideUsingDepotConfigurations(){
	if (usingDepotConfiguration == 0)//not using
		analyzeLocation();
	else if (usingDepotConfiguration == 1){// only using
		std::set<int> array;
		LocConfigure.clear();
		for (int i=0;i<(int)depotCon.size();i++){
			array.clear();
			for (int j=0;j<(int)depotCon[i].size();j++)
				array.emplace (depotCon[i][j]);
			LocConfigure.push_back({0,array});
		}
		spanningTreeForLoc();
		std::sort(locRatio.begin(),locRatio.end());

	}
	else if (usingDepotConfiguration == 2){// mixed used
		analyzeLocation();
		std::set<long>binary;
		long var;
		int size;
		for (int i=0;i<(int)LocConfigure.size();i++){
			auto it = LocConfigure[i].second.begin();
			var = 0;
			while(it != LocConfigure[i].second.end()){
				var += std::pow(2,*it-numV);
				it++;
			}
			binary.emplace(var);
		}
		//
		std::set<int> array;
		for (int i=0;i<(int)depotCon.size();i++){
			array.clear();
			var = 0;
			for (int j=0;j<(int)depotCon[i].size();j++){
				array.emplace (depotCon[i][j]);
				var += std::pow(2, depotCon[i][j] - numV);
			}
			size = binary.size();
			binary.insert(var);
			if (size == (int)binary.size())continue;
			else LocConfigure.insert(LocConfigure.begin(),{0,array});
		}
	}
}

std::vector<std::string>read_data:: splitwithstr(std::string &str,char ch){
	  std::string tmpstr = str+ch;
	  std::vector<std::string> res;
	  if (str.size()<=0)
		return res;
	  size_t pos = tmpstr.find(ch);
	  while (pos!=std::string::npos){
			std::string child_str = tmpstr.substr(0, pos);
			if (child_str.size()>0)
			  res.push_back(child_str);
			tmpstr = tmpstr.substr(pos + 1);
			pos = tmpstr.find(ch);
	  }
	  if(res.empty()){
		  res.push_back(str);
		  return res;
	  }
	  return res;
}
//
bool read_data::stopCondition(double dis){
	if ((double)clock()/(double)CLOCKS_PER_SEC > timeLimit)
		return true;
	if (iteration > iterLimit)
		return true;
	if (meetOptimal && bestDis > dis +Min){
		timeToTarget = (double)clock()/(double)CLOCKS_PER_SEC;
		meetOptimal = false;
	}
	if (std::abs(bestDis - dis) < 0.01){
		if (isOptimal)return true;
	}
	return false;
}
bool read_data::equalTwoVar(double A, double B){
	if (abs(A-B) < Min)return true;
	else return false;
}
//////////////////////////////////////////////////to configure the promising locations.
void read_data::analyzeLocation(){
//	for (int i=numV; i<numC;i++)
//		locRatio.push_back({- double(invLoc[i]) / double(openCostLoc[i]), i});

	spanningTreeForLoc();
	if (numL>=10)analyzeConflict();
	//
//	solutionTabuSearchFindCompactConfiguration();
//	if (locCostSignificant == 1 || locCostSignificant == 0)
//		tabuSearchFindCompactConfiguration();
//	else
	int min_depot = Max;
	int max_depot = 0;
	for (int i=numV;i<numC;i++){
		if (min_depot > invLoc[i])
			min_depot = invLoc[i];
		if (max_depot < invLoc[i])
			max_depot = invLoc[i];
	}
	if (min_depot > totalDemand){
		int array[numL];
		for (int i=0;i<numL;i++){
			array[i] = invLoc[i+numV];
			invLoc [i+numV] = (double)totalDemand / ((double)numL / 2.);
		//	invLoc[i+numV] = 0;
		}
	//	for (int i=0;i<numV;i++){
	//		invLoc[customerToLoc[i][0]] += demCus[i] * 2;
	//	}


		constructedDepotConfigurations();
		std::cout<<"The number of Depot Configurations is: "<<(int)LocConfigure.size()<<std::endl;
	//	if ((int)LocConfigure.size() < 500){
			std::cout<<"triggering the tabu search to find more depot configurations"<<std::endl;
		//	tabuSearchFindCompactConfiguration();

		for (int i=0;i<numL;i++){
			invLoc[i+numV] = array[i];
		}
//		tabuSearchFindCompactConfiguration();
//		no_capacity_location = true;
		std::sort(locRatio.begin(),locRatio.end());
		return;
	}

	if (numL < 10)
		tabuSearchFindCompactConfiguration();
	else{
		constructedDepotConfigurations();
		std::cout<<"The number of Depot Configurations is: "<<(int)LocConfigure.size()<<std::endl;
	//	if ((int)LocConfigure.size() < 500){
			std::cout<<"triggering the tabu search to find more depot configurations"<<std::endl;
			tabuSearchFindCompactConfiguration();
	//	}
	}

	std::sort(LocConfigure.begin(), LocConfigure.end());
	std::cout<<"finishing the depot configurations"<<std::endl;
	std::sort(locRatio.begin(),locRatio.end());
}
void read_data::analyzeConflict(){
	double min_x = Max, min_y = Max, max_x = 0,max_y = 0;
	double range_x, range_y;
	for (int i=0;i<numC;i++){
		if (x[i] < min_x) min_x = x[i];
		if (y[i] < min_y) min_y = y[i];
		if (x[i] > max_x) max_x = x[i];
		if (y[i] > max_y) max_y = y[i];
	}
	double range_loc, min_loc = Max, max_loc = 0;
	for (int i=numV;i<numC;i++){
		if (invLoc[i] < min_loc)min_loc = invLoc[i];
		if (invLoc[i] > max_loc)max_loc = invLoc[i];
	}
	range_loc = max_loc - min_loc;
	range_x = max_x - min_x;
	range_y = max_y - min_y;
	//decide the instance whether fixed location costs significant.
	double avgCost = 0;
	int count = 0;
	for (int i=0;i<numV;i++){
		for (int j=0;j<(int)edgeNode[i].size();j++){
			avgCost += edgeNode[i][j];
			count ++;
		}
	}
	avgCost = avgCost / count;
	double travelCost = avgCost * numV;
	double locCost = 0;
	for (int i=0;i<numL;i++)
		locCost += openCostLoc[i + numV];
	// there are two cases. the first case is type C and the other cases are all other instances
//	if (travelCost > locCost * 10)// this type is not complete since types a b d are not
//		locCostSignificant = 0;// type c
//	else if (travelCost * 100 < locCost)
//		locCostSignificant = 1;// type e
//	else locCostSignificant = 2;
	//the following is to decide some locations conflict
	double dis_measure = (range_x + range_y) / 2.;
	double ratio = range_loc / min_loc;// if the value is greater than 1, it is type e, otherwise it is other type.
	double xx,yy,xy;
	for (int i=numV;i<numC;i++){
		if (ratio > 1 && ((double)invLoc[i] / (double)totalDemand > 0.2)){// the type e
			if (max_loc / invLoc[i] > 1.8)continue;
			for (int j = i; j < numC; j++){
				if (i == j || ((double)invLoc[i] / (double)invLoc[j] > 1.8))continue;
				xx = std::abs(x[i] - x[j]);
				yy = std::abs(y[i] - y[j]);
				xy = (xx + yy) / 2.;
				if (xy / dis_measure < (double)invLoc[i] / (double)totalDemand){
					conflictLoc[i - numV][j - numV] = 1;
					conflictLoc[j - numV][i - numV] = 1;
				}
			}
		}
		if (ratio < 1 && (double)invLoc[i] / (double)totalDemand > 0.1){// the type a b d
			for (int j = i; j < numC; j++){
				if (i == j)continue;
				xx = std::abs(x[i] - x[j]);
				yy = std::abs(y[i] - y[j]);
				xy = (xx + yy) / 2.;
				if (2.0*xy / dis_measure < (double)invLoc[i] / (double)totalDemand){// the coefficient 2 is used to not limit too much
					conflictLoc[i - numV][j - numV] = 1;
					conflictLoc[j - numV][i - numV] = 1;
				}
			}
		}
	}
/*
	for (int i=0;i<numL;i++){
		for (int j=0;j<numL;j++)
			std::cout<<conflictLoc[i][j]<<" ";
		std::cout<<std::endl;
	}
*/
}
void read_data::spanningTreeForLoc(){
	int * check_cus;
	check_cus = new int [numV];

	double * low_cost;
	low_cost = new double [numV];

	//////////////the following is the spanning tree method
	double min_dis;
	double total_dis_loc;
	int total_dem;
	int v;
	for (int l=0;l<numL;l++){
		for (int i=0;i<numV;i++)check_cus[i] = 0;
		total_dis_loc = 0;
		total_dem = 0;
		for (int j=0;j<numV;j++)
			low_cost[j]=D[l+numV][j];
		int numVehicle = 0;
		while(1){
			int totalVeh = 0;
			for (int i=0;i< numV;i++){
				min_dis=Max;
				v=-1;
				for (int j=0;j<numV;j++){
					if (check_cus[j]==0 && min_dis>low_cost[j]){
						v=j;
						min_dis=low_cost[j];
					}
				}
				if (v!=-1){
					if (total_dem + demCus[v] >= invLoc[l+numV] || totalVeh >= vehCap)break;
					check_cus[v] = 1;
					locIncCus[l].insert(v);
					total_dis_loc += min_dis;
					total_dem += demCus[v];
					totalVeh += demCus[v];
					for (int k=0;k<numV;k++){
						if (check_cus[k]==0 && D[k][v]<low_cost[k])
							low_cost[k]=D[k][v];
					}
				}
			}
			numVehicle++;
			if (total_dem + demCus[v] >= invLoc[l+numV] || total_dem + demCus[v] == totalDemand)break;
			for (int j=0;j<numV;j++)
				if (check_cus[j] == 1)
					low_cost[j]=Max;

		}
		double cost_loc = openCostLoc[l + numV] + total_dis_loc + numVehicle * usingCostVeh;
	//	locRatio.push_back({ cost_loc / double(total_dem), l+numV});
		locRatio.push_back({(double)cost_loc, l+numV});
	}
	delete [] check_cus;
	delete [] low_cost;
	//
/*	double total_dis_loc;
	int total_dem;
	for (int l=0;l<numL;l++){
		for (int i=0;i<numV;i++)check_cus[i] = 0;
		total_dis_loc = 0;
		total_dem = 0;
		for (int i=0;i<numV;i++){
			if (total_dem + demCus[nearCityLoca[l][i]] <= invLoc[l+numV]){
				total_dem += demCus[nearCityLoca[l][i]];
				total_dis_loc += D[l+numV][i];
				locIncCus[l].insert(nearCityLoca[l][i]);
			}
			else break;
		}
		double cost_loc = openCostLoc[l + numV] + total_dis_loc;
	//	locRatio.push_back({ cost_loc / double(total_dem), l+numV});
		locRatio.push_back({(double)cost_loc, l+numV});
	}
*/
}

void read_data::tabuSearchFindCompactConfiguration(){
	std::vector<std::set<int>> listLocConfigure;
	// using tabu search to determine that does this instance has perfect locations configurations
	int *sol,*best_sol, *tabu_table, *dem;
	sol = new int [numL];
	best_sol = new int [numL];
	tabu_table = new int [numL];
	for (int i=0;i<numL;i++)tabu_table[i] = 0;
	dem = new int [numL];
	int cusDem = 0;
	int obj=0,best_obj;
	// construct a initial solution
	int count;
	for (int i=0;i<numL;i++){
		dem[i] = locRatio[i].first;
	}
	while(1){
		for (int i=0;i<numL;i++)sol[i] = 0;
		int seed = rand()%numL;
		int cumLoc = 0;
		count = 0;
		while(1){
			cumLoc += invLoc[seed+numV];
			sol[seed] = 1;
			cusDem += invLoc[seed+numV];
			obj += dem[seed];
			if (cumLoc > totalDemand)break;
			count = 0;
			while(1){
				seed = rand()%numL;
				if (sol[seed] == 0){
					bool isConfilict = false;
					for (int i=0;i<numL;i++){
						if (sol[i] == 1){
							if(conflictLoc[i][seed] == 1){
								isConfilict = true;
								break;
							}
						}
					}
					if (!isConfilict)break;
				}
				count ++;
				if (count > 1000)
					break;
			}
			if (count > 1000)break;
		}
		if (count < 1000)break;
	}
	int seed;
	best_obj = obj;
	int can_node_1[100],can_node_2[100];
	int can_tabu_node_1[100],can_tabu_node_2[100];
	//
	int delta,min_delta, min_tabu_delta;
	int num_best, num_tabu_best;
	int no_improve = 0;
	double tabuLength = 0.3;
	int length_improve = 2000;
	int tabu_length = ceil(numL * tabuLength);
	int step = 1;
	int loc1, loc2;

	while (1){
		min_delta=Max;min_tabu_delta=Max;num_best=0;num_tabu_best=0;
		for (int i=0;i<numL;i++){
			if (sol[i] == 1){
				if (cusDem - invLoc[i+numV] >= totalDemand){
					delta = -dem[i];
					if (tabu_table[i] < step){
						if (delta < min_delta){
							min_delta = delta;
							can_node_1[0] = i;
							can_node_2[0] = -1;
							num_best = 1;
						}
						else if (delta == min_delta && num_best < 100){
							can_node_1[num_best] = i;
							can_node_2[num_best] = -1;
							num_best ++;
						}
					}
					else if (tabu_table[i] > step){
						if (delta < min_tabu_delta){
							min_tabu_delta = delta;
							can_tabu_node_1[0] = i;
							can_tabu_node_2[0] = -1;
							num_tabu_best = 1;
						}
						else if (delta == min_tabu_delta && num_tabu_best < 100){
							can_tabu_node_1[num_tabu_best] = i;
							can_tabu_node_2[num_tabu_best] = -1;
							num_tabu_best ++;
						}
					}
				}
				//
				for (int j=0; j<numL;j++){
					if (i == j || sol[j] == 1)continue;
					if (cusDem - invLoc[i+numV] + invLoc[j+numV] < totalDemand)continue;
					//decide conflict
					seed = j;
					bool isConfilict = false;
					for (int ii=0;ii<numL;ii++){
						if (ii == i)continue;
						if (sol[ii] == 1){
							if(conflictLoc[ii][seed] == 1){
								isConfilict = true;
								break;
							}
						}
					}
					if (isConfilict)continue;
					//

					delta = dem[j] - dem[i];
					if (tabu_table[i] <= step && tabu_table[j] <= step){
						if (delta < min_delta){
							min_delta = delta;
							can_node_1[0] = i;
							can_node_2[0] = j;
							num_best = 1;
						}
						else if (delta == min_delta && num_best < 100){
							can_node_1[num_best] = i;
							can_node_2[num_best] = j;
							num_best ++;
						}
					}
					else if (tabu_table[i] > step || tabu_table[j]> step ){
						if (delta < min_tabu_delta){
							min_tabu_delta = delta;
							can_tabu_node_1[0] = i;
							can_tabu_node_2[0] = j;
							num_tabu_best = 1;
						}
						else if (delta == min_tabu_delta && num_tabu_best < 100){
							can_tabu_node_1[num_tabu_best] = i;
							can_tabu_node_2[num_tabu_best] = j;
							num_tabu_best ++;
						}
					}
				}
			}
			else{
				seed = i;
				bool isConfilict = false;
				for (int ii=0;ii<numL;ii++){
					if (sol[ii] == 1){
						if(conflictLoc[ii][seed] == 1){
							isConfilict = true;
							break;
						}
					}
				}
				if (isConfilict)continue;
				////////////////////////////////////////////////////
				delta = dem[i];
				if (tabu_table[i] <= step){
					if (delta < min_delta){
						min_delta = delta;
						can_node_1[0] = i;
						can_node_2[0] = -2;
						num_best = 1;
					}
					else{
						can_node_1[num_best] = i;
						can_node_2[num_best] = -2;
						num_best ++;
					}
				}
				else if (tabu_table[i] > step){
					if (delta < min_tabu_delta){
						min_tabu_delta = delta;
						can_tabu_node_1[0] = i;
						can_tabu_node_2[0] = -2;
						num_tabu_best = 1;
					}
					else if (delta == min_tabu_delta && num_tabu_best < 100){
						can_tabu_node_1[num_tabu_best] = i;
						can_tabu_node_2[num_tabu_best] = -2;
						num_tabu_best ++;
					}
				}
			}
		}
		if (min_tabu_delta == Max && min_delta == Max)break;
		int rand_order;
		if ((num_tabu_best>0 && min_tabu_delta < min_delta && (obj + min_tabu_delta< best_obj)) || num_best==0){
			obj += min_tabu_delta;
			rand_order=rand()% num_tabu_best;
			loc1 =can_tabu_node_1[rand_order];
			loc2 =can_tabu_node_2[rand_order];
			tabu_table[loc1]=step+tabu_length+rand()%tabu_length;
			if (loc2 >-1)
				tabu_table[loc2]=step+tabu_length+rand()%tabu_length;
		}
		else{
			obj += min_delta;
			rand_order=rand()%num_best;
			loc1 = can_node_1[rand_order];
			loc2 = can_node_2[rand_order];
			tabu_table[loc1]=step+tabu_length+rand()%tabu_length;
			if (loc2 > -1)
				tabu_table[loc2]=step+tabu_length+rand()%tabu_length;
		}
		if (loc2 > -1){
			sol[loc1] = 0;
			sol[loc2] = 1;
			cusDem += invLoc[loc2 + numV];
			cusDem = cusDem - invLoc[loc1 +numV];
		}
		else if (loc2 == -1){
			sol[loc1] = 0;
			cusDem = cusDem - invLoc[loc1 + numV];
		}
		else if (loc2 == -2){
			sol[loc1] = 1;
			cusDem += invLoc[loc1 + numV];
		}
		//
		if (best_obj > obj){
			best_obj = obj;
			for (int i=0;i<numL;i++)
				best_sol[i] = sol[i];
		}
		else	no_improve ++;
		step++;
		if (no_improve > length_improve){
	//		disturbation(sol, cusDem);
	//		no_improve = 0;

			break;
		}
		//
/*		for (int i=0;i<numL;i++)
			std::cout<<i<<" ";
		std::cout<<std::endl;
		for (int i=0;i<numL;i++)
			std::cout<<sol[i]<<" ";
		std::cout<<std::endl;
*/
		//
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
			if ((int)LocConfigure.size() > numLocConfiguration){
				std::sort(LocConfigure.begin(), LocConfigure.end());
				LocConfigure.pop_back();
			}
			LocConfigure.push_back({cost,depot});
		}
		int sum =0;
		for (int i=0;i<numL;i++)
			sum += sol[i];

//		if (sol[4] == 1 && sol[6] == 1 && sol[14] == 1 && sol[16] == 1)
//			std::cout<<" the best results  "<<std::endl;
	}
	delete [] sol;
	delete [] best_sol;
	delete [] tabu_table;
	delete [] dem;
	//
//	std::sort(LocConfigure.begin(), LocConfigure.end());
	//
/*	std::set<int>depot;
	depot.insert(304);
	depot.insert(306);depot.insert(314);depot.insert(316);
	LocConfigure.push_back({100,depot});
	std::sort(LocConfigure.begin(), LocConfigure.end());
*/
/*  this code repeates the evaluated function of Schneider's paper
	std::set<int>depot;
	depot.insert(200);
	depot.insert(203);depot.insert(204);depot.insert(205);depot.insert(206);depot.insert(210);
	LocConfigure.push_back({100,depot});
	std::sort(LocConfigure.begin(), LocConfigure.end());
	return;

	LocConfigure.push_back({100,depot});
	std::vector<int>loc;
	for (int i=0;i<(int)LocConfigure.size();i++){
		loc.clear();
		auto it = LocConfigure[i].second.begin();
		for (;it != LocConfigure[i].second.end();it++)
			loc.push_back(*it);
		//computing the rating function
		double costLoc = 0;
		double part1 = 0,part2 = 0;
		for (int j=0;j<(int)loc.size();j++){
			for (int k=0;k<numV;k++)
				part1 += demCus[k] * pow(0.95, (D[loc[j]][k]* 2)/100);

			for (int k=0;k<(int)loc.size();k++)
				part2 += invLoc[loc[k]] * pow(0.95, (D[loc[j]][loc[k]] * 2)/100);
			costLoc += std::abs(part1 - part2) + openCostLoc[loc[j]];
		}
		LocConfigure[i].first = costLoc;
	}
	std::sort(LocConfigure.begin(), LocConfigure.end());
*/



/*	for (int i=0;i < (int)LocConfigure.size(); i++){
		auto it = LocConfigure[i].second.begin();
		while(it != LocConfigure[i].second.end()){
			std::cout<<*it<<" ";
			it ++;
		}
		std::cout<<std::endl;
	}
	*/
}
void read_data::constructedDepotConfigurations(){
	//the line function to estimate the coverRatio
	double min_coverRatio = 0.1;
	double max_coverRatio = 0.6;
	double kParameter = max_coverRatio - min_coverRatio;
	//
	solutionTabu = new int [dimension];
	for (int i=0;i<dimension;i++)
		solutionTabu[i] = 0;
	int seed;
	double coverRatio = 0.2;// assuming the covering ratio is 50%
	double costDepotConf;
	int countOver;
	int iter;
	int totalDem;
	std::set<int>coverVertex;
	std::vector<std::pair<double, int>>priorityOrder;// the first is the ratio of overlaping vertices and the second is the order of depot
	std::set<int>sol;
	std::vector<int>depot;
	std::vector<std::pair<double, int>>orderDepots;
	std::set<int>array;
	std::set<int>array2;
	int iteration = 0;
	int no_get = 0;
	bool isUsed;
	isUsed = true;
	while(1){
		sol.clear();
		depot.clear();
		coverVertex.clear();
		for (int i=0;i<numL;i++)depot.push_back(i);
		seed = rand() % numL;
		costDepotConf = locRatio[seed].first;
		sol.insert(seed);
		totalDem = invLoc[seed+numV];
		depot.erase(depot.begin()+seed);//
		//
		auto it = locIncCus[seed].begin();
		for (;it != locIncCus[seed].end(); it++) coverVertex.insert(*it);
		while(totalDem < totalDemand){// select other depots
			priorityOrder.clear();
			for (int i=0;i<(int)depot.size();i++){//deicde the overlapping vertices
				countOver = 0;iter = 0;
				auto it1 = locIncCus[depot[i]].begin();
				for (;it1 != locIncCus[depot[i]].end(); it1++){
					array2.insert(*it1);

					auto it2 = coverVertex.find(*it1);
					if (it2 != coverVertex.end()){
						countOver ++;
					}
					iter ++;
				}

				double var = (double)countOver / (double)iter;
				priorityOrder.push_back({var, depot[i]});
			}
			sort(priorityOrder.begin(), priorityOrder.end());
			orderDepots.clear();
			for (int i=0;i<(int)priorityOrder.size();i++){
				int cap = invLoc[priorityOrder[i].second + numV] + totalDem;
				coverRatio = kParameter * ((double)cap / (double)totalDemand) + min_coverRatio;
				if (priorityOrder[i].first > coverRatio)break;// discard these non-promising depots
				else orderDepots.push_back({locRatio[priorityOrder[i].second].first,priorityOrder[i].second});
			}

			if ((int)orderDepots.size() == 0){
				if (isUsed){
					if (iteration > 100 && (int)LocConfigure.size() == 0)
						isUsed = false;
					break;
				}
				else{
					while ((int)orderDepots.size() == 0){
						coverRatio += 0.1;
						for (int i=0;i<(int)priorityOrder.size();i++){
						//	int cap = invLoc[priorityOrder[i].second + numV] + totalDem;
						//	coverRatio = kParameter * ((double)cap / (double)totalDemand) + min_coverRatio;
							if (priorityOrder[i].first > coverRatio)break;// discard these non-promising depots
							else orderDepots.push_back({locRatio[priorityOrder[i].second].first,priorityOrder[i].second});
						}
					}
				}
			}

			sort(orderDepots.begin(),orderDepots.end());

			double y=double(rand()%10000/10000.0);
			int rank=int(pow(y,determine_para)*(int)orderDepots.size());

			seed = orderDepots[rank].second;

			auto it1 = locIncCus[seed].begin();
			for (;it1 != locIncCus[seed].end(); it1++)
				coverVertex.insert(*it1);
			int index;
			for (int i=0;i<(int)depot.size();i++)
				if (depot[i] == seed)
					index = i;
			depot.erase(depot.begin() + index);
			sol.insert(seed);
			costDepotConf += locRatio[seed].first;
			totalDem += invLoc[seed+numV];
		}
		if (orderDepots.size() == 0){
			no_get ++;
			iteration ++;
			continue;
		}


		it = sol.begin();
		array.clear();
		long long int binaryVal = 0;
		for (;it != sol.end();it++){
			array.insert(*it + numV);
			binaryVal += pow(2,*it);
		}
		// solution based
		if(solutionTabu[binaryVal % dimension] == 0){
			solutionTabu[binaryVal % dimension] = 1;
			//
			LocConfigure.push_back({costDepotConf,array});
			if ((int)LocConfigure.size() > numLocConfiguration){
				std::sort(LocConfigure.begin(), LocConfigure.end());
				std::cout<<"finishing construction depots configurations"<<std::endl;
				break;
			}
			no_get = 0;
		}
		else no_get ++;
		if (no_get > 1000){
			std::sort(LocConfigure.begin(), LocConfigure.end());
			std::cout<<"finishing construction depots configurations"<<std::endl;
			std::cout<<"time consuming  "<<float((double)clock()/(double)CLOCKS_PER_SEC)<<std::endl;
			break;
		}
		iteration++;
	//	std::cout<<iteration<<"   "<<(int)LocConfigure.size()<<std::endl;
	}
	delete [] solutionTabu;
}

void read_data::readDepotConfigurations(){
	std::vector<int>array;
	std::string var;
	if (pathDepotCon != "NULL"){
		std::ifstream inputFile(pathDepotCon);
		if (inputFile.is_open()){
			int numDepotCon;
			inputFile >> numDepotCon;
			for (int i=0;i<numDepotCon;i++){
				array.clear();
				inputFile >> var;
				while(var != "e"){
					array.push_back(std::stoi(var) + numV);
					inputFile >> var;
				}
				depotCon.push_back(array);
			}
		}
		else
			std::cout<<"The input depot configuration file is wrong"<<std::endl;
	}
	else return;
}
