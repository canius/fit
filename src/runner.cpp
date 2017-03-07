﻿//
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
#include <numeric>
#include <thread>
#include <fstream>
#include <atomic>
#include "fit8092.h"

using namespace std;

static std::atomic_int counter;
static int progress;

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

data8092 *generate(int n)
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
    
	data8092 *data = new data8092[n];
	double *x = new double[8]{ 1.491,1.837,2.217,2.505,2.813,3.216,3.748,4.22 };
	double *ey = new double[8]{ 0.01,0.01,0.01,0.01,0.01,0.01,0.01,1.0 };

	data8092 *p = data;
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
		p->x = x;
		p->y = y;
		p->ey = ey;
		p->n = 8;
		p->p = new double[5]{ 1.0,1.0,1.0,1.0,1.0 };
		p->np = 5;
		p++;
    }
    return data;
}

void release(data8092 *data,int n)
{
	data8092 *p = data;
	delete[] p->x;
	delete[] p->ey;
	for (int i = 0; i < n; ++i) {
		delete[] p->y;
		delete[] p->p;
		p++;
	}
	delete[] data;
}

void thread_call(int tid, data8092 *data,int n,int total)
{
	int span = total / 100;
    int i = 0;
	data8092 *p = data;
    do {
        p->output = fit8092(p, 50.0);
        if (isnan(p->output)) {
            print(p);
        }
        p++;
		if (counter++ > span) {
			counter = 0;
			cout << ++progress << "%" << endl;
		}
    } while (++i < n);
}

void fit(data8092 *data, int n)
{
	cout << "拟合8092曲线..." << endl;

	const int num_threads = 8;
	std::thread threads[num_threads];

	int sub_num = n / num_threads;
	int last_num = n - (num_threads - 1) * sub_num;
	data8092 *p = data;
	counter = 0;
	for (int i = 0; i < num_threads; ++i) {
        int step = i == num_threads - 1 ? last_num : sub_num;
		threads[i] = std::thread(thread_call, i, p, step, n);
		p += step;
	}

	for (int i = 0; i < num_threads; ++i) {
		threads[i].join();
	}
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

void statistics(data8092 *data, int n, ofstream &file)
{
    if (n == 0) {
        return;
    }
	double *x = new double[n];
	data8092 *p = data;
	for (int i = 0; i < n; ++i) {
		x[i] = p->output;
		p++;
	}

	double min = x[0];
	double x25 = x[(int)(0.25*n)];
	double x50 = x[(int)(0.50*n)];
	double x75 = x[(int)(0.75*n)];
	double max = x[n - 1];
	double mean = accumulate(x, x + n, 0.0) / n;

	double sd = 0;
	for (int i = 0; i < n; ++i) {
		sd += pow(x[i] - mean, 2);
	}
	sd = sqrt(sd / mean);

	vector<double> mode;
	calculate_mode(x, n, mode, 0.0001);

	cout << "当Y=50%时X的结果:" << endl;
	cout << "最小值 = " << min << endl;
	cout << "25分位点 = " << x25 << endl;
	cout << "中数 = " << x50 << endl;
	cout << "75分位点 = " << x75 << endl;
	cout << "最大值 = " << max << endl;
	cout << "众数 = " << mode.front() << " n = " << mode.size() << endl;
	cout << "均值 = " << mean << endl;
	cout << "标准差 = " << sd << endl;

	cout << "写入文件..." << endl;

	file << u8"当Y=50%时X的结果:" << endl;
	file << u8"最小值 = " << min << endl;
	file << u8"25分位点 = " << x25 << endl;
	file << u8"中数 = " << x50 << endl;
	file << u8"75分位点 = " << x75 << endl;
	file << u8"最大值 = " << max << endl;
	file << u8"众数 = " << mode.front() << " n = " << mode.size() << endl;
	file << u8"均值 = " << mean << endl;
	file << u8"标准差 = " << sd << endl;
	file << endl;
	file << u8"正态分布生成参数:" << endl;
	file << "x1 = " << data->x[0] << " y1 = 95 +- 5" << endl; 
	file << "x2 = " << data->x[1] << " y2 = 87.5 +- 4" << endl;
	file << "x3 = " << data->x[2] << " y3 = 65 +- 3" << endl;
	file << "x4 = " << data->x[3] << " y4 = 50 +- 2" << endl;
	file << "x5 = " << data->x[4] << " y5 = 15 +- 1" << endl;
	file << "x6 = " << data->x[5] << " y6 = 5 +- 1" << endl;
	file << "x7 = " << data->x[6] << " y7 = 0.5 +- 0.1" << endl;
	file << "x8 = " << data->x[7] << " y8 = 0.1 +- 0.01" << endl;
	file << endl;

	delete[] x;
}

double calculate_fit(double *y)
{
    double error = 0.01;
    data8092 data;
    double x[8] = { 1.491,1.837,2.217,2.505,2.813,3.216,3.748,4.22 };
    double ey[8] = { error,error,error,error,error,error,error,0.1 };
    double p[5] = { 1.0, 1.0, 1.0, 1.0, 1.0 };
    data.x = x;
    data.y = y;
    data.ey = ey;
    data.n = 8;
    data.p = p;
    data.np = 5;
    double output = fit8092(&data, 50.0);
    for (int i = 0; i < 5; ++i) {
        cout << data.p[i] << " ";
    }
    cout << endl;
    return output;
}

void wrong_test()
{
    double y[8] = {91.542409750962676, 79.522345573289584, 54.383359109550653, 49.113343322319032, 14.856214592949819, 3.2337041734557759, 0.47242181348390755, 0.10179758745913106 };
    cout << calculate_fit(y) << endl;
}

void run(int n)
{
//    wrong_test();
//    return;

	cout << "正在生成" << n << "组正态分布数据Y..." << endl;
	data8092 *data = generate(n);
	fit(data, n);

	data8092 *p = partition(data, data + n, [](const data8092 & a) -> bool
	{
		return isnan(a.output);
	});

	std::sort(p, data + n,[](const data8092 & a, const data8092 & b) -> bool
	{
		return a.output < b.output;
	});

	ofstream file;
	file.open("output.txt", std::ios::out);
	unsigned char bom[] = { 0xEF,0xBB,0xBF };
	file.write((char*)bom, sizeof(bom));

	statistics(p, n - (int)(p - data), file);
    
    file << u8"生成数据:" << endl;
    p = data;
    for (int i = 0; i < n; i++) {
        file << "x=" << p->output << " a=" << p->p[0] << " b=" << p->p[1] << " c=" << p->p[2] << " d=" << p->p[3] << " e=" << p->p[4] << " ";
        for (int j = 0; j < p->n; j++) {
            file << "y" << j + 1 << "=" << p->y[j] << " ";
        }
        file << endl;
        p++;
    }
    
	file.close();
	release(data, n);

	cout << "请按任意键退出..." << endl;
	getchar();
}
