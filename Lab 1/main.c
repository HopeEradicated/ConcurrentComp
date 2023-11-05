#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include "timer.h"

#define DT 0.5
#define EPSILON 1e-6

typedef struct {
    double x, y;
} vector;

typedef struct Params {
    int start;
    int end;
} Params;

int bodies, timeSteps;
double *masses, GravConstant;
vector *positions, *velocities, *accelerations;

vector addVectors(vector a, vector b) {
    vector c = {a.x + b.x, a.y + b.y};
    return c;
}

vector scaleVector(double b, vector a) {
    vector c = {b * a.x, b * a.y}; 
    return c;
}

vector subtractVectors(vector a, vector b){
    vector c = {a.x - b.x, a.y - b.y};
    return c;
}

double mod(vector a) {
    return sqrt(a.x * a.x + a.y * a.y);
}

void initiateSystem(char *fileName) {
    FILE *fp = fopen(fileName, "r");
    fscanf(fp, "%lf%d%d", &GravConstant, &bodies, &timeSteps);

    masses = (double *)malloc(bodies * sizeof(double));
    positions = (vector *)malloc(bodies * sizeof(vector));
    velocities = (vector *)malloc(bodies * sizeof(vector));
    accelerations = (vector *)malloc(bodies * sizeof(vector));

    for (int i = 0; i < bodies; i++) {
        fscanf(fp, "%lf", &masses[i]);
        fscanf(fp, "%lf%lf", &positions[i].x, &positions[i].y);
        fscanf(fp, "%lf%lf", &velocities[i].x, &velocities[i].y);
    }

    fclose(fp);
}

void computeAccelerations(int start, int end) {
    for (int i = start; i < end; i++) {
        accelerations[i].x = 0;
        accelerations[i].y = 0;
        for (int j = 0; j < bodies; j++) {
	if (i != j) {
   	 double d = mod(subtractVectors(positions[j], positions[i]));
   	 accelerations[i] = addVectors(accelerations[i], scaleVector(GravConstant * masses[j] / ((d*d*d < EPSILON) ? EPSILON : d*d*d), subtractVectors(positions[j], positions[i])));
            	}
        }
    }
}

void computeVelocities(int start, int end) {
    for (int i = start; i < end; i++)
        velocities[i] = addVectors(velocities[i], scaleVector(DT, accelerations[i]));
}

void computePositions(int start, int end) {
    for (int i = start; i < end; i++)
        positions[i] = addVectors(positions[i], scaleVector(DT,velocities[i]));
}

void* simulate(void* arg) {
    Params* a =  (Params*)arg;
    computeAccelerations(a->start, a->end);
    computePositions(a->start, a->end);
    computeVelocities(a->start, a->end);
    return NULL;
}

int main(int argC, char *argV[]) {

    if (argC != 2) {
        printf("Not enougth arguments");
    } else {
         initiateSystem(argV[1]);
    }
    
    int nthreads = 2;
	printf("Enter number of threads: ");
    scanf("%d", &nthreads);
    pthread_t* threads = malloc(nthreads*sizeof(pthread_t));
    Params* params = malloc(nthreads*sizeof(Params));

    int chunksNum = bodies / nthreads;
    for (int i = 0; i < nthreads; i++) {
        params[i].start = chunksNum * i;
        params[i].end = chunksNum * (i + 1);
    }
    params[nthreads - 1].end = bodies;

    FILE *fpt;
    fpt = fopen("output.csv", "w+");

    fprintf(fpt, "t, ");
    for (int i = 0; i < bodies; i++) 
        fprintf(fpt, "x%d, y%d, ", i+1, i+1);
    fprintf(fpt, "\n");

    double start, end;
    GET_TIME(start);

        for (int i = 0; i < timeSteps; i++) {
            for (int j = 0; j < nthreads; ++j)
                pthread_create(&threads[j], NULL, simulate, &params[j]);

            for (int j = 0; j < nthreads; ++j)
                pthread_join(threads[j], NULL);

            fprintf(fpt, "%d, ", i);
            for (int j = 0; j < bodies; j++) 
                fprintf(fpt, "%lf, %lf, ", positions[j].x, positions[j].y);
            fprintf(fpt, "\n");
        }

    GET_TIME(end);
   
    printf("Programm worked for %lfseconds\n", end - start);

    free(threads);
    free(params);
    fclose(fpt);
    return 0;
}
