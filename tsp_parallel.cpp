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
#define CITYCOUNT 10
#define ITEMP 100			//Iterations will start at this temperature
#define FTEMP 0.000001		//Iterations will stop at this temperature
#define CTEMP 0.95			//Temperature will  decrease by this factor		
#define ITER_PER_TEMP 50  	//Number of iterations at each temperature
#define PROBABILITY 0.01 	//If probability is greater than this than solution will get accepted.
static int helper=0;		//makes sure that consequtive pair of random numbers
using namespace std;
struct city{
int name ; 
double x;
double y; 
};
/* Function to take Input */
void fileio(const string filename , struct city* country)
{
	std::ifstream fin(filename.c_str(), std::ifstream::in); 
	int temps;
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

/*MAIN FUNCTION*/
int main( int argc, char * argv[])
{
	bool flag =0;		//Used for signaling
    int rank;
    int size;
    struct city country[CITYCOUNT];
    MPI_Datatype mystruct;
    int          blocklens[3];
    MPI_Aint     indices[3];
    MPI_Datatype old_types[3];

    MPI_Init( &argc, &argv );

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    blocklens[0] = 1;
    blocklens[1] = 1;
    blocklens[2] = 1;
    old_types[0] = MPI_INT;
    old_types[1] = MPI_DOUBLE;
    old_types[2] = MPI_DOUBLE;
    if (rank == 0)
    fileio("test10.txt",country);
    /*Master keeping track of time*/
    clock_t starttime;
    clock_t endtime;
    if(rank==0)
    starttime=clock();
    
	float temperature=ITEMP;			 //Initial temperature is equal to start temperature
while(temperature>FTEMP)
{
	for( int i=0 ; i< CITYCOUNT ; i++)
		{
    		MPI_Address( &country[i].name, &indices[0] );
    		MPI_Address( &country[i].x, &indices[1] );
    		MPI_Address( &country[i].y, &indices[2] );
    		indices[2] = indices[2] - indices[0];
    		indices[1] = indices[1] - indices[0];
    		indices[0] = 0;
		    MPI_Type_struct( 3, blocklens, indices, old_types, &mystruct );
    		MPI_Type_commit(&mystruct);
			MPI_Bcast( &country[i], 1, mystruct, 0, MPI_COMM_WORLD );
   		}

   	/*Master waiting for the Minimum distance for next temperature*/
   	if(rank==0)
   	{
   	  double minimum;
   	  MPI_Status status;
   	  int id=1;
   	  struct city temp[size-1][CITYCOUNT];
   	  MPI_Recv(&minimum,1,MPI_DOUBLE,1,1,MPI_COMM_WORLD,&status);
   	  for( int i=2;i<size-2;i++)
   	  { 
   	   double newmin;
   	   MPI_Recv(&newmin,1,MPI_DOUBLE,i,1,MPI_COMM_WORLD,&status);
   	   if(newmin<minimum) {minimum=newmin; id=i;}
   	  }
   	  
   	  for( int i=1;i<size-1;i++)
   	  { 
   	   MPI_Recv(temp[i-1],CITYCOUNT,mystruct,i,2,MPI_COMM_WORLD,&status);
   	  }
   		for ( int i=0 ; i<CITYCOUNT;i++)
   		country[i]=temp[id-1][i];
   		
   		/*Master Displaying the results*/
   		cout<<"Minimum Distance : "<<minimum<<endl;
   		for ( int i=0 ; i<CITYCOUNT;i++)
   		cout<<country[i].name<<"-->";
   		cout<<country[0].name<<endl;
   	}
   	
   /*~~~~~~~~~~~~~~~~~~~DATA HAS BEEN DISTRIBUTED TO ALL THE WORKERS~~~~~~~~~~~~~~~~~~~*/
   if(rank!=0)
   {
   
   /*Initial state of all worker processes*/
   struct city initialorder[CITYCOUNT]; //Used to store the initial tour route
   struct city finalorder[CITYCOUNT]; //Used to store the final tour route
   for(int k=0 ; k<CITYCOUNT ; k++)
   {initialorder[k]=country[k];finalorder[k]=country[k];}
   double initialres;
   for(int l=0;l<ITER_PER_TEMP;l++)
   {
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
 /*If solution is poor than initial solution and also has very low probabitlity 
   of acceptance.Then We revert back to previous solution*/
  		else {
  			for ( int i=0 ; i< CITYCOUNT;i++)
 	 		{finalorder[i]=initialorder[i];}
  			 }
 		}
   } // for loop for total iterations at each time step
   MPI_Send(&initialres,1,MPI_DOUBLE,0,1,MPI_COMM_WORLD);
   MPI_Send(initialorder,CITYCOUNT,mystruct,0,2,MPI_COMM_WORLD);
   }
   MPI_Barrier(MPI_COMM_WORLD);
   temperature=CTEMP*temperature;
  }/*Reduce the temperature and repeat the process*/  
  if(rank==0)
  {endtime=clock(); cout<<"\nTime Taken :"<<double(endtime-starttime)/CLOCKS_PER_SEC<<endl;}
    /* Clean up the type */
    MPI_Type_free( &mystruct );
    MPI_Finalize( );
    return 0;
}




