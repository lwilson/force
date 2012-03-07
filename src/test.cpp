#include <vector>
#include <string>
#include <iostream>
#include "problem.h"

int main() {

  KeyObject k("A[x][y][z+1][t]");

  vector<pair<string, string> > p;
  p.push_back(make_pair("x","0"));
  p.push_back(make_pair("y","0"));
  p.push_back(make_pair("z","10"));
  p.push_back(make_pair("t","0"));

  std::cout << k.toString(p) << std::endl;

  return 0;
}
