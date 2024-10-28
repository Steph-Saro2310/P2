#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "pav_analysis.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void hamming(float *w, unsigned int N){
    for (unsigned int n = 0; n<N; n++){
        w[n] = 0.54 - 0.46 * cos(2 * M_PI * n / (N - 1));
    }
}


float compute_power(const float *x, unsigned int N) {
    float pot = 1e-12;
    float suma = 0.0;
    float w[N];
    hamming(w, N);

    
    for(unsigned int n = 0; n<N; n++){
        pot += x[n] * w[n] * x[n] * w[n];
        suma += w[n] * w[n];
    }

    return 10*log10(pot/suma);
}

float compute_am(const float *x, unsigned int N) {
    float amp = 1e-12;
    
    for(unsigned int n = 0; n<N; n++){
        amp += fabs(x[n]);
    }

    return amp/N;
}

float compute_zcr(const float *x, unsigned int N, float fm) {
    unsigned int zcr = 1e-12;
    
    for (unsigned int n = 1; n < N; n++) {
        if ((x[n] > 0 && x[n-1] < 0) || (x[n] < 0 && x[n-1] > 0)) {
            zcr++;
        }
    }
    return (fm/2)*(zcr/N-1);
}
