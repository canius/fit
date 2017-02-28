//
//  fit.c
//  fit
//
//  Created by canius.chu on 27/2/2017.
//  Copyright © 2017 canius.chu. All rights reserved.
//

#include "fit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mpfit.h"

#define PRINT 0

struct vars_struct {
    double *x;
    double *y;
    double *ey;
};

inline double calculate8092(double x,double a,double b,double c,double d,double e)
{
    return a + b * (1 - pow((1 + exp((x + d*log(pow(2, 1/e)-1) - c)/d)),-e));
}

inline double calculate8092reverse(double y,double a,double b,double c,double d,double e)
{
    return c + d * (log(pow(b / (a + b - y), 1/e) - 1) - log(pow(2, 1/e)-1));
}

int func8092(int m, int n, double *p, double *dy, double **dvec, void *vars)
{
    int i;
    struct vars_struct *v = (struct vars_struct *) vars;
    double *x, *y, *ey, f;
    
    x = v->x;
    y = v->y;
    ey = v->ey;
    
    for (i=0; i<m; i++) {
        f = calculate8092(x[i], p[0], p[1], p[2], p[3], p[4]);
        dy[i] = (y[i] - f)/ey[i];
    }

    return 0;
}

double fit8092(double *x,double *y,double *ey,int n,double inputY)
{
    double p[5] = {1.0, 1.0, 1.0, 1.0, 0.1};
    double perror[5];
    struct vars_struct v;
    int status;
    mp_result result;
    
    memset(&result,0,sizeof(result));
    result.xerror = perror;
    
    v.x = x;
    v.y = y;
    v.ey = ey;
    
    status = mpfit(func8092, n, 5, p, 0, 0, (void *) &v, &result);

#if PRINT
    for (int i = 0; i < result.npar; i++) {
        printf("P[%d] = %f +/- %f\n",i, p[i], result.xerror[i]);
    }
    printf("\n");
#endif
    
    return calculate8092reverse(inputY,p[0],p[1],p[2],p[3],p[4]);
}

