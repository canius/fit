//
//  fit.h
//  fit
//
//  Created by canius.chu on 27/2/2017.
//  Copyright © 2017 canius.chu. All rights reserved.
//

#ifndef fit_hpp
#define fit_hpp

#include <stdio.h>
#include "mpfit.h"

#ifdef __cplusplus
extern "C" {
#endif

double calculate8092(double x,double a,double b,double c,double d,double e);
double calculate8092reverse(double y,double a,double b,double c,double d,double e);
double fit8092(double *x,double *y,double *ey,int n,double inputY);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* fit_hpp */
