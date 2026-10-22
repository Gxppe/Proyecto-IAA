#pragma once
#include <map>
#include <string>
#include <vector>

using namespace std;

struct Node {
  double x = 0;
  double y = 0;
  int w = 0;
  int q = 0;
};

struct Instance {
  vector<int> O;
  vector<int> N;
  vector<int> M;
  vector<int> K;
  int D = 0;
  int Q = 0;
  double v = 0;
  map<int, double> f;
  vector<Node> nodes;
  vector<vector<double>> dist;
};

Instance read_instance(const string &path);
