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
#include <algorithm>
#include <thread>
#include "fit8092.h"

using namespace std;

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

double **generate(int n)
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
    
    double **data = new double*[n];
    for (int i = 0; i < n; ++i) {
        double *y = new double[8];
        y[0] = d1(gen1);
        y[1] = d2(gen2);
        y[2] = d3(gen3);
        y[3] = d4(gen4);
        y[4] = d5(gen5);
        y[5] = d6(gen6);
        y[6] = d7(gen7);
        y[7] = d8(gen8);
        data[i] = y;
    }
    return  data;
}

void thread_call(int tid,double *x,double *ey,double **p,double *r,int n)
{
    int i = 0;
    do {
        double *y = *p;
        *r = fit8092(x, y, ey, 8, 50.0);
        delete y;
        p++;
        r++;
    } while (++i < n);
}

void calculate(int n,double *result)
{
    double x[8] = {1.491,1.837,2.217,2.505,2.813,3.216,3.748,4.22};
    double ey[8] = {0.01,0.01,0.01,0.01,0.01,0.01,0.01,0.01};
    
    cout << "正在生成" << n << "组正态分布数据Y..." << endl;
    
    double **yset = generate(n);

    cout << "拟合8092曲线..." << endl;
    
    const int num_threads = 8;
    std::thread threads[num_threads];
    
    int sub_num = n / num_threads;
    int last_num = n - (num_threads - 1) * sub_num;
    double **p = yset;
    double *r = result;
    
    for (int i = 0; i < num_threads; ++i) {
        threads[i] = std::thread(thread_call,i,x,ey,p,r,sub_num);
        int step = i == num_threads - 1 ? last_num : sub_num;
        p += step;
        r += step;
    }
    
    for (int i = 0; i < num_threads; ++i) {
        threads[i].join();
    }

    delete [] yset;
}

void calculate_mode(double *x,int n,vector<double> &mode,double precision)
{
    vector<vector<double>> modes;
    double last = x[0];
    vector<double> *current = nullptr;
    
    for (int i = 1; i < n; ++i) {
        double value = x[i];
        if (abs(last - value) < precision) {
            if (current == nullptr) {
                vector<double> b = {last,value};
                modes.push_back(b);
                current = &modes.back();
            }
            else {
                current->push_back(value);
            }
        }
        else {
            current = nullptr;
            last = value;
        }
    }
    auto r = max_element(modes.begin(), modes.end(), [](const vector<double>& a, const vector<double>& b) {
        return a.size() < b.size();
    });
    mode = *r;
}

void statistics(double *x,int n)
{
    double min = x[0];
    double x25 = x[(int)(0.25*n)];
    double x50 = x[(int)(0.50*n)];
    double x75 = x[(int)(0.75*n)];
    double max = x[n - 1];
    double avg = accumulate(x, x + n, 0.0) / n;
    vector<double> mode;
    calculate_mode(x,n,mode,0.0001);

    cout << "当Y=50%时X的结果:" << endl;
    cout << "最小值 = " << min << endl;
    cout << "25分位点 = " << x25 << endl;
    cout << "中数 = " << x50 << endl;
    cout << "75分位点 = " << x75 << endl;
    cout << "最大值 = " << max << endl;
    cout << "均值 = " << avg << endl;
    cout << "众数 = " << mode.front() << " n = " << mode.size() << endl;
}

void run(int n)
{
    double *result = new double[n];
    calculate(n, result);
    std::sort(result, result + n);
    int skip = 0.025 * n;
    statistics(result + skip, n - 2*skip);
    delete [] result;
    cout << "请按任意键退出..." << endl;
    getchar();
}
