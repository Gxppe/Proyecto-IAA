#pragma once
#include "instance.h"
#include <vector>

using namespace std;

typedef vector<vector<int>> Solution;

Solution empty_solution(const Instance &I);

void print_solution(const Instance &I, const Solution &sol);

bool is_feasible(const Instance &I, const Solution &sol);
