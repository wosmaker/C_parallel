#include <stdio.h>
#include <stdlib.h>

#define Nrow 10
#define Ncol 10
#define MatrixA "matrixA.txt"
#define MatrixB "matrixB.txt"

void writeToFile(char *path,int row,int col){
    int i,j;
    FILE *fp = fopen(path,"w");
    for(i=0;i<row;i++){
        for(j=0;j<col;j++){
            fprintf(fp,"%.1f ",(float)i+j);
        }
        fputs("\n",fp);
    }
    fclose(fp);
}

int main(){
    writeToFile(MatrixA,Nrow,Ncol);
    writeToFile(MatrixB,Nrow,Ncol);
    return 0;
}