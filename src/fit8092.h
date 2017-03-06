//
//  fit8092.h
//  fit
//
//  Created by canius.chu on 27/2/2017.
//  Copyright © 2017 canius.chu. All rights reserved.
//

#ifndef fit8092_h
#define fit8092_h

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct data8092_struct {
	double *x;
	double *y;
	double *ey;
	int n;
	double *p;
	int np;
	double output;
};

typedef struct data8092_struct data8092;

double fit8092(data8092 *data, double inputY);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* fit8092_h */
