#include <mpi.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>

#define Nrow 5000
#define Ncol 5000

#define MatrixA "matAlarge.txt"
#define MatrixB "matBlarge.txt"
#define MatrixOut "matBlockOut.txt"

float** malloc2d(int row,int col){
    float *data = malloc(col * row * sizeof(float));
    float **arr = malloc(row * row * sizeof(float));
    int i;
    for (i=0;i<row;i++)
        arr[i] = &(data[col*i]);
    return arr;
}   

void initMatrix(float **matrix, int row , int col){  
    int i,j;
    for(i = 0; i < row;i++){
        for(j=0;j<col;j++){
            matrix[i][j] = i+j;
        }
    }
}

void writeToFile(float **InMatrix,char *path,int row,int col){
    int i,j;
    FILE *fp = fopen(path,"w");
    fprintf(fp,"%d %d\n",row,col);
    for(i=0;i<row;i++){
        for(j=0;j<col;j++){
            fprintf(fp,"%.1f ",InMatrix[i][j]);
        }
        fputs("\n",fp);
    }
    fclose(fp);
}

void readToMatrix(float **matrix, char *path){
    int i,j;
    int row,col;
    FILE *fp = fopen(path,"r");
    fscanf(fp,"%d %d",&row,&col);
    for(i=0;i<row;i++){
        for(j=0;j<col;j++){
            fscanf(fp,"%f",&matrix[i][j]);
        }
    }
    fclose(fp);
}

void checkMatrixSize( char *path,int *row,int *col){
    FILE *fp = fopen(path,"r");
    fscanf(fp,"%d %d",row,col);
    fclose(fp);
}

float* plusMatrix(float *matrix1, float *matrix2 , float blockSize){
    float *result = (float*)malloc(blockSize * sizeof(float));
    int i;
    for(i = 0; i<blockSize; i++){
        result[i] = matrix1[i] + matrix2[i];
    }
    return result;
}

float main(int argc, char *argv[]){
    // mpi rank and size variable
    int p,id;
    int i;
    int row,col;
    MPI_Status status;
    MPI_Request req[2];
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    double startTime, endTime;

    if (id == 0){ // master
        int row,col;

        checkMatrixSize(MatrixA,&row,&col);
        // printf("Start MPI... Master Node \n");
        float **matrix1 = malloc2d(row,col);
        float **matrix2 = malloc2d(row,col);
        float **result =  malloc2d(row,col);
        // Load data from file to matrix     
        printf("Reading from file\n");
        readToMatrix(matrix1,MatrixA);
        readToMatrix(matrix2,MatrixB);
        printf("Complete reading\n");

        // // test Load using generate data
        // row = Nrow;
        // col = Ncol;
        // initMatrix(matrix1,Nrow,Ncol);
        // initMatrix(matrix2,Nrow,Ncol);

        int blockSize = (row * col)/(p);
        int SizeWithRemainder = blockSize + ((row * col) % blockSize);

        startTime = MPI_Wtime();
        // send data to anathor node 
        for (i = 1; i < p; i++){
            printf("Send to Node %d\n",i);
            int Size = i == p-1 ? SizeWithRemainder : blockSize;
            MPI_Send(&Size, 1 , MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&matrix1[0][0] + blockSize*i, Size , MPI_FLOAT, i, 1, MPI_COMM_WORLD);
            MPI_Send(&matrix2[0][0] + blockSize*i, Size , MPI_FLOAT, i, 2, MPI_COMM_WORLD);
        }

        // calculate
        memcpy(&result[0][0],plusMatrix(&matrix1[0][0],&matrix2[0][0],blockSize),sizeof(float)*blockSize);
        for (i=1; i<p ;i++){
            int Size = i == p-1 ? SizeWithRemainder : blockSize;
            MPI_Recv(&result[0][0]+blockSize*i,Size,MPI_FLOAT,i,0,MPI_COMM_WORLD, &status);     
        }

        endTime = MPI_Wtime();
        printf("End >> Time usage: %lf\n",endTime - startTime);
        printf("Waiting for write to file\n");
        writeToFile(result,MatrixOut,Nrow,Ncol);
    }
    else {
        int Size = 0;
        printf("start received Size\n");
        MPI_Recv(&Size,1,MPI_INT, 0,0,MPI_COMM_WORLD, &status);
        float *matrix1 = (float*)malloc(Size * sizeof(float));
        float *matrix2 = (float*)malloc(Size * sizeof(float));
        float *result = (float*)malloc(Size * sizeof(float));
        printf("start receive\n");
        MPI_Recv(matrix1,Size,MPI_FLOAT, 0,1,MPI_COMM_WORLD, &status);
        MPI_Recv(matrix2,Size,MPI_FLOAT , 0,2,MPI_COMM_WORLD, &status);

        MPI_Send( plusMatrix(matrix1,matrix2,Size),Size,MPI_FLOAT,0,0,MPI_COMM_WORLD);
        printf("complete send back\n");
    }
    MPI_Finalize();
}

