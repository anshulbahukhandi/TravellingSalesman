/* This code solves the traditional "Travelling salesman problem" using MPI*/
/* Cost of the tour in this case is the total tour distance*/
/* Tour forms a closed loop. means salesman finishes where he starts*/
/* 2-opt swap is used to do a slight disturbance from initial order*/
#include<iostream>
#include<mpi.h>
#include<fstream>
#include<ctime>
#include<iomanip>
#include<cstdlib>
#include<cstring>
#include<cmath>
#include<random>
using namespace std; 
#define CITYCOUNT 50
#define ITEMP 100			//Iterations will start at this temperature
#define FTEMP 0.000001		//Iterations will stop at this temperature
#define CTEMP 0.95			//Temperature will  decrease by this factor		
#define ITER_PER_TEMP 50  	//Number of iterations at each temperature
#define PROBABILITY 0.01 	//If probability is greater than this than solution
static int helper=0;
/*Data structure to save the details of a city*/
struct city{
string name; 
double x; 
double y;
};

/* Function to take Input */
void fileio(const string filename , struct city* country)
{
	std::ifstream fin(filename.c_str(), std::ifstream::in); 
	string temps;
	double tempx;
	double tempy;
	for(int i=0 ; i<CITYCOUNT;i++)
	{
	fin>>temps>>tempx>>tempy;
	country[i].name=temps;
	country[i].x=tempx;
	country[i].y=tempy;
	}
}
/*Function to return the distance between any two cities*/
double distance( struct city* a  , struct city* b)
{
	double res;
	res=sqrt( (b->x-a->x)*(b->x-a->x) + (b->y-a->y)*(b->y-a->y) );
	return res;
} 
/*Function to calculate the cost of the tour*/
double cost(struct city* tour)
{
  double res=0;
  for ( int i=0; i<CITYCOUNT-1 ; i++)
  {
  	res+=distance(tour+i,tour+i+1);
  }
  res=res+distance(tour , tour+CITYCOUNT-1);
  return res;
}
/*Data structure for 2-opt swap*/
struct rand2{
int num1; 
int num2;
};
/* Random number generator (Number range is from 0 to CITYCOUNT-1)*/
struct rand2 generate(){
helper++;
struct rand2 random2number;
srand(helper+time(NULL));
random2number.num1=rand()%CITYCOUNT ;
random2number.num2=rand()%CITYCOUNT;
return random2number;
}
/* Function to do one swap to the initial order */
void swapcity(struct city * order)
{
 int count =0 ; 
 struct rand2 temprand ; 
 temprand=generate();
/*If the indexes to swap comes out to be same , swap function is called again*/
 while(count<1)
 {
 if(temprand.num1==temprand.num2)
    {swapcity(order); count++;}
 else
  {
  struct city tempcity;
  tempcity=order[temprand.num1];
  order[temprand.num1]=order[temprand.num2];
  order[temprand.num2]=tempcity;
  break;
  }
 }
}

/*Main Function*/
int main(int argc , char * argv[])
{	
struct city country[CITYCOUNT];		 //Used to store the input
string filename="test50.txt";
fileio(filename, country);
/* Initial tour route is assumed to be same as the order of the input*/
struct city initialorder[CITYCOUNT]; //Used to store the initial tour route
struct city finalorder[CITYCOUNT]; //Used to store the final tour route			
for(int k=0 ; k<CITYCOUNT ; k++)
{initialorder[k]=country[k];finalorder[k]=country[k];}
float temperature=ITEMP;			 //Initial temperature is equal to start temperature
double initialres;
clock_t start=clock();
/* KEEP LOOPING TILL THE SYTEM IS FROZEN*/
while(temperature>=FTEMP)
{
for( int j=0 ;j<ITER_PER_TEMP;j++)
{
for(int i=0 ; i<CITYCOUNT ; i++)
{cout<<initialorder[i].name<<"  "<<initialorder[i].x<<"  "<<initialorder[i].y<<"\n";}
initialres=cost(initialorder);
swapcity(finalorder);
double finalres=cost(finalorder);
/*Accepting if the solution has lower cost*/
if(finalres<=initialres)
 {
  initialres=finalres;
  for ( int i=0 ; i< CITYCOUNT;i++)
  {initialorder[i]=finalorder[i];}
 }
/*Not rejecting solution straight away , instead calculating its probability*/
else
 {
  float probab = exp(-(finalres-initialres)/temperature) ;
  if(probab>PROBABILITY)
  {
  	initialres=finalres;
 	for ( int i=0 ; i< CITYCOUNT;i++)
 	 {initialorder[i]=finalorder[i];}
  }
  /*If solution is prro than initial solution and also has very low probabitlity 
    of acceptance.Then We revert back to previous solution*/
  else {
  for ( int i=0 ; i< CITYCOUNT;i++)
 	 {finalorder[i]=initialorder[i];}
  }
 }
}	//one temperature step  complete
cout<<"Result is now: \n";
for(int i=0 ; i<CITYCOUNT ; i++)
cout<<initialorder[i].name<<"-->";
cout<<initialorder[0].name<<endl;
cout<<"Total distance is :"<<initialres<<endl;
 
temperature=temperature*CTEMP;
}
clock_t end=clock();
cout<<"Total time take :"<<double(end-start)/CLOCKS_PER_SEC<<endl;
return 0;
}


























