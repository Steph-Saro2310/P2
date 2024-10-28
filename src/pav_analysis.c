#include <math.h>
#include "pav_analysis.h"

float compute_power(const float *x, unsigned int N) {
    float pot=1e-12;

    for(unsigned int i=0;i<N;i++){
        pot+=x[i]*x[i];
    }
    return 10*log10(pot/N);
}

float compute_am(const float *x, unsigned int N) {
    float amp=0;

    for(unsigned int i=0;i<N;i++){
        amp+=fabs(x[i]);
    }
    return amp/N;
}

float compute_zcr(const float *x, unsigned int N, float fm) {
    float ceros=0;

    for(unsigned int i=1;i<N;i++){
        if((x[i]<0 && x[i-1]>0)||(x[i]>0 && x[i-1]<0)){
            ceros++;
        }
    }

    return (fm/2)*(1/(N-1))*ceros;
}
