#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>

struct timeval stop, start;
char* fileName[3];
int** a; int** b; int** c;
int aRows, aCols, bRows, bCols, cRows, cCols;

void writeMatrixToFile(char* name) {
    char* line = malloc(sizeof(char) * 50);
    if (line == NULL) {
        printf("Error: Failed to allocate memory.\n");
        return;
    }
    strcpy(line, fileName[2]);
    strcat(line, name);
    strcat(line, ".txt");
    FILE* outputFile = fopen(line, "w");

    if(name == "_per_matrix"){
        fprintf(outputFile, "Method: A thread per matrix\n");
    }
    else if(name == "_per_row"){
        fprintf(outputFile, "Method: A thread per row\n");
    }
    else if(name == "_per_element"){
        fprintf(outputFile, "Method: A thread per element\n");
    }
    fprintf(outputFile, "row=%d col=%d\n", cRows, cCols);
    for (int i = 0; i < cRows; i++) {
        for (int j = 0; j < cCols; j++) {
            fprintf(outputFile, "%d ", c[i][j]);
        }
        fprintf(outputFile, "\n");
    }

    free(line);
    fclose(outputFile);
}

int** parse(char* name, int* rows, int* cols){
    char* line = malloc(sizeof(char) * 50);
    if (line == NULL) {
        printf("Error: Failed to allocate memory.\n");
        return NULL;
    }
    strcpy(line, name);
    strcat(line, ".txt");

    int row, col;
    FILE* fp = fopen(line, "r");
    if(fp == NULL){
        printf("Error opening file %s\n", line);
        return NULL;
    }
    fscanf(fp, "row=%d col=%d", &row, &col);
    *rows = row; *cols = col;

    // Allocate memory for the 2D array
    int** myArray = (int**) malloc(sizeof(int*) * row);
    for (int i = 0; i < row; i++) {
        myArray[i] = (int*) malloc(sizeof(int) * col);
    }

    // Initialize the array with some values
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if(fscanf(fp, "%d", &myArray[i][j]) != 1){
                printf("Error reading file\n");
                return NULL;
            }
        }
    }

    // Free the memory
    free(line);
    fclose(fp);
    return myArray;
}

int** multiplyPerMatrix(){
    cRows = aRows; cCols = bCols;

    int** myArray = (int**) malloc(sizeof(int*) * cRows);
    for (int i = 0; i < cRows; i++) {
        myArray[i] = (int*) malloc(sizeof(int) * cCols);
    }

    for(int i=0; i<cRows; i++) {
        for(int j=0; j<cCols; j++) {
            myArray[i][j] = 0;
            for(int k=0; k<aCols; k++) {
                myArray[i][j] += a[i][k] * b[k][j];
            }
        }
    }

    return myArray;
}

void* calcRow(void *args){
    int* rowNum = (int*)args;
    for(int i = 0; i < cCols; i++){
        c[*rowNum][i] = 0;
        for(int j = 0; j < aCols; j++){
            c[*rowNum][i] += a[*rowNum][j] * b[j][i];
        }
    }
    pthread_exit(0);
}

void multiplyPerRow(){
    pthread_t threads[cRows];
    int thread_args[cRows];
    for (int i = 0; i < cRows; i++) {
        thread_args[i] = i;
        pthread_create(&threads[i], NULL, calcRow, &thread_args[i]);
    }
    for (int i = 0; i < cRows; i++) {
        pthread_join(threads[i], NULL);
    }
}

void* calcElement(void *args){
    int* totalNum = (int*)args;
    int rowNum = (*totalNum) / bCols;
    int colNum = (*totalNum) % bCols;
    c[rowNum][colNum] = 0;
    for(int i = 0; i < aCols; i++){
        c[rowNum][colNum] += a[rowNum][i] * b[i][colNum];
    }
    pthread_exit(0);
}

void multiplyPerElement(){
    int n = cRows * cCols;
    pthread_t threads[n];
    int thread_args[n];
    for(int i = 0; i < n; i++){
        thread_args[i] = i;
        pthread_create(&threads[i], NULL, calcElement, &thread_args[i]);
    }
    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main(int argc, char* argv[]) {

    if((argc != 1) && (argc != 4)){
        printf("wrong arguments\n");
        return 0;
    }

    fileName[0] = "a", fileName[1] = "b", fileName[2] = "c";
    for (int i = 1; i < argc; i++) {
        fileName[i-1] = argv[i];
    }

    a = parse(fileName[0], &aRows, &aCols);
    b = parse(fileName[1], &bRows, &bCols);
    if((a == NULL) || (b == NULL)){
        printf("Error in matrices\n");
        return 0;
    }

    //start checking time
    gettimeofday(&start, NULL);
    //only one thread used
    c = multiplyPerMatrix(); writeMatrixToFile("_per_matrix");
    //end checking time
    gettimeofday(&stop, NULL);
    printf("Per matrix method:\nNumber of threads = 1\n");
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n\n", stop.tv_usec - start.tv_usec);

    //start checking time
    gettimeofday(&start, NULL);
    //thread for every row
    multiplyPerRow(); writeMatrixToFile("_per_row");
    //end checking time
    gettimeofday(&stop, NULL);
    printf("Per row method:\nNumber of threads = %d\n", cRows);
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n\n", stop.tv_usec - start.tv_usec);

    //start checking time
    gettimeofday(&start, NULL);
    //thread for every element
    multiplyPerElement(); writeMatrixToFile("_per_element");
    //end checking time
    gettimeofday(&stop, NULL);
    printf("Per element method:\nNumber of threads = %d\n", cRows * cCols);
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n\n", stop.tv_usec - start.tv_usec);

    return 0;
}
