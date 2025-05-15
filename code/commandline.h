/*
 * commandline.h
 *
 *  Created on: 22 Jun 2021
 *      Author: Peng
 */

#ifndef COMMANDLINE_H_
#define COMMANDLINE_H_
#include "basic.h"
class CommandLine{
public:
	std::string pathInstance;
	std::string pathDepotCon;
	std::string pathSolution;
	int timeLimit=Max;
	int iterLimit;
	int seed=0;
	double bestDis;
	int isOptimal;
	CommandLine(int argc, char* argv[]){
		if (argc!=8 && argc != 9){
			std::cout << "the input parameters in configuration are wrong" <<std::endl;
			exit(0);
		}
		else if (argc == 8){
			pathInstance=std::string(argv[1]);
			pathDepotCon = "NULL";
			pathSolution=std::string(argv[2]);
			sscanf(argv[3],"%lf",&bestDis);
			sscanf(argv[4],"%d",&isOptimal);
			sscanf(argv[5],"%d",&timeLimit);
			sscanf(argv[6],"%d",&iterLimit);
			sscanf(argv[7],"%d",&seed);
			display_infor();
		}
		else if (argc == 9){
			pathInstance=std::string(argv[1]);
			pathDepotCon=std::string(argv[2]);
			pathSolution=std::string(argv[3]);
			sscanf(argv[4],"%lf",&bestDis);
			sscanf(argv[5],"%d",&isOptimal);
			sscanf(argv[6],"%d",&timeLimit);
			sscanf(argv[7],"%d",&iterLimit);
			sscanf(argv[8],"%d",&seed);
			display_infor();
		}
	}
	void display_infor(){
		std::cout<<"Starting The Algorithm"<<std::endl;
		std::cout<<"The Instance is "<<pathInstance<<std::endl;
	}
};
#endif /* COMMANDLINE_H_ */
