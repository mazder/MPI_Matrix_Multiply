#include<stdio.h>
#include<mpi.h>
#include<stdlib.h>

int main(int argc, char* argv[])
{
    // Local variables for every process
    int rank, size; // process id and no. of processes 
    int i,j,q,remain, stotal=0, rtotal=0, C;

    int k=4,m=7,n=3; // dimensions
    int *mA; // Matrix pointers to scatter mA[m,k]
    int *mB; // Matrix pointers to broadcast mB[k,n]
    int *mC; // Matrix pointers to receive scatterred 
    
    // for scatter, different processes receive different sizes 
    int *sendcounts, *receivecounts, *displs, *rdispls, revcounts;
    int *rbuff;

    // Every process calculate
    int *cC;    

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    //printf("Id: %d,  size=%d\n", rank, size);
    //MPI_Barrier(MPI_COMM_WORLD);

    // allocate memory 
    if(rank==0){
        mA=malloc((m*k)*sizeof(int));
        mC=malloc((m*n)*sizeof(int));
    }
    mB=malloc((k*n)*sizeof(int));
    
    // just for initialize matrices
    if(rank==0){
        // initialize mA and mB
        for(i=0; i<(m*k); i++){
            mA[i]=i+1;
        }
        for(i=0; i<(k*n); i++){
            mB[i]=i+1;
        }
        // print mA and mB
        printf("print mA=========\n");
        for(i=0;i<m; i++){
            for(j=0; j<k; j++){
                printf("%d ",mA[i*k+j]);
            }
            printf("\n");
        }
        printf("print mB=========\n");
        for(i=0;i<k; i++){
            for(j=0; j<n; j++){
                printf("%d ",mB[i*n+j]);
            }
            printf("\n");
        }
    }

    sendcounts = malloc(sizeof(int)*size);
    receivecounts = malloc(sizeof(int)*size);
    displs = malloc(sizeof(int)*size);
    rdispls = malloc(sizeof(int)*size);
    remain = (m)%size;
    for(i=0; i<size; i++){
        sendcounts[i]=m/size * k;
        if(remain>0){
            sendcounts[i]=sendcounts[i]+k;
            remain--;
        }
        displs[i]=stotal;
        stotal=stotal+sendcounts[i];
        receivecounts[i]= (sendcounts[i]/k)*n;
        rdispls[i]=rtotal;
        rtotal = rtotal+receivecounts[i];
    }
    // to receive scatter
    rbuff = malloc(sendcounts[rank]*sizeof(int));
    // to calculate
    cC = malloc((sendcounts[rank]/k)*n*sizeof(int));

    MPI_Scatterv(mA, sendcounts, displs, MPI_INT, rbuff, sendcounts[rank], MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(mB, k*n, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    // calculate
    for ( i = 0; i < sendcounts[rank]/k; i++) {
        for(j=0; j<n; j++){
            //printf("%d ", rbuff[i*k+j]);
            C=0;
            for(q=0;q<k; q++){
                //printf("%d %d <>",rbuff[i*k+q], mB[q*n+j]);
                C+=rbuff[i*k+q]*mB[q*n+j];
            }
            //printf("%d ",C);
            cC[i*n+j]=C;    
        }
       // printf("\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // gather all result data from all prcesses
    MPI_Gatherv( cC ,receivecounts[rank], MPI_INT, mC , receivecounts, rdispls, MPI_INT , 0, MPI_COMM_WORLD ) ;

    if(rank==0){
        printf("print mC=========\n");
        for(i=0;i<m; i++){
            for(j=0; j<n; j++){
                printf("%d ",mC[i*n+j]);
            }
            printf("\n");
        }
    }

    MPI_Finalize();
    return 0;
}
