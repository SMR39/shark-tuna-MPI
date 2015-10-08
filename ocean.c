
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "ocean.h"
#include <mpi.h>

/* constants for the ocean */
#define N 40
#define M 20
#define WALL 100
#define STEP 15
#define RATIO 10

int main (int argc, char **argv)
{
    int i,j;int my_rank = 0;
    int rank,size;
    int *ns_north,*nt_north,*ns_south,*nt_south ;
    
    int totalfishnorth[2];
    int totalfishsouth[2];
    fish_t *ocean = (fish_t *)malloc(N*M*sizeof(fish_t));
    fish_t *oceanRec = (fish_t *)malloc(N*M*sizeof(fish_t));
    fish_t *oceanRecFinal = (fish_t *)malloc(N*M*sizeof(fish_t));
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    
    MPI_Datatype MPI_FISH; //
    int n_items = 2;  //
    MPI_Datatype fish_types[2]= { MPI_CHAR, MPI_CHAR }; //
    int fish_lengths[2] = {1,1}; //
    MPI_Aint fish_offsets[2];  //
    fish_offsets[0] = 0;//
    fish_offsets[1] = 1;//
    
    
    MPI_Type_create_struct(n_items, fish_lengths,fish_offsets, fish_types, &MPI_FISH); //
    MPI_Type_commit(&MPI_FISH); //
    
    init_ocean(ocean, N, M, RATIO);
    if(rank==0)
    display_ocean(ocean, N, M);
    MPI_Scatter(ocean,N*M/4, MPI_FISH, oceanRec,N*M/4, MPI_FISH, 0, MPI_COMM_WORLD);
    printf(CLS "\n");
    printf("Rank is %d\n",rank);
    display_ocean(oceanRec, N/4, M);
    
    for (i = 0; i < WALL; i++) {
        //printf("Rank is %d \n",rank);
        usleep(STEP);
       // printf(CLS "\n");
        update_ocean_part(oceanRec,N/4,M,ns_north,nt_north,ns_south,nt_south);
        totalfishnorth[0] = (int)ns_north; totalfishnorth[1] = (int)nt_north;
        totalfishsouth[0] = (int)ns_south; totalfishsouth[1] = (int)nt_south;
        MPI_Status status;//printf("//////////i is %d\n",i);
        
        if(rank==0)
        {
            MPI_Send(totalfishnorth,4,MPI_INT,3,0,MPI_COMM_WORLD);
            MPI_Send(totalfishsouth,4,MPI_INT,1,0,MPI_COMM_WORLD);
            MPI_Recv(totalfishsouth,4,MPI_INT,3,0,MPI_COMM_WORLD,&status);
            MPI_Recv(totalfishnorth,4,MPI_INT,1,0,MPI_COMM_WORLD,&status);
	    inject_ocean(ocean, N/4, M, totalfishnorth[0]+totalfishsouth[0], totalfishnorth[1]+totalfishsouth[1]);
        }
        else if(rank==3)
        {
            MPI_Send(totalfishsouth,4,MPI_INT,0,0,MPI_COMM_WORLD);
            MPI_Send(totalfishnorth,4,MPI_INT,2,0,MPI_COMM_WORLD);
            MPI_Recv(totalfishsouth,4,MPI_INT,2,0,MPI_COMM_WORLD,&status);
            MPI_Recv(totalfishnorth,4,MPI_INT,0,0,MPI_COMM_WORLD,&status);
	    inject_ocean(ocean, N/4, M, totalfishnorth[0]+totalfishsouth[0], totalfishnorth[1]+totalfishsouth[1]);
        }
        else if(rank==1 || rank==2)
        {   my_rank =rank;
            MPI_Send(totalfishsouth,4,MPI_INT,my_rank+1,0,MPI_COMM_WORLD);
            MPI_Send(totalfishnorth,4,MPI_INT,my_rank-1,0,MPI_COMM_WORLD);
            MPI_Recv(totalfishnorth,4,MPI_INT,my_rank-1,0,MPI_COMM_WORLD,&status);
            MPI_Recv(totalfishsouth,4,MPI_INT,my_rank+1,0,MPI_COMM_WORLD,&status);
	    inject_ocean(ocean, N/4, M, totalfishnorth[0]+totalfishsouth[0], totalfishnorth[1]+totalfishsouth[1]);
        }
 	
        
    }
    display_ocean(oceanRec, N/4, M);
    MPI_Gather(oceanRec, N*M/4, MPI_INT,oceanRecFinal, N*M/4, MPI_INT, 0, MPI_COMM_WORLD); 
    if(!my_rank)
    //display_ocean(oceanRecFinal, N, M);
    MPI_Finalize();
    return 0;
} /* main */

