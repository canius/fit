//
//  runner.cpp
//  fit
//
//  Created by canius.chu on 27/2/2017.
//  Copyright © 2017 canius.chu. All rights reserved.
//

#include "runner.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>
#include "fit.h"

//static inline double generate_min(std::normal_distribution<> &d,std::mt19937 &gen,double min)
//{
//    double r;
//    do {
//        r = d(gen);
//    } while (r < min);
//    return r;
//}
//
//static inline double generate_max(std::normal_distribution<> &d,std::mt19937 &gen,double max)
//{
//    double r;
//    do {
//        r = d(gen);
//    } while (r > max);
//    return r;
//}

void run(int n)
{
    std::random_device rd;
    
    std::mt19937 gen1(rd());
    std::mt19937 gen2(rd());
    std::mt19937 gen3(rd());
    std::mt19937 gen4(rd());
    std::mt19937 gen5(rd());
    std::mt19937 gen6(rd());
    std::mt19937 gen7(rd());
    std::mt19937 gen8(rd());
    
    std::normal_distribution<> d1(95,5);
    std::normal_distribution<> d2(87.5,4);
    std::normal_distribution<> d3(65,3);
    std::normal_distribution<> d4(50,2);
    std::normal_distribution<> d5(15,1);
    std::normal_distribution<> d6(5,1);
    std::normal_distribution<> d7(0.5,0.1);
    std::normal_distribution<> d8(0.1,0.01);
    
    double x[8] = {1.491,1.837,2.217,2.505,2.813,3.216,3.748,4.22};
    double ey[8] = {0.01,0.01,0.01,0.01,0.01,0.01,0.01,0.01};
    double y[8];
    double outputX;
    
    for (int i = 0; i < n; ++i) {
        y[0] = d1(gen1);
        y[1] = d2(gen2);
        y[2] = d3(gen3);
        y[3] = d4(gen4);
        y[4] = d5(gen5);
        y[5] = d6(gen6);
        y[6] = d7(gen7);
        y[7] = d8(gen8);
        outputX = fit8092(x, y, ey, 8, 50.0);
        std::cout << outputX << std::endl << std::endl;
    }
}
