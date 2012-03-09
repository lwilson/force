#include <vector>
#include <string>
#include <iostream>
#include "problem.h"

using namespace std;

int main() {

  Codelet p;

  p.setName("foo");
  p.addParameter(make_pair("x","1:10"));
  p.addParameter(make_pair("y","1:10"));
  p.addParameter(make_pair("t","10"));

  p.addDepend("A[x-1][y][t-1]");
  p.addDepend("A[x][y-1][t-1]");
  p.addDepend("A[x][y][t-1]");

  p.addOutput("A[x][y][t]");
  p.addOutput("B[x][t]");

  vector<string> rdeps;
  vector<string> outs;

  bool match = p.matchesDep("A[1][0][10]", rdeps, outs);

  cout << "A[1][0][10]" << endl << match << endl;
  
  if(match) {
    for(int i=0; i<rdeps.size(); i++)
      cout << rdeps[i] << " ";
    cout << endl;
    for(int i=0; i<outs.size(); i++)
      cout << outs[i] << " ";
    cout << endl;
  }
  return 0;
}
