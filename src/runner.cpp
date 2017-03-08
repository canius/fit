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
#include <numeric>
#include <thread>
#include <fstream>
#include <atomic>
#include "fit8092.h"

using namespace std;

static std::atomic_int counter;
static int progress;

data8092 *generate(int n, ofstream &file)
{
	data8092 *data = new data8092[n];
	double *x = new double[8]{ 1.491,1.837,2.217,2.505,2.813,3.216,3.748,4.22 };
	double *ey = new double[8]{ 0.01,0.01,0.01,0.01,0.01,0.01,0.01,0.02 };
	double y_min[8] = { 92.5,85.0,62.5,47.5,13.5,4.0,0.4,0.05 };
	double y_max[8] = { 97.5,90.0,67.5,52.5,16.5,6.0,0.6,0.15 };

	cout << "正在生成" << n << "组均匀分布数据Y..." << endl;
	file << u8"均匀分布生成参数:" << endl;
	for (int i = 0; i < 8; i++) {
		cout << "x" << i+1 << " = " << x[i] << " y" << i+1 << " = [" << y_min[i] << "," << y_max[i] << "]" << endl;
		file << "x" << i+1 << " = " << x[i] << " y" << i+1 << " = [" << y_min[i] << "," << y_max[i] << "]" << endl;
	}
	file << endl;

	std::random_device rd;
	vector<std::mt19937> gens;
	vector<std::uniform_real_distribution<>> ds;
	for (int i = 0; i < 8; i++) {
		std::mt19937 gen(rd());
		gens.push_back(gen);
		std::uniform_real_distribution<> d(y_min[i], y_max[i]);
		ds.push_back(d);
	}

	data8092 *p = data;
	for (int i = 0; i < n; ++i) {
		double *y = new double[8];
		for (int i = 0; i < 8; i++) {
			y[i] = ds[i](gens[i]);
		}
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
        p++;
		if (counter++ > span) {
			counter = 0;
            cout << ++progress << "%" << endl;
		}
    } while (++i < n);
}

void fit(data8092 *data, int n)
{
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
	cout << endl;
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

int calculate_minWindow(double *x, int n, int size)
{
	int minIndex = 0;
	double *first = x;
	double *last = x + (size - 1);
	double minVal = *last - *first;
	int step = n - size;
	for (int i = 0; i < step; i++) {
		first++;
		last++;
		double val = *last - *first;
		if (val < minVal) {
			minVal = val;
			minIndex = i+1;
		}
	}
	return minIndex;
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

	int windowSize = (int)ceil(0.95 * (n / 10)) * 10;
	int windowFrom = calculate_minWindow(x, n, windowSize);
	int windowTo = windowFrom + windowSize - 1;
	double windowMinVal = x[windowTo] - x[windowFrom];

	cout << "当Y=50%时X的结果:" << endl;
	cout << "最小值 = " << min << endl;
	cout << "25分位点 = " << x25 << endl;
	cout << "中数 = " << x50 << endl;
	cout << "75分位点 = " << x75 << endl;
	cout << "最大值 = " << max << endl;
	cout << "众数 = " << mode.front() << " n = " << mode.size() << endl;
	cout << "均值 = " << mean << endl;
	cout << "标准差 = " << sd << endl;
	cout << windowSize << "范围内的最小区间 x" << windowFrom+1 << "=" << x[windowFrom] << " ~ x" << windowTo + 1 << "=" << x[windowTo] << " 差值 = " << windowMinVal << endl;

	file << u8"当Y=50%时X的结果:" << endl;
	file << u8"最小值 = " << min << endl;
	file << u8"25分位点 = " << x25 << endl;
	file << u8"中数 = " << x50 << endl;
	file << u8"75分位点 = " << x75 << endl;
	file << u8"最大值 = " << max << endl;
	file << u8"众数 = " << mode.front() << " n = " << mode.size() << endl;
	file << u8"均值 = " << mean << endl;
	file << u8"标准差 = " << sd << endl;
	file << windowSize << u8"范围内的最小区间 x" << windowFrom + 1 << "=" << x[windowFrom] << " ~ x" << windowTo + 1 << "=" << x[windowTo] << u8" 差值 = " << windowMinVal << endl;
	file << endl;

	delete[] x;
}

void run(int n)
{
	ofstream file;
	file.open("output.txt", std::ios::out);
	unsigned char bom[] = { 0xEF,0xBB,0xBF };
	file.write((char*)bom, sizeof(bom));

	data8092 *data = generate(n,file);
	data8092 *end = data + n;

	cout << "拟合8092曲线..." << endl;
	fit(data, n);

	data8092 *start = partition(data, end, [](const data8092 & a) -> bool
	{
		return isnan(a.output);
	});

	std::sort(start, end, [](const data8092 & a, const data8092 & b) -> bool
	{
		return a.output < b.output;
	});

	int exclude = int(start - data);
	if (exclude > 0) {
		cout << "排除" << exclude << "个溢出值" << endl;
	}

	statistics(start, n - (int)(start - data), file);

	cout << "写入文件..." << endl;
	file << u8"数据:" << endl;
	data8092 *p = start;
	while (p != end) {
		file << "x=" << p->output << endl;
		p++;
	}

	file.close();
	release(data, n);

	cout << "请按任意键退出..." << endl;
	getchar();
}
