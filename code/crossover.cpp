/*
 * crossover.cpp
 *
 *  Created on: 7 Dec 2020
 *      Author: Peng
 */
#include"crossover.h"

crossover::crossover(read_data * data,  popManager * popman) {
	this->data=data;
	this->popman=popman;
	this->numV = data->numV;
	this->numL = data->numL;
	this->numC = data->numC;
	sizeSubPop = numC / 2;
/******************************************/
	AcapB=new Node1 [numC];
	for (int i=0;i<numV;i++){
		AcapB[i].edgeF=new Node0 [2];
		AcapB[i].edgeM=new Node0 [2];
	}
	for (int i=numV;i<numC;i++){
		AcapB[i].edgeF=new Node0 [data->numC*2];
		AcapB[i].edgeM=new Node0 [data->numC*2];
	}
	ABcycles=new Node2 [numC/2];
	for (int i=0;i<(int)numC/2;i++)
		ABcycles[i].sque=new Node0 [numC*4];
	auxArray1=new int [numC];
	auxArray2=new int [numC];
	auxArray3=new Node6 [numC];

	whichCycles = std::vector<std::set <int > >(numC);
	commconCycles = std::vector<std::set <int > >(numC);

	eSets=new Node5 [numC/2];
	existCity=new int [numC];
	//
	gStart = new Node [numC];
	gEnd = new Node [numC];
	for (int i=0;i<numV; i++){
		gStart[i].dem = 0;
		gStart[i].cour = -1;
		gStart[i].isDepot = false;
		gEnd[i].dem = 0;
		gEnd[i].cour = -1;
		gEnd[i].isDepot = false;
	}
	for (int i=numV;i<numC;i++){
		gStart[i].dem = 0;
		gStart[i].cour = i;
		gStart[i].isDepot = true;
		gStart[i].pre = NULL;
		gStart[i].next = NULL;
		gEnd[i].dem = 0;
		gEnd[i].cour = i;
		gEnd[i].isDepot = true;
		gEnd[i].pre = NULL;
		gEnd[i].next = NULL;
	}
	auxA1 = new bool [numV];
	auxA2 = new bool [numV];
}
crossover::~crossover() {
	// TODO Auto-generated destructor stub
	for (int i=0;i<numC;i++){
		delete [] AcapB[i].edgeF;
		delete [] AcapB[i].edgeM;
	}
	delete [] AcapB;
	for (int i=0;i<(int)numC/2;i++)
		delete [] ABcycles[i].sque;
	delete [] ABcycles;
	delete [] auxArray1;
	delete [] auxArray2;
	delete [] auxArray3;
	delete [] eSets;
	delete [] existCity;
	//
	delete [] gStart;
	delete [] gEnd;
	delete [] auxA1;
	delete [] auxA2;
}
//////////////////////////////////////////////////////////////////
void crossover::loadFaCapMa(){
	for (int i = 0;i < numC;i++){
		AcapB[i].nbeF = 0;
		AcapB[i].nbeM = 0;
	}
	Node *p = NULL;
	for (int i = 0; i < numL; i++){// extract all edges from father solution
		if (fa->location[i].used){
			for (int j = 0; j<fa->location[i].maxVisit; j++){
				if (fa->location[i].depots[j].route->used){
					p = &fa->location[i].depots[j];
					if (ma->client[p->next->cour].pre->cour != p->cour && ma->client[p->next->cour].next->cour != p->cour){
						AcapB[p->cour].edgeF[AcapB[p->cour].nbeF].cour = p->next->cour;
						AcapB[p->cour].nbeF++;
					}
					p = p->next;
					while(!p->isDepot){
						if (ma->client[p->cour].pre->cour != p->next->cour && ma->client[p->cour].next->cour != p->next->cour){
							AcapB[p->cour].edgeF[AcapB[p->cour].nbeF].cour = p->next->cour;
							AcapB[p->cour].nbeF++;
						}
						if (ma->client[p->cour].pre->cour != p->pre->cour && ma->client[p->cour].next->cour != p->pre->cour){
							AcapB[p->cour].edgeF[AcapB[p->cour].nbeF].cour = p->pre->cour;
							AcapB[p->cour].nbeF++;
						}
						p = p->next;
					}
					if (ma->client[p->pre->cour].pre->cour != p->cour && ma->client[p->pre->cour].next->cour != p->cour){
						AcapB[p->cour].edgeF[AcapB[p->cour].nbeF].cour = p->pre->cour;
						AcapB[p->cour].nbeF++;
					}
				}
			}
		}
		if (ma->location[i].used){
			for (int j=0; j<ma->location[i].maxVisit; j++){
				if (ma->location[i].depots[j].route->used){
					p = &ma->location[i].depots[j];
					if (fa->client[p->next->cour].pre->cour != p->cour && fa->client[p->next->cour].next->cour != p->cour){
						AcapB[p->cour].edgeM[AcapB[p->cour].nbeM].cour = p->next->cour;
						AcapB[p->cour].nbeM++;
					}
					p = p->next;
					while(!p->isDepot){
						if (fa->client[p->cour].pre->cour != p->next->cour && fa->client[p->cour].next->cour != p->next->cour){
							AcapB[p->cour].edgeM[AcapB[p->cour].nbeM].cour = p->next->cour;
							AcapB[p->cour].nbeM++;
						}
						if (fa->client[p->cour].pre->cour != p->pre->cour && fa->client[p->cour].next->cour != p->pre->cour){
							AcapB[p->cour].edgeM[AcapB[p->cour].nbeM].cour = p->pre->cour;
							AcapB[p->cour].nbeM++;
						}
						p = p->next;
					}
					if (fa->client[p->pre->cour].pre->cour != p->cour && fa->client[p->pre->cour].next->cour != p->cour){
						AcapB[p->cour].edgeM[AcapB[p->cour].nbeM].cour = p->pre->cour;
						AcapB[p->cour].nbeM++;
					}
				}
			}
		}
	}
	// add extra remedy strategies
	p = NULL;
	for (int i=0; i<numV; i++){
		if (fa->client[i].pre->cour == fa->client[i].next->cour){
			if ((ma->client[i].pre->cour == fa->client[i].pre->cour && ma->client[i].next->cour != fa->client[i].pre->cour)
				|| (ma->client[i].next->cour == fa->client[i].pre->cour && ma->client[i].pre->cour != fa->client[i].pre->cour)){
				p = &fa->client[i];
				AcapB[p->cour].edgeF[AcapB[p->cour].nbeF].cour = p->pre->cour;
				AcapB[p->cour].nbeF++;
				AcapB[p->pre->cour].edgeF[AcapB[p->pre->cour].nbeF].cour = p->cour;
				AcapB[p->pre->cour].nbeF++;
			}
		}
		if (ma->client[i].pre->cour == ma->client[i].next->cour){
			if ((fa->client[i].pre->cour == ma->client[i].pre->cour && fa->client[i].next->cour != ma->client[i].pre->cour)
				|| (fa->client[i].next->cour == ma->client[i].pre->cour && fa->client[i].pre->cour != ma->client[i].pre->cour)){
				p = &ma->client[i];
				AcapB[p->cour].edgeM[AcapB[p->cour].nbeM].cour = p->pre->cour;
				AcapB[p->cour].nbeM++;
				AcapB[p->pre->cour].edgeM[AcapB[p->pre->cour].nbeM].cour = p->cour;
				AcapB[p->pre->cour].nbeM++;
			}
		}
	}
	//
	int nbDummy;// adding dummy edges
	for (int i=numV;i<numC;i++){
		if (AcapB[i].nbeF == AcapB[i].nbeM)continue;
		if (AcapB[i].nbeF > AcapB[i].nbeM){
			nbDummy=(AcapB[i].nbeF - AcapB[i].nbeM)/2;
			for (int j=0; j<nbDummy; j++){
				AcapB[i].edgeM[AcapB[i].nbeM].cour=-1;
				AcapB[i].nbeM++;
			}
		}
		else{
			nbDummy=(AcapB[i].nbeM - AcapB[i].nbeF)/2;
			for (int j=0; j<nbDummy; j++){
				AcapB[i].edgeF[AcapB[i].nbeF].cour=-1;
				AcapB[i].nbeF++;
			}
		}
	}
	//output all edges
//	if (data->iteration == Max){
//		for (int i=0;i<numC;i++){
//			if (AcapB[i].nbeF == 0 && AcapB[i].nbeM == 0)continue;
//			std::cout<<i<<"  ";
//			for (int j=0;j<AcapB[i].nbeF;j++)
//				std::cout<<AcapB[i].edgeF[j].cour<<" ";
//			std::cout<<"       ";
//			for (int j=0;j<AcapB[i].nbeM;j++)
//				std::cout<<AcapB[i].edgeM[j].cour<<" ";
//			std::cout<<"       "<<std::endl;
//		}
//	}
}
void crossover::addNodeToABcycles(int &nb, Node0 &tem){
	ABcycles[nb].sque[ABcycles[nb].nbCity].cour=tem.cour;
	ABcycles[nb].sque[ABcycles[nb].nbCity].isFather=tem.isFather;
	ABcycles[nb].nbCity++;
}
void crossover::emptyACity(int &city, int &numAvaCity){
	auxArray2[auxArray1[city]]=auxArray2[numAvaCity-1];
	auxArray1[auxArray2[numAvaCity-1]]=auxArray1[city];
	numAvaCity--;
}
void crossover::extractABcycles(){
	for (int i=0;i<sizeSubPop;i++)ABcycles[i].nbCity=0;
	int numAvaCity=0;
	for (int i=0;i<numC;i++){
		whichCycles[i].clear();
		if (!(AcapB[i].nbeF==0 && AcapB[i].nbeM==0)){
			auxArray2[numAvaCity]=i;// this array is used to find random city at the beginning of ABcycles.
			auxArray1[i]=numAvaCity;
			numAvaCity++;
		}
	}
	//
	Node0 startCity,ori,rCity;bool father;int pos,count;//int city1;
	nbABcycles=0;
	int isF,isM;
	while(numAvaCity){// the isFather is used to define the edge (x,y) come from which solution, if isfather is true,
		ABcycles[nbABcycles].nbCity=0;// the edge (x,y) comes from father, otherwise, it comes from mother solution
		ori.cour=rand()%numAvaCity;
		ori.cour=auxArray2[ori.cour];
		//father=false;
		isF=0;isM=0;
		for (int i=0;i<AcapB[ori.cour].nbeF;i++)
			if (AcapB[ori.cour].edgeF[i].cour > -1){
				isF=1;break;
			}
		for (int i=0;i<AcapB[ori.cour].nbeM;i++)
			if (AcapB[ori.cour].edgeM[i].cour > -1){
				isM=1;break;
			}
		if (isF==1 && isM==0)father=true;
		if (isF==0 && isM==1)father=false;
		if (isF==1 && isM==1){
			if (rand()%2==0)father=true;
			else father=true;
		}
		//
		ori.isFather=father;
		count=1;
		startCity.cour=ori.cour;// the startCity.dem will be determine in the last step
		addNodeToABcycles(nbABcycles, ori);
		rCity.cour=-2;
		while(1){
			if (father){
				pos=rand()%AcapB[ori.cour].nbeF;
				while (ABcycles[nbABcycles].nbCity==1 && AcapB[ori.cour].edgeF[pos].cour ==-1)// to avoid select dummy edge at first
					pos=rand()%AcapB[ori.cour].nbeF;
				//to select a substring from the same solution
				rCity.cour=AcapB[ori.cour].edgeF[pos].cour;
				father=false;
				rCity.isFather=father;
			}
			else{
				pos=rand()%AcapB[ori.cour].nbeM;
				while (ABcycles[nbABcycles].nbCity == 1 && AcapB[ori.cour].edgeM[pos].cour ==-1)
					pos=rand()%AcapB[ori.cour].nbeM;
				//
				rCity.cour=AcapB[ori.cour].edgeM[pos].cour;
				father=true;
				rCity.isFather=father;
			}
			if (rCity.cour > -1){
				addNodeToABcycles(nbABcycles, rCity);
				//
				if (!father){
					for (int i=pos;i<AcapB[ori.cour].nbeF-1;i++)
						AcapB[ori.cour].edgeF[i].cour=AcapB[ori.cour].edgeF[i+1].cour;
					AcapB[ori.cour].nbeF--;
					//
					for (int i=0;i<AcapB[rCity.cour].nbeF;i++)
						if (AcapB[rCity.cour].edgeF[i].cour==ori.cour){
							pos=i;break;
						}
					//
					for (int i=pos;i<AcapB[rCity.cour].nbeF-1;i++)
						AcapB[rCity.cour].edgeF[i].cour=AcapB[rCity.cour].edgeF[i+1].cour;
					AcapB[rCity.cour].nbeF--;
				}
				else{
					for (int i=pos;i<AcapB[ori.cour].nbeM-1;i++)
						AcapB[ori.cour].edgeM[i].cour=AcapB[ori.cour].edgeM[i+1].cour;
					AcapB[ori.cour].nbeM--;
					for (int i=0;i<AcapB[rCity.cour].nbeM;i++)
						if (AcapB[rCity.cour].edgeM[i].cour==ori.cour){
							pos=i;break;
						}
					//
					for (int i=pos;i<AcapB[rCity.cour].nbeM-1;i++)
						AcapB[rCity.cour].edgeM[i].cour=AcapB[rCity.cour].edgeM[i+1].cour;
					AcapB[rCity.cour].nbeM--;
				}
				if (AcapB[ori.cour].nbeF == 0 && AcapB[ori.cour].nbeM == 0)
					emptyACity(ori.cour, numAvaCity);
				if (AcapB[rCity.cour].nbeF == 0 && AcapB[rCity.cour].nbeM == 0)
					emptyACity(rCity.cour, numAvaCity);
				ori.cour=rCity.cour;
				ori.isFather=rCity.isFather;
			}
			else{
				if (!father){
					for (int i=pos; i<AcapB[ori.cour].nbeF-1; i++)
						AcapB[ori.cour].edgeF[i].cour=AcapB[ori.cour].edgeF[i+1].cour;
					AcapB[ori.cour].nbeF--;
				}
				else{
					for (int i=pos; i<AcapB[ori.cour].nbeM-1; i++)
						AcapB[ori.cour].edgeM[i].cour=AcapB[ori.cour].edgeM[i+1].cour;
					AcapB[ori.cour].nbeM--;
				}
				//to determine the edge comes from the right solution
				if(ABcycles[nbABcycles].sque[ABcycles[nbABcycles].nbCity-1].isFather)
					ABcycles[nbABcycles].sque[ABcycles[nbABcycles].nbCity-1].isFather=false;
				else
					ABcycles[nbABcycles].sque[ABcycles[nbABcycles].nbCity-1].isFather=true;
				//
				if (AcapB[ori.cour].nbeF == 0 && AcapB[ori.cour].nbeM == 0)
					emptyACity(ori.cour, numAvaCity);
			}

			whichCycles[ori.cour].insert(nbABcycles);
			if (ori.cour == startCity.cour && count > 3 && count%2 ==0 ){
				whichCycles[startCity.cour].insert(nbABcycles);
				nbABcycles++;
				if (nbABcycles == sizeSubPop)return;
				break;
			}
/*			if (ori.cour == startCity.cour && count == 2 && AcapB[ori.cour].nbeF == 0 ){
				whichCycles[startCity.cour].insert(nbABcycles);
				nbABcycles++;
				if (nbABcycles == sizeSubPop)return;
				break;
			}
*/
			count++;
		}
	}
/*	if (data->iteration == 76281){
		std::cout<<"Output the ABcycles"<<std::endl;
		if (1){
			for (int i = 0; i < nbABcycles; i++){
				for (int j = 0; j < ABcycles[i].nbCity; j++){
					std::cout<<ABcycles[i].sque[j].cour<<" ";
				}
				std::cout<<std::endl;
			}
		}
	}
*/
}
void crossover::generationEsets(){
	for (int i=0;i<numC/2;i++) eSets[i].cycles.clear();// clear all eSets

	for (int i=0;i<numC/2;i++){
		commconCycles[i].clear();
		if (whichCycles[i].empty())continue;
		std::set < int >::iterator k1 = whichCycles[i].begin();
		if (whichCycles[i].size() != 1){
			while (k1 != whichCycles[i].end()){
				std::set < int >::iterator k2=k1;		k2++;
				commconCycles[*k1].insert(*k2);
				commconCycles[*k2].insert(*k1);
				k1++;
			}
		}
		else
			commconCycles[*k1].insert(*k1);
	}
	//to eliminate the same cycles
	for (int i=0; i<nbABcycles; i++){
		for (int j=0; j<i; j++){
			if (commconCycles[j].empty())continue;
			if (commconCycles[i].size() != commconCycles[j].size())continue;
			bool theSame=true;
			std::set < int >::iterator k1 = commconCycles[i].begin();
			std::set < int >::iterator k2 = commconCycles[j].begin();
			while(k1 != commconCycles[i].end() && k2 != commconCycles[j].end()){
				int r1= *k1; int r2= *k2;
				if (r1 != r2){
					theSame=false;
					break;
				}
				k1++;
				k2++;
			}
			if (theSame)
				commconCycles[j].clear();
		}
	}
	nbeSets=0;
	//strategy 1 ///common nodes
	if (data->eSets_strategy == 1){
		for (int i=0; i<nbABcycles; i++){
			if (commconCycles[i].empty())continue;
			std::set < int >::iterator k1 = commconCycles[i].begin();
			while(k1 != commconCycles[i].end()){
				eSets[nbeSets].cycles.insert(*k1);
				k1++;
			}
			nbeSets++;
		}
		return;
	}
	//strategy 2 partition ABcycles////////////////////////////////////////////
	int actualNbCycles = nbABcycles;
	int reccord=0;int iiter=1;
	//
	int bEta=3;//**********************************************************************************
	//*********************************************************************************************
	if (data->eSets_strategy == 2){
		if (actualNbCycles < bEta){
			for (int i=0; i < actualNbCycles;i++){
				eSets[nbeSets].cycles.insert(i);
				nbeSets++;
			}
			return;
		}
		for (int ii=0;ii<bEta;ii++){
			for (int i=reccord;i<iiter*actualNbCycles/bEta;i++){
				eSets[nbeSets].cycles.insert(i);
				reccord++;
			}
			nbeSets++;
			iiter++;
		}
		return;
	}
	// strategy 3 partition some AB-cycles with common nodes
	int nbCommonCycles=0;
	for (int i=0; i<nbABcycles; i++)
		if (!commconCycles[i].empty())
			nbCommonCycles++;
	actualNbCycles = nbCommonCycles;
	reccord=0;
	iiter=1;
	if (data->eSets_strategy == 3){
		if (actualNbCycles < bEta){
			for (int i=0; i<nbABcycles; i++){
				if (commconCycles[i].empty())continue;
				std::set < int >::iterator k1 = commconCycles[i].begin();
				while(k1 != commconCycles[i].end()){
					eSets[nbeSets].cycles.insert(*k1);
					k1++;
				}
				nbeSets++;
			}
			return;
		}
		//////////////////////////////////////////////////////////////////
		for (int ii=0;ii<bEta;ii++){
			int jj=0;
			for (int i=reccord;i<iiter*actualNbCycles/bEta;){
				if (commconCycles[jj].empty()){
					jj++;
					continue;
				}
				std::set < int >::iterator k1 = commconCycles[i].begin();
				while(k1 != commconCycles[i].end()){
					eSets[nbeSets].cycles.insert(*k1);
					k1++;
				}
				reccord++;
				i++;
				jj++;
			}
			nbeSets++;
			iiter++;
		}
		return;
	}
}
void crossover::flipSubString(Node *nodeU, Node *nodeV,bool reverse){
	Node *tem=NULL;
	if (!reverse){
		while(nodeU != nodeV){
			tem=nodeU->next;
			nodeU->next=nodeU->pre;
			nodeU->pre=tem;
			nodeU=tem;
		}
		tem=nodeU->next;
		nodeU->next=nodeU->pre;
		nodeU->pre=tem;
	}
	else{
		while(nodeU != nodeV){
			tem=nodeU->pre;
			nodeU->pre=nodeU->next;
			nodeU->next=tem;
			nodeU=tem;
		}
		tem=nodeU->pre;
		nodeU->pre=nodeU->next;
		nodeU->next=tem;
	}
}
void crossover::generationIntermediateSolutions(){
	int p1,p2;
	Node *tem=NULL, *tem2=NULL;
	for (int i=0;i<nbeSets;i++){
		popman->copyAllData(subPop+i, fa);
		off=subPop+i;
		//the first phase is to delete all edges from father solution
		std::set <int>::iterator k = eSets[i].cycles.begin();
		for (;k!=eSets[i].cycles.end();k++){
			for (int j=0;j<ABcycles[*k].nbCity-1;j++){
				if (ABcycles[*k].sque[j].isFather){
					p1 = ABcycles[*k].sque[j].cour;
					p2 = ABcycles[*k].sque[j+1].cour;
					//check the ABcycles
					if (p1 == p2 ){std::cout<<"the ABcycles is wrong with that an edge starts and ends at the same city"<<std::endl;exit(0);}

					Node *pp1, *pp2;
					if (p1 >= numV){// p1 is the location
						if (off->client[p2].pre != NULL && off->client[p2].pre->cour == p1)
							pp1 = off->client[p2].pre;
						else if (off->client[p2].next != NULL && off->client[p2].next->cour == p1)
							pp1 = off->client[p2].next;
						pp2 = &off->client[p2];
					}
					else if (p2 >= numV){
						if (off->client[p1].pre != NULL && off->client[p1].pre->cour == p2)
							pp2 = off->client[p1].pre;
						else if (off->client[p1].next != NULL &&off->client[p1].next->cour == p2)
							pp2 = off->client[p1].next;
						pp1 = &off->client[p1];
					}
					else {
						pp1 = &off->client[p1];
						pp2 = &off->client[p2];
					}
					//
					if (pp1->pre != NULL && pp1->pre == pp2){
						pp1->pre=NULL;
						pp2->next=NULL;
						continue;
					}
					if (pp1->next != NULL && pp1->next == pp2){
						pp1->next=NULL;
						pp2->pre=NULL;
					}
				}
			}
		}
		////////////////////////////////////////////////////////////////////////////
		int p1Before,p2Before;
		for (k=eSets[i].cycles.begin();k!=eSets[i].cycles.end();k++){
			for (int j=0;j<ABcycles[*k].nbCity-1;j++){
				if ( ! ABcycles[*k].sque[j].isFather){// all edges from the mother solution
					p1=ABcycles[*k].sque[j].cour;
					p2=ABcycles[*k].sque[j+1].cour;
					if (p1==p2 ){std::cout<<"the ABcycles is wrong with that an edge starts and ends at the same city"<<std::endl;exit(0);}

					if (p1 >= numV){
						if (off->client[p2].pre == NULL)
							off->client[p2].pre = &gStart[p1];
						else if (off->client[p2].next == NULL)
							off->client[p2].next = &gEnd[p1];
					}
					else{
						if (p2 >= numV){
							if (off->client[p1].pre == NULL)
								off->client[p1].pre = &gStart[p2];
							else if (off->client[p1].next == NULL)
								off->client[p1].next = &gEnd[p2];
						}
						else{
							p1Before=0;
							while(1){
								if (off->client[p1].pre==NULL){
									p1Before=1;break;
								}
								if (off->client[p1].next==NULL){
									p1Before=-1;break;
								}
							}
							p2Before=0;
							while(1){
								if (p1Before == 1 && off->client[p2].next == NULL){
									p2Before=-1;break;
								}
								if (p1Before == -1 && off->client[p2].pre == NULL){
									p2Before=1;break;
								}
								if (off->client[p2].pre==NULL){
									p2Before=1;break;
								}
								if (off->client[p2].next==NULL){
									p2Before=-1;break;
								}
							}
							// determine four cases
							if (p1Before == 1 && p2Before == -1){
								off->client[p1].pre=&off->client[p2];
								off->client[p2].next=&off->client[p1];
								continue;
							}
							if (p1Before == -1 && p2Before == 1){
								off->client[p1].next=&off->client[p2];
								off->client[p2].pre=&off->client[p1];
								continue;
							}
							if (p1Before ==1 && p2Before ==1){
								tem=&off->client[p2];
								tem2=tem;
								//while(tem2->next != NULL && tem2->next->cour != 0)
								while(tem2->next != NULL)
									tem2=tem2->next;
								flipSubString(tem,tem2,false);
								off->client[p1].pre=&off->client[p2];
								off->client[p2].next=&off->client[p1];
								continue;
							}
							if (p1Before == -1 && p2Before ==-1){
								tem=&off->client[p2];
								tem2=tem;
								//while(tem2->pre != NULL && tem2->pre->cour != 0)
								while(tem2->pre != NULL)
									tem2=tem2->pre;
								flipSubString(tem,tem2,true);
								off->client[p1].next=&off->client[p2];
								off->client[p2].pre=&off->client[p1];
								continue;
							}
						}
					}
				}
			}
		}
		//identify extra edges from father solution
		for (int k=0;k<numV;k++){
			if (off->client[k].pre == NULL && off->client[k].next != NULL)
				std::cout<<"report a wrong left   "<< k<<std::endl;
			if (off->client[k].pre != NULL && off->client[k].next == NULL)
				std::cout<<"report a wrong right   "<< k<<std::endl;
		}

		updateTheLocation();
		eliminateSubtours();/*    STEP 5*/
		checkTheSolution();
//		if (data->iteration == 30)
//		off->outputSolution();
	}
}
void crossover::checkTheSolution(){
	for (int i=0;i<numV;i++)auxA2[i] = false;
	Node *p;
	for (int i=0;i<numL; i++){
		if (off->location[i].used){
			for (int j=0;j<off->location[i].maxVisit; j++){
				if (off->location[i].depots[j].route->used){
					p = off->location[i].depots[j].next;
					while(!p->isDepot){
						auxA2[p->cour] = true;
						p = p->next;
					}
				}
			}
		}
	}
	for (int i=0;i<numV;i++){
		if (!auxA2[i]){
			std::cout<<"The intermediate solution is wrong and the customer "<<i<<" is wrong"<<std::endl;
		}
	}
}
void crossover::updateTheLocation(){
	//initialization the offspring solution
	headSubstring.clear();
	for (int i=0;i<numL;i++){
		for (int j=0;j<off->location[i].maxVisit; j++){
			off->location[i].depots[j].next = &off->location[i].endDepots[j];
			off->location[i].depots[j].pre = &off->location[i].endDepots[j];
			off->location[i].depots[j].route->load = 0;
			off->location[i].endDepots[j].pre = &off->location[i].depots[j];
			off->location[i].endDepots[j].next = &off->location[i].depots[j];
		}
		int ll = i+numV;
		off->updateLocationInfor(ll);
	}
	for (int i=0;i<numV;i++)auxA1[i] = false;// if the customer is considered, the value is true.
	//
	Node *p, *start;
	bool isFirst, subtour;
	Node *nodeU, *nodeV;
	for (int i=0;i<numV;i++){
		if (!auxA1[i]){
			nodeU = NULL; nodeV = NULL;
			p = &off->client[i];
			start = p;
			auxA1[p->cour] = true;
			isFirst = true;subtour = true;
			while (p != start || isFirst){
				if (p->next->cour >= numV){
					subtour = false;
					p->route = NULL;
					nodeU = p;
					break;
				}
				p->route = NULL;
				p = p->next;
				auxA1[p->cour] = true;
				isFirst = false;
			}
			if (subtour){// this is a subtour and keep the subtour head
				headSubstring.push_back(start);
				continue;
			}
			p = start;isFirst = true;
			while(p != start || isFirst){
				p->route = NULL;
				if (p->pre->cour >= numV){
					p->route = NULL;
					nodeV = p;
					break;
				}
				p = p->pre;
				auxA1[p->cour] = true;
				isFirst = false;
			}
			//finish the extract tour and then decide if the giant tour.
			if (nodeU != NULL && nodeV != NULL && nodeU->next->cour == nodeV->pre->cour){// if no giant tour
				int loc = nodeU->next->cour-numV;
				off->location[loc].depots[off->location[loc].splitTime].next = nodeV;
				nodeV->pre = &off->location[loc].depots[off->location[loc].splitTime];
				off->location[loc].endDepots[off->location[loc].splitTime].pre = nodeU;
				nodeU->next = &off->location[loc].endDepots[off->location[loc].splitTime];
				Route * routeU = off->location[loc].depots[off->location[loc].splitTime].route;
				while(nodeU != nodeV){// update the route information
					nodeV->route = routeU;
					nodeV = nodeV->next;
				}
				nodeU->route = routeU;
				off->updateRouteInfor(off->location[loc].depots[off->location[loc].splitTime].route->cour,-1);
				off->location[loc].splitTime++;
				loc = loc + numV;
				off->updateLocationInfor(loc);// update the location
			}
			else if (nodeU != NULL && nodeV != NULL && nodeU->next->cour != nodeV->pre->cour){// if giant tours
				p = nodeV;// start
				p->cumulatedLoad = p->dem;
				while (p != nodeU){
					p = p->next;
					p->cumulatedLoad = p->pre->cumulatedLoad + p->dem;
				}
				// for giant tours, there are two cases to divide it, the first is related to capacity of depots, the second is related to non-capacity of depots
				// for vehicles, there are also two cases, the first is about fixed cost of using vehicles, the second is about non-fixed cost of using vehicles.
				Node *depot = NULL;
				if (off->location[nodeU->next->cour - numV].iscapCon){// if the location constrained by capacity
					int loc1 = nodeV->pre->cour - numV;	int loc2 = nodeU->next->cour - numV;
					if (off->location[loc1].remainInven + p->cumulatedLoad <= off->location[loc1].inventory
							&& off->location[loc2].remainInven + p->cumulatedLoad <= off->location[loc2].inventory){
						if ((off->location[loc1].inventory - off->location[loc1].remainInven - p->cumulatedLoad) <=
								(off->location[loc2].inventory - off->location[loc2].remainInven - p->cumulatedLoad))
							depot = &off->location[loc1].depots[off->location[loc1].splitTime];
						else depot = &off->location[loc2].depots[off->location[loc2].splitTime];
					}
					else if (off->location[loc1].remainInven + p->cumulatedLoad > off->location[loc1].inventory
							&& off->location[loc2].remainInven + p->cumulatedLoad <= off->location[loc2].inventory)
						depot = &off->location[loc2].depots[off->location[loc2].splitTime];
					else if (off->location[loc1].remainInven + p->cumulatedLoad <= off->location[loc1].inventory
							&& off->location[loc2].remainInven + p->cumulatedLoad > off->location[loc2].inventory)
						depot = &off->location[loc1].depots[off->location[loc1].splitTime];
					else if (off->location[loc1].remainInven + p->cumulatedLoad > off->location[loc1].inventory
							&& off->location[loc2].remainInven + p->cumulatedLoad > off->location[loc2].inventory){
						if ((off->location[loc1].remainInven + p->cumulatedLoad - off->location[loc1].inventory) <=
								(off->location[loc2].remainInven + p->cumulatedLoad - off->location[loc2].inventory))
							depot = &off->location[loc1].depots[off->location[loc1].splitTime];
						else depot = &off->location[loc2].depots[off->location[loc2].splitTime];
					}
					if (depot == NULL)std::cout<<"report a bug in assign locations"<<std::endl;
					else{
						nodeU->next = &off->location[depot->cour-numV].endDepots[off->location[depot->cour-numV].splitTime];
						off->location[depot->cour-numV].endDepots[off->location[depot->cour-numV].splitTime].pre = nodeU;
						nodeV->pre = &off->location[depot->cour-numV].depots[off->location[depot->cour-numV].splitTime];
						off->location[depot->cour-numV].depots[off->location[depot->cour-numV].splitTime].next = nodeV;
						int loc = depot->cour-numV;
						Route * routeU = off->location[loc].depots[off->location[loc].splitTime].route;
						while(nodeU != nodeV){// update the route information
							nodeV->route = routeU;
							nodeV = nodeV->next;
						}
						nodeU->route = routeU;
						off->updateRouteInfor(off->location[loc].depots[off->location[loc].splitTime].route->cour,-1);
						off->location[loc].splitTime++;
						loc = loc + numV;
						off->updateLocationInfor(loc);// update the location
					}
				}
			//	else if (data->usingCostVeh > 0){// to save only one route
				else {// to save only one route
					int loc;
					if (data->D[nodeV->cour][nodeV->pre->cour] > data->D[nodeU->cour][nodeU->next->cour])// decide which location
						loc = nodeV->pre->cour-numV;
					else
						loc = nodeU->next->cour-numV;
					off->location[loc].depots[off->location[loc].splitTime].next = nodeV;
					nodeV->pre = &off->location[loc].depots[off->location[loc].splitTime];
					off->location[loc].endDepots[off->location[loc].splitTime].pre = nodeU;
					nodeU->next = &off->location[loc].endDepots[off->location[loc].splitTime];
					nodeV->route = off->location[loc].depots[off->location[loc].splitTime].route;
					while (nodeV != nodeU){
						nodeV = nodeV->next;
						nodeV->route= nodeV->pre->route;
					}
					nodeU->route = nodeV->pre->route;
					off->updateRouteInfor(off->location[loc].depots[off->location[loc].splitTime].route->cour,-1);
					off->location[loc].splitTime++;
					loc = loc + numV;
					off->updateLocationInfor(loc);// update the location
				}
/*				else{// when no vehicle costs, we need to split these routes
					double min_delta,delta;
					int pos;
					min_delta = Max;
					delta = data->D[nodeV->cour][nodeU->next->cour] - data->D[nodeV->pre->cour][nodeV->cour];
					if (delta < min_delta){
						min_delta = delta;
						pos = nodeV->pre->cour;
					}
					Node *pointer= nodeV;
					while (pointer->cour != nodeU->next->cour){
						delta = data->D[pointer->cour][nodeV->pre->cour] + data->D[pointer->next->cour][nodeU->next->cour] - data->D[pointer->cour][pointer->next->cour];
						if (delta < min_delta){
							min_delta = delta;
							pos = pointer->cour;
						}
						pointer = pointer->next;
					}
					// there are three cases
					if (pos == nodeV->pre->cour){
						if (nodeU->route->depot->next->isDepot){// when a route is included into the location nodeU->route->whichLocation
							Route * ru = nodeU->next->route;
							Route * rv = nodeV->pre->route;
							nodeV->pre = ru->depot;
							ru->depot->next = nodeV;
							while(nodeV != ru->endDepot){
								nodeV->route = ru;
								nodeV = nodeV->next;
							}
							rv->depot->next = rv->endDepot;
							off->updateRouteInfor(ru->cour, -1);
							off->updateRouteInfor(rv->cour, -1);
							off->updateLocationInfor(ru->whichLocation->cour);
							off->updateLocationInfor(rv->whichLocation->cour);
						}
						else{
							Route * re = &off->route[*nodeU->route->whichLocation->emptyRoutes.begin()];
							Route * ru = nodeU->next->route;
							Route * rv = nodeV->pre->route;
							ru->endDepot->pre = ru->depot;// not use this route
							nodeV->pre = re->depot;
							re->depot->next = nodeV;
							re->endDepot->pre = nodeU;
							nodeU->next = re->endDepot;
							while(nodeV != re->endDepot){
								nodeV->route = re;
								nodeV = nodeV->next;
							}
							off->updateRouteInfor(ru->cour, -1);
							off->updateRouteInfor(re->cour, -1);
							off->updateRouteInfor(rv->cour, -1);
							off->updateLocationInfor(ru->whichLocation->cour);
							off->updateLocationInfor(rv->whichLocation->cour);
						}
					}
					else if (pos == nodeU->cour){
						if (nodeV->route->endDepot->pre->isDepot){// when a route is included into the location nodeU->route->whichLocation
							Route * ru = nodeU->next->route;
							Route * rv = nodeV->pre->route;
							nodeU->next = rv->endDepot;
							rv->endDepot->pre = nodeU;
							while(nodeU != rv->depot){
								nodeU->route = rv;
								nodeU = nodeU->pre;
							}
							ru->endDepot->pre = ru->depot;
							off->updateRouteInfor(ru->cour, -1);
							off->updateRouteInfor(rv->cour, -1);
							off->updateLocationInfor(ru->whichLocation->cour);
							off->updateLocationInfor(rv->whichLocation->cour);
						}
						else{
							Route * re = &off->route[*nodeV->route->whichLocation->emptyRoutes.begin()];
							Route * ru = nodeU->next->route;
							Route * rv = nodeV->pre->route;
							rv->depot->next = rv->endDepot;// not use this route
							nodeV->pre = re->depot;
							re->depot->next = nodeV;
							re->endDepot->pre = nodeU;
							nodeU->next = re->endDepot;
							while(nodeU != re->depot){
								nodeU->route = re;
								nodeU = nodeU->pre;
							}
							off->updateRouteInfor(ru->cour, -1);
							off->updateRouteInfor(re->cour, -1);
							off->updateRouteInfor(rv->cour, -1);
							off->updateLocationInfor(ru->whichLocation->cour);
							off->updateLocationInfor(rv->whichLocation->cour);
						}
					}
					else {
						Node *split = &off->client[pos];
						Node *splitR = split->next;
						// for nodeV
						if (nodeV->pre->route == NULL){
							Route * re = &off->route[*off->location[nodeV->pre->cour-numV].emptyRoutes.begin()];
							re->depot->next = nodeV;
							re->endDepot->pre = split;
							split->next = re->endDepot;
							Node *V= nodeV;
							while(V != re->endDepot){
								V->route = re;
								V = V->next;
							}
							off->updateRouteInfor(re->cour, -1);
							off->updateLocationInfor(nodeV->pre->cour);

						}
						else{
							if (nodeV->pre->route->endDepot->pre->isDepot){
								Route * rv = nodeV->pre->route;
								rv->endDepot->pre = split;
								split->next = rv->endDepot;
								Node *V = nodeV;
								while(V!= rv->endDepot){
									V->route = rv;
									V = V->next;
								}
								off->updateRouteInfor(rv->cour, -1);
								off->updateLocationInfor(rv->whichLocation->cour);
							}
							else{
								Route * re = &off->route[*nodeV->route->whichLocation->emptyRoutes.begin()];
								Route * rv = nodeV->pre->route;
								rv->depot->next = rv->endDepot;
								nodeV->pre = re->depot;
								re->depot->next = nodeV;
								re->endDepot->pre = split;
								split->next = re->endDepot;
								Node *V= nodeV;
								while(V != re->endDepot){
									V->route = re;
									V = V->next;
								}
								off->updateRouteInfor(rv->cour, -1);
								off->updateRouteInfor(re->cour, -1);
								off->updateLocationInfor(rv->whichLocation->cour);
							}
						}
						// for nodeU
						if (nodeU->next->route == NULL){
							Route * re = &off->route[*off->location[nodeU->next->cour-numV].emptyRoutes.begin()];
							nodeU->next = re->endDepot;
							re->depot->next = splitR;
							re->endDepot->pre = nodeU;
							splitR->pre = re->depot;
							Node *U= nodeU;
							while(U != re->depot){
								U->route = re;
								U = U->pre;
							}
							off->updateRouteInfor(re->cour, -1);
							off->updateLocationInfor(nodeU->next->cour);
						}
						else{
							if (nodeU->next->route->depot->next->isDepot){
								Route * ru = nodeU->next->route;
								ru->depot->next = splitR;
								splitR->pre = ru->depot;
								Node *U = nodeU;
								while(U!= ru->depot){
									U->route = ru;
									U = U->pre;
								}
								off->updateRouteInfor(ru->cour, -1);
								off->updateLocationInfor(ru->whichLocation->cour);
							}
							else{
								Route * re = &off->route[*nodeU->route->whichLocation->emptyRoutes.begin()];
								Route * ru = nodeV->pre->route;
								ru->depot->next = ru->endDepot;
								nodeU->next = re->endDepot;
								re->depot->next = splitR;
								re->endDepot->pre = nodeU;
								splitR->pre = re->depot;
								Node *U= nodeU;
								while(U != re->depot){
									U->route = re;
									U = U->pre;
								}
								off->updateRouteInfor(ru->cour, -1);
								off->updateRouteInfor(re->cour, -1);
								off->updateLocationInfor(ru->whichLocation->cour);
							}
						}
					}
				}
				*/
			}
		}
	}
}
void crossover::eliminateSubtours(){
	Node *nodeU,*nodeV,*begin,*p,*q;
	bool isFirst; int load;
	while((int)headSubstring.size() > 0){
		//first step to compute the total load
		double delta,min_delta;
		p = headSubstring[0];
		headSubstring.erase(headSubstring.begin());
		begin = p;
		load = 0;isFirst = true;
		while(p != begin || isFirst){
			load += p->dem;
			p = p->next;
			isFirst = false;
		}
		p = begin; isFirst = true;
		min_delta = Max; nodeU = NULL; nodeV = NULL;
		while(p != begin || isFirst){
			for (int i=0; i<(int)data->edgeNode[p->cour].size(); i++){
				if (data->edgeNode[p->cour][i] < numV){
					q = &off->client[data->edgeNode[p->cour][i]];
					if (q->route != NULL){
						if (q->route->whichLocation->remainInven + load < q->route->whichLocation->inventory){
							delta = data->D[p->cour][q->cour] + data->D[p->pre->cour][q->next->cour]
								- data->D[p->cour][p->pre->cour] - data->D[q->cour][q->next->cour];
							if (delta < min_delta){
								nodeU = p;
								nodeV = q;
								min_delta = delta;
							}
						}
					}
				}
				else{
					q = &off->location[data->edgeNode[p->cour][i]-numV].depots[off->location[data->edgeNode[p->cour][i]-numV].splitTime];
					if (q->route->whichLocation->remainInven + load < q->route->whichLocation->inventory){
						delta = data->D[p->cour][q->cour] + data->D[p->pre->cour][q->next->cour]
							- data->D[p->cour][p->pre->cour] - data->D[q->cour][q->next->cour];
						if (delta < min_delta){
							nodeU = p;
							nodeV = q;
							min_delta = delta;
						}
					}

				}
			}
			p = p->next;
			isFirst = false;
		}
		if (nodeU == NULL || nodeV == NULL){// remove the load feasibility
			isFirst = true; p = begin;
			while (p != begin || isFirst){
				for (int i=0; i<(int)data->edgeNode[p->cour].size(); i++){
					if (data->edgeNode[p->cour][i] < numV){
						q = &off->client[data->edgeNode[p->cour][i]];
						if (q->route != NULL){
							delta = data->D[p->cour][q->cour] + data->D[p->pre->cour][q->next->cour]
								- data->D[p->cour][p->pre->cour] - data->D[q->cour][q->next->cour];
							if (delta < min_delta){
								nodeU = p;
								nodeV = q;
								min_delta = delta;
							}
						}
					}
					else{
						q = &off->location[data->edgeNode[p->cour][i]-numV].depots[off->location[data->edgeNode[p->cour][i]-numV].splitTime];
						delta = data->D[p->cour][q->cour] + data->D[p->pre->cour][q->next->cour]
							- data->D[p->cour][p->pre->cour] - data->D[q->cour][q->next->cour];
						if (delta < min_delta){
							nodeU = p;
							nodeV = q;
							min_delta = delta;
						}
					}
				}
				p = p->next;
				isFirst = false;
			}
		}
		if (nodeU == NULL || nodeV == NULL){
			isFirst = true; p = begin;
			while (p != begin || isFirst){
				for (int i=0; i< numC; i++){
					if (i<numV){
						q = &off->client[i];
						if (q->route != NULL){
							delta = data->D[p->cour][q->cour] + data->D[p->pre->cour][q->next->cour]
								- data->D[p->cour][p->pre->cour] - data->D[q->cour][q->next->cour];
							if (delta < min_delta){
								nodeU = p;
								nodeV = q;
								min_delta = delta;
							}
						}
					}
					else{
						q = &off->location[i-numV].depots[off->location[i-numV].splitTime];
						delta = data->D[p->cour][q->cour] + data->D[p->pre->cour][q->next->cour]
							- data->D[p->cour][p->pre->cour] - data->D[q->cour][q->next->cour];
						if (delta < min_delta){
							nodeU = p;
							nodeV = q;
							min_delta = delta;
						}

					}
				}
				p = p->next;
				isFirst = false;
			}
		}
		if (nodeU == NULL || nodeV == NULL)std::cout<<"there are errors in eliminating subtours"<<std::endl;
		else{
			Node * nodeX, *nodeY;
			nodeU->route = nodeV->route;
			p = nodeU->next;
			while(p != nodeU){
				p->route = nodeV->route;
				p = p->next;
			}
			nodeX = nodeU->pre;
			nodeY = nodeV->next;
			//
			nodeV->next = nodeU;
			nodeU->pre = nodeV;
			nodeX->next = nodeY;
			nodeY->pre = nodeX;
		}
		off->updateRouteInfor(nodeV->route->cour, -1);
		off->updateLocationInfor(nodeV->route->whichLocation->cour);
	}
}
void crossover::adjustSolutions(){// to adjust some locations after crossover.
	for (int i=0;i<nbeSets;i++){
		off=subPop+i;




	}
}
bool crossover::gEAX(Individual *fa, Individual *ma, Individual *subPop, int &numSubPop){
	this->fa=fa;
	this->ma=ma;
	this->subPop=subPop;
	//
//	if (data->iteration == 76281){
//		fa->outputSolution();
//		ma->outputSolution();
//	}
//	ma->evaluationDis();
	loadFaCapMa();/*                                         STEP 1*/
	extractABcycles();/*                                       STEP 2*/
	if (nbABcycles == sizeSubPop)
		return false;
	generationEsets();/*                                        STEP 3*/
	generationIntermediateSolutions();/*                         STEPs 4 & 5*/
	//
//	adjustSolutions();
	//
	numSubPop=nbeSets;//return the number of offspring solutions
	return true;
}
